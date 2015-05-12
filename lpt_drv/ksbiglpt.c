//========================================================================
// File name  : ksbiglpt.c
// Description: Contains the kernel LPT code.
// Author     : Jan Soldan - Linux
// Author     : Matt Longmire - Windows
// Copyright (C) 2002 - 2004 Jan Soldan, Matt Longmire, SBIG			     
// All rights reserved.		
//========================================================================
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/tty.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "ulptdrv.h"
#include "sbigudrv.h"
#include "ksbiglptmain.h"
#include "ksbiglpthw.h"
#include "ksbiglpt.h"
#include "paropts.h" 

// This was optimized to remove 2 outportb() calls.
// Assumes AD3 is addressed coming into it and leaves
// with AD0 address going out.

 #define K_LPT_READ_AD16(pd, u)                      \
 /*	outb(AD3_MDI, pd->port_base); */             \
 u = (unsigned short)(inb(pd->port_base  + 1) & 0x78) << 9;  \
 outb(AD2, pd->port_base);                           \
 u += (unsigned short)(inb(pd->port_base + 1) & 0x78) << 5;  \
 outb(AD1, pd->port_base);                           \
 u += (unsigned short)(inb(pd->port_base + 1) & 0x78) << 1;  \
 outb(AD0, pd->port_base);                           \
 u += (unsigned short)(inb(pd->port_base + 1) & 0x78) >> 3
