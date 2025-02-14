/* edit.c
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/edit.c,v $
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
static void edit_label(Label l)
{
  char *txt, inp_str[80];
  double f;
  int i;

  while ( getstring(inp_str) ) {
    if (strcmp(inp_str, ":") == 0) {
      if ((txt = getlabel()) == CNULL) return;
      l->label = txt;
    } else if (strcmp(inp_str, "x") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->x = f;
    } else if (strcmp(inp_str, "y") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->y = f;
    } else if (strcmp(inp_str, "fontsize") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->fontsize = f;
    } else if (strcmp(inp_str, "linesep") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->linesep = f;
    } else if (strcmp(inp_str, "hjl") == 0) {
      l->hj = 'l';
    } else if (strcmp(inp_str, "hjc") == 0) {
      l->hj = 'c';
    } else if (strcmp(inp_str, "hjr") == 0) {
      l->hj = 'r';
    } else if (strcmp(inp_str, "vjc") == 0) {
      l->vj = 'c';
    } else if (strcmp(inp_str, "vjt") == 0) {
      l->vj = 't';
    } else if (strcmp(inp_str, "vjb") == 0) {
      l->vj = 'b';
    } else if (strcmp(inp_str, "font") == 0) {
      if (!getstring(inp_str)) return;
      txt = (char *) malloc (sizeof(char)*strlen(inp_str)+2);
      strcpy(txt, inp_str);
      l->font = txt;
    } else if (strcmp(inp_str, "rotate") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->rotate = f;
    } else if (strcmp(inp_str, "lgray") == 0) {
        if (!getdouble(&f)) rejecttoken(); else {
          l->graytype = 'g';
          l->gray[0] = f;
        }
    } else if (strcmp(inp_str, "lcolor") == 0) {
        l->graytype = 'c';
        for( i = 0 ; i < 3 ; i++ )  {
          if(!getdouble(&f)) {
            rejecttoken();
            l->graytype = 'n';
            break ;
          } else l->gray[i] = f ;
        }
    } else {
      rejecttoken();
      return;
    }
  }
}

/* ------------------------------------------------------------------------ */
/* Copies curve c2 to c1 */
static void copy_curve(Curve c1, Curve c2)
{
  Flist f, newf;
  Point p, newp;

  copy_label(c1->l, c2->l);
  copy_label(c1->lmark, c2->lmark);
  c1->l->label = CNULL;
  c1->clip = c2->clip;
  for (f = first(c2->gen_linetype); 
       f != nil(c2->gen_linetype); 
       f = next(f)) {
    newf = (Flist) get_node((List) c1->gen_linetype);
    newf->f = f->f;
    insert((List) newf, (List) c1->gen_linetype);
  }
  c1->pattern = c2->pattern;
  c1->apattern = c2->apattern;
  c1->ppattern = c2->ppattern;
  c1->parg = c2->parg;
  c1->aparg = c2->aparg;
  c1->pparg = c2->pparg;
  c1->marktype = c2->marktype;
  c1->linetype = c2->linetype;
  c1->linethick = c2->linethick;
  c1->marksize[0] = c2->marksize[0];
  c1->marksize[1] = c2->marksize[1];
  c1->mrotate = c2->mrotate;
  for (p = first(c2->general_marks); 
       p != nil(c2->general_marks); 
       p = next(p)) {
    newp = (Point) get_node((List) c1->general_marks);
    newp->x = p->x;
    newp->y = p->y;
    insert((List) newp, (List) c1->general_marks);
  }
  c1->graytype = c2->graytype;
  c1->gray[0] = c2->gray[0];
  c1->gray[1] = c2->gray[1];
  c1->gray[2] = c2->gray[2];
  c1->filltype = c2->filltype;
  c1->fill[0] = c2->fill[0];
  c1->fill[1] = c2->fill[1];
  c1->fill[2] = c2->fill[2];
  c1->poly = c2->poly;
  c1->pfilltype = c2->pfilltype;
  c1->pfill[0] = c2->pfill[0];
  c1->pfill[1] = c2->pfill[1];
  c1->pfill[2] = c2->pfill[2];
  c1->afilltype = c2->afilltype;
  c1->afill[0] = c2->afill[0];
  c1->afill[1] = c2->afill[1];
  c1->afill[2] = c2->afill[2];
  c1->postscript = c2->postscript;
  c1->postfile = c2->postfile;
  c1->eps = c2->eps;
  c1->rarrow = c2->rarrow;
  c1->larrow = c2->larrow;
  c1->rarrows = c2->rarrows;
  c1->larrows = c2->larrows;
  c1->asize[0] = c2->asize[0];
  c1->asize[1] = c2->asize[1];
  c1->bezier = c2->bezier;
}

/* ------------------------------------------------------------------------ */
/* Copies label l2 to l1 */
void copy_label(Label l1, Label l2)
{
  l1->label = l2->label;
  l1->x = l2->x;
  l1->y = l2->y;
  l1->rotate = l2->rotate;
  l1->font = l2->font;
  l1->fontsize = l2->fontsize;
  l1->hj = l2->hj;
  l1->vj = l2->vj;
  l1->graytype = l2->graytype;
  l1->gray[0] = l2->gray[0];
  l1->gray[1] = l2->gray[1];
  l1->gray[2] = l2->gray[2];
  l1->linesep = l2->linesep;
}

