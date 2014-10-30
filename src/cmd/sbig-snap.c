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
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <math.h> /* fabs */

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/xzmalloc.h"
#include "src/common/libsbig/sbfits.h"
#include "src/common/libini/ini.h"

typedef struct snap_struct {
    SBIG_DEVICE_TYPE device;
    CCD_REQUEST chip;
    READOUT_BINNING_MODE readout_mode;
    double t;
    int count;
    double time_delta;
    char *imagedir;
    const char *message;
    bool verbose;
    char *observer;
    char *telescope;
    char *filter;
    const char *object;
    double focal_length;
    double aperture_diameter;
    double aperture_area;
    char *sitename;
    char *latitude;
    char *longitude;
    double elevation;
} snap_t;

/* FIXME: add 1/2 and 1/4 (centered) subframe modes */
/* FIXME: add fast cycling focus mode */
/* FIXME: add image monitoring for focus/centering (show contrast FOM) */

#define OPTIONS "ht:d:C:r:n:D:m:O:f"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {"exposure-time", required_argument,     0, 't'},
    {"image-directory", required_argument,   0, 'd'},
    {"chip",          required_argument,     0, 'C'},
    {"resolution",    required_argument,     0, 'r'},
    {"count",         required_argument,     0, 'n'},
    {"time-delta",    required_argument,     0, 'D'},
    {"message",       required_argument,     0, 'm'},
    {"object",        required_argument,     0, 'O'},
    {"force",         no_argument,           0, 'f'},
    {0, 0, 0, 0},
};

void snap_series (sbig_t sb, snap_t snap);
int config_cb (void *user, const char *section, const char *name,
               const char *value);

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-snap [OPTIONS]\n"
"  -t, --exposure-time DOUBLE exposure time in seconds (default 1.0)\n"
"  -d, --image-directory DIR  where to put images (default /mnt/img)\n"
"  -C, --ccd-chip CHIP        use imaging, tracking, or ext-tracking\n"
"  -r, --resolution RES       select hi, med, or lo resolution\n"
"  -n, --count N              take N exposures\n"
"  -D, --time-delta N         increase exposure time by N on each exposure\n"
"  -m, --message string       add annotation to FITS file\n"
"  -O, --object NAME          name of object being observed (e.g. M33)\n"
"  -f, --force                press on even if FITS header will be incomplete\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    const char *config_filename = getenv ("SBIG_CONFIG_FILE");
    int e, ch;
    sbig_t sb;
    snap_t opt;
    CAMERA_TYPE type;
    bool force = false;

    log_init ("sbig-snap");

    /* Set default option values.
     */
    memset (&opt, 0, sizeof (opt));
    opt.device = DEV_USB1;          /* first USB device */
    opt.chip = CCD_IMAGING;         /* main imaging ccd */
    opt.readout_mode = RM_1X1;      /* high resolution */
    opt.imagedir = xstrdup("/mnt/img"); /* where to write files */
    opt.t = 1.0;                    /* 1s exposure time */
    opt.count = 1;                  /* one exposure */
    opt.verbose = true;

    /* Override defaults with config file
     */
    if (!config_filename)
        msg_exit ("SBIG_CONFIG_FILE is not set");
    if (ini_parse (config_filename, config_cb, &opt) < 0)
        msg ("cannot load %s", config_filename);

    /* Override defaults and config file with command line
     */
    optind = 0;
    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'f': /* --force */
                force = true; 
                break;
            case 'c': /* --config FILE */
                /* already parsed in first pass */
                break;
            case 'O': /* --object NAME */
                opt.object = optarg;
                break;
            case 'm': /* --message string */
                opt.message = optarg;
                break;
            case 'n': /* --count N */
                opt.count = strtoul (optarg, NULL, 10);
                break;
            case 'D': /* --time-delta N */
                opt.time_delta = strtod (optarg, NULL);
                if (opt.time_delta < 0 || opt.time_delta > 86400)
                    msg_exit ("error parsing --time-delta argument");
                break;
            case 't': /* --exposure-time SEC */
                opt.t = strtod (optarg, NULL);
                if (opt.t < 0 || opt.t > 86400)
                    msg_exit ("error parsing --exposure-time argument");
                break;
            case 'C': /* --ccd-chip CHIP */
                if (!strcmp (optarg, "imaging"))
                    opt.chip = CCD_IMAGING;
                else if (!strcmp (optarg, "tracking"))
                    opt.chip = CCD_TRACKING;
                else if (!strcmp (optarg, "ext-tracking"))
                    opt.chip = CCD_EXT_TRACKING;
                else
                    msg_exit ("error parsing --ccd-chip argument (imaging, tracking, ext-tracking)");
                break;
            case 'd': /* --image-directory DIR */
                if (opt.imagedir)
                    free (opt.imagedir);
                opt.imagedir = xstrdup (optarg);
                break;
            case 'r': /* --resolution hi|med|lo */
                if (!strcmp (optarg, "hi"))
                    opt.readout_mode = RM_1X1;
                else if (!strcmp (optarg, "med"))
                    opt.readout_mode = RM_2X2;
                else if (!strcmp (optarg, "lo"))
                    opt.readout_mode = RM_3X3;
                else
                    msg_exit ("error parsing --resolution (hi, med, lo)");
                break;
            case 'h': /* --help */
            default:
                usage ();
        }
    }
    if (optind != argc)
        usage ();

    /* Verify we have all the info we need for a complete FITS header.
     */
    if (!force) {
        if (!opt.object)
            msg_exit ("Please specify --object or --force");
        if (!opt.telescope || !opt.observer || opt.focal_length == 0
                || opt.aperture_diameter == 0 || opt.aperture_area == 0)
            msg_exit ("Please populate config file or --force");
    }

    /* Connect to driver
     */
    if (!sbig_udrv)
        msg_exit ("SBIG_UDRV is not set");
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != CE_NO_ERROR)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));

    /* Open camera
     */
    if ((e = sbig_open_device (sb, opt.device)) != CE_NO_ERROR)
        msg_exit ("sbig_open_device: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("Device open");
    if ((e = sbig_establish_link (sb, &type)) != CE_NO_ERROR)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("Link established to %s", sbig_strcam (type));

    /* Take pictures.
     */
    snap_series (sb, opt);

    /* Clean up.
     * N.B. this does not reset the camera's TE cooler
     */
    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));

    if (opt.observer)
        free (opt.observer);
    if (opt.telescope)
        free (opt.telescope);
    if (opt.filter)
        free (opt.filter);
    if (opt.imagedir)
        free (opt.imagedir);
    if (opt.sitename)
        free (opt.sitename);
    if (opt.latitude)
        free (opt.latitude);
    if (opt.longitude)
        free (opt.longitude);

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

