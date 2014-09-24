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

#include "sbig.h"

typedef struct {
    CFW_MODEL_SELECT type;
    const char *desc;
} devtab_t;

static devtab_t dtab[] = {
  { CFWSEL_UNKNOWN, "unknown" },
  { CFWSEL_CFW2, "CFW-2" },
  { CFWSEL_CFW5, "CFW-5" },
  { CFWSEL_CFW8, "CFW-8" },
  { CFWSEL_CFWL, "CFW-L" },
  { CFWSEL_CFW402, "CFW-402" },
  { CFWSEL_AUTO, "auto" },
  { CFWSEL_CFW6A, "CFW-6A" },
  { CFWSEL_CFW10, "CFW-10" },
  { CFWSEL_CFW10_SERIAL, "CFW-10 (serial)" },
  { CFWSEL_CFW9, "CFW-9" },
  { CFWSEL_CFWL8, "CFW-L8" },
  { CFWSEL_CFWL8G, "CFW-L8G" },
  { CFWSEL_CFW1603, "CFW-1603" },
  { CFWSEL_FW5_STX, "FW-5-STX" },
  { CFWSEL_FW5_8300, "FW-5-8300" },
  { CFWSEL_FW8_8300, "FW-8-8300" },
  { CFWSEL_FW7_STX, "FW-7-STX" },
  { CFWSEL_FW8_STT, "FW-8-STT" },
};

const char *sbig_strcfw (CFW_MODEL_SELECT type)
{
    int i, max = sizeof (dtab) / sizeof (dtab[0]);
    for (i = 0; i < max; i++)
        if (dtab[i].type == type)
            return dtab[i].desc;
    return "unknown";
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
