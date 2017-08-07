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
#include <sys/wait.h>
#include <pwd.h>
#include <time.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/xzmalloc.h"
#include "src/common/libsbig/sbfits.h"

typedef struct opt_struct {
    CCD_REQUEST chip;
    READOUT_BINNING_MODE readout_mode;
    double partial;
    double t;
    bool verbose;
} opt_t;

#define OPTIONS "ht:C:r:p:"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {"exposure-time", required_argument,     0, 't'},
    {"chip",          required_argument,     0, 'C'},
    {"resolution",    required_argument,     0, 'r'},
    {"partial",       required_argument,     0, 'p'},
    {0, 0, 0, 0},
};

static bool interrupted = false;

void snap_series (sbig_t *sb, opt_t opt);

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-focus [OPTIONS]\n"
"  -t, --exposure-time SEC exposure time in seconds (default 1.0)\n"
"  -C, --ccd-chip CHIP     use imaging, tracking, or ext-tracking\n"
"  -r, --resolution RES    select hi, med, or lo resolution\n"
"  -p, --partial N         take centered partial frame (0 < N <= 1.0)\n"
);
    exit (1);
}

void handle_sigint (int signal)
{
    msg ("interrupted: aborting");
    interrupted = true;
}

int main (int argc, char *argv[])
{
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    const char *sbig_device = getenv ("SBIG_DEVICE");
    int e, ch;
    sbig_t *sb;
    CAMERA_TYPE type;
    opt_t opt;
    struct sigaction sa;

    log_init ("sbig-focus");

    /* Set default option values.
     */
    memset (&opt, 0, sizeof (opt));
    opt.chip = CCD_IMAGING;         /* main imaging ccd */
    opt.readout_mode = RM_3X3;      /* lo resolution */
    opt.t = 1.0;                    /* 1s exposure time */
    opt.verbose = true;
    opt.partial = 1.0;

    optind = 0;
    while ((ch = getopt_long (argc, argv, OPTIONS, longopts, NULL)) != -1) {
        switch (ch) {
            case 'p': /* --partial */
                opt.partial = strtod (optarg, NULL); 
                if (opt.partial <= 0 || opt.partial > 1.0)
                    usage ();
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

    /* Connect to driver
     */
    if (!sbig_udrv)
        msg_exit ("SBIG_UDRV is not set");
    if (!sbig_device)
        msg_exit ("SBIG_DEVICE is not set");
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != CE_NO_ERROR)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));

    sa.sa_handler = &handle_sigint;
    sa.sa_flags = 0;
    sigfillset (&sa.sa_mask);
    if (sigaction (SIGINT, &sa, NULL) < 0)
        err_exit ("sigaction");

    /* Open camera
     */
    if ((e = sbig_open_device (sb, sbig_device)) != CE_NO_ERROR)
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

    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("Device closed");

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

bool exposure_wait (sbig_t *sb, sbig_ccd_t ccd, opt_t opt)
{
    PAR_COMMAND_STATUS status;
    int e;

    usleep (1E6 * opt.t);
    do {
        if ((e = sbig_ccd_get_exposure_status (ccd, &status)) != CE_NO_ERROR)
            msg_exit ("sbig_get_exposure_status: %s", sbig_get_error_string (sb, e));
        if (status != CS_INTEGRATION_COMPLETE)
            usleep (1E3 * 500); /* 500ms */
    } while (status != CS_INTEGRATION_COMPLETE && !interrupted);
    return !interrupted;
}

void preview_ds9 (sbfits_t sbf)
{
    char *cmd;
    int status;

    if (asprintf (&cmd, "xpaset ds9 fits <%s", sbfits_get_filename (sbf))<0)
        oom ();
    if ((status = system (cmd)) < 0)
        err ("preview");
    else if (WIFEXITED (status)) {
        if (WEXITSTATUS (status) != 0)
            msg ("preview: xpaset exited with rc=%d", WEXITSTATUS (status));
    } else if (WIFSIGNALED (status)) {
        msg ("preview: killed by %s", strsignal (WTERMSIG (status)));
    } else if (WIFSTOPPED (status)) {
        msg ("preview: stopped");
    } else if (WIFCONTINUED (status)) {
        msg ("preview: continued");
    }
    free (cmd);
}

bool snap (sbig_t *sb, sbig_ccd_t ccd, opt_t opt)
{
    int e;
    int flags = START_SKIP_VDD; 
    sbfits_t sbf;
    const char *tmpdir = getenv ("TMPDIR");
    if (!tmpdir)
        tmpdir = "/tmp";

    sbf = sbfits_create ();
    if (sbfits_create_file (sbf, tmpdir, "FOCUS") < 0)
        msg_exit ("%s: %s", sbfits_get_filename (sbf), sbfits_get_errstr (sbf));

    if ((e = sbig_ccd_set_shutter_mode (ccd, SC_OPEN_SHUTTER)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_set_shutter_mode: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("exposure (%.0fs)", opt.t);
    if ((e = sbig_ccd_start_exposure (ccd, flags, opt.t)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_start_exposure: %s", sbig_get_error_string (sb, e));
    if (!exposure_wait (sb, ccd, opt))
        goto abort;
    if ((e = sbig_ccd_end_exposure (ccd, 0)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure: %s", sbig_get_error_string (sb, e));
    if (opt.verbose)
        msg ("readout");
    if ((e = sbig_ccd_readout (ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_readout: %s", sbig_get_error_string (sb, e));

    sbfits_set_ccdinfo (sbf, ccd);
    if (sbfits_write_file (sbf) < 0)
        err_exit ("sbfits_write: %s", sbfits_get_errstr (sbf));
    if (sbfits_close_file (sbf))
        err_exit ("sbfits_close: %s", sbfits_get_errstr (sbf));

    msg ("previewing image");
    preview_ds9 (sbf);
    (void)unlink (sbfits_get_filename (sbf));
    sbfits_destroy (sbf);
    return true;
abort:
    (void)sbig_ccd_end_exposure (ccd, ABORT_DONT_END);
    (void)unlink (sbfits_get_filename (sbf));
    sbfits_destroy (sbf);
    return false;
}

void snap_series (sbig_t *sb, opt_t opt)
{
    int e;
    sbig_ccd_t ccd;

    if ((e = sbig_ccd_create (sb, opt.chip, &ccd)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_ccd_end_exposure (ccd, ABORT_DONT_END)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_end_exposure: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_ccd_set_readout_mode (ccd, opt.readout_mode)) != CE_NO_ERROR)
        msg_exit ("sbig_ccd_set_readout_mode: %s", sbig_get_error_string (sb, e));
    if (opt.partial < 1.0) {
        if ((e = sbig_ccd_set_partial_frame (ccd, opt.partial)) != CE_NO_ERROR)
            msg_exit ("sbig_ccd_set_partial_frame: %s", sbig_get_error_string (sb, e));
    }
    msg ("Type ctrl-C to interrupt");
    while (!interrupted) {
        snap (sb, ccd, opt);
    }

    sbig_ccd_destroy (ccd);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
