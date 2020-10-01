/* TODO */
/* 1) _fdata symbol section and value saved in special section. */
/* 2) elfpic binary to work. */

/* This causes a null section header to be inserted.  */
/* #define OBJDUMP_COMPAT */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "elf.h"

static int      debug = 0;

static struct pt_list {
  char           *memory;
  struct elf32_phdr pt_data;		/* Entry for entry. */
  struct pt_list *pt_next;
}              *pt_start = NULL, *pt_last;

static struct et_list {
  char           *name;
  char           *memory;
  Elf32_Shdr      et_data;		/* Entry for entry. */
  struct pt_list *tpt;
  unsigned int    tpt_offset;
  struct et_list *et_next;
}              *et_start = NULL, *et_last;

/* ------------------------------------------------------------------------ */
static char     buf[128];
/* ------------------------------------------------------------------------ */
#define PARTIAL_SIZE (1*1024*1024)
/* ------------------------------------------------------------------------ */
static void     load_elf_binary(int fd)
{
  Elf32_Shdr     *elf_shdata = NULL;
  Elf32_Shdr     *new_elf_shdata = NULL;
  Elf32_Shdr     *elf_psh;
  int             retval;
  int             size;
  int             i;
  int             n;
  struct elfhdr   elf_ex;
  struct elfhdr   new_elf_ex;
  int             fpos;
  char           *shstrtab = NULL;	/* section names */
  struct elf_phdr *elf_phdata = NULL;
/*   struct elf_phdr *new_elf_phdata = NULL; */
  struct elf_phdr *elf_ppnt;
  struct pt_list *npt;
  struct pt_list *tpt;
  struct et_list *net;
  struct et_list *shstr;
  struct et_list *tet;
  int             name_size = 0;

/*   fprintf(stderr, "struct elfhdr=%x(%d), struct elf_phdr=%x(%d), Elf32_Shdr=%x(%d), Elf32_RegInfo=%x(%d)\n", sizeof(struct elfhdr), sizeof(struct elfhdr), sizeof(struct elf_phdr), sizeof(struct elf_phdr), sizeof(Elf32_Shdr), sizeof(Elf32_Shdr), sizeof(Elf32_RegInfo), sizeof(Elf32_RegInfo)); */
/* --------------- */
  elf_ex = *((struct elfhdr *) buf);	/* Get the exec-header */
  if (debug != 0) {
    fprintf(stderr, "elf_ex: e_type=0x%04x e_machine=0x%04x e_version=0x%08x e_entry=0x%08x e_phoff=0x%08x e_shoff=0x%08x e_flags=0x%08x e_ehsize=0x%04x e_phentsize=0x%04x e_phnum=0x%04x e_shentsize=0x%04x e_shnum=0x%04x e_shstrndx=0x%04x\n", ntohs(elf_ex.e_type), ntohs(elf_ex.e_machine), ntohl(elf_ex.e_version), ntohl(elf_ex.e_entry), ntohl(elf_ex.e_phoff), ntohl(elf_ex.e_shoff), ntohl(elf_ex.e_flags), ntohs(elf_ex.e_ehsize), ntohs(elf_ex.e_phentsize), ntohs(elf_ex.e_phnum), ntohs(elf_ex.e_shentsize), ntohs(elf_ex.e_shnum), ntohs(elf_ex.e_shstrndx));
  }
/* --------------- */
/* First of all, some simple consistency checks */
  if (memcmp(elf_ex.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "e_ident: check failed\n");
    exit(1);
  }
  switch (ntohs(elf_ex.e_type)) {
      case ET_REL:
	fprintf(stderr, "e_type: ET_REL (relocatable file) is not allowed, must be executable with -q\n");
	exit(1);
      case ET_EXEC:
	break;
      case ET_DYN:
	break;
      case ET_CORE:
	fprintf(stderr, "e_type: ET_CORE (core file) is not allowed, must be executable with -q\n");
	exit(1);
      default:
	fprintf(stderr, "e_type: check failed %x (%d) != (%d || %d)\n", ntohs(elf_ex.e_type), ntohs(elf_ex.e_type), ET_EXEC, ET_DYN);
	exit(1);
  }
  if (!elf_check_arch(&elf_ex)) {
    fprintf(stderr, "elf_check_arch: check failed\n");
    exit(1);
  }
/* --------------- */
/* start filling in new_elf_ex structure. */
  bcopy(elf_ex.e_ident, new_elf_ex.e_ident, EI_NIDENT);	/* ELF "magic number" */
  new_elf_ex.e_type = elf_ex.e_type;	/* object file type */
  new_elf_ex.e_machine = elf_ex.e_machine;	/* required machine architecture */
  new_elf_ex.e_version = elf_ex.e_version;	/* object file version */
  new_elf_ex.e_entry = elf_ex.e_entry;	/* entry point */
  new_elf_ex.e_phoff = -1;		/* Program header offset elf_phdr structure */
  new_elf_ex.e_shoff = -2;		/* Section header offset Elf32_Shdr structures */
  new_elf_ex.e_flags = elf_ex.e_flags;	/* Processor specific flags */
  new_elf_ex.e_ehsize = elf_ex.e_ehsize;/* ELF header size in bytes */
  new_elf_ex.e_phentsize = elf_ex.e_phentsize;	/* Program header table entry size. */
  new_elf_ex.e_phnum = -1;		/* Number of Program header sections. */
  new_elf_ex.e_shentsize = elf_ex.e_shentsize;	/* Section header table entry size */
  new_elf_ex.e_shnum = -1;		/* Number of Section headers */
  new_elf_ex.e_shstrndx = -1;		/* Section header string table index. zero based. */
/* --------------- */
  size = ntohs(elf_ex.e_shentsize) * ntohs(elf_ex.e_shnum);
  if (size > PARTIAL_SIZE || size < 4) {
    fprintf(stderr, "elf_shdata size=%d (%d * %d)\n", size, ntohs(elf_ex.e_shentsize), ntohs(elf_ex.e_shnum));
    exit(1);
  }
  fpos = ntohl(elf_ex.e_shoff);
  elf_shdata = (Elf32_Shdr *) malloc(size);
  if (elf_shdata == NULL) {
    fprintf(stderr, "malloc, elf_shdata size=%d (%d * %d)\n", size, ntohs(elf_ex.e_shentsize), ntohs(elf_ex.e_shnum));
    exit(1);
  }
  retval = lseek(fd, fpos, SEEK_SET);
  if (retval < 0) {
    perror("lseek elf_shdata");
    exit(1);
  }
  retval = read(fd, (char *) elf_shdata, size);
  if (retval < 0) {
    perror("read elf_shdata");
    exit(1);
  }
  new_elf_shdata = (Elf32_Shdr *) malloc(size);
  if (new_elf_shdata == NULL) {
    fprintf(stderr, "malloc, new_elf_shdata size=%d (%d * %d)\n", size, ntohs(elf_ex.e_shentsize), ntohs(elf_ex.e_shnum));
    exit(1);
  }
/* --------------- */
/* Read the shstrndx (.shstrtab) table. */
  elf_psh = elf_shdata + ntohs(elf_ex.e_shstrndx);
  switch (ntohl(elf_psh->sh_type)) {
      case SHT_STRTAB:			/* A string table */
	size = ntohl(elf_psh->sh_size);
	fpos = ntohl(elf_psh->sh_offset);
	if (size > PARTIAL_SIZE || size < 4) {
	  fprintf(stderr, "SHT_STRTAB bad size=%d\n", size);
	  exit(1);
	}
	shstrtab = (char *) malloc(size);
	if (!shstrtab) {
	  fprintf(stderr, "strtab malloc failure, size=%d\n", size);
	  exit(1);
	}
	retval = lseek(fd, fpos, SEEK_SET);
	if (retval < 0) {
	  perror("read elf_shdata");
	  exit(1);
	}
	retval = read(fd, (char *) shstrtab, size);
	if (retval < 0) {
	  perror("read shstrtab");
	  exit(1);
	}
	break;

      default:
	fprintf(stderr, "e_shstrndx (%d) is not a SHT_STRTAB (%d [0x%x])\n",
		ntohs(elf_ex.e_shstrndx), ntohl(elf_psh->sh_type), ntohl(elf_psh->sh_type));
	exit(1);
  }
/* --------------- */
/* Read in all of the Program header information */
  size = ntohs(elf_ex.e_phentsize) * ntohs(elf_ex.e_phnum);
  if (size > PARTIAL_SIZE || size < 4) {
    fprintf(stderr, "elf_phdata size=%d (%d * %d)\n", size, ntohs(elf_ex.e_phentsize), ntohs(elf_ex.e_phnum));
    exit(1);
  }
  fpos = ntohl(elf_ex.e_phoff);		/* location in file */
  elf_phdata = (struct elf_phdr *) malloc(size);
  if (!elf_phdata) {
    fprintf(stderr, "malloc elf_phdata size=%d (%d * %d)\n", size, ntohs(elf_ex.e_phentsize), ntohs(elf_ex.e_phnum));
    exit(1);
  }
  retval = lseek(fd, fpos, SEEK_SET);
  if (retval < 0) {
    perror("lseek, elf_phdata");
    exit(1);
  }
  retval = read(fd, (char *) elf_phdata, size);
  if (retval < 0) {
    perror("read elf_phdata");
    exit(1);
  }
/* --------------- */
/* fprintf(stderr, "elf_ex.e_phnum=%d\n", ntohs(elf_ex.e_phnum)); */
  for (i = 0, n = 0, elf_ppnt = elf_phdata; i < ntohs(elf_ex.e_phnum); i++, elf_ppnt++) {
    if (debug != 0) {
      fprintf(stderr, "i=%d of %d, p_type=%x, (PT_LOAD=%d), offset=0x%x, filesz=0x%x\n", i, ntohs(elf_ex.e_phnum), ntohl(elf_ppnt->p_type), PT_LOAD, ntohl(elf_ppnt->p_offset), ntohl(elf_ppnt->p_filesz));
    }
    if (ntohl(elf_ppnt->p_type) == PT_LOAD) {
      npt = (struct pt_list *) malloc(sizeof(struct pt_list));
      if (npt == NULL) {
	fprintf(stderr, "malloc npt\n");
	exit(1);
      }
      size = ntohl(elf_ppnt->p_filesz);
      npt->memory = (char *) malloc(size);
      if (npt->memory == NULL) {
	fprintf(stderr, "malloc npt->memory (%d)\n", size);
	exit(1);
      }
      fpos = ntohl(elf_ppnt->p_offset);	/* location in file */
      retval = lseek(fd, fpos, SEEK_SET);
      if (retval < 0) {
	perror("lseek, elf_ppnt->p_offset");
	exit(1);
      }
      retval = read(fd, npt->memory, size);
      if (retval < 0) {
	perror("read npt->memory");
	exit(1);
      }
      bcopy((char *) elf_ppnt, (char *) &(npt->pt_data), sizeof(struct elf_phdr));
      n++;
      npt->pt_next = NULL;
      if (pt_start == NULL) {
	pt_start = npt;
      } else {
	pt_last->pt_next = npt;
      }
      pt_last = npt;
    }
  }
  new_elf_ex.e_phnum = ntohs(n);	/* Number of Program header sections. */
/* --------------- */
/* Go through section tables. */
  for (i = 0, n = 0, elf_psh = elf_shdata; i < ntohs(elf_ex.e_shnum); i++, elf_psh++) {
    if (debug != 0) {
      fprintf(stderr, "%2.2d ", i);
      if (shstrtab != NULL) {
	fprintf(stderr, "%s: ", shstrtab + ntohl(elf_psh->sh_name));
      }
      fprintf(stderr, "off = %d  size = %d\n   ", ntohl(elf_psh->sh_offset), ntohl(elf_psh->sh_size));
#if 1
      fprintf(stderr, "name=%x, type=%x, flags=%x, addr=%x, off=%x, size=%x, link=%x, info=%x, align=%x, entsize=%x\n",
	      ntohl(elf_psh->sh_name), ntohl(elf_psh->sh_type),
	      ntohl(elf_psh->sh_flags), ntohl(elf_psh->sh_addr), ntohl(elf_psh->sh_offset),
	      ntohl(elf_psh->sh_size), ntohl(elf_psh->sh_link), ntohl(elf_psh->sh_info),
	      ntohl(elf_psh->sh_addralign), ntohl(elf_psh->sh_entsize));
#endif					/* 0 */
    }
    switch (ntohl(elf_psh->sh_type)) {
	case SHT_PROGBITS:		/* private */
	  switch (ntohl(elf_psh->sh_flags)) {
	      case 0:
		if (shstrtab != NULL) {
		  if (strcmp(shstrtab + ntohl(elf_psh->sh_name), ".stack") == 0) {
		    goto save_elf_ex;
		  }
		}
		break;
	      case 2:
		break;
	      case 3:
		break;
	      case 6:			/* 6 = relocatable .text (read-only) */
	      case 7:			/* 7 = executable .text (writable) */
		break;
	      case 0x10000003:		/* GP rel, alloc writable. */
		break;
	      default:
		fprintf(stderr, "SHT_PROGBITS, unknown flag types 0x%x\n", ntohl(elf_psh->sh_flags));
		break;
	  }
	  break;

	case SHT_NOBITS:		/* Section occupies no space in file */
	  switch (ntohl(elf_psh->sh_flags)) {
	      case 3:
		break;
	      case 0x10000003:
		break;
	      default:
		fprintf(stderr, "elf: SHT_NOBITS unknown flag types (%x)\n", ntohl(elf_psh->sh_flags));
		break;
	  }
	  break;

	case SHT_STRTAB:		/* A string table */
	  if (i == ntohs(elf_ex.e_shstrndx)) {
	    shstr = (struct et_list *) malloc(sizeof(struct et_list));
	    if (shstr == NULL) {
	      fprintf(stderr, "malloc shstr\n");
	      exit(1);
	    }
	    size = strlen(shstrtab + ntohl(elf_psh->sh_name)) + 1;
	    shstr->name = (char *) malloc(size);
	    if (shstr->name == NULL) {
	      fprintf(stderr, "malloc shstr->name\n");
	      exit(1);
	    }
	    name_size += size;
	    strcpy(shstr->name, shstrtab + ntohl(elf_psh->sh_name));
	    bcopy((char *) elf_psh, (char *) &(shstr->et_data), sizeof(Elf32_Shdr));
	    shstr->et_data.sh_name = -1;
	    shstr->et_data.sh_addr = 0;
	    shstr->et_data.sh_offset = -1;	/* make -1, set later */
	    shstr->et_data.sh_size = -1;
	    shstr->et_data.sh_link = 0;
	    shstr->et_data.sh_info = 0;
	    shstr->et_data.sh_entsize = 0;
	  }
	  break;

	case SHT_RELA:			/* Relocation entries, with addends */
	  break;
	case SHT_REL:			/* Relocation entries, no addends */
	  if (shstrtab != NULL) {	/* don't do the following. */
	    if (strcmp(shstrtab + ntohl(elf_psh->sh_name), ".rel.stack") == 0) {
	      break;
	    }
	  }
      save_elf_ex:
	  fpos = ntohl(elf_psh->sh_offset);	/* location in file */
	  tpt = pt_start;
	  while (tpt != NULL) {
	    if (ntohl(tpt->pt_data.p_offset) <= fpos) {
	      if (ntohl(tpt->pt_data.p_offset) + ntohl(tpt->pt_data.p_filesz) > fpos) {
		break;
	      }
	    }
	    tpt = tpt->pt_next;
	  }
	  if (tpt != NULL) {
	    if (debug != 0) {
	      fprintf(stderr, "entry already in PT_LOAD save area.  Do not save it.\n");

	    }
/* 	    break; */
	  }
	  net = (struct et_list *) malloc(sizeof(struct et_list));
	  if (net == NULL) {
	    fprintf(stderr, "malloc net\n");
	    exit(1);
	  }
	  size = strlen(shstrtab + ntohl(elf_psh->sh_name)) + 1;
	  net->name = (char *) malloc(size);
	  if (net->name == NULL) {
	    fprintf(stderr, "malloc net->name\n");
	    exit(1);
	  }
	  name_size += size;
	  strcpy(net->name, shstrtab + ntohl(elf_psh->sh_name));
	  if (tpt == NULL) {
	    size = ntohl(elf_psh->sh_size);
	    net->memory = (char *) malloc(size);
	    if (net->memory == NULL) {
	      fprintf(stderr, "malloc net->memory (%d)\n", size);
	      exit(1);
	    }
	    retval = lseek(fd, fpos, SEEK_SET);
	    if (retval < 0) {
	      perror("lseek, elf_psh->sh_offset");
	      exit(1);
	    }
	    retval = read(fd, net->memory, size);
	    if (retval < 0) {
	      perror("read net->memory");
	      exit(1);
	    }
	  } else {
	    net->memory = NULL;		/* special flag that it is in tpt section */
	    net->tpt = tpt;		/* so know which one */
	    net->tpt_offset = fpos - ntohl(tpt->pt_data.p_offset);
	  }
	  bcopy((char *) elf_psh, (char *) &(net->et_data), sizeof(Elf32_Shdr));
	  n++;
	  net->et_data.sh_name = -1;
	  net->et_data.sh_addr = 0;
	  net->et_data.sh_offset = -1;	/* make -1, set later */
	  net->et_data.sh_link = 0;	/* ?? points to symtab section ?? */
	  net->et_data.sh_info = 0;	/* ?? points to section (.rodata, .text,...) ?? */
	  net->et_next = NULL;
	  if (et_start == NULL) {
	    et_start = net;
	  } else {
	    et_last->et_next = net;
	  }
	  et_last = net;
	  break;
	case SHT_MIPS_REGINFO:		/* register usage information */
	  goto save_elf_ex;
	case SHT_NULL:			/* Section header table entry unused */
	  break;
	case SHT_SYMTAB:		/* A symbol table */
	  break;
	case SHT_DYNAMIC:		/* Dynamic linking information */
	  break;
	case SHT_HASH:			/* Section header table entry unused */
	  break;
	case SHT_DYNSYM:		/* Section header table entry unused */
	  break;
	case SHT_MIPS_DEBUG:		/* MIPS ECOFF debugging information */
	  break;
	case SHT_MIPS_DWARF:		/* Section header table entry unused */
	  break;

	default:
	  fprintf(stderr, "unknown SHT_  =0x%x (%d)\n", ntohl(elf_psh->sh_type), ntohl(elf_psh->sh_type));
	  break;
    }
  }
  if (et_start == NULL) {
    et_start = shstr;
  } else {
    et_last->et_next = shstr;
  }
  et_last = shstr;
/* ------------------------------------------------------------------------ */
#ifdef OBJDUMP_COMPAT
 /* Note: null section */
  new_elf_ex.e_shnum = ntohs(n + 2);	/* Number of Section headers */
  new_elf_ex.e_shstrndx = ntohs(n + 1);	/* Section header string table index. zero based. */
/* name_size needs to be created, with alignment of 1 (2^1); */
  name_size = (name_size + 1 + 1) & (~1);	/* +1 for null section, +1 for alignment */
#else
  new_elf_ex.e_shnum = ntohs(n + 1);	/* Number of Section headers */
  new_elf_ex.e_shstrndx = ntohs(n);	/* Section header string table index. zero based. */
/* name_size needs to be created, with alignment of 1 (2^1); */
  name_size = (name_size + 1) & (~1);	/* +1 for alignment */
#endif
  shstr->et_data.sh_size = ntohl(name_size);
  shstr->memory = (char *) malloc(name_size);
  if (shstr->memory == NULL) {
    fprintf(stderr, "malloc shstr->memory (%d)\n", name_size);
    exit(1);
  }
/* Fill in the name table, and the entries that point to these. */
  tet = et_start;
#ifdef OBJDUMP_COMPAT
  *(shstr->memory) = '\0';		/* null section name */
  i = 1;
#else
  i = 0;
#endif
  while (tet != NULL) {
    tet->et_data.sh_name = ntohl(i);
    strcpy(shstr->memory + i, tet->name);
    i += strlen(tet->name) + 1;		/* +1 for end of string */
    tet = tet->et_next;
  }
/* Calculate offsets for pt_load section(s), and section headers. */
  i = sizeof(new_elf_ex);		/* put data after this structure */
  new_elf_ex.e_phoff = ntohl(i);	/* Program header offset elf_phdr structure */
  tpt = pt_start;
  while (tpt != NULL) {
    i = i + sizeof(struct elf32_phdr);
    tpt = tpt->pt_next;
  }
/* At this point, i = start of offset for PT_LOAD's. */
  tpt = pt_start;
  while (tpt != NULL) {
    tpt->pt_data.p_offset = ntohl(i);	/* starting location */
 /* ignore alignment, should be the same. */
    i = i + ntohl(tpt->pt_data.p_filesz);	/* size of memory section */
    tpt = tpt->pt_next;
  }
/* i = where sections headers start. */
/* Order really doesn't matter, put them in order, save seeking on disks. */
/* 1) section headers themself, 2) data for section headers -- ignore alignments */
  new_elf_ex.e_shoff = ntohl(i);	/* Section header offset Elf32_Shdr structures */
  tet = et_start;
  while (tet != NULL) {
    i += sizeof(Elf32_Shdr);
    tet = tet->et_next;
  }
#ifdef OBJDUMP_COMPAT
  i += sizeof(Elf32_Shdr);		/* null first entry */
#endif
/* i = where first section will be placed. */
  tet = et_start;
  while (tet != NULL) {
    if (tet->memory != NULL) {
      tet->et_data.sh_offset = ntohl(i);
      i += ntohl(tet->et_data.sh_size);
    } else {				/* data section already loaded/exists. */
      tet->et_data.sh_offset = ntohl(ntohl(tet->tpt->pt_data.p_offset) + net->tpt_offset);
    }
    tet = tet->et_next;
  }
/* Reduce the size of the file. */
  n = ftruncate(fd, i);
  if (n < 0) {
    perror("ftruncate");
    exit(1);
  }
/* ------------------------------------------------------------------------ */
/* Now write out the file. */
  if (debug == 1) {			/* do not write if in debug mode */
    return;
  }
  fpos = 0;
  retval = lseek(fd, fpos, SEEK_SET);
  if (retval < 0) {
    perror("lseek elf_shdata");
    exit(1);
  }
/* Write out the new_elf_ex structure. */
  n = write(fd, (char *) &new_elf_ex, sizeof(new_elf_ex));
  if (n < 0) {
    perror("write new_elf_ex structure");
    exit(1);
  }
  if (debug != 0) {
    fprintf(stderr, "new_elf_ex: e_type=0x%04x e_machine=0x%04x e_version=0x%08x e_entry=0x%08x e_phoff=0x%08x e_shoff=0x%08x e_flags=0x%08x e_ehsize=0x%04x e_phentsize=0x%04x e_phnum=0x%04x e_shentsize=0x%04x e_shnum=0x%04x e_shstrndx=0x%04x\n", ntohs(new_elf_ex.e_type), ntohs(new_elf_ex.e_machine), ntohl(new_elf_ex.e_version), ntohl(new_elf_ex.e_entry), ntohl(new_elf_ex.e_phoff), ntohl(new_elf_ex.e_shoff), ntohl(new_elf_ex.e_flags), ntohs(new_elf_ex.e_ehsize), ntohs(new_elf_ex.e_phentsize), ntohs(new_elf_ex.e_phnum), ntohs(new_elf_ex.e_shentsize), ntohs(new_elf_ex.e_shnum), ntohs(new_elf_ex.e_shstrndx));
  }
/* write out new pt_load section(s) */
  tpt = pt_start;
  if (debug != 0) {
    i = 0;
  }
  while (tpt != NULL) {
    n = write(fd, (char *) &tpt->pt_data, sizeof(struct elf32_phdr));
    if (n < 0) {
      perror("write elf32_phdr structure");
      exit(1);
    }
    if (debug != 0) {
      fprintf(stderr, "i=%d of %d, p_type=%x, p_offset=0x%x\n", i, ntohs(new_elf_ex.e_phnum), ntohl(tpt->pt_data.p_type), ntohl(tpt->pt_data.p_offset));
      i++;
    }
    tpt = tpt->pt_next;
  }
/* write out data for pt_load section(s). */
  tpt = pt_start;
  while (tpt != NULL) {
    n = write(fd, tpt->memory, ntohl(tpt->pt_data.p_filesz));
    if (n < 0) {
      perror("write data for PT_LOAD section");
      exit(1);
    }
    if (debug != 0) {
      fprintf(stderr, "tpt->memory=0x%x, p_filesz=%d\n", (int) (tpt->memory), ntohl(tpt->pt_data.p_filesz));
    }
    tpt = tpt->pt_next;
  }
/* write out section header(s). */
#ifdef OBJDUMP_COMPAT
  bzero(buf, sizeof(Elf32_Shdr));
  n = write(fd, buf, sizeof(Elf32_Shdr));	/* null first entry */
  if (n < 0) {
    perror("write null section header");
    exit(1);
  }
#endif
  tet = et_start;
  while (tet != NULL) {
    n = write(fd, (char *) &tet->et_data, sizeof(Elf32_Shdr));
    if (n < 0) {
      perror("write section headers");
      exit(1);
    }
    if (debug != 0) {
      fprintf(stderr, "name=%x, type=%x, flags=%x, addr=%x, off=%x, size=%x, link=%x, info=%x, align=%x, entsize=%x\n",
	      ntohl(tet->et_data.sh_name), ntohl(tet->et_data.sh_type),
	      ntohl(tet->et_data.sh_flags), ntohl(tet->et_data.sh_addr), ntohl(tet->et_data.sh_offset),
	      ntohl(tet->et_data.sh_size), ntohl(tet->et_data.sh_link), ntohl(tet->et_data.sh_info),
	      ntohl(tet->et_data.sh_addralign), ntohl(tet->et_data.sh_entsize));
    }
    tet = tet->et_next;
  }
/* write out section(s) data. */
  tet = et_start;
  while (tet != NULL) {
    if (debug != 0) {
      fprintf(stderr, "tet->memory=0x%x\n", (int) (tet->memory));
    }
    if (tet->memory != NULL) {
      n = write(fd, tet->memory, ntohl(tet->et_data.sh_size));
      if (n < 0) {
	perror("write data for section headers");
	exit(1);
      }
    }
    tet = tet->et_next;
  }
  close(fd);
}					/* end of load_elf_binary */

/* ------------------------------------------------------------------------ */
int             main(int argc, char *argv[])
{
  int             fd;
  int             i;

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Need file name as an argument\n");
    exit(1);
  }
  if (argc == 3) {
    if (strcmp("-d", argv[1]) == 0) {
      debug = 1;
      i = 2;
    } else {
      if (strcmp("-dd", argv[1]) == 0) {
	debug = 2;
	i = 2;
      } else {
	fprintf(stderr, "Not -d argument.  (%s)\n", argv[1]);
	exit(1);
      }
    }
  } else {
    i = 1;
  }
  if ((fd = open(argv[i], O_RDWR, 0)) < 0) {
    perror("open");
    exit(1);
  }
  i = read(fd, buf, sizeof(buf));
  if (i < 0) {
    perror("read");
    exit(1);
  }
  load_elf_binary(fd);
  exit(0);
}					/* end of main */

/* ------------------------------------------------------------------------ */
