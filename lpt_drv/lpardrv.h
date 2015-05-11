/*

lpardrv.h - Local options for pardrv.h

*/
#ifndef _LPARDRV_
#define _LPARDRV_

#define ENV_WIN			1				/* Target for Windows environment */
#define ENV_WINVXD		2				/* SBIG Use Only, Win 9X VXD */
#define ENV_WINSYS		3				/* SBIG Use Only, Win NT SYS */
#define ENV_ESRVJK		4				/* SBIG Use Only, Ethernet Remote */
#define ENV_ESRVWIN		5				/* SBIG Use Only, Ethernet Remote */
#define ENV_MACOSX		6				/* SBIG Use Only, Mac OSX */
#define ENV_LINUX		7				/* SBIG Use Only, Linux */
#ifndef TARGET
 #define TARGET     ENV_LINUX			/* Set for your target */
#endif

#include "sbigudrv.h"

#endif
