#ifndef _SBIG_CAMERA_H
#define _SBIG_CAMERA_H

#include <time.h>

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
 * Default: ABG_LOW7.
 */
int sbig_ccd_set_abg (sbig_ccd_t ccd, ABG_STATE7 state);
int sbig_ccd_get_abg (sbig_ccd_t ccd, ABG_STATE7 *statep);

/* Get/set readout binning mode (used on next exposure/readout)
 *  RM_1X1, RM_2X2, RM_3X3, RM_NX1, RM_NX2, RM_NX3,
 *  RM_1X1_VOFFCHIP, RM_2X2_VOFFCHIP, RM_3X3_VOFFCHIP, RM_9X9, RM_MXN.
 * Default: RM_1X1
 */
int sbig_ccd_set_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE mode);
int sbig_ccd_get_readout_mode (sbig_ccd_t ccd, READOUT_BINNING_MODE *modep);

/* Get/set shutter mode (used on next exposure/readout)
 *  SC_LEAVE_SHUTTER, SC_OPEN_SHUTTER, SC_CLOSE_SHUTTER,
 *  SC_INITIALIZE_SHUTTER, SC_OPEN_EXT_SHUTTER, SC_CLOSE_EXT_SHUTTER
 * Default: SC_OPEN_SHUTTER
 */
int sbig_ccd_set_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND mode);
int sbig_ccd_get_shutter_mode (sbig_ccd_t ccd, SHUTTER_COMMAND *modep);

/* Set top most row to readout (0 based), left most pixel (0, based),
 * and image height and width in binned pixels (used on next exposure/readout)
 * Note: reset to full size internally when readout mode is selected.
 * Default: full size for readout mode
 */
int sbig_ccd_set_window (sbig_ccd_t ccd, ushort top, ushort left,
                         ushort height, ushort width);
int sbig_ccd_get_window (sbig_ccd_t ccd, ushort *topp, ushort *leftp,
                         ushort *heightp, ushort *widthp);

/* Set window to a centered fraction of the area of the full frame.
 * (0 < part <= 1.0)
 */
int sbig_ccd_set_partial_frame (sbig_ccd_t ccd, double part);

/* Set/clear exposure flags:
 *  EXP_FAST_READOUT, EXP_DUAL_CHANNEL_MODE, EXP_WAIT_FOR_TRIGGER_IN,
 *  EXP_LIGHT_CLEAR, EXP_RIPPLE CORRECTION (for STF-8050/4070 only)
 * Ref SBIGUdrv sec 3.2.1
 */
int sbig_ccd_set_exposure_flags (sbig_ccd_t ccd, ulong flags);
int sbig_ccd_clr_exposure_flags (sbig_ccd_t ccd, ulong flags);

/* Exposure control: start exposure, poll for status, then end exposure
 *  Status: CS_IDLE, CS_IN_PROGRESS, CS_INTEGRATING, CS_INTEGRATION_COMPLETE
 * Flags for start_exposure: START_SKIP_VDD, START_MOTOR_ALWAYS_ON.
 * Flags for end_exposure: ABORT_DONT_END, END_SKIP_DELAY
 * Ref SBIGUDrv sec 3.2.1, 3.2.2
 */
int sbig_ccd_start_exposure (sbig_ccd_t ccd, ushort flags, double exposureTime);
int sbig_ccd_get_exposure_status (sbig_ccd_t ccd, PAR_COMMAND_STATUS *sp);
int sbig_ccd_end_exposure (sbig_ccd_t ccd, ushort flags);

/* Readout to internal buffer (start, iterate reading lines, stop).
 * Ref SBIGUDrv sec 3.2.3, 3.2.4, 3.2.5
 */
int sbig_ccd_readout (sbig_ccd_t ccd);
int sbig_ccd_readout_subtract (sbig_ccd_t ccd);

/* Get reference to internal buffer, a sequence of rows, pixels.
 */
ushort *sbig_ccd_get_data (sbig_ccd_t ccd, ushort *height, ushort *width);

/* Get the system time recorded when start_exposure was called.
 */
time_t sbig_ccd_get_start_time (sbig_ccd_t ccd);

/* Get the exposure time in seconds recorded when start_exposure was called.
 */
double sbig_ccd_get_exposure_time (sbig_ccd_t ccd);

/* Copy from internal buffer to a PGM file
 * Ref: netpbm.sourceforge.net/doc/pgm.html
 */
int sbig_ccd_writepgm (sbig_ccd_t ccd, const char *filename);

int sbig_ccd_get_max (sbig_ccd_t ccd, ushort *max);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
