/*

	sbigudrg.h - Contains prototypes for global functions

*/

#ifndef _SBIGUDRG_
#define _SBIGUDRG_

#ifndef _SBIGUDRV_
#include "sbigudrv.h"
#endif

extern PAR_ERROR PulseOut( PulseOutParams *Parameters );
extern PAR_ERROR QueryCommandStatus( QueryCommandStatusParams *Parameters,
	QueryCommandStatusResults *Results);

#endif