/* ------------------------------------------------------------------------ */
/* Copies axis a2 to a1 */
static void copy_axis(Axis a1, Axis a2)
{
  copy_label(a1->label, a2->label);
  copy_label(a1->hl, a2->hl);
  a1->max = a2->max;
  a1->min = a2->min;
  a1->pmax = a2->pmax;
  a1->pmin = a2->pmin;
  a1->size = a2->size;
  a1->hash_interval = a2->hash_interval;
  a1->hash_start = a2->hash_start;
  a1->log_base = a2->log_base;
  a1->draw_hash_marks_at = a2->draw_hash_marks_at;
  a1->draw_hash_labels_at = a2->draw_hash_labels_at;
  a1->draw_at = a2->draw_at;
  a1->draw_hash_labels = a2->draw_hash_labels;
  a1->draw_axis_line = a2->draw_axis_line;
  a1->draw_hash_marks = a2->draw_hash_marks;
  a1->draw_axis_label = a2->draw_axis_label;
  a1->auto_hash_labels = a2->auto_hash_labels;
  a1->auto_hash_marks = a2->auto_hash_marks;
  a1->minor_hashes = a2->minor_hashes;
  a1->hash_scale = a2->hash_scale;
  a1->hash_format = a2->hash_format;
  a1->graytype = a2->graytype;
  a1->gray[0] = a2->gray[0];
  a1->gray[1] = a2->gray[1];
  a1->gray[2] = a2->gray[2];
  a1->mgr_graytype = a2->mgr_graytype;
  a1->mgr_gray[0] = a2->mgr_gray[0];
  a1->mgr_gray[1] = a2->mgr_gray[1];
  a1->mgr_gray[2] = a2->mgr_gray[2];
  a1->gr_graytype = a2->gr_graytype;
  a1->gr_gray[0] = a2->gr_gray[0];
  a1->gr_gray[1] = a2->gr_gray[1];
  a1->gr_gray[2] = a2->gr_gray[2];
  a1->grid_lines = a2->grid_lines;
  a1->mgrid_lines = a2->mgrid_lines;
  a1->precision = a2->precision;
  a1->start_given = a2->start_given;
  a1->is_lg = a2->is_lg;
  a1->is_x = a2->is_x;
}

/* ------------------------------------------------------------------------ */
static Curve do_copy_curve(Graph g, Graphs gs, Graphs all_gs)
{
  Curve lastc, newc;
  Graph oldg;
  Graphs oldgs;
  int num;

  if (!getint(&num)) {
    rejecttoken();
    oldg = g;
    oldgs = gs;
    while(gs != nil(all_gs)) {
      if (gs != oldgs) g = last(gs->g);
      while(g != nil(gs->g)) {
        if (first(g->curves) == nil(g->curves)) g = prev(g);
        else {
          lastc = last(g->curves);
          if (first(oldg->curves) == nil(oldg->curves))
            newc = new_curve(oldg->curves, 0);
          else newc = new_curve(oldg->curves, last(oldg->curves)->num + 1);
          copy_curve(newc, lastc);
          return newc;
        }
      }
      gs = prev(gs);
    }
    error_header(); 
    fprintf(stderr, "Cannot perform copycurve on first curve\n");
    exit(1);
  } else {
    if (first(g->curves) == nil(g->curves))
      newc = new_curve(g->curves, 0);
    else newc = new_curve(g->curves, last(g->curves)->num + 1);
    lastc = g->curves; 
    while(1) {
      lastc = prev(lastc);
      if (lastc == nil(g->curves) || lastc->num < num) {
        error_header(); 
        fprintf(stderr, "copycurve: curve #%d not found\n", num);
        exit(1);
      }
      if (lastc->num == num) {
        copy_curve(newc, lastc);
        return newc;
      }
    }
  }
  return newc; /* To shut lint up */
}

/* ------------------------------------------------------------------------ */
static Label do_copy_string(Graph g, Graphs gs, Graphs all_gs)
{
  String lastl, newl;
  Graph oldg;
  Graphs oldgs;
  int num;

  if (!getint(&num)) {
    rejecttoken();
    oldgs = gs;
    oldg = g;
    while(gs != nil(all_gs)) {
      if (gs != oldgs) g = last(gs->g);
      while(g != nil(gs->g)) {
        if (first(g->strings) == nil(g->strings)) g = prev(g);
        else {
          lastl = last(g->strings);
          if (first(oldg->strings) == nil(oldg->strings))
            newl = new_string(oldg->strings, 0);
          else newl = new_string(oldg->strings, last(oldg->strings)->num + 1);
          copy_label(newl->s, lastl->s);
          return newl->s;
        }
      }
      gs = prev(gs);
    }
    error_header(); 
    fprintf(stderr, "Cannot perform copystring on first string\n");
    exit(1);
    return newl->s; /* To shut lint up */
  } else {
    if (first(g->strings) == nil(g->strings))
      newl = new_string(g->strings, 0);
    else newl = new_string(g->strings, last(g->strings)->num + 1);
    lastl = g->strings; 
    while(1) {
      lastl = prev(lastl);
      if (lastl == nil(g->strings) || lastl->num < num) {
        error_header(); 
        fprintf(stderr, "copystring: string #%d not found\n", num);
        exit(1);
      }
      if (lastl->num == num) {
        copy_label(newl->s, lastl->s);
        return newl->s;
      }
    }
  }
}

