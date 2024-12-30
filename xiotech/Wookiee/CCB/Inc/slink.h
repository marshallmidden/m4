/* $Id: slink.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       slink.h
** MODULE TITLE:    Header file for slink.c
**
** DESCRIPTION:     A generic singly linked list that anyone can use.  It
**                  also has the ability to act as a queue or a stack.
**
**                  Note: If you use the list as a stack or as a queue
**                  do not mix the use of the other functions as your
**                  list will get mixed up e.g. using the list as a queue
**                  and then using push, pop or AddElement would not be good.
** NOTES:
**  This library uses a function pointer for the FindElement and
**  RemoveElement.  This function needs to be supplied by the caller for the
**  given data type that one is storing in the linked list.  The function
**  needs to have the following signature:
**
**  int compare(void const *element1, void const *element2);
**
**  With the following return values:
**      -1  if element1 < element2
**      0   if element1 == element2
**      1   if element1 > element2
**
** EXAMPLE: If one was storing integers in the linked list this would be
**          your compare function.
**
**      int compare( void const *a, void const *b)
**      {
**          if( *(int *)a > *(int *)b )
**          {
**              return 1;
**          }
**          else if ( *(int *)a < *(int *)b )
**          {
**              return -1;
**          }
**          else
**          {
**              return 0;
**          }
**      }
**
**  Even though today we are only using == in the library, if you follow this
**  we can then add sorting functionality etc. to the list later and your code
**  will automatically take advantage of that :-)
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**
**==========================================================================*/
#ifndef _SLINK_H_
#define _SLINK_H_

#include "XIO_Types.h"
#include "stddef.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#ifndef offsetof
#define offsetof(type, memb)((size_t)&((type *)0)->memb)
#endif /* offsetof */

/* Structure for each node in the list */
typedef struct _SNODE
{
    struct _SNODE *next;
    void       *element;
} S_NODE;

/* Structure for the list header */
typedef struct _SLIST
{
    UINT32      numElements;
    S_NODE     *head;
    S_NODE     *tail;
    S_NODE     *iterator;
} S_LIST;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern S_LIST *CreateList(void);
extern void DeleteList(S_LIST *);
extern UINT32 NumberOfItems(S_LIST *);
extern void AddElement(S_LIST *, void *);
extern void *RemoveElement(S_LIST *, void const *, int (*compare)(void const *, void const *));
extern void *FindElement(S_LIST *, void const *, int (*compare)(void const *, void const *));
extern void Enqueue(S_LIST *list, void *);
extern void *Dequeue(S_LIST *);
extern void *Pop(S_LIST *);
extern void SetIterator(S_LIST *);
extern void *Iterate(S_LIST *);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SLINK_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
