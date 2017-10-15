#ifndef _UTIL_XZMALLOC_H
#define _UTIL_XZMALLOC_H

#include <stdarg.h>

/* Memory allocation functions that call oom() on allocation error.
 */
void *xzmalloc (size_t size);
char *xstrdup (const char *s);
char *xasprintf (const char *fmt, ...);
char *xvasprintf(const char *fmt, va_list ap);

#endif /* !_UTIL_XZMALLOC_H */
/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
