/* $Id: xio3d.c 100580 2009-09-08 14:42:41Z mdr $ */
/*
 * xio3d.c: Xiotech Mag 3D kernel support module
 *
 * Copyright 2004-2008 Xiotech Corporation
 *
 * Mark Rustad (Mark_Rustad@Xiotech.com)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * This driver provides kernel-level support for the Xiotech Mag 3D
 * processes.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/smp_lock.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/compat.h>
#include <linux/hugetlb.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#include "xio3d.h"

/*
 * Module and version information
 */

#define XIO3D_REVISION	"$Revision: 100580 $"
#define XIO3D_VERSION	(&(XIO3D_REVISION[11]))
#define XIO3D_VERLENGTH	(sizeof(XIO3D_REVISION) - 14)

#define XIO3D_MAJOR	252

#define	MEMBLK_SIZE	HPAGE_SIZE
#ifndef	phys_to_page
#define	phys_to_page(phys) pfn_to_page((phys) >> PAGE_SHIFT)
#endif /* phys_to_page */

#define PFX	"xio3d: "

#ifdef	__GFP_NO_COMP
#define	XIO_MEM_GFP	(GFP_KERNEL | __GFP_NO_COMP)
#else
#define	XIO_MEM_GFP	(GFP_KERNEL)
#endif	/* __GFP_NO_COMP */

#ifndef	PCI_DEVICE_ID_INTEL_SMCH
#define	PCI_DEVICE_ID_INTEL_SMCH	PCI_DEVICE_ID_INTEL_E7520_MCH
#endif	/* !PCI_DEVICE_ID_INTEL_SMCH */


/*
 * Define default segment sizes
 */

#define XIO3D_SEG0_SIZE		(128*1024*1024)
#define XIO3D_SEG1_SIZE		(128*1024*1024)
#define XIO3D_SEG2_SIZE		(128*1024*1024)
#define XIO3D_SEG3_SIZE		(0)

/*
 * Event occurrence counters.
 */

#define XIO3D_CNT_INT		0	/* Interrupt occurred */
#define XIO3D_CNT_READ		1	/* Dev_read() called */
#define XIO3D_CNT_WRITE		2	/* Dev_write() called */
#define XIO3D_CNT_REGINT	3	/* Ioctl(REGINT) called */
#define XIO3D_CNT_MASKEVT	4	/* Ioctl(MASKEVT) called */
#define XIO3D_CNT_REGEVT	5	/* Ioctl(REGEVT) called */
#define XIO3D_CNT_SENDEVT	6	/* Ioctl(SENDEVT) called */
#define XIO3D_CNT_GETINF	7	/* Ioctl(GETINF) called */

#define XIO3D_CNT_WAIT		8	/* Semaphore wait */
#define XIO3D_CNT_SIGNAL	9	/* Sempahore signal */
#define	XIO3D_CNT_POLL		10	/* Count of calls to xio3d_dev_poll */

#define XIO3D_CNT_INTBASE	(32)	/* 32 Interrupt/Event counters */

#define XIO3D_CNT_MAX		128	/* Max counter value */

unsigned long	xio3d_counter[XIO3D_CNT_MAX];	/* Counters */

/*
 * Inline procedures to bump counters.
 */

#if 1
#define bump(x) xio3d_counter[x]++
#else
#define bump(x)
#endif

#if 1
inline void bump_mask(int mask) {
	unsigned int	m = mask;
	int	i;
	for (i = 0; m != 0; m >>= 1, i++) {
		while ((m & 0xf) == 0) {
			m >>= 4;
			i += 4;
		}
		if ((m & 0x1) != 0)
			xio3d_counter[XIO3D_CNT_INTBASE + i]++;
	}
}
#else
#define bump_mask(x)
#endif

/*
 * Represents an IRQ registered by the user program.
 */

struct irq_state
{
	s16		irq;		/* System IRQ number */
	u8		index;		/* Event bit index */
	u8		enabled;	/* Interrupt enabled */
	int		event_bit;	/* Corresponding event bit */
	struct pci_dev	*dev;		/* PCI device */
	struct irq_reg	*ir;		/* Pointer to enclosing structure */
};

/*
 * Represents the set of interrupt and event data associated
 * with a particular open() of the xio3d device.
 */

#define	DEVNAME_SIZE	sizeof(((struct task_struct *)0)->comm)
struct irq_reg
{
	struct xio3d_drvinfo	info;	/* User task report structure */
	struct irq_state	irqs[XIO3D_MAX_IRQS];	/* Associated irqs */
	wait_queue_head_t	wqueue;	/* Wait queue for this fd */
	char	devname[DEVNAME_SIZE];	/* Device name */
};

/*
 * The following array keeps track of all open irq_reg structures,
 * one of which is created for each device open. This code is
 * present only to help with debugging.
 */

#define XIO3D_OPEN_MAX	10		/* Max number of concurrent opens */

struct irq_reg	*xio3d_info[XIO3D_OPEN_MAX]; /* Pointers to open structures */


// #define XIODEBUG

#ifdef	XIODEBUG
#define	dprintk	printk
#else
#define	dprintk(x...)	do { } while (0)
#endif	/* XIODEBUG */

/* Describes a block of allocated kernel memory */

struct memblk {
	struct memblk	*next;	/* Next block (sequential by addr) */
	dma64_addr_t	dma;	/* PCI/DMA/Phys Address */
	struct page	*page;	/* struct page for this huge page */
};

static struct memblk	*blklist;
static struct memblk	*memblks;
static struct memblk	*blks;
static int	nmegs;
static int	blknum;

static struct xio3d_drvinfo	xio3d_drvinfo = {
	.id		= XIO3D_ID,
	.mem_regions[0]	= {0, XIO3D_SEG0_SIZE},
	.mem_regions[1]	= {0, XIO3D_SEG1_SIZE},
	.mem_regions[2]	= {0, XIO3D_SEG2_SIZE},
	.mem_regions[3]	= {0, XIO3D_SEG3_SIZE},
	.info_region	= {XIO3D_INFO_OFFSET, XIO3D_INFO_SIZE},
};

static spinlock_t __cacheline_aligned_in_smp	irqlock;

static struct irq_reg	*event[XIO3D_EVT_MAX - XIO3D_EVT_BASE];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
#define	IRQ_HANDLER(name, irq, ptr) irqreturn_t name(int irq, void *ptr)
#else
#define	IRQ_HANDLER(name, irq, ptr) irqreturn_t name(int irq, void *ptr, struct pt_regs *regs)
#endif

/**
 * irq_handler - Generic interrupt handler for any device registered.
 * @irq: irq number
 * @vp: pointer to struct irq_state for this interrupt
 *
 * This function is called for all interrupts that any client of this
 * module has registered to receive.
 */

static IRQ_HANDLER(irq_handler, irq, vp)
{
	struct irq_state	*is = vp;
	struct irq_reg	*ir;
	unsigned long	flags;
	int	enabled;
	int	ebit;

	spin_lock_irqsave(&irqlock, flags);

	ir = is->ir;
	ebit = is->event_bit;

	enabled = is->enabled;
	is->enabled = 0;
	disable_irq_nosync(irq);

	if ((ir->info.mask & ebit) != 0 &&
		(ir->info.mask & ir->info.active) == 0) {
		ir->info.active |= ebit;
		bump(XIO3D_CNT_SIGNAL);
		wake_up_interruptible(&ir->wqueue);
	} else
		ir->info.active |= ebit;

	spin_unlock_irqrestore(&irqlock, flags);

	bump(XIO3D_CNT_INT);
	bump(XIO3D_CNT_INTBASE + is->index);

	if (enabled == 0)
		printk(KERN_WARNING PFX "unexpected interrupt: %d\n", irq);

	return IRQ_HANDLED;
}


#define free_one_page(x) __free_page(x)


/**
 * xio3d_dev_open - /dev/xio3d0 character device open handling.
 * @inode: pointer to struct inode
 * @filp: pointer to struct file
 *
 * Handle device open calls. Initialize associated structure,
 * including the waitqueue within.
 */

