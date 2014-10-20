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
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>

#include "handle.h"
#include "handle_impl.h"
#include "sbigudrv.h"
#include "sbig.h"

#include "src/common/libutil/bcd.h"
#include "src/common/libutil/xzmalloc.h"

struct sbig_ccd_struct {
    sbig_t sb;
    CCD_REQUEST ccd;
    ABG_STATE7 abg_mode;
    READOUT_BINNING_MODE readout_mode;
    SHUTTER_COMMAND shutter_mode;
    GetCCDInfoResults0 info;
    unsigned short top, left, height, width;
};

static int lookup_roinfo (sbig_ccd_t ccd, READOUT_BINNING_MODE mode)
{
    int i;
    for (i = 0; i < ccd->info.readoutModes; i++)
        if (ccd->info.readoutInfo[i].mode == mode)
            return i;
    return -1;
}

int sbig_ccd_create (sbig_t sb, CCD_REQUEST chip, sbig_ccd_t *ccdp)
{
    sbig_ccd_t ccd = xzmalloc (sizeof (*ccd));
    ccd->ccd = chip;
    ccd->sb = sb;
    int e = sbig_ccd_get_info0 (ccd, &ccd->info);
    if (e != CE_NO_ERROR) {
        free (ccd);
        return e;
    }
    ccd->abg_mode = ABG_CLK_MED7;
    ccd->shutter_mode = SC_OPEN_SHUTTER; /* open during exp, close during ro */
    ccd->readout_mode = RM_1X1;

    assert (ccd->info.readoutModes > 0);
    ccd->readout_mode = ccd->info.readoutInfo[0].mode;
    ccd->top = 0;
    ccd->left = 0;
    ccd->height = ccd->info.readoutInfo[0].height;
    ccd->width = ccd->info.readoutInfo[0].width;
    
    *ccdp = ccd;
    return CE_NO_ERROR;     
}

void sbig_ccd_destroy (sbig_ccd_t ccd)
{
    free (ccd);
}

int sbig_ccd_get_info0 (sbig_ccd_t ccd, GetCCDInfoResults0 *info)
{
    GetCCDInfoParams in;
    if (ccd->ccd == CCD_IMAGING)
        in.request = CCD_INFO_IMAGING;
    else if (ccd->ccd == CCD_TRACKING)
        in.request = CCD_INFO_TRACKING;
    else
        return CE_BAD_PARAMETER;
    return ccd->sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_ccd_get_info2 (sbig_ccd_t ccd, GetCCDInfoResults2 *info)
{
    GetCCDInfoParams in;
    if (ccd->ccd == CCD_IMAGING)
        in.request = CCD_INFO_EXTENDED;
    else
        return CE_BAD_PARAMETER;
    return ccd->sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_ccd_get_info4 (sbig_ccd_t ccd, GetCCDInfoResults4 *info)
{
    GetCCDInfoParams in;
    if (ccd->ccd == CCD_IMAGING)
        in.request = CCD_INFO_EXTENDED2_IMAGING;
    else if (ccd->ccd == CCD_TRACKING)
        in.request = CCD_INFO_EXTENDED2_TRACKING;
    else
        return CE_BAD_PARAMETER;
    return ccd->sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_ccd_set_abg_mode (sbig_ccd_t ccd, ABG_STATE7 mode)
{
    ccd->abg_mode = mode;
    return CE_NO_ERROR;
}

int sbig_ccd_get_abg_mode (sbig_ccd_t ccd, ABG_STATE7 *modep)
{
    *modep = ccd->abg_mode;
    return CE_NO_ERROR;
}

int sbig_ccd_set_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE mode)
{
    int ro_index = lookup_roinfo (ccd, mode);
    if (ro_index == -1)
        return CE_BAD_PARAMETER;
    ccd->readout_mode = ccd->info.readoutInfo[ro_index].mode;
    ccd->top = 0;
    ccd->left = 0;
    ccd->height = ccd->info.readoutInfo[ro_index].height;
    ccd->width = ccd->info.readoutInfo[ro_index].width;
    return CE_NO_ERROR;
}

int sbig_ccd_get_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE *modep)
{
    *modep = ccd->readout_mode;
    return CE_NO_ERROR;
}

int sbig_ccd_set_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND mode)
{
    ccd->shutter_mode = mode;
    return CE_NO_ERROR;
}

