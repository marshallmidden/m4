/* jgraph.c
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/jgraph.c,v $
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
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jgraph.h"

/* ------------------------------------------------------------------------ */
int NMARKTYPES = 17;
int NORMALMARKTYPES = 6;

const char *MARKTYPESTRS[] = { "circle", "box", "diamond", "triangle", "x", "cross", 
                         "ellipse", "general", "general_nf", "general_bez",
                         "general_bez_nf", "postscript", "eps", 
                         "xbar", "ybar", "none", "text"};
const char MARKTYPES[] = {     'o',      'b',   'd',       't',        'x', 'c', 
			 'e',       'g',       'G',          'Z',
                         'B',              'p',          'E',
                         'X',    'Y',    'n',    'l' };

int NPATTERNS = 3;
const char *PATTERNS[] = { "solid", "stripe", "estripe" };
const char PTYPES[] = { 's', '/', 'e' };

/* ------------------------------------------------------------------------ */
Label new_label(void)
{
  Label l;
  int i;

  l = (Label) malloc (sizeof(struct label));
  l->label = CNULL;
  l->hj = 'c';
  l->vj = 'b';
  l->font = "Times-Roman";
  l->fontsize = 9.0;
  l->rotate = 0;
  l->graytype = 'n';
  for (i = 0; i < 3; i++) l->gray[i] = 0.0;
  l->linesep = FSIG;

  return l;
}

/* ------------------------------------------------------------------------ */
Curve new_curve(Curve c, int num)
{
  Curve new_c;
  int i;

  new_c = (Curve) get_node((List) c);
  new_c->num = num;
  new_c->l = new_label();
  new_c->l->vj = 't';
  new_c->lmark = new_label();
  new_c->lmark->hj = 'c';
  new_c->lmark->vj = 'c';
  new_c->clip = 0;
  new_c->pattern = 's';
  new_c->parg = FSIG;
  new_c->apattern = 's';
  new_c->aparg = FSIG;
  new_c->ppattern = 's';
  new_c->pparg = FSIG;
  new_c->graytype = 'n';
  for (i = 0; i < 3; i++) new_c->gray[i] = 0.0;
  new_c->afilltype = 'g';
  for (i = 0; i < 3; i++) new_c->afill[i] = 0.0;
  new_c->pts = (Point) make_list(sizeof(struct point));
  new_c->yepts = (Point) make_list(sizeof(struct point));
  new_c->xepts = (Point) make_list(sizeof(struct point));
  new_c->npts = 0;
  new_c->gen_linetype = (Flist) make_list(sizeof(struct flist));
  new_c->marktype = MARKTYPES[num % NORMALMARKTYPES];
  new_c->linetype = '0';
  new_c->linethick = 1.0;
  new_c->marksize[0] = FSIG;
  new_c->marksize[1] = FSIG;
  new_c->mrotate = 0.0;
  new_c->general_marks = (Point) make_list(sizeof(struct point));
  new_c->filltype = 'n';
  for (i = 0; i < 3; i++) new_c->fill[i] = 0.0;
  new_c->pfilltype = 'n';
  for (i = 0; i < 3; i++) new_c->pfill[i] = 0.0;
  new_c->poly = 0;
  new_c->rarrow = 0;
  new_c->larrow = 0;
  new_c->rarrows = 0;
  new_c->larrows = 0;
  new_c->asize[0] = FSIG;
  new_c->asize[1] = FSIG;
  new_c->bezier = 0;
  new_c->postscript = CNULL;
  new_c->postfile = 0;
  new_c->eps = CNULL;
  prio_insert((Prio_list)new_c, (Prio_list)c, 0);
  return new_c;
}

/* ------------------------------------------------------------------------ */
Curve new_line(Curve c, int num)
{
  Curve new_c;
  new_c = new_curve(c, num);
  new_c->linetype = 's';
  new_c->marktype = 'n';
  return new_c;
}

/* ------------------------------------------------------------------------ */
Curve get_curve(Curve c, int num)
{
  Curve new_c;
  for(new_c = last(c); new_c != nil(c) && new_c->num > num; new_c = prev(new_c));
  if (new_c == nil(c) || new_c->num < num) return new_curve(c, num);
  return new_c;
}

/* ------------------------------------------------------------------------ */
static Axis new_axis(int is_x)
{
  int i;
  Axis a;

  a = (Axis) malloc (sizeof(struct axis));
  a->label = new_label();
  a->label->x = FSIG;
  a->label->y = FSIG;
  a->label->font = "Times-Bold";
  a->label->fontsize = 10.0;
  a->label->rotate = FSIG;
  a->label->hj = '0';
  a->label->vj = '0';
  a->size = 3.0;
  a->max = FSIG;
  a->min = FSIG;
  a->pmax = FSIG;
  a->pmin = FSIG;
  a->hash_interval = -1.0;
  a->log_base = 10.0;
  a->minor_hashes = -1;
  a->precision = -1;
  a->hl = new_label();
  a->hl->label = strdup("");
  a->hl->font = "Times-Roman";
  a->hl->fontsize = 9.0;
  a->hl->rotate = 0.0;
  a->hl->hj = '0';
  a->hl->vj = '0';
  a->hash_format = 'f';
  a->draw_at = FSIG;
  a->draw_hash_marks_at = FSIG;
  a->draw_hash_labels_at = FSIG;
  a->draw_hash_labels = 1;
  a->draw_axis_line = 1;
  a->draw_hash_marks = 1;
  a->draw_axis_label = 1;
  a->auto_hash_labels = 1;
  a->auto_hash_marks = 1;
  a->start_given = 0;
  a->hash_scale = -1.0;
  a->grid_lines = 0;
  a->mgrid_lines = 0;
  a->graytype = 'n';
  for (i = 0; i < 3; i++) a->gray[i] = 0.0;
  a->gr_graytype = '0';
  for (i = 0; i < 3; i++) a->gr_gray[i] = 0.0;
  a->mgr_graytype = '0';
  for (i = 0; i < 3; i++) a->mgr_gray[i] = 0.0;
  a->is_x = is_x;
  a->is_lg = 0;
  a->hash_labels = (String) make_list (sizeof(struct string));
  a->hash_lines = (Hash) make_list (sizeof(struct hash));
  return a;
}

