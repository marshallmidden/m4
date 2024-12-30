/* $Id: subst-elf.c 90925 2009-06-30 22:11:07Z mdr $ */
/*
 *	subst-elf - Substitute section in elf core dump.
 *
 *	Modified from Marshall Midden's truncate program by:
 *	Mark D. Rustad, 2009/06/29.
 *
 *	Copyright 2009 Xiotech Corporation. All rights reserved.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include <elf.h>

extern int	errno;

static int      debug = 0;

#define PARTIAL_SIZE (1*1024*1024)

/***
 *
 * @brief	Read a section from an elf file
 *
 * @param	fd - File descriptor to read from
 * @param	fpos - File position to read from
 * @param	size - Amount to read
 *
 * @return	Pointer to allocated memory holding the data
 *
 * @attention	If anything goes wrong, this function aborts the process.
 */

static void	*read_section(int fd, unsigned long fpos, size_t size)
{
	void	*p;
	int	rc;

	if (debug > 0) {
		fprintf(stderr, "%s: fpos=%lu, size=%zu\n",
			__func__, fpos, size);
	}
	if (size > PARTIAL_SIZE || size < 4) {
		fprintf(stderr, "%s: Invalid section size=%d\n",
			__func__, size);
		exit(1);
	}
	p = malloc(size);
	if (!p) {
		fprintf(stderr, "%s: malloc failed, size = %zu, err = %s\n",
			__func__, size, strerror(errno));
		exit(1);
	}
	rc = lseek(fd, fpos, SEEK_SET);
	if (rc < 0) {
		perror("lseek");
		exit(1);
	}
	rc = read(fd, p, size);
	if (rc < 0) {
		perror("read");
		exit(1);
	}
	return p;
}


/***
 *
 * @brief	Substitute elf sections
 *
 * @param	fd - File decriptor of elf file
 * @param	segs - Array of pointers to substitute segment names
 * @param	nsegs - Number of elements in segs array
 *
 * @return	none
 *
 * @attention	If anything goes wrong, this function aborts the process
 */

