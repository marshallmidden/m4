/* show.c
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/show.c,v $
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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "jgraph.h"

/* ------------------------------------------------------------------------ */
static void spaces(int nsp)
{
  while(nsp-- > 0) putchar(' ');
}

/* ------------------------------------------------------------------------ */
static double ptoin(double p)
{
  return p / FCPI;
}

/* ------------------------------------------------------------------------ */
double ptoc(double p, Axis a)
{
  if (a->is_lg) {
    return (double) exp((p / a->factor + a->logmin) * a->logfactor);
  } else {
    return (p / a->factor) + a->min;
  }
}

/* ------------------------------------------------------------------------ */
double ptodist(double p, Axis a)
{
  if (a->is_lg) {
    return p  / FCPI;
  } else {
    return (p / a->factor);
  }
}

/* ------------------------------------------------------------------------ */
static void show_mltiline(char *s)
{
  int i;

  if (s != CNULL) {
    for (i = 0; s[i] != '\0'; i++) {
      if (s[i] == '\n') putchar('\\');
      putchar(s[i]);
    }
  }
  putchar('\n');
}

/* ------------------------------------------------------------------------ */
static void show_string(char *s)
{
  int i;

  if (s != CNULL) {
    printf(": ");
    for (i = 0; s[i] != '\0'; i++) {
      if (s[i] == '\n') putchar('\\');
      if (s[i] == '\\') i++;
      putchar(s[i]);
    }
  }
  putchar('\n');
}
    
/* ------------------------------------------------------------------------ */
static void show_label(Label l, int nsp, Graph g)
{
  spaces(nsp); 
  show_string(l->label);
  spaces(nsp); printf("x %f ", ptoc(l->x, g->x_axis));
               printf("y %f\n", ptoc(l->y, g->y_axis));
  spaces(nsp); printf("hj%c vj%c ", l->hj, l->vj);
               printf("rotate %f\n", l->rotate);
  spaces(nsp); printf("font %s ", l->font);
               printf("fontsize %f ", l->fontsize);
	       printf("linesep %f\n", l->linesep);
  if (l->graytype == 'g') {
    spaces(nsp);
    printf("lgray %f\n", l->gray[0]);
  } else if (l->graytype == 'c') {
    spaces(nsp);
    printf("lcolor %f %f %f\n", l->gray[0], l->gray[1], l->gray[2]);
  }
  return;
}

/* ------------------------------------------------------------------------ */
static void show_lmark(Label l, int nsp, Graph g)
{
  spaces(nsp); show_string(l->label);
  spaces(nsp); printf("x %f ", ptodist(l->x, g->x_axis));
               printf("y %f\n", ptodist(l->y, g->y_axis));
  spaces(nsp); printf("hj%c vj%c ", l->hj, l->vj);
               printf("rotate %f\n", l->rotate);
  spaces(nsp); printf("font %s ", l->font);
               printf("fontsize %f\n", l->fontsize);
  return;
}

