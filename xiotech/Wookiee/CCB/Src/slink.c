/* $Id: slink.c 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       slink.c
** MODULE TITLE:    Linked list implementation (singly linked)
**
** DESCRIPTION:     Generic singly linked list implementation
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "slink.h"

#include "misc.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static S_NODE *allocateNode(void *e);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*===========================================================================
** Name:    S_LIST *CreateList(void);
**
** Desc:    Creates a singly linked list
**
** In:      none
**
** Returns: Pointer to a list
**==========================================================================*/
S_LIST     *CreateList(void)
{
    return MallocWC(sizeof(S_LIST));
}


/*===========================================================================
** Name:    UINT8 DeleteList(S_LIST *list);
**
** Desc:    Removes the entire list
**
** In:      S_LIST *list    Pointer to a singly linked list
**
** Returns: NONE
**
** NOTE:    User should remove all the elements first then call this as this
**          function will remove all nodes, but not the memory that each
**          pointer in the list points to, that is the responsibility of the
**          user
**
**==========================================================================*/
void DeleteList(S_LIST *list)
{
    S_NODE     *tmp = NULL;
    S_NODE     *toDel = NULL;

    ccb_assert(list != NULL, list);

    tmp = list->head;

    while (tmp != NULL)
    {
        toDel = tmp;
        tmp = tmp->next;
        Free(toDel);
    }
    Free(list);
}


/*===========================================================================
** Name:    UINT32 NumberOfItems(S_LIST *list);
**
** Desc:    Returns the number of elements in the list
**
** In:      S_LIST *list    Pointer to a singly linked list
**
** Returns: Number of elements in the list
**==========================================================================*/
UINT32 NumberOfItems(S_LIST *list)
{
    ccb_assert(list != NULL, list);

    return list->numElements;
}


/*===========================================================================
** Name:    void AddElement(S_LIST *list, void *e);
**
** Desc:    Adds an element to a list
**
** In:      S_LIST *list    Pointer to a singly linked list
**          Void *e Pointer to the element to add
**==========================================================================*/
void AddElement(S_LIST *list, void *e)
{
    Enqueue(list, e);
}


/*===========================================================================
** Name:    void * RemoveElement(S_LIST *list, void const *value,
**                       int (*compare)(void const *e1, void const *e2));
**
** Desc:    Removes an element from a list
**
** In:      S_LIST *list    Pointer to a singly linked list
**          void *value     Pointer to the value that should be used
**                          for comparison
**
**          Pointer to a function that does compare (see documentation above)
**
** Returns: Pointer to element that was removed
**==========================================================================*/
void       *RemoveElement(S_LIST *list, void const *value,
                          int (*compare)(void const *e1, void const *e2))
{
    S_NODE     *prev;
    S_NODE     *toDel;
    void       *e = NULL;

    ccb_assert(list != NULL, list);
    ccb_assert(value != NULL, value);

    prev = list->head;
    toDel = list->head;

    while (toDel != NULL)
    {
        if (compare(toDel->element, value) == 0)
        {
            /* Set return value and decrement counter */
            e = toDel->element;
            list->numElements--;

            /* List is going empty */
            if (list->head == list->tail)
            {
                list->head = list->tail = NULL;
            }

            /* We are removing at the beginning of the list */
            else if (toDel == list->head)
            {
                list->head = toDel->next;
            }

            /* Default case with one exception */
            else
            {
                /*
                 * We are removing the last element in the list
                 * so reset tail to prev node
                 */
                if (toDel == list->tail)
                {
                    list->tail = prev;
                }
                prev->next = toDel->next;
            }

            /* Delete node */
            Free(toDel);
            break;
        }
        prev = toDel;
        toDel = toDel->next;
    }
    return e;
}


/*===========================================================================
** Name:    void * FindElement(S_LIST *list, void const *value,
**                       int (*compare)(void const *e1, void const *e2));
**
** Desc:    Finds an element in a list
**
** In:      S_LIST *list    Pointer to a singly linked list
**          void *value     Pointer to the value that should be used
**                          for comparison
**
**          Pointer to a function that does compare (see documentation above)
**
** Returns: Pointer to element that matches, else null
**==========================================================================*/
void       *FindElement(S_LIST *list, void const *value,
                        int (*compare)(void const *e1, void const *e2))
{
    S_NODE     *tmp = NULL;
    void       *e = NULL;

    ccb_assert(list != NULL, list);
    ccb_assert(value != NULL, value);

    tmp = list->head;

    while (tmp != NULL)
    {
        if (compare(tmp->element, value) == 0)
        {
            e = tmp->element;
            break;
        }
        tmp = tmp->next;
    }
    return e;
}


