#ifndef _SBIG_CAMERA_H
#define _SBIG_CAMERA_H

#include "camera.h"

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
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
