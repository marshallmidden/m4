/* $Id: rb.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       rb.h
**
**  Copyright (c) 2003-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _RB_H_
#define _RB_H_

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define BLACK    0                   /* Black Attribute                  */
#define RED      1                   /* Red Attribute                    */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
/*
**  RB Element Structure definition -----------------------------------------
*/
typedef struct RB
{
    UINT64      key;            /* Key value                  <l>           */
    struct RB  *cLeft;          /* Child pointer left         <w>           */
    struct RB  *cRight;         /* Child pointer right        <w>           */
    struct RB  *bParent;        /* Parent node pointer        <w>           */
    UINT32      color;          /* Color attribute            <b>           */
    UINT64      keym;           /* Key endpoint               <l>           */
    UINT64      nodem;          /* Node endpoint              <l>           */
    struct RB  *fthd;           /* Forward thread pointer     <w>           */
    struct RB  *bthd;           /* Backward thread pointer    <w>           */
    struct TG  *dPoint;         /* Pointer to Tag (data payload)<w>         */
} RB;

#endif /* _RB_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
