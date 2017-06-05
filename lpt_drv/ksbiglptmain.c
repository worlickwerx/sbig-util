//========================================================================
// File name  : ksbiglptmain.c
// Description: The main source file of the kernel driver.
// Author     : Jan Soldan    
// Copyright (C) 2002 - 2003 Jan Soldan			     
// All rights reserved.		
//========================================================================
/* kernel stuff */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
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

/* project stuff */
#include "ulptdrv.h"
#include "ksbiglptd.h"
#include "ksbiglptmain.h"   

extern unsigned short gLastError;
//========================================================================
module_init(KModInit);
module_exit(KModExit);
//========================================================================
// driver variables
//========================================================================
dev_t       dev_device;
struct cdev dev_cdev;

spinlock_t  d0_spinlock;
spinlock_t  d1_spinlock;
spinlock_t  d2_spinlock;
//========================================================================
// file_operations structures
//========================================================================
struct file_operations d0_fops = {	// device 0
 open:    KDev0Open,
 release: KDev0Release,
 unlocked_ioctl:   KDev0Ioctl,
};

struct file_operations d1_fops = {	// device 1
 open:    KDev1Open,
 release: KDev1Release,
 unlocked_ioctl:   KDev1Ioctl,
};

struct file_operations d2_fops = {	// device 2
 open:    KDev2Open,
 release: KDev2Release,
 unlocked_ioctl:   KDev2Ioctl,
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
 gLastError = CE_NO_ERROR;

 // LPT cameras
 // initialize spinlocks
 spin_lock_init(&d0_spinlock);
 spin_lock_init(&d1_spinlock);
 spin_lock_init(&d2_spinlock);

 if(alloc_chrdev_region(&dev_device, 0, LDEV_MAX_INDEX + 1, LDEV_NAME) < 0){
    printk(KERN_ERR "%s() : alloc_chrdev_region failed\n", __FUNCTION__);
    gLastError = CE_DEVICE_NOT_IMPLEMENTED;
    return(-1);
 }
 cdev_init (&dev_cdev, &d0_fops);
 if(cdev_add (&dev_cdev, dev_device, LDEV_MAX_INDEX + 1)<0){
    printk(KERN_ERR "%s() : cdev_add failed\n", __FUNCTION__);
    gLastError = CE_DEVICE_NOT_IMPLEMENTED;
    unregister_chrdev_region(dev_device, LDEV_MAX_INDEX + 1);
    return(-1);
 }

 #ifdef _CHATTY_	
 printk(KERN_DEBUG "%s() : module loaded, MAJOR number: %d\n",
                    __FUNCTION__, MAJOR (dev_device));
 #endif

 return(0);
}
//========================================================================
// KModExit - called by rmmod
//========================================================================
void KModExit(void)
{
 // unregister LPT driver
 unregister_chrdev_region(dev_device, LDEV_MAX_INDEX + 1);

 #ifdef _CHATTY_
 printk(KERN_DEBUG "%s() : module unloaded...\n", __FUNCTION__);
 #endif
}
//========================================================================







