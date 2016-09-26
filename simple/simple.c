/*
 * Simple - REALLY simple memory mapping demonstration.
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: simple.c,v 1.12 2005/01/31 16:15:31 rubini Exp $
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>

#include <linux/device.h>

static int simple_major = 0;
module_param(simple_major, int, 0);
MODULE_AUTHOR("Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");
static int *mybuf, *ptr;
static int *mybuf_area = NULL;

//#define LEN (10*1024)

static int ldd_simple_open (struct inode *inode, struct file *filp)
{
    printk("--- ldd_simple_open()\n");
    return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
    printk("--- ldd_simple_release()\n");
    return 0;
}

void simple_vma_open(struct vm_area_struct *vma)
{
    printk( "Simple VMA open, virt %lx, phys %lx\n",
            vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
    printk("simple_vma_open: mybuf: %d\n", *ptr);
}

void simple_vma_close(struct vm_area_struct *vma)
{
        printk("--- simple_vma_open()\n");
    printk( "Simple VMA close.\n");
}

static struct vm_operations_struct simple_remap_vm_ops = {
    .open =  simple_vma_open,
    .close = simple_vma_close,
};

static int simple_remap_mmap(struct file *filp, struct vm_area_struct *vma)
{
    printk("ENTER simple_remap_mmap\n");
    printk("simple_remap_mmap: mybuf: %ld, size %d\n", *mybuf, vma->vm_end - vma->vm_start);
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        virt_to_phys((void *)mybuf) >> PAGE_SHIFT,
                        //(int)mybuf >> PAGE_SHIFT, /*0x90000000 >> PAGE_SHIFT,  vma->vm_pgoff,*/ 
                        /*0x100,*/ vma->vm_end - vma->vm_start, 
                        vma->vm_page_prot) ) {
        return -EAGAIN;
    }
    
    vma->vm_ops = &simple_remap_vm_ops;
    simple_vma_open(vma);
    return 0;
}

static void simple_setup_cdev(struct cdev *dev, int minor,
        struct file_operations *fops)
{
    int err, devno = MKDEV(simple_major, minor);
    
    cdev_init(dev, fops);
    dev->owner = THIS_MODULE;
    dev->ops = fops;
    err = cdev_add (dev, devno, 1);
    /* Fail gracefully if need be */
    if (err)
        printk (KERN_NOTICE "Error %d adding simple%d", err, minor);
}


/*
 * Our various sub-devices.
 */
/* Device 0 uses remap_pfn_range */
static struct file_operations simple_remap_ops = {
    .owner   = THIS_MODULE,
    .open    = ldd_simple_open,
    .release = simple_release,
    .mmap    = simple_remap_mmap,
};

#define MAX_SIMPLE_DEV 1

#if 0
static struct file_operations *simple_fops[MAX_SIMPLE_DEV] = {
    &simple_remap_ops,
    &simple_fault_ops,
};
#endif

/*
 * We export two simple devices.  There's no need for us to maintain any
 * special housekeeping info, so we just deal with raw cdevs.
 */
static struct cdev SimpleDevs[MAX_SIMPLE_DEV];

/*
 * Module housekeeping.
 */
static int simple_init(void)
{
    int result;
    dev_t dev = MKDEV(simple_major, 0);
    mybuf=kzalloc(PAGE_SIZE, GFP_KERNEL);
    printk("simple_init: PAGE_SIZE %d\n", PAGE_SIZE);
    //mybuf=kzalloc(LEN*2*PAGE_SIZE, GFP_KERNEL);
    //mybuf_area=(int *)(((unsigned long)mybuf + PAGE_SIZE -1) & PAGE_MASK);
    
    if(mybuf <= 0)
            printk("ERROR mem alloc");

    mybuf[0]=10;
    mybuf[1]=20;

    ptr=mybuf;
    printk("mybuf1: %d\n", *ptr);
    ptr++;
    printk("mybuf: %d\n", *ptr);
    //printk("mybuf: 0x%p \n", mybuf_area );

    
    printk("--- simple init 3\n");
    /* Figure out our device number. */
    if (simple_major)
        result = register_chrdev_region(dev, 2, "simple");
    else {
        result = alloc_chrdev_region(&dev, 0, 2, "simple");
        simple_major = MAJOR(dev);
    }
    if (result < 0) {
        printk(KERN_WARNING "simple: unable to get major %d\n", simple_major);
        return result;
    }
    if (simple_major == 0)
        simple_major = result;

    /* Now set up two cdevs. */
    simple_setup_cdev(SimpleDevs, 0, &simple_remap_ops);
    return 0;
}


static void simple_cleanup(void)
{
    cdev_del(SimpleDevs);
    unregister_chrdev_region(MKDEV(simple_major, 0), 2);
    kfree(mybuf);
}


module_init(simple_init);
module_exit(simple_cleanup);


