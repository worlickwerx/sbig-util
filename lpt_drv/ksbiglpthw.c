//========================================================================
// File name  : ksbiglpthw.c
// Description: The hardware source file.
// Author     : Jan Soldan 
// Copyright (C) 2002 - 2004 Jan Soldan			     
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

#include "sbigudrv.h"
#include "ksbiglptmain.h"
#include "ksbiglpthw.h"
#include "ksbigdef.h"

extern spinlock_t  d0_spinlock;
//========================================================================
// KAllocatePrivateData
//========================================================================
int KAllocatePrivateData(struct file *filp, 
                         int          port_base, 
			 int          port_span, 
			 int          buffer_size)
{
 char *buff = (char *)NULL;
 struct private_data *pd = (struct private_data *)(filp->private_data);

 // grab spinlock
 spin_lock(&d0_spinlock);

 // check if the device is available, ie. 'pd' should be NULL pointer
 if(pd){
    // device already open
    #ifdef _CHATTY_
    printk("<0>KAllocatePrivateData() : device already open!\n");
    #endif
    // release spinlock
    spin_unlock(&d0_spinlock);
    return(-EBUSY);
 }

 // device is free, we got it...

 // allocate private_data structure
 if((pd = kmalloc(sizeof(struct private_data), GFP_KERNEL)) == NULL){
    printk("<0>KAllocatePrivateData() : kmalloc() : "
           "struct private_data : error!\n");
    filp->private_data = (struct private_data *)NULL;
    spin_unlock(&d0_spinlock);
    return(-ENOMEM);
 }

 // allocate I/O buffer
 if((buff = kmalloc(buffer_size, GFP_KERNEL)) == NULL){
    printk("<0>KAllocatePrivateData() : kmalloc() : "
           "I/O buffer : error!\n");
    kfree(pd);
    spin_unlock(&d0_spinlock);
    return(-ENOMEM);
 }

 // set pointer to private_data inside the filp
 filp->private_data = pd;

 // release spinlock because nobody can change our private data now
 spin_unlock(&d0_spinlock);

 // initialize private_data structure
 pd->port_base          = port_base;
 pd->port_span          = port_span;
 pd->buffer_size        = buffer_size;
 pd->flags              = 0;
 pd->control_out        = 0;
 pd->imaging_clocks_out = 0;
 pd->noBytesRd          = 0;
 pd->noBytesWr          = 0;
 pd->state              = 0;
 pd->buffer             = buff;
 strcpy(pd->dev_name, LDEV_NAME);

 return(0);
}
//========================================================================
// KReleasePrivateData
//========================================================================
int KReleasePrivateData(struct file *filp)
{
 struct private_data *pd  = (struct private_data *)(filp->private_data);

 if(pd){
    // release I/O buffer
    if(pd->buffer){
       kfree(pd->buffer);
    }
    // release private_data
    kfree(pd);
    filp->private_data = NULL;
 }

 return(0);
}
//========================================================================
// KAllocateLptPorts
//========================================================================
int KAllocateLptPorts(struct file *filp)
{
 struct private_data *pd = (struct private_data *)(filp->private_data);

 // request I/O region
 if(request_region(pd->port_base, pd->port_span, pd->dev_name) == NULL){
    printk("<0>KAllocateLptPorts() : port_base %X, port_span %d error!\n",
	   pd->port_base, pd->port_span);
    printk("<0>LPT port probably allocated by your printer.\n");
    printk("<0>Please uninstall your printer and try again.\n");
    printk("<0>Use: modprobe -r lp, modprobe -r parport_pc, modprobe -r parport\n");
    return(-EBUSY);
 }

 return(0);
}
//========================================================================
// KReleaseLptPorts
//========================================================================
int KReleaseLptPorts(struct file *filp)
{
 struct private_data *pd = (struct private_data *)(filp->private_data);
 release_region(pd->port_base, pd->port_span);

 return(0);
}
//========================================================================
// KReallocateLptPorts
//========================================================================
int KReallocateLptPorts(struct file *filp, LinuxLptPortParams *arg)
{
 int                  status = CE_NO_ERROR;
 LinuxLptPortParams   llpp;
 struct private_data *pd = (struct private_data *)(filp->private_data);

 // copy IOC_LINUX_PORT_PARAMS structure from the user space
 status = copy_from_user(&llpp, (LinuxLptPortParams *)arg,
                         sizeof(LinuxLptPortParams)); 					
 if(status != 0){ 					
    printk("<0>KReallocateLptPorts() : copy_from_user : error!\n");
    return(status);
 }

 /*
 printk("<0>KReallocateLptPorts() : \n");
 printk("<0>current  values : portBase %X, portSpan %d, name %s\n",
        pd->port_base, pd->port_span, pd->dev_name);
 printk("<0>requested values: portBase %X, portSpan %d, name %s\n",
	llpp.portBase, llpp.portSpan, pd->dev_name);
 */

 // request I/O region
 if(request_region(llpp.portBase, llpp.portSpan, pd->dev_name) == NULL){
    // somebody holds requested lpt ports...
    printk("<0>KReallocateLptPorts() : request_region() : error!\n");
    printk("<0>current  values : portBase %X, portSpan %d, name %s\n",
	   pd->port_base, pd->port_span, pd->dev_name);
    printk("<0>requested values: portBase %X, portSpan %d, name %s\n",
	   llpp.portBase, llpp.portSpan, pd->dev_name);
    return(-EBUSY);
 }

 // reallocation OK, release old ports and record changes inside the
 // private data structure 

 // release old LPT ports
 KReleaseLptPorts(filp);

 // set new port's base & span values
 pd->port_base = llpp.portBase;
 pd->port_span = llpp.portSpan;

 return(0);
}
//========================================================================
// KDumpPrivateData
//========================================================================
void KDumpPrivateData(struct file *filp)
{
 struct private_data *pd  = (struct private_data *)(filp->private_data);

 printk("<0>---------------------------------------------------\n");
 printk("<0>Device private data :\n");
 printk("<0>---------------------------------------------------\n");
 printk("<0>dev_name    : %s\n", pd->dev_name);
 printk("<0>port_base   : %x\n", pd->port_base);
 printk("<0>port_span   : %x\n", pd->port_span);
 printk("<0>buffer      : p = %p,  size = %d\n",
         &(pd->buffer), pd->buffer_size);
 
 if((filp->f_mode & FMODE_READ) && (filp->f_mode & FMODE_WRITE)){
    printk("<0>I/O mode    : device opened for reading and writing\n");
 }else if(filp->f_mode & FMODE_READ){
    printk("<0>I/O mode    : device opened for reading\n");
 }else if(filp->f_mode & FMODE_WRITE){
    printk("<0>I/O mode    : device opened for writing\n");
 }
 printk("<0>---------------------------------------------------\n");
}
//========================================================================
