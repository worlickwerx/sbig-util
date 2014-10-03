#ifndef _SBIG_HANDLE_IMPL_H
#define _SBIG_HANDLE_IMPL_H

struct sbig_struct {
    void *dso;
    short (*fun)(short cmd, void *parm, void *result);
};

#endif