/* ------------------------------------------------------------------------ */
static Graph last_graph(Graph g, Graphs gs, Graphs all_gs)
{
  Graph lastg;

  lastg = prev(g);
  while(lastg == nil(gs->g)) {
    if (prev(gs) == nil(all_gs)) {
      error_header(); 
      fprintf(stderr, "First graph cannot inherit axes\n");
      exit(1);
    } else {
      gs = prev(gs);
      lastg = last(gs->g);
    }
  }
  return lastg;
}

/* ------------------------------------------------------------------------ */
static void copy_legend(Legend l1, Legend l2)
{
  l1->linelength = l2->linelength;
  l1->linebreak = l2->linebreak;
  l1->midspace = l2->midspace;
  l1->type = l2->type;
  copy_label(l1->l, l2->l);
}

/* ------------------------------------------------------------------------ */
static void inherit_axes(Graph g, Graph lastg)
{
  char *s;
  copy_axis(g->x_axis, lastg->x_axis);
  copy_axis(g->y_axis, lastg->y_axis);
  g->x_translate = lastg->x_translate;
  g->y_translate = lastg->y_translate;
  g->clip = lastg->clip;
  g->border = lastg->border;
  copy_legend(g->legend, lastg->legend);
  s = g->title->label;
  copy_label(g->title, lastg->title);
  g->title->label = s;
}

/* ------------------------------------------------------------------------ */
static void getpattern(char *inp_str, const char *key, char *p, double *a)
{
  int i;
  double f;

  if (!getstring(inp_str)) return;
  for (i = 0; i < NPATTERNS; i++) {
    if (strcmp(inp_str, PATTERNS[i]) == 0) {
      *p = PTYPES[i];
      if (getdouble(&f)) {
        *a = f;
      } else {
        rejecttoken();
      }
      i = NPATTERNS + 1;
    } 
  }
  if (i == NPATTERNS) {
    error_header(); fprintf(stderr, "Bad %s: %s\n", key, inp_str);
    error_header(); fprintf(stderr, "             Valid %ss are:", key);
    for (i = 0; i < NPATTERNS; i++) fprintf(stderr, " %s", PATTERNS[i]);
    fprintf(stderr, "\n");
    exit(1);
  }
  return;
}