static int	xio3d_dev_open(struct inode *inode, struct file *filp)
{
	struct irq_reg	*ir;
	int	i;
	unsigned long	flags;

	/*
	 * Allocate a page of memory to keep track of this
	 * open instance.
	 */

	if (XIO3D_INFO_SIZE > PAGE_SIZE) {
		void XIO3D_INFO_SIZE_LARGER_THAN_PAGE_SIZE(void);
		XIO3D_INFO_SIZE_LARGER_THAN_PAGE_SIZE();
	}

	ir = (struct irq_reg *)get_zeroed_page(GFP_KERNEL);
	if (ir == NULL)
		return -ENOMEM;

	ir->info = xio3d_drvinfo;
	init_waitqueue_head(&ir->wqueue);

	strncpy(ir->devname, current->comm, sizeof(ir->devname) - 1);

	for (i = 0; i < XIO3D_MAX_IRQS; ++i)
		ir->irqs[i].ir = ir;

	ir->info.info_region.phys = virt_to_phys(ir);
	ir->info.info_region.kmem = (ulong) ir;

	filp->private_data = ir;

	spin_lock_irqsave(&irqlock, flags);

	ir->info.open_index = -1;

	for (i = 0; i < XIO3D_OPEN_MAX; i++) {
		if (xio3d_info[i] == 0) {
			xio3d_info[i] = ir;
			ir->info.open_index = i;
			break;
		}
	}

	spin_unlock_irqrestore(&irqlock, flags);

	nonseekable_open(inode, filp);
	return 0;
}


/**
 * xio3d_dev_read - /dev/xio3d0 read processing.
 * @filp: pointer to struct file
 * @buf: pointer to location to receive event mask
 * @size: size of buffer to receive event mask
 * @offp: current file location
 *
 * Receive unmasked events, blocking if there are none.
 */

static ssize_t	xio3d_dev_read(struct file *filp, char *buf, size_t size,
				loff_t *offp)
{
	unsigned int	ints;
	unsigned long	flags;
	struct irq_reg	*ir = filp->private_data;
	int	err;

	bump(XIO3D_CNT_READ);

	if (size != sizeof ints)
		return -EINVAL;

	for (;;) {
		/*
		 * Read and clear all unmasked events, but leave
		 * any active interrupts still active.
		 */

		spin_lock_irqsave(&irqlock, flags);

		ints = ir->info.mask & ir->info.active;

		ir->info.active &= (1 << XIO3D_MAX_IRQS) - 1;

		/*
		 * If no events or interrupts were pending,
		 * wait until we are signalled to take another
		 * look.
		 */

		if (ints)
			break;

		spin_unlock_irqrestore(&irqlock, flags);

		/* Wait for an interrupt or an event. */

		bump(XIO3D_CNT_WAIT);

		if (wait_event_interruptible(ir->wqueue,
			ir->info.mask & ir->info.active))
			return -ERESTARTSYS;
        }
	spin_unlock_irqrestore(&irqlock, flags);

	bump_mask(ints);

	/*
	 * Copy the mask of currently pending interrupts to
	 * the user program.
	 */

	err = copy_to_user(buf, &ints, sizeof ints);
	if (err)
		return err;

	return sizeof ints;
}


/**
 * xio3d_dev_write - /dev/xio3d0 write processing
 * @filp: pointer to struct file
 * @buf: pointer to buffer of data
 * @size: size of buffer pointed to by @buf
 * @offp: current file offset
 *
 * The user program may re-enable a set of previously disabled
 * interrupts and/or a set of active events by writing the set
 * of desired interrupts & events to the xio device.
 */

static ssize_t	xio3d_dev_write(struct file *filp, const char *buf,
				size_t size, loff_t *offp)
{
	unsigned int	ints;
	int	ret;
	int	i;
	unsigned long	flags;
	struct irq_reg	*ir = filp->private_data;

	bump(XIO3D_CNT_WRITE);

	/* Copy the write data, which must be exactly sizeof(int). */

	if (size != sizeof ints)
		return -ENXIO;

	ret = copy_from_user(&ints, buf, sizeof(ints));
	if (ret != 0)
		return ret;

	/* Re-enable the specified set of interrupt and event bits. */

	spin_lock_irqsave(&irqlock, flags);
	ints &= ir->info.active;
	ir->info.active &= ~ints;

	for (i = 0; i < XIO3D_MAX_IRQS; ++i) {
		struct irq_state *is;

		if ((ints & (1 << i)) == 0)
			continue;

		is = &ir->irqs[i];
		if (is->irq == 0 && is->dev == NULL)
			continue;

		bump(XIO3D_CNT_INTBASE + i);
		is->enabled = 1;
		enable_irq(is->irq);
	}

	spin_unlock_irqrestore(&irqlock, flags);

	bump_mask(ints);

	return sizeof ints;
}


/**
 * regint - Handle XIO3D_REGINT ioctl
 * @filp: pointer to struct file
 * @req: irq number being requested
 *
 * This function is called to process an %XIO3D_REGINT ioctl.
 * This function registers the function irq_handler() to handle
 * the interrupt and arranges for that function to be passed
 * the address of of a struct irq_state structure, which holds
 * a pointer to the struct irq_reg, which holds the semaphore
 * to awaken the process.
 *
 * The ioctl caller is returned the event number corresponding
 * to this irq. The event number indicates which bit in an
 * event mask corresponds to this irq. Event masks are used for
 * masking events (see the XIO3D_MASKINT ioctl) and are returned
 * by the read() call.
 *
 * This function automatically adjusts the current event mask to
 * include the interrupt just registered.
 */

static int	regint(struct file *filp, unsigned long req)
{
	struct irq_reg	*ir;
	int	irq;
	int	ret;
	int	i;
	unsigned long	flags;

	bump(XIO3D_CNT_REGINT);

	/* Get the interrupt number to register */

	ret = copy_from_user(&irq, (int *)req, sizeof irq);
	if (ret)
		return ret;
	if (irq <= 0)
		return -EINVAL;
	ir = filp->private_data;
	if (ir == 0) {
		printk(KERN_ERR PFX "ir==0?\n");
		return -EINVAL;
	}

	/*
	 * Find an unused IRQ entry in the table, and enter the
	 * IRQ number there.
	 */
        
	spin_lock_irqsave(&irqlock, flags);

	for (i = 0; i < XIO3D_MAX_IRQS; ++i) {
		if (ir->irqs[i].irq == 0)
			break;
	}

	if (i >= XIO3D_MAX_IRQS) {
		spin_unlock_irqrestore(&irqlock, flags);
		return -ENOMEM;
	}

	ir->irqs[i].irq = irq;
	ir->irqs[i].enabled = 1;
	ir->irqs[i].index = i;
	ir->irqs[i].dev = 0;
	ir->irqs[i].event_bit = (1 << i);

	ir->info.reg  |= (1 << i);
	ir->info.mask |= (1 << i);

	spin_unlock_irqrestore(&irqlock, flags);

	/* Grab the requested system interrupt */

	ret = request_irq(irq, irq_handler, 0, ir->devname, &ir->irqs[i]);
	if (ret < 0) {
		spin_lock_irqsave(&irqlock, flags);
		ir->irqs[i].irq = 0;
		ir->irqs[i].enabled = 0;
		ir->irqs[i].event_bit = 0;

		ir->info.reg  &= ~(1 << i);
		ir->info.mask &= ~(1 << i);
		spin_unlock_irqrestore(&irqlock, flags);
		return ret;
	}

	/* Return the table entry number to the user */

	ret = copy_to_user((int *)req, &i, sizeof i);
	return ret;
}


