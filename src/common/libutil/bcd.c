/*****************************************************************************\
 *  Copyright (c) 2014 Lawrence Livermore National Security, LLC.  Produced at
 *  the Lawrence Livermore National Laboratory (cf, AUTHORS, DISCLAIMER.LLNS).
 *  LLNL-CODE-658032 All rights reserved.
 *
 *  This file is part of the Flux resource manager framework.
 *  For details, see https://github.com/flux-framework.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license, or (at your option)
 *  any later version.
 *
 *  Flux is distributed in the hope that it will be useful, but WITHOUT
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

#include "bcd.h"

double bcd2_2 (ushort i)
{
    return (1E-2 * ((i) & 0xf))
          + 1E-1 * ((i >> 4) & 0xf)
          + 1E0 * ((i >> 8) & 0xf)
          + 1E1 * ((i >> 12) & 0xf);
}

double bcd6_2 (ulong i)
{
    return (1E-2 * ((i) & 0xf))
          + 1E-1 * ((i >> 4) & 0xf)
          + 1E0 * ((i >> 8) & 0xf)
          + 1E1 * ((i >> 12) & 0xf)
          + 1E2 * ((i >> 16) & 0xf)
          + 1E3 * ((i >> 20) & 0xf)
          + 1E4 * ((i >> 24) & 0xf)
          + 1E5 * ((i >> 28) & 0xf);
}

void bcd4str (ushort i, char *buf, int len)
{
    snprintf (buf, len, "%2.2f", bcd2_2 (i));
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
