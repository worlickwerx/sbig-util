/*****************************************************************************\
 *  Copyright (c) 2017 Jim Garlick All rights reserved.
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

#include "color.h"

static void addcell (ushort *frame, int width, int height,
                    int row, int col, int weight,
                    int *val, int *count)
{
    if (row >= 0 && row < height && col >= 0 && col < width) {
        *val += frame[(row * width) + col] * weight;
        *count += weight;
    }
}

void color_bayer_to_mono (ushort *in, ushort *out, int width, int height)
{
    int row, col;

    for (row = 0; row < height;  row++) {
        for (col = 0; col < width; col++) {
            int count = 0;
            int val = 0;

            addcell (in, width, height, row - 1, col - 1, 1, &val, &count);
            addcell (in, width, height, row - 1, col + 0, 1, &val, &count);
            addcell (in, width, height, row - 1, col + 1, 1, &val, &count);

            addcell (in, width, height, row + 0, col - 1, 2, &val, &count);
            addcell (in, width, height, row + 0, col + 0, 4, &val, &count);
            addcell (in, width, height, row + 0, col + 1, 2, &val, &count);

            addcell (in, width, height, row + 1, col - 1, 1, &val, &count);
            addcell (in, width, height, row + 1, col + 0, 1, &val, &count);
            addcell (in, width, height, row + 1, col + 1, 1, &val, &count);

            val /= count;
            if (val < 0 || val > 65535)
                val = 65535;
            out[(row * width) + col] = (ushort)val;
         }
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
