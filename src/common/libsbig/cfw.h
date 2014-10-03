#ifndef _SBIG_CFW_H
#define _SBIG_CFW_H

#include "handle.h"
#include "sbigudrv.h"

int sbig_cfw_get_info (sbig_t sb, CFW_MODEL_SELECT *model,
                       ulong *fwrev, ulong *numpos);

int sbig_cfw_goto (sbig_t sb, CFW_POSITION position);
int sbig_cfw_query (sbig_t sb, CFW_STATUS *status, CFW_POSITION *position);

const char *sbig_strcfw (CFW_MODEL_SELECT type);
#endif

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