/* ------------------------------------------------------------------------ */
static void show_curve(Curve c, int nsp, Graph g)
{
  Point p;
  Point px;
  Point py;
  int i;
  Flist fl;

  if (c->l->label != CNULL) {
    spaces(nsp); printf("label\n");
    spaces(nsp+2);
    printf("(* unless <legend custom>, this label\'s x\'s and y\'s will be ignored *)\n");
    show_label(c->l, nsp+2, g);
  }
  
  px = first(c->xepts);
  py = first(c->yepts);
  for(p = first(c->pts); p != nil(c->pts); p = next(p)) {
    if (p->e == 'p') {
      spaces(nsp); printf("pts %f %f\n", p->x, p->y);
    } else if (p->e == 'x') {
      spaces(nsp); 
      printf("x_epts %f %f %f %f\n", p->x, p->y, px->x, next(px)->x);
      px = next(next(px));
    } else if (p->e == 'y') {
      spaces(nsp); 
      printf("y_epts %f %f %f %f\n", p->y, p->y, py->y, next(py)->y);
      py = next(next(py));
    } else {
      fprintf(stderr, "Internal error: p->e == %c\n", p->e);
      exit(1);
    }
  }
  if (c->eps != CNULL) {
    spaces(nsp);
    printf("eps %s\n", c->eps);
  }
  if (c->postscript != CNULL) {
    spaces(nsp);
    printf("postscript ");
    if (!c->postfile) printf(": ");
    show_mltiline(c->postscript);
  }
  spaces(nsp); printf("marktype ");
  for (i = 0; i < NMARKTYPES && c->marktype != MARKTYPES[i]; i++) ;
  if (i == NMARKTYPES) {
    error_header();
    fprintf(stderr, "Unknown mark type %c\n", c->marktype);
    exit(1);
  } else printf("%s ", MARKTYPESTRS[i]);
  if (c->marktype == 'l') {
    show_lmark(c->lmark, nsp+2, g);
    spaces(nsp);
  }
  printf("marksize %f %f ", ptodist(c->marksize[0], g->x_axis), 
                            ptodist(c->marksize[1], g->y_axis));
  printf("mrotate %f ", c->mrotate);
  if (c->filltype == 'g') {
     printf("fill %f\n", c->fill[0] );
  } else if (c->filltype == 'c')  {
     printf("cfill %f %f %f\n", c->fill[0], c->fill[1], c->fill[2] );
  }
  if (first(c->general_marks) != c->general_marks) {
    spaces(nsp); printf("gmarks");
    for(p = first(c->general_marks); p != nil(c->general_marks); p = next(p))
      printf(" %f %f ", p->x, p->y);
  }
  printf("\n");
  spaces(nsp);
  if(!c->poly) printf("no"); printf("poly ");
  if (c->pfilltype == 'g') {
     printf("pfill %f", c->pfill[0] );
  } else if (c->pfilltype == 'c')  {
     printf("pcfill %f %f %f", c->pfill[0], c->pfill[1], c->pfill[2] );
  }
  printf("\n");
  
  spaces(nsp); printf("linetype ");
  if (c->linetype == '0')  printf("none ");
  else if (c->linetype == 's') printf("solid ");
  else if (c->linetype == '.') printf("dotted ");
  else if (c->linetype == '-') printf("dashed ");
  else if (c->linetype == 'l') printf("longdash ");
  else if (c->linetype == 'd') printf("dotdash ");
  else if (c->linetype == 'D') printf("dotdotdash ");
  else if (c->linetype == '2') printf("dotdotdashdash ");
  else if (c->linetype == 'g') {
    printf("general\n");
    spaces(nsp+2);
    printf("glines "); 
    i = 0;
    for (fl = first(c->gen_linetype); fl != nil(c->gen_linetype); 
                                      fl = next(fl)) {
      if (i == 6) {
        printf("\n");
        spaces(nsp + 9);
        i = 0;
      }
      printf("%f ", fl->f);
      i++;
    }
    printf("\n");
    spaces(nsp);
  }
  printf("linethickness %f\n", c->linethick);
  spaces(nsp); 
  if (c->graytype == 'g') {
    printf("gray %f ", c->gray[0]);
  } else if (c->graytype == 'c') {
    printf("color %f %f %f ", c->gray[0], c->gray[1], c->gray[2]);
  }
  if(!c->clip) printf("no"); printf("clip\n");
  spaces(nsp);
  for (i = 0; i < NPATTERNS && PTYPES[i] != c->pattern; i++) ;
  printf("pattern %s %f ", PATTERNS[i], c->parg);
  for (i = 0; i < NPATTERNS && PTYPES[i] != c->ppattern; i++) ;
  printf("ppattern %s %f ", PATTERNS[i], c->pparg);
  for (i = 0; i < NPATTERNS && PTYPES[i] != c->apattern; i++) ;
  printf("apattern %s %f\n", PATTERNS[i], c->aparg);
  spaces(nsp); 
  if(!c->rarrow) printf("no"); printf("rarrow ");
  if(!c->larrow) printf("no"); printf("larrow ");
  if(!c->rarrows) printf("no"); printf("rarrows ");
  if(!c->larrows) printf("no"); printf("larrows ");
  if (c->afilltype == 'g') {
     printf("afill %f\n", c->afill[0] );
  } else if (c->afilltype == 'c')  {
     printf("acfill %f %f %f\n", c->afill[0], c->afill[1], c->afill[2] );
  }
  spaces(nsp);
  if(!c->bezier) printf("no"); printf("bezier ");
  printf("asize %f %f\n", ptodist(c->asize[0], g->x_axis), 
                          ptodist(c->asize[1], g->y_axis) * 2.0);
}