/* ------------------------------------------------------------------------ */
static void edit_curve(Curve c, Graph g)
{
  char inp_str[256], *txt;
  double x, y, f, e1, e2;
  double xh, yh, xl, yl;
  Point p, p1, p2;
  Flist fl;
  FILE *fi;
  int i;
  char e;

  while ( getstring(inp_str) ) {
    if (strcmp(inp_str, "y_epts") == 0 ||
        strcmp(inp_str, "pts") == 0    ||
        strcmp(inp_str, "x_epts") == 0) {
      e = inp_str[0];
      while (getdouble(&x)) {
        if (e == 'p') {
          if (!getdouble(&y)) {
            error_header(); 
            fprintf(stderr, "Reading Points, no y value for x=%f\n", x);
            exit(1);
          }
        } else {
          if (!getdouble(&y) || !getdouble(&e1) || !getdouble(&e2)) {
            error_header();
              fprintf(stderr, 
                      "Reading %s, need 4 values per data point\n", inp_str);
              exit(1);
          }
        }
        p = (Point) get_node((List) c->pts);
        p->x = x;
        p->y = y;
        p->e = e;
	insert((List) p, (List) c->pts);
        c->npts++;
        if (e == 'x') {
          p1 = (Point) get_node((List) c->xepts);
          p1->x = e1;
          p1->y = y;
          p2 = (Point) get_node((List) c->xepts);
          p2->x = e2;
          p2->y = y;
	  insert((List) p1, (List) c->xepts);
	  insert((List) p2, (List) c->xepts);
          xh = MAX(e1, e2); xh = MAX(xh, x);
          xl = MIN(e1, e2); xl = MIN(xl, x);
          yh = y; yl = y;
        } else if (e == 'y') {
          p1 = (Point) get_node((List) c->yepts);
          p1->y = e1;
          p1->x = x;
          p2 = (Point) get_node((List) c->yepts);
          p2->y = e2;
          p2->x = x;
	  insert((List) p1, (List) c->yepts);
	  insert((List) p2, (List) c->yepts);
          yh = MAX(e1, e2); yh = MAX(yh, y);
          yl = MIN(e1, e2); yl = MIN(yl, y);
          xh = x; xl = x;
        } else {
          xh = x; xl = x; yh = y; yl = y;
        }
        if (!(g->x_axis->pmax < FSIG || g->x_axis->pmax > FSIG)) {
          g->x_axis->pmax = xh;
          g->x_axis->pmin = xl;
          g->y_axis->pmax = yh;
          g->y_axis->pmin = yl;
        } else {
          g->x_axis->pmax = MAX(g->x_axis->pmax, xh);
          g->x_axis->pmin = MIN(g->x_axis->pmin, xl);
          g->y_axis->pmax = MAX(g->y_axis->pmax, yh);
          g->y_axis->pmin = MIN(g->y_axis->pmin, yl);
        }
      }
      rejecttoken();

    } else if (strcmp(inp_str, "label") == 0) {
      edit_label(c->l);
    } else if (strcmp(inp_str, "marksize") == 0) {
      if (!getdouble(&f)) rejecttoken(); 
      else {
        c->marksize[0] = f;
        if (!getdouble(&f)) rejecttoken(); 
        else c->marksize[1] = f;
      }
    } else if (strcmp(inp_str, "gmarks") == 0) {
      while (getdouble(&x)) {
        if (!getdouble(&y)) {
          error_header(); 
          fprintf(stderr, "Reading GMarks, no y value for x=%f\n", x);
          exit(1);
        }
        p =  (Point) get_node((List) c->general_marks);
        p->x = x;
        p->y = y;
        insert((List) p, (List) c->general_marks);
      }
      rejecttoken();
    } else if (strcmp(inp_str, "pfill") == 0) {
        if (!getdouble(&f)) rejecttoken(); else {
          /* grey fill */
          c->pfilltype = 'g';
          c->pfill[0] = f;
        }
    } else if (strcmp(inp_str, "pcfill") == 0) {
         /* color fill */
        c->pfilltype = 'c';
        for( i = 0 ; i < 3 ; i++ )  {
          if(!getdouble(&f)) {
            rejecttoken();
            c->pfilltype = 'n';
            break ;
          } else c->pfill[i] = f ;
        }
    } else if (strcmp(inp_str, "fill") == 0) {
        if (!getdouble(&f)) rejecttoken(); else {
          /* grey fill */
          c->filltype = 'g';
          c->fill[0] = f;
        }
    } else if (strcmp(inp_str, "cfill") == 0) {
         /* color fill */
        c->filltype = 'c';
        for( i = 0 ; i < 3 ; i++ )  {
          if(!getdouble(&f)) {
            rejecttoken();
            c->filltype = 'n';
            break ;
          } else c->fill[i] = f ;
        }
    } else if (strcmp(inp_str, "afill") == 0) {
	if (!getdouble(&f)) rejecttoken(); else {
	  c->afilltype = 'g';
          c->afill[0] = f;
	}
    } else if (strcmp(inp_str, "acfill") == 0) {
	c->afilltype = 'c';
	for( i = 0 ; i < 3 ; i++ )  {
	  if(!getdouble(&f)) { 
	    rejecttoken(); 
 	    c->afilltype = 'n';
            break ;
	  } else c->afill[i] = f ;
	}
    } else if (strcmp(inp_str, "marktype") == 0) {
      if (!getstring(inp_str)) return;
      for (i = 0; i < NMARKTYPES && strcmp(inp_str, MARKTYPESTRS[i]) != 0; i++) ;
      if (i == NMARKTYPES) {
        error_header(); fprintf(stderr, "Bad mark: %s\n", inp_str);
        fprintf(stderr, "             Valid marks are:");
        for (i = 0; i < NMARKTYPES; i++) {
          fprintf(stderr, " %s", MARKTYPESTRS[i]);
        }
        fprintf(stderr, "\n");
        exit(1);
      } else {
        c->marktype = MARKTYPES[i];
        if (c->marktype == 'l') edit_label(c->lmark);
      }
    } else if (strcmp(inp_str, "glines") == 0) {
      while (getdouble(&f)) {
        fl = (Flist) get_node((List) c->gen_linetype);
        fl->f = f;
        insert((List) fl, (List) c->gen_linetype);
      } 
      rejecttoken();
    } else if (strcmp(inp_str, "pattern") == 0) {
      getpattern(inp_str, "pattern", &(c->pattern), &(c->parg));
    } else if (strcmp(inp_str, "apattern") == 0) {
      getpattern(inp_str, "apattern", &(c->apattern), &(c->aparg));
    } else if (strcmp(inp_str, "ppattern") == 0) {
      getpattern(inp_str, "ppattern", &(c->ppattern), &(c->pparg));

    } else if (strcmp(inp_str, "linetype") == 0) {
      if (!getstring(inp_str)) return;
      if (strcmp(inp_str, "none") == 0) c->linetype = '0';
      else if (strcmp(inp_str, "solid") == 0) c->linetype = 's';
      else if (strcmp(inp_str, "dotted") == 0) c->linetype = '.';
      else if (strcmp(inp_str, "dashed") == 0) c->linetype = '-';
      else if (strcmp(inp_str, "longdash") == 0) c->linetype = 'l';
      else if (strcmp(inp_str, "dotdash") == 0) c->linetype = 'd';
      else if (strcmp(inp_str, "dotdotdash") == 0) c->linetype = 'D';
      else if (strcmp(inp_str, "dotdotdashdash") == 0) c->linetype = '2';
      else if (strcmp(inp_str, "general") == 0) c->linetype = 'g';
      else {
        error_header(); fprintf(stderr, "Bad line type: %s\n", inp_str);
        error_header(); fprintf(stderr, "             Valid marks are %s\n", 
          "solid, dotted, dashed, longdash, dotdash,"); 
        error_header(); fprintf(stderr, "             %s.\n", 
          "dotdotdash, dotdotdashdash, none");
        exit(1);
      }
    } else if (strcmp(inp_str, "linethickness") == 0) {
      if (!getdouble(&f)) rejecttoken(); else c->linethick = f;
    } else if (strcmp(inp_str, "gray") == 0) {
        if (!getdouble(&f)) rejecttoken(); else {
          c->graytype = 'g';
          c->gray[0] = f;
        }
    } else if (strcmp(inp_str, "color") == 0) {
        c->graytype = 'c';
        for( i = 0 ; i < 3 ; i++ )  {
          if(!getdouble(&f)) {
            rejecttoken();
            c->graytype = 'n';
            break ;
          } else c->gray[i] = f ;
        }
    } else if (strcmp(inp_str, "mrotate") == 0) {
      if (!getdouble(&f)) rejecttoken(); else {
        c->mrotate = f;
      }
    } else if (strcmp(inp_str, "eps") == 0) {
      if (!getstring(inp_str)) {
        error_header();
        fprintf(stderr, "eps token must be followed by an %s\n",
                "encapsulated postscript file\n");
        exit(1);
      }
      c->marktype = 'E';
      c->eps = (char *) malloc ((strlen(inp_str)+1)*sizeof(char));
      strcpy(c->eps, inp_str);
      fi = fopen(c->eps, "r");
      if (fi == NULL) {
        error_header();
        fprintf(stderr, "couldn't open eps file '%s'\n", c->eps);
        exit(1);
      }
      fclose(fi);
    } else if (strcmp(inp_str, "postscript") == 0) {
      if (!getstring(inp_str)) return;
      c->marktype = 'p';
      if (strcmp(inp_str, ":") == 0) {
        c->postfile = 0;
        if ((txt = getmultiline()) == CNULL) return;
        c->postscript = txt;
      } else {
        c->postfile = 1;
        c->postscript = (char *) malloc ((strlen(inp_str)+1)*sizeof(char));
        strcpy(c->postscript, inp_str);
        fi = fopen(c->postscript, "r");
        if (fi == NULL) {
          error_header();
          fprintf(stderr, "couldn't open postscript file '%s'\n", 
                  c->postscript);
          exit(1);
        }
        fclose(fi);
      }
    } else if (strcmp(inp_str, "poly") == 0) {
      c->poly = 1;
    } else if (strcmp(inp_str, "nopoly") == 0) {
      c->poly = 0;
    } else if (strcmp(inp_str, "larrow") == 0) {
      c->larrow = 1;
    } else if (strcmp(inp_str, "nolarrow") == 0) {
      c->larrow = 0;
    } else if (strcmp(inp_str, "rarrow") == 0) {
      c->rarrow = 1;
    } else if (strcmp(inp_str, "norarrow") == 0) {
      c->rarrow = 0;
    } else if (strcmp(inp_str, "larrows") == 0) {
      c->larrows = 1;
    } else if (strcmp(inp_str, "nolarrows") == 0) {
      c->larrows = 0;
    } else if (strcmp(inp_str, "rarrows") == 0) {
      c->rarrows = 1;
    } else if (strcmp(inp_str, "norarrows") == 0) {
      c->rarrows = 0;
    } else if (strcmp(inp_str, "bezier") == 0) {
      c->bezier = 1;
    } else if (strcmp(inp_str, "nobezier") == 0) {
      c->bezier = 0;
    } else if (strcmp(inp_str, "asize") == 0) {
      if (!getdouble(&f)) rejecttoken(); 
      else {
        c->asize[0] = f;
        if (!getdouble(&f)) rejecttoken(); 
        else c->asize[1] = f;
      }
    } else if (strcmp(inp_str, "clip") == 0) {
      c->clip = 1;
    } else if (strcmp(inp_str, "noclip") == 0) {
      c->clip = 0;
    } else {
      rejecttoken();
      return;
    }
  }
}

