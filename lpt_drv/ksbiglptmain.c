//========================================================================
// File name  : ksbiglptmain.c
// Description: The main source file of the kernel driver.
// Author     : Jan Soldan    
// Copyright (C) 2002 - 2003 Jan Soldan			     
// All rights reserved.		
//========================================================================
/* kernel stuff */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/wrapper.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/tty.h>
#include <linux/delay.h>
#include <asm/io.h>

/* project stuff */
#include "ulptdrv.h"
#include "ksbiglptd.h"
#include "ksbiglptmain.h"   

extern unsigned long gLptHz;
extern unsigned long gLastError;
//========================================================================
module_init(KModInit);
module_exit(KModExit);
//========================================================================
// driver variables
//========================================================================
uint        dev_major  = LDEV_MAJOR;
char        dev_name[] = LDEV_NAME;

spinlock_t  d0_spinlock;
spinlock_t  d1_spinlock;
spinlock_t  d2_spinlock;
//========================================================================
// file_operations structures
//========================================================================
struct file_operations d0_fops = {	// device 0
 open:    KDev0Open,
 release: KDev0Release,
 ioctl:   KDev0Ioctl,
};

struct file_operations d1_fops = {	// device 1
 open:    KDev1Open,
 release: KDev1Release,
 ioctl:   KDev1Ioctl,
};

struct file_operations d2_fops = {	// device 2
 open:    KDev2Open,
 release: KDev2Release,
 ioctl:   KDev2Ioctl,
};
//========================================================================
// array of file_operations structures
//========================================================================
// Don't forget update the DEV_MAX_IDX constant defined in kmain.h
// after adding new file operations structure into the following array!

struct file_operations *dev_fops_array[] = {
 &d0_fops,  // device 0
 &d1_fops,  // device 1
 &d2_fops,  // device 2
};
//========================================================================
// KModInit called by insmod
// return 0 if OK
//========================================================================
int KModInit(void)
{
 int status = 0;
 unsigned long t0, t1, dt;

 EXPORT_NO_SYMBOLS;
 
 gLastError = CE_NO_ERROR;

 // Determine an actual HZ value.
 gLptHz = 1;

 do{
    t0 = jiffies;
    mdelay(1000);
    t1 = jiffies;
 }while(t1 < t0);

 dt = t1 - t0;

 if(dt >= 90 && dt <= 110){
    gLptHz = 100;
 }else if(dt >= 950 && dt <= 1010){
    gLptHz = 1000;
 }else if(dt >= 1011 && dt <= 1030){
    gLptHz = 1024;
 }

 #ifdef _CHATTY_	
 printk("<0>gLptHz: %lu.\n", gLptHz);
 #endif

 // LPT cameras
 // initialize spinlocks
 spin_lock_init(&d0_spinlock);
 spin_lock_init(&d1_spinlock);
 spin_lock_init(&d2_spinlock);

 if((status = register_chrdev(dev_major, dev_name, &d0_fops)) < 0){
    printk("<0>register_chrdev() : error %X [hex]\n", status);
    gLastError = CE_DEVICE_NOT_IMPLEMENTED;
    return(status);
 }

 if(LDEV_MAJOR == 0){
    dev_major = status;
    status = 0;
 }

 #ifdef _CHATTY_	
 printk("<0>KModInit() : module loaded, MAJOR number: %d\n", dev_major);
 #endif

 return(status);
}
//========================================================================
// KModExit - called by rmmod
//========================================================================
void KModExit(void)
{
 // unregister LPT driver
 unregister_chrdev(dev_major, dev_name);

 #ifdef _CHATTY_
 printk("<0>KModExit() : module unloaded...\n");
 #endif
}
//========================================================================







