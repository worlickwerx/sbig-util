#ifndef _SBIG_DRIVER_H
#define _SBIG_DRIVER_H

#include "handle.h"
#include "sbigudrv.h"

/* Initialize/finalize driver
 * Ref SBIGUDrv sec 3.1.1, 3.1.2
 */
int sbig_open_driver (sbig_t *sb);
int sbig_close_driver (sbig_t *sb);

/* Initialize/finalize low level transport (N.B. only tested on USB)
 * usb: "USB" picks first camera, "USB1"..."USB8" picks specific one.
 * parallel: "LPT1"..."LPT3" chooses parallel port
 * ethernet: "a.b.c.d" is parsed as the camera's IPv4 address.
 * Ref SBIGUDrv sec 3.1.3, 3.1.4
 */
int sbig_open_device (sbig_t *sb, const char *name);
int sbig_close_device (sbig_t *sb);

/* Get info about driver code
 * Ref SBIGUDrv sec 3.1.5
 */
int sbig_get_driver_info (sbig_t *sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info);

/* Get/set driver handle
 * Ref SBIGUdrv sec 3.1.6
 */
int sbig_get_driver_handle (); // unimplemented
int sbig_set_driver_handle (); // unimplemented

/* Get command status
 * Ref SBIGUdrv sec 3.5.4
 */
int sbig_query_cmd_status (sbig_t *sb, ushort cmd, ushort *outp);

/* Convert string to device type and vv
 */
const char *sbig_strdev (SBIG_DEVICE_TYPE type);
SBIG_DEVICE_TYPE sbig_devstr (const char *str);

/* Query USB
 * Ref SBIGUdrv sec 3.5.16
 */
int sbig_query_usb (sbig_t *sb, QueryUSBResults *results);

/* Query ethernet
 * Ref SBIGUdrv sec 3.5.17
 */
int sbig_query_ethernet (sbig_t *sb, QueryEthernetResults *results);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
