//========================================================================
// File name  : ksbiglpt.h
// Description: Function prototypes for kernel LPT code.
// Author     : Jan Soldan - Linux
// Author     : Matt Longmire (SBIG) - Windows
// Copyright (C) 2002 - 2004 Jan Soldan, Matt Longmire
// All rights reserved.
//========================================================================
#ifndef _KSBIG_LPT_H_
#define _KSBIG_LPT_H_

#include "ksbigdef.h"
#include "sbigudrv.h"

// Kernel functions prototypes	
int  KLptGetDriverInfo       (GetDriverInfoResults0 *);
int  KLptGetJiffies          (unsigned long         *);
int  KLptGetHz               (unsigned long         *);
int  KSbigLptGetLastError    (unsigned short        *);
int  KLptTestCommand         (void);

void KLptInitPort            (struct private_data *);
int  KLptCameraOutWrapper    (struct private_data *, LinuxCameraOutParams  *);
int  KLptSendMicroBlock      (struct private_data *, LinuxMicroblock       *);
int  KLptGetMicroBlock       (struct private_data *, LinuxMicroblock       *);
int  KLptSetVdd              (struct private_data *, IocSetVdd             *);
int  KLptClearImagingArray   (struct private_data *, IOC_CLEAR_CCD_PARAMS  *);
int  KLptClearTrackingArray  (struct private_data *, IOC_CLEAR_CCD_PARAMS  *);
int  KLptGetPixels           (struct private_data *, LinuxGetPixelsParams  *);
int  KLptGetArea             (struct private_data *, LinuxGetAreaParams    *);
int  KLptRVClockImagingCCD   (struct private_data *, IOC_VCLOCK_CCD_PARAMS *);
int  KLptRVClockTrackingCCD  (struct private_data *, IOC_VCLOCK_CCD_PARAMS *);
int  KLptRVClockST5CCCD      (struct private_data *, IOC_VCLOCK_CCD_PARAMS *);
int  KLptDumpImagingLines    (struct private_data *, IOC_DUMP_LINES_PARAMS *);
int  KLptDumpTrackingLines   (struct private_data *, IOC_DUMP_LINES_PARAMS *);
int  KLptDumpST5CLines       (struct private_data *, IOC_DUMP_LINES_PARAMS *);
int  KLptClockAD             (struct private_data *, short                 *);
void KLptIoDelay             (struct private_data *, short);
void KLptMicroOut            (struct private_data *, unsigned char val);
int  KLptHClear              (struct private_data *, short times);
int  KLptWaitForPLD          (struct private_data *);
int  KLptWaitForAD           (struct private_data *);
void KDisable                (struct private_data *);
void KEnable                 (struct private_data *);
void KLptDisableLptInterrupts(struct private_data *);
void KLptReadyToRx           (struct private_data *);
void KLptForceMicroIdle      (struct private_data *);
int  KLptGetBufferSize       (struct private_data *);
int  KLptVClockImagingCCD    (struct private_data *, CAMERA_TYPE cameraID,	
			                             unsigned char baseClks,	  
			                             short hClears);
void KLptCameraOut           (struct private_data *, unsigned char reg, 
                                                     unsigned char val);
int  KLptSetBufferSize       (struct private_data *, spinlock_t     *, 
                                                     unsigned short *);
unsigned char KLptMicroStat  (struct private_data *);
unsigned char KLptCameraIn   (struct private_data *, unsigned char reg);
unsigned char KLptMicroIn    (struct private_data *, unsigned char ackIt);

long KDevIoctl     	     (struct file *, unsigned int,
                              unsigned long , spinlock_t *);
//========================================================================
#endif  // _KSBIG_LPT_H_

