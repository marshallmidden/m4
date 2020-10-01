
m.o:     file format elf32-i386
m.o
architecture: i386, flags 0x00000011:
HAS_RELOC, HAS_SYMS
start address 0x00000000

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000000bf  00000000  00000000  00000034  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000000  00000000  00000000  000000f4  2**2
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  00000000  00000000  000000f4  2**2
                  ALLOC
  3 .debug_abbrev 00000103  00000000  00000000  000000f4  2**0
                  CONTENTS, READONLY, DEBUGGING
  4 .debug_info   0000049e  00000000  00000000  000001f7  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  5 .debug_line   0000007d  00000000  00000000  00000695  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  6 .rodata       00000048  00000000  00000000  00000712  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  7 .debug_frame  0000003c  00000000  00000000  0000075c  2**2
                  CONTENTS, RELOC, READONLY, DEBUGGING
  8 .debug_pubnames 0000001b  00000000  00000000  00000798  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  9 .debug_aranges 00000020  00000000  00000000  000007b3  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
 10 .debug_str    00000016  00000000  00000000  000007d3  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .note.GNU-stack 00000000  00000000  00000000  000007e9  2**0
                  CONTENTS, READONLY
 12 .comment      0000001f  00000000  00000000  000007e9  2**0
                  CONTENTS, READONLY
SYMBOL TABLE:
00000000 l    df *ABS*	00000000 m.c
00000000 l    d  .text	00000000 
00000000 l    d  .data	00000000 
00000000 l    d  .bss	00000000 
00000000 l    d  .debug_abbrev	00000000 
00000000 l    d  .debug_info	00000000 
00000000 l    d  .debug_line	00000000 
00000000 l    d  .rodata	00000000 
00000000 l    d  .debug_frame	00000000 
00000000 l    d  .debug_pubnames	00000000 
00000000 l    d  .debug_aranges	00000000 
00000000 l    d  .debug_str	00000000 
00000000 l    d  .note.GNU-stack	00000000 
00000000 l    d  .comment	00000000 
00000000 g     F .text	000000bf main
00000000         *UND*	00000000 stderr
00000000         *UND*	00000000 fprintf
00000000         *UND*	00000000 a
00000000         *UND*	00000000 exit


Disassembly of section .text:

00000000 <main>:
main():
/home/marshall_midden/src/testing/calling/m.c:4
#include <stdio.h>
#include "local.h"
int main()
{
   0:	55                   	push   %ebp
   1:	89 e5                	mov    %esp,%ebp
   3:	83 ec 18             	sub    $0x18,%esp
   6:	83 e4 f0             	and    $0xfffffff0,%esp
   9:	b8 00 00 00 00       	mov    $0x0,%eax
   e:	83 c0 0f             	add    $0xf,%eax
  11:	83 c0 0f             	add    $0xf,%eax
  14:	c1 e8 04             	shr    $0x4,%eax
  17:	c1 e0 04             	shl    $0x4,%eax
  1a:	29 c4                	sub    %eax,%esp
/home/marshall_midden/src/testing/calling/m.c:6
  int w,x,y,z;
  w = 0;
  1c:	c7 45 fc 00 00 00 00 	movl   $0x0,0xfffffffc(%ebp)
/home/marshall_midden/src/testing/calling/m.c:7
  x = 1;
  23:	c7 45 f8 01 00 00 00 	movl   $0x1,0xfffffff8(%ebp)
/home/marshall_midden/src/testing/calling/m.c:8
  y = 2;
  2a:	c7 45 f4 02 00 00 00 	movl   $0x2,0xfffffff4(%ebp)
/home/marshall_midden/src/testing/calling/m.c:9
  z = 3;
  31:	c7 45 f0 03 00 00 00 	movl   $0x3,0xfffffff0(%ebp)
/home/marshall_midden/src/testing/calling/m.c:10
  fprintf(stderr, "in m  ebp=%p  esp=%p\n", get_ebp(), get_esp());
  38:	89 e0                	mov    %esp,%eax
  3a:	89 45 ec             	mov    %eax,0xffffffec(%ebp)
  3d:	8b 55 ec             	mov    0xffffffec(%ebp),%edx
  40:	89 e8                	mov    %ebp,%eax
  42:	89 45 ec             	mov    %eax,0xffffffec(%ebp)
  45:	8b 45 ec             	mov    0xffffffec(%ebp),%eax
  48:	52                   	push   %edx
  49:	50                   	push   %eax
  4a:	68 00 00 00 00       	push   $0x0
			4b: R_386_32	.rodata
  4f:	ff 35 00 00 00 00    	pushl  0x0
			51: R_386_32	stderr
  55:	e8 fc ff ff ff       	call   56 <main+0x56>
			56: R_386_PC32	fprintf
  5a:	83 c4 10             	add    $0x10,%esp
/home/marshall_midden/src/testing/calling/m.c:11
  fprintf(stderr, "  locals=%d, %d, %d, %d\n", w,x,y,z);
  5d:	83 ec 08             	sub    $0x8,%esp
  60:	ff 75 f0             	pushl  0xfffffff0(%ebp)
  63:	ff 75 f4             	pushl  0xfffffff4(%ebp)
  66:	ff 75 f8             	pushl  0xfffffff8(%ebp)
  69:	ff 75 fc             	pushl  0xfffffffc(%ebp)
  6c:	68 16 00 00 00       	push   $0x16
			6d: R_386_32	.rodata
  71:	ff 35 00 00 00 00    	pushl  0x0
			73: R_386_32	stderr
  77:	e8 fc ff ff ff       	call   78 <main+0x78>
			78: R_386_PC32	fprintf
  7c:	83 c4 20             	add    $0x20,%esp
/home/marshall_midden/src/testing/calling/m.c:12
  a(0x100+x);
  7f:	83 ec 0c             	sub    $0xc,%esp
  82:	8b 45 f8             	mov    0xfffffff8(%ebp),%eax
  85:	05 00 01 00 00       	add    $0x100,%eax
  8a:	50                   	push   %eax
  8b:	e8 fc ff ff ff       	call   8c <main+0x8c>
			8c: R_386_PC32	a
  90:	83 c4 10             	add    $0x10,%esp
/home/marshall_midden/src/testing/calling/m.c:13
  fprintf(stderr, "m locals=%d, %d, %d, %d\n", w,x,y,z);
  93:	83 ec 08             	sub    $0x8,%esp
  96:	ff 75 f0             	pushl  0xfffffff0(%ebp)
  99:	ff 75 f4             	pushl  0xfffffff4(%ebp)
  9c:	ff 75 f8             	pushl  0xfffffff8(%ebp)
  9f:	ff 75 fc             	pushl  0xfffffffc(%ebp)
  a2:	68 2f 00 00 00       	push   $0x2f
			a3: R_386_32	.rodata
  a7:	ff 35 00 00 00 00    	pushl  0x0
			a9: R_386_32	stderr
  ad:	e8 fc ff ff ff       	call   ae <main+0xae>
			ae: R_386_PC32	fprintf
  b2:	83 c4 20             	add    $0x20,%esp
/home/marshall_midden/src/testing/calling/m.c:14
  exit(0);
  b5:	83 ec 0c             	sub    $0xc,%esp
  b8:	6a 00                	push   $0x0
  ba:	e8 fc ff ff ff       	call   bb <main+0xbb>
			bb: R_386_PC32	exit
Disassembly of section .debug_abbrev:

00000000 <.debug_abbrev>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	01 11                	add    %edx,(%ecx)
   2:	01 10                	add    %edx,(%eax)
   4:	06                   	push   %es
   5:	12 01                	adc    (%ecx),%al
   7:	11 01                	adc    %eax,(%ecx)
   9:	03 08                	add    (%eax),%ecx
   b:	1b 08                	sbb    (%eax),%ecx
   d:	25 08 13 0b 00       	and    $0xb1308,%eax
  12:	00 02                	add    %al,(%edx)
  14:	24 00                	and    $0x0,%al
  16:	03 0e                	add    (%esi),%ecx
  18:	0b 0b                	or     (%ebx),%ecx
  1a:	3e 0b 00             	or     %ds:(%eax),%eax
/home/marshall_midden/src/testing/calling/m.c:6
  1d:	00 03                	add    %al,(%ebx)
  1f:	24 00                	and    $0x0,%al
  21:	03 08                	add    (%eax),%ecx
/home/marshall_midden/src/testing/calling/m.c:7
  23:	0b 0b                	or     (%ebx),%ecx
  25:	3e 0b 00             	or     %ds:(%eax),%eax
  28:	00 04 16             	add    %al,(%esi,%edx,1)
/home/marshall_midden/src/testing/calling/m.c:8
  2b:	00 03                	add    %al,(%ebx)
  2d:	08 3a                	or     %bh,(%edx)
  2f:	0b 3b                	or     (%ebx),%edi
/home/marshall_midden/src/testing/calling/m.c:9
  31:	0b 49 13             	or     0x13(%ecx),%ecx
  34:	00 00                	add    %al,(%eax)
  36:	05 0f 00 0b 0b       	add    $0xb0b000f,%eax
/home/marshall_midden/src/testing/calling/m.c:10
  3b:	49                   	dec    %ecx
  3c:	13 00                	adc    (%eax),%eax
  3e:	00 06                	add    %al,(%esi)
  40:	13 01                	adc    (%ecx),%eax
  42:	01 13                	add    %edx,(%ebx)
  44:	03 0e                	add    (%esi),%ecx
  46:	0b 0b                	or     (%ebx),%ecx
  48:	3a 0b                	cmp    (%ebx),%cl
  4a:	3b 0b                	cmp    (%ebx),%ecx
  4c:	00 00                	add    %al,(%eax)
  4e:	07                   	pop    %es
  4f:	0d 00 03 08 3a       	or     $0x3a080300,%eax
  54:	0b 3b                	or     (%ebx),%edi
  56:	05 49 13 38 0a       	add    $0xa381349,%eax
  5b:	00 00                	add    %al,(%eax)
/home/marshall_midden/src/testing/calling/m.c:11
  5d:	08 0f                	or     %cl,(%edi)
  5f:	00 0b                	add    %cl,(%ebx)
  61:	0b 00                	or     (%eax),%eax
  63:	00 09                	add    %cl,(%ecx)
  65:	16                   	push   %ss
  66:	00 03                	add    %al,(%ebx)
  68:	08 3a                	or     %bh,(%edx)
  6a:	0b 3b                	or     (%ebx),%edi
  6c:	0b 00                	or     (%eax),%eax
  6e:	00 0a                	add    %cl,(%edx)
  70:	13 01                	adc    (%ecx),%eax
  72:	01 13                	add    %edx,(%ebx)
  74:	03 08                	add    (%eax),%ecx
  76:	0b 0b                	or     (%ebx),%ecx
  78:	3a 0b                	cmp    (%ebx),%cl
  7a:	3b 0b                	cmp    (%ebx),%ecx
  7c:	00 00                	add    %al,(%eax)
  7e:	0b 0d 00 03 08 3a    	or     0x3a080300,%ecx
/home/marshall_midden/src/testing/calling/m.c:12
  84:	0b 3b                	or     (%ebx),%edi
  86:	0b 49 13             	or     0x13(%ecx),%ecx
  89:	38 0a                	cmp    %cl,(%edx)
  8b:	00 00                	add    %al,(%eax)
  8d:	0c 01                	or     $0x1,%al
  8f:	01 01                	add    %eax,(%ecx)
  91:	13 49 13             	adc    0x13(%ecx),%ecx
/home/marshall_midden/src/testing/calling/m.c:13
  94:	00 00                	add    %al,(%eax)
  96:	0d 21 00 49 13       	or     $0x13490021,%eax
  9b:	2f                   	das    
  9c:	0b 00                	or     (%eax),%eax
  9e:	00 0e                	add    %cl,(%esi)
  a0:	2e 01 01             	add    %eax,%cs:(%ecx)
  a3:	13 3f                	adc    (%edi),%edi
  a5:	0c 03                	or     $0x3,%al
  a7:	08 3a                	or     %bh,(%edx)
  a9:	0b 3b                	or     (%ebx),%edi
  ab:	0b 49 13             	or     0x13(%ecx),%ecx
  ae:	11 01                	adc    %eax,(%ecx)
  b0:	12 01                	adc    (%ecx),%al
  b2:	40                   	inc    %eax
  b3:	0a 00                	or     (%eax),%al
/home/marshall_midden/src/testing/calling/m.c:14
  b5:	00 0f                	add    %cl,(%edi)
  b7:	34 00                	xor    $0x0,%al
  b9:	03 08                	add    (%eax),%ecx
  bb:	3a 0b                	cmp    (%ebx),%cl
  bd:	3b 0b                	cmp    (%ebx),%ecx
  bf:	49                   	dec    %ecx
  c0:	13 02                	adc    (%edx),%eax
  c2:	0a 00                	or     (%eax),%al
  c4:	00 10                	add    %dl,(%eax)
  c6:	2e 01 01             	add    %eax,%cs:(%ecx)
  c9:	13 3f                	adc    (%edi),%edi
  cb:	0c 03                	or     $0x3,%al
  cd:	08 3a                	or     %bh,(%edx)
  cf:	0b 3b                	or     (%ebx),%edi
  d1:	0b 49 13             	or     0x13(%ecx),%ecx
  d4:	3c 0c                	cmp    $0xc,%al
  d6:	00 00                	add    %al,(%eax)
  d8:	11 18                	adc    %ebx,(%eax)
  da:	00 00                	add    %al,(%eax)
  dc:	00 12                	add    %dl,(%edx)
  de:	0b 01                	or     (%ecx),%eax
  e0:	01 13                	add    %edx,(%ebx)
  e2:	11 01                	adc    %eax,(%ecx)
  e4:	12 01                	adc    (%ecx),%al
  e6:	00 00                	add    %al,(%eax)
  e8:	13 0b                	adc    (%ebx),%ecx
  ea:	01 11                	add    %edx,(%ecx)
  ec:	01 12                	add    %edx,(%edx)
  ee:	01 00                	add    %eax,(%eax)
  f0:	00 14 34             	add    %dl,(%esp,%esi,1)
  f3:	00 03                	add    %al,(%ebx)
  f5:	08 3a                	or     %bh,(%edx)
  f7:	0b 3b                	or     (%ebx),%edi
  f9:	0b 49 13             	or     0x13(%ecx),%ecx
  fc:	3f                   	aas    
  fd:	0c 3c                	or     $0x3c,%al
  ff:	0c 00                	or     $0x0,%al
	...
Disassembly of section .debug_info:

00000000 <.debug_info>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	9a 04 00 00 02 00 00 	lcall  $0x0,$0x2000004
			6: R_386_32	.debug_abbrev
   7:	00 00                	add    %al,(%eax)
   9:	00 04 01             	add    %al,(%ecx,%eax,1)
   c:	00 00                	add    %al,(%eax)
			c: R_386_32	.debug_line
   e:	00 00                	add    %al,(%eax)
  10:	bf 00 00 00 00       	mov    $0x0,%edi
			10: R_386_32	.text
			14: R_386_32	.text
  15:	00 00                	add    %al,(%eax)
  17:	00 6d 2e             	add    %ch,0x2e(%ebp)
  1a:	63 00                	arpl   %ax,(%eax)
/home/marshall_midden/src/testing/calling/m.c:6
  1c:	2f                   	das    
  1d:	68 6f 6d 65 2f       	push   $0x2f656d6f
  22:	6d                   	insl   (%dx),%es:(%edi)
/home/marshall_midden/src/testing/calling/m.c:7
  23:	61                   	popa   
  24:	72 73                	jb     99 <.debug_info+0x99>
  26:	68 61 6c 6c 5f       	push   $0x5f6c6c61
/home/marshall_midden/src/testing/calling/m.c:8
  2b:	6d                   	insl   (%dx),%es:(%edi)
  2c:	69 64 64 65 6e 2f 73 	imul   $0x72732f6e,0x65(%esp,2),%esp
  33:	72 
/home/marshall_midden/src/testing/calling/m.c:9
  34:	63 2f                	arpl   %bp,(%edi)
  36:	74 65                	je     9d <.debug_info+0x9d>
/home/marshall_midden/src/testing/calling/m.c:10
  38:	73 74                	jae    ae <.debug_info+0xae>
  3a:	69 6e 67 2f 63 61 6c 	imul   $0x6c61632f,0x67(%esi),%ebp
  41:	6c                   	insb   (%dx),%es:(%edi)
  42:	69 6e 67 00 47 4e 55 	imul   $0x554e4700,0x67(%esi),%ebp
  49:	20 43 20             	and    %al,0x20(%ebx)
  4c:	33 2e                	xor    (%esi),%ebp
  4e:	33 2e                	xor    (%esi),%ebp
  50:	33 20                	xor    (%eax),%esp
  52:	28 53 75             	sub    %dl,0x75(%ebx)
  55:	53                   	push   %ebx
  56:	45                   	inc    %ebp
  57:	20 4c 69 6e          	and    %cl,0x6e(%ecx,%ebp,2)
  5b:	75 78                	jne    d5 <.debug_info+0xd5>
/home/marshall_midden/src/testing/calling/m.c:11
  5d:	29 00                	sub    %eax,(%eax)
  5f:	01 02                	add    %eax,(%edx)
  61:	09 00                	or     %eax,(%eax)
			61: R_386_32	.debug_str
  63:	00 00                	add    %al,(%eax)
  65:	04 07                	add    $0x7,%al
  67:	03 75 6e             	add    0x6e(%ebp),%esi
  6a:	73 69                	jae    d5 <.debug_info+0xd5>
  6c:	67 6e                	addr16 outsb %ds:(%si),(%dx)
  6e:	65 64 20 63 68       	and    %ah,%fs:%gs:0x68(%ebx)
  73:	61                   	popa   
  74:	72 00                	jb     76 <.debug_info+0x76>
  76:	01 08                	add    %ecx,(%eax)
  78:	03 73 68             	add    0x68(%ebx),%esi
  7b:	6f                   	outsl  %ds:(%esi),(%dx)
  7c:	72 74                	jb     f2 <.debug_info+0xf2>
  7e:	20 75 6e             	and    %dh,0x6e(%ebp)
/home/marshall_midden/src/testing/calling/m.c:12
  81:	73 69                	jae    ec <.debug_info+0xec>
  83:	67 6e                	addr16 outsb %ds:(%si),(%dx)
  85:	65 64 20 69 6e       	and    %ch,%fs:%gs:0x6e(%ecx)
  8a:	74 00                	je     8c <.debug_info+0x8c>
  8c:	02 07                	add    (%edi),%al
  8e:	03 6c 6f 6e          	add    0x6e(%edi,%ebp,2),%ebp
  92:	67 20 75 6e          	addr16 and %dh,110(%di)
/home/marshall_midden/src/testing/calling/m.c:13
  96:	73 69                	jae    101 <.debug_info+0x101>
  98:	67 6e                	addr16 outsb %ds:(%si),(%dx)
  9a:	65 64 20 69 6e       	and    %ch,%fs:%gs:0x6e(%ecx)
  9f:	74 00                	je     a1 <.debug_info+0xa1>
  a1:	04 07                	add    $0x7,%al
  a3:	03 73 69             	add    0x69(%ebx),%esi
  a6:	67 6e                	addr16 outsb %ds:(%si),(%dx)
  a8:	65 64 20 63 68       	and    %ah,%fs:%gs:0x68(%ebx)
  ad:	61                   	popa   
  ae:	72 00                	jb     b0 <.debug_info+0xb0>
  b0:	01 06                	add    %eax,(%esi)
  b2:	03 73 68             	add    0x68(%ebx),%esi
/home/marshall_midden/src/testing/calling/m.c:14
  b5:	6f                   	outsl  %ds:(%esi),(%dx)
  b6:	72 74                	jb     12c <.debug_info+0x12c>
  b8:	20 69 6e             	and    %ch,0x6e(%ecx)
  bb:	74 00                	je     bd <.debug_info+0xbd>
  bd:	02 05 03 69 6e 74    	add    0x746e6903,%al
  c3:	00 04 05 03 6c 6f 6e 	add    %al,0x6e6f6c03(,%eax,1)
  ca:	67 20 6c 6f          	addr16 and %ch,111(%si)
  ce:	6e                   	outsb  %ds:(%esi),(%dx)
  cf:	67 20 69 6e          	addr16 and %ch,110(%bx,%di)
  d3:	74 00                	je     d5 <.debug_info+0xd5>
  d5:	08 05 03 6c 6f 6e    	or     %al,0x6e6f6c03
  db:	67 20 6c 6f          	addr16 and %ch,111(%si)
  df:	6e                   	outsb  %ds:(%esi),(%dx)
  e0:	67 20 75 6e          	addr16 and %dh,110(%di)
  e4:	73 69                	jae    14f <.debug_info+0x14f>
  e6:	67 6e                	addr16 outsb %ds:(%si),(%dx)
  e8:	65 64 20 69 6e       	and    %ch,%fs:%gs:0x6e(%ecx)
  ed:	74 00                	je     ef <.debug_info+0xef>
  ef:	08 07                	or     %al,(%edi)
  f1:	04 5f                	add    $0x5f,%al
  f3:	5f                   	pop    %edi
  f4:	6f                   	outsl  %ds:(%esi),(%dx)
  f5:	66 66 5f             	pop    %di
  f8:	74 00                	je     fa <.debug_info+0xfa>
  fa:	04 8f                	add    $0x8f,%al
  fc:	00 01                	add    %al,(%ecx)
  fe:	00 00                	add    %al,(%eax)
 100:	03 6c 6f 6e          	add    0x6e(%edi,%ebp,2),%ebp
 104:	67 20 69 6e          	addr16 and %ch,110(%bx,%di)
 108:	74 00                	je     10a <.debug_info+0x10a>
 10a:	04 05                	add    $0x5,%al
 10c:	04 5f                	add    $0x5f,%al
 10e:	5f                   	pop    %edi
 10f:	6f                   	outsl  %ds:(%esi),(%dx)
 110:	66                   	data16
 111:	66                   	data16
 112:	36                   	ss
 113:	34 5f                	xor    $0x5f,%al
 115:	74 00                	je     117 <.debug_info+0x117>
 117:	04 90                	add    $0x90,%al
 119:	c6 00 00             	movb   $0x0,(%eax)
 11c:	00 02                	add    %al,(%edx)
 11e:	09 00                	or     %eax,(%eax)
			11e: R_386_32	.debug_str
 120:	00 00                	add    %al,(%eax)
 122:	04 07                	add    $0x7,%al
 124:	05 04 2a 01 00       	add    $0x12a04,%eax
 129:	00 03                	add    %al,(%ebx)
 12b:	63 68 61             	arpl   %bp,0x61(%eax)
 12e:	72 00                	jb     130 <.debug_info+0x130>
 130:	01 06                	add    %eax,(%esi)
 132:	06                   	push   %es
 133:	77 03                	ja     138 <.debug_info+0x138>
 135:	00 00                	add    %al,(%eax)
 137:	00 00                	add    %al,(%eax)
			137: R_386_32	.debug_str
 139:	00 00                	add    %al,(%eax)
 13b:	94                   	xchg   %eax,%esp
 13c:	02 2e                	add    (%esi),%ch
 13e:	07                   	pop    %es
 13f:	5f                   	pop    %edi
 140:	66                   	data16
 141:	6c                   	insb   (%dx),%es:(%edi)
 142:	61                   	popa   
 143:	67 73 00             	addr16 jae 146 <.debug_info+0x146>
 146:	03 09                	add    (%ecx),%ecx
 148:	01 bf 00 00 00 02    	add    %edi,0x2000000(%edi)
 14e:	23 00                	and    (%eax),%eax
 150:	07                   	pop    %es
 151:	5f                   	pop    %edi
 152:	49                   	dec    %ecx
 153:	4f                   	dec    %edi
 154:	5f                   	pop    %edi
 155:	72 65                	jb     1bc <.debug_info+0x1bc>
 157:	61                   	popa   
 158:	64                   	fs
 159:	5f                   	pop    %edi
 15a:	70 74                	jo     1d0 <.debug_info+0x1d0>
 15c:	72 00                	jb     15e <.debug_info+0x15e>
 15e:	03 0e                	add    (%esi),%ecx
 160:	01 24 01             	add    %esp,(%ecx,%eax,1)
 163:	00 00                	add    %al,(%eax)
 165:	02 23                	add    (%ebx),%ah
 167:	04 07                	add    $0x7,%al
 169:	5f                   	pop    %edi
 16a:	49                   	dec    %ecx
 16b:	4f                   	dec    %edi
 16c:	5f                   	pop    %edi
 16d:	72 65                	jb     1d4 <.debug_info+0x1d4>
 16f:	61                   	popa   
 170:	64                   	fs
 171:	5f                   	pop    %edi
 172:	65 6e                	outsb  %gs:(%esi),(%dx)
 174:	64 00 03             	add    %al,%fs:(%ebx)
 177:	0f 01 24 01          	smswl  (%ecx,%eax,1)
 17b:	00 00                	add    %al,(%eax)
 17d:	02 23                	add    (%ebx),%ah
 17f:	08 07                	or     %al,(%edi)
 181:	5f                   	pop    %edi
 182:	49                   	dec    %ecx
 183:	4f                   	dec    %edi
 184:	5f                   	pop    %edi
 185:	72 65                	jb     1ec <.debug_info+0x1ec>
 187:	61                   	popa   
 188:	64                   	fs
 189:	5f                   	pop    %edi
 18a:	62 61 73             	bound  %esp,0x73(%ecx)
 18d:	65 00 03             	add    %al,%gs:(%ebx)
 190:	10 01                	adc    %al,(%ecx)
 192:	24 01                	and    $0x1,%al
 194:	00 00                	add    %al,(%eax)
 196:	02 23                	add    (%ebx),%ah
 198:	0c 07                	or     $0x7,%al
 19a:	5f                   	pop    %edi
 19b:	49                   	dec    %ecx
 19c:	4f                   	dec    %edi
 19d:	5f                   	pop    %edi
 19e:	77 72                	ja     212 <.debug_info+0x212>
 1a0:	69 74 65 5f 62 61 73 	imul   $0x65736162,0x5f(%ebp,2),%esi
 1a7:	65 
 1a8:	00 03                	add    %al,(%ebx)
 1aa:	11 01                	adc    %eax,(%ecx)
 1ac:	24 01                	and    $0x1,%al
 1ae:	00 00                	add    %al,(%eax)
 1b0:	02 23                	add    (%ebx),%ah
 1b2:	10 07                	adc    %al,(%edi)
 1b4:	5f                   	pop    %edi
 1b5:	49                   	dec    %ecx
 1b6:	4f                   	dec    %edi
 1b7:	5f                   	pop    %edi
 1b8:	77 72                	ja     22c <.debug_info+0x22c>
 1ba:	69 74 65 5f 70 74 72 	imul   $0x727470,0x5f(%ebp,2),%esi
 1c1:	00 
 1c2:	03 12                	add    (%edx),%edx
 1c4:	01 24 01             	add    %esp,(%ecx,%eax,1)
 1c7:	00 00                	add    %al,(%eax)
 1c9:	02 23                	add    (%ebx),%ah
 1cb:	14 07                	adc    $0x7,%al
 1cd:	5f                   	pop    %edi
 1ce:	49                   	dec    %ecx
 1cf:	4f                   	dec    %edi
 1d0:	5f                   	pop    %edi
 1d1:	77 72                	ja     245 <.debug_info+0x245>
 1d3:	69 74 65 5f 65 6e 64 	imul   $0x646e65,0x5f(%ebp,2),%esi
 1da:	00 
 1db:	03 13                	add    (%ebx),%edx
 1dd:	01 24 01             	add    %esp,(%ecx,%eax,1)
 1e0:	00 00                	add    %al,(%eax)
 1e2:	02 23                	add    (%ebx),%ah
 1e4:	18 07                	sbb    %al,(%edi)
 1e6:	5f                   	pop    %edi
 1e7:	49                   	dec    %ecx
 1e8:	4f                   	dec    %edi
 1e9:	5f                   	pop    %edi
 1ea:	62 75 66             	bound  %esi,0x66(%ebp)
 1ed:	5f                   	pop    %edi
 1ee:	62 61 73             	bound  %esp,0x73(%ecx)
 1f1:	65 00 03             	add    %al,%gs:(%ebx)
 1f4:	14 01                	adc    $0x1,%al
 1f6:	24 01                	and    $0x1,%al
 1f8:	00 00                	add    %al,(%eax)
 1fa:	02 23                	add    (%ebx),%ah
 1fc:	1c 07                	sbb    $0x7,%al
 1fe:	5f                   	pop    %edi
 1ff:	49                   	dec    %ecx
 200:	4f                   	dec    %edi
 201:	5f                   	pop    %edi
 202:	62 75 66             	bound  %esi,0x66(%ebp)
 205:	5f                   	pop    %edi
 206:	65 6e                	outsb  %gs:(%esi),(%dx)
 208:	64 00 03             	add    %al,%fs:(%ebx)
 20b:	15 01 24 01 00       	adc    $0x12401,%eax
 210:	00 02                	add    %al,(%edx)
 212:	23 20                	and    (%eax),%esp
 214:	07                   	pop    %es
 215:	5f                   	pop    %edi
 216:	49                   	dec    %ecx
 217:	4f                   	dec    %edi
 218:	5f                   	pop    %edi
 219:	73 61                	jae    27c <.debug_info+0x27c>
 21b:	76 65                	jbe    282 <.debug_info+0x282>
 21d:	5f                   	pop    %edi
 21e:	62 61 73             	bound  %esp,0x73(%ecx)
 221:	65 00 03             	add    %al,%gs:(%ebx)
 224:	17                   	pop    %ss
 225:	01 24 01             	add    %esp,(%ecx,%eax,1)
 228:	00 00                	add    %al,(%eax)
 22a:	02 23                	add    (%ebx),%ah
 22c:	24 07                	and    $0x7,%al
 22e:	5f                   	pop    %edi
 22f:	49                   	dec    %ecx
 230:	4f                   	dec    %edi
 231:	5f                   	pop    %edi
 232:	62 61 63             	bound  %esp,0x63(%ecx)
 235:	6b 75 70 5f          	imul   $0x5f,0x70(%ebp),%esi
 239:	62 61 73             	bound  %esp,0x73(%ecx)
 23c:	65 00 03             	add    %al,%gs:(%ebx)
 23f:	18 01                	sbb    %al,(%ecx)
 241:	24 01                	and    $0x1,%al
 243:	00 00                	add    %al,(%eax)
 245:	02 23                	add    (%ebx),%ah
 247:	28 07                	sub    %al,(%edi)
 249:	5f                   	pop    %edi
 24a:	49                   	dec    %ecx
 24b:	4f                   	dec    %edi
 24c:	5f                   	pop    %edi
 24d:	73 61                	jae    2b0 <.debug_info+0x2b0>
 24f:	76 65                	jbe    2b6 <.debug_info+0x2b6>
 251:	5f                   	pop    %edi
 252:	65 6e                	outsb  %gs:(%esi),(%dx)
 254:	64 00 03             	add    %al,%fs:(%ebx)
 257:	19 01                	sbb    %eax,(%ecx)
 259:	24 01                	and    $0x1,%al
 25b:	00 00                	add    %al,(%eax)
 25d:	02 23                	add    (%ebx),%ah
 25f:	2c 07                	sub    $0x7,%al
 261:	5f                   	pop    %edi
 262:	6d                   	insl   (%dx),%es:(%edi)
 263:	61                   	popa   
 264:	72 6b                	jb     2d1 <.debug_info+0x2d1>
 266:	65                   	gs
 267:	72 73                	jb     2dc <.debug_info+0x2dc>
 269:	00 03                	add    %al,(%ebx)
 26b:	1b 01                	sbb    (%ecx),%eax
 26d:	ca 03 00             	lret   $0x3
 270:	00 02                	add    %al,(%edx)
 272:	23 30                	and    (%eax),%esi
 274:	07                   	pop    %es
 275:	5f                   	pop    %edi
 276:	63 68 61             	arpl   %bp,0x61(%eax)
 279:	69 6e 00 03 1d 01 d0 	imul   $0xd0011d03,0x0(%esi),%ebp
 280:	03 00                	add    (%eax),%eax
 282:	00 02                	add    %al,(%edx)
 284:	23 34 07             	and    (%edi,%eax,1),%esi
 287:	5f                   	pop    %edi
 288:	66 69 6c 65 6e 6f 00 	imul   $0x6f,0x6e(%ebp,2),%bp
 28f:	03 1f                	add    (%edi),%ebx
 291:	01 bf 00 00 00 02    	add    %edi,0x2000000(%edi)
 297:	23 38                	and    (%eax),%edi
 299:	07                   	pop    %es
 29a:	5f                   	pop    %edi
 29b:	66                   	data16
 29c:	6c                   	insb   (%dx),%es:(%edi)
 29d:	61                   	popa   
 29e:	67 73 32             	addr16 jae 2d3 <.debug_info+0x2d3>
 2a1:	00 03                	add    %al,(%ebx)
 2a3:	23 01                	and    (%ecx),%eax
 2a5:	bf 00 00 00 02       	mov    $0x2000000,%edi
 2aa:	23 3c 07             	and    (%edi,%eax,1),%edi
 2ad:	5f                   	pop    %edi
 2ae:	6f                   	outsl  %ds:(%esi),(%dx)
 2af:	6c                   	insb   (%dx),%es:(%edi)
 2b0:	64                   	fs
 2b1:	5f                   	pop    %edi
 2b2:	6f                   	outsl  %ds:(%esi),(%dx)
 2b3:	66                   	data16
 2b4:	66                   	data16
 2b5:	73 65                	jae    31c <.debug_info+0x31c>
 2b7:	74 00                	je     2b9 <.debug_info+0x2b9>
 2b9:	03 25 01 f1 00 00    	add    0xf101,%esp
 2bf:	00 02                	add    %al,(%edx)
 2c1:	23 40 07             	and    0x7(%eax),%eax
 2c4:	5f                   	pop    %edi
 2c5:	63 75 72             	arpl   %si,0x72(%ebp)
 2c8:	5f                   	pop    %edi
 2c9:	63 6f 6c             	arpl   %bp,0x6c(%edi)
 2cc:	75 6d                	jne    33b <.debug_info+0x33b>
 2ce:	6e                   	outsb  %ds:(%esi),(%dx)
 2cf:	00 03                	add    %al,(%ebx)
 2d1:	29 01                	sub    %eax,(%ecx)
 2d3:	78 00                	js     2d5 <.debug_info+0x2d5>
 2d5:	00 00                	add    %al,(%eax)
 2d7:	02 23                	add    (%ebx),%ah
 2d9:	44                   	inc    %esp
 2da:	07                   	pop    %es
 2db:	5f                   	pop    %edi
 2dc:	76 74                	jbe    352 <.debug_info+0x352>
 2de:	61                   	popa   
 2df:	62 6c 65 5f          	bound  %ebp,0x5f(%ebp,2)
 2e3:	6f                   	outsl  %ds:(%esi),(%dx)
 2e4:	66                   	data16
 2e5:	66                   	data16
 2e6:	73 65                	jae    34d <.debug_info+0x34d>
 2e8:	74 00                	je     2ea <.debug_info+0x2ea>
 2ea:	03 2a                	add    (%edx),%ebp
 2ec:	01 a3 00 00 00 02    	add    %esp,0x2000000(%ebx)
 2f2:	23 46 07             	and    0x7(%esi),%eax
 2f5:	5f                   	pop    %edi
 2f6:	73 68                	jae    360 <.debug_info+0x360>
 2f8:	6f                   	outsl  %ds:(%esi),(%dx)
 2f9:	72 74                	jb     36f <.debug_info+0x36f>
 2fb:	62 75 66             	bound  %esi,0x66(%ebp)
 2fe:	00 03                	add    %al,(%ebx)
 300:	2b 01                	sub    (%ecx),%eax
 302:	d6                   	(bad)  
 303:	03 00                	add    (%eax),%eax
 305:	00 02                	add    %al,(%edx)
 307:	23 47 07             	and    0x7(%edi),%eax
 30a:	5f                   	pop    %edi
 30b:	6c                   	insb   (%dx),%es:(%edi)
 30c:	6f                   	outsl  %ds:(%esi),(%dx)
 30d:	63 6b 00             	arpl   %bp,0x0(%ebx)
 310:	03 2f                	add    (%edi),%ebp
 312:	01 e6                	add    %esp,%esi
 314:	03 00                	add    (%eax),%eax
 316:	00 02                	add    %al,(%edx)
 318:	23 48 07             	and    0x7(%eax),%ecx
 31b:	5f                   	pop    %edi
 31c:	6f                   	outsl  %ds:(%esi),(%dx)
 31d:	66                   	data16
 31e:	66                   	data16
 31f:	73 65                	jae    386 <.debug_info+0x386>
 321:	74 00                	je     323 <.debug_info+0x323>
 323:	03 38                	add    (%eax),%edi
 325:	01 0c 01             	add    %ecx,(%ecx,%eax,1)
 328:	00 00                	add    %al,(%eax)
 32a:	02 23                	add    (%ebx),%ah
 32c:	4c                   	dec    %esp
 32d:	07                   	pop    %es
 32e:	5f                   	pop    %edi
 32f:	5f                   	pop    %edi
 330:	70 61                	jo     393 <.debug_info+0x393>
 332:	64 31 00             	xor    %eax,%fs:(%eax)
 335:	03 3e                	add    (%esi),%edi
 337:	01 77 03             	add    %esi,0x3(%edi)
 33a:	00 00                	add    %al,(%eax)
 33c:	02 23                	add    (%ebx),%ah
 33e:	54                   	push   %esp
 33f:	07                   	pop    %es
 340:	5f                   	pop    %edi
 341:	5f                   	pop    %edi
 342:	70 61                	jo     3a5 <.debug_info+0x3a5>
 344:	64 32 00             	xor    %fs:(%eax),%al
 347:	03 3f                	add    (%edi),%edi
 349:	01 77 03             	add    %esi,0x3(%edi)
 34c:	00 00                	add    %al,(%eax)
 34e:	02 23                	add    (%ebx),%ah
 350:	58                   	pop    %eax
 351:	07                   	pop    %es
 352:	5f                   	pop    %edi
 353:	6d                   	insl   (%dx),%es:(%edi)
 354:	6f                   	outsl  %ds:(%esi),(%dx)
 355:	64 65 00 03          	add    %al,%fs:%gs:(%ebx)
 359:	41                   	inc    %ecx
 35a:	01 bf 00 00 00 02    	add    %edi,0x2000000(%edi)
 360:	23 5c 07 5f          	and    0x5f(%edi,%eax,1),%ebx
 364:	75 6e                	jne    3d4 <.debug_info+0x3d4>
 366:	75 73                	jne    3db <.debug_info+0x3db>
 368:	65 64 32 00          	xor    %fs:%gs:(%eax),%al
 36c:	03 43 01             	add    0x1(%ebx),%eax
 36f:	ec                   	in     (%dx),%al
 370:	03 00                	add    (%eax),%eax
 372:	00 02                	add    %al,(%edx)
 374:	23 60 00             	and    0x0(%eax),%esp
 377:	08 04 09             	or     %al,(%ecx,%ecx,1)
 37a:	5f                   	pop    %edi
 37b:	49                   	dec    %ecx
 37c:	4f                   	dec    %edi
 37d:	5f                   	pop    %edi
 37e:	6c                   	insb   (%dx),%es:(%edi)
 37f:	6f                   	outsl  %ds:(%esi),(%dx)
 380:	63 6b 5f             	arpl   %bp,0x5f(%ebx)
 383:	74 00                	je     385 <.debug_info+0x385>
 385:	03 ad 0a ca 03 00    	add    0x3ca0a(%ebp),%ebp
 38b:	00 5f 49             	add    %bl,0x49(%edi)
 38e:	4f                   	dec    %edi
 38f:	5f                   	pop    %edi
 390:	6d                   	insl   (%dx),%es:(%edi)
 391:	61                   	popa   
 392:	72 6b                	jb     3ff <.debug_info+0x3ff>
 394:	65                   	gs
 395:	72 00                	jb     397 <.debug_info+0x397>
 397:	0c 03                	or     $0x3,%al
 399:	b3 0b                	mov    $0xb,%bl
 39b:	5f                   	pop    %edi
 39c:	6e                   	outsb  %ds:(%esi),(%dx)
 39d:	65                   	gs
 39e:	78 74                	js     414 <.debug_info+0x414>
 3a0:	00 03                	add    %al,(%ebx)
 3a2:	b4 ca                	mov    $0xca,%ah
 3a4:	03 00                	add    (%eax),%eax
 3a6:	00 02                	add    %al,(%edx)
 3a8:	23 00                	and    (%eax),%eax
 3aa:	0b 5f 73             	or     0x73(%edi),%ebx
 3ad:	62 75 66             	bound  %esi,0x66(%ebp)
 3b0:	00 03                	add    %al,(%ebx)
 3b2:	b5 d0                	mov    $0xd0,%ch
 3b4:	03 00                	add    (%eax),%eax
 3b6:	00 02                	add    %al,(%edx)
 3b8:	23 04 0b             	and    (%ebx,%ecx,1),%eax
 3bb:	5f                   	pop    %edi
 3bc:	70 6f                	jo     42d <.debug_info+0x42d>
 3be:	73 00                	jae    3c0 <.debug_info+0x3c0>
 3c0:	03 b9 bf 00 00 00    	add    0xbf(%ecx),%edi
 3c6:	02 23                	add    (%ebx),%ah
 3c8:	08 00                	or     %al,(%eax)
 3ca:	05 04 87 03 00       	add    $0x38704,%eax
 3cf:	00 05 04 32 01 00    	add    %al,0x13204
 3d5:	00 0c e6             	add    %cl,(%esi,8)
 3d8:	03 00                	add    (%eax),%eax
 3da:	00 2a                	add    %ch,(%edx)
 3dc:	01 00                	add    %eax,(%eax)
 3de:	00 0d 1d 01 00 00    	add    %cl,0x11d
 3e4:	00 00                	add    %al,(%eax)
 3e6:	05 04 79 03 00       	add    $0x37904,%eax
 3eb:	00 0c fc             	add    %cl,(%esp,%edi,8)
 3ee:	03 00                	add    (%eax),%eax
 3f0:	00 2a                	add    %ch,(%edx)
 3f2:	01 00                	add    %eax,(%eax)
 3f4:	00 0d 1d 01 00 00    	add    %cl,0x11d
 3fa:	33 00                	xor    (%eax),%eax
 3fc:	0e                   	push   %cs
 3fd:	8d 04 00             	lea    (%eax,%eax,1),%eax
 400:	00 01                	add    %al,(%ecx)
 402:	6d                   	insl   (%dx),%es:(%edi)
 403:	61                   	popa   
 404:	69 6e 00 01 04 bf 00 	imul   $0xbf0401,0x0(%esi),%ebp
 40b:	00 00                	add    %al,(%eax)
 40d:	00 00                	add    %al,(%eax)
			40d: R_386_32	.text
 40f:	00 00                	add    %al,(%eax)
 411:	bf 00 00 00 01       	mov    $0x1000000,%edi
			411: R_386_32	.text
 416:	55                   	push   %ebp
 417:	0f 77                	emms   
 419:	00 01                	add    %al,(%ecx)
 41b:	05 bf 00 00 00       	add    $0xbf,%eax
 420:	02 91 7c 0f 78 00    	add    0x780f7c(%ecx),%dl
 426:	01 05 bf 00 00 00    	add    %eax,0xbf
 42c:	02 91 78 0f 79 00    	add    0x790f78(%ecx),%dl
 432:	01 05 bf 00 00 00    	add    %eax,0xbf
 438:	02 91 74 0f 7a 00    	add    0x7a0f74(%ecx),%dl
 43e:	01 05 bf 00 00 00    	add    %eax,0xbf
 444:	02 91 70 10 58 04    	add    0x4581070(%ecx),%dl
 44a:	00 00                	add    %al,(%eax)
 44c:	01 61 00             	add    %esp,0x0(%ecx)
 44f:	01 0c bf             	add    %ecx,(%edi,%edi,4)
 452:	00 00                	add    %al,(%eax)
 454:	00 01                	add    %al,(%ecx)
 456:	11 00                	adc    %eax,(%eax)
 458:	12 74 04 00          	adc    0x0(%esp,%eax,1),%dh
 45c:	00 38                	add    %bh,(%eax)
			45d: R_386_32	.text
 45e:	00 00                	add    %al,(%eax)
 460:	00 3d 00 00 00 0f    	add    %bh,0xf000000
			461: R_386_32	.text
 466:	5f                   	pop    %edi
 467:	73 5f                	jae    4c8 <main+0x4c8>
 469:	00 01                	add    %al,(%ecx)
 46b:	0a 8e 00 00 00 02    	or     0x2000000(%esi),%cl
 471:	91                   	xchg   %eax,%ecx
 472:	6c                   	insb   (%dx),%es:(%edi)
 473:	00 13                	add    %dl,(%ebx)
 475:	40                   	inc    %eax
			475: R_386_32	.text
 476:	00 00                	add    %al,(%eax)
 478:	00 45 00             	add    %al,0x0(%ebp)
			479: R_386_32	.text
 47b:	00 00                	add    %al,(%eax)
 47d:	0f 5f 73 5f          	maxps  0x5f(%ebx),%xmm6
 481:	00 01                	add    %al,(%ecx)
 483:	0a 8e 00 00 00 02    	or     0x2000000(%esi),%cl
 489:	91                   	xchg   %eax,%ecx
 48a:	6c                   	insb   (%dx),%es:(%edi)
 48b:	00 00                	add    %al,(%eax)
 48d:	14 73                	adc    $0x73,%al
 48f:	74 64                	je     4f5 <main+0x4f5>
 491:	65                   	gs
 492:	72 72                	jb     506 <main+0x506>
 494:	00 02                	add    %al,(%edx)
 496:	90                   	nop    
 497:	d0 03                	rolb   (%ebx)
 499:	00 00                	add    %al,(%eax)
 49b:	01 01                	add    %eax,(%ecx)
	...
Disassembly of section .debug_line:

00000000 <.debug_line>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	79 00                	jns    2 <.debug_line+0x2>
   2:	00 00                	add    %al,(%eax)
   4:	02 00                	add    (%eax),%al
   6:	57                   	push   %edi
   7:	00 00                	add    %al,(%eax)
   9:	00 01                	add    %al,(%ecx)
   b:	01 fb                	add    %edi,%ebx
   d:	0e                   	push   %cs
   e:	0a 00                	or     (%eax),%al
  10:	01 01                	add    %eax,(%ecx)
  12:	01 01                	add    %eax,(%ecx)
  14:	00 00                	add    %al,(%eax)
  16:	00 01                	add    %al,(%ecx)
  18:	2f                   	das    
  19:	75 73                	jne    8e <main+0x8e>
  1b:	72 2f                	jb     4c <.debug_line+0x4c>
/home/marshall_midden/src/testing/calling/m.c:6
  1d:	69 6e 63 6c 75 64 65 	imul   $0x6564756c,0x63(%esi),%ebp
/home/marshall_midden/src/testing/calling/m.c:7
  24:	00 2f                	add    %ch,(%edi)
  26:	75 73                	jne    9b <main+0x9b>
  28:	72 2f                	jb     59 <.debug_line+0x59>
/home/marshall_midden/src/testing/calling/m.c:8
  2a:	69 6e 63 6c 75 64 65 	imul   $0x6564756c,0x63(%esi),%ebp
/home/marshall_midden/src/testing/calling/m.c:9
  31:	2f                   	das    
  32:	62 69 74             	bound  %ebp,0x74(%ecx)
  35:	73 00                	jae    37 <.debug_line+0x37>
  37:	00 6d 2e             	add    %ch,0x2e(%ebp)
/home/marshall_midden/src/testing/calling/m.c:10
  3a:	63 00                	arpl   %ax,(%eax)
  3c:	00 00                	add    %al,(%eax)
  3e:	00 73 74             	add    %dh,0x74(%ebx)
  41:	64 69 6f 2e 68 00 01 	imul   $0x10068,%fs:0x2e(%edi),%ebp
  48:	00 
  49:	00 6c 69 62          	add    %ch,0x62(%ecx,%ebp,2)
  4d:	69 6f 2e 68 00 01 00 	imul   $0x10068,0x2e(%edi),%ebp
  54:	00 74 79 70          	add    %dh,0x70(%ecx,%edi,2)
  58:	65                   	gs
  59:	73 2e                	jae    89 <main+0x89>
  5b:	68 00 02 00 00       	push   $0x200
/home/marshall_midden/src/testing/calling/m.c:11
  60:	00 00                	add    %al,(%eax)
  62:	05 02 00 00 00       	add    $0x2,%eax
			64: R_386_32	.text
  67:	00 12                	add    %dl,(%edx)
  69:	08 ab 72 72 72 72    	or     %ch,0x72727272(%ebx)
  6f:	02 25 10 08 fe 08    	add    0x8fe0810,%ah
  75:	3a 08                	cmp    (%eax),%cl
  77:	fe 02                	incb   (%edx)
  79:	0a 00                	or     (%eax),%al
  7b:	01 01                	add    %eax,(%ecx)
Disassembly of section .rodata:

00000000 <.rodata>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	69 6e 20 6d 20 20 65 	imul   $0x6520206d,0x20(%esi),%ebp
   7:	62 70 3d             	bound  %esi,0x3d(%eax)
   a:	25 70 20 20 65       	and    $0x65202070,%eax
   f:	73 70                	jae    81 <main+0x81>
  11:	3d 25 70 0a 00       	cmp    $0xa7025,%eax
  16:	20 20                	and    %ah,(%eax)
  18:	6c                   	insb   (%dx),%es:(%edi)
  19:	6f                   	outsl  %ds:(%esi),(%dx)
  1a:	63 61 6c             	arpl   %sp,0x6c(%ecx)
/home/marshall_midden/src/testing/calling/m.c:6
  1d:	73 3d                	jae    5c <main+0x5c>
  1f:	25 64 2c 20 25       	and    $0x25202c64,%eax
/home/marshall_midden/src/testing/calling/m.c:7
  24:	64                   	fs
  25:	2c 20                	sub    $0x20,%al
  27:	25 64 2c 20 25       	and    $0x25202c64,%eax
/home/marshall_midden/src/testing/calling/m.c:8
  2c:	64 0a 00             	or     %fs:(%eax),%al
  2f:	6d                   	insl   (%dx),%es:(%edi)
  30:	20 6c 6f 63          	and    %ch,0x63(%edi,%ebp,2)
/home/marshall_midden/src/testing/calling/m.c:9
  34:	61                   	popa   
  35:	6c                   	insb   (%dx),%es:(%edi)
  36:	73 3d                	jae    75 <main+0x75>
/home/marshall_midden/src/testing/calling/m.c:10
  38:	25 64 2c 20 25       	and    $0x25202c64,%eax
  3d:	64                   	fs
  3e:	2c 20                	sub    $0x20,%al
  40:	25 64 2c 20 25       	and    $0x25202c64,%eax
  45:	64 0a 00             	or     %fs:(%eax),%al
Disassembly of section .debug_frame:

00000000 <.debug_frame>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	10 00                	adc    %al,(%eax)
   2:	00 00                	add    %al,(%eax)
   4:	ff                   	(bad)  
   5:	ff                   	(bad)  
   6:	ff                   	(bad)  
   7:	ff 01                	incl   (%ecx)
   9:	00 01                	add    %al,(%ecx)
   b:	7c 08                	jl     15 <.debug_frame+0x15>
   d:	0c 04                	or     $0x4,%al
   f:	04 88                	add    $0x88,%al
  11:	01 00                	add    %eax,(%eax)
  13:	00 24 00             	add    %ah,(%eax,%eax,1)
	...
			18: R_386_32	.debug_frame
			1c: R_386_32	.text
/home/marshall_midden/src/testing/calling/m.c:6
  1e:	00 00                	add    %al,(%eax)
  20:	bf 00 00 00 41       	mov    $0x41000000,%edi
/home/marshall_midden/src/testing/calling/m.c:7
  25:	0e                   	push   %cs
  26:	08 85 02 42 0d 05    	or     %al,0x50d4202(%ebp)
/home/marshall_midden/src/testing/calling/m.c:8
  2c:	02 52 2e             	add    0x2e(%edx),%dl
  2f:	10 62 2e             	adc    %ah,0x2e(%edx)
/home/marshall_midden/src/testing/calling/m.c:9
  32:	20 54 2e 10          	and    %dl,0x10(%esi,%ebp,1)
  36:	62 2e                	bound  %ebp,(%esi)
/home/marshall_midden/src/testing/calling/m.c:10
  38:	20 4d 2e             	and    %cl,0x2e(%ebp)
  3b:	10                   	.byte 0x10
Disassembly of section .debug_pubnames:

00000000 <.debug_pubnames>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	17                   	pop    %ss
   1:	00 00                	add    %al,(%eax)
   3:	00 02                	add    %al,(%edx)
   5:	00 00                	add    %al,(%eax)
			6: R_386_32	.debug_info
   7:	00 00                	add    %al,(%eax)
   9:	00 9e 04 00 00 fc    	add    %bl,0xfc000004(%esi)
   f:	03 00                	add    (%eax),%eax
  11:	00 6d 61             	add    %ch,0x61(%ebp)
  14:	69 6e 00 00 00 00 00 	imul   $0x0,0x0(%esi),%ebp
Disassembly of section .debug_aranges:

00000000 <.debug_aranges>:
   0:	1c 00                	sbb    $0x0,%al
   2:	00 00                	add    %al,(%eax)
   4:	02 00                	add    (%eax),%al
   6:	00 00                	add    %al,(%eax)
			6: R_386_32	.debug_info
   8:	00 00                	add    %al,(%eax)
   a:	04 00                	add    $0x0,%al
	...
			10: R_386_32	.text
  14:	bf 00 00 00 00       	mov    $0x0,%edi
  19:	00 00                	add    %al,(%eax)
  1b:	00 00                	add    %al,(%eax)
/home/marshall_midden/src/testing/calling/m.c:6
  1d:	00 00                	add    %al,(%eax)
	...
Disassembly of section .debug_str:

00000000 <.debug_str>:
/home/marshall_midden/src/testing/calling/m.c:4
   0:	5f                   	pop    %edi
   1:	49                   	dec    %ecx
   2:	4f                   	dec    %edi
   3:	5f                   	pop    %edi
   4:	46                   	inc    %esi
   5:	49                   	dec    %ecx
   6:	4c                   	dec    %esp
   7:	45                   	inc    %ebp
   8:	00 75 6e             	add    %dh,0x6e(%ebp)
   b:	73 69                	jae    76 <main+0x76>
   d:	67 6e                	addr16 outsb %ds:(%si),(%dx)
   f:	65 64 20 69 6e       	and    %ch,%fs:%gs:0x6e(%ecx)
  14:	74 00                	je     16 <main+0x16>
Disassembly of section .comment:

00000000 <.comment>:
   0:	00 47 43             	add    %al,0x43(%edi)
   3:	43                   	inc    %ebx
   4:	3a 20                	cmp    (%eax),%ah
   6:	28 47 4e             	sub    %al,0x4e(%edi)
   9:	55                   	push   %ebp
   a:	29 20                	sub    %esp,(%eax)
   c:	33 2e                	xor    (%esi),%ebp
   e:	33 2e                	xor    (%esi),%ebp
  10:	33 20                	xor    (%eax),%esp
  12:	28 53 75             	sub    %dl,0x75(%ebx)
  15:	53                   	push   %ebx
  16:	45                   	inc    %ebp
  17:	20 4c 69 6e          	and    %cl,0x6e(%ecx,%ebp,2)
  1b:	75 78                	jne    95 <main+0x95>
/home/marshall_midden/src/testing/calling/m.c:6
  1d:	29 00                	sub    %eax,(%eax)