/* ------------------------------------------------------------------------ */
static void edit_hash_label(Axis a)
{
  double at, f;
  char *s;
  char inp_str[256];
  String st;
  int done;

  s = CNULL;

  at = (first(a->hash_lines) == nil(a->hash_lines)) ? FSIG
          : first(a->hash_lines)->loc;
  while(1) {
    done = 0;
    if (getstring(inp_str)) {
      if (strcmp(inp_str, ":") == 0) {
        if ((s = getlabel()) == CNULL) return;
      } else if (strcmp(inp_str, "at") == 0) {
        if (getdouble(&f)) {
          at = f;  
        } else {
          rejecttoken();
          done = 1;
        }
      } else {
        rejecttoken();
        done = 1;
      }
    } else {
      done = 1;
    }
    if (done) {
      if (s == CNULL) return;
      if (!(at < FSIG || at > FSIG)) {
        error_header();
        fprintf(stderr, 
          "hash_label either needs \"at\" or an associated \"hash_at\"\n");
        exit(1);
      }
      st = (String) get_node((List) a->hash_labels);
      st->s = new_label();
      st->s->label = s;
      st->s->x = at;
      st->s->y = at;
      insert((List) st, (List) a->hash_labels);
      return;
    }
  }
}

/* ------------------------------------------------------------------------ */
static void edit_axis(Axis a)
{
  char inp_str[256];
  double f;
  int i;
  Hash h;

  while ( getstring(inp_str) ) {

    if (strcmp(inp_str, "size") == 0) {
      if ( getdouble(&f)) a->size = f; else rejecttoken();
    } else if (strcmp(inp_str, "max") == 0) {
      if ( getdouble(&f)) a->max = f; else rejecttoken();
    } else if (strcmp(inp_str, "min") == 0) {
      if ( getdouble(&f)) a->min = f; else rejecttoken();
    } else if (strcmp(inp_str, "hash") == 0) {
      if ( getdouble(&f)) a->hash_interval = f; else rejecttoken();
    } else if (strcmp(inp_str, "shash") == 0) {
      if ( getdouble(&f)) {
        a->hash_start = f;
        a->start_given = 1;
        } 
      else rejecttoken();
    } else if (strcmp(inp_str, "mhash") == 0) {
      if (getint(&i)) a->minor_hashes = i; else rejecttoken();
    } else if (strcmp(inp_str, "precision") == 0) {
      if (getint(&i)) a->precision = i; else rejecttoken();
    } else if (strcmp(inp_str, "label") == 0) {
      edit_label(a->label);
    } else if (strcmp(inp_str, "hash_format") == 0) {
      if (!getstring(inp_str)) return;
      if (strcmp(inp_str, "g") == 0) {
        a->hash_format = 'g';
      } else if (strcmp(inp_str, "G") == 0) {
        a->hash_format = 'G';
      } else if (strcmp(inp_str, "E") == 0) {
        a->hash_format = 'E';
      } else if (strcmp(inp_str, "e") == 0) {
        a->hash_format = 'e';
      } else if (strcmp(inp_str, "f") == 0) {
        a->hash_format = 'f';
      } else {
        error_header();
        fprintf(stderr, "Invalid hash_style %s.  Must be f, g, G, e or E\n",
                inp_str);
        exit(1);
      }
    } else if (strcmp(inp_str, "hash_labels") == 0) {
      edit_label(a->hl);
    } else if (strcmp(inp_str, "log_base") == 0) {
      if (getdouble(&f)) {
        if (f <= 1.0) {
          error_header();
          fprintf(stderr, "\"log_base %f\": log_base must be > 1.0\n", f);
          exit(1);
        } else a->log_base = f; 
      } else rejecttoken();
    } else if (strcmp(inp_str, "draw_at") == 0) {
      if ( getdouble(&f)) a->draw_at = f; else rejecttoken();
    } else if (strcmp(inp_str, "log") == 0) {
      a->is_lg = 1;
    } else if (strcmp(inp_str, "linear") == 0) {
      a->is_lg = 0;
    } else if (strcmp(inp_str, "nodraw") == 0) {
      a->draw_hash_labels = 0;
      a->draw_axis_line = 0;
      a->draw_hash_marks = 0;
      a->draw_axis_label = 0;
    } else if (strcmp(inp_str, "draw") == 0) {
      a->draw_hash_labels = 1;
      a->draw_axis_line = 1;
      a->draw_hash_marks = 1;
      a->draw_axis_label = 1;
    } else if (strcmp(inp_str, "hash_at") == 0 ||
               strcmp(inp_str, "mhash_at") == 0) {
      if (getdouble(&f)) {
        h = (Hash) get_node((List) a->hash_lines);
        h->loc = f;
        h->major = (inp_str[0] == 'h');
        h->size = h->major ? HASH_SIZE : MHASH_SIZE;
        insert((List) h, (List) a->hash_lines);
      } else rejecttoken();
    } else if (strcmp(inp_str, "hash_label") == 0) {
      edit_hash_label(a);       
    } else if (strcmp(inp_str, "hash_scale") == 0) {
      if ( getdouble(&f)) a->hash_scale = f; else rejecttoken();
    } else if (strcmp(inp_str, "auto_hash_marks") == 0) {
      a->auto_hash_marks = 1;
    } else if (strcmp(inp_str, "no_auto_hash_marks") == 0) {
      a->auto_hash_marks = 0;
    } else if (strcmp(inp_str, "auto_hash_labels") == 0) {
      a->auto_hash_labels = 1;
    } else if (strcmp(inp_str, "no_auto_hash_labels") == 0) {
      a->auto_hash_labels = 0;
    } else if (strcmp(inp_str, "draw_hash_labels_at") == 0) {
      if (getdouble(&f)) a->draw_hash_labels_at = f; else rejecttoken();
    } else if (strcmp(inp_str, "draw_hash_marks_at") == 0) {
      if (getdouble(&f)) a->draw_hash_marks_at = f; else rejecttoken();
    } else if (strcmp(inp_str, "no_draw_hash_labels") == 0) {
      a->draw_hash_labels = 0;
    } else if (strcmp(inp_str, "draw_hash_labels") == 0) {
      a->draw_hash_labels = 1;
    } else if (strcmp(inp_str, "no_draw_axis_line") == 0) {
      a->draw_axis_line = 0;
    } else if (strcmp(inp_str, "draw_axis_line") == 0) {
      a->draw_axis_line = 1;
    } else if (strcmp(inp_str, "no_draw_axis") == 0) {
      a->draw_axis_line = 0;
    } else if (strcmp(inp_str, "draw_axis") == 0) {
      a->draw_axis_line = 1;
    } else if (strcmp(inp_str, "no_draw_hash_marks") == 0) {
      a->draw_hash_marks = 0;
    } else if (strcmp(inp_str, "draw_hash_marks") == 0) {
      a->draw_hash_marks = 1;
    } else if (strcmp(inp_str, "no_draw_axis_label") == 0) {
      a->draw_axis_label = 0;
    } else if (strcmp(inp_str, "draw_axis_label") == 0) {
      a->draw_axis_label = 1;
    } else if (strcmp(inp_str, "no_grid_lines") == 0) {
      a->grid_lines = 0;
    } else if (strcmp(inp_str, "grid_lines") == 0) {
      a->grid_lines = 1;
    } else if (strcmp(inp_str, "no_mgrid_lines") == 0) {
      a->mgrid_lines = 0;
    } else if (strcmp(inp_str, "mgrid_lines") == 0) {
      a->mgrid_lines = 1;
    } else if (strcmp(inp_str, "gray") == 0) {
      if (!getdouble(&f)) rejecttoken(); else {
        a->graytype = 'g';
        a->gray[0] = f;
      }
    } else if (strcmp(inp_str, "color") == 0) {
      a->graytype = 'c';
      for( i = 0 ; i < 3 ; i++ )  {
        if(!getdouble(&f)) {
          rejecttoken();
          a->graytype = 'n';
          break ;
        } else a->gray[i] = f ;
      }
    } else if (strcmp(inp_str, "grid_gray") == 0) {
      if (!getdouble(&f)) rejecttoken(); else {
        a->gr_graytype = 'g';
        a->gr_gray[0] = f;
      }
    } else if (strcmp(inp_str, "grid_color") == 0) {
      a->gr_graytype = 'c';
      for( i = 0 ; i < 3 ; i++ )  {
        if(!getdouble(&f)) {
          rejecttoken();
          a->gr_graytype = 'n';
          break ;
        } else a->gr_gray[i] = f ;
      }
    } else if (strcmp(inp_str, "mgrid_gray") == 0) {
      if (!getdouble(&f)) rejecttoken(); else {
        a->mgr_graytype = 'g';
        a->mgr_gray[0] = f;
      }
    } else if (strcmp(inp_str, "mgrid_color") == 0) {
      a->mgr_graytype = 'c';
      for( i = 0 ; i < 3 ; i++ )  {
        if(!getdouble(&f)) {
          rejecttoken();
          a->mgr_graytype = 'n';
          break ;
        } else a->mgr_gray[i] = f ;
      }
    } else {
      rejecttoken(); 
      return;
    }
  }
}

