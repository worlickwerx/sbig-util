/* SBIG astronomy camera parallel port driver
 *
 * Replacement driver mainline developed on Linux 4.10, supercedes mainline
 * written for Linux 2.4 by Jan Soldan (c) 2002-2003.
 *
 * Copyright (c) 2017 by Jim Garlick
 */

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
#include <linux/device.h>
#include <linux/parport.h>
#include <asm/io.h>

#include "ulptdrv.h"
#include "ksbiglptmain.h"
#include "ksbiglpt.h"

#define SBIG_NO	3
static struct {
	struct pardevice 	*dev;
	spinlock_t  		spinlock;
} sbig_table[SBIG_NO];
static unsigned int sbig_count = 0;

static dev_t sbig_dev;
static struct class *sbig_class;
static struct cdev sbig_cdev;

static void sbig_outb (uint8_t data, unsigned int minor)
{
	struct parport *port = sbig_table[minor].dev->port;

	port->ops->write_data(port, data);
}

static uint8_t sbig_inb (unsigned int minor)
{
	struct parport *port = sbig_table[minor].dev->port;

	return port->ops->read_status (port);
}

static int sbig_open(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	struct private_data *pd = (struct private_data *)(file->private_data);
	int rc = 0;

	if (minor >= sbig_count) {
		rc = -ENODEV;
		goto out;
	}
	spin_lock (&sbig_table[minor].spinlock);
	if (pd) {
		rc = -EBUSY;
		goto out_unlock;
	}
	if (!(pd = kmalloc(sizeof(*pd), GFP_KERNEL))) {
		rc = -ENOMEM;
		goto out_unlock;
	}
	if (!(pd->buffer = kmalloc(LDEFAULT_BUFFER_SIZE, GFP_KERNEL))) {
		kfree (pd);
		rc = -ENOMEM;
		goto out_unlock;
	}
	pd->buffer_size = LDEFAULT_BUFFER_SIZE;
	pd->flags = 0;
	pd->control_out = 0;
	pd->imaging_clocks_out = 0;
	pd->noBytesRd = 0;
	pd->noBytesWr = 0;
	pd->state = 0;
	pd->pp_outb = sbig_outb;
	pd->pp_inb = sbig_inb;
	pd->minor = minor;
	file->private_data = pd;
out_unlock:
	spin_unlock (&sbig_table[minor].spinlock);
out:
	return rc;
}

static int sbig_release(struct inode *inode, struct file *file)
{
	struct private_data *pd = (struct private_data *)(file->private_data);
	if (pd) {
		if (pd->buffer)
			kfree(pd->buffer);
		kfree(pd);
		file->private_data = NULL;
	}
	return 0;
}

static long sbig_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int minor = iminor(file_inode(file));

	return KDevIoctl(file, cmd, arg, &sbig_table[minor].spinlock);
}

static void sbig_attach (struct parport *port)
{
	int nr = 0 + sbig_count;

	if (!(port->modes & PARPORT_MODE_PCSPP)) {
		printk(KERN_INFO "sbiglpt: ignoring %s - no SPP capability\n",
			port->name);
		return;
	}
	if (nr == SBIG_NO) {
		printk(KERN_INFO "sbiglpt: ignoring %s - max %d reached\n",
			port->name, SBIG_NO);
		return;
	}
	sbig_table[nr].dev = parport_register_device(port, "sbiglpt",
						     NULL, NULL, NULL, 0, NULL);
	if (sbig_table[nr].dev == NULL) {
		printk(KERN_ERR "sbiglpt%d: parport_register_device failed\n",
			nr);
		return;
	}
	if (parport_claim (sbig_table[nr].dev)) {
		parport_unregister_device(sbig_table[nr].dev);
		printk(KERN_ERR "sbiglpt%d: parport_claim failed\n", nr);
		return;
	}
	device_create(sbig_class, port->dev, MKDEV (MAJOR(sbig_dev), nr),
		      NULL, "sbiglpt%d", nr);
	spin_lock_init(&sbig_table[nr].spinlock);
	sbig_count++;
	printk(KERN_INFO "sbiglpt%d: using %s\n", nr, port->name);
}

static void sbig_detach (struct parport *port)
{
	//printk(KERN_INFO "sbiglpt: detach %s\n", port->name);
}

static struct parport_driver sbig_driver = {
	.name = "sbiglpt",
	.attach = sbig_attach,
	.detach = sbig_detach,
};

static struct file_operations sbig_fops = {
	.owner		= THIS_MODULE,
	.open 		= sbig_open,
	.release	= sbig_release,
	.unlocked_ioctl = sbig_ioctl,
};

static int sbig_init_module (void)
{
	if (alloc_chrdev_region(&sbig_dev, 0, SBIG_NO, "sbiglpt") < 0) {
		printk(KERN_ERR "sbiglpt: alloc_chrdev_region failed\n");
		goto out;
	}
	if (!(sbig_class = class_create(THIS_MODULE, "chardrv"))) {
		printk(KERN_ERR "sbiglpt: class_create failed\n");
		goto out_reg;
	}
	cdev_init (&sbig_cdev, &sbig_fops);
	if (cdev_add (&sbig_cdev, sbig_dev, SBIG_NO) < 0) {
		printk(KERN_ERR "sbiglpt: cdev_add failed\n");
		goto out_class;
	}
	if (parport_register_driver (&sbig_driver)) {
		printk(KERN_ERR "sbiglpt: parport_register_driver failed\n");
		goto out_cdev;
	}
	return(0);
out_cdev:
	cdev_del(&sbig_cdev);
out_class:
	class_destroy(sbig_class);
out_reg:
	unregister_chrdev_region(sbig_dev, SBIG_NO);
out:
	return(-1);
}

static void sbig_cleanup_module(void)
{
	int nr;
	parport_unregister_driver(&sbig_driver);
	cdev_del(&sbig_cdev);
	for (nr = 0; nr < sbig_count; nr++) {
		parport_release(sbig_table[nr].dev);
		parport_unregister_device(sbig_table[nr].dev);
 		device_destroy(sbig_class, MKDEV (MAJOR(sbig_dev), nr));
	}
	class_destroy(sbig_class);
	unregister_chrdev_region(sbig_dev, SBIG_NO);
}

module_init(sbig_init_module);
module_exit(sbig_cleanup_module);

// device_create() and device_destroy() require this
MODULE_LICENSE("GPL");

// N.B.  This *file* is a new work, and is declared GPL by its author,
// who is not affiliated with the copyright holders of the original SBIG
// parallel port Linux driver.
//
// Some of the SBIG files linked with it were brought over from Windows
// and do not contain a license declaration, so we assume "proprietary",
// noting that they were publicly distributed with SBIG's original linux
// driver for many years.
//
// Is this a problem?  This driver would seem to be analogous to the
// AFS example cited in an email discussing "gray areas" by Linus Torvalds:
//   https://lkml.org/lkml/2003/12/3/228
//
// Hopefully declaring the module license to be GPL doesn't offend anyone.
// I'm not sure how one can do otherwise and get anything to work!
