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

#include <elf.h>
/* #include <sys/exec_elf.h> */

/* ------------------------------------------------------------------------ */
/* #define LIST_NTOHS(x)	ntohs(x) */
/* #define LIST_NTOHL(x)	ntohl(x) */

#define list_NTOHS(x)	(x)
#define list_NTOHL(x)	(x)
/* ------------------------------------------------------------------------ */
static struct pt_list {
  char           *memory;
  Elf32_Phdr      pt_data;		/* Entry for entry. */
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
  Elf32_Ehdr   elf_ex;
  Elf32_Ehdr   new_elf_ex;
  int             fpos;
  char           *shstrtab = NULL;	/* section names */
  Elf32_Phdr       *elf_phdata = NULL;
/*   Elf_Phdr *new_elf_phdata = NULL; */
  Elf32_Phdr       *elf_ppnt;
  struct pt_list *npt;
  struct pt_list *tpt;
  struct et_list *net;
  struct et_list *shstr;
  struct et_list *tet;
  int             name_size = 0;

  fprintf(stderr, "Elf32_Ehdr=%x(%d), Elf32_Phdr=%x(%d), Elf32_Shdr=%x(%d)\n",
/* , Elf32_RegInfo=%x(%d)\n", */
	  sizeof(Elf32_Ehdr), sizeof(Elf32_Ehdr), sizeof(Elf32_Phdr), sizeof(Elf32_Phdr), sizeof(Elf32_Shdr), sizeof(Elf32_Shdr));
/* , sizeof(Elf32_RegInfo), sizeof(Elf32_RegInfo)); */
/* --------------- */
  elf_ex = *((Elf32_Ehdr *) buf);	/* Get the exec-header */
  fprintf(stderr, "elf_ex: e_type=0x%04x e_machine=0x%04x e_version=0x%08x e_entry=0x%08x e_phoff=0x%08x e_shoff=0x%08x e_flags=0x%08x e_ehsize=0x%04x e_phentsize=0x%04x e_phnum=0x%04x e_shentsize=0x%04x e_shnum=0x%04x e_shstrndx=0x%04x\n", list_NTOHS(elf_ex.e_type), list_NTOHS(elf_ex.e_machine), list_NTOHL(elf_ex.e_version), list_NTOHL(elf_ex.e_entry), list_NTOHL(elf_ex.e_phoff), list_NTOHL(elf_ex.e_shoff), list_NTOHL(elf_ex.e_flags), list_NTOHS(elf_ex.e_ehsize), list_NTOHS(elf_ex.e_phentsize), list_NTOHS(elf_ex.e_phnum), list_NTOHS(elf_ex.e_shentsize), list_NTOHS(elf_ex.e_shnum), list_NTOHS(elf_ex.e_shstrndx));
/* --------------- */
/* First of all, some simple consistency checks */
  if (memcmp(elf_ex.e_ident, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "e_ident: check failed\n");
    exit(1);
  }
  switch (list_NTOHS(elf_ex.e_type)) {
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
	fprintf(stderr, "e_type: check failed %x (%d) != (%d || %d)\n", list_NTOHS(elf_ex.e_type), list_NTOHS(elf_ex.e_type), ET_EXEC, ET_DYN);
	exit(1);
  }
  if (elf_ex.e_machine != EM_386) {
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
  new_elf_ex.e_phoff = -1;		/* Program header offset Elf32_Phdr structure */
  new_elf_ex.e_shoff = -2;		/* Section header offset Elf32_Shdr structures */
  new_elf_ex.e_flags = elf_ex.e_flags;	/* Processor specific flags */
  new_elf_ex.e_ehsize = elf_ex.e_ehsize;/* ELF header size in bytes */
  new_elf_ex.e_phentsize = elf_ex.e_phentsize;	/* Program header table entry size. */
  new_elf_ex.e_phnum = -1;		/* Number of Program header sections. */
  new_elf_ex.e_shentsize = elf_ex.e_shentsize;	/* Section header table entry size */
  new_elf_ex.e_shnum = -1;		/* Number of Section headers */
  new_elf_ex.e_shstrndx = -1;		/* Section header string table index. zero based. */
/* --------------- */
  size = list_NTOHS(elf_ex.e_shentsize) * list_NTOHS(elf_ex.e_shnum);
  if (size > PARTIAL_SIZE || size < 4) {
    fprintf(stderr, "elf_shdata size=%d (%d * %d)\n", size, list_NTOHS(elf_ex.e_shentsize), list_NTOHS(elf_ex.e_shnum));
    exit(1);
  }
  fpos = list_NTOHL(elf_ex.e_shoff);
  elf_shdata = (Elf32_Shdr *) malloc(size);
  if (elf_shdata == NULL) {
    fprintf(stderr, "malloc, elf_shdata size=%d (%d * %d)\n", size, list_NTOHS(elf_ex.e_shentsize), list_NTOHS(elf_ex.e_shnum));
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
    fprintf(stderr, "malloc, new_elf_shdata size=%d (%d * %d)\n", size, list_NTOHS(elf_ex.e_shentsize), list_NTOHS(elf_ex.e_shnum));
    exit(1);
  }
/* --------------- */
/* Read the shstrndx (.shstrtab) table. */
  elf_psh = elf_shdata + list_NTOHS(elf_ex.e_shstrndx);
  switch (list_NTOHL(elf_psh->sh_type)) {
      case SHT_STRTAB:			/* A string table */
	size = list_NTOHL(elf_psh->sh_size);
	fpos = list_NTOHL(elf_psh->sh_offset);
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
	   list_NTOHS(elf_ex.e_shstrndx), list_NTOHL(elf_psh->sh_type), list_NTOHL(elf_psh->sh_type));
	exit(1);
  }
/* --------------- */
/* Read in all of the Program header information */
  size = list_NTOHS(elf_ex.e_phentsize) * list_NTOHS(elf_ex.e_phnum);
  if (size > PARTIAL_SIZE || size < 4) {
    fprintf(stderr, "elf_phdata size=%d (%d * %d)\n", size, list_NTOHS(elf_ex.e_phentsize), list_NTOHS(elf_ex.e_phnum));
    exit(1);
  }
  fpos = list_NTOHL(elf_ex.e_phoff);	/* location in file */
  elf_phdata = (Elf32_Phdr *) malloc(size);
  if (!elf_phdata) {
    fprintf(stderr, "malloc elf_phdata size=%d (%d * %d)\n", size, list_NTOHS(elf_ex.e_phentsize), list_NTOHS(elf_ex.e_phnum));
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
/* fprintf(stderr, "elf_ex.e_phnum=%d\n", list_NTOHS(elf_ex.e_phnum)); */
  for (i = 0, n = 0, elf_ppnt = elf_phdata; i < list_NTOHS(elf_ex.e_phnum); i++, elf_ppnt++) {
    char           *p = "Unknown";
    static char    *pt_convert[PT_NUM] = {
      "PT_NULL",
      "PT_LOAD",
      "PT_DYNAMIC",
      "PT_INTERP",
      "PT_NOTE",
      "PT_SHLIB",
      "PT_PHDR",
    };
    if (list_NTOHL(elf_ppnt->p_type) >= 0 &&
	list_NTOHL(elf_ppnt->p_type) < PT_NUM) {
      p = pt_convert[list_NTOHL(elf_ppnt->p_type)];
    }
    fprintf(stderr, "i=%d of %d, p_type=%x (%s), offset=0x%x, filesz=0x%x\n",
	    i, list_NTOHS(elf_ex.e_phnum), list_NTOHL(elf_ppnt->p_type), p,
	    list_NTOHL(elf_ppnt->p_offset), list_NTOHL(elf_ppnt->p_filesz));

    if (list_NTOHL(elf_ppnt->p_type) == PT_LOAD) {
      npt = (struct pt_list *) malloc(sizeof(struct pt_list));
      if (npt == NULL) {
	fprintf(stderr, "malloc npt\n");
	exit(1);
      }
      size = list_NTOHL(elf_ppnt->p_filesz);
      npt->memory = (char *) malloc(size);
      if (npt->memory == NULL) {
	fprintf(stderr, "malloc npt->memory (%d)\n", size);
	exit(1);
      }
      fpos = list_NTOHL(elf_ppnt->p_offset);	/* location in file */
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
      bcopy((char *) elf_ppnt, (char *) &(npt->pt_data), sizeof(Elf32_Phdr));
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
  new_elf_ex.e_phnum = list_NTOHS(n);	/* Number of Program header sections. */
/* --------------- */
/* Go through section tables. */
  for (i = 0, n = 0, elf_psh = elf_shdata; i < list_NTOHS(elf_ex.e_shnum); i++, elf_psh++) {
    fprintf(stderr, "%2.2d ", i);
    if (shstrtab != NULL) {
      fprintf(stderr, "%s: ", shstrtab + list_NTOHL(elf_psh->sh_name));
    }
    fprintf(stderr, "off = %d  size = %d\n   ", list_NTOHL(elf_psh->sh_offset), list_NTOHL(elf_psh->sh_size));
    fprintf(stderr, "name=%x, type=%x, flags=%x, addr=%x, off=%x, size=%x, link=%x, info=%x, align=%x, entsize=%x\n",
	    list_NTOHL(elf_psh->sh_name), list_NTOHL(elf_psh->sh_type),
	  list_NTOHL(elf_psh->sh_flags), list_NTOHL(elf_psh->sh_addr), list_NTOHL(elf_psh->sh_offset),
	    list_NTOHL(elf_psh->sh_size), list_NTOHL(elf_psh->sh_link), list_NTOHL(elf_psh->sh_info),
	    list_NTOHL(elf_psh->sh_addralign), list_NTOHL(elf_psh->sh_entsize));
    switch (list_NTOHL(elf_psh->sh_type)) {
	 /* 1 */ case SHT_PROGBITS:	/* private */
	  switch (list_NTOHL(elf_psh->sh_flags)) {
	      case 0:
		if (shstrtab != NULL) {
		  if (strcmp(shstrtab + list_NTOHL(elf_psh->sh_name), ".stack") == 0) {
		    goto save_elf_ex;
		  }
		}
		break;
	      case SHF_WRITE:		/* 1  writeable */
		break;
	      case SHF_ALLOC:		/* 2  occupies memory */
		break;
	      case SHF_ALLOC | SHF_WRITE:	/* 3 */
		break;
	      case SHF_EXECINSTR:	/* 4  executable */
		break;
	      case SHF_EXECINSTR | SHF_ALLOC:	/* 6 = relocatable .text (read-only) */
	      case SHF_EXECINSTR | SHF_ALLOC | SHF_WRITE:	/* 7 = executable .text (writable) */
		break;
	      case 0x10000003:		/* GP rel, alloc writable. */
		break;
	      default:
		fprintf(stderr, "SHT_PROGBITS, unknown flag types 0x%x\n", list_NTOHL(elf_psh->sh_flags));
		break;
	  }
	  break;

	 /* 8 */ case SHT_NOBITS:	/* Section occupies no space in file */
	  switch (list_NTOHL(elf_psh->sh_flags)) {
	      case 3:
		break;
	      case 0x10000003:
		break;
	      default:
		fprintf(stderr, "elf: SHT_NOBITS unknown flag types (%x)\n", list_NTOHL(elf_psh->sh_flags));
		break;
	  }
	  break;

	 /* 3 */ case SHT_STRTAB:	/* A string table */
	  if (i == list_NTOHS(elf_ex.e_shstrndx)) {
	    shstr = (struct et_list *) malloc(sizeof(struct et_list));
	    if (shstr == NULL) {
	      fprintf(stderr, "malloc shstr\n");
	      exit(1);
	    }
	    size = strlen(shstrtab + list_NTOHL(elf_psh->sh_name)) + 1;
	    shstr->name = (char *) malloc(size);
	    if (shstr->name == NULL) {
	      fprintf(stderr, "malloc shstr->name\n");
	      exit(1);
	    }
	    name_size += size;
	    strcpy(shstr->name, shstrtab + list_NTOHL(elf_psh->sh_name));
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

	 /* 4 */ case SHT_RELA:	/* Relocation entries, with addends */
	  break;
	 /* 9 */ case SHT_REL:		/* Relocation entries, no addends */
	  if (shstrtab != NULL) {	/* don't do the following. */
	    if (strcmp(shstrtab + list_NTOHL(elf_psh->sh_name), ".rel.stack") == 0) {
	      break;
	    }
	  }
      save_elf_ex:
	  fpos = list_NTOHL(elf_psh->sh_offset);	/* location in file */
	  tpt = pt_start;
	  while (tpt != NULL) {
	    if (list_NTOHL(tpt->pt_data.p_offset) <= fpos) {
	      if (list_NTOHL(tpt->pt_data.p_offset) + list_NTOHL(tpt->pt_data.p_filesz) > fpos) {
		break;
	      }
	    }
	    tpt = tpt->pt_next;
	  }
	  if (tpt != NULL) {
	    fprintf(stderr, "entry already in PT_LOAD save area.  Do not save it.\n");
/* 	    break; */
	  }
	  net = (struct et_list *) malloc(sizeof(struct et_list));
	  if (net == NULL) {
	    fprintf(stderr, "malloc net\n");
	    exit(1);
	  }
	  size = strlen(shstrtab + list_NTOHL(elf_psh->sh_name)) + 1;
	  net->name = (char *) malloc(size);
	  if (net->name == NULL) {
	    fprintf(stderr, "malloc net->name\n");
	    exit(1);
	  }
	  name_size += size;
	  strcpy(net->name, shstrtab + list_NTOHL(elf_psh->sh_name));
	  if (tpt == NULL) {
	    size = list_NTOHL(elf_psh->sh_size);
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
	    net->tpt_offset = fpos - list_NTOHL(tpt->pt_data.p_offset);
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
 /* case SHT_MIPS_REGINFO: *//* register usage information */
/*	  goto save_elf_ex; */
	 /* 0 */ case SHT_NULL:	/* Section header table entry unused */
	  break;
	 /* 2 */ case SHT_SYMTAB:	/* A symbol table */
	  break;
	 /* 6 */ case SHT_DYNAMIC:	/* Dynamic linking information */
	  break;
	 /* 5 */ case SHT_HASH:	/* Section header table entry unused */
	  break;
	 /* 11 */ case SHT_DYNSYM:	/* Section header table entry unused */
	  break;
 /* case SHT_MIPS_DEBUG: *//* MIPS ECOFF debugging information */
/* 	  break; */
 /* case SHT_MIPS_DWARF: *//* Section header table entry unused */
/* 	  break; */
	case SHT_NOTE:			/* note section */
	  break;
	case SHT_SHLIB:		/* reserved - purpose unknown */
	  break;

	default:
	  fprintf(stderr, "unknown SHT_  =0x%x (%d)\n", list_NTOHL(elf_psh->sh_type), list_NTOHL(elf_psh->sh_type));
	  break;
    }
  }

/* ------------------------------------------------------------------------ */
  return;
  close(fd);
}					/* end of load_elf_binary */

/* ------------------------------------------------------------------------ */
int             main(int argc, char *argv[])
{
  int             fd;
  int             i;

  if (argc != 2) {
    fprintf(stderr, "Need file name as an argument\n");
    exit(1);
  }
  if ((fd = open(argv[1], O_RDWR, 0)) < 0) {
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
