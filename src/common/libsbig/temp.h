#ifndef _SBIG_TEMP_H
#define _SBIG_TEMP_H

/* Enable/disable CCD temperature regulation.
 * 'reg': REGULATION_OFF, REGULATION_ON, REGULATION_OVERRIDE,
 *        REGULATION_FREEZE, REGULATION_UNFREEZE,
 *        REGULATION__ENABLE_AUTOFREEZE, _DISABLE_AUTOFREEZE.
 * ccdSetpoint is in degrees Celcius
 */
int sbig_temp_set (sbig_t sb, TEMPERATURE_REGULATION reg, double ccdSetpoint);

/* Query temperature status
 */
int sbig_temp_get_info (sbig_t sb, QueryTemperatureStatusResults2 *info);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
