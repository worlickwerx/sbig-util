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
#include <unistd.h>
#include <sys/param.h>
#include <time.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/bcd.h"

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

char *ctime_iso8601_now (char *buf, size_t sz);

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-snap [OPTIONS]\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    sbig_t sb;
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    int e;
    int ch;
    sbig_ccd_t ccd;
    CAMERA_TYPE type;
    CCD_REQUEST chip = CCD_IMAGING;
    PAR_COMMAND_STATUS status;
    char filename[PATH_MAX];
    char date[64];
    double t = 0.2;
    bool verbose = true;

    log_init ("sbig-info");

    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (optind != argc)
        usage ();

    snprintf (filename, sizeof (filename), "/mnt/img/LF_%s.png",
              ctime_iso8601_now (date, sizeof (date)));

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
    if (verbose)
        msg ("Device open");
    if ((e = sbig_establish_link (sb, &type)) != 0)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));
    if (verbose)
        msg ("Link established to %s", sbig_strcam (type));
    if ((e = sbig_ccd_create (sb, chip, &ccd)))
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_ccd_set_readout_mode (ccd, RM_1X1)))
        msg_exit ("sbig_ccd_set_readout_mode");

    /* Just in case we left an exposure going, end it
     */
    if ((e = sbig_ccd_end_exposure (ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure");
    if (verbose)
        msg ("exposure: end");
    if ((e = sbig_ccd_start_exposure (ccd, t)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_start_exposure");
    if (verbose)
        msg ("exposure: start");
    //sleep (t);
    do {
        if ((e = sbig_ccd_get_exposure_status (ccd, &status)) != CE_NO_ERROR)
            msg_exit ("sbig_ccd_start_exposure");
        if (verbose)
            msg ("exposure: %d", status);
        if (status != CS_INTEGRATION_COMPLETE)
            usleep (100000); /* 100 msec */
    } while (status != CS_INTEGRATION_COMPLETE);
    if ((e = sbig_ccd_end_exposure (ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure");
    if (verbose)
        msg ("exposure: end");
    if ((e = sbig_ccd_readout (ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure");
    if (verbose) {
        unsigned short max;
        unsigned short top, left, h, w;
        if ((e = sbig_ccd_get_max (ccd, &max)) != CE_NO_ERROR)
            msg_exit ("sbig_ccd_get_max");
        msg ("readout complete (max pixel %hu)", max);
        if ((e = sbig_ccd_get_window (ccd, &top, &left, &h, &w)) != CE_NO_ERROR)
            msg_exit ("sbig_ccd_get_window"); 
        msg ("image is %hux%hu", h, w); /* FIXME */
    }

    if ((e = sbig_ccd_writepgm (ccd, filename)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_writepgm %s", filename);
    if (verbose)
        msg ("wrote image to %s", filename);

    sbig_ccd_destroy (ccd);
    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

char *ctime_iso8601_now (char *buf, size_t sz)
{
    struct tm tm;
    time_t now = time (NULL);

    memset (buf, 0, sz);

    if (!localtime_r (&now, &tm))
        return (NULL);
    strftime (buf, sz, "%FT%T", &tm);

    return (buf);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