int config_cb (void *user, const char *section, const char *name,
               const char *value)
{
    snap_t *opt = user;

    if (!strcmp (section, "system")) {
        if (!strcmp (name, "imagedir")) {
            if (opt->imagedir)
                free (opt->imagedir);
            opt->imagedir = xstrdup (value);
        } else if (!strcmp (name, "device")) {
            opt->device = sbig_devstr (value);
        }
    } else if (!strcmp (section, "cfw")) {
        /* XXX set filter to slot mapping here */
    } else if (!strcmp (section, "config")) {
        if (!strcmp (name, "observer"))
            opt->observer = xstrdup (value);
        else if (!strcmp (name, "telescope"))
            opt->telescope = xstrdup (value);
        else if (!strcmp (name, "filter"))
            opt->filter = xstrdup (value);
        else if (!strcmp (name, "focal_length"))
            opt->focal_length = strtod (value, NULL);
        else if (!strcmp (name, "aperture_diameter"))
            opt->aperture_diameter = strtod (value, NULL);
        else if (!strcmp (name, "aperture_area"))
            opt->aperture_area = strtod (value, NULL);
    } else if (!strcmp (section, "site")) {
        if (!strcmp (name, "name"))
            opt->sitename = xstrdup (value);
        else if (!strcmp (name, "elevation"))
            opt->elevation = strtod (value, NULL);
        else if (!strcmp (name, "latitude"))
            opt->latitude = xstrdup (value);
        else if (!strcmp (name, "longitude"))
            opt->longitude = xstrdup (value);
    }

    return 0; /* 0=success, 1=error */
}

/* Wait for an exposure in progress to complete.
 * We avoid polling the camera excessively.
 */
void exposure_wait (sbig_t sb, sbig_ccd_t ccd, snap_t opt)
{
    PAR_COMMAND_STATUS status;
    int e;

    usleep (1E6 * opt.t);
    do {
        if ((e = sbig_ccd_get_exposure_status (ccd, &status)) != CE_NO_ERROR)
            msg_exit ("sbig_get_exposure_status: %s", sbig_get_error_string (sb, e));
        if (opt.verbose)
            fprintf (stderr, ".");
        if (status != CS_INTEGRATION_COMPLETE)
            usleep (1E3 * 500); /* 500ms */
    } while (status != CS_INTEGRATION_COMPLETE);
    if (opt.verbose)
        fprintf (stderr, "\n");
}

typedef enum { SNAP_DF, SNAP_LF, SNAP_LF_SUB } snap_type_t;

/* Take a picture:
 * SNAP_DF: take a dark frame
 * SNAP_LF: take a light frame
 * SNAP_LF_SUB: take a light frame, subtracting previous DF during readout
 */
