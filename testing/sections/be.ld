/* Script for -z combreloc: combine and sort reloc sections */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SEARCH_DIR("/usr/i586-suse-linux/lib");  SEARCH_DIR("/usr/local/lib"); SEARCH_DIR("/lib"); SEARCH_DIR("/usr/lib");
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  PROVIDE (__executable_start = 0x08048000);
  /*. = 0x08048000 + SIZEOF_HEADERS;*/
  . = 0x08048000 + 512;
PROVIDE (LOADER_INTERP_start = .);
  .interp         : { *(.interp) }
PROVIDE (LOADER_INTERP_stop = .);
PROVIDE (LOADER_HASH_start = .);
  .hash           : { *(.hash) }
PROVIDE (LOADER_HASH_stop = .);
PROVIDE (LOADER_DYNSYM_start = .);
  .dynsym         : { *(.dynsym) }
PROVIDE (LOADER_DYNSYM_stop = .);
PROVIDE (LOADER_DYNSTR_start = .);
  .dynstr         : { *(.dynstr) }
PROVIDE (LOADER_DYNSTR_stop = .);
PROVIDE (LOADER_GNUVERSION_start = .);
  .gnu.version    : { *(.gnu.version) }
PROVIDE (LOADER_GNUVERSION_stop = .);
PROVIDE (LOADER_GNUVERSION_D_start = .);
  .gnu.version_d  : { *(.gnu.version_d) }
PROVIDE (LOADER_GNUVERSION_D_stop = .);
PROVIDE (LOADER_GNUVERSION_R_start = .);
  .gnu.version_r  : { *(.gnu.version_r) }
PROVIDE (LOADER_GNUVERSION_R_stop = .);

PROVIDE (LOADER_RELDYN_start = .);
  .rel.dyn        :
    {
      *(.rel.init)
      *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
      *(.rel.fini)
      *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
      *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
      *(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
      *(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
      *(.rel.ctors)
      *(.rel.dtors)
      *(.rel.got)
      *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
    }
PROVIDE (LOADER_RELDYN_stop = .);

PROVIDE (LOADER_RELADYN_start = .);
  .rela.dyn       :
    {
      *(.rela.init)
      *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
      *(.rela.fini)
      *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
      *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
      *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
      *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
      *(.rela.ctors)
      *(.rela.dtors)
      *(.rela.got)
      *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
    }
PROVIDE (LOADER_RELADYN_stop = .);
PROVIDE (LOADER_RELPLT_start = .);
  .rel.plt        : { *(.rel.plt) }
PROVIDE (LOADER_RELPLT_stop = .);
PROVIDE (LOADER_RELAPLT_start = .);
  .rela.plt       : { *(.rela.plt) }
PROVIDE (LOADER_RELAPLT_stop = .);

PROVIDE (LOADER_INIT_start = .);
  .init           :
  {
    KEEP (*(.init))
  } =0x90909090
PROVIDE (LOADER_INIT_stop = .);

PROVIDE (LOADER_PLT_start = .);
  .plt            : { *(.plt) }
PROVIDE (LOADER_PLT_stop = .);
/*  PROVIDE (CT_start = .); */
/*  .codetranslated : { ../obj_7000Debug/b_back.CT.o(.text) } */
/*  PROVIDE (CT_end = .); */

PROVIDE (LOADER_TEXT_start = .);
  .text           :
  {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  } =0x90909090
PROVIDE (LOADER_TEXT_stop = .);

PROVIDE (LOADER_FINI_start = .);
  .fini           :
  {
    KEEP (*(.fini))
  } =0x90909090
PROVIDE (LOADER_FINI_stop = .);

  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
PROVIDE (LOADER_RODATA_start = .);
  .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
PROVIDE (LOADER_RODATA_stop = .);
PROVIDE (LOADER_RODATA1_start = .);
  .rodata1        : { *(.rodata1) }
PROVIDE (LOADER_RODATA1_stop = .);
PROVIDE (LOADER_EHFRAMEHDR_start = .);
  .eh_frame_hdr : { *(.eh_frame_hdr) }
PROVIDE (LOADER_EHFRAMEHDR_stop = .);
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  . = ALIGN (0x1000) - ((0x1000 - .) & (0x1000 - 1)); . = DATA_SEGMENT_ALIGN (0x1000, 0x1000);
  /* Ensure the __preinit_array_start label is properly aligned.  We
     could instead move the label definition inside the section, but
     the linker would then create the section even if it turns out to
     be empty, which isn't pretty.  */
  . = ALIGN(32 / 8);
  PROVIDE (__preinit_array_start = .);
  .preinit_array     : { *(.preinit_array) }
  PROVIDE (__preinit_array_end = .);
  PROVIDE (__init_array_start = .);
  .init_array     : { *(.init_array) }
  PROVIDE (__init_array_end = .);
  PROVIDE (__fini_array_start = .);
  .fini_array     : { *(.fini_array) }
  PROVIDE (__fini_array_end = .);
PROVIDE (LOADER_DATA_start = .);
  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
    PROVIDE (CT_start_shdata = .);
    *(.shdata)
    PROVIDE (CT_end_shdata = .);
    PROVIDE (CT_start_shinittable = .);
    *(.shinittable)
    PROVIDE (CT_end_shinittable = .);
    SORT(CONSTRUCTORS)
  }
PROVIDE (LOADER_DATA_stop = .);
PROVIDE (LOADER_DATA1_start = .);
  .data1          : { *(.data1) }
PROVIDE (LOADER_DATA1_stop = .);
PROVIDE (LOADER_TDATA_start = .);
  .tdata   : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
PROVIDE (LOADER_TDATA_stop = .);
PROVIDE (LOADER_TBSS_start = .);
  .tbss    : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
PROVIDE (LOADER_TBSS_stop = .);
PROVIDE (LOADER_EHFRAME_start = .);
  .eh_frame       : { KEEP (*(.eh_frame)) }
PROVIDE (LOADER_EHFRAME_stop = .);
PROVIDE (LOADER_GCCEXCEPTTABLE_start = .);
  .gcc_except_table   : { *(.gcc_except_table) }
PROVIDE (LOADER_GCCEXCEPTTABLE_stop = .);
PROVIDE (LOADER_DYNAMIC_start = .);
  .dynamic        : { *(.dynamic) }
PROVIDE (LOADER_DYNAMIC_stop = .);
PROVIDE (LOADER_CTORS_start = .);
  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin*.o(.ctors))
    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  }