/**
 * pciint - Handle XIO3D_PCIINT ioctl
 * @filp: pointer to struct file
 * @req: pointer PCI device (busdevfn) being requested
 *
 * This function is called to process an %XIO3D_PCIINT ioctl.
 * The function enables the device and registers the function
 * irq_handler() to handle the interrupt and arranges for that
 * function to be passed the address of of a struct irq_state
 * structure, which holds a pointer to the struct irq_reg, which
 * holds the semaphore to awaken the process.
 *
 * The ioctl caller is returned the event number corresponding
 * to this irq. The event number indicates which bit in an
 * event mask corresponds to this irq. Event masks are used for
 * masking events (see the XIO3D_MASKINT ioctl) and are returned
 * by the read() call.
 *
 * This function automatically adjusts the current event mask to
 * include the interrupt just registered.
 */

static int	pciint(struct file *filp, unsigned long req)
{
	struct irq_reg	*ir;
	int32_t	busdevfn;
	struct pci_dev	*dev;
	int	busnr;
	unsigned int devfn;
	int	ret;
	int	i;
	unsigned long	flags;

	bump(XIO3D_CNT_REGINT);

	/* Get the interrupt number to register */

	ret = copy_from_user(&busdevfn, (int32_t *)req, sizeof busdevfn);
	if (ret)
		return ret;
	if (busdevfn <= 0)
		return -EINVAL;
	ir = filp->private_data;
	if (ir == 0) {
		printk(KERN_ERR PFX "ir==0?\n");
		return -EINVAL;
	}

	printk(KERN_ERR PFX "pciint busdevfn=%04x\n", busdevfn);

	busnr = busdevfn >> 8;
	devfn = busdevfn & 0xff;
	dev = NULL;
	while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != NULL) {
		if (dev->bus->number != busnr)
			continue;
		if (dev->devfn != devfn)
			continue;
		break;
	}
	if (dev == NULL) {
		printk(KERN_ERR PFX "pci device %04x not found\n", busdevfn);
		ret = -EINVAL;
		goto out;
	}
	printk(KERN_ERR PFX "bus=%d, devfn=%02x\n", busnr, devfn);
	ret = pci_enable_device(dev);
	if (ret) {
		printk(KERN_ERR PFX "pci_enable_device returned %d for "
			"device %04x.\n", ret, busdevfn);
		goto out2;
	}
	ret = pci_request_regions(dev, "xio3d");
	if (ret) {
		printk(KERN_ERR PFX "pci_request_resources returned %d for "
			"device %04x.\n", ret, busdevfn);
		goto out2;
	}

	/*
	 * Find an unused IRQ entry in the table, and enter the
	 * IRQ number there.
	 */
 
	spin_lock_irqsave(&irqlock, flags);

	for (i = 0; i < XIO3D_MAX_IRQS; ++i)
		if (ir->irqs[i].irq == 0)
			break;

	if (i >= XIO3D_MAX_IRQS) {
		ret = -ENOMEM;
		goto out3;
	}

	ir->irqs[i].irq = dev->irq;
	ir->irqs[i].enabled = 1;
	ir->irqs[i].index = i;
	ir->irqs[i].dev = dev;
	ir->irqs[i].event_bit = (1 << i);

	ir->info.reg  |= (1 << i);
	ir->info.mask |= (1 << i);

	spin_unlock_irqrestore(&irqlock, flags);

	/* Grab the requested system interrupt */

	printk(KERN_ERR PFX "Requesting irq %d\n", dev->irq);
	ret = request_irq(dev->irq, irq_handler, 0, ir->devname, &ir->irqs[i]);
	if (ret < 0) {
		printk(KERN_ERR PFX "request_irq returned %d for %04x\n",
			ret, busdevfn);
		spin_lock_irqsave(&irqlock, flags);
		ir->irqs[i].irq = 0;
		ir->irqs[i].enabled = 0;
		ir->irqs[i].dev = 0;
		ir->irqs[i].event_bit = 0;

		ir->info.reg  &= ~(1 << i);
		ir->info.mask &= ~(1 << i);
		goto out3;
	}

	pci_set_master(dev);

	/* Return the table entry number to the user */

	ret = copy_to_user((int *)req, &i, sizeof i);
	return ret;

out3:
	spin_unlock_irqrestore(&irqlock, flags);
	pci_release_regions(dev);
out2:
	pci_dev_put(dev);
out:
	return ret;
}


/**
 * maskevt - Handle XIO3D_MASKEVT ioctl
 * @filp: pointer to struct file
 * @req: the new mask for events
 *
 * This function handles the XIO3D_MASKEVT ioctl calls. The passed-in
 * mask replaces the existing mask and the existing mask is returned
 * to the ioctl caller.
 */