/*===========================================================================
** Name:    void Enqueue(S_LIST *list, void *e);
**
** Desc:    Enqueues an element to the list
**
** In:      S_LIST *list    Pointer to a singly linked list
**          void *e         Pointer to element to be stored
**
** Returns: SUCCESS if completed without errors, else returns FAILURE
**==========================================================================*/
void Enqueue(S_LIST *list, void *e)
{
    S_NODE     *newNode = NULL;

    ccb_assert(list != NULL, list);
    ccb_assert(e != NULL, e);

    newNode = allocateNode(e);

    /* Increment the number of elements */
    list->numElements++;

    /* Check to see if this is the first addtion to the list */
    if (list->head == NULL)
    {
        list->head = newNode;
        list->tail = newNode;
    }
    else
    {
        /* Set the end of the list to the new node */
        list->tail->next = newNode;

        /* Set the tail pointer to the new end of the list */
        list->tail = newNode;
    }
}


/*===========================================================================
** Name:    void *Dequeue(S_LIST *list);
**
** Desc:    Dequeues an element from the list
**
** In:      S_LIST *list    Pointer to a singly linked list
**
** Returns: Pointer to element removed if sucessful, else returns NULL
**==========================================================================*/
void       *Dequeue(S_LIST *list)
{
    void       *e = NULL;       /* Return value */
    S_NODE     *oneToDel = NULL;        /* Node we are deleting */

    ccb_assert(list != NULL, list);

    /* Initialize to the beginning */
    oneToDel = list->head;

    if (list->head != NULL)
    {
        /* Decrement the number of elements */
        list->numElements--;

        /* Assign the return value */
        e = list->head->element;

        /* Check to see if this is the last one */
        if (list->head == list->tail)
        {
            list->head = list->tail = NULL;
        }
        else
        {
            list->head = list->head->next;
        }

        /* Delete node */
        Free(oneToDel);
    }
    return e;
}


/*===========================================================================
** Name:    void *pop(S_LIST *list);
**
** Desc:    Pops an element from the list
**
** In:      S_LIST     *list    Pointer to a singly linked list
**
** Returns: Pointer to element removed if sucessful, else returns NULL
**==========================================================================*/
void       *Pop(S_LIST *list)
{
    ccb_assert(list != NULL, list);
    return Dequeue(list);
}


/*===========================================================================
** Name:    S_NODE *allocateNode(void *e)
**
** Desc:    Private function to allocate a node and intialize it
**
** In:      void *e     Pointer to users data
**
**
** Returns: Pointer to new node, else NULL
**==========================================================================*/
static S_NODE *allocateNode(void *e)
{
    S_NODE     *newNode = MallocWC(sizeof(*newNode));

    /*
     * Set up the next and values of the new node
     * We are appending to the end for the add element
     */
    newNode->next = NULL;
    newNode->element = e;

    return newNode;
}


/*===========================================================================
** Name:    void SetIterator(S_LIST *list);
**
** Desc:    Initializes the Iterator to the beginning of the list
**
** In:      S_LIST     *list    Pointer to a singly linked list
**
** Returns: None
**==========================================================================*/
void SetIterator(S_LIST *list)
{
    ccb_assert(list != NULL, list);
    list->iterator = list->head;
}


/*===========================================================================
** Name:    void Iterate(S_LIST *list);
**
** Desc:    Transverse each item in the container
**
** In:      S_LIST     *list    Pointer to a singly linked list
**
** Returns: Pointer to the next item in the list or NULL when we reach the
**          end of the container
**
**          Note:   There is only one iterator pointer per list instance
**                  so if you share a list you are respondsible for makeing
**                  sure that two different threads of execution don't step
**                  on each other while using.
**
**          Call SetIterator before calling Iterate
**==========================================================================*/
void       *Iterate(S_LIST *list)
{
    void       *rc = NULL;

    ccb_assert(list != NULL, list);

    if (list->iterator)
    {
        rc = list->iterator->element;
        list->iterator = list->iterator->next;
    }
    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