//-----------------------------------------------------------------------------
// Static variables:
static unsigned long  gLptHz;  
static unsigned short gLastError;
//========================================================================
// KLptInitPort
// Disable interrupts and force the LPT com to idle.
//========================================================================
void KLptInitPort(struct private_data *pd)
{
 KLptForceMicroIdle(pd);
 KLptDisableLptInterrupts(pd);
}
//========================================================================
// KLptCameraOutWraper
// Write data to one of the Camera Registers.
//========================================================================
int KLptCameraOutWrapper(struct private_data  *pd,
                         LinuxCameraOutParams *arg)
{
 int status;
 LinuxCameraOutParams cop;

 // copy IOC_CAMERA_OUT_PARAMS structure from the user space
 status = copy_from_user(&cop,(LinuxCameraOutParams *)arg,
                               sizeof(LinuxCameraOutParams)); 					
 if(status != 0){ 					
    printk("<0>KLptCameraOutWrapper() : copy_from_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 KLptCameraOut(pd, cop.reg, cop.value);
 return(CE_NO_ERROR);
}
//========================================================================
// KLptCameraOut
// Write data to one of the Camera Registers.
//========================================================================
void KLptCameraOut(struct private_data *pd,
                   unsigned char        reg,
                   unsigned char        val)
{
 unsigned port = pd->port_base;

 outb((unsigned char)(reg + val), port);
 outb((unsigned char)(reg + val + 0x80), port);
 outb((unsigned char)(reg + val + 0x80), port);
 outb((unsigned char)(reg + val), port);

 if(reg == CONTROL_OUT){
    pd->control_out = val;
 }else if(reg == IMAGING_CLOCKS){
    pd->imaging_clocks_out = val;
 }
}
//========================================================================
// KLptSendMicroBlock
// Send a block of data to the micro.
//========================================================================
int KLptSendMicroBlock(struct private_data  *pd,
                       LinuxMicroblock      *arg)
{
 int              status;
 short            i, nibbleLen;
 unsigned char    *p = pd->buffer;
 unsigned long    t0, delay, nibbleTimeout;
 LinuxMicroblock  lmb;

 // Set nibbleTimeout to 300 ms.
 nibbleTimeout = 0.3 * gLptHz;

 // copy LinuxMicroblock structure from the user space
 status = copy_from_user(&lmb, arg, sizeof(LinuxMicroblock)); 					
 if(status != 0){ 					
    printk("<0>KLptSendMicroBlock() : copy_from_user : lmb error.\n");     
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 status = copy_from_user(p, lmb.pBuffer, lmb.length);
 if(status != 0){ 					
    printk("<0>KLptSendMicroBlock() : copy_from_user : lmb.pData error.\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 // caller passes bytes, we need nibbles
 nibbleLen = (short)(lmb.length << 1);
 t0 = jiffies;
 KLptCameraOut(pd, CONTROL_OUT, MICRO_SYNC);

 for(i=0; i <= nibbleLen; ){
     if(KLptMicroStat(pd)){
        if(i == nibbleLen)
           break;
        if(i == 1)
           KLptCameraOut(pd,
                         CONTROL_OUT,
                         (unsigned char)(pd->control_out & ~MICRO_SYNC));
        if((i % 2) == 0)
           KLptMicroOut(pd, (unsigned char)((*p >> 4) & 0x0f));
        else
           KLptMicroOut(pd, (unsigned char)(*p++ & 0x0f));
        i++;
        t0 = jiffies;
     }else{
        // compute time delay in milliseconds
	delay = jiffies - t0;
        if(delay > nibbleTimeout){
           status = gLastError = CE_TX_TIMEOUT;
           KLptCameraOut(pd, CONTROL_OUT, 0);
           break;
	}
     }
 }
 pd->noBytesWr = (i+1)/2;
 pd->state = i;

 return(status);
}
//========================================================================
// KLptGetMicroBlock
// Get a block of bytes (nibbles) from the camera on the parallel port.
//========================================================================
int KLptGetMicroBlock(struct private_data *pd,
                      LinuxMicroblock     *arg)
{
 int              status;
 short            state, rx_len, cmp_len, packet_len = 0;
 unsigned char    *kbuf = pd->buffer, c;
 unsigned long    t0, delay, nibbleTimeout;
 LinuxMicroblock  lmb;

 // Set nibbleTimeout to 300 ms.
 nibbleTimeout = 0.3 * gLptHz;

 // copy LinuxMicroblock structure from the user space
 status = copy_from_user(&lmb, arg, sizeof(LinuxMicroblock)); 					
 if(status != 0){ 					
    printk("<0>KLptGetMicroBlock() : copy_from_user : lmb error.\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 state   = rx_len = 0;
 cmp_len = (short)(2 * lmb.length);
 t0      = jiffies;
 KLptReadyToRx(pd);
	
 do{
    if(KLptMicroStat(pd)){
       switch(state){
         case 0:           // waiting for start of packet
              rx_len = 1;  // 1
              c = KLptMicroIn(pd, (unsigned char)(rx_len < cmp_len));
              *kbuf = c << 4;
              state++;
              if(!(c == 0xA      || c == (CAN >> 4) ||
                 c == (NAK >> 4) || c == (ACK >> 4))){
       	         status = CE_UNKNOWN_RESPONSE;
              }
              break;
         case 1:         // waiting for start of packet
              rx_len++;  // 2
              c = KLptMicroIn(pd, (unsigned char)(rx_len < cmp_len));
              c += *kbuf;
              *kbuf++ = c;
              state++;
              if(c == ACK)
    	         break;
	      else if(c == CAN)
	         status = CE_CAN_RECEIVED;
	      else if(c == NAK)
	         status = CE_NAK_RECEIVED;
              else if(c != 0xA5)
	         status = CE_UNKNOWN_RESPONSE;
              break;
         case 2:         // waiting for command
              rx_len++;  // 3
              c = KLptMicroIn(pd, (unsigned char)(rx_len < cmp_len));
              *kbuf = c << 4;
              state++;
              break;
         case 3:         // waiting for len
              rx_len++;  // 4
	      c = KLptMicroIn(pd, (unsigned char)(rx_len < cmp_len));
	      *kbuf++ += c;
              packet_len = c << 1;
              if((lmb.length << 1) != (unsigned long)(packet_len + 4))
	         status = CE_BAD_LENGTH;
              state++;
              break;
         case 4:         // receiving data
              rx_len++;  // 5,6,7...
              c = KLptMicroIn(
                  pd,
                  (unsigned char)(rx_len < packet_len + 4 && rx_len < cmp_len));
              if((rx_len % 2) == 1)
	         *kbuf = c << 4;
	      else
	         *kbuf++ += c;
	      if(rx_len == packet_len + 4)
	         state++;
	      break;
       } // switch state
       t0 = jiffies;
    }else{
       // compute time delay in milliseconds
       delay = jiffies - t0; 
       if(delay > nibbleTimeout) status = CE_RX_TIMEOUT;
    }
 }while((state < 5) && (status == CE_NO_ERROR) && (rx_len < cmp_len));

 pd->noBytesRd = (unsigned long)(rx_len/2);
 pd->state = state;

 if(status != CE_NO_ERROR) gLastError = status;

 status = copy_to_user(lmb.pBuffer, pd->buffer, lmb.length);
 if(status != 0){ 					
    printk("<0>KLptGetMicroBlock() : copy_to_user : lmb.pData : error.\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 return(status);
}
//========================================================================
// KLptSetVdd
//========================================================================
int  KLptSetVdd(struct private_data *pd, IocSetVdd *arg)
{
 IocSetVdd svdd;

 // copy IocSetVdd structure from the user space
 if(copy_from_user(&svdd, (IocSetVdd *)arg, sizeof(IocSetVdd)) != 0){ 					
    printk("<0>KLptSetVdd() : copy_from_user : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 // raise or lower the Vdd
 svdd.vddWasLow = ((pd->imaging_clocks_out & TRG_H) == TRG_H);
 KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(svdd.raiseIt ? 0 : TRG_H));

 if(copy_to_user((IocSetVdd *)arg, &svdd, sizeof(IocSetVdd)) != 0){ 					
    printk("<0>KLptSetVdd() : copy_to_user : svdd : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 return(CE_NO_ERROR);
}
//========================================================================
// KDisable
// Disable interrupts.
//========================================================================
void KDisable(struct private_data *pd)
{
 //__save_flags(pd->flags);
 // we should disable only certain types of interrupts,
 // stopping all we freeze the system including system clock
 //__cli();
}
//========================================================================
// KEnable
// Enable interrupts.
//========================================================================
void KEnable(struct private_data *pd)
{
 //__restore_flags(pd->flags);
}
//========================================================================
// DisableLPTInterrupts
// Make sure the LPT port can not generate interrupts by disabling them
// at the card.
//========================================================================
void KLptDisableLptInterrupts(struct private_data *pd)
{
 outb(0x0e, pd->port_base + 2);
}
//========================================================================
// KLptIoDelay
// Delay a passed number of IO instructions.
//========================================================================
void KLptIoDelay(struct private_data *pd, short i)
{
 unsigned port_base = pd->port_base;

 for(; i > 0; i--){
     inb(port_base);
 }
}
//========================================================================
// KLptReadyToRx
// Indicate to micro that we're ready to receive data.
//========================================================================
void KLptReadyToRx(struct private_data *pd)
{
 KLptMicroOut(pd, 0);	// let micro know we're ready to rx
}
//========================================================================
// KLptForceMicroIdle
// Reset the handshake line to the microcontroller to the idel state
// and delay long enough to ensure the systems are in sync.
//========================================================================
void KLptForceMicroIdle(struct private_data *pd)
{
 // all clocks low
 KLptCameraOut(pd, CONTROL_OUT, 0);
 mdelay(LIDLE_STATE_DELAY);
}
//========================================================================
// KLptMicroStat
// Return TRUE if the next nibble can be sent or received at the
// microcontroller.
//========================================================================
unsigned char KLptMicroStat(struct private_data *pd)
{
 unsigned char val;

 KLptCameraOut(pd, 
               CONTROL_OUT, 
               (unsigned char)(pd->control_out | MICRO_SELECT));
 val = KLptCameraIn(pd, AD3_MDI);
 if(pd->control_out & HSO) return(val & HSI);
 return((~val) & HSI);
}
//========================================================================
// KLptMicroIn
// Read the data from the microcontroller and toggle the HSO line.
// This assumes KLptMicroStat has been called and returned TRUE.
//========================================================================
unsigned char KLptMicroIn(struct private_data *pd, unsigned char ackIt)
{
 unsigned char val;

 KLptCameraOut(pd, 
               CONTROL_OUT, 
	       (unsigned char)(pd->control_out | MICRO_SELECT));
 val = KLptCameraIn(pd, AD3_MDI) & 0x0F;
 if(ackIt){
    KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(pd->control_out ^ HSO));
 }
 return(val);
}
//========================================================================
// KLptMicroOut
// Send data to the microcontroller and toggle the HSO line.
// This assumes MicroStat has been called and returned TRUE.
//========================================================================
void KLptMicroOut(struct private_data *pd, unsigned char val)
{
 KLptCameraOut(pd, MICRO_OUT, val);
 KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(pd->control_out ^ HSO));
}
//========================================================================
// KLptWaitForPLD
// Wait till the PLD signals complete.
//========================================================================
int KLptWaitForPLD(struct private_data *pd)
{
 short t0 = 0;

 while(1){
   if(!(KLptCameraIn(pd, AD0) & CIP)) break;
   if(t0++ >= LCONVERSION_DELAY) return(gLastError = CE_AD_TIMEOUT);
 }
 return(CE_NO_ERROR);
}
//========================================================================
// KLptWaitForAD
// Wait till the AD signals complete.
//========================================================================
int KLptWaitForAD(struct private_data *pd)
{
 unsigned  port_base = pd->port_base;
 short     t0 = 0;

 outb(AD0, port_base);
 while(1){
   if(!(inb(port_base + 1) & 0x80)) break;
   if(t0++ >= LCONVERSION_DELAY) return(gLastError = CE_AD_TIMEOUT);
 }
 return(CE_NO_ERROR);
}
//========================================================================
// KLptHClear
// Clear block of 10 pixels the number of times passed.
//========================================================================
int KLptHClear(struct private_data *pd, short times)
{
 int status;

 for( ;times > 0; times--){
     // shift CLEAR_BLOCK horizontally
     KLptCameraOut(pd, CONTROL_OUT, IMAGING_SELECT + AD_TRIGGER);
     KLptCameraOut(pd, CONTROL_OUT, IMAGING_SELECT);

     // wait for PLD
     if((status = KLptWaitForPLD(pd)) != CE_NO_ERROR){
        return(gLastError = status);
     }
 }
 return(CE_NO_ERROR);
}
//========================================================================
// VClockImaginCCD
// Clock the Imaging CCD vertically one time.
//========================================================================
int KLptVClockImagingCCD(struct private_data *pd,
                         CAMERA_TYPE          cameraID,
                         unsigned char        baseClks,
		         short                hClears)
{
 int status = CE_NO_ERROR;
 unsigned char v1_h, v2_h;
	
 short vclock_delay = (cameraID == ST1K_CAMERA ?
                      ST1K_VCLOCK_X * VCLOCK_DELAY : VCLOCK_DELAY);

 if(cameraID == ST10_CAMERA){
    v1_h = V2_H; v2_h = V1_H;
 }else{
    v1_h = V1_H; v2_h = V2_H;
 }

 if(cameraID == ST1K_CAMERA && hClears != 0){
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v1_h));  // V1 high
    if((status = KLptHClear(pd, hClears)) != CE_NO_ERROR) return(gLastError = status);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v2_h));  // V2 high
    if((status = KLptHClear(pd, hClears)) != CE_NO_ERROR) return(gLastError = status);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v1_h));  // V1 high
    if((status = KLptHClear(pd,hClears)) != CE_NO_ERROR ) return(gLastError = status);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks));         // all low
    if((status = KLptHClear(pd, hClears)) != CE_NO_ERROR) return(gLastError = status);
 }else{
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v1_h));  // V1 high
    KLptIoDelay(pd, vclock_delay);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v2_h));  // V2 high
    KLptIoDelay(pd, vclock_delay);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks | v1_h));  // V1 high
    KLptIoDelay(pd, vclock_delay);
    KLptCameraOut(pd, IMAGING_CLOCKS, (unsigned char)(baseClks));         // all low
    KLptIoDelay(pd, vclock_delay);
 }
 return(status);
}
//========================================================================
// KLptBlockClearPixels
// Clear the passed number of pixels in the imaging CCD. Do this in
// groups of CLEAR_BLOCK with the readout PAL then do the remainder.
//========================================================================
int KLptBlockClearPixels(struct private_data *pd,
                         CAMERA_TYPE         cameraID,
                         CCD_REQUEST         ccd,
                         short               len,
                         short               readoutMode)
{
 int   status = CE_NO_ERROR;
 short j, bulk, individual;
 unsigned char ccd_select;

 ccd_select = (ccd == CCD_IMAGING ? IMAGING_SELECT : TRACKING_SELECT);

 // clear pixels in groups of CLEAR_BLOCK which is supported by
 // the readout PLD
 if(cameraID == ST5C_CAMERA || cameraID == ST237_CAMERA){
    KLptCameraOut(pd, READOUT_CONTROL, CLR_SELECT);
 }else{ // ST-7/8/...
    KLptCameraOut(pd, CONTROL_OUT, ccd_select);
    KLptCameraOut(pd, TRACKING_CLOCKS, CLR);
 }

 bulk       = len / CLEAR_BLOCK;
 individual = len % CLEAR_BLOCK;

 // clear blocks of CLEAR_BLOCK pixels
 for(j = bulk; j > 0; j--){
     KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(ccd_select + AD_TRIGGER));
     KLptCameraOut(pd, CONTROL_OUT, ccd_select);
     if((status = KLptWaitForPLD(pd)) != CE_NO_ERROR) return(gLastError = status);
 }

 if(cameraID == ST5C_CAMERA || cameraID == ST237_CAMERA){
    KLptCameraOut(pd, READOUT_CONTROL, 0);
 }else{ // ST-7/8/...
    // clear remainder
    switch ( readoutMode ) {
      case 0:
            KLptCameraOut(pd, TRACKING_CLOCKS, 0);
            break;
      case 1:
            KLptCameraOut(pd, TRACKING_CLOCKS, BIN);
            break;
      case 2:
            KLptCameraOut(pd, TRACKING_CLOCKS, BIN + CLR);
            break;
    }
 }
 // clear remainder pixels
 for(j = individual; j > 0; j--){
     KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(ccd_select + AD_TRIGGER));
     KLptCameraOut(pd, CONTROL_OUT, ccd_select);
     if((status = KLptWaitForPLD(pd)) != CE_NO_ERROR) return(gLastError = status);
 }
 return(status);
}
//========================================================================
// KLptCameraIn
// Read data from one of the Camera Registers.
//========================================================================
unsigned char KLptCameraIn(struct private_data *pd, unsigned char reg)
{
 unsigned port_base = pd->port_base;

 outb(reg, port_base);
 return(inb(port_base + 1) >> 3);
}
//========================================================================
// KLptGetPixels
// Get a row of pixels, discarding any on the left, digitizing len,
// discarding on the right.
//========================================================================
int KLptGetPixels(struct private_data  *pd,
                  LinuxGetPixelsParams *arg)
{
 int                    status;
 LinuxGetPixelsParams   lgpp;
 IOC_VCLOCK_CCD_PARAMS  ivcp;
 CAMERA_TYPE            cameraID;
 CCD_REQUEST            ccd;
 unsigned short         u = 0;
 unsigned short         mask;
 unsigned char          ccd_select;
 short                  i, left, len, right, horzBin, st237A;
 unsigned short         *kbuf = (unsigned short *)(pd->buffer);

 // copy IOC_LINUX_GET_PIXELS_PARAMS structure from the user space
 status = copy_from_user(&lgpp,(LinuxGetPixelsParams *)arg,
                         sizeof(LinuxGetPixelsParams)); 					
 if(status != 0){ 					
    printk("<0>KLptGetPixels() : copy_from_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 // init vars
 cameraID = lgpp.gpp.cameraID;
 ccd      = lgpp.gpp.ccd;
 left     = lgpp.gpp.left;
 len      = lgpp.gpp.len;
 right    = lgpp.gpp.right;
 horzBin  = lgpp.gpp.horzBin;
 st237A   = lgpp.gpp.st237A;

 if(lgpp.length < (unsigned long)(2L*len)) return(gLastError = CE_BAD_PARAMETER);

 pd->noBytesRd = 0;
 mask = (cameraID == ST237_CAMERA && !st237A) ? 0x0FFF : 0xFFFF;

 // disable interrupts
 KDisable(pd);

 // do a vertical clock
 ivcp.cameraID   = cameraID;
 ivcp.clearWidth = lgpp.gpp.clearWidth;
 ivcp.onVertBin  = lgpp.gpp.vertBin;

 if(cameraID == ST5C_CAMERA || cameraID == ST237_CAMERA){
    KLptRVClockST5CCCD(pd, &ivcp);
 }else if(ccd == CCD_IMAGING){
    KLptRVClockImagingCCD(pd, &ivcp);
 }else{
    KLptRVClockTrackingCCD(pd, &ivcp);
 }

 // discard unused pixels on left and fill pipeline
 // using the block clear function
 ccd_select = (ccd == CCD_IMAGING ? IMAGING_SELECT : TRACKING_SELECT);
 if(left != 0 && (status = KLptBlockClearPixels(
                           pd,
                           cameraID,
                           ccd,
                           left,
                           0)) != CE_NO_ERROR){
    KEnable(pd);
    return(gLastError = status);
 }
	
 if((status = KLptBlockClearPixels(
              pd,
              cameraID,
              ccd,
              2,
              (short)(horzBin - 1))) != CE_NO_ERROR){
    KEnable(pd);
    return(gLastError = status);
 }

 // Digitize desired pixels
 switch(horzBin){
    case 2:
         KLptCameraOut(pd, TRACKING_CLOCKS, BIN);
         break;
    case 3:
         KLptCameraOut(pd, TRACKING_CLOCKS, BIN + CLR);
         break;
    default:
         KLptCameraOut(pd, TRACKING_CLOCKS, 0);
         break;
 }
	
 KLptCameraOut(pd, CONTROL_OUT, ccd_select); // select desired CCD
 outb(AD0, pd->port_base);                   // address done bit
	
 for(i = 0; i < len; i++){
     // optimize as non-subroutine for Speed
     u = LCONVERSION_DELAY;
     while(1){
        if(!(inb(pd->port_base + 1) & 0x80))
    	   break;
        if(--u == 0){
      	   KEnable(pd);
      	   return(gLastError = CE_AD_TIMEOUT);
        }
     }

     // trigger A/D for next cycle
     KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(ccd_select + AD_TRIGGER));
     KLptCameraOut(pd, CONTROL_OUT, ccd_select);
     K_LPT_READ_AD16(pd, u);
     u &= mask;
     *kbuf++ = u;
 }

 KEnable(pd);

 // wait for last A/D
 if((status = KLptWaitForAD(pd)) != CE_NO_ERROR) return(status);

 // discard unused right pixels
 if(right != 0){
    status = KLptBlockClearPixels(
             pd,
             cameraID,
             ccd,
             (short)(CLEAR_BLOCK*((right + CLEAR_BLOCK - 1)/ CLEAR_BLOCK)),
             0);
 }
 pd->noBytesRd = len << 1;

 status = copy_to_user(lgpp.dest, pd->buffer, lgpp.length);
 if(status != 0){ 					
    printk("<0>KLptGetPixels() : copy_to_user : lgpp.dest : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 return(status);
}
//========================================================================
// KLptGetArea
// Get one or more rows of pixel data.
//========================================================================
int KLptGetArea(struct private_data *pd,
                LinuxGetAreaParams  *arg)
{
 int                   status = CE_NO_ERROR;
 LinuxGetAreaParams    lgap;
 IOC_VCLOCK_CCD_PARAMS ivcp;
 CAMERA_TYPE           cameraID;
 CCD_REQUEST           ccd;
 unsigned short        u = 0;
 unsigned short        mask;
 unsigned char         ccd_select;
 short                 i, j, left, len, right, horzBin, height;
 unsigned short        *kbuf = (unsigned short *)(pd->buffer);
 unsigned short        *p;

 // copy IOC_LINUX_GET_AREA_PARAMS structure from the user space
 status = copy_from_user(&lgap,(LinuxGetAreaParams *)arg,
                         sizeof(LinuxGetAreaParams)); 					
 if(status != 0){ 					
    printk("<0>KLptGetArea() : copy_from_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 // init variables
 cameraID        = lgap.gap.cameraID;
 ccd             = lgap.gap.ccd;
 left            = lgap.gap.left;
 len             = lgap.gap.len;
 right           = lgap.gap.right;
 horzBin         = lgap.gap.horzBin;
 height          = lgap.gap.height;
 pd->noBytesRd   = 0;
 ivcp.cameraID   = cameraID;
 ivcp.clearWidth = lgap.gap.clearWidth;
 ivcp.onVertBin  = lgap.gap.vertBin;
 mask            = (cameraID == ST237_CAMERA && !lgap.gap.st237A) ? 
                   0x0FFF : 0xFFFF;

 // check input parameters 
 if(lgap.length != (unsigned long)height * len * 2){
    return(gLastError = CE_BAD_PARAMETER);
 }
 // check if internal data buffer is long enough
 if(pd->buffer_size < (unsigned long)height * len * 2){
    return(gLastError = CE_BAD_PARAMETER);
 }
 for(i=0; i < height; i++){

     // set actual buffer position
     p = kbuf + i * 2 * len;

     KDisable(pd);

     // do a vertical clock
     if(cameraID == ST5C_CAMERA || cameraID == ST237_CAMERA){
        KLptRVClockST5CCCD(pd, &ivcp);
     }else if(ccd == CCD_IMAGING){
        KLptRVClockImagingCCD(pd, &ivcp);
     }else{
        KLptRVClockTrackingCCD(pd, &ivcp);
     }

     // discard unused pixels on left and fill pipeline
     // using the block clear function
     ccd_select = (ccd == CCD_IMAGING ? IMAGING_SELECT : TRACKING_SELECT);
     if(left != 0 && (status = KLptBlockClearPixels(
                               pd,
                               cameraID,
                               ccd,
                               left,
                               0)) != CE_NO_ERROR){
        KEnable(pd);
        return(gLastError = status);
     }
	
     if((status = KLptBlockClearPixels(
                  pd,
                  cameraID,
                  ccd,
                  2,
                  (short)(horzBin - 1))) != CE_NO_ERROR){
        KEnable(pd);
        return(gLastError = status);
     }

     // Digitize desired pixels
     switch(horzBin){
       case 2:
            KLptCameraOut(pd, TRACKING_CLOCKS, BIN);
            break;
       case 3:
            KLptCameraOut(pd, TRACKING_CLOCKS, BIN + CLR);
            break;
       default:
            KLptCameraOut(pd, TRACKING_CLOCKS, 0);
            break;
     }
	
     KLptCameraOut(pd, CONTROL_OUT, ccd_select); // select desired CCD
     outb(AD0, pd->port_base);                   // address done bit
	
     for(j = 0; j < len; j++){
         // optimize as non-subroutine for Speed
         u = LCONVERSION_DELAY;
         while(1){
            if(!(inb(pd->port_base + 1) & 0x80))
    	       break;
            if(--u == 0){
      	       KEnable(pd);
      	       return(gLastError = CE_AD_TIMEOUT);
            }
         }
         // trigger A/D for next cycle
         KLptCameraOut(pd, CONTROL_OUT, (unsigned char)(ccd_select + AD_TRIGGER));
         KLptCameraOut(pd, CONTROL_OUT, ccd_select);
         K_LPT_READ_AD16(pd, u);
         u &= mask;
         *p++ = u;
     }

     KEnable(pd);

     // wait for last A/D
     if((status = KLptWaitForAD(pd)) != CE_NO_ERROR) return(gLastError = status);

     // discard unused right pixels
     if(right != 0){
        status = KLptBlockClearPixels(
                 pd,
                 cameraID,
                 ccd,
                 (short)(CLEAR_BLOCK*((right + CLEAR_BLOCK - 1)/ CLEAR_BLOCK)),
                 0);
     }

     pd->noBytesRd += len << 1;
 }

 // copy area back to the user space
 status = copy_to_user(lgap.dest, pd->buffer, lgap.length);
 if(status != 0){ 					
    printk("<0>KLptGetArea() : copy_to_user : lgap.dest : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 return(status);
}
//========================================================================
// KLptRVClockImagingCCD
// Vertically clock the Imaging CCD and prepare it for a line readout.
//========================================================================
int KLptRVClockImagingCCD(struct private_data   *pd,
	                  IOC_VCLOCK_CCD_PARAMS *pParams)
{
 int         status     = CE_NO_ERROR;
 CAMERA_TYPE cameraID   = pParams->cameraID;
 short       onVertBin  = pParams->onVertBin;
 short       clearWidth = pParams->clearWidth;
 short       i;

 // clear serial register in case an interrupt came along
 // this needs to be passed incase its the large KAF1600 CCD
 status = KLptBlockClearPixels(pd, cameraID, CCD_IMAGING, clearWidth, 0);
 if(status != CE_NO_ERROR) return(gLastError = status);

 // do vertical shift into readout register
 KDisable(pd);
 // select imaging CCD
 KLptCameraOut(pd, CONTROL_OUT, IMAGING_SELECT);			
 for(i = 0; i < onVertBin; i++){
     KLptVClockImagingCCD(pd, cameraID, IABG_M, 0);
 }
 KEnable(pd);
 return(CE_NO_ERROR);
}
//========================================================================
// KLptRVClockTrackingCCD
// Vertically clock the Tracking CCD and prepare it for a line readout.
//========================================================================
int KLptRVClockTrackingCCD(struct private_data   *pd,
	                   IOC_VCLOCK_CCD_PARAMS *pParams)
{
 short i, onVertBin = pParams->onVertBin;

 // no clear of the serial register is required since when not addressing
 // the CCD the SRG is left low in the low dark current state

 // do vertical shift
 KDisable(pd); // shorts off for vert shift
 KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT); // select tracking CCD
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M + CLR);// set ABG to MID level

 // do vertical shift into readout register
 KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT); // select tracking CCD
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M);      // set ABG to MID level
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M + CLR);// set SRG low from MID

 // do vertical shift
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR + IAG_H);      // IAG high
 for(i = 0; i < onVertBin; i++){
     KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN+IAG_H);// IAG+SRG high
     KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN);      // SRG high
 }
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR);  // both low
 KLptCameraOut(pd, TRACKING_CLOCKS, 0);           // all clocks low
 KEnable(pd);
 return(CE_NO_ERROR);
}
//========================================================================
// KLptRVClockST5CCCD
// Vertically clock the 255 CCD and prepare it for a line readout.
//========================================================================
int KLptRVClockST5CCCD(struct private_data   *pd,
		       IOC_VCLOCK_CCD_PARAMS *pParams)
{
 short i, onVertBin = pParams->onVertBin;

 // no clear of the serial register is required since when not addressing
 // the CCD the SRG is left low in the low dark current state

 // do vertical shift into readout register
 KDisable(pd);
 KLptCameraOut(pd, READOUT_CONTROL, 0);    // select PC control of CCD
 // do vertical shift
 KLptCameraOut(pd, IMAGING_CLOCKS, SAG_H);           // SAG high
 for(i = 0; i < onVertBin; i++){
     KLptCameraOut(pd, IMAGING_CLOCKS, SAG_H+SRG_H); // SAG+SRG high
     KLptCameraOut(pd, IMAGING_CLOCKS, SRG_H);       // SRG high
 }
 KLptCameraOut(pd, IMAGING_CLOCKS, 0);               // all clocks low
 KEnable(pd);
 return(CE_NO_ERROR);
}
//========================================================================
// KLptDumpImagingLines
// Dump lines of pixels at the Imaging CCD.
//
// Since the Imaging CCD is a kodak part without parallel register clear
// the entire readout	register must be cleared by clocking it.
//
// The width is the width of the CCD unbinned and the	len is the number
// of bined rows to dump.
//========================================================================
int KLptDumpImagingLines(struct private_data   *pd,
                         IOC_DUMP_LINES_PARAMS *arg)
{
 int                   status;
 IOC_DUMP_LINES_PARAMS dlp;
 CAMERA_TYPE           cameraID;
 short                 width;
 short                 len;
 short                 vertBin;
 short                 i,j;
 short                 dumpRatio;
 unsigned char         ic;

 // copy IOC_CLEAR_CCD_PARAMS structure from the user space
 status = copy_from_user(&dlp,(IOC_DUMP_LINES_PARAMS *)arg,
                               sizeof(IOC_DUMP_LINES_PARAMS)); 					
 if(status != 0){ 					
    printk("<0>KLptDumpImagingLines() : copy_from_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 cameraID = dlp.cameraID;
 width    = dlp.width;
 len      = dlp.len;
 vertBin  = dlp.vertBin;

 // 5/14/99 - don't turn up Vdd yet
 // ic = TRG_H;

 // 2/21/02 - don't change Vdd, set outside of here
 ic = (pd->imaging_clocks_out & TRG_H);

 KLptCameraOut(pd, CONTROL_OUT, IMAGING_SELECT);

 if(dlp.vToHRatio == 0){
    dumpRatio = DUMP_RATIO;
 }else{
    dumpRatio = dlp.vToHRatio;
 }

 for(i = 0; i < len; i++){
     // do vertical shift of lines
     for(j = 0; j < vertBin; j++){
         KLptVClockImagingCCD(pd, cameraID, (unsigned char)(ic | IABG_M), 0);
     }
     if((i % dumpRatio) == dumpRatio - 1 || i >= len - 3){
        status = KLptBlockClearPixels(
                 pd,
                 cameraID,
                 CCD_IMAGING,
                 (short)(CLEAR_BLOCK*((width+CLEAR_BLOCK-1)/CLEAR_BLOCK)),
                 0);
        if(status != CE_NO_ERROR) return(gLastError = status);
     }
 }
 return(CE_NO_ERROR);
}
//========================================================================
// KLptDumpTrackingLines
// Discard a line of pixels in the tracking CCD.
//
// Since the tracking CCD is a TC211 with full drain clear we only need
// to do the vertical transfer.  No horizontal clear is required.
//
// The width is the width of the CCD unbinned and the	len is the number
// of bined rows to dump.
//========================================================================
int KLptDumpTrackingLines(struct private_data   *pd,
                          IOC_DUMP_LINES_PARAMS *arg)
{
 int                   status;
 IOC_DUMP_LINES_PARAMS dlp;
 CAMERA_TYPE           cameraID;
 short                 width;
 short                 len;
 short                 vertBin;
 short                 i;

 // copy IOC_CLEAR_CCD_PARAMS structure from the user space
 status = copy_from_user(&dlp,(IOC_DUMP_LINES_PARAMS *)arg,
                               sizeof(IOC_DUMP_LINES_PARAMS)); 					
 if(status != 0){ 					
    printk("<0>KLptDumpTrackingLines() : copy_from_user : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 cameraID = dlp.cameraID;
 width    = dlp.width;
 len      = dlp.len;
 vertBin  = dlp.vertBin;

 KDisable(pd); // shorts off for vert shift
 KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT);  // select tracking CCD
 KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR);   // set ABG to MID level

 // do vertical shift
 for(i = vertBin * len; i > 0; i--){
     KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN+IAG_H);// IAG+SRG high
     KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN);      // SRG high
     KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR);          // both low
 }
 KLptCameraOut(pd, TRACKING_CLOCKS, 0);                       // all clocks low
 KEnable(pd);
 // dump the serial register too
 status = KLptBlockClearPixels(pd, cameraID, CCD_TRACKING, width, 0);
 if(status != CE_NO_ERROR) gLastError = status;
 return(status);
}
//========================================================================
// KLptDumpST5CLines:
// Discard a line of pixels in the TC255 CCD.
//
// Since the CCD is a TC255 with full	drain clear we only need to do the
// vertical transfer. No horizontal clear is required.
//
// The width is the width of the CCD unbinned and the	len is the number
// of bined rows to dump.
//========================================================================
int KLptDumpST5CLines(struct private_data   *pd,
                      IOC_DUMP_LINES_PARAMS *arg)
{
 int                   status;
 IOC_DUMP_LINES_PARAMS dlp;
 CAMERA_TYPE           cameraID;
 short                 width;
 short                 len;
 short                 vertBin;
 short                 i;

 // copy IOC_CLEAR_CCD_PARAMS structure from the user space
 status = copy_from_user(&dlp,(IOC_DUMP_LINES_PARAMS *)arg,
                               sizeof(IOC_DUMP_LINES_PARAMS)); 					
 if(status != 0){ 					
    printk("<0>KLptDumpST5CLines() : copy_from_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 cameraID = dlp.cameraID;
 width    = dlp.width;
 len      = dlp.len;
 vertBin  = dlp.vertBin;

 KDisable(pd); // interrupts off for vert shift
 KLptCameraOut(pd, READOUT_CONTROL, 0); // select PC control of CCD
 // do vertical shift
 for(i = vertBin*len; i > 0; i--){
     KLptCameraOut(pd, IMAGING_CLOCKS, SAG_H);      // SAG high
     KLptCameraOut(pd, IMAGING_CLOCKS, SAG_H+SRG_H);// SAG+SRG high
     KLptCameraOut(pd, IMAGING_CLOCKS, SRG_H);      // SRG
     KLptCameraOut(pd, IMAGING_CLOCKS, 0);          // all low
 }
 KEnable(pd);
 // Dump the serial register too
 status = KLptBlockClearPixels(pd, cameraID, CCD_IMAGING, width, 0);
 if(status != CE_NO_ERROR) gLastError = status;
 return(status);
}
//========================================================================
// KLptClockAD
// Clock the AD the number of times passed.
//========================================================================
int KLptClockAD(struct private_data *pd, short *arg)
{
 int    status;
 short  len;
 unsigned short u;

 // get arg value
 if((status = get_user(len, arg)) != 0){ 					
    printk("<0>KLptClockAD() : get_user() : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(status);
 }

 KLptCameraOut(pd, TRACKING_CLOCKS, KBIN1);
 while(len){
     // wait for A/D
     if((status = KLptWaitForAD(pd)) != CE_NO_ERROR){
        return(gLastError = status);
     }
     // trigger A/D for next cycle
     KLptCameraOut(pd, CONTROL_OUT, AD_TRIGGER);
     KLptCameraOut(pd, CONTROL_OUT, 0);
     K_LPT_READ_AD16(pd, u);
     len--;
 }
 // wait for last A/D
 status = KLptWaitForAD(pd);
 if(status != CE_NO_ERROR) gLastError = status;
 return(status);
}
//========================================================================
// KLptClearImagingArray
// Do a fast clear of the imaging array by doing vertical shifts and
// only clearing CLEAR_BLOCK pixels per line.
//========================================================================
int KLptClearImagingArray(struct private_data  *pd,
		  	  IOC_CLEAR_CCD_PARAMS *arg)
{
 int                  status = CE_NO_ERROR;
 CAMERA_TYPE          cameraID;
 short                height;
 short                times;
 short                i;
 IOC_CLEAR_CCD_PARAMS cccdp;

 // copy IOC_CLEAR_CCD_PARAMS structure from the user space
 status = copy_from_user(&cccdp,(IOC_CLEAR_CCD_PARAMS *)arg,
                         sizeof(IOC_CLEAR_CCD_PARAMS)); 					
 if(status != 0){ 					
    printk("<0>KLptClearImagingArray() : copy_from_user : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 cameraID = arg->cameraID;
 height   = arg->height;
 times    = arg->times;

 KLptCameraOut(pd, CONTROL_OUT, IMAGING_SELECT);
 KLptCameraOut(pd, TRACKING_CLOCKS, CLR);
 times *= height;

 // clear the array the required number of times
 for(i = 0; i < times; i++){
     status = KLptVClockImagingCCD(pd, cameraID, IABG_M, 2);
     if(status != CE_NO_ERROR) return(gLastError = status);

     // do Horizontal Clear of Blocks of 10 Pixels
     status = KLptHClear(pd, (short)(cameraID == ST1K_CAMERA ? 6 : 1));
     if(status != CE_NO_ERROR) return(gLastError = status);
 }
 return(CE_NO_ERROR);
}
//========================================================================
// KLptClearTrackingArray
// Do a fast clear of the tracking array by doing vertical shifts and
// only clearing CLEAR_BLOCK pixels per line.
//========================================================================
int KLptClearTrackingArray(struct private_data  *pd,
     	                   IOC_CLEAR_CCD_PARAMS *arg)
{
 int                  status = CE_NO_ERROR;
 CAMERA_TYPE          cameraID;
 short                height;
 short                times;
 short                i;
 IOC_CLEAR_CCD_PARAMS cccdp;

 // copy IOC_CLEAR_CCD_PARAMS structure from the user space
 status = copy_from_user(&cccdp,(IOC_CLEAR_CCD_PARAMS *)arg,
                         sizeof(IOC_CLEAR_CCD_PARAMS)); 					
 if(status != 0){ 					
    printk("<0>KLptClearTrackingArray() : copy_from_user : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 cameraID = arg->cameraID;
 height   = arg->height;
 times    = arg->times;

 KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT);
 times *= height;
	
 // clear the array the required number of times
 for(i = 0; i < times; i++){
    KDisable(pd);

    KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+IAG_H);    // IAG high
    KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN+IAG_H);// IAG+SRG high
    KLptCameraOut(pd, TRACKING_CLOCKS, TABG_M+CLR+BIN);      // SRG high

    // then shift CLEAR_BLOCK horizontally
    KLptCameraOut(pd, TRACKING_CLOCKS, CLR);  // set PAL for clear
                                              // all clocks low
    KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT + AD_TRIGGER);
    KLptCameraOut(pd, CONTROL_OUT, TRACKING_SELECT);

    KEnable(pd);

    if((status = KLptWaitForPLD(pd)) != CE_NO_ERROR){
       return(gLastError = status);
    }
 }
 return(CE_NO_ERROR);
}
//========================================================================
// KLptGetDriverInfo
// Return the driver info.
//========================================================================
int KLptGetDriverInfo(GetDriverInfoResults0 *results)
{
 int                   status;
 GetDriverInfoResults0 gdir0;

 gdir0.version    = DRIVER_VERSION;
 strcpy(gdir0.name, DRIVER_STRING);
 gdir0.maxRequest = 1;

 status = copy_to_user(results, &gdir0, sizeof(GetDriverInfoResults0));
 if(status != 0){ 					
    printk("<0>KLptGetDriverInfo() : copy_to_user : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }
 return(CE_NO_ERROR);
}
//========================================================================
int KLptSetBufferSize(struct private_data *pd,
                      spinlock_t          *lock,
                      unsigned short      *new_size)
{
 int    status = CE_NO_ERROR;
 unsigned short buffer_size;
 char   *kbuff = (char *)pd->buffer;

 // check if user-space variable is available
 if(access_ok(VERIFY_READ, new_size, sizeof(unsigned short *)) == 0){
    printk("<0>KLptSetBufferSize() : access_ok() : error\n"); 
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }
 // get size of the new buffer, ie. read unsigned short
 if(__get_user(buffer_size, new_size) != 0){ 					
    printk("<0>KLptSetBufferSize() : get_user() : error\n");
    gLastError = CE_BAD_PARAMETER;
    return(-EFAULT);
 }

 // allocate new kernel-space I/O buffer
 if((kbuff = kmalloc(buffer_size, GFP_KERNEL)) == NULL){
    printk("<0>KLptSetBufferSize() : kmalloc() : new size : %d : error\n",
            buffer_size);
    // allocation failured, return previous buffer size
    return(pd->buffer_size);
 }

 // set pointer to new I/O buffer and swap buffers
 pd->buffer_size = buffer_size;

 // kfree old kernel-space I/O buffer and asign new one
 spin_lock(lock);
 kfree(pd->buffer);
 pd->buffer = kbuff;
 spin_unlock(lock);

 status = pd->buffer_size;
 # ifdef _CHATTY_
 printk("<0>KLptSetBufferSize() : %d\n", status);
 #endif
 return(status);
}
//========================================================================
int KLptGetBufferSize(struct private_data *pd)
{
 PAR_ERROR status = pd->buffer_size;

 #ifdef _CHATTY_
 printk("<0>KLptGetBufferSize() : %d\n", status);
 #endif
 return(status);
}
//========================================================================
int KLptTestCommand(void){
 #ifdef _CHATTY_
 printk("<0>KLptTestCommand() : ok\n");
 #endif
 return(CE_NO_ERROR);
}
//========================================================================
// KLptGetJiffies
// Get jiffies, ie. number of ticks from the boot time.
//========================================================================
int KLptGetJiffies(unsigned long *arg)
{
 return(put_user((unsigned long)jiffies, arg));
}
//========================================================================
// KLptGetHz
// Get HZ.
//========================================================================
int KLptGetHz(unsigned long *arg)
{
// return(put_user((unsigned long)HZ, arg));
 return(put_user((unsigned long)gLptHz, arg));
}
//========================================================================
// KDevOpen
//========================================================================
int KDevOpen(struct inode *inode,
             struct file  *filp,
	     int           lpt_base,
	     int 	   lpt_span,
	     int           buffer_size)
{
 int status = 0;  // assume success

 // allocate private data structure
 if((status = KAllocatePrivateData(
              filp,
              lpt_base,
	      lpt_span,
	      buffer_size)) < 0){
    // decrement the usage count
    gLastError = CE_DEVICE_NOT_OPEN;
    return(status);
 }

 // allocate LPT ports
 if((status = KAllocateLptPorts(filp)) < 0){
    // release private data structure
    KReleasePrivateData(filp);
    gLastError = CE_DEVICE_NOT_OPEN;
    return(status);
 }

 #ifdef _CHATTY_	
 KDumpPrivateData(filp);
 #endif

 return(status);	
}
//========================================================================
// KDevRelease
//========================================================================
int KDevRelease(struct inode *inode,
                struct file  *filp)
{
 // release lpt ports
 KReleaseLptPorts(filp);

 // release private data structure				
 KReleasePrivateData(filp);

 return(0);	
}
//========================================================================
// KDevIoctl
//========================================================================
long KDevIoctl(struct file   *filp,
              unsigned int   cmd,
              unsigned long  arg,
	      spinlock_t    *spin_lock)
{
 int status = CE_NO_ERROR;
 struct private_data *pd = (struct private_data *)(filp->private_data);

 if(_IOC_TYPE(cmd) != IOCTL_BASE){
    printk("<0>KDevIoctl() : error: IOCTL base %d, must be %d\n",
           _IOC_TYPE(cmd), IOCTL_BASE);	 
    gLastError = CE_BAD_PARAMETER;
    return(-ENOTTY);
 }

 switch(cmd){
   case LIOCTL_INIT_PORT:
	KLptInitPort(pd);
	break;

   case LIOCTL_CAMERA_OUT:
        KLptCameraOutWrapper(pd, (LinuxCameraOutParams *)arg);
        break;

   case LIOCTL_SEND_MICRO_BLOCK:
        status = KLptSendMicroBlock(pd, (LinuxMicroblock *)arg);
	break;

   case LIOCTL_GET_MICRO_BLOCK:
        status = KLptGetMicroBlock (pd, (LinuxMicroblock *)arg);
	break;

   case LIOCTL_SET_VDD:
	KLptSetVdd(pd, (IocSetVdd *)arg);
	break;

   case LIOCTL_CLEAR_IMAG_CCD:
        status = KLptClearImagingArray(pd, (IOC_CLEAR_CCD_PARAMS *)arg);
	break;

   case LIOCTL_CLEAR_TRAC_CCD:
        status = KLptClearTrackingArray(pd, (IOC_CLEAR_CCD_PARAMS *)arg);
	break;

   case LIOCTL_GET_PIXELS:
        status = KLptGetPixels(pd, (LinuxGetPixelsParams *)arg);
	break;

   case LIOCTL_GET_AREA:
        status = KLptGetArea(pd, (LinuxGetAreaParams *)arg);
        break;

   case LIOCTL_GET_JIFFIES:
        status = KLptGetJiffies((unsigned long *)arg);
        break;

   case LIOCTL_GET_HZ:
        status = KLptGetHz((unsigned long *)arg);
        break;

   case LIOCTL_GET_LAST_ERROR:
        status = KSbigLptGetLastError((unsigned short *)arg);
        break;

   case LIOCTL_DUMP_ILINES:
        status = KLptDumpImagingLines(pd, (IOC_DUMP_LINES_PARAMS *)arg);
        break;

   case LIOCTL_DUMP_TLINES:
	status = KLptDumpTrackingLines(pd, (IOC_DUMP_LINES_PARAMS *)arg);
        break;

   case LIOCTL_DUMP_5LINES:
        status = KLptDumpST5CLines(pd, (IOC_DUMP_LINES_PARAMS *)arg);
        break;

   case LIOCTL_CLOCK_AD:
	status = KLptClockAD(pd, (short *)arg);
	break;

   case LIOCTL_GET_DRIVER_INFO:
        status = KLptGetDriverInfo((GetDriverInfoResults0 *)arg);
	break;

   case LIOCTL_REALLOCATE_PORTS:
        status = KReallocateLptPorts(filp, (LinuxLptPortParams *)arg);
	break;

   case LIOCTL_SET_BUFFER_SIZE:
        status = KLptSetBufferSize(pd, spin_lock, (unsigned short *)arg);
        break;

   case LIOCTL_GET_BUFFER_SIZE:
        status = KLptGetBufferSize(pd);
        break;

   case LIOCTL_TEST_COMMAND:
        status = KLptTestCommand();
        break;

   default:
        printk("<0>KDevIoctl() : Developer error: undefined LIOCTL.\n"); 
	gLastError = CE_BAD_PARAMETER;
        return(-ENOTTY);
 }

 if(status != CE_NO_ERROR) gLastError = status;

 return(status);
}
//========================================================================
// KSbigLptGetLastError
//========================================================================
int KSbigLptGetLastError(unsigned short *arg)
{ 
 int status = put_user(gLastError, arg);
 if(status == 0){
    gLastError = CE_NO_ERROR;
 }else{
    gLastError = CE_BAD_PARAMETER;
 }
 return(status);
}
//========================================================================
