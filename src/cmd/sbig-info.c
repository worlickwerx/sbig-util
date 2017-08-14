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
#include "src/common/libutil/xzmalloc.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/bcd.h"
#include "src/common/libini/ini.h"

struct options {
    double focal_length;
};

void show_cfw_info (const char *sbig_udrv, const char *sbig_device,
                    int ac, char **av);
void show_driver_info (const char *sbig_udrv, int ac, char **av);
void show_ccd_info (const char *sbig_udrv, const char *sbig_device,
                    int ac, char **av);
void show_cooler_info (const char *sbig_udrv, const char *sbig_device,
                       int ac, char **av);
void show_fov (const char *sbig_udrv, const char *sbig_device,
               const struct options *opt, int ac, char **av);
int config_cb (void *user, const char *section, const char *name,
               const char *value);

#define OPTIONS "h"
static const struct option longopts[] = {
    {"help",          no_argument,           0, 'h'},
    {0, 0, 0, 0},
};

void usage (void)
{
    fprintf (stderr,
"Usage: sbig-info driver\n"
"       sbig-info ccd [tracking|imaging]\n"
"       sbig-info cfw\n"
"       sbig-info cooler\n"
"       sbig-info fov {tracking|imaging} {lo|med|hi} [focal-length]\n"
);
    exit (1);
}

int main (int argc, char *argv[])
{
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    const char *sbig_device = getenv ("SBIG_DEVICE");
    const char *config_filename = getenv ("SBIG_CONFIG_FILE");
    int ch;
    char *cmd;
    struct options *opt;

    log_init ("sbig-info");

    opt = xzmalloc (sizeof (*opt));

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
    if (!sbig_device)
        msg_exit ("SBIG_DEVICE is not set");
    if (!config_filename)
        msg_exit ("SBIG_CONFIG_FILE is not set");
    (void)ini_parse (config_filename, config_cb, opt);

    if (!strcmp (cmd, "ccd")) {
        show_ccd_info (sbig_udrv, sbig_device, argc - optind, argv + optind);
    } else if (!strcmp (cmd, "driver")) {
        show_driver_info (sbig_udrv, argc - optind, argv + optind);
    } else if (!strcmp (cmd, "cfw")) {
        show_cfw_info (sbig_udrv, sbig_device, argc - optind, argv + optind);
    } else if (!strcmp (cmd, "cooler")) {
        show_cooler_info(sbig_udrv, sbig_device, argc - optind, argv + optind);;
    } else if (!strcmp (cmd, "fov")) {
        show_fov (sbig_udrv, sbig_device, opt, argc - optind, argv + optind);
    } else
        usage ();

    log_fini ();
    free (opt);
    return 0;
}

int config_cb (void *user, const char *section, const char *name,
               const char *value)
{
    struct options *opt = user;
    if (!strcmp (section, "config")) {
        if (!strcmp (name, "focal_length"))
            opt->focal_length = strtod (value, NULL);
    }
    return 0;
}

sbig_t *init_driver (const char *sbig_udrv)
{
    int e;
    sbig_t *sb;

    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != 0)
        msg_exit ("sbig_open_driver: %s", sbig_get_error_string (sb, e));
    return sb;
}

void fini_driver (sbig_t *sb)
{
    sbig_destroy (sb);
}

void init_device (sbig_t *sb, const char *sbig_device)
{
    CAMERA_TYPE type;
    int e;

    if ((e = sbig_open_device (sb, sbig_device) != 0))
        msg_exit ("sbig_open_device: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_establish_link (sb, &type)) != 0)
        msg_exit ("sbig_establish_link: %s", sbig_get_error_string (sb, e));
}

void fini_device (sbig_t *sb)
{
    int e;
    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_get_error_string (sb, e));
}

static int lookup_readoutmode_index (GetCCDInfoResults0 info,
                                     READOUT_BINNING_MODE mode)
{
    int i;
    for (i = 0; i < info.readoutModes; i++) {
        if (info.readoutInfo[i].mode == mode)
            return i;
    }
    return -1;
}