/* ------------------------------------------------------------------------ */
static void edit_legend(Legend l)
{
  char inp_str[256];
  double f;

  while ( getstring(inp_str) ) {
    if (strcmp(inp_str, "x") == 0) {
      if (!getdouble(&f)) rejecttoken(); 
      else {
        l->l->x = f; 
        l->l->hj = 'l';
        l->l->vj = 't';
        l->type = 'u';
      }
    } else if (strcmp(inp_str, "y") == 0) {
      if (!getdouble(&f)) rejecttoken(); 
      else {
        l->l->y = f; 
        l->l->hj = 'l';
        l->l->vj = 't';
        l->type = 'u';
      }
    } else if (strcmp(inp_str, "right") == 0 ||
               strcmp(inp_str, "on") == 0) {
      l->type = 'u';
      l->l->y = FSIG; 
      l->l->x = FSIG; 
      l->l->hj = 'l';
      l->l->vj = 'c';
    } else if (strcmp(inp_str, "left") == 0) {
      l->type = 'u';
      l->l->y = FSIG; 
      l->l->x = FSIG; 
      l->l->hj = 'r';
      l->l->vj = 'c';
    } else if (strcmp(inp_str, "off") == 0) {
      l->type = 'n';
    } else if (strcmp(inp_str, "top") == 0) {
      l->type = 'u';
      l->l->y = FSIG; 
      l->l->x = FSIG; 
      l->l->hj = 'l';
      l->l->vj = 'b';
    } else if (strcmp(inp_str, "bottom") == 0) {
      l->type = 'u';
      l->l->y = FSIG; 
      l->l->x = FSIG; 
      l->l->hj = 'l';
      l->l->vj = 't';
    } else if (strcmp(inp_str, "custom") == 0) {
      l->type = 'c';
    } else if (strcmp(inp_str, "linelength") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->linelength = f;
    } else if (strcmp(inp_str, "linebreak") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->linebreak = f;
    } else if (strcmp(inp_str, "midspace") == 0) {
      if (!getdouble(&f)) rejecttoken(); else l->midspace = f;
    } else if (strcmp(inp_str, "defaults") == 0) {
      edit_label(l->l);
    } else {
      rejecttoken();
      return;
    }
  }
}

