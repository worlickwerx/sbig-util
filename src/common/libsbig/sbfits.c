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

/* Write FITS file with SBIG header extensions
 * See http://hesarc.gsfc.nasa.gov/docs/fcg/standard_dict.html
 *     http://www.sbig.com/pdffiles/SBFITSEXT_1r0.pdf"
 */

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
#include <unistd.h>
#include <sys/param.h>
#include <time.h>
#include <fitsio.h>
#include <math.h>

#include "sbig.h"
#include "sbfits.h"

#include "src/common/libutil/xzmalloc.h"
#include "src/common/libutil/bcd.h"

struct sbfits_struct {
    fitsfile *fptr;
    int status;
    char error_string[31];       /* buffer for err str (<=30 chars per docs) */
    time_t t_create;             /* time of file creation */
    time_t t_obs;                /* time of observation */
    char filename[PATH_MAX];     /* full path of output file */
    const char *annotation;      /* (opt) an extra note from teh observer */
    double exposure_time;        /* length of exposure in seconds */
    ushort num_exposures;        /* number of exposures co-added */
    double temperature;          /* temperature of CCD at start of exp */
    const char *object;          /* (opt) name of object being studied */
    const char *telescope;       /* (opt) telescope/lens used */
    const char *filter;          /* (opt) filter used */
    const char *observer;        /* (opt) name of observer */
    ushort *data;                /* image data */
    ushort height, width;        /* size of image data */
    int top, left;               /* subframe origin */
    READOUT_BINNING_MODE readout_mode;
    GetCCDInfoResults0 info0;
    double focal_length;
    double aperture_diameter;
    double aperture_area;
};

static char *gmtime_str (time_t t, char *buf, int sz)
{
    struct tm tm;

    memset (buf, 0, sz);
    if (!gmtime_r (&t, &tm))
        return (NULL);
    strftime (buf, sz, "%FT%T", &tm);
    return buf;
}

sbfits_t sbfits_create (void)
{
    sbfits_t sbf = xzmalloc (sizeof (*sbf));
    sbf->num_exposures = 1;
    return sbf;
}

void sbfits_destroy (sbfits_t sbf)
{
    free (sbf);
}

int sbfits_create_file (sbfits_t sbf, const char *imagedir,
                                           const char *prefix)
{
    char buf[64];
    int n;
    int rc = -1;

    sbf->t_create = time (NULL);
    n = snprintf (sbf->filename, sizeof (sbf->filename),
                  "%s/%s_%s.fits", imagedir, prefix,
                  gmtime_str (sbf->t_create, buf, sizeof (buf)));
    if (n >= sizeof (sbf->filename)) {
        errno = EINVAL;
        goto done;
    }
    (void)unlink (sbf->filename);
    fits_create_file (&sbf->fptr, sbf->filename, &sbf->status);
    if (sbf->status)
        goto done;
    rc = 0;
done:
    return rc;
}

const char *sbfits_get_filename (sbfits_t sbf)
{
    return sbf->filename;
}

const char *sbfits_get_errstr (sbfits_t sbf)
{
    fits_get_errstatus (sbf->status, sbf->error_string);
    return sbf->error_string;
}

int sbfits_close_file (sbfits_t sbf)
{
    int rc = -1;
    fits_close_file (sbf->fptr, &sbf->status);
    if (sbf->status)
        goto done;
    rc = 0;
done:
    return rc;
}

void sbfits_set_ccdinfo (sbfits_t sbf, sbig_ccd_t ccd)
{
    ushort top, left, height, width;
    sbf->t_obs         = sbig_ccd_get_start_time (ccd);
    sbf->exposure_time = sbig_ccd_get_exposure_time (ccd);
    sbf->data          = sbig_ccd_get_data (ccd, &sbf->height, &sbf->width);
    (void)sbig_ccd_get_readout_mode (ccd, &sbf->readout_mode); /* FIXME */
    (void)sbig_ccd_get_info0 (ccd, &sbf->info0); /* FIXME */
    (void)sbig_ccd_get_window (ccd, &top, &left, &height, &width);
    sbf->top = top;   /* need as int */
    sbf->left = left; /* need as int */
}

