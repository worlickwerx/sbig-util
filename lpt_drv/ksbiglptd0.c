//========================================================================
// File name  : ksbiglptd0.c
// Description: The LPT device 0 of the kernel driver.
// Author     : Jan Soldan
// Copyright (C) 2002 - 2003 Jan Soldan			     
// All rights reserved.		
//========================================================================
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
#include <asm/io.h>

#include "ksbigdef.h"
#include "ksbiglptmain.h"
#include "ksbiglpthw.h"
#include "ksbiglpt.h"
#include "ksbiglptd.h"

extern struct file_operations  *dev_fops_array[];
extern spinlock_t              d0_spinlock;
//========================================================================
// KDev0Open
//========================================================================
int KDev0Open(struct inode *inode,
              struct file  *filp)
{
 int minor  = MINOR(inode->i_rdev);

 #ifdef _CHATTY_	
 printk(KERN_DEBUG "Minor number : %d\n", minor);
 #endif

 // check device minor number
 if(minor > LDEV_MAX_INDEX){
    printk(KERN_ERR "%s() : minor must be <= %d\n", __FUNCTION__, LDEV_MAX_INDEX);
    return(-ENODEV);
 }

 if(minor > 0){	
    // dispatch to specific open
    filp->f_op = dev_fops_array[minor];
    return(filp->f_op->open(inode, filp));	
 }

 // continue with device 0
 return(KDevOpen(inode, filp, LPT0_BASE, LPT_SPAN, LDEFAULT_BUFFER_SIZE));
}
//========================================================================
// KDev0Release
//========================================================================
int KDev0Release(struct inode *inode,
                 struct file  *filp)
{
 return(KDevRelease(inode, filp));
}
//========================================================================
// KDev0Ioctl
//========================================================================
long KDev0Ioctl(struct file  *filp,
               uint         cmd,
               ulong        arg)
{
 return(KDevIoctl(filp, cmd, arg, &d0_spinlock));
}
//========================================================================




