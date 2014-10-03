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

#include "src/common/libutil/bcd.h"

int sbig_establish_link (sbig_t sb, CAMERA_TYPE *type)
{
    EstablishLinkParams in = { .sbigUseOnly = 0 };
    EstablishLinkResults out;
    int e = sb->fun (CC_ESTABLISH_LINK, &in, &out);
    if (e == CE_NO_ERROR)
        *type = out.cameraType;
    return e;
}

int sbig_get_ccd_info (sbig_t sb, CCD_INFO_REQUEST request,
                       GetCCDInfoResults0 *info)
{
    GetCCDInfoParams in = { .request = request };
    if (request != CCD_INFO_TRACKING && request != CCD_INFO_IMAGING)
        return CE_BAD_PARAMETER;
    return sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_get_ccd_xinfo (sbig_t sb, GetCCDInfoResults2 *info)
{
    GetCCDInfoParams in = { .request = CCD_INFO_EXTENDED };
    return sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_get_ccd_xinfo2 (sbig_t sb, CCD_INFO_REQUEST request,
                         GetCCDInfoResults4 *info)
{
    GetCCDInfoParams in = { .request = request };
    if (request != CCD_INFO_EXTENDED2_TRACKING
                        && request != CCD_INFO_EXTENDED2_IMAGING)
        return CE_BAD_PARAMETER;
    return sb->fun (CC_GET_CCD_INFO, &in, info);
}

int sbig_start_exposure2 (sbig_t sb, CCD_REQUEST ccd,
                          unsigned long exposureTime, ABG_STATE7 abgState,
                          SHUTTER_COMMAND openShutter,
                          unsigned short readoutMode,
                          unsigned short top, unsigned short left,
                          unsigned short height, unsigned short width)
{
    StartExposureParams2 in = { .ccd = ccd,
                                .exposureTime = exposureTime,
                                .abgState = abgState,
                                .openShutter = openShutter,
                                .readoutMode = readoutMode,
                                .top = top, .left = left,
                                .height = height, .width = width };
    return sb->fun (CC_START_EXPOSURE2, &in, NULL);
}

int sbig_end_exposure (sbig_t sb, CCD_REQUEST ccd)
{
    EndExposureParams in = { .ccd = ccd };
    return sb->fun (CC_END_EXPOSURE, &in, NULL);
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