/* ------------------------------------------------------------------------ */
static void show_axis(Axis a, int nsp, Graph g)
{
  Axis other;
  Hash h;
  String s;

  if (a->is_x) other = g->y_axis; else other = g->x_axis;
  spaces(nsp); printf("size %f\n", a->size);
  spaces(nsp); printf("min %f max %f %s\n", a->min, a->max,
                      (a->is_lg) ? "log" : "linear");
  if (!(a->draw_hash_labels || a->draw_axis_line ||
        a->draw_hash_marks || a->draw_axis_label)) {
    spaces(nsp);
    printf("nodraw\n");
    return;
  }
  spaces(nsp); printf("draw_at %f\n", ptoc(a->draw_at, other));
  if (a->label->label != CNULL) {
    spaces(nsp); printf("label\n");
    show_label(a->label, nsp+2, g);
  }
  spaces(nsp); 
  printf("%sdraw_hash_labels\n", (a->draw_hash_labels) ? "" : "no_");
  spaces(nsp); 
  printf("%sdraw_axis_line\n", (a->draw_axis_line) ? "" : "no_");
  spaces(nsp); 
  printf("%sdraw_hash_marks\n", (a->draw_hash_marks) ? "" : "no_");
  spaces(nsp); 
  printf("%sgrid_lines\n", (a->grid_lines) ? "" : "no_");
  spaces(nsp); 
  printf("%smgrid_lines\n", (a->mgrid_lines) ? "" : "no_");
  spaces(nsp); 
  printf("%sdraw_axis_label\n", (a->draw_axis_label) ? "" : "no_");
  spaces(nsp); 
  if (a->graytype == 'g') {
    printf("gray %f\n", a->gray[0]);
  } else if (a->graytype == 'c') {
    printf("color %f %f %f\n", a->gray[0], a->gray[1], a->gray[2]);
  }
  spaces(nsp); 
  if (a->gr_graytype == 'g') {
    printf("gr_gray %f ", a->gr_gray[0]);
  } else if (a->gr_graytype == 'c') {
    printf("color %f %f %f ", a->gr_gray[0], a->gr_gray[1], a->gr_gray[2]);
  }
  if (a->mgr_graytype == 'g') {
    printf("mgr_gray %f\n", a->mgr_gray[0]);
  } else if (a->mgr_graytype == 'c') {
    printf("color %f %f %f\n", a->mgr_gray[0], a->mgr_gray[1], a->mgr_gray[2]);
  }

  spaces(nsp);

  printf("(* The real settings for generating auto_hash_labels:\n");
  spaces(nsp+5);
  printf("%sauto_hash_marks ", (a->auto_hash_marks) ? "" : "no_");
  printf("%sauto_hash_labels\n", (a->auto_hash_labels) ? "" : "no_");
  spaces(nsp+5); printf("hash %f shash %f mhash %d hash_format %c\n", 
                       a->hash_interval, a->hash_start, a->minor_hashes,
                       a->hash_format);
  spaces(nsp+5);
  if (a->is_lg) {
    printf("log_base %f ", a->log_base);
  }
  printf("hash_scale %f ", a->hash_scale);
  printf("precision %d\n", a->precision);
  spaces(nsp+3);
  printf("The following are explicit and implicit hash marks and labels *)\n");
  
  spaces(nsp); 
  printf("hash 0 draw_hash_marks_at %f draw_hash_labels_at %f\n",
         ptoc(a->draw_hash_marks_at, other), 
         ptoc(a->draw_hash_labels_at, other));
  spaces(nsp); printf("hash_labels (* The :, x, and y values are ignored *)\n");
  show_label(a->hl, nsp + 2, g);
  
  for (h = first(a->hash_lines); h != nil(a->hash_lines); h = next(h)) {
    spaces(nsp);
    printf("%s %f\n", (!(h->size < HASH_SIZE || h->size > HASH_SIZE) ? "hash_at" : "mhash_at"),
           ptoc(h->loc, a));
  }
  for (s = first(a->hash_labels); s != nil(a->hash_labels); s = next(s)) {
    spaces(nsp);
    printf("hash_label at %f ", ptoc(s->s->x, a));
    show_string(s->s->label);
  }
}

