//========================================================================
// File name  : ksbiglptmain.h
// Description: The main header file of the linux LPT kernel driver.
// Copyright (C) 2002 - 2004 Jan Soldan
// All rights reserved.
//========================================================================
#ifndef _KSBIGLPTMAIN_H_
#define _KSBIGLPTMAIN_H_
//========================================================================  
#include "sbigudrv.h"

//#define _CHATTY_

// device name, major & index definition
#define LDEV_NAME			"sbiglpt"	
#define LDEV_MAJOR			0
#define LDEV_MAX_INDEX 		        2

// The buffer may be resized via IOCTL_SET_BUFFER_SIZE call
#define LDEFAULT_BUFFER_SIZE            4096
#define LMAX_DEV_NAME_SIZE		32
#define LMAX_MSG_SIZE			512

// LPT ports definition
#define LPT0_BASE			0x378
#define LPT0_DATA			LPT0_BASE + 0
#define LPT0_STATUS			LPT0_BASE + 1
#define LPT0_CONTROL			LPT0_BASE + 2

#define LPT1_BASE			0x278
#define LPT1_DATA			LPT1_BASE + 0
#define LPT1_STATUS			LPT1_BASE + 1
#define LPT1_CONTROL			LPT1_BASE + 2

#define LPT2_BASE			0x3bc
#define LPT2_DATA			LPT2_BASE + 0
#define LPT2_STATUS			LPT2_BASE + 1
#define LPT2_CONTROL			LPT2_BASE + 2

#define LPT_SPAN      			3
//========================================================================
struct private_data{
 unsigned short  port_base;
 unsigned short  port_span;
 char            dev_name[LMAX_DEV_NAME_SIZE];
 unsigned long   flags;
 unsigned long   value;
 unsigned char   control_out;
 unsigned char   imaging_clocks_out;
 unsigned char   noBytesRd;
 unsigned char   noBytesWr;
 unsigned short  state;
 unsigned short  buffer_size;
 char           *buffer;
};
//========================================================================
// function prototypes
int	KModInit(void);
void	KModExit(void);
//========================================================================
#endif // _KSBIGLPTMAIN_H_