static int	maskevt(struct file *filp, unsigned long req)
{
	struct irq_reg	*ir;
	unsigned int	mask;
	unsigned int	oldmask;
	unsigned long	flags;
	int	ret;

	bump(XIO3D_CNT_MASKEVT);

	ret = copy_from_user(&mask, (int *)req, sizeof mask);
	if (ret)
		return ret;
	ir = filp->private_data;
	if (unlikely(ir == 0)) {
		printk(KERN_ERR PFX "ir==0?\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&irqlock, flags);

	oldmask = ir->info.mask;
	ir->info.mask = mask & ir->info.reg;

	spin_unlock_irqrestore(&irqlock, flags);

	ret = copy_to_user((int *)req, &oldmask, sizeof oldmask);
	return ret;
}


/**
 * regevt - Handle XIO3D_REGEVT ioctl call
 * @filp: pointer to struct file
 * @evt: event number to register for
 *
 * This function handles XIO3D_REGEVT ioctl() calls. The event
 * number specified in @evt is registered to this process. Only
 * a single process can register to receive notification of any
 * particular event.
 *
 * Registering for event notification automatically adds the event
 * the the currently enabled event mask.
 */

static int	regevt(struct file *filp, int evt)
{
	struct irq_reg	*ir;
	int	evtix;
	unsigned long	flags;

	bump(XIO3D_CNT_REGEVT);

	if (evt < XIO3D_EVT_BASE || evt >= XIO3D_EVT_MAX) {
		printk(KERN_ERR PFX "evt=%d\n", evt);
		return -EINVAL;
	}

	ir = filp->private_data;
	if (ir == 0) {
		printk(KERN_ERR PFX "ir==0?\n");
		return -EINVAL;
	}

	evtix = evt - XIO3D_EVT_BASE;

	spin_lock_irqsave(&irqlock, flags);

	if (event[evtix]) {
		spin_unlock_irqrestore(&irqlock, flags);
		return -EADDRINUSE;
	}

	event[evtix] = ir;
	ir->info.reg  |= 1 << evt;
	ir->info.mask |= 1 << evt;

	spin_unlock_irqrestore(&irqlock, flags);

	return 0;
}


/**
 * sendevt - Handle XIO3D_SENDEVT ioctl
 * @filp: pointer to struct file
 * @evt: number of event to send
 *
 * This function handles XIO3D_SENDEVT ioctl() calls. If no process
 * has registered for the event, the error code -%EHOSTUNREACH is
 * returned.
 */

static int	sendevt(struct file *filp, int evt)
{
	struct irq_reg	*ir;
	struct irq_reg	*irt;
	unsigned long	flags;
	int	evtix;

	bump(XIO3D_CNT_SENDEVT);
	bump(XIO3D_CNT_INTBASE + evt);

	if (unlikely(evt < XIO3D_EVT_BASE || evt >= XIO3D_EVT_MAX)) {
		printk(KERN_ERR PFX "sendevt=%d\n", evt);
		return -EINVAL;
	}

	ir = filp->private_data;
	if (unlikely(ir == 0)) {
		printk(KERN_ERR PFX "ir==0?\n");
		return -EINVAL;
	}

	evtix = evt - XIO3D_EVT_BASE;
	spin_lock_irqsave(&irqlock, flags);

	irt = event[evtix];
	if (unlikely(irt == 0)) {
		spin_unlock_irqrestore(&irqlock, flags);
		return -EHOSTUNREACH;
	}

        if ((irt->info.mask & (1 << evt)) != 0 &&
            (irt->info.mask & irt->info.active) == 0) {
		irt->info.active |= (1 << evt);
		bump(XIO3D_CNT_SIGNAL);
		wake_up_interruptible(&irt->wqueue);
	} else
		irt->info.active |= (1 << evt);

	spin_unlock_irqrestore(&irqlock, flags);

	return 0;
}



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)
#define	DEFAULT_POLICY
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
#define	DEFAULT_POLICY	NULL, 0
#else
#define	DEFAULT_POLICY
#endif

/**
 * alloc_insert - Insert a struct memblk into list in address order.
 * @memblks: Address of an array of struct memblks
 * @nblks: Number of struct memblks in memblks
 *
 * Globals:
 *	blknum  Number of blocks allocated in memblks
 *	blklist Pointer to ordered list
 *
 * Returns:
 *	Pointer to struct page for block allocated or 0 if failure of any kind.
 */

static struct page *
alloc_insert(struct memblk *memblks, int nblks)
{
	struct memblk	*p;
	struct memblk	*n;
	struct page	*page;
	dma64_addr_t	addr;

	/* Allocate a new memory block structure */

	if (blknum >= nblks)    /* If all structures already used */
		return 0;
	n = &memblks[blknum];
	++blknum;

	/* Allocate memory block */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
	page = get_huge_page(DEFAULT_POLICY);
#else
	page = alloc_huge_page(DEFAULT_POLICY);
#endif
	if (page == 0)
		return 0;
	n->page = page;
	n->dma = page_to_phys(page);

	/* Insert block into list in order */

	addr = n->dma;
	p = blklist;
	if (p == 0 || addr < p->dma) {
		n->next = p;
		blklist = n;
		return page;
	}
	for (;;) {
		if (p->next == 0) {
			p->next = n;
			return page;
		}
		if (addr < p->next->dma) {
			n->next = p->next;
			p->next = n;
			return page;
		}
		p = p->next;
	}
}


/**
 * find_contig - This function checks for a contiguous block nmeg long.
 * @p: Pointer to memory block list to search for contiguous blocks
 * @nmeg: Number of megabytes of contiguous memory needed
 *
 * Globals:
 *	blklist Ordered list of memory blocks to search.
 *
 * Returns:
 *	Address of blklist entry where contiguous block begins, or
 *		0 if the contiguous block was not found
 */

static struct memblk *
find_contig(struct memblk *p, unsigned long nmeg)
{
	int		contigmax, contig;
	struct memblk	*blkmax, *blk;
	dma64_addr_t	dmaaddr;
	int		discontigs;

	contigmax = 0;
	blkmax = 0;
	discontigs = 0;
	if (p == 0)
		return 0;

	contig = 1;
	dmaaddr = p->dma;
	blk = p;
	for (p = p->next; p != 0; p = p->next) {
		dmaaddr += MEMBLK_SIZE;
		if (dmaaddr != p->dma) {
			++discontigs;
			printk(KERN_ERR PFX "discontigs=%d, contig=%d\n",
				discontigs, contig);
			if (contig > contigmax) {
				contigmax = contig;
				blkmax = blk;
			}
			dmaaddr = p->dma;
			contig = 1;
			blk = p;
		} else {
			++contig;
			if (contig == nmeg)
				return blk;
		}
	}

	/* Contiguous memory not found this time */
	printk(KERN_ERR PFX "discontigs=%d, contigmax=%d, contig=%d\n",
		discontigs, contigmax, contig);

	return 0;
}


/**
 * get_contig - Allocate contiguous memory
 * @size: size of area to allocate, will be rounded up to next megabyte
 *
 * Returns:
 *	Pointer to struct page for first page.
 */

static struct memblk *
get_contig(unsigned long size)
{
	int	i;
	int	nblks;
	struct page	*mem;

	if (size <= 0) {
		printk(KERN_ERR PFX "get_contig called with size <= 0\n");
		return 0;
	}
	if (memblks) {
		printk(KERN_ERR PFX "memblks already set!\n");
		return 0;
	}
	nmegs = ((size + (MEMBLK_SIZE - 1)) & ~(MEMBLK_SIZE - 1)) / MEMBLK_SIZE;
	nblks = 10 * nmegs;
	memblks = kzalloc(sizeof(struct memblk) * nblks, XIO_MEM_GFP);
	if (!memblks) {
		printk(KERN_ERR PFX "kmalloc failure\n");
		return 0;
	}
	blknum = 0;
	blklist = 0;
	mem = 0;
	for (i = 0; i < nmegs; ++i) {
		mem = alloc_insert(memblks, nblks);
		if (!mem) {
			printk(KERN_ERR PFX "initial alloc failed\n");
			break;
		}
	}
	while (mem)
		mem = alloc_insert(memblks, nblks);

	return find_contig(blklist, nmegs);
}


/**
 * free_extra - free unused blocks
 */

static void
free_extra(void)
{
	int	i;
	int	nfree;
	struct memblk	*p;

	/* At this point, free what we won't use */

	if (!memblks) {
		printk(KERN_ERR PFX "No memblks to free\n");
		return;
	}
	nfree = 0;
	i = 0;
	for (p = blklist; p != 0; p = p->next) {
		if (p == blks)
			i = 1;
		else if (i != 0)
			++i;
		if (i == 0 || i > nmegs) {
			put_page(p->page);
			++nfree;
		}
	}

	kfree(memblks);

	memblks = 0;
}


/**
 * alloc_shm - Allocate shared memory blocks.
 * @ix: shared memory region to allocate
 * Returns:
 *	0 if success, non-zero error code if failure.
 */

static int alloc_shm(int ix)
{
	struct xio3d_mem_region *mr;
	struct memblk	*mb;

	if (ix < 0 || ix >= XIO3D_NSHM_MAX) {
		printk(KERN_ERR PFX "Bad region index %d\n", ix);
		return -EINVAL;
	}
	mr = &xio3d_drvinfo.mem_regions[ix];
	if (!mr->size) {
		printk(KERN_ERR PFX "Region %d size=0!\n", ix);
		return -ENOMEM;
	}
	mb = get_contig(mr->size);
	if (!mb)
		return -ENOMEM;

	mr->phys = page_to_phys(mb->page);
	mr->kmem = (unsigned long)mb->page;
	printk(KERN_ERR PFX "region %d@%08llx, size=%08llx\n",
		ix, mr->phys, mr->size);

	return 0;
}


/*
 * shmalloc - Allocate contiguous huge pages to shared memory file.
 * @filp: pointer to struct file
 * @req: pointer to struct xio3d_shmalloc
 *
 * This function handles XIO3D_SHMALLOC ioctls. This ioctl is used to
 * assign contiguous pages to the file passed in the xio3d_shmalloc
 * structure. The allocation that does this should allocate the larger
 * segments first.
 */

static int	shmalloc(struct file *filp, unsigned long req)
{
	uint32_t	len;
	uint32_t	ix;
	uint64_t	vaddr;
	dma64_addr_t	paddr;
	struct page	*mem;
	struct page	**pages;
	struct xio3d_shmalloc	shmreq;
	struct xio3d_mem_region	*mr;
	int	rc;
	int	ret;

	rc = copy_from_user(&shmreq, (void *)req, sizeof(shmreq));
	if (rc) {
		printk(KERN_ERR PFX "Accessing %08lx failed with %d\n",
		    req, rc);
		return -EFAULT;
	}
	len = shmreq.len;
	if (len & ~HPAGE_MASK) {
		printk(KERN_ERR PFX "len not huge-aligned, %08x\n", len);
		return -EINVAL;
	}
	vaddr = shmreq.vaddr;
	ix = shmreq.rgn;
	if (ix > XIO3D_NSHM_MAX) {
		printk(KERN_ERR PFX "rgn out of range - %d\n", ix);
		return -EINVAL;
	}
	mr = &xio3d_drvinfo.mem_regions[ix];
	if (mr->kmem) {
		printk(KERN_ERR PFX "rgn%d already allocated\n", ix);
		return -EINVAL;
	}
	ret = alloc_shm(ix);
	if (ret) {
		printk(KERN_ERR PFX "alloc_shm(%d) returned %d\n", ix, ret);
		goto out;
	}
	paddr = mr->phys;
	if (!paddr) {
		printk(KERN_ERR PFX "No physical address for region %d\n", ix);
		ret = -EINVAL;
		goto out;
	}
	mem = phys_to_page(paddr);
	if (!mem) {
		printk(KERN_ERR PFX "No mem, region %d, phys=%08llx, len=%u\n",
			ix, paddr, len);
		ret = -ENOMEM;
		goto out;
	}
	pages = kmalloc(sizeof(*pages) * (HPAGE_SIZE / PAGE_SIZE), XIO_MEM_GFP);
	if (!pages) {
		printk(KERN_ERR PFX "Failed to allocate pages array\n");
		ret = -ENOMEM;
		goto out;
	}
	ret = 0;
	while (len && !ret) {
		int	i;

		put_page(mem);		/* Give up page to grab it below */
		down_read(&current->mm->mmap_sem);
		rc = get_user_pages(current, current->mm, (unsigned long)vaddr,
		    HPAGE_SIZE / PAGE_SIZE, 0, 0, pages, NULL);
		up_read(&current->mm->mmap_sem);
		if (rc < 0) {
			printk(KERN_ERR PFX "get_user_pages failed with %d\n",
			    rc);
			ret = rc;
			goto out2;
		}
		if (rc != HPAGE_SIZE / PAGE_SIZE) {
			printk(KERN_ERR PFX "get_user_pages returned %d "
			    "instead of %lu\n", rc, HPAGE_SIZE / PAGE_SIZE);
			ret = -ENOMEM;
		}

		for (i = 0; i < rc; ++i) {
			dma64_addr_t	phys;

			phys = page_to_phys(pages[i]);
			if (!phys) {
				printk(KERN_ERR PFX "phys=0!\n");
				ret = -EFAULT;
				continue;
			}
			if (phys != (paddr + i * PAGE_SIZE)) {
				printk(KERN_ERR PFX "Expected %08llx, "
				    "got %08llx\n",
				    paddr + i * PAGE_SIZE, phys);
				ret = -EFAULT;
				continue;
			}
		}
		paddr += HPAGE_SIZE;
		len -= HPAGE_SIZE;
		vaddr += HPAGE_SIZE;
		mem += HPAGE_SIZE / PAGE_SIZE;
	}
	if (ret) {
		mr->phys = 0;	/* Clear physical address in error case */
		printk(KERN_ERR PFX "Region %d physical address cleared\n",
		    ix);
	}

out2:
	kfree(pages);
out:
	free_extra();
	return ret;
}


/**
 * xio3d_dev_ioctl - Dispatch xio3d ioctl calls.
 * @inode: pointer to struct inode
 * @filp: pointer to struct file
 * @cmd: ioctl command code
 * @req: ioctl command argument
 *
 * This function dispatches ioctl() calls to the appropriate function
 * for processing.
 */

static int	xio3d_dev_ioctl(struct inode *inode, struct file *filp,
				unsigned int cmd, unsigned long req)
{
	int	ret;
	struct xio3d_drvinfo *ir;

	switch (cmd) {
	case XIO3D_GETINF:
		bump(XIO3D_CNT_GETINF);
		ir = filp->private_data;
		ret = copy_to_user((void *)req, ir, sizeof xio3d_drvinfo);
		break;

	case XIO3D_REGINT:
		ret = regint(filp, req);
		break;

	case XIO3D_PCIINT:
		ret = pciint(filp, req);
		break;

	case XIO3D_MASKEVT:
		ret = maskevt(filp, req);
		break;

	case XIO3D_REGEVT:
		ret = regevt(filp, req);
		break;

	case XIO3D_SENDEVT:
		ret = sendevt(filp, req);
		break;

	case XIO3D_SHMALLOC:
		ret = shmalloc(filp, req);
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}


#if defined(CONFIG_COMPAT)
/*
 * The following code provides 32-bit compatability for 32-bit
 * apps running on a 64-bit kernel. This is really only semi-
 * compatible because the xio3d_drvinfo structure contains kernel
 * addresses that simply must be 64-bit. Sorry, but at least the
 * calls are accessible with this in place.
 */

/**
 * xio3d_dev_ioctl32 - Dispatch 32-bit xio3d ioctl calls.
 * @filp: pointer to struct file
 * @cmd: ioctl command code
 * @req: ioctl command argument
 *
 * This function translates the 32-bit compatability ioctl to
 * a regular one.
 */

static long	xio3d_dev_ioctl32(struct file *filp, unsigned int cmd,
			unsigned long arg)
{
	return xio3d_dev_ioctl(filp->f_dentry->d_inode, filp, cmd,arg);
}
#endif	/* CONFIG_COMPAT */


/**
 * xio3d_dev_poll - Perform poll for xio3d driver.
 * @filp: pointer to file descriptor
 * @wait: pointer to poll_table
 *
 * This function is called in support of the poll/epoll interfaces. The
 * return value indicates the state of the driver, i.e. whether there is
 * data to read.
 */

static unsigned int	xio3d_dev_poll(struct file *filp, poll_table *wait)
{
	unsigned int	mask = 0;
	struct irq_reg	*ir = filp->private_data;

	bump(XIO3D_CNT_POLL);

	poll_wait(filp, &ir->wqueue, wait);
	if (ir->info.mask & ir->info.active)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


/**
 * xio3d_dev_release - Handle xio3d close.
 * @inode: pointer to struct inode
 * @filp: pointer to struct file
 *
 * This function is called when a close takes place. Events are
 * deregistered and the private struct irq_reg structure is
 * freed.
 */

static int	xio3d_dev_release(struct inode *inode, struct file *filp)
{
	struct irq_reg	*ir = filp->private_data;
	unsigned long	flags;
	int	i;

	if (ir) {
		for (i = 0; i < XIO3D_MAX_IRQS; ++i) {
			struct pci_dev	*dev;

			if (ir->irqs[i].irq == 0)
				continue;
			free_irq(ir->irqs[i].irq, &ir->irqs[i]);
			dev = ir->irqs[i].dev;
			if (dev) {
				pci_release_regions(dev);
				pci_dev_put(dev);
			}
			ir->irqs[i].irq = 0;
			ir->irqs[i].dev = 0;
		}

		spin_lock_irqsave(&irqlock, flags);

		for (i = 0; i < (XIO3D_EVT_MAX - XIO3D_EVT_BASE); ++i)
			if (event[i] == ir)
				event[i] = 0;

		spin_unlock_irqrestore(&irqlock, flags);
	} else
		printk(KERN_ERR PFX "private_data not set\n");

	if (ir->info.open_index >= 0)
		xio3d_info[ir->info.open_index] = 0;
	free_one_page(virt_to_page(ir));

	filp->private_data = 0;

	return 0;
}


/**
 * xio3d_vm_open - Detect opening of an mmapped area.
 * @vma: pointer to struct vm_area_struct describing area to map
 *
 * This function is called by xio3d_dev_mmap() to increment the
 * module use count when a shared memory region is mapped.
 */

static void	xio3d_vm_open(struct vm_area_struct *vma)
{
	__module_get(THIS_MODULE);
}


/**
 * xio3d_vm_close - Detect closing of an mmapped area.
 * @vma: pointer to struct vm_area_struct describing area to map
 *
 * This function is called by the kernel when a shared memory
 * region is no longer mapped. In response, this function decrements
 * the module use count.
 */

static void	xio3d_vm_close(struct vm_area_struct *vma)
{
	module_put(THIS_MODULE);
}


#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,25)
/**
 * xio3d_vm_nopage - Report bug if this is ever called
 */

static struct page *xio3d_vm_nopage(struct vm_area_struct *vma,
		unsigned long address, int *unused)
{
	BUG();
	return NULL;
}
#endif /* KERNEL_VERSION <= 2.6.25 */

/*
 * VM operations table, so that we know when a mapping is removed.
 */

static struct vm_operations_struct	xio3d_vm_ops = {
	.open	= &xio3d_vm_open,
	.close	= &xio3d_vm_close,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,25)
	.nopage = &xio3d_vm_nopage,
#endif /* KERNEL_VERSION <= 2.6.25 */
};


/**
 * xio3d_dev_mmap - Handle mmap() calls in xio3d device.
 * @filp: pointer to struct file
 * @vma: pointer to vm_area_struct describing the area being mapped
 *
 * This function handles mmap() calls made on an xio3d device. These
 * are used to map one of the shared memory regions allocated by the
 * xio3d module into process space. Each shared memory region is
 * contiguous kernel memory, suitable for DMA operation.
 */

static int	xio3d_dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	loff_t	size = (loff_t)(vma->vm_end - vma->vm_start);
	unsigned long	offset = vma->vm_pgoff << PAGE_SHIFT;

	/*
	 * Verify proper alignment to page boundaries, proper
	 * permissions and so on.
	 */

	if (offset & ~PAGE_MASK) {
		printk(KERN_ERR PFX "offset not aligned: %lu\n", offset);
		return -EINVAL;
	}
	if (size & ~PAGE_MASK) {
		printk(KERN_WARNING PFX "size not multiple of page size, "
			"rounding up\n");
		size = (size + (PAGE_MASK - 1)) & PAGE_MASK;
	}
	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		printk(KERN_ERR PFX "offset out of range\n");
		return -EINVAL;
	}
	if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) {
		printk(KERN_ERR PFX "writable mappings must be shared\n");
		return -EINVAL;
	}
	if (offset == 0) {
		printk(KERN_ERR PFX "offset==0\n");
		return -EINVAL;
	}

	/*
	 * If the requested offset and size match our info region,
	 * map the info region.
	 */

	if (xio3d_drvinfo.info_region.offset == offset) {
		void	*virt = (void *) filp->private_data;

		if (size != XIO3D_INFO_SIZE) {
			printk(KERN_ERR PFX "info region map size incorrect\n");
			return -EINVAL;
		}

		vma->vm_ops = &xio3d_vm_ops;
		vma->vm_flags &= ~(VM_WRITE | VM_MAYWRITE);
		vma->vm_flags |= (VM_LOCKED   |
				VM_DONTEXPAND |
#ifdef VM_RESERVED
				VM_RESERVED   |
#endif
#ifdef VM_ALWAYSDUMP
				VM_ALWAYSDUMP |
#endif
				VM_MAYREAD    |
				VM_MAYSHARE   );

		if (vm_insert_page(vma, vma->vm_start, virt_to_page(virt))) {
			printk(KERN_ERR PFX "vm_insert_page failed\n");
			return -ENXIO;
		}

		xio3d_vm_open(vma);

		return 0;
	}

	return -EINVAL;
}

