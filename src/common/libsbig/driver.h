#ifndef _SBIG_DRIVER_H
#define _SBIG_DRIVER_H

#include "handle.h"
#include "sbigudrv.h"

/* Initialize/finalize driver
 * Ref SBIGUDrv sec 3.1.1, 3.1.2
 */
int sbig_open_driver (sbig_t sb);
int sbig_close_driver (sbig_t sb);

/* Initialize/finalize low level transport (N.B. only tested on USB)
 * USB: DEV_USB picks first camera, DEV_USB1...DEV_USB8 picks specific one.
 * LPT: DEV_LPT1...DEV_LPT3 chooses parallel port
 * IP: user-provided IPv4 address
 * Ref SBIGUDrv sec 3.1.3, 3.1.4
 */
int sbig_open_device (sbig_t sb, SBIG_DEVICE_TYPE type);
int sbig_open_device_ip (sbig_t sb, ulong ipaddr);
int sbig_close_device (sbig_t sb);

/* Get info about driver code
 * Ref SBIGUDrv sec 3.1.5
 */
int sbig_get_driver_info (sbig_t sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info);

/* Get/set driver handle
 * Ref SBIGUdrv sec 3.1.6
 */
int sbig_get_driver_handle (); // unimplemented
int sbig_set_driver_handle (); // unimplemented

/* Get command status
 * Ref SBIGUdrv sec 3.5.4
 */
int sbig_query_cmd_status (sbig_t sb, ushort cmd, ushort *outp);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