static void	subst_elf_sections(int fd, char *segs[], int nsegs)
{
	Elf32_Shdr	*elf_shdata;
	Elf32_Shdr	*elf_psh;
	int	rc;
	int	size;
	int	i;
	int	n;
	Elf32_Ehdr	*elf_ex;
	char	*shstrtab;		/* Section names */
	Elf32_Phdr	*elf_phdata;
	Elf32_Phdr	*elf_ph;

	/* Get the exec-header */

	elf_ex = read_section(fd, 0, sizeof(*elf_ex));

	if (debug > 0) {
		fprintf(stderr, "elf_ex: e_type=0x%04x e_machine=0x%04x "
			"e_version=0x%08x e_entry=0x%08x e_phoff=0x%08x "
			"e_shoff=0x%08x e_flags=0x%08x e_ehsize=0x%04x "
			"e_phentsize=0x%04x e_phnum=0x%04x e_shentsize=0x%04x "
			"e_shnum=0x%04x e_shstrndx=0x%04x\n",
			elf_ex->e_type, elf_ex->e_machine, elf_ex->e_version,
			elf_ex->e_entry, elf_ex->e_phoff, elf_ex->e_shoff,
			elf_ex->e_flags, elf_ex->e_ehsize, elf_ex->e_phentsize,
			elf_ex->e_phnum, elf_ex->e_shentsize, elf_ex->e_shnum,
			elf_ex->e_shstrndx);
	}

	/* First of all, some simple consistency checks */

	if (memcmp(elf_ex->e_ident, ELFMAG, SELFMAG) != 0) {
		fprintf(stderr, "e_ident: check failed\n");
		exit(1);
	}

	if (elf_ex->e_type != ET_CORE) {
		fprintf(stderr, "e_type: type %d not allowed, must be core\n",
			elf_ex->e_type);
		exit(1);
	}

	size = elf_ex->e_shentsize * elf_ex->e_shnum;
	elf_shdata = read_section(fd, elf_ex->e_shoff, size);

	/* Read the shstrndx (.shstrtab) table. */

	elf_psh = elf_shdata + elf_ex->e_shstrndx;
	if (elf_psh->sh_type != SHT_STRTAB) {
		fprintf(stderr, "%s: e_shstrndx (%d) is not a SHT_STRTAB "
			"(%d [0x%x])\n", __func__,
			elf_ex->e_shstrndx, elf_psh->sh_type, elf_psh->sh_type);
		exit(1);
	}

	/* A string table */

	shstrtab = read_section(fd, elf_psh->sh_offset, elf_psh->sh_size);

	/* Read in all of the Program header information */

	size = elf_ex->e_phentsize * elf_ex->e_phnum;
	elf_phdata = read_section(fd, elf_ex->e_phoff, size);

	if (debug > 1) {
		fprintf(stderr, "elf_ex->e_phnum=%d\n", elf_ex->e_phnum);
	}

	/* Look for sections to substitute */
	for (i = 0; i < nsegs; ++i) {
		elf_psh = elf_shdata;
		for (n = 0; n < elf_ex->e_shnum; ++elf_psh, ++n) {
			if (debug > 0) {
				fprintf(stderr, "%2.2d %s: off=%d size=%d\n",
					n, shstrtab + elf_psh->sh_name,
					elf_psh->sh_offset, elf_psh->sh_size);
			}
			if (strcmp(segs[i], shstrtab + elf_psh->sh_name) == 0) {
				break;
			}
		}

		if (n >= elf_ex->e_shnum) {
			fprintf(stderr, "%s: Missing segment %s\n",
				__func__, segs[i]);
			continue;
		}

		if (debug > 0) {
			fprintf(stderr, "%s: Found segment %s at %d\n",
				__func__, segs[i], n);
		}

		elf_ph = elf_phdata;
		for (n = 0; n < elf_ex->e_shnum; ++elf_ph, ++n) {
			if (debug > 0) {
				fprintf(stderr, "%s: p_offset=%x, p_vaddr=%x, "
					"p_filesz=%x\n",
					__func__, elf_ph->p_offset,
					elf_ph->p_vaddr, elf_ph->p_filesz);
			}
			if (elf_psh->sh_addr == elf_ph->p_vaddr) {
				if (debug > 0) {
					fprintf(stderr, "%s: Found at %d\n",
						__func__, n);
				}
				break;
			}
		}

		if (n >= elf_ex->e_shnum) {
			fprintf(stderr, "%s: PH not found for %s\n",
				__func__, segs[i]);
			continue;
		}
		if (debug > 0) {
			fprintf(stderr, "%s: Adjusting segment %s\n",
				__func__, segs[i]);
		}
		elf_ph->p_offset = elf_psh->sh_offset;
		elf_ph->p_filesz = elf_psh->sh_size;
	}

	/* Update program header */

	size = elf_ex->e_phentsize * elf_ex->e_phnum;
	rc = lseek(fd, elf_ex->e_phoff, SEEK_SET);
	if (rc < 0) {
		perror("lseek");
		exit(1);
	}
	rc = write(fd, elf_phdata, size);
	if (rc < 0) {
		perror("write");
		exit(1);
	}

	free(elf_phdata);
	free(shstrtab);
	free(elf_shdata);
	free(elf_ex);

	return;

} /* End of subst_elf_sections */


/***
 *
 * @brief	Main program for subst-elf
 *
 */

int	main(int argc, char *argv[])
{
	int	c;
	int	errflg = 0;
	int	nsegs = 0;
	char	*segs[16];
	extern char	*optarg;
	extern int	optind, opterr, optopt;

	while ((c = getopt(argc, argv, ":ds:")) != -1) {
		switch (c) {
		case 'd':
			++debug;
			break;

		case 's':
			if (nsegs == 16) {
				fprintf(stderr, "Too many segments\n");
				exit(1);
			}
			segs[nsegs] = optarg;
			++nsegs;
			break;

		case ':':
			fprintf(stderr, "Option -%c requires an operand\n",
				optopt);
			++errflg;
			break;

		case '?':
			fprintf(stderr, "Unrecognized option: -%c\n", optopt);
			++errflg;
			break;
		}
	}

	if (errflg) {
		fprintf(stderr, "Usage: subst-elf -s <seg> ... <elfcore>\n");
		exit(2);
	}

	for ( ; optind < argc; optind++) {
		int	fd;

		if ((fd = open(argv[optind], O_RDWR, 0)) < 0) {
			perror("open");
			exit(1);
		}

		subst_elf_sections(fd, segs, nsegs);

		close(fd);
	}

	exit(0);
}					/* end of main */

/* ------------------------------------------------------------------------ */
