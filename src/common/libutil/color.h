#ifndef _UTIL_COLOR_H
#define _UTIL_COLOR_H

#include <sys/types.h>

/* Apply 3x3 kernel described in SBIGUDrv sec 5.2 to convert a raw Bayer
 * mosaic image to monochrome, which expects even rows to start with a
 * blue pixel and alternate blue/green and odd rows to start with a green
 * pixel and alternate green/red.  The 'in' and 'out' parameters point to
 * pre-allocated image buffers of 'height' rows and 'width' columns,
 * in row-major order.
 */
void color_bayer_to_mono (ushort *in, ushort *out, int width, int height);


#endif /* _UTIL_COLOR_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
