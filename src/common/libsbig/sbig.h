#ifndef _SBIG_H
#define _SBIG_H

#include "sbigudrv.h"

typedef struct sbig_struct *sbig_t;

typedef struct {
    int mode;
    int width;
    int height;
    double gain;
    double pixw;
    double pixh;
} sbig_readout_mode_t;

typedef struct {
    char version[6];
    char name[64];
    int nmodes;
    sbig_readout_mode_t modes[20];
} sbig_ccd_info_t;

sbig_t sbig_new (void);
int sbig_dlopen (sbig_t sb, const char *path);
int sbig_open_driver (sbig_t sb);
int sbig_open_device (sbig_t sb, SBIG_DEVICE_TYPE type);
int sbig_establish_link (sbig_t sb);

int sbig_close_device (sbig_t sb);
void sbig_destroy (sbig_t sb);

int sbig_get_driver_info (sbig_t sb, DRIVER_REQUEST request,
                          GetDriverInfoResults0 *info);
		
int sbig_get_ccd_info (sbig_t sb, sbig_ccd_info_t *ip);

const char *sbig_strerror (int err);
const char *sbig_strdevice (int dev);
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