#if !defined(CONFIG_EDAC)

static unsigned int	pci_byte(struct pci_dev *pdev, int offset)
{
	u8	value;

	pci_read_config_byte(pdev, offset, &value);
	return value & 0xFF;
}

static unsigned int	pci_word(struct pci_dev *pdev, int offset)
{
	u16	value;

	pci_read_config_word(pdev, offset, &value);
	return value;
}

static unsigned int	pci_dword(struct pci_dev *pdev, int offset)
{
	u32	value;

	pci_read_config_dword(pdev, offset, &value);
	return value;
}


struct pci_probe_matrix {
	int	vendor;		/* pci vendor id */
	int	device;		/* pci device id */
	void	(*check)(struct pci_dev *, int); /* pointer to chipset probing routine */
};


#define	HA_PFX	KERN_ALERT PFX


static void __init	pcicmdset(struct pci_dev *pdev, const char *str,
		unsigned bits, unsigned sts)
{
	u16	pcicmd;

	pcicmd = pci_word(pdev, PCI_COMMAND);
	pci_write_config_word(pdev, PCI_COMMAND, pcicmd | bits);
	pci_write_config_word(pdev, PCI_STATUS, sts);
	dprintk(HA_PFX "%s: pcicmd=%04x -> %04x\n",
		str, pcicmd, pci_word(pdev, PCI_COMMAND));
}


