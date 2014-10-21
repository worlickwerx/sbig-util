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
 * Internally, create calls get_info0.
 */
int sbig_ccd_create (sbig_t sb, CCD_REQUEST chip, sbig_ccd_t *ccdp);
void sbig_ccd_destroy (sbig_ccd_t ccd);

/* Query ccd info
 * info0: all ccds, info2: imaging only, info4: imaging/tracking
 * Ref SBIGUDrv sec 3.5.2
 */
int sbig_ccd_get_info0 (sbig_ccd_t ccd, GetCCDInfoResults0 *info);
int sbig_ccd_get_info2 (sbig_ccd_t ccd, GetCCDInfoResults2 *info);
int sbig_ccd_get_info4 (sbig_ccd_t ccd, GetCCDInfoResults4 *info);

/* Get/set ccd anti-blooming gate state (used on next exposure/readout)
 *  ABG_LOW7, ABG_CLK_LOW7, ABG_CLK_MED7, ABG_CLK_HI7
 * Only affects TC211 tracking CCD on ST-7/8/etc and imaging CCD on PixCel255.
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
 * Note: reset to full size internally when readout mode is selected.
 */
int sbig_ccd_set_window (sbig_ccd_t ccd,
                         unsigned short top, unsigned short left,
                         unsigned short height, unsigned short width);
int sbig_ccd_get_window (sbig_ccd_t ccd,
                         unsigned short *topp, unsigned short *leftp,
                         unsigned short *heightp, unsigned short *widthp);


/* Exposure control: start exposure, poll for status, then end exposure
 *  Status: CS_IDLE, CS_IN_PROGRESS, CS_INTEGRATING, CS_INTEGRATION_COMPLETE
 * Ref SBIGUDrv sec 3.2.1, 3.2.2
 */
int sbig_ccd_start_exposure (sbig_ccd_t ccd, double exposureTime);
int sbig_ccd_get_exposure_status (sbig_ccd_t ccd, ushort *sp);
int sbig_ccd_end_exposure (sbig_ccd_t ccd);

/* Readout to internal buffer (start, iterate reading lines, stop).
 * Ref SBIGUDrv sec 3.2.3, 3.2.4, 3.2.5
 */
int sbig_ccd_readout (sbig_ccd_t ccd);

/* Copy from internal buffer as a sequence of rows.
 * Buffer len must == height*width (use get_window above)
 */
int sbig_ccd_writemem (sbig_ccd_t ccd, unsigned short *buf, int len);

/* Copy out buffer to a PGM format file
 * Ref: netpbm.sourceforge.net/doc/pgm.html
 */
int sbig_ccd_writepgm (sbig_ccd_t ccd, const char *filename);

/* Readout control
 */

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