/* ------------------------------------------------------------------------ */
static void show_legend(Legend l, int nsp, Graph g)
{
  if (l->type == 'c') {
    spaces(nsp); printf("custom\n");
  } else if (l->type == 'n') {
    spaces(nsp); printf("off\n");
  }

  spaces(nsp); printf("linelength %f linebreak %f midspace %f\n",
     ptodist(l->linelength, g->x_axis), ptodist(l->linebreak, g->y_axis),
     ptodist(l->midspace, g->x_axis));
  if (l->type == 'u') {
    spaces(nsp); printf("defaults");
    show_label(l->l, nsp+2, g);
  }
}

/* ------------------------------------------------------------------------ */
static void show_graph(Graph g, int nsp)
{

  Curve c;
  String s;    
  spaces(nsp); printf("x_translate %f y_translate %f\n", 
                       ptoin(g->x_translate), ptoin(g->y_translate));
  spaces(nsp); printf("xaxis\n"); show_axis(g->x_axis, nsp+2, g);
  spaces(nsp); printf("yaxis\n"); show_axis(g->y_axis, nsp+2, g);
  spaces(nsp); if(!g->clip) printf("no"); printf("clip ");
               if(!g->border) printf("no"); printf("border\n");
  for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
    spaces(nsp);
    printf("curve %d\n", c->num);
    show_curve(c, nsp+2, g);
  }
  spaces(nsp); printf("legend\n");
  show_legend(g->legend, nsp+2, g);
  if (g->title->label != CNULL) {
    spaces(nsp);
    printf("title\n");
    show_label(g->title, nsp+2, g);
  }
  for (s = first(g->strings); s != nil(g->strings); s = next(s)) {
    spaces(nsp);
    printf("string %d\n", s->num);
    show_label(s->s, nsp+2, g);
  }
}

/* ------------------------------------------------------------------------ */
void show_graphs(Graphs gs)
{
  Graphs the_g;
  Graph g;
  char started;
  int i;

  started = 0;
  for (the_g = first(gs); the_g != nil(gs); the_g = next(the_g)) {
    if (started) printf("\nnewpage\n");
    started = 1;
    printf("X %f Y %f\n", ptoin(the_g->width), ptoin(the_g->height));
    if (the_g->preamble != CNULL) {
      printf("preamble ");
      if (!the_g->prefile) printf(": ");
      show_mltiline(the_g->preamble);
    }
    if (the_g->epilogue != CNULL) {
      printf("epilogue ");
      if (!the_g->epifile) printf(": ");
      show_mltiline(the_g->epilogue);
    }
    printf("bbox"); 
    for (i = 0; i < 4; i++) printf(" %d", the_g->bb[i]);
    printf("\n");
    for (g = first(the_g->g); g != nil(the_g->g); g = next(g)) {
      printf("graph %d\n", g->num);
      show_graph(g, 2);
    }
  }
}

/* ------------------------------------------------------------------------ */
/* End of file show.c */