int sbig_ccd_get_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND *modep)
{
    *modep = ccd->shutter_mode;
    return CE_NO_ERROR;
}

int sbig_ccd_set_window (sbig_ccd_t ccd,
                         unsigned short top, unsigned short left,
                         unsigned short height, unsigned short width)
{
    if (top > ccd->height || left > ccd->width || height > ccd->height
                                               || width > ccd->width)
        return CE_BAD_PARAMETER;
    ccd->top = top;
    ccd->left = left;
    ccd->height = height;
    ccd->width = width;
    return CE_NO_ERROR;
}

int sbig_ccd_get_window (sbig_ccd_t ccd,
                         unsigned short *topp, unsigned short *leftp,
                         unsigned short *heightp, unsigned short *widthp)
{
    *topp = ccd->top;
    *leftp = ccd->left;
    *heightp = ccd->height;
    *widthp = ccd->width;
    return CE_NO_ERROR;
}


int sbig_ccd_start_exposure (sbig_ccd_t ccd, double exposureTime)
{
    StartExposureParams2 in = { .ccd = ccd->ccd,
                                .exposureTime = exposureTime * 100.0,
                                .abgState = ccd->abg_mode,
                                .openShutter = ccd->shutter_mode,
                                .readoutMode = ccd->readout_mode,
                                .top = ccd->top, .left = ccd->left,
                                .height = ccd->height, .width = ccd->width };
    if (in.exposureTime < 1 || in.exposureTime > 0x00ffffff)
        return CE_BAD_PARAMETER;
    return ccd->sb->fun (CC_START_EXPOSURE2, &in, NULL);
}

int sbig_ccd_get_exposure_status (sbig_ccd_t ccd, ushort *sp)
{
    ushort status;
    int e = sbig_query_cmd_status (ccd->sb, CC_START_EXPOSURE, &status);
    if (e == CE_NO_ERROR) {
        if (ccd->ccd == CCD_IMAGING)
            *sp = status & 3;
        else
            *sp = (status >> 2) & 3;
    }
    return e;
}

int sbig_ccd_end_exposure (sbig_ccd_t ccd)
{
    EndExposureParams in = { .ccd = ccd->ccd };
    return ccd->sb->fun (CC_END_EXPOSURE, &in, NULL);
}

int sbig_establish_link (sbig_t sb, CAMERA_TYPE *type)
{
    EstablishLinkParams in = { .sbigUseOnly = 0 };
    EstablishLinkResults out;
    int e = sb->fun (CC_ESTABLISH_LINK, &in, &out);
    if (e == CE_NO_ERROR)
        *type = out.cameraType;
    return e;
}


typedef struct {
    CAMERA_TYPE type;
    const char *desc;
} devtab_t;

static devtab_t dtab[] = {
  { ST7_CAMERA, "ST-7" },
  { ST8_CAMERA, "ST-8" },
  { ST5C_CAMERA, "ST-5C" },
  { TCE_CONTROLLER, "TCE Controller" },
  { ST237_CAMERA, "ST-237" },
  { STK_CAMERA, "STK" },
  { ST9_CAMERA, "ST9" },
  { STV_CAMERA, "STV" },
  { ST10_CAMERA, "ST10" },
  { ST1K_CAMERA, "ST1K" },
  { ST2K_CAMERA, "ST2K" },
  { STL_CAMERA, "STL" },
  { ST402_CAMERA, "ST402" },
  { STX_CAMERA, "STX" },
  { ST4K_CAMERA, "ST4K" },
  { STT_CAMERA, "STT" },
  { STI_CAMERA, "STI" },
  { STF_CAMERA, "STF" },
  { NEXT_CAMERA, "NEXT" },
  { NO_CAMERA, "no camera" },
};

const char *sbig_strcam (CAMERA_TYPE type)
{
    int i, max = sizeof (dtab) / sizeof (dtab[0]);
    for (i = 0; i < max; i++)
        if (dtab[i].type == type)
            return dtab[i].desc;
    return "unknown";
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
