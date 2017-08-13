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
#include <sys/types.h>
#include <stdio.h>

#include "handle.h"
#include "handle_impl.h"
#include "sbigudrv.h"
#include "sbig.h"

int sbig_temp_set (sbig_t *sb, TEMPERATURE_REGULATION reg, double ccdSetpoint)
{
    SetTemperatureRegulationParams2 in = { .regulation = reg,
                                           .ccdSetpoint = ccdSetpoint };
    
    return sb->fun (CC_SET_TEMPERATURE_REGULATION2, &in, NULL); 
}

int sbig_temp_get_info (sbig_t *sb, QueryTemperatureStatusResults2 *info)
{
    QueryTemperatureStatusParams in = { .request = TEMP_STATUS_ADVANCED2};
    return sb->fun (CC_QUERY_TEMPERATURE_STATUS, &in, info); 
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
