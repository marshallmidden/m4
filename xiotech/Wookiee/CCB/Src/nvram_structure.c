/* $Id: nvram_structure.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       nvram_structure.c
** MODULE TITLE:    Structure definition for CCB NVRAM
**
** DESCRIPTION:     This file is used to declare the structure and allocate
**                  the space which is overlayed on top of the NVRAM.  This
**                  allows the C code to directly address the data stored
**                  there without having to reference an assembly file.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "nvram_structure.h"

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
NVRAM_STRUCTURE *nvramData;

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