void show_fov (const char *sbig_udrv, const char *sbig_device,
               const struct options *opt, int ac, char **av)
{
    sbig_t *sb;
    double focal_length;                /* telescope FL in mm */
    double pixel_width, pixel_height;   /* pixel dims in um */
    double sensor_width, sensor_height; /* sensor dims in mm */
    double fov_height, fov_width;       /* field of view in arcmin */
    double vres, hres;                  /* pixel resolution in arcsec/pixel */
    int e, rm_index;
    sbig_ccd_t *ccd;
    CCD_REQUEST chip;
    READOUT_BINNING_MODE readout_mode;
    GetCCDInfoResults0 info;

    if (ac != 2 && ac != 3)
        usage ();
    if (!strcmp (av[0], "tracking"))
        chip = CCD_TRACKING;
    else if (!strcmp (av[0], "imaging"))
        chip = CCD_IMAGING;
    else
        usage ();
    if (!strcmp (av[1], "lo"))
        readout_mode = RM_3X3;
    else if (!strcmp (av[1], "med"))
        readout_mode = RM_2X2;
    else if (!strcmp (av[1], "hi"))
        readout_mode = RM_1X1;
    else
        usage ();

    if (ac == 3)
        focal_length = strtod (av[2], NULL);
    else {
        if (opt->focal_length == 0)
            msg_exit("Please set focal_length");
        focal_length = opt->focal_length;
    }
    printf ("focal length: %.2fmm\n", focal_length);

    sb = init_driver (sbig_udrv);
    init_device (sb, sbig_device);

    if ((e = sbig_ccd_create (sb, chip, &ccd)))
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));
    if ((e = sbig_ccd_get_info0 (ccd, &info)) != 0)
        msg_exit ("sbig_ccd_get_info: %s", sbig_get_error_string (sb, e));

    rm_index = lookup_readoutmode_index (info, readout_mode);
    if (rm_index < 0)
        msg_exit ("could not look up readout mode!");

    pixel_height = bcd6_2 (info.readoutInfo[rm_index].pixelHeight);
    pixel_width = bcd6_2 (info.readoutInfo[rm_index].pixelWidth);
    printf ("pixel size: %.1fum H x %.1fum W\n", pixel_height, pixel_width);

    sensor_height = pixel_height * 1E-3 * info.readoutInfo[rm_index].height;
    sensor_width = pixel_width * 1E-3 * info.readoutInfo[rm_index].width;
    printf ("sensor size: %.2fmm H x %.2fmm W\n", sensor_height, sensor_width);

    fov_height = (sensor_height * 3478.0 / focal_length);
    fov_width = (sensor_width * 3478.0 / focal_length);
    printf ("field of view: %.2f'H x %.2f'W\n", fov_height, fov_width);
    if (fov_height > 60)
        printf ("  or: %.2f x %.2f degrees\n", fov_height/60, fov_width/60);

    vres = 206.265 * pixel_height / focal_length;
    hres = 206.265 * pixel_width / focal_length;
    printf ("resolution: %.2f''H x %.2f''W per pixel\n", vres, hres);

    sbig_ccd_destroy (ccd);
    fini_device (sb);
    fini_driver (sb);
}

void show_cooler_info (const char *sbig_udrv, const char *sbig_device,
                       int ac, char **av)
{
    sbig_t *sb;
    QueryTemperatureStatusResults2 info;
    int e;

    sb = init_driver (sbig_udrv);
    init_device (sb, sbig_device);

    if ((e = sbig_temp_get_info (sb, &info)) != CE_NO_ERROR)
        msg_exit ("sbig_temp_get_info: %s", sbig_get_error_string (sb, e));
    printf ("cooling:              %s\n", info.coolingEnabled ? "enabled"
                                                         : "disabled");
    printf ("imaging ccd setpoint: %.2fC\n", info.ccdSetpoint);
    printf ("imaging ccd:          %.2fC\n", info.imagingCCDTemperature);
    printf ("tracking setpoint:    %.2fC\n", info.trackingCCDSetpoint);
    printf ("tracking ccd:         %.2fC\n", info.trackingCCDTemperature);
    //printf ("ext-track ccd:     %.2fC\n", info.externalTrackingCCDTemperature);
    printf ("ambient:              %.2fC\n", info.ambientTemperature);
    printf ("heatsink:             %.2fC\n", info.heatsinkTemperature);

    printf ("imaging pwr:          %.0f%%\n", info.imagingCCDPower);
    printf ("tracking pwr:         %.0f%%\n", info.trackingCCDPower);
    //printf ("ext-track pwr:        %.0f percent\n", info.externalTrackingCCDPower);

    printf ("fan:                  %s\n", info.fanEnabled == FS_OFF ? "off"
                                   : info.fanEnabled == FS_ON ? "manual"
                                   : "auto");
    printf ("fan pwr:              %.0f%%\n", info.fanPower);
    printf ("fan speed:            %.0fRPM\n", info.fanSpeed);

    fini_device (sb);
    fini_driver (sb);
}

