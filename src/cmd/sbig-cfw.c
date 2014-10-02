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

void cfw_query (sbig_t sb, int ac, char **av);
void cfw_goto (sbig_t sb, int ac, char **av);

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-cfw query\n"
"       sbig-cfw goto N\n"
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
    CAMERA_TYPE type;

    log_init ("sbig-cfw");

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
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_open_device (sb, DEV_USB1)) != 0)
        msg_exit ("sbig_open_device: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_establish_link (sb, &type)) != 0)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));

    if (!strcmp (cmd, "query"))
        cfw_query (sb, argc - optind, argv + optind);
    else if (!strcmp (cmd, "goto"))
        cfw_goto (sb, argc - optind, argv + optind);
    else
        usage ();

    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

void cfw_goto (sbig_t sb, int ac, char **av)
{
    int e;
    CFW_STATUS status;
    CFW_POSITION position, actual;

    if (ac != 1)
        usage ();
    position = strtoul (av[0], NULL, 10);
    
    if ((e = sbig_cfw_goto (sb, position)) != CE_NO_ERROR)
        msg_exit ("sbig_cfw_goto: %s", sbig_get_error_string (sb, e));
    do {
        if ((e = sbig_cfw_query (sb, &status, &actual)) != CE_NO_ERROR)
            msg_exit ("sbig_cfw_query: %s", sbig_get_error_string (sb, e));
        if (status == CFWS_BUSY)
            usleep (1000*100);
    } while (status == CFWS_BUSY);
    if (actual == CFWP_UNKNOWN)
        msg ("position: unknown");
    else
        msg ("position: %d", actual);
}

void cfw_query (sbig_t sb, int ac, char **av)
{
    int e;
    CFW_STATUS status;
    CFW_POSITION position;
    if (ac != 0)
        msg_exit ("show takes no arguments");
    if ((e = sbig_cfw_query (sb, &status, &position)) != CE_NO_ERROR)
        msg_exit ("sbig_cfw_query: %s", sbig_get_error_string (sb, e));
        msg ("status:   %s", status == CFWS_UNKNOWN ? "unknown" :
                             status == CFWS_IDLE ? "idle" : "busy");
    if (position == CFWP_UNKNOWN)
        msg ("position: unknown");
    else
        msg ("position: %d", position);
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