void sbfits_set_num_exposures (sbfits_t sbf, ushort num_exposures)
{
    sbf->num_exposures = num_exposures;
}

void sbfits_set_observer (sbfits_t sbf, const char *observer)
{
    sbf->observer = observer;
}

void sbfits_set_telescope (sbfits_t sbf, const char *telescope)
{
    sbf->telescope = telescope;
}

void sbfits_set_filter (sbfits_t sbf, const char *filter)
{
    sbf->filter = filter;
}

void sbfits_set_object (sbfits_t sbf, const char *object)
{
    sbf->object = object ;
}

void sbfits_set_temperature (sbfits_t sbf, double temperature)
{
    sbf->temperature = temperature;
}

void sbfits_set_focal_length (sbfits_t sbf, double d)
{
    sbf->focal_length = d;
}

void sbfits_set_aperture_diameter (sbfits_t sbf, double d)
{
    sbf->aperture_diameter = d;
}

void sbfits_set_aperture_area (sbfits_t sbf, double d)
{
    sbf->aperture_area = d;
}


void sbfits_set_annotation (sbfits_t sbf, const char *str)
{
    sbf->annotation = str;
}

static int sbfits_write_image (sbfits_t sbf)
{
    long naxes[2] = { sbf->width, sbf->height };

    fits_create_img (sbf->fptr, USHORT_IMG, 2, naxes, &sbf->status);
    fits_write_img (sbf->fptr, TUSHORT, 1,
                    sbf->height * sbf->width, sbf->data, &sbf->status);

    return sbf->status ? -1 : 0;

}

static int lookup_readoutmode_index (sbfits_t sbf)
{
    int i;
    for (i = 0; i < sbf->info0.readoutModes; i++) {
        if (sbf->info0.readoutInfo[i].mode == sbf->readout_mode)
            return i;
    }
    return -1;
}

