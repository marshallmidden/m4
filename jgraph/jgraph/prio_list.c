/* prio_list.c
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/prio_list.c,v $
 * $Revision: 8.5 $
 * $Date: 2017/11/28 17:33:27 $
 * $Author: plank $

James S. Plank
Department of Electrical Engineering and Computer Science
University of Tennessee
Knoxville, TN 37996
plank@cs.utk.edu

Copyright (c) 2011, James S. Plank
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 - Neither the name of the University of Tennessee nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
/* ------------------------------------------------------------------------ */

#include "jgraph.h"
/* #include "list.h" */
#include "prio_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
/* Prio_insert inserts nodes into their proper places in priority lists.  It first 
 * checks for inserting into the head or tail, and then proceeds sequentially.
 * Thus, it is worst case linear, but for most cases constant time (right). */

void prio_insert(Prio_list node, Prio_list list, Boolean desc)
{
  Prio_list p;

  /* Check nil and head of list */
  if (first(list) == nil(list) || 
      (!desc && first(list)->prio >= node->prio) ||
      (desc  && first(list)->prio <= node->prio) ) {
    node->blink = list;
    node->flink = list->flink;
    list->flink->blink = node;
    list->flink = node;
    return;
  }
  /* Check tail of list */
  if ((desc  && last(list)->prio >= node->prio) ||
      (!desc && last(list)->prio <= node->prio) ) {
    node->flink = list;
    node->blink = list->blink;
    list->blink->flink = node;
    list->blink = node;
    return;
  }
  /* Check the rest of the list sequentially */
  for(p = next(first(list));  ; p = next(p)) {
    if (p == nil(list)) fprintf(stderr, "inserting into tail did not work\n");
    if ((!desc && p->prio >= node->prio) ||
        (desc  && p->prio <= node->prio)) {
      node->flink = p;
      node->blink = p->blink;
      p->blink->flink = node;
      p->blink = node;
      return;
    }
  }
}
      
/* ------------------------------------------------------------------------ */
/* End of file prio_list.c */