void show_driver_info (const char *sbig_udrv, int ac, char **av)
{
    GetDriverInfoResults0 info;
    int e;
    char version[16];
    sbig_t *sb;

    if (ac != 0)
        msg_exit ("driver takes no arguments");

    sb = init_driver (sbig_udrv);

    if ((e = sbig_get_driver_info (sb, DRIVER_STD, &info)) != 0)
        msg_exit ("sbig_get_driver_info: %s", sbig_get_error_string (sb, e));
    bcd4str (info.version, version, sizeof (version));
    printf ("version: %s\n", version);
    printf ("name:    %s\n", info.name);
    printf ("maxreq:  %d\n", info.maxRequest);

    fini_driver (sb);
}

void show_cfw_info (const char *sbig_udrv, const char *sbig_device,
                    int ac, char **av)
{
    int e;
    ulong fwrev, numpos;
    CFW_MODEL_SELECT model;
    sbig_t *sb;
    char version[16];

    if (ac != 0)
        msg_exit ("cfw takes no arguments");

    sb = init_driver (sbig_udrv);
    init_device (sb, sbig_device);

    if ((e = sbig_cfw_get_info (sb, &model, &fwrev, &numpos)) != 0)
        msg_exit ("sbig_cfw_get_info: %s", sbig_get_error_string (sb, e));

    printf ("model:            %s\n", sbig_strcfw (model));
    bcd4str (fwrev, version, sizeof (version));
    printf ("firmware-version: %s\n", version);
    printf ("num-positions:    %lu\n", numpos);

    fini_device (sb);
    fini_driver (sb);
}

/* All cameras support GetCCDInfoResults0 (imaging or tracking)
*/
static void show_ccd_info0 (sbig_t *sb, sbig_ccd_t *ccd)
{
    GetCCDInfoResults0 info0;
    int i, e;
    char version[16];
    const char *camera_type;

    e = sbig_ccd_get_info0 (ccd, &info0);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_get_info: %s", sbig_get_error_string (sb, e));

    bcd4str (info0.firmwareVersion, version, sizeof (version));
    printf ("firmware-version:   %s\n", version);

    camera_type = sbig_strcam (info0.cameraType);
    if (info0.cameraType == STX_CAMERA) { /* fixup STXL - reports as STX */
        GetCCDInfoResults6 info6;
        if (sbig_ccd_get_info6 (ccd, &info6) == CE_NO_ERROR) {
            if (info6.cameraBits & 1)
                camera_type = "STXL";
        }
    }
    printf ("camera-type:        %s\n", camera_type);
    printf ("name:               %s\n", info0.name);

    for (i = 0; i < info0.readoutModes; i++) {
        char pfx[16];
        snprintf (pfx, sizeof (pfx), "mode-%d:", info0.readoutInfo[i].mode);
        printf ("%-18s %4d x %-4d %2.2f e-/ADU %3.2f x %-3.2f microns\n",
             pfx,
             info0.readoutInfo[i].width,
             info0.readoutInfo[i].height,
             bcd2_2 (info0.readoutInfo[i].gain),
             bcd6_2 (info0.readoutInfo[i].pixelWidth),
             bcd6_2 (info0.readoutInfo[i].pixelHeight));
    }
}

/* All cameras but ST5C and ST237 support GetCCDInfoResults2,
 * imaging chip only.
 */
static void show_ccd_info2 (sbig_t *sb, sbig_ccd_t *ccd)
{
    GetCCDInfoResults2 info2;
    int e;

    e = sbig_ccd_get_info2 (ccd, &info2);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info2:  %s", sbig_get_error_string (sb, e));
    }
    printf ("bad-columns:        %d\n", info2.badColumns);
    printf ("ABG:                %s\n", info2.imagingABG == ABG_PRESENT
                                  ? "yes" : "no");
    printf ("serial-number:      %s\n", info2.serialNumber);
}

