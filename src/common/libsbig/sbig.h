#ifndef _SBIG_H
#define _SBIG_H

typedef struct sbig_struct *sbig_t;

sbig_t sbig_new (void);
int sbig_dlopen (sbig_t sb, const char *path);
int sbig_open_driver (sbig_t sb);
int sbig_open_device (sbig_t sb);
int sbig_close_device (sbig_t sb);
void sbig_destroy (sbig_t sb);

const char *sbig_strerror (int err);
#endif
