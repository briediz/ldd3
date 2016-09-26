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
MODULE_AUTHOR("briediz");
MODULE_LICENSE("Dual BSD/GPL");
static int *mybuf, *ptr;
static int *mybuf_area = NULL;

static int ldd_simple_open (struct inode *inode, struct file *filp)
{
    printk("ldd_simple_open()\n");
    return 0;
}

static int simple_release(struct inode *inode, struct file *filp)
{
    printk("ldd_simple_release()\n");
    return 0;
}

void simple_vma_open(struct vm_area_struct *vma)
{
    printk( "Simple VMA open, virt %lx, phys %lx\n", vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void simple_vma_close(struct vm_area_struct *vma)
{
    printk("--- simple_vma_close()\n");
}

static struct vm_operations_struct simple_remap_vm_ops = {
    .open =  simple_vma_open,
    .close = simple_vma_close,
};

static int simple_remap_mmap(struct file *filp, struct vm_area_struct *vma)
{
    printk("simple_remap_mmap: mybuf: %ld, size %d\n", *mybuf, vma->vm_end - vma->vm_start);
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        virt_to_phys((void *)mybuf) >> PAGE_SHIFT,
                        vma->vm_end - vma->vm_start, 
                        vma->vm_page_prot) ) {
        return -EAGAIN;
    }
    
    //vma->vm_ops = &simple_remap_vm_ops;
    //simple_vma_open(vma);
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
    if (err)
        printk (KERN_NOTICE "Error %d adding simple%d", err, minor);
}

static struct file_operations simple_remap_ops = {
    .owner   = THIS_MODULE,
    .open    = ldd_simple_open,
    .release = simple_release,
    .mmap    = simple_remap_mmap,
};

#define MAX_SIMPLE_DEV 1

static struct cdev SimpleDevs[MAX_SIMPLE_DEV];

static int simple_init(void)
{
    int result;
    dev_t dev = MKDEV(simple_major, 0);
    mybuf=kzalloc(PAGE_SIZE, GFP_KERNEL);    
    if(mybuf <= 0){
            printk("ERROR mem alloc");
    }

    printk("simple_init: PAGE_SIZE %d\n", PAGE_SIZE);

    mybuf[0]=12;
    mybuf[1]=34;

    ptr=mybuf;
    
    printk("mybuf: %d\n", *ptr);
    ptr++;
    printk("mybuf: %d\n", *ptr);
    
    if (simple_major){
        result = register_chrdev_region(dev, 2, "simple");
    } else {
        result = alloc_chrdev_region(&dev, 0, 2, "simple");
        simple_major = MAJOR(dev);
    }
    
    if (result < 0) {
        printk(KERN_WARNING "simple: unable to get major %d\n", simple_major);
        return result;
    }
    
    if (simple_major == 0){
        simple_major = result;
    }
    
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