/* ------------------------------------------------------------------------ */
static void edit_graph(Graph g, Graphs gs, Graphs all_gs)
{
  char inp_str[80];
  int num;
  String s;
  double f;

  while ( getstring(inp_str) ) {
    if (strcmp(inp_str, "xaxis") == 0)
      edit_axis(g->x_axis);
    else if (strcmp(inp_str, "yaxis") == 0)
      edit_axis(g->y_axis);
    else if (strcmp(inp_str, "curve") == 0) {
      if (!getint(&num)) {
        error_header(); fprintf(stderr, "\"curve\" not followed by number\n");
        exit(1);
      }
      edit_curve(get_curve(g->curves, num), g);
    } else if (strcmp(inp_str, "newcurve") == 0) {
      if (first(g->curves) == nil(g->curves))
        edit_curve(new_curve(g->curves, 0), g);
      else edit_curve(new_curve(g->curves, last(g->curves)->num + 1), g);
    } else if (strcmp(inp_str, "copycurve") == 0) {
      edit_curve(do_copy_curve(g, gs, all_gs), g);
    } else if (strcmp(inp_str, "newline") == 0) {
      if (first(g->curves) == nil(g->curves))
        edit_curve(new_line(g->curves, 0), g);
      else edit_curve(new_line(g->curves, last(g->curves)->num + 1), g);
    } else if (strcmp(inp_str, "title") == 0) {
      edit_label(g->title);
    } else if (strcmp(inp_str, "legend") == 0) {
      edit_legend(g->legend);
    } else if (strcmp(inp_str, "x_translate") == 0) {
      if (!getdouble(&f)) rejecttoken(); else g->x_translate = f;
    } else if (strcmp(inp_str, "y_translate") == 0) {
      if (!getdouble(&f)) rejecttoken(); else g->y_translate = f;
    } else if (strcmp(inp_str, "string") == 0) {
      if (!getint(&num)) {
        error_header(); fprintf(stderr, "\"string\" not followed by number\n");
        exit(1);
      }
      s = get_string(g->strings, num);
      edit_label(s->s);
    } else if (strcmp(inp_str, "newstring") == 0) {
      if (first(g->strings) == nil(g->strings))
        s = new_string(g->strings, 0);
      else s = new_string(g->strings, last(g->strings)->num + 1);
      edit_label(s->s);
    } else if (strcmp(inp_str, "copystring") == 0 ||
               strcmp(inp_str, "copyline") == 0) {
      edit_label(do_copy_string(g, gs, all_gs));
    } else if (strcmp(inp_str, "inherit_axes") == 0) {
      inherit_axes(g, last_graph(g, gs, all_gs));
    } else if (strcmp(inp_str, "Y") == 0) {
      if (!getdouble(&f)) rejecttoken(); else gs->height = f;
    } else if (strcmp(inp_str, "X") == 0) {
      if (!getdouble(&f)) rejecttoken(); else gs->width = f;
    } else if (strcmp(inp_str, "border") == 0) {
      g->border = 1;
    } else if (strcmp(inp_str, "noborder") == 0) {
      g->border = 0;
    } else if (strcmp(inp_str, "clip") == 0) {
      g->clip = 1;
    } else if (strcmp(inp_str, "noclip") == 0) {
      g->clip = 0;
    } else {
      rejecttoken();
      return;
    } 
  }
}