PROVIDE (LOADER_CTORS_stop = .);
PROVIDE (LOADER_DTORS_start = .);
  .dtors          :
  {
    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  }
PROVIDE (LOADER_DTORS_stop = .);
PROVIDE (LOADER_JCR_start = .);
  .jcr            : { KEEP (*(.jcr)) }
PROVIDE (LOADER_JCR_stop = .);
PROVIDE (LOADER_GOT_start = .);
  .got            : { *(.got.plt) *(.got) }
PROVIDE (LOADER_GOT_stop = .);
  _edata = .;
  PROVIDE (edata = .);
PROVIDE (LOADER_BSS_start = .);
  __bss_start = .;
  .bss            :
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections.  */
   . = ALIGN(32 / 8);
  }
PROVIDE (LOADER_BSS_stop = .);
  . = ALIGN(32 / 8);
  _end = .;
  PROVIDE (end = .);
/*  . = DATA_SEGMENT_END (.); */
. = 0;
PROVIDE (LOADER_STAB_start = .);
  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
PROVIDE (LOADER_STAB_stop = .);
. = 0;
PROVIDE (LOADER_COMMENT_start = .);
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
PROVIDE (LOADER_COMMENT_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_start = .);
  .debug          0 : { *(.debug) }
PROVIDE (LOADER_DEBUG_stop = .);
. = 0;
PROVIDE (LOADER_LINE_start = .);
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
PROVIDE (LOADER_LINE_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_SRCINFO_start = .);
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
PROVIDE (LOADER_DEBUG_SRCINFO_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_SFNAMES_start = .);
  .debug_sfnames  0 : { *(.debug_sfnames) }
PROVIDE (LOADER_DEBUG_SFNAMES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_ARANGES_start = .);
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
PROVIDE (LOADER_DEBUG_ARANGES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_PUBNAMES_start = .);
  .debug_pubnames 0 : { *(.debug_pubnames) }
PROVIDE (LOADER_DEBUG_PUBNAMES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_INFO_start = .);
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
PROVIDE (LOADER_DEBUG_INFO_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_ABBREV_start = .);
  .debug_abbrev   0 : { *(.debug_abbrev) }
PROVIDE (LOADER_DEBUG_ABBREV_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_LINE_start = .);
  .debug_line     0 : { *(.debug_line) }
PROVIDE (LOADER_DEBUG_LINE_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_FRANE_start = .);
  .debug_frame    0 : { *(.debug_frame) }
PROVIDE (LOADER_DEBUG_FRANE_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_STR_start = .);
  .debug_str      0 : { *(.debug_str) }
PROVIDE (LOADER_DEBUG_STR_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_LOC_start = .);
  .debug_loc      0 : { *(.debug_loc) }
PROVIDE (LOADER_DEBUG_LOC_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_MACINFO_start = .);
  .debug_macinfo  0 : { *(.debug_macinfo) }
PROVIDE (LOADER_DEBUG_MACINFO_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_WEAKNAMES_start = .);
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
PROVIDE (LOADER_DEBUG_WEAKNAMES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_FUNCNAMES_start = .);
  .debug_funcnames 0 : { *(.debug_funcnames) }
PROVIDE (LOADER_DEBUG_FUNCNAMES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_TYPENAMES_start = .);
  .debug_typenames 0 : { *(.debug_typenames) }
PROVIDE (LOADER_DEBUG_TYPENAMES_stop = .);
. = 0;
PROVIDE (LOADER_DEBUG_VARNAMES_start = .);
  .debug_varnames  0 : { *(.debug_varnames) }
PROVIDE (LOADER_DEBUG_VARNAMES_stop = .);
  /DISCARD/ : { *(.note.GNU-stack) }
PROVIDE (LOADER_DEBUG_ALL_stop = .);
}
