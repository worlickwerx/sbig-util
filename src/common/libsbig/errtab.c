/*****************************************************************************\
 *  Copyright (c) 2014 Jim Garlick All rights reserved.
 *
 *  This file is part of the sbig-util.
 *  For details, see https://github.com/garlick/sbig-util.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 3 of the license, or (at your option)
 *  any later version.
 *
 *  sbig-util is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>

#define ENV_LINUX   7
#define TARGET      ENV_LINUX
#include "sbigudrv.h"

#include "sbig.h"

typedef struct {
    int err;
    const char *msg;
} errtab_t;

static errtab_t etab[] = {
  { CE_NO_ERROR, "no error" },
  { CE_CAMERA_NOT_FOUND, "camera not found" },
  { CE_EXPOSURE_IN_PROGRESS, "exposure in progress" },
  { CE_NO_EXPOSURE_IN_PROGRESS, "no exposure in progress" },
  { CE_UNKNOWN_COMMAND, "unknown command" },
  { CE_BAD_CAMERA_COMMAND, "bad camera command" },
  { CE_BAD_PARAMETER, "bad parameter" },
  { CE_TX_TIMEOUT, "tx timeout" },
  { CE_RX_TIMEOUT, "rx timeout" },
  { CE_NAK_RECEIVED, "nak received" },
  { CE_CAN_RECEIVED, "can received" },
  { CE_UNKNOWN_RESPONSE, "unknown response" },
  { CE_BAD_LENGTH, "bad length" },
  { CE_AD_TIMEOUT, "ad timeout" },
  { CE_KBD_ESC, "kbd esc" },
  { CE_CHECKSUM_ERROR, "checksum error" },
  { CE_EEPROM_ERROR, "eeprom error" },
  { CE_SHUTTER_ERROR, "shutter error" },
  { CE_UNKNOWN_CAMERA, "unknown camera" },
  { CE_DRIVER_NOT_FOUND, "driver not found" },
  { CE_DRIVER_NOT_OPEN, "driver not open" },
  { CE_DRIVER_NOT_CLOSED, "driver not closed" },
  { CE_SHARE_ERROR, "share error" },
  { CE_TCE_NOT_FOUND, "tce not found" },
  { CE_AO_ERROR, "ao error" },
  { CE_ECP_ERROR, "ecp error" },
  { CE_MEMORY_ERROR, "memory error" },
  { CE_DEVICE_NOT_FOUND, "device not found" },
  { CE_DEVICE_NOT_OPEN, "device not open" },
  { CE_DEVICE_NOT_CLOSED, "device not closed" },
  { CE_DEVICE_NOT_IMPLEMENTED, "device not implemented" },
  { CE_DEVICE_DISABLED, "device disabled" },
  { CE_OS_ERROR, "os error" },
  { CE_SOCK_ERROR, "sock error" },
  { CE_SERVER_NOT_FOUND, "server not found" },
  { CE_CFW_ERROR, "cfw error" },
  { CE_MF_ERROR, "mf error" },
  { CE_FIRMWARE_ERROR, "firmware error" },
  { CE_DIFF_GUIDER_ERROR, "diff guider error" },
  { CE_RIPPLE_CORRECTION_ERROR, "ripple correction error" },
  { CE_EZUSB_RESET, "ezusb reset" },
  { CE_NEXT_ERROR, "next error" },
  { -1, NULL },
};

const char *sbig_strerror (int err)
{
    int i, max = sizeof (etab) / sizeof (etab[0]);
    static char unknown[] = "unknown error XXXXXXXX";
    for (i = 0; i < max; i++)
        if (etab[i].err == err)
            return etab[i].msg;
    snprintf (unknown + 14, sizeof (unknown) - 14, "%d", err);
    return unknown;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