/* ------------------------------------------------------------------------ */
static Legend new_legend(void)
{
  Legend l;
  l = (Legend) malloc (sizeof(struct legend));
  l->linelength = FSIG;
  l->linebreak = FSIG;
  l->midspace = FSIG;
  l->type = 'u';
  l->l = new_label();
  l->l->label = strdup("");
  l->l->hj = 'l';
  l->l->vj = 'c';
  l->l->x = FSIG;
  l->l->y = FSIG;
  return l;
}

/* ------------------------------------------------------------------------ */
static Label new_title(void)
{
  Label t;

  t = new_label();
  t->x = FSIG;
  t->y = FSIG;
  t->rotate = 0.0;
  t->hj = 'c';
  t->vj = 't';
  t->fontsize = 12.0;
  return t;
}

/* ------------------------------------------------------------------------ */
String new_string(String s, int num)
{
  String new_s;

  new_s = (String) get_node((List) s);
  new_s->num = num;
  new_s->s = new_label();
  prio_insert((Prio_list)new_s, (Prio_list)s, 0);
  return new_s;
}

/* ------------------------------------------------------------------------ */
String get_string(String s, int num)
{
  String new_s;

  for(new_s = last(s); new_s != nil(s) && new_s->num > num; new_s = prev(new_s));
  if (new_s == nil(s) || new_s->num < num) return new_string(s, num);
  return new_s;
}

/* ------------------------------------------------------------------------ */
Graph new_graph(Graph gs, int num)
{
  Graph g;

  g = (Graph) get_node((List) gs);
  g->num = num;
  g->xminval = 0.0;
  g->yminval = 0.0;
  g->xmaxval = 0.0;
  g->ymaxval = 0.0;
  g->x_axis = new_axis(1);
  g->y_axis = new_axis(0);
  g->x_translate = 0.0;
  g->y_translate = 0.0;
  g->curves = (Curve) make_list(sizeof(struct curve));
  g->strings = (String) make_list(sizeof(struct string));
  g->title = new_title();
  g->clip = 0;
  g->border = 0;
  g->legend = new_legend();
/*  g->def = new_default(); */
  prio_insert((Prio_list)g, (Prio_list)gs, 0);
  return g;
}

/* ------------------------------------------------------------------------ */
Graph get_graph(Graph g, int num)
{
  Graph new_g;
  for(new_g = last(g); new_g != nil(g) && new_g->num > num; new_g = prev(new_g));
  if (new_g == nil(g) || new_g->num < num) return new_graph(g, num);
  return new_g;
}

/* ------------------------------------------------------------------------ */
void new_graphs(Graphs gs)
{
  Graphs newg;

  newg = (Graphs) get_node((List) gs);
  newg->g = (Graph) make_list(sizeof(struct graph));
  newg->height = 0.0;
  newg->width = 0.0;
  newg->bb[0] = ISIG; newg->bb[1] = ISIG;
  newg->bb[2] = ISIG; newg->bb[3] = ISIG;
  newg->preamble = CNULL;
  newg->epilogue = CNULL;
  newg->prefile = 0;
  newg->epifile = 0;
  if (first(gs) == nil(gs)) newg->page = 1; else newg->page = last(gs)->page+1;
  insert((List) newg, (List) gs);
}

/* ------------------------------------------------------------------------ */
int main(int argc, char **argv)
{
  Graphs gs;
  int i;
  int show, pp;
  int landscape;
  int comments;
  int nfiles;

  show = 0;
  pp = 0;
  comments = 0;
  landscape = 0;
  nfiles = 0;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) show = 1;
    else if (strcmp(argv[i], "-comments") == 0) comments = 1;
    else if (strcmp(argv[i], "-P") == 0) pp = 1;
    else if (strcmp(argv[i], "-L") == 0) landscape = 1;
    else {
      nfiles++;
      set_input_file(argv[i]);
    }
  }
  if (nfiles == 0) set_input_file(CNULL);
  gs = (Graphs) make_list(sizeof(struct graphs));
  set_comment(comments);
  new_graphs(gs);
  edit_graphs(gs);
  process_graphs(gs);
  if (show) show_graphs(gs); else draw_graphs(gs, pp, landscape);
  exit(0);
  return 0;
}

/* ------------------------------------------------------------------------ */
/* End of file jgraph.c */
