#ifndef _UTIL_BCD_H
#define _UTIL_BCD_H

#include <sys/types.h>

void bcd4str (ushort i, char *buf, int len);
double bcd2_2 (ushort i);
double bcd6_2 (ulong i);

#endif /* !_UTIL_BCD_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
