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

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-cooler [OPTIONS] mode setpoint\n"
"  where mode is: off|on|override|freeze|unfreeze|auto|noauto\n"
"  and setpoint is in degrees C (sbig recommends 30 degrees below ambient)\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    const char *sbig_device = getenv ("SBIG_DEVICE");
    sbig_t sb;
    int e;
    int ch;
    CAMERA_TYPE type;
    SBIG_DEVICE_TYPE device;
    double setpoint;
    char *modestr;
    TEMPERATURE_REGULATION mode;

    log_init ("sbig-cooler");

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (optind != argc - 2)
        usage ();
    modestr = argv[optind++];
    if (!strcmp (modestr, "off"))
        mode = REGULATION_OFF;
    else if (!strcmp (modestr, "on"))
        mode = REGULATION_ON;
    else if (!strcmp (modestr, "override"))
        mode = REGULATION_OVERRIDE;
    else if (!strcmp (modestr, "freeze"))
        mode = REGULATION_FREEZE;
    else if (!strcmp (modestr, "unfreeze"))
        mode = REGULATION_UNFREEZE;
    else if (!strcmp (modestr, "auto"))
        mode = REGULATION_ENABLE_AUTOFREEZE;
    else if (!strcmp (modestr, "noauto"))
        mode = REGULATION_DISABLE_AUTOFREEZE;
    else
        usage ();
    setpoint = strtod (argv[optind++], NULL);

    if (!sbig_udrv)
        msg_exit ("SBIG_UDRV is not set");
    if (!sbig_device)
        msg_exit ("SBIG_DEVICE is not set");
    device = sbig_devstr (sbig_device);
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != CE_NO_ERROR)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != CE_NO_ERROR)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_open_device (sb, device)) != CE_NO_ERROR)
        msg_exit ("sbig_open_device: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_establish_link (sb, &type)) != CE_NO_ERROR)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_temp_set (sb, mode, setpoint) != CE_NO_ERROR))
        msg_exit ("sbig_temp_set: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_close_device (sb)) != CE_NO_ERROR)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