/* Only ST5C and ST237 cameras support GetCCDInfoResults3.
 * If tracking chip was selected, that would have failed earlier.
 */
static void show_ccd_info3 (sbig_t *sb, sbig_ccd_t *ccd)
{
    GetCCDInfoResults3 info3;
    int e;

    e = sbig_ccd_get_info3 (ccd, &info3);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info3: %s", sbig_get_error_string (sb, e));
    }
    printf ("A/D-bits:           %s\n",
                                  info3.adSize == AD_UNKNOWN ? "unknown" :
                                  info3.adSize == AD_12_BITS ? "12" :
                                  info3.adSize == AD_16_BITS ? "16" :
                                  "invalid");
    printf ("filter-type:        %s\n",
                                  info3.filterType == FW_UNKNOWN ? "unknown" :
                                  info3.filterType == FW_EXTERNAL ? "external" :
                                  info3.filterType == FW_VANE ? "2 position" :
                                  info3.filterType == FW_FILTER_WHEEL
                                                  ? "5 position" : "invalid");
}

/* All cameras support GetCCDInfoResults4 (imaging or tracking)
 */
static void show_ccd_info4 (sbig_t *sb, sbig_ccd_t *ccd)
{
    GetCCDInfoResults4 info4;
    int e;
    ushort cap;

    e = sbig_ccd_get_info4 (ccd, &info4);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_get_ccd_xinfo4: %s", sbig_get_error_string (sb, e));
    cap = info4.capabilitiesBits;
    printf ("ccd-type:           %s\n", (cap & CB_CCD_TYPE_FRAME_TRANSFER)
                                  ? "frame_transfer" : "full frame");
    printf ("electronic-shutter: %s\n", !(cap & CB_CCD_ESHUTTER_YES) ? "no"
         : "Interline Imaging CCD with electronic shutter and ms exposure");
    printf ("remote-guide-port:  %s\n", (cap & CB_CCD_EXT_TRACKER_YES)
                                  ? "yes" : "no");
    printf ("biorad-tdi-mode:    %s\n", (cap & CB_CCD_BTDI_YES) ? "yes" : "no");
    printf ("AO8-detected:       %s\n", (cap & CB_AO8_YES) ? "yes" : "no");
    printf ("frame-buffer:       %s\n", (cap & CB_FRAME_BUFFER_YES)
                                  ? "yes" : "no");
    printf ("use-startexp2:      %s\n", (cap & CB_REQUIRES_STARTEXP2_YES)
                                  ? "yes" : "no");
}

/* All cameras support GetCCDInfoREsults6, imaging only.
*/
static void show_ccd_info6 (sbig_t *sb, sbig_ccd_t *ccd)
{
    GetCCDInfoResults6 info6;
    int e;

    e = sbig_ccd_get_info6 (ccd, &info6);
    if (e != CE_NO_ERROR) {
        if (e == CE_BAD_PARAMETER)
            return;
        msg_exit ("sbig_ccd_get_info6: %s", sbig_get_error_string (sb, e));
    }
    printf ("color-type:         %s\n",
                                 !(info6.ccdBits & 1) ? "mono" :
                                 !(info6.ccdBits & 2) ? "bayer" : "truesense");
}

void show_ccd_info (const char *sbig_udrv, const char *sbig_device,
                    int ac, char **av)
{
    int e;
    CCD_REQUEST chip;
    sbig_ccd_t *ccd;
    sbig_t *sb;

    if (ac == 0)
        chip = CCD_IMAGING;
    else if (ac == 1) {
        if (!strcmp (av[0], "tracking"))
            chip = CCD_TRACKING;
        else if (!strcmp (av[0], "imaging"))
            chip = CCD_IMAGING;
        else
            usage ();
    }
    else
        usage ();

    sb = init_driver (sbig_udrv);
    init_device (sb, sbig_device);

    e = sbig_ccd_create (sb, chip, &ccd);
    if (e != CE_NO_ERROR)
        msg_exit ("sbig_ccd_create: %s", sbig_get_error_string (sb, e));

    show_ccd_info0 (sb, ccd);
    show_ccd_info2 (sb, ccd);
    show_ccd_info3 (sb, ccd);
    show_ccd_info4 (sb, ccd);
    show_ccd_info6 (sb, ccd);

    sbig_ccd_destroy (ccd);
    fini_device (sb);
    fini_driver (sb);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