/* ------------------------------------------------------------------------ */
void edit_graphs(Graphs gs)
{
  Graphs the_g;
  Graph g, tmp_g;
  char inp_str[80];
  double f;
  int num, i, ok, j;

  the_g = first(gs);
  while ( getstring(inp_str) ) {
    if (strcmp(inp_str, "graph") == 0) {
      if (!getint(&num)) {
        error_header(); fprintf(stderr, "\"graph\" not followed by number\n");
        exit(1);
      }
      edit_graph(get_graph(the_g->g, num), the_g, gs);
    } else if (strcmp(inp_str, "newgraph") == 0) {
      if (first(the_g->g) == nil(the_g->g))
        edit_graph(new_graph(the_g->g, 0), the_g, gs);
      else edit_graph(new_graph(the_g->g, last(the_g->g)->num + 1), the_g, gs);
    } else if (strcmp(inp_str, "copygraph") == 0) {
      if (first(the_g->g) == nil(the_g->g))
        g = new_graph(the_g->g, 0);
      else g = new_graph(the_g->g, last(the_g->g)->num + 1);
      if (!getint(&num)) {
        rejecttoken();
        inherit_axes(g, last_graph(g, the_g, gs));
      } else {
        ok = 0;
        tmp_g = the_g->g;
        while(!ok) {
          tmp_g = prev(tmp_g);
          if (tmp_g == nil(the_g->g) || tmp_g->num < num) {
            error_header();
            fprintf(stderr, "copygraph: no graph #%d\n", num);
            exit(1);
          }
          ok = (tmp_g->num == num);
        }
        inherit_axes(g, tmp_g);
      }
      edit_graph(g, the_g, gs);
    } else if (strcmp(inp_str, "Y") == 0) {
      if (!getdouble(&f)) rejecttoken(); else the_g->height = f;
    } else if (strcmp(inp_str, "X") == 0) {
      if (!getdouble(&f)) rejecttoken(); else the_g->width = f;
    } else if (strcmp(inp_str, "newpage") == 0) {
      new_graphs(gs);
      the_g = last(gs);
    } else if (strcmp(inp_str, "bbox") == 0) {
      for (i = 0; i < 4; i++) {
        if (!getint(&j)) {
          error_header();
          fprintf(stderr, "Bbox definition must have four integers\n");
          exit(1);
        } else {
          the_g->bb[i] = j;
        }
      }
    } else if (strcmp(inp_str, "preamble") == 0) {
      if (!getstring(inp_str)) return;
      if (strcmp(inp_str, ":") != 0) {
        the_g->prefile = 1;
        the_g->preamble = (char *) malloc (sizeof(char)*(strlen(inp_str)+1));
        strcpy(the_g->preamble, inp_str);
      } else {
        the_g->prefile = 0;
        the_g->preamble = getmultiline();
        if (the_g->preamble == CNULL) return;
      }
    } else if (strcmp(inp_str, "epilogue") == 0) {
      if (!getstring(inp_str)) return;
      if (strcmp(inp_str, ":") != 0) {
        the_g->epifile = 1;
        the_g->epilogue = (char *) malloc (sizeof(char)*(strlen(inp_str)+1));
        strcpy(the_g->epilogue, inp_str);
      } else {
        the_g->epifile = 0;
        the_g->epilogue = getmultiline();
        if (the_g->epilogue == CNULL) return;
      }
    } else {
      error_header(); fprintf(stderr, "Bad token: %s\n", inp_str);
      exit(1);
    } 
  }
}

/* ------------------------------------------------------------------------ */
/* End of file edit.c */
