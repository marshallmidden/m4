/* $Id: shmdump.c 90925 2009-06-30 22:11:07Z mdr $ */
/*
 * shmdump.c - Dump shared hugetlb file.
 *
 * Copyright (c) 2009 Xiotech Corporation. All Rights Reserved.
 *
 * Mark D. Rustad, 2009/06/22.
 */

/*
 * This program dumps a hugetlb file passed as a parameter to stdout.
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


int main(int argc, char *argv[])
{
	int	hfd;
	int	rc;
	uintptr_t	shmoff = 0x48000000;
	struct stat	statbuf;

	if (argc != 2) {
		fprintf(stderr, "Single argument required.\n");
		exit(1);
	}

	void	*p;

	if (!argv[1] || !argv[1][0]) {
		fprintf(stderr, "Null parameter ignored\n");
		exit(1);
	}
	hfd = open(argv[1], O_RDONLY, 0);
	if (hfd < 0) {
		perror("open");
		fprintf(stderr, "Open of %s failed returning %d\n",
			argv[1], hfd);
		exit(1);
	}
	rc = fstat(hfd, &statbuf);
	if (rc == -1) {
		perror("fstat");
		fprintf(stderr, "Fstat of %s failed, returning %d\n",
			argv[1], rc);
		exit(1);
	}
	rc = munmap((void *)(intptr_t)shmoff, statbuf.st_size);
	if (rc < 0) {
		perror("munmap");
		fprintf(stderr, "munmap returned %d\n", rc);
		exit(1);
	}
	p = mmap((void *)(intptr_t)shmoff, statbuf.st_size,
		PROT_READ, MAP_SHARED | MAP_FIXED, hfd, 0);
	if (p == MAP_FAILED) {
		perror("mmap");
		fprintf(stderr, "mmap failed at %08x\n", shmoff);
		exit(1);
	}

	rc = fwrite(p, 1, statbuf.st_size, stdout);
	if (rc != statbuf.st_size) {
		fprintf(stderr, "Fwrite returned %d instead of %lu\n",
			rc, statbuf.st_size);
		exit(1);
	}
	close(hfd);

	return 0;
}


/**
** vi:sw=8 ts=8 noexpandtab
*/
