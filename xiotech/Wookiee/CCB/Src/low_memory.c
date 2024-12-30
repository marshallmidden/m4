/* $Id: low_memory.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       low_memory.c
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "LL_Stats.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/* NOTE: these are all in the bss section, and thus zero upon startup. */
static LL_STATS L_stattbl_Space;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/****
** Link Layer definitions
**    This is used by the intertupt handler to find the right process
**    associated with an inbound message request for a sync state change.
****/
LL_STATS   *L_stattbl = &L_stattbl_Space;

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
