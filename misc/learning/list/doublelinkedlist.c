/*
 * vim: ts=4 sts=4 et sw=4
 */

/* ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
#define TRUE  1
#define FALSE 0

struct node
{
    int             data;
    int             key;
    struct node    *next;
    struct node    *prev;
};

struct node_list
{
    struct node    *first;
    struct node    *last;
    unsigned int    count;
};

/* ------------------------------------------------------------------------ */
// Check if list empty.
static int dll_isEmpty(struct node_list *list)
{
    return (list->first == NULL);
}   /* End of dll_isEmpty */

/* ------------------------------------------------------------------------ */
// Display list from first to last.
static void dll_displayForward(struct node_list *list)
{
    struct node    *ptr = list->first;

    printf("dll_displayForward\n");
    while (ptr != NULL)
    {
        printf("(%d,%d) ", ptr->key, ptr->data);
        ptr = ptr->next;
    }
    printf("\n");
}   /* End of dll_displayForward */

/* ------------------------------------------------------------------------ */
// Display list from last to first.
static void dll_displayBackward(struct node_list *list)
{
    struct node    *ptr = list->last;

    printf("dll_displayBackward\n");
    while (ptr != NULL)
    {
        printf("(%d,%d) ", ptr->key, ptr->data);
        ptr = ptr->prev;
    }
    printf("\n");
}   /* End of dll_displayBackward */

/* ------------------------------------------------------------------------ */
// Insert link at first location.
static void dll_insertFirst(struct node_list *list, int key, int data)
{
    struct node    *new_node = (struct node *)malloc(sizeof(struct node));

    new_node->key = key;
    new_node->data = data;
    if (dll_isEmpty(list))
    {
        list->last = new_node;
    }
    else
    {
        list->first->prev = new_node;
    }
    new_node->next = list->first;
    new_node->prev = NULL;
    list->first = new_node;
}   /* End of dll_insertFirst */

/* ------------------------------------------------------------------------ */
// Insert link at the last location.
static void dll_insertLast(struct node_list *list, int key, int data)
{
    struct node    *new_node = (struct node *)malloc(sizeof(struct node));

    new_node->key = key;
    new_node->data = data;
    if (dll_isEmpty(list))
    {
        list->last = new_node;
    }
    else
    {
        list->last->next = new_node;
        new_node->prev = list->last;
    }
    list->last = new_node;
}   /* End of dll_insertLast */

/* ------------------------------------------------------------------------ */
// Delete first item.
static struct node    *dll_deleteFirst(struct node_list *list)
{
    struct node    *temp_node = list->first;

    if (list->first->next == NULL)
    {
        list->last = NULL;
    }
    else
    {
        list->first->next->prev = NULL;
    }
    list->first = list->first->next;
    return temp_node;
}   /* End of dll_deleteFirst */

/* ------------------------------------------------------------------------ */
// Delete link at the last location.
static struct node    *dll_deleteLast(struct node_list *list)
{
    struct node    *temp_node = list->last;

    if (list->first->next == NULL)
    {
        list->first = NULL;
    }
    else
    {
        list->last->prev->next = NULL;
    }
    list->last = list->last->prev;
    return temp_node;
}   /* End of dll_deleteLast */

/* ------------------------------------------------------------------------ */
// Delete a link with given key.
static struct node    *dll_delete(struct node_list *list, int key)
{
    struct node    *current = list->first;
    struct node    *previous = NULL;

    if (list->first == NULL)
    {
        return NULL;
    }
    while (current->key != key)
    {
        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }
    if (current == list->first)
    {
        list->first = list->first->next;
    }
    else
    {
        current->prev->next = current->next;
    }
    if (current == list->last)
    {
        list->last = current->prev;
    }
    else
    {
        current->next->prev = current->prev;
    }
    return current;
}   /* End of dll_delete */

/* ------------------------------------------------------------------------ */
static int dll_insertAfter(struct node_list *list, int key, int newKey, int data)
{
    struct node    *current = list->first;
    struct node    *new_node;

    if (list->first == NULL)
    {
        return FALSE;
    }
    while (current->key != key)
    {
        if (current->next == NULL)
        {
            return FALSE;
        }
        else
        {
            current = current->next;
        }
    }
    new_node = (struct node *)malloc(sizeof(struct node));
    new_node->key = newKey;
    new_node->data = data;
    if (current == list->last)
    {
        new_node->next = NULL;
        list->last = new_node;
    }
    else
    {
        new_node->next = current->next;
        current->next->prev = new_node;
    }
    new_node->prev = current;
    current->next = new_node;
    return TRUE;
}   /* End of dll_insertAfter */

/* ------------------------------------------------------------------------ */
int main(void)
{
    struct node_list *the_list = (struct node_list *)malloc(sizeof the_list);
    struct node *dn;                    /* Delete node! */

    the_list->first = NULL;
    the_list->last = NULL;
    the_list->count = 0;

    dll_insertFirst(the_list, 1, 10);
    dll_insertFirst(the_list, 2, 20);
    dll_insertFirst(the_list, 3, 30);
    dll_insertFirst(the_list, 4, 1);
    dll_insertFirst(the_list, 5, 40);
    dll_insertFirst(the_list, 6, 56);

    dll_displayForward(the_list);

    dll_displayBackward(the_list);

    dn = dll_deleteFirst(the_list);
    if (dn != NULL)
    {
        printf("After deleting first record:\n");
        dll_displayForward(the_list);
    } else {
        printf("dll_deleteFirst returned NULL - nothing to delete!\n");
    }

    dn = dll_deleteLast(the_list);
    if (dn != NULL)
    {
        printf("After deleting last record:\n");
        dll_displayForward(the_list);
    } else {
        printf("dll_deleteFirst returned NULL - nothing to delete!\n");
    }

    if (dll_insertAfter(the_list, 4, 7, 13))
    {
        printf("Insert after key(4):\n");
        dll_displayForward(the_list);
    } else {
        printf("dll_insertAfter returned FALSE!\n");
    }

    dn = dll_delete(the_list, 4);
    if (dn != NULL)
    {
        printf("After delete key(4):\n");
        dll_displayForward(the_list);
    } else {
        printf("No entry 4 to delete!\n");
    }

    dll_insertLast(the_list, 8, 88);
    printf("After dll_insertLast key(8):\n");
    dll_displayForward(the_list);
}   /* End of main */

/* ------------------------------------------------------------------------ */
