#ifndef _SBIG_H
#define _SBIG_H

#include "sbigudrv.h"

typedef struct sbig_struct *sbig_t;

sbig_t sbig_new (void);
int sbig_dlopen (sbig_t sb, const char *path);
void sbig_destroy (sbig_t sb);

int sbig_open_driver (sbig_t sb);
int sbig_get_driver_info (sbig_t sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info);

int sbig_open_device (sbig_t sb, SBIG_DEVICE_TYPE type);
int sbig_establish_link (sbig_t sb, CAMERA_TYPE *type);
int sbig_close_device (sbig_t sb);


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

int sbig_cfw_get_info (sbig_t sb, CFW_MODEL_SELECT *model,
                       ulong *fwrev, ulong *numpos);

const char *sbig_strerror (int err);
const char *sbig_strcam (CAMERA_TYPE type);
const char *sbig_strcfw (CFW_MODEL_SELECT type);
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
