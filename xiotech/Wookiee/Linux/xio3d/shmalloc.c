/* $Id: shmalloc.c 15414 2006-11-16 20:27:45Z RustadM $ */
/*
 * shmalloc.c - Allocate shared hugetlb files contiguously.
 *
 * Copyright (c) 2006 Xiotech Corporation. All Rights Reserved.
 *
 * Mark D. Rustad, 2006/03/21.
 */

/*
 * This program creates files passed as parameters out of huge pages
 * in a hugetlb filesystem out of contiguous memory. The xio3d driver
 * provides an ioctl for that purpose. There are lots of potential
 * pitfalls in general system usage, but in a Xiotech controller during
 * boot time, this seems to work fine. Be sure that the files that you
 * pass on the command line really are in a hugetlb filesystem!
 */

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "xio3d.h"


int main(int argc, char *argv[])
{
	int	xiofd;
	int	hfd;
	int	rc;
	int	i;
	uintptr_t	shmoff = 0x48000000;
	struct xio3d_drvinfo	*drvinf;
	struct xio3d_shmalloc	shmreq;

	xiofd = open("/dev/xio3d0", O_RDWR);
	if (xiofd < 0) {
		perror("open");
		fprintf(stderr, "Open of xio3d failed returning %d\r\n",
		    xiofd);
		exit(1);
	}
	drvinf = mmap(0, XIO3D_INFO_SIZE, PROT_READ, MAP_SHARED | MAP_LOCKED,
		xiofd, XIO3D_INFO_OFFSET);
	if (drvinf == MAP_FAILED) {
		perror("mmap");
		fprintf(stderr, "mmap of info failed\n");
		exit(1);
	}

	for (i = 1; i < argc; ++i) {
		void	*p;

		if (!argv[i] || !argv[i][0]) {
			fprintf(stderr, "Null parameter ignored\n");
			continue;
		}
		hfd = open(argv[i], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (hfd < 0) {
			perror("open");
			fprintf(stderr, "Open of %s failed returning %d\n",
			    argv[i], hfd);
			exit(1);
		}
		shmreq.vaddr = shmoff;
		shmreq.rgn = i - 1;
		shmreq.len = drvinf->mem_regions[shmreq.rgn].size;
		rc = munmap((void *)(intptr_t)shmreq.vaddr, shmreq.len);
		if (rc < 0) {
			perror("munmap");
			fprintf(stderr, "munmap rgn%d returned %d\n",
			    shmreq.rgn, rc);
			exit(1);
		}
		p = mmap((void *)(intptr_t)shmreq.vaddr, shmreq.len,
		    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, hfd, 0);
		if (p == MAP_FAILED) {
			perror("mmap");
			fprintf(stderr, "mmap failed rgn%d at %08x\n",
			    shmreq.rgn, shmoff);
			exit(1);
		}
		rc = ioctl(xiofd, XIO3D_SHMALLOC, &shmreq);
		if (rc < 0) {
			fprintf(stderr, "ioctl shmalloc rgn%d returned %d\n",
			    shmreq.rgn, rc);
			exit(1);
		}
		close(hfd);
		printf("Region %d, size %08x in file %s\n",
		    shmreq.rgn, shmreq.len, argv[i]);
		shmoff += shmreq.len;
	}

	return 0;
}

/**
** vi:sw=8 ts=8 noexpandtab
*/
