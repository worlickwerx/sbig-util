#ifndef _SBIG_CAMERA_H
#define _SBIG_CAMERA_H

#include "handle.h"
#include "sbigudrv.h"

typedef struct sbig_ccd_struct *sbig_ccd_t;

/* Call before any camera commands.  Camera type (model) is returned.
 * Somewhat vestigual as far as I can tell.
 */
int sbig_establish_link (sbig_t sb, CAMERA_TYPE *type);

/* Convert CAMERA_TYPE to camera model string
 */
const char *sbig_strcam (CAMERA_TYPE type);

/* Establish a handle for a particular chip:
 *  CCD_IMAGING, CCD_TRACKING, CCD_EXT_TRACKING
 */
int sbig_ccd_create (sbig_t sb, CCD_REQUEST chip, sbig_ccd_t *ccdp);
void sbig_ccd_destroy (sbig_ccd_t ccd);

/* Query ccd info
 * info0: all ccds, info2: imaging only, info4: imaging/tracking
 */
int sbig_ccd_get_info0 (sbig_ccd_t ccd, GetCCDInfoResults0 *info);
int sbig_ccd_get_info2 (sbig_ccd_t ccd, GetCCDInfoResults2 *info);
int sbig_ccd_get_info4 (sbig_ccd_t ccd, GetCCDInfoResults4 *info);

/* Get/set ccd anti-blooming gate state (used on next exposure/readout)
 *  ABG_LOW7, ABG_CLK_LOW7, ABG_CLK_MED7, ABG_CLK_HI7
 */
int sbig_ccd_set_abg (sbig_ccd_t ccd, ABG_STATE7 state);
int sbig_ccd_get_abg (sbig_ccd_t ccd, ABG_STATE7 *statep);

/* Get/set readout binning mode (used on next exposure/readout)
 *  RM_1X1, RM_2X2, RM_3X3, RM_NX1, RM_NX2, RM_NX3,
 *  RM_1X1_VOFFCHIP, RM_2X2_VOFFCHIP, RM_3X3_VOFFCHIP, RM_9X9, RM_MXN.
 */
int sbig_ccd_set_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE mode);
int sbig_ccd_get_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE *modep);

/* Get/set shutter mode (used on next exposure/readout)
 *  SC_LEAVE_SHUTTER, SC_OPEN_SHUTTER, SC_CLOSE_SHUTTER,
 *  SC_INITIALIZE_SHUTTER, SC_OPEN_EXT_SHUTTER, SC_CLOSE_EXT_SHUTTER
 */
int sbig_ccd_set_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND mode);
int sbig_ccd_get_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND *modep);

/* Set top most row to readout (0 based), left most pixel (0, based),
 * and image height and width in binned pixels (used on next exposure/readout)
 */
int sbig_ccd_set_window (sbig_ccd_t ccd,
                         unsigned short top, unsigned short left,
                         unsigned short height, unsigned short width);
int sbig_ccd_get_window (sbig_ccd_t ccd,
                         unsigned short *topp, unsigned short *leftp,
                         unsigned short *heightp, unsigned short *widthp);

int sbig_start_exposure (sbig_ccd_t sb, double exposureTime);

int sbig_end_exposure (sbig_ccd_t sb);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
