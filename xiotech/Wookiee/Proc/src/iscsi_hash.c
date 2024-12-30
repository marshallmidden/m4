/* $Id: iscsi_hash.c 144139 2010-07-14 19:46:01Z m4 $ */
/**
 ******************************************************************************
 **
 **  @file       iscsi_hash.c
 **
 **  @brief      Hash table APIs(basic)
 **
 **  This provides APIs for adding/deleting to a hash table
 **
 **  Copyright (c) 2005-2010 Xiotech Corporation.  All rights reserved.
 **
 ******************************************************************************
 **/

#include <stdio.h>
#include <stdlib.h>

#include "XIO_Types.h"
#include "XIO_Std.h"
#include "iscsi_common.h"
#include "iscsi_pdu.h"

/**
 ******************************************************************************
 **
 **  @brief      hash
 **
 **              Hash algorithm, it uses a prime number-sized table
 **
 **  @param      key        - key
 **
 **  @return     hash index
 **
 ******************************************************************************
 **/
static hashIndexType hash(UINT32 Key)
{
    UINT32  hash_ix = Key % HASH_TABLE_SIZE;

    return (hashIndexType)hash_ix;
}

/**
 ******************************************************************************
 **
 **  @brief      hash_insert
 **
 **              Inserting into hash table
 **
 **  @param      hashTable  - Hash table
 **  @param      key        - key
 **  @param      rec        - record to be inserted
 **  @param      ttt        - ttt
 **
 **  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
 **
 ******************************************************************************
 **/
UINT8 hash_insert(hash_node **hashTable,UINT32 key, ILT *rec, UINT32 ttt)
{
    hash_node *insert_node;

    /*
    **  Allocate node for data and insert in table,
    **  insert at beginning of list; if not existing
    */
    insert_node = hash_lookup(hashTable, key, ttt);
    if (insert_node == NULL)
    {
        hashIndexType   hash_ix;

        insert_node = (hash_node *)s_MallocC(sizeof(hash_node), __FILE__, __LINE__);
        if (insert_node == NULL)
        {
            return XIO_FAILURE;
        }

        hash_ix = hash(key);
        insert_node->pNext = hashTable[hash_ix];
        hashTable[hash_ix] = insert_node;

        /* Update key, ttt, rec */

        insert_node->key = key;
        insert_node->ttt = ttt;
        insert_node->rec = rec;
    }
    else
    {
        /*
        ** Warn that node is existing;
        ** this should never happen, added print just incase we hit
        */
        fprintf(stderr, "%s: node already exists for itt %x, ttt %x\n",
            __func__, key, ttt);
    }
    return XIO_SUCCESS;
}

/**
******************************************************************************
**
**  @brief      hash_delete
**
**              Deleting a node from hash table
**
**  @param      hashTable  - Hash table
**  @param      key        - key
**  @param      ttt        - ttt
**
**  @return     XIO_SUCCESS on success XIO_FAILURE otherwise
**
******************************************************************************
**/
UINT8 hash_delete(hash_node **hashTable, UINT32 key, UINT32 ttt)
{
    hash_node *curr_node;
    hash_node *prev_node   = NULL;
    hashIndexType   hash_ix;

    hash_ix = hash(key);
    for (curr_node = hashTable[hash_ix]; curr_node != NULL;
            prev_node = curr_node, curr_node = curr_node->pNext)
    {
        if (curr_node->key == key && curr_node->ttt == ttt)
        {
            /* found node */

           if (prev_node == NULL)
           {
               /* first node on chain, remove and free the node */

               hashTable[hash_ix] = curr_node->pNext;
           }
           else
           {
               /* remove the node */

               prev_node->pNext = curr_node->pNext;
           }

           /* free the node */

           s_Free(curr_node, sizeof(*curr_node), __FILE__, __LINE__);
           return XIO_SUCCESS;
        }
    }
    return XIO_FAILURE;
}

/**
 ******************************************************************************
 **
 **  @brief      hash_find
 **
 **              Finding a node in hash table
 **
 **  @param      hashTable  - Hash table
 **  @param      key        - key
 **  @param      ttt        - ttt
 **
 **  @return     record on success otherwise NULL
 **
 ******************************************************************************
 **/
ILT *hash_find(hash_node **hashTable, UINT32 key, UINT32 ttt)
{
    hash_node *curr_node;

    for (curr_node = hashTable[hash(key)]; curr_node != NULL;
            curr_node = curr_node->pNext)
    {
        /* return ILT matching key & ttt */

        if (curr_node->key == key && curr_node->ttt == ttt)
        {
            return curr_node->rec;
        }
    }
    return NULL;
}


/**
 ******************************************************************************
 **
 **  @brief      hash_lookup
 **
 **              Lookup of a hash_node if already existing
 **
 **  @param      hashTable  - Hash table
 **  @param      key        - key
 **  @param      ttt        - ttt
 **
 **  @return     hash_node  on success otherwise NULL
 **
 ******************************************************************************
 **/
hash_node *hash_lookup(hash_node **hashTable, UINT32 key, UINT32 ttt)
{
    hash_node *curr_node;

    for (curr_node = hashTable[hash(key)]; curr_node != NULL;
            curr_node = curr_node->pNext)
    {
        /* Return ILT matching key & ttt */

        if (curr_node->key == key && curr_node->ttt == ttt)
        {
            return curr_node;
        }
    }
    return NULL;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
