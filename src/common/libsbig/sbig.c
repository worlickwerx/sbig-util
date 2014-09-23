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

#include "sbig.h"

#include "src/common/libutil/bcd.h"

struct sbig_struct {
    void *dso;
    short (*fun)(short cmd, void *parm, void *result);
};

sbig_t sbig_new (void)
{
    sbig_t sb = malloc (sizeof (*sb));
    if (!sb) {
        errno = ENOMEM;
        return NULL;
    }
    memset (sb, 0, sizeof (*sb));
    return sb;
}

int sbig_dlopen (sbig_t sb, const char *path)
{
    dlerror ();
    if (!(sb->dso = dlopen (path, RTLD_LAZY | RTLD_LOCAL)))
        return CE_OS_ERROR;
    if (!(sb->fun = dlsym (sb->dso, "SBIGUnivDrvCommand")))
        return CE_OS_ERROR;
    return CE_NO_ERROR;
}

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

int sbig_open_device (sbig_t sb)
{
    OpenDeviceParams in;
    in.deviceType = DEV_USB1; // FIXME
    return sb->fun (CC_OPEN_DEVICE, &in, NULL);
}

int sbig_close_device (sbig_t sb)
{
    return sb->fun (CC_CLOSE_DEVICE, NULL, NULL);
}

int sbig_establish_link (sbig_t sb)
{
    EstablishLinkParams in;
    EstablishLinkResults out;
    in.sbigUseOnly = 0;
    return sb->fun (CC_ESTABLISH_LINK, &in, &out);
}

int sbig_get_ccd_info (sbig_t sb, sbig_ccd_info_t *ip)
{
    int i, e;
    GetCCDInfoParams in;
    GetCCDInfoResults0 out;

    in.request = CCD_INFO_IMAGING;
    e = sb->fun (CC_GET_CCD_INFO, &in, &out);
    if (e != CE_NO_ERROR)
        goto done;
    bcd4str (out.firmwareVersion, ip->version, sizeof (ip->version));
    assert (sizeof (ip->name) == sizeof (out.name));
    memcpy (ip->name, out.name, sizeof (ip->name));
    ip->nmodes = out.readoutModes;
    for (i = 0; i < ip->nmodes; i++) {
        ip->modes[i].mode = out.readoutInfo[i].mode;
        ip->modes[i].width = out.readoutInfo[i].width;
        ip->modes[i].height = out.readoutInfo[i].height;
        ip->modes[i].gain = bcd2_2 (out.readoutInfo[i].gain);
        ip->modes[i].pixw = bcd6_2 (out.readoutInfo[i].pixelWidth);
        ip->modes[i].pixh = bcd6_2 (out.readoutInfo[i].pixelHeight);
    }
done:
    return e;
}

void sbig_destroy (sbig_t sb)
{
    if (sb->dso)
        dlclose (sb->dso);
    free (sb);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