#define	SETUP_PCI_CMD(nm, str, bits, sts) static void __init nm(struct pci_dev *pdev, int pass) \
	{ static char __initdata nmstr[] = str; \
		pcicmdset(pdev, nmstr, bits, sts); }


static void __init	setup_mch(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "MCH before: pcicmd=%04x, status=%04x, eccdiag=%08x, "
		"rev=%02x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_dword(pdev, 0x84), pci_byte(pdev, PCI_REVISION_ID));
	pci_write_config_word(pdev, PCI_COMMAND,
		pci_word(pdev, PCI_COMMAND) | PCI_COMMAND_SERR |
			PCI_COMMAND_PARITY);
	if (pass != 0) {
		pci_write_config_dword(pdev, 0x84,
			pci_dword(pdev, 0x84) | 0x40000);
	}
	pci_write_config_word(pdev, PCI_STATUS,
		PCI_STATUS_DETECTED_PARITY | PCI_STATUS_SIG_SYSTEM_ERROR |
		PCI_STATUS_REC_TARGET_ABORT);
	dprintk(HA_PFX "MCH after : pcicmd=%04x, status=%04x, eccdiag=%08x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_dword(pdev, 0x84));
}


SETUP_PCI_CMD(setup_mch_dma, "MCH dma", PCI_COMMAND_SERR,
	PCI_STATUS_SIG_SYSTEM_ERROR)


static void __init	setup_mch_exp(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "MCH exp before: pcicmd=%04x, status=%04x, brctl=%02x, "
		"rpctl=%08x, uncerrsts=%08x, uncerrsev=%08x, rperrms=%08x, "
		"uniterrsts=%08x, errdocmd=%08x, errctl=%08x, secsts=%04x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, PCI_BRIDGE_CONTROL), pci_dword(pdev, 0x80),
		pci_dword(pdev, 0x104), pci_dword(pdev, 0x10C),
		pci_dword(pdev, 0x130), pci_dword(pdev, 0x140),
		pci_dword(pdev, 0x148), pci_dword(pdev, 0x168),
		pci_word(pdev, PCI_SEC_STATUS));
	pci_write_config_word(pdev, PCI_COMMAND,
		pci_word(pdev, PCI_COMMAND) | PCI_COMMAND_PARITY |
			PCI_COMMAND_SERR);
	if (pass != 0) {
		pci_write_config_byte(pdev, PCI_BRIDGE_CONTROL,
			pci_byte(pdev, PCI_BRIDGE_CONTROL) | PCI_BRIDGE_CTL_SERR |
			PCI_BRIDGE_CTL_PARITY);
		pci_write_config_dword(pdev, 0x80, pci_dword(pdev, 0x80) | 0x4);
		pci_write_config_dword(pdev, 0x10C, 0x16D010);
		pci_write_config_dword(pdev, 0x148, 0x7030);
		pci_write_config_dword(pdev, 0x168,
			pci_dword(pdev, 0x168) | 0x40000);
		printk(KERN_ALERT PFX "Registering for irq %d for device %s\n",
			pdev->irq, pci_name(pdev));
	}
	pci_write_config_dword(pdev, 0x130, 0x7F);
	pci_write_config_dword(pdev, 0x140, 0xFFE7);
	pci_write_config_dword(pdev, 0x104, 0x17F010);
	pci_write_config_word(pdev, PCI_SEC_STATUS, 0xF900);
	pci_write_config_word(pdev, PCI_STATUS, PCI_STATUS_SIG_SYSTEM_ERROR |
		PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT |
		PCI_STATUS_PARITY);
	dprintk(HA_PFX "MCH exp after: pcicmd=%04x, status=%04x, brctl=%02x, "
		"rpctl=%08x, uncerrsts=%08x, uncerrsev=%08x, rperrms=%08x, "
		"uniterrsts=%08x, errdocmd=%08x, errctl=%08x, secsts=%04x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, PCI_BRIDGE_CONTROL), pci_dword(pdev, 0x80),
		pci_dword(pdev, 0x104), pci_dword(pdev, 0x10C),
		pci_dword(pdev, 0x130), pci_dword(pdev, 0x140),
		pci_dword(pdev, 0x148), pci_dword(pdev, 0x168),
		pci_word(pdev, PCI_SEC_STATUS));
}


