#ifndef _SBIG_DRIVER_H
#define _SBIG_DRIVER_H

#include "handle.h"
#include "sbigudrv.h"

int sbig_open_driver (sbig_t sb);
int sbig_close_driver (sbig_t sb);

int sbig_get_driver_info (sbig_t sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info);

int sbig_open_device (sbig_t sb, SBIG_DEVICE_TYPE type);
int sbig_close_device (sbig_t sb);

#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
