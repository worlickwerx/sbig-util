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
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>

#include "handle.h"
#include "handle_impl.h"
#include "sbigudrv.h"
#include "driver.h"

#include "src/common/libutil/bcd.h"

int sbig_open_driver (sbig_t sb)
{
    return sb->fun (CC_OPEN_DRIVER, NULL, NULL); 
}

int sbig_get_driver_info (sbig_t sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info)
{
    GetDriverInfoParams in = { .request = request };
    return sb->fun (CC_GET_DRIVER_INFO, &in, info);
}

int sbig_open_device (sbig_t sb, SBIG_DEVICE_TYPE type)
{
    OpenDeviceParams in = { .deviceType = type };
    return sb->fun (CC_OPEN_DEVICE, &in, NULL);
}

int sbig_close_device (sbig_t sb)
{
    return sb->fun (CC_CLOSE_DEVICE, NULL, NULL);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */