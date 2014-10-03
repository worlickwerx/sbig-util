#ifndef _SBIG_HANDLE_H
#define _SBIG_HANDLE_H

typedef struct sbig_struct *sbig_t;

sbig_t sbig_new (void);
int sbig_dlopen (sbig_t sb, const char *path);
void sbig_destroy (sbig_t sb);

const char *sbig_get_error_string (sbig_t sb, unsigned short errorNo);
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */