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
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <getopt.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/bcd.h"

void show_driver_info (sbig_t sb, int ac, char **av);
void show_ccd_info (sbig_t sb, CCD_INFO_REQUEST, int ac, char **av);

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-info driver\n"
"       sbig-info imaging-ccd\n"
"       sbig-info tracking-ccd\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    sbig_t sb;
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    int e;
    int ch;
    char *cmd;

    log_init ("sbig-info");

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (optind == argc)
        usage ();
    cmd = argv[optind++];
 
    if (!sbig_udrv)
        msg_exit ("SBIG_UDRV is not set");
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != 0)
        msg_exit ("sbig_open_driver: %s", sbig_strerror (e));

    if (!strcmp (cmd, "imaging-ccd"))
        show_ccd_info (sb, CCD_INFO_IMAGING, argc - optind, argv + optind);
    else if (!strcmp (cmd, "tracking-ccd"))
        show_ccd_info (sb, CCD_INFO_TRACKING, argc - optind, argv + optind);
    else if (!strcmp (cmd, "driver"))
        show_driver_info (sb, argc - optind, argv + optind);
    else
        usage ();

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

void show_driver_info (sbig_t sb, int ac, char **av)
{
    GetDriverInfoResults0 info;
    int e;
    char version[16];

    if (ac != 0)
        msg_exit ("driver takes no arguments");
    if ((e = sbig_get_driver_info (sb, DRIVER_STD, &info)) != 0)
        msg_exit ("sbig_get_driver_info: %s", sbig_strerror (e));
    bcd4str (info.version, version, sizeof (version));
    msg ("version: %s", version);
    msg ("name:    %s", info.name);
    msg ("maxreq:  %d", info.maxRequest);
}

void show_ccd_info (sbig_t sb, CCD_INFO_REQUEST request, int ac, char **av)
{
    int i, e;
    CAMERA_TYPE type;
    GetCCDInfoResults0 info;
    char version[16];

    if (ac != 0)
        msg_exit ("device takes no arguments");
    if ((e = sbig_open_device (sb, DEV_USB1)) != 0)
        msg_exit ("sbig_open_device: %s", sbig_strerror (e));
    if ((e = sbig_establish_link (sb, &type)) != 0)
        msg_exit ("sbig_establish_link: %s", sbig_strerror (e));
    if ((e = sbig_get_ccd_info (sb, request, &info)) != 0)
        msg_exit ("sbig_get_ccd_info: %s", sbig_strerror (e));

    bcd4str (info.firmwareVersion, version, sizeof (version));
    msg ("firmware-version: %s", version); 
    msg ("camera-type:      %s", sbig_strdevice (info.cameraType));
    msg ("name:             %s", info.name); 
    msg ("readout-modes:");
    for (i = 0; i < info.readoutModes; i++) {
        msg ("%d: %dx%d, %2.2f e-/ADU, %6.2fx%6.2f microns",
             info.readoutInfo[i].mode,
             info.readoutInfo[i].width,
             info.readoutInfo[i].height,
             bcd2_2 (info.readoutInfo[i].gain),
             bcd6_2 (info.readoutInfo[i].pixelWidth),
             bcd6_2 (info.readoutInfo[i].pixelHeight));
    } 
    if (request == CCD_INFO_IMAGING) {
        GetCCDInfoResults2 xinfo;
        if ((e = sbig_get_ccd_info_ext (sb, &xinfo)) != 0)
            msg_exit ("sbig_get_ccd_info_ext: %s", sbig_strerror (e));
        msg ("bad columns:       %d", xinfo.badColumns);
        msg ("ABG:               %s", xinfo.imagingABG == ABG_PRESENT ? "yes"
                                                                      : "no");
        msg ("serial-number:     %s", xinfo.serialNumber);
    }

    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_strerror (e));
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
