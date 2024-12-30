/* $Id: xio3d.h 11794 2006-07-17 20:39:06Z RustadM $ */
/*
 * xio3d.h - Definitions for Xiotech Mag 3D module.
 *
 * Copyright 2004-2006 Xiotech Corporation. All rights reserved.
 *
 * Mark D. Rustad, 2004/03/24.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#ifndef _XIO3D_H_
#define _XIO3D_H_   1

#include <asm/ioctl.h>

#define XIO3D_MODULE_NAME	"xio3d"

#define XIO3D_ID	0xFEBECCB0

#define XIO3D_NSHM_MAX	4	/* Maximum number of shared memory regions */
#define XIO3D_NDEVS_MAX	16	/* Maximum number of device nodes */

#define XIO3D_MAX_IRQS	16	/* Maximum irq number + 1 */

#define XIO3D_EVT_BASE	16	/* Base for event numbers */
#define XIO3D_EVT_MAX	24	/* Maximum event number + 1 */

#define XIO3D_BE_EVT	16	/* Event to wake BE scheduler */
#define XIO3D_FE_EVT	17	/* Event to wake FE scheduler */
#define XIO3D_CCB_EVT	18	/* Event to wake CCB scheduler */

#define XIO3D_INFO_OFFSET	0x50000000
#define XIO3D_INFO_SIZE		4096

/* Describes a single allocated kernel memory region */

struct xio3d_mem_region {
	uint64_t	offset;	/* Offset in "file" to request */
	uint64_t	size;	/* Size of region */
	uint64_t	phys;	/* Physical address for DMA */
	uint64_t	kmem;	/* Kernel memory address */
};

/*
 * Driver information structure.
 */

struct xio3d_drvinfo {
	uint64_t	id;	/* Holds ID for validation/version check */

	struct xio3d_mem_region	mem_regions[XIO3D_NSHM_MAX];
				/* FE/BE/CCB regions */

	struct xio3d_mem_region	info_region;
				/* Info region */

        uint32_t	active;	/* Set of active ints & events */
        uint32_t	mask;	/* Set of unmasked ints & events */
        uint32_t	reg;	/* Set of registered ints & events */

	uint32_t	open_index;	/* Index in irq_reg array */
};

struct xio3d_shmalloc {
	uint64_t	vaddr;
	uint32_t	len;
	uint32_t	rgn;
};

/*
 * xio3d ioctls
 */

#define XIO3D_GETINF	_IOWR('X', 1, struct xio3d_drvinfo)
#define XIO3D_REGINT	_IOWR('X', 2, int32_t)
#define XIO3D_MASKEVT	_IOWR('X', 3, u_int32_t)
#define XIO3D_REGEVT	_IO('X', 4)
#define XIO3D_SENDEVT	_IO('X', 5)
#define XIO3D_PCIINT	_IOWR('X', 6, int32_t)
#define XIO3D_SHMALLOC	_IOR('X', 7, struct xio3d_shmalloc)

#endif	/* _XIO3D_H_ */

/***
** vi:sw=8 ts=8 noexpandtab
**/
