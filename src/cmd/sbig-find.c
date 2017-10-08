/*****************************************************************************\
 *  Copyright (c) 2014 Jim Garlick All rights reserved.
 *
 *  This file is part of the sbig-util.
 *  For details, see https://github.com/garlick/sbig-util.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 3 of the license, or (at your option)
 *  any later version.
 *
 *  sbig-util is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <dlfcn.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"

void show_eth (sbig_t *sb);
void show_usb (sbig_t *sb);
void show_lpt (sbig_t *sb);

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-find [eth] [usb] [lpt]\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    sbig_t *sb;
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    int e;
    int ch;
    int find_eth = 0;
    int find_usb = 0;
    int find_lpt = 0;

    log_init ("sbig-find");

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (optind == argc) {
        find_eth = find_usb = find_lpt = 1;
    } else {
        while (optind < argc) {
            if (!strcasecmp (argv[optind], "eth"))
                find_eth = 1;
            else if (!strcasecmp (argv[optind], "usb"))
                find_usb = 1;
            else if (!strcasecmp (argv[optind], "lpt"))
                find_lpt = 1;
            else
                usage ();
            optind++;
        }
    }

    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != 0)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));

    if (find_eth)
        show_eth (sb);
    if (find_usb)
        show_usb (sb);
    if (find_lpt)
        show_lpt (sb);

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

void show_eth (sbig_t *sb)
{
    int i, e;
    QueryEthernetResults devices;

    e = sbig_query_ethernet (sb, &devices);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_query_ethernet: %s", sbig_get_error_string (sb, e));
    for (i = 0; i < devices.camerasFound; i++) {
        if (devices.ethernetInfo[i].cameraFound)
            printf ("%lu.%lu.%lu.%lu: %s '%s' %s\n",
                 (devices.ethernetInfo[i].ipAddress >> 24) & 0xff,
                 (devices.ethernetInfo[i].ipAddress >> 16) & 0xff,
                 (devices.ethernetInfo[i].ipAddress >> 8) & 0xff,
                 devices.ethernetInfo[i].ipAddress & 0xff,
                 sbig_strcam (devices.ethernetInfo[i].cameraType),
                 devices.ethernetInfo[i].name,
                 devices.ethernetInfo[i].serialNumber);
    }
}

void show_usb (sbig_t *sb)
{
    int i, e;
    QueryUSBResults devices;

    e = sbig_query_usb (sb, &devices);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_query_usb: %s", sbig_get_error_string (sb, e));
    for (i = 0; i < devices.camerasFound; i++) {
        if (devices.usbInfo[i].cameraFound)
            printf ("USB%d: %s '%s' %s\n", i + 1,
                 sbig_strcam (devices.usbInfo[i].cameraType),
                 devices.usbInfo[i].name,
                 devices.usbInfo[i].serialNumber);
    }
}

void show_device_info (sbig_t *sb, const char *device)
{
    sbig_ccd_t *ccd;
    GetCCDInfoResults0 info0;
    GetCCDInfoResults2 info2;
    int e;
    const char *serial = "serial-unknown";

    if ((e = sbig_ccd_create (sb, CCD_IMAGING, &ccd)))
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));
    e = sbig_ccd_get_info0 (ccd, &info0);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_get_info: %s", sbig_get_error_string (sb, e));
    e = sbig_ccd_get_info2 (ccd, &info2);
    if (e == CE_NO_ERROR)
        serial = info2.serialNumber;
    printf ("%s: %s '%s' %s\n", device,
        sbig_strcam (info0.cameraType),
        info0.name,
        serial);
}

void show_lpt (sbig_t *sb)
{
    int i, e;
    char device[8];
    CAMERA_TYPE type;

    for (i = 1; i <= 3; i++) {
        snprintf (device, sizeof (device), "LPT%d", i);

        e = sbig_open_device (sb, device);
        if (e != CE_NO_ERROR)
            continue;

        e = sbig_establish_link (sb, &type);
        if (e != CE_NO_ERROR) {
            msg ("sbig_establish_link %s: %s",
                 device, sbig_get_error_string (sb, e));
            goto close_continue;
        }

        show_device_info (sb, device);

close_continue:
        e = sbig_close_device (sb);
        if (e != CE_NO_ERROR) {
            msg ("sbig_close_device %s: %s",
                 device, sbig_get_error_string (sb, e));
        }
    }
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
