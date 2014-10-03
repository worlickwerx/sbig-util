#ifndef _SBIG_CAMERA_H
#define _SBIG_CAMERA_H

#include "handle.h"
#include "sbigudrv.h"

/* Call before any camera commands.
 * Somewhat vestigual as far as I can tell.
 */
int sbig_establish_link (sbig_t sb, CAMERA_TYPE *type);

/* Get ccd info
 * Set request to CCD_INFO_IMAGING or CCD_INFO_TRACKING
 */
int sbig_get_ccd_info (sbig_t sb, CCD_INFO_REQUEST request,
                       GetCCDInfoResults0 *info);

/* Get extended ccd info about imaging ccd.
 */
int sbig_get_ccd_xinfo (sbig_t sb, GetCCDInfoResults2 *info);

/* Get extended ccd info
 * Set request to CCD_INFO_EXTENDED2_IMAGING or CCD_INFO_EXTENDED2_TRACKING.
 */
int sbig_get_ccd_xinfo2 (sbig_t sb, CCD_INFO_REQUEST request,
                         GetCCDInfoResults4 *info);

const char *sbig_strcam (CAMERA_TYPE type);

int sbig_start_exposure2 (sbig_t sb, CCD_REQUEST ccd, double exp_msec,
                          ABG_STATE7 abgState, SHUTTER_COMMAND openShutter,
                          unsigned short readoutMode,
                          unsigned short top, unsigned short left,
                          unsigned short height, unsigned short width);

int sbig_end_exposure (sbig_t sb, CCD_REQUEST ccd);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