void snap (sbig_t sb, sbig_ccd_t ccd, snap_t opt, snap_type_t type, int seq)
{
    int e;

    /* Set shutter mode
     */
    if (type == SNAP_DF)
        e = sbig_ccd_set_shutter_mode (ccd, SC_CLOSE_SHUTTER);
    else
        e = sbig_ccd_set_shutter_mode (ccd, SC_OPEN_SHUTTER);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_set_shutter_mode: %s", sbig_get_error_string (sb, e));

    /* Start exposure, then wait for it to finish.
     */
    if ((e = sbig_ccd_start_exposure (ccd, 0, opt.t)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_start_exposure: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("exposure: start %s #%d (%.2fs)", type == SNAP_DF ? "DF" : "LF",
             seq, opt.t);
    exposure_wait (sb, ccd, opt);

    /* Finalize exposure, then read out from camera to sbig_ccd_t internal
     * buffer.  Subtract a previous DF left there if type is SNAP_LF_SUB.
     */
    if ((e = sbig_ccd_end_exposure (ccd, 0)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("exposure: end %s #%d", type == SNAP_DF ? "DF" : "LF", seq);
    if (type == SNAP_LF_SUB)
        e = sbig_ccd_readout_subtract (ccd);
    else
        e = sbig_ccd_readout (ccd);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_readout: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("readout: %s%s #%d complete", type == SNAP_DF ? "DF" : "LF",
             type == SNAP_LF_SUB ? " (subtracted)" : "", seq);
}

/* Return current CCD temperature in degrees C
 */
double snap_temp (sbig_t sb, snap_t opt)
{
    QueryTemperatureStatusResults2 temp;
    int e;
    if ((e = sbig_temp_get_info (sb, &temp)) != CE_NO_ERROR)
        msg_exit ("sbig_temp_get_info: %s", sbig_get_error_string (sb, e));
    if (opt.verbose) {
        msg ("cooler: %s setpoint %.2fC ccd %.2fC ambient %.2fC",
             temp.coolingEnabled ? "enabled" : "disabled",
             temp.ccdSetpoint,
             temp.imagingCCDTemperature,
             temp.ambientTemperature);
    }
    return temp.imagingCCDTemperature;
}

void snap_one_autodark (sbig_t sb, sbig_ccd_t ccd, snap_t opt, int seq)
{
    double dt, temp_lf, temp_df;
    sbfits_t sbf;

    /* Create FITS file for output.
     */
    sbf = sbfits_create ();
    if (sbfits_create_file (sbf, opt.imagedir, "LF") < 0)
        msg_exit ("%s: %s", sbfits_get_filename (sbf), sbfits_get_errstr (sbf));

    /* Dark frame
     */
    do {
        temp_df = snap_temp (sb, opt);
        snap (sb, ccd, opt, SNAP_DF, seq);

        temp_lf = snap_temp (sb, opt);
        dt = fabs (temp_df - temp_lf);
        if (dt > 1) /* FIXME: 1C threshold is somewhat arbitrary */
            msg ("Retaking DF as CCD temp was not stable");
    } while (dt > 1);

    snap (sb, ccd, opt, SNAP_LF_SUB, seq);

    /* Write out FITS file
     */
    sbfits_set_ccdinfo (sbf, ccd);
    sbfits_set_temperature (sbf, temp_lf);
    sbfits_set_annotation (sbf, opt.message);
    sbfits_set_observer (sbf, opt.observer);
    sbfits_set_telescope (sbf, opt.telescope);
    sbfits_set_filter (sbf, opt.filter);
    sbfits_set_focal_length (sbf, opt.focal_length);
    sbfits_set_aperture_diameter (sbf, opt.aperture_diameter);
    sbfits_set_aperture_area (sbf, opt.aperture_area);
    sbfits_set_object (sbf, opt.object);
    sbfits_set_site (sbf, opt.sitename, opt.latitude, opt.longitude,
                     opt.elevation);

    if (sbfits_write_file (sbf) < 0)
        err_exit ("sbfits_write: %s", sbfits_get_errstr (sbf));
    if (sbfits_close_file (sbf))
        err_exit ("sbfits_close: %s", sbfits_get_errstr (sbf));
    if (opt.verbose)
        msg ("wrote %s", sbfits_get_filename (sbf));
    sbfits_destroy (sbf);
}

void snap_series (sbig_t sb, snap_t opt)
{
    int e, i;
    sbig_ccd_t ccd;

    if ((e = sbig_ccd_create (sb, opt.chip, &ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));

    /* Abort any in-progress exposure
     */
    if ((e = sbig_ccd_end_exposure (ccd, ABORT_DONT_END)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("exposure: abort (just in case)");
    /* FIXME: could verify that camera is idle here */

    /* Set up the readout binning mode which we hold constant over a series.
     */
    if ((e = sbig_ccd_set_readout_mode (ccd, opt.readout_mode)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_set_readout_mode: %s", sbig_get_error_string (sb, e));

    /* Take series of images and write them out as FITS files.
     * Optionally increase the exposure time by time_delta on each exposure.
     */
    for (i = 0; i < opt.count; i++) {
        snap_one_autodark (sb, ccd, opt, i);
        opt.t += opt.time_delta;
    }

    sbig_ccd_destroy (ccd);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