static void __init	setup_ich5_hub(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "ICH5 hub before: "
		"pcicmd=%04x, status=%04x, pcibrctl=%04x, secsts=%04x, "
		"pcierrcmd=%02x, pcierrsts=%02x, hi1_cmd=%08x, "
		"bridge_cnt=%04x, cnf=%08x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, PCI_BRIDGE_CONTROL),
		pci_word(pdev, PCI_SEC_STATUS), pci_byte(pdev, 0x90),
		pci_byte(pdev, 0x92), pci_dword(pdev, 0x40),
		pci_word(pdev, PCI_BRIDGE_CONTROL), pci_dword(pdev, 0x50));
	pci_write_config_word(pdev, PCI_COMMAND,
		pci_word(pdev, PCI_COMMAND) | PCI_COMMAND_PARITY |
			PCI_COMMAND_SERR);
	if (pass != 0) {
		pci_write_config_word(pdev, PCI_BRIDGE_CONTROL,
			pci_word(pdev, PCI_BRIDGE_CONTROL) |
				PCI_BRIDGE_CTL_PARITY | PCI_BRIDGE_CTL_SERR);
		pci_write_config_byte(pdev, 0x90, pci_byte(pdev, 0x90) | 4);
		pci_write_config_dword(pdev, 0x40,
			pci_dword(pdev, 0x40) & ~0x100000);
		pci_write_config_dword(pdev, 0x50,
			pci_dword(pdev, 0x50) | 0xc0000);
		pci_write_config_byte(pdev, 0x92, 0x5);
	}
	pci_write_config_word(pdev, PCI_SEC_STATUS, 0xF100);
	pci_write_config_word(pdev, PCI_STATUS, PCI_STATUS_DETECTED_PARITY |
		PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
		PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_SIG_TARGET_ABORT |
		PCI_STATUS_PARITY);
	dprintk(HA_PFX "ICH5 hub after: "
		"pcicmd=%04x, status=%04x, pcibrctl=%04x, secsts=%04x, "
		"pcierrcmd=%02x, pcierrsts=%02x, hi1_cmd=%08x, "
		"bridge_cnt=%04x, cnf=%08x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, PCI_BRIDGE_CONTROL),
		pci_word(pdev, PCI_SEC_STATUS), pci_byte(pdev, 0x90),
		pci_byte(pdev, 0x92), pci_dword(pdev, 0x40),
		pci_word(pdev, PCI_BRIDGE_CONTROL), pci_dword(pdev, 0x50));
}


static void __init	setup_ich5_lpc(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "ICH5 lpc before: pcicmd=%04x, status=%04x, "
		"nmi_sc=%02x, nmi_en=%02x, errcfg=%02x, errsts=%02x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, 0x61), pci_byte(pdev, 0x70),
		pci_byte(pdev, 0x88), pci_byte(pdev, 0x8A));
	pci_write_config_word(pdev, PCI_COMMAND,
		pci_word(pdev, PCI_COMMAND) | PCI_COMMAND_PARITY |
			PCI_COMMAND_SERR);
	if (pass != 0) {
		pci_write_config_byte(pdev, 0x61, pci_byte(pdev, 0x61) & ~0x0c);
		pci_write_config_byte(pdev, 0x70, pci_byte(pdev, 0x70) & ~0x7f);
		pci_write_config_byte(pdev, 0x88, pci_byte(pdev, 0x88) | 6);
		pci_write_config_byte(pdev, 0x8A, 6);
	}
	pci_write_config_word(pdev, PCI_STATUS, PCI_STATUS_DETECTED_PARITY |
		PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
		PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_SIG_TARGET_ABORT |
		PCI_STATUS_PARITY);
	dprintk(HA_PFX "ICH5 lpc after: pcicmd=%04x, status=%04x, nmi_sc=%02x, "
		"nmi_en=%02x, errcfg=%02x, errsts=%02x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_byte(pdev, 0x61), pci_byte(pdev, 0x70),
		pci_byte(pdev, 0x88), pci_byte(pdev, 0x8A));
}


SETUP_PCI_CMD(setup_ich5_sata, "ICH5 sata", PCI_COMMAND_PARITY,
	PCI_STATUS_DETECTED_PARITY | PCI_STATUS_REC_MASTER_ABORT)
SETUP_PCI_CMD(setup_ich5_ehci, "ICH5 ehci", PCI_COMMAND_SERR,
	PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
	PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_PARITY)


static void __init	setup_ich5_smbus(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "ICH5 smbus status: hostc=%04x\n",
		pci_word(pdev, 0x40));
}


static void __init	setup_pxh_br(struct pci_dev *pdev, int pass)
{
	dprintk(HA_PFX "PXH br before: pcicmd=%04x, status=%04x, brcnt=%04x, "
		"secsts=%04x, expdevcntl=%04x, expdevsts=%04x, "
		"erruncsev=%08x, erruncsts=%08x, "
		"uncpxerrsts=%04x, unxpxerrmsk=%08x, uncpxerrsev=%04x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_word(pdev, PCI_BRIDGE_CONTROL),
		pci_word(pdev, PCI_SEC_STATUS),
		pci_word(pdev, 0x4C), pci_word(pdev, 0x4E),
		pci_dword(pdev, 0x104), pci_word(pdev, 0x12C),
		pci_dword(pdev, 0x10C), pci_dword(pdev, 0x130),
		pci_word(pdev, 0x134));
	pci_write_config_word(pdev, PCI_COMMAND,
		pci_word(pdev, PCI_COMMAND) | 0x140);
	if (pass != 0) {
		pci_write_config_word(pdev, PCI_BRIDGE_CONTROL,
			pci_word(pdev, PCI_BRIDGE_CONTROL) | 0xC00 |
			PCI_BRIDGE_CTL_MASTER_ABORT | PCI_BRIDGE_CTL_SERR |
			PCI_BRIDGE_CTL_PARITY);
		pci_write_config_word(pdev, 0x4C, pci_word(pdev, 0x4C) | 0x4);
		pci_write_config_dword(pdev, 0x10C,
			pci_dword(pdev, 0x10C) | 0x11F00);
		pci_write_config_dword(pdev, 0x130, 0);
		pci_write_config_word(pdev, 0x134,
			pci_word(pdev, 0x134) | 0x3FEF);
	}
	pci_write_config_word(pdev, 0x12C, 0x3FEF);
	pci_write_config_dword(pdev, 0x104, 0x1FF010);
	pci_write_config_word(pdev, 0x4E, 0xF);
	pci_write_config_word(pdev, PCI_SEC_STATUS, 0xF900);
	pci_write_config_word(pdev, PCI_STATUS, PCI_STATUS_DETECTED_PARITY |
		PCI_STATUS_SIG_SYSTEM_ERROR | PCI_STATUS_REC_MASTER_ABORT |
		PCI_STATUS_REC_TARGET_ABORT | PCI_STATUS_SIG_TARGET_ABORT |
		PCI_STATUS_PARITY);
	dprintk(HA_PFX "PXH br after: pcicmd=%04x, status=%04x, brcnt=%04x, "
		"secsts=%04x, expdevcntl=%04x, expdevsts=%04x, "
		"erruncsev=%08x, erruncsts=%08x, "
		"uncpxerrsts=%04x, uncpxerrmsk=%08x, uncpxerrsev=%04x\n",
		pci_word(pdev, PCI_COMMAND), pci_word(pdev, PCI_STATUS),
		pci_word(pdev, PCI_BRIDGE_CONTROL),
		pci_word(pdev, PCI_SEC_STATUS),
		pci_word(pdev, 0x4C), pci_word(pdev, 0x4E),
		pci_dword(pdev, 0x104), pci_word(pdev, 0x12C),
		pci_dword(pdev, 0x10C), pci_dword(pdev, 0x130),
		pci_word(pdev, 0x134));
}