static int sbfits_write_header (sbfits_t sbf)
{
    char buf[64];

    fits_write_key (sbf->fptr, TSTRING, "COMMENT", 
                    "SBIG FITS header format per:",
                    "", &sbf->status);
    fits_write_key (sbf->fptr, TSTRING, "COMMENT", 
                    " http://www.sbig.com/pdffiles/SBFITSEXT_1r0.pdf",
                    "", &sbf->status);
    if (sbf->annotation)
        fits_write_key (sbf->fptr, TSTRING, "COMMENT", (char *)sbf->annotation,
                        "", &sbf->status);

    fits_write_key(sbf->fptr, TSTRING, "DATE",
                   gmtime_str (sbf->t_create, buf, sizeof (buf)),
                   "GMT date when this file created", &sbf->status);
    fits_write_key(sbf->fptr, TSTRING, "DATE-OBS",
                   gmtime_str (sbf->t_obs, buf, sizeof (buf)),
                   "GMT start of exposure", &sbf->status);

    fits_write_key (sbf->fptr, TDOUBLE, "EXPTIME", &sbf->exposure_time,
                    "Exposure in seconds", &sbf->status);
    fits_write_key (sbf->fptr, TDOUBLE, "CCD-TEMP", &sbf->temperature,
                    "CCD temp in degress C", &sbf->status);

    if (sbf->object)
        fits_write_key(sbf->fptr, TSTRING, "OBJECT", (char *)sbf->object,
                       "", &sbf->status);
    if (sbf->telescope)
        fits_write_key(sbf->fptr, TSTRING, "TELESCOP", (char *)sbf->telescope,
                       "", &sbf->status);
    if (sbf->filter)
        fits_write_key(sbf->fptr, TSTRING, "FILTER", (char *)sbf->filter, 
                       "Optical filter name", &sbf->status);
    if (sbf->observer)
        fits_write_key(sbf->fptr, TSTRING, "OBSERVER", (char *)sbf->observer,
                       "", &sbf->status);

    fits_write_key(sbf->fptr, TSTRING, "INSTRUME", sbf->info0.name,
                   "Camera Model", &sbf->status);

    int rm_index = lookup_readoutmode_index (sbf);
    if (rm_index != -1) {
        ushort xbin, ybin;
        switch (sbf->info0.readoutInfo[rm_index].mode) {
            case RM_1X1:
                xbin = ybin = 1;
                break;
            case RM_2X2:
                xbin = ybin = 2;
                break;
            case RM_3X3:
                xbin = ybin = 3;
                break;
            case RM_NX1:
            case RM_NX2:
            case RM_NX3:
            case RM_1X1_VOFFCHIP:
            case RM_2X2_VOFFCHIP:
            case RM_3X3_VOFFCHIP:
            case RM_9X9:
            case RM_NXN:
                break; /* Not supported yet, and not allowed by sbig-snap */
        }
        fits_write_key(sbf->fptr, TUSHORT, "XBINNING", &xbin,
                       "Horizontal binning factor", &sbf->status);
        fits_write_key(sbf->fptr, TUSHORT, "YBINNING", &ybin,
                       "Vertical binning factor", &sbf->status);

        double pixw = bcd6_2 (sbf->info0.readoutInfo[rm_index].pixelWidth);
        double pixh = bcd6_2 (sbf->info0.readoutInfo[rm_index].pixelHeight);
        fits_write_key(sbf->fptr, TDOUBLE, "XPIXSZ", &pixw,
                       "Pixel width in microns", &sbf->status);
        fits_write_key(sbf->fptr, TDOUBLE, "YPIXSZ", &pixh,
                       "Pixel height in microns", &sbf->status);

        double gain = bcd2_2 (sbf->info0.readoutInfo[rm_index].gain);
        fits_write_key(sbf->fptr, TDOUBLE, "EGAIN", &gain,
                       "Electrons per ADU", &sbf->status);

    }
    fits_write_key(sbf->fptr, TINT, "XORGSUBF", &sbf->left,
                   "Subframe origin x_pos", &sbf->status);
    fits_write_key(sbf->fptr, TINT, "YORGSUBF", &sbf->top,
                   "Subframe origin y_pos", &sbf->status);
    fits_write_key(sbf->fptr, TUSHORT, "RESMODE", &sbf->readout_mode,
                    "Resolution mode", &sbf->status);
    fits_write_key(sbf->fptr, TSTRING, "SBSTDVER", "SBFITSEXT Version 1.0",
                    "SBIG FITS extensions ver", &sbf->status);
    fits_write_key(sbf->fptr, TUSHORT, "SNAPSHOT", &sbf->num_exposures,
                    "Number images coadded", &sbf->status);

    fits_write_key(sbf->fptr, TDOUBLE, "FOCALLEN", &sbf->focal_length,
                   "Focal length in mm", &sbf->status);
    fits_write_key(sbf->fptr, TDOUBLE, "APTDIA", &sbf->aperture_diameter,
                   "Aperture diameter in mm", &sbf->status);
    fits_write_key(sbf->fptr, TDOUBLE, "APTAREA", &sbf->aperture_area,
                   "Aperture area in sq-mm", &sbf->status);

    return sbf->status ? -1 : 0;
}

    /* FIXME More header values to write!!
     */
#if 0
    fits_write_key(sbf->fptr, TLONG,   "CBLACK", &m_lBackground,
                   "BLACK ADU FOR DISPLAY", &sbf->status);
    fits_write_key(sbf->fptr, TLONG,   "CWHITE", &fitsWhite,
                    "WHITE ADU FOR DISPLAY", &sbf->status);
    fits_write_key(sbf->fptr, TLONG,   "PEDESTAL", &fitsPedestal,
                    "ADD TO ADU FOR 0-BASE", &sbf->status);
    fits_write_key(sbf->fptr, TUSHORT, "DATAMAX", &m_uSaturationLevel,
                    "SATURATION LEVEL", &sbf->status);
    fits_write_key(sbf->fptr, TSTRING, "SWACQIR", "?",
                    "DATA ACQ SOFTWARE", &sbf->status);
    fits_write_key(sbf->fptr, TSTRING, "SWCREAT", "?", "", &sbf->status);
#endif

int sbfits_write_file (sbfits_t sbf)
{
    if (sbfits_write_image (sbf) < 0)
        return -1;
    if (sbfits_write_header (sbf) < 0)
        return -1;
    return 0;
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */