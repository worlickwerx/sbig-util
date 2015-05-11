//========================================================================
// File name  : ksbiglptd1.c
// Description: The device 1 of the driver.
// Author     : Jan Soldan 
// Copyright (C) 2002 - 2003 Jan Soldan			     
// All rights reserved.		
//========================================================================
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
#include <asm/io.h>

#include "ksbigdef.h"
#include "ksbiglptmain.h"
#include "ksbiglpthw.h"
#include "ksbiglpt.h"
#include "ksbiglptd.h"

extern spinlock_t d1_spinlock;
//========================================================================
// KDev1Open
//========================================================================
int KDev1Open(struct inode *inode,
              struct file  *filp)
{
 return(KDevOpen(inode, filp, LPT1_BASE, LPT_SPAN, LDEFAULT_BUFFER_SIZE));
}
//========================================================================
// KDev1Release
//========================================================================
int KDev1Release(struct inode *inode,
                 struct file  *filp)
{
 return(KDevRelease(inode, filp));
}
//========================================================================
// KDev1Ioctl
//========================================================================
int KDev1Ioctl(struct inode   *inode,
               struct file    *filp,
               unsigned int   cmd,
               unsigned long  arg)
{
 return(KDevIoctl(inode, filp, cmd, arg, &d1_spinlock));
}
//========================================================================