SETUP_PCI_CMD(setup_pxh_ioapic, "PXH ioapic",
	PCI_COMMAND_SERR | PCI_COMMAND_PARITY,
	PCI_STATUS_DETECTED_PARITY | PCI_STATUS_SIG_SYSTEM_ERROR)


static struct pci_probe_matrix __initdata	setup_matrix[] = {
	/* Intel MCH */
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SMCH, setup_mch },
	{ PCI_VENDOR_ID_INTEL, 0x3594, setup_mch_dma },
	{ PCI_VENDOR_ID_INTEL, 0x3595, setup_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3596, setup_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3597, setup_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3598, setup_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3599, setup_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x359A, setup_mch_exp },
	/* Intel ICH5 */
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801BA_11, setup_ich5_hub },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801EB_0, setup_ich5_lpc },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801EB_1, setup_ich5_sata },
	{ PCI_VENDOR_ID_INTEL, 0x24DF, setup_ich5_sata },
	{ PCI_VENDOR_ID_INTEL, 0x24dd, setup_ich5_ehci },
	{ PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82801EB_3, setup_ich5_smbus },
	/* Intel PXH */
	{ PCI_VENDOR_ID_INTEL, 0x0329, setup_pxh_br },
	{ PCI_VENDOR_ID_INTEL, 0x032A, setup_pxh_br },
	{ PCI_VENDOR_ID_INTEL, 0x032C, setup_pxh_br },
	{ PCI_VENDOR_ID_INTEL, 0x0326, setup_pxh_ioapic },
	{ PCI_VENDOR_ID_INTEL, 0x0327, setup_pxh_ioapic },
	{ 0, 0, 0 }
};


static void __init	setup_hw(int pass)
{
	int	count = 0;
	static u16 __initdata	vendor, device;
	struct pci_dev	*pdev;

	pdev = NULL;
	dprintk(HA_PFX "set_hw pass %d\n", pass);
	while ((pdev = pci_find_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
		int	loop = 0;

		pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
		pci_read_config_word(pdev, PCI_DEVICE_ID, &device);
		while (setup_matrix[loop].vendor) {
			if (vendor == setup_matrix[loop].vendor &&
				device == setup_matrix[loop].device) {
				dprintk(HA_PFX "Found device %x:%04x\n",
					vendor, device);
				setup_matrix[loop].check(pdev, pass);
				++count;
			}
			++loop;
		}
	}
	if (count == 0)
		printk(HA_PFX "Pass %d: Can't find anything to set up.\n",
			pass);
}


static struct {
	u16	vendor;
	u16	device;
} disabled_devices[] = {
	{ PCI_VENDOR_ID_ADAPTEC2, 0x801d },
	{ 0, 0 }	/* End of list */
};


/**
 * xio3d_disable_pci_device - Check for disabled devices.
 * @dev: PCI device to check
 *
 * Non-zero is returned if device is to be disabled.
 */

int	xio3d_disable_pci_device(struct pci_dev *dev)
{
	int	ix = 0;

	if (PCI_FUNC(dev->devfn) != 1)
		return 0;	/* If not function 1 */
	while (disabled_devices[ix].vendor) {
		if (dev->vendor == disabled_devices[ix].vendor &&
			dev->device == disabled_devices[ix].device)
			return 1;
		++ix;
	}
	return 0;
}


static void	restore_mch_exp(struct pci_dev *pdev, int pass)
{
	pci_write_config_word(pdev, PCI_COMMAND, pci_word(pdev, PCI_COMMAND) &
		~(PCI_COMMAND_PARITY | PCI_COMMAND_SERR));
	pci_write_config_byte(pdev, PCI_BRIDGE_CONTROL,
		pci_byte(pdev, PCI_BRIDGE_CONTROL) & ~(PCI_BRIDGE_CTL_SERR |
		PCI_BRIDGE_CTL_PARITY));
	pci_write_config_dword(pdev, 0x148, 0);
}


static struct pci_probe_matrix	restore_matrix[] = {
	/* Intel MCH */
	{ PCI_VENDOR_ID_INTEL, 0x3595, restore_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3596, restore_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3597, restore_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3598, restore_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x3599, restore_mch_exp },
	{ PCI_VENDOR_ID_INTEL, 0x359A, restore_mch_exp },
	{ 0, 0, 0 }
};


/**
 * xio3d_restore_hw_poweroff - Restore hardware to allow poweroff to work.
 */

void	xio3d_restore_hw_poweroff(void)
{
	u16	vendor, device;
	struct pci_dev	*pdev;

	pdev = NULL;
	while ((pdev = pci_find_device(PCI_ANY_ID, PCI_ANY_ID, pdev))) {
		int	loop = 0;

		pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
		pci_read_config_word(pdev, PCI_DEVICE_ID, &device);
		while (restore_matrix[loop].vendor) {
			if (vendor == restore_matrix[loop].vendor &&
				device == restore_matrix[loop].device) {
				printk(HA_PFX "Found device %x:%04x\n",
					vendor, device);
				restore_matrix[loop].check(pdev, 0);
			}
			++loop;
		}
	}
}
#endif /* !defined(CONFIG_EDAC) */


MODULE_AUTHOR("Xiotech Corporation");
MODULE_DESCRIPTION("Xiotech kernel support module");
MODULE_LICENSE("GPL");

static struct file_operations	xio3d_chrdev_ops = {
	.owner	= THIS_MODULE,
	.open	= xio3d_dev_open,
	.ioctl	= xio3d_dev_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = xio3d_dev_ioctl32,
#endif
	.poll	= xio3d_dev_poll,
	.read	= xio3d_dev_read,
	.write	= xio3d_dev_write,
	.mmap	= xio3d_dev_mmap,
	.release = xio3d_dev_release,
};


/**
 * xio3d_init - Initialize the xio3d module.
 */

static int __init	xio3d_init(void)
{
	int	rc;

#ifdef	CONFIG_XIO_7000
	printk(KERN_INFO PFX "Xiotech Emprise kernel support module, v%.*s\n",
		(int)XIO3D_VERLENGTH, XIO3D_VERSION);
#else
	printk(KERN_INFO PFX "Xiotech Mag 3D kernel support module, v%.*s\n",
		(int)XIO3D_VERLENGTH, XIO3D_VERSION);
#endif	/* CONFIG_XIO_7000 */

	spin_lock_init(&irqlock);

#if !defined(CONFIG_EDAC)
	setup_hw(0);		/* Do first pass of hw initialization */
	setup_hw(1);		/* Do second pass of hw initialization */
#endif	/* !defined(CONFIG_EDAC) */

	rc = register_chrdev(XIO3D_MAJOR, XIO3D_MODULE_NAME,
			&xio3d_chrdev_ops);
	if (rc < 0) {
		printk(KERN_ERR PFX "device register failed with %d\n", rc);
		goto err_out_cleanup;
	}

	return 0;

err_out_cleanup:
	return rc;
}

module_init(xio3d_init);


/**
 * xio3d_cleanup - Cleanup the xio3d module prior to removal.
 */

static void __exit	xio3d_cleanup(void)
{
	unregister_chrdev(XIO3D_MAJOR, XIO3D_MODULE_NAME);
	printk(KERN_INFO PFX "module cleaned up\n");
}

module_exit(xio3d_cleanup);

/*
 * xio3dshm_setup - xio3dshm kernel parameter handler.
 */

static int __init	xio3dshm_setup(char *s)
{
	int	i;
	char	*str;
	u_int64_t	mem;

	str = s;
	for (i = 0; i < XIO3D_NSHM_MAX; ++i) {
		mem = memparse(str, &str);
		/* Round up to next block */
		mem = (mem + MEMBLK_SIZE - 1) & ~(MEMBLK_SIZE - 1);
		printk(KERN_INFO PFX "mem[%d]=%lld\n", i, mem);
		xio3d_drvinfo.mem_regions[i].size = mem;
		if (*str++ != ',')
			break;
	}
	return 1;
}

__setup("xio3dshm=", xio3dshm_setup);

/***
** vi:sw=8 ts=8 noexpandtab
**/
