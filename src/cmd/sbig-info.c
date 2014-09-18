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
#include <libgen.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "src/common/libsbig/sbig.h"
#include "src/common/libutil/log.h"

char *prog;

int main(int argc, char *argv[])
{
    sbig_t sb;
    const char *sbig_udrv = getenv ("SBIG_UDRV");
    int e;

    log_init ("sbig-info");

    if (!sbig_udrv)
        msg_exit ("SBIG_UDRV is not set");
    if (!(sb = sbig_new ()))
        err_exit ("sbig_new");
    if (sbig_dlopen (sb, sbig_udrv) != 0)
        msg_exit ("%s", dlerror ());
    if ((e = sbig_open_driver (sb)) != 0)
        msg_exit ("sbig_open_driver: %s", sbig_strerror (e));
    if ((e = sbig_open_device (sb)) != 0)
        msg_exit ("sbig_open_device: %s", sbig_strerror (e));

    printf ("Hello\n");

    if ((e = sbig_close_device (sb)) != 0)
        msg_exit ("sbig_close_device: %s", sbig_strerror (e));

    sbig_destroy (sb);
    log_fini ();
    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
