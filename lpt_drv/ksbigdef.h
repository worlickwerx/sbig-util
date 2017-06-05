//=============================================================================
// File name  : ksbigdef.h                                                   
// Description: The SBIG Linux LPT/USB definition file.	
// Author     : Jan Soldan - Linux  kernel driver development                   
// Copyright (C) 2002 - 2004 Jan Soldan			     
// All rights reserved.							     
//=============================================================================
#ifndef _KSBIGDEF_H_	      
#define _KSBIGDEF_H_

#include "ulptdrv.h"
#include "wintypes.h"
#include "parmicro.h"  
//-----------------------------------------------------------------------------
//#define _USB_CHATTY_
//#define _SBIG_DEBUG_	
//=============================================================================
#define LNIBBLE_TIMEOUT                 (55 * NIBBLE_TIMEOUT)
#define LIDLE_STATE_DELAY               (55 * IDLE_STATE_DELAY)
#define LCONVERSION_DELAY               500
//-----------------------------------------------------------------------------
typedef struct{
 unsigned char  reg;
 unsigned char  value;
}LinuxCameraOutParams;
//-----------------------------------------------------------------------------
typedef struct{
 IOC_GET_AREA_PARAMS  gap;
 unsigned short *dest;
 unsigned long  length;
}LinuxGetAreaParams;
//-----------------------------------------------------------------------------
typedef struct{
 unsigned short portBase;
 unsigned short portSpan;
}LinuxLptPortParams;
//=============================================================================
#endif // _KSBIGDEF_H_
