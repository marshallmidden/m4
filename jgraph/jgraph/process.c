/* process.c
 * James S. Plank

Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/process.c,v $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "jgraph.h"

#define ABS(a) ((a > 0.0) ? (a) : (-a))
#define AXIS_CHAR(a) ((a->is_x) ? 'x' : 'y')
#define HASH_DIR(a) ((a->hash_scale > 0.0) ? 1 : -1)

static double Pi;

/* ------------------------------------------------------------------------------ */
void process_title(Graph g)
{

  double ytitleloc;

  if (!(g->title->x < FSIG || g->title->x >FSIG))
  {
      g->title->x = g->x_axis->psize / 2.0;
  }
  else
  {
      g->title->x = ctop(g->title->x, g->x_axis);
  }
  if (g->title->y < FSIG || g->title->y > FSIG)
  {
      g->title->y = ctop(g->title->y, g->y_axis);
  }
  else
  {
    ytitleloc = 0.0;
    if (g->x_axis->draw_axis_label && g->x_axis->label->label != CNULL)
      ytitleloc = MIN(ytitleloc, g->x_axis->label->ymin);
    if (g->x_axis->draw_hash_labels)
      ytitleloc = MIN(ytitleloc, g->x_axis->hl->ymin);
    if (g->x_axis->draw_hash_marks)
      ytitleloc = MIN(ytitleloc, g->x_axis->draw_hash_marks_at - HASH_SIZE);
    if (g->legend->type == 'u')
      ytitleloc = MIN(ytitleloc, g->legend->l->ymin);

    g->title->y = ytitleloc - 10.0;
  }
  process_label(g->title, g, 0);
}

/* ------------------------------------------------------------------------------ */
void process_legend(Graph g)
{
  Legend l;
  int anything;
  double height, hdist, y, x, width, maxmark, maxmarky;
  Curve c;
  char *s;

  l = g->legend;
  if (l->type == 'n') return;
  if (!(l->l->linesep < FSIG || l->l->linesep > FSIG))
  {
      l->l->linesep = l->l->fontsize;
  }
  l->anylines = 0;
  maxmark = 0.0;
  maxmarky = 0.0;
  anything = 0;
  for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
    if (c->l->label != CNULL) {
      anything = 1;
      if (c->marktype == 'l') {
         maxmark = MAX(maxmark, c->lmark->xmax - c->lmark->xmin);
         maxmarky = MAX(maxmarky, c->lmark->ymax - c->lmark->ymin);
      } else if (c->marktype != 'n') {
        maxmark = MAX(maxmark, ABS(c->marksize[0]));
        maxmarky = MAX(maxmarky, ABS(c->marksize[1]));
      }
      if (c->linetype != '0') l->anylines = 1;
    }
  }
  if (!(l->linelength < FSIG || l->linelength > FSIG))
  {
    l->linelength = (l->anylines) ? (MAX(maxmark + 6.0, 24.0)) : 0.0;
  }
    else l->linelength = disttop(l->linelength, g->x_axis);
  if (!(l->midspace < FSIG || l->midspace > FSIG))
  {
    l->midspace = (l->anylines) ? 4.0 : (maxmark / 2.0) + 4.0;
  }
    else l->midspace = disttop(l->midspace, g->x_axis);
  if (!(l->linebreak < FSIG || l->linebreak > FSIG))
    l->linebreak = MAX(l->l->linesep * FCPI / FPPI, maxmarky);
    else l->linebreak = disttop(l->linebreak, g->y_axis);

  if (l->type == 'c') {
    for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
      if (c->l->label != CNULL) process_label(c->l, g, 1);
    }
    return;
  }

  if (!anything) {
    l->anylines = -1;
    return;
  }

  width = 0.0;
  height = -l->linebreak;
  for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
    if (c->l->label != CNULL) {
      s = c->l->label;
      copy_label(c->l, l->l);
      c->l->x = 0.0;
      c->l->y = 0.0;
      c->l->rotate = 0.0;
      c->l->hj = 'l';
      c->l->vj = 'b';
      c->l->label = s;
      process_label(c->l, g, 0);
      height += c->l->ymax + l->linebreak;
      width = MAX(width, c->l->xmax);
    }
  }
  hdist = (l->anylines) ? l->midspace + l->linelength : l->midspace;
  width += hdist;

  if (!(l->l->x < FSIG || l->l->x > FSIG)) {
    if (l->l->hj == 'c') {
      l->l->x = g->x_axis->psize / 2;
    } else if (l->l->hj == 'l') {
      if (l->l->vj == 'c') {
        l->l->x = g->x_axis->psize;
        if (g->y_axis->draw_axis_label)
          l->l->x = MAX(l->l->x, g->y_axis->label->xmax);
        if (g->y_axis->draw_hash_labels)
          l->l->x = MAX(l->l->x, g->y_axis->hl->xmax);
        if (g->y_axis->draw_hash_marks) {
          l->l->x = MAX(l->l->x, g->y_axis->draw_hash_marks_at);
          l->l->x = MAX(l->l->x, g->y_axis->draw_hash_marks_at +
                                 HASH_DIR(g->y_axis) * HASH_SIZE);
        }
        l->l->x += 15.0;
      } else {
        l->l->x = 0.0;
      }
    } else {
      if (l->l->vj == 'c') {
        l->l->x = 0.0;
        if (g->y_axis->draw_axis_label)
          l->l->x = MIN(l->l->x, g->y_axis->label->xmin);
        if (g->y_axis->draw_hash_labels)
          l->l->x = MIN(l->l->x, g->y_axis->hl->xmin);
        if (g->y_axis->draw_hash_marks) {
          l->l->x = MIN(l->l->x, g->y_axis->draw_hash_marks_at);
          l->l->x = MIN(l->l->x, g->y_axis->draw_hash_marks_at +
                                 HASH_DIR(g->y_axis) * HASH_SIZE);
        }
        l->l->x = l->l->x - 15.0;
      } else {
        l->l->x = g->x_axis->psize;
      }
    }
  } else {
    l->l->x = ctop(l->l->x, g->x_axis);
  }
  if (!(l->l->y < FSIG || l->l->y > FSIG)) {
    if (l->l->vj == 'c') {
      l->l->y = g->y_axis->psize / 2.0;
    } else if (l->l->vj == 'b') {
      l->l->y = g->y_axis->psize;
      if (g->x_axis->draw_axis_label)
        l->l->y = MAX(l->l->y, g->x_axis->label->ymax);
      if (g->x_axis->draw_hash_labels)
        l->l->y = MAX(l->l->y, g->x_axis->hl->ymax);
      if (g->x_axis->draw_hash_marks) {
        l->l->y = MAX(l->l->y, g->x_axis->draw_hash_marks_at);
        l->l->y = MAX(l->l->y, g->x_axis->draw_hash_marks_at +
                               HASH_DIR(g->x_axis) * HASH_SIZE);
      }
      l->l->y += 15.0;
    } else {
      l->l->y = 0.0;
      if (g->x_axis->draw_axis_label)
        l->l->y = MIN(l->l->y, g->x_axis->label->ymin);
      if (g->x_axis->draw_hash_labels)
        l->l->y = MIN(l->l->y, g->x_axis->hl->ymin);
      if (g->x_axis->draw_hash_marks) {
        l->l->y = MIN(l->l->y, g->x_axis->draw_hash_marks_at);
        l->l->y = MIN(l->l->y, g->x_axis->draw_hash_marks_at +
                               HASH_DIR(g->x_axis) * HASH_SIZE);
      }
      l->l->y -= 15.0;
    }
  } else {
    l->l->y = ctop(l->l->y, g->y_axis);
  }

  if (l->l->hj == 'l') x = 0.0;
  else if (l->l->hj == 'c') x = - width/2.0;
  else x = -width;

  if (l->l->vj == 't') y = 0.0;
  else if (l->l->vj == 'c') y = height / 2.0;
  else y = height;

  for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
    if (c->l->label != CNULL) {
      c->l->x = hdist + x;
      c->l->y = y;
      c->l->vj = 't';
      c->l->hj = 'l';
      c->l->rotate = 0.0;
      process_label(c->l, g, 0);
      y = c->l->ymin - l->linebreak;
    }
  }

  process_label_max_n_mins(l->l, width, height);
}

/* ------------------------------------------------------------------------------ */
double find_reasonable_hash_interval(Axis a)
{
  double s, d;

  if (a->is_lg) return 0.0;
  s = a->max - a->min;
  d = 1.0;
  if (s > 5.0) {
    while(1) {
      if (s / d < 6.0) return d;
      d *= 2.0;
      if (s / d < 6.0) return d;
      d *= 2.5;
      if (s / d < 6.0) return d;
      d *= 2.0;
    }
  } else {
    while(1) {
      if (s / d > 2.0) return d;
      d /= 2.0;
      if (s / d > 2.0) return d;
      d /= 2.5;
      if (s / d > 2.0) return d;
      d /= 2.0;
    }
  }
}

/* ------------------------------------------------------------------------------ */
double find_reasonable_hash_start(Axis a)
{
  int i;

  if (a->is_lg) return 0.0;
  if (a->max > 0.0 && a->min < 0.0) return 0.0;
  i = ((int) (a->min / a->hash_interval));
  return ((double) i) * a->hash_interval;
}

/* ------------------------------------------------------------------------------ */
int find_reasonable_precision(Axis a)
{
  int i, b, b2, done;
  double x, x2, tolerance;

  if (a->hash_format == 'g' || a->hash_format == 'G') return 6;
  if (a->hash_format == 'e' || a->hash_format == 'E') return 0;
  if (a->is_lg) return 0;

  tolerance = 0.000001;
  b = 0;
  x = a->hash_interval;

  done = 0;
  while(b < 6 && !done) {
    i = (int) (x + 0.4);
    x2 = i;
    if (x2 - x < tolerance && x - x2 < tolerance) done = 1;
    else {
      b++;
      x *= 10.0;
      tolerance *= 10.0;
    }
  }

  tolerance = 0.000001;
  b2 = 0;
  x = a->hash_start;

  done = 0;
  while(b2 < 6 && !done) {
    i = (int) (x + 0.4);
    x2 = i;
    if (x2 - x < tolerance && x - x2 < tolerance) done = 1;
    else {
      b2++;
      x *= 10.0;
      tolerance *= 10.0;
    }
  }
  return MAX(b, b2);
}

/* ------------------------------------------------------------------------------ */
int find_reasonable_minor_hashes(Axis a)
{
  double d;
  int i;

  if (a->is_lg) {
    d = a->log_base;
    while(d > 10.0) d /= 10.0;
    while(d <= 1.0) d *= 10.0;
    i = (int) d;
    return MAX((i - 2), 0);
  } else {
    d = a->hash_interval;
    if (!(d < 0.0 || d > 0.0)) return 0;
    while(d > 10.0) d /= 10.0;
    while(d <= 1.0) d *= 10.0;
    i = (int) d;
    if (((double) i) < d || ((double) i) > d)
    {
	return 0;
    }
    return i-1;
  }
}

/* ------------------------------------------------------------------------------ */
void process_axis1(Axis a, Graph g)
{
  double tmp;
  int i;

  if (!(a->min < FSIG || a->min > FSIG)) {
    if (!(a->pmin < FSIG || a->pmin > FSIG)) {
      error_header();
      fprintf(stderr,
              "Graph %d: %c axis has no minimum, and cannot derive one\n",
              g->num, AXIS_CHAR(a));
      fprintf(stderr, "  Use %caxis min\n", AXIS_CHAR(a));
      exit(1);
    } else if (a->pmin <= 0.0 && a->is_lg) {
      error_header();
      fprintf(stderr, "Trying to derive %c axis\n", AXIS_CHAR(a));
      fprintf(stderr,
        "        Minimum value %f will be -infinity with log axes\n", a->pmin);
      exit(1);
    } else a->min = a->pmin;
  }
  if (!(a->max < FSIG || a->max > FSIG)) {
    if (!(a->pmax < FSIG || a->pmax > FSIG)) {
      error_header();
      fprintf(stderr,
              "Graph %d: %c axis has no maximum, and cannot derive one\n",
              g->num, AXIS_CHAR(a));
      fprintf(stderr, "  Use %caxis max\n", AXIS_CHAR(a));
      exit(1);
    } else if (a->pmax <= 0.0 && a->is_lg) {
      error_header();
      fprintf(stderr, "Trying to derive %c axis\n", AXIS_CHAR(a));
      fprintf(stderr,
        "        Maximum value %f will be -infinity with log axes\n", a->pmax);
      exit(1);
    } else a->max = a->pmax;
  }
  if (a->max < a->min) {
    tmp = a->max;  a->max = a->min;  a->min = tmp;
  } else if (!(a->max < a->min || a->max > a->min)) {
    if (!a->is_lg) a->min -= 1;
    a->max += 1;
  }
  a->psize = intop(a->size);
  if (a->is_lg) {
    if (a->min <= 0.0) {
      error_header();
      fprintf(stderr,
        "Graph %d, %c axis: Min value = %f.  This is -infinity with logrhythmic axes\n",
        g->num, (a->is_x) ? 'x' : 'y', a->min);
      exit(1);
    }
    a->logfactor = log(a->log_base);
    a->logmin = log(a->min) / a->logfactor;
    a->factor = a->psize / (log(a->max) / a->logfactor - a->logmin);
  } else {
    a->factor = a->psize / (a->max - a->min);
  }
  if (a->gr_graytype == '0') {
    a->gr_graytype = a->graytype;
    for (i = 0; i < 3; i++) a->gr_gray[i] = a->gray[i];
  }
  if (a->mgr_graytype == '0') {
    a->mgr_graytype = a->gr_graytype;
    for (i = 0; i < 3; i++) a->mgr_gray[i] = a->gr_gray[i];
  }
}

/* ------------------------------------------------------------------------------ */
void process_axis2(Axis a, Graph g)
{
  double t1, t2, t3, minor_hashes, hloc, tmp;
  double ymin, ymax, xmin, xmax;
  int prec, i1;
  Hash h;
  String s;
  Axis other;

  other = (a->is_x) ? g->y_axis : g->x_axis;
  if (!(a->draw_at < FSIG || a->draw_at > FSIG))
    a->draw_at = (HASH_DIR(a) == -1) ? 0.0 : other->psize;
  else a->draw_at = ctop(a->draw_at, other);

  if (a->hash_interval < 0.0) {
    a->hash_interval = find_reasonable_hash_interval(a);
    if (!a->start_given)
      a->hash_start = find_reasonable_hash_start(a);
  } else if (!a->start_given) a->hash_start = a->min;
  if (a->minor_hashes < 0) {
    a->minor_hashes = find_reasonable_minor_hashes(a);
  }
  if (a->precision < 0) a->precision = find_reasonable_precision(a);

  for (h = first(a->hash_lines) ; h != nil(a->hash_lines); h = next(h)) {
    h->loc = ctop(h->loc, a);
  }

  for (s = first(a->hash_labels); s != nil(a->hash_labels); s = next(s)) {
    s->s->x = ctop(s->s->x, a);
    s->s->y = ctop(s->s->y, a);
  }

  if ((((a->hash_interval < 0.0 || a->hash_interval > 0.0) && !a->is_lg) || a->is_lg) && a->auto_hash_marks) {
    if (a->is_lg) {
      for (t1 = 1.0; t1 > a->min; t1 /= a->log_base) ;
      t2 = t1 * a->log_base - t1;
    } else {
      for (t1 = a->hash_start; t1 > a->min; t1 -= a->hash_interval) ;
      t2 = a->hash_interval;
    }
    while (t1 <= a->max) {
      hloc = ctop(t1, a);
      if (hloc > -.05 && hloc < a->psize + .05) {
        h = (Hash) get_node((List) a->hash_lines);
        h->loc = hloc;
        h->size = HASH_SIZE;
        h->major = 1;
        insert((List) h, (List) a->hash_lines);
        if (a->auto_hash_labels) {
          s = (String) get_node ((List) a->hash_labels);
          s->s = new_label();
          s->s->x = hloc;
          s->s->y = hloc;
          s->s->label = (char *) malloc (80);
          if (a->precision >= 0) {
            prec = a->precision;
          } else {
            if (ABS(t1) >= 1.0 || !(t1 < 0.0 || t1 > 0.0)) prec = 0;
            else {
              tmp = ABS(t1);
              prec = -1;
              while(tmp < 1.0) {tmp *= 10.0; prec++;}
            }
          }
	  switch(a->hash_format) {
            case 'G': sprintf(s->s->label, "%.*G", prec, t1); break;
            case 'g': sprintf(s->s->label, "%.*g", prec, t1); break;
            case 'E': sprintf(s->s->label, "%.*E", prec, t1); break;
            case 'e': sprintf(s->s->label, "%.*e", prec, t1); break;
            case 'f': sprintf(s->s->label, "%.*f", prec, t1); break;
            default: fprintf(stderr, "Internal jgraph error: hl_st\n");
		     exit(1);
          }
          insert((List) s, (List) a->hash_labels);
        }
      }
      minor_hashes = t2 / ((double) (a->minor_hashes + 1));
      t3 = t1;
      for (i1 = 1; i1 <= a->minor_hashes; i1++) {
        t3 += minor_hashes;
        hloc = ctop(t3, a);
        if (hloc > -.05 && hloc < a->psize + .05) {
          h = (Hash) get_node((List) a->hash_lines);
          h->loc = hloc;
          h->size = MHASH_SIZE;
          h->major = 0;
          insert((List) h, (List) a->hash_lines);
        }
      }
      if (a->is_lg) {
        t1 *= a->log_base;
        t2 = t1 * a->log_base - t1;
      } else t1 += t2;
    }
  }

  if (!(a->draw_hash_marks_at < FSIG || a->draw_hash_marks_at > FSIG))
    a->draw_hash_marks_at = a->draw_at;
  else a->draw_hash_marks_at = ctop(a->draw_hash_marks_at, other);
  if (!(a->draw_hash_labels_at < FSIG || a->draw_hash_labels_at > FSIG))
    a->draw_hash_labels_at = a->draw_hash_marks_at +
      a->hash_scale * HASH_SIZE + HASH_DIR(a) * 3.0;
  else a->draw_hash_labels_at = ctop(a->draw_hash_labels_at, other);

  if (a->is_x) {
    a->hl->y = a->draw_hash_labels_at;
    if (a->hl->hj == '0')
      a->hl->hj = 'c';
    if (a->hl->vj == '0')
      a->hl->vj = (HASH_DIR(a) == -1) ? 't' : 'b';
  } else {
    a->hl->x = a->draw_hash_labels_at;
    if (a->hl->vj == '0') a->hl->vj = 'c';
    if (a->hl->hj == '0')
      a->hl->hj = (HASH_DIR(a) == -1) ? 'r' : 'l';
  }

  ymin = (a->is_x) ? a->hl->y : 0;
  ymax = (a->is_x) ? a->hl->y : a->psize;
  xmin = (!a->is_x) ? a->hl->x : 0;
  xmax = (!a->is_x) ? a->hl->x : a->psize;

  for (s = first(a->hash_labels); s != nil(a->hash_labels); s = next(s)) {
    if (a->is_x) a->hl->x = s->s->x; else a->hl->y = s->s->y;
    a->hl->label = s->s->label;
    process_label(a->hl, g, 0);
    xmin = MIN(a->hl->xmin, xmin);
    ymin = MIN(a->hl->ymin, ymin);
    xmax = MAX(a->hl->xmax, xmax);
    ymax = MAX(a->hl->ymax, ymax);
  }
  a->hl->xmin = xmin;
  a->hl->ymin = ymin;
  a->hl->xmax = xmax;
  a->hl->ymax = ymax;

  /* HERE -- now either test or continue */

  if (a->is_x) {
    if (!(a->label->x < FSIG || a->label->x > FSIG))
      a->label->x = a->psize / 2.0;
      else a->label->x = ctop(a->label->x, g->x_axis);
    if (!(a->label->y < FSIG || a->label->y > FSIG)) {
      ymin = 0.0;
      ymax = other->psize;
      if (a->draw_hash_labels) {
        ymin = MIN(ymin, a->hl->ymin);
        ymax = MAX(ymax, a->hl->ymax);
      }
      if (a->draw_hash_marks) {
        ymin = MIN(ymin, a->draw_hash_marks_at);
        ymin = MIN(ymin, a->draw_hash_marks_at + a->hash_scale * HASH_SIZE);
        ymax = MAX(ymax, a->draw_hash_marks_at);
        ymax = MAX(ymax, a->draw_hash_marks_at + a->hash_scale * HASH_SIZE);
      }
      a->label->y = (HASH_DIR(a) == -1) ? ymin - 8.0 : ymax + 8.0 ;
    } else a->label->y = ctop(a->label->y, g->y_axis);
    if (a->label->hj == '0') a->label->hj = 'c';
    if (a->label->vj == '0') a->label->vj = (HASH_DIR(a) == -1) ? 't' : 'b' ;
    if (!(a->label->rotate < FSIG || a->label->rotate > FSIG))
    {
    a->label->rotate = 0.0;
    }
  } else {
    if (!(a->label->y < FSIG || a->label->y > FSIG))
      a->label->y = a->psize / 2.0;
      else a->label->y = ctop(a->label->y, g->y_axis);
    if (!(a->label->x < FSIG || a->label->x > FSIG)) {
      xmin = 0.0;
      xmax = other->psize;
      if (a->draw_hash_labels) {
        xmin = MIN(xmin, a->hl->xmin);
        xmax = MAX(xmax, a->hl->xmax);
      }
      if (a->draw_hash_marks) {
        xmin = MIN(xmin, a->draw_hash_marks_at);
        xmin = MIN(xmin, a->draw_hash_marks_at + a->hash_scale * HASH_SIZE);
        xmax = MAX(xmax, a->draw_hash_marks_at);
        xmax = MAX(xmax, a->draw_hash_marks_at + a->hash_scale * HASH_SIZE);
      }
      a->label->x = (HASH_DIR(a) == -1) ? xmin - 8.0 : xmax + 8.0 ;
    } else a->label->x = ctop(a->label->x, g->x_axis);
    if (a->label->hj == '0') a->label->hj = 'c';
    if (a->label->vj == '0') a->label->vj = 'b';
    if (!(a->label->rotate < FSIG || a->label->rotate > FSIG))
      a->label->rotate = (HASH_DIR(a) == -1) ? 90.0 : -90.0;
  }
  process_label (a->label, g, 0);
}

/* ------------------------------------------------------------------------------ */
void process_label(Label l, Graph g, int adjust)
{
  double len, height;
  int f, i;
  double fnl, tmp;
  char *s;

  if (l->label == CNULL) return;

  if (adjust) {
    l->x = ctop(l->x, g->x_axis);
    l->y = ctop(l->y, g->y_axis);
  }
  if (!(l->linesep < FSIG || l->linesep > FSIG))
  {
      l->linesep = l->fontsize;
  }

  l->nlines = 0;
  for (i = 0; l->label[i] != '\0'; i++) {
    if (l->label[i] == '\n') {
      l->label[i] = '\0';
      l->nlines++;
    }
  }
  fnl = (double) l->nlines;

  len = 0.0;
  s = l->label;
  for (i = 0; i <= l->nlines; i++) {
    tmp = l->fontsize * FCPI / FPPI * strlen(s) * 0.8;
    len = MAX(len, tmp);
    if (i != l->nlines) {
      f = strlen(s);
      s[f] = '\n';
      s = &(s[f+1]);
    }
  }
  height = (l->fontsize * (fnl+1) + l->linesep * fnl) * FCPI / FPPI;
  process_label_max_n_mins(l, len, height);
}

/* ------------------------------------------------------------------------------ */
void process_label_max_n_mins(Label l, double len, double height)
{
  double xlen, ylen, xheight, yheight;
  double x, y;

  xlen = len * cos(l->rotate * Pi / 180.00);
  ylen = height * cos((l->rotate + 90.0) * Pi / 180.00);
  xheight = len * sin(l->rotate * Pi / 180.00);
  yheight = height * sin((l->rotate + 90.0) * Pi / 180.00);

  x = l->x;
  y = l->y;

  if (l->hj == 'c') {
    x -= xlen / 2.0;
    y -= xheight / 2.0;
  } else if (l->hj == 'r') {
    x -= xlen;
    y -= xheight;
  }
  if (l->vj == 'c') {
    x -= ylen / 2.0;
    y -= yheight / 2.0;
  } else if (l->vj == 't') {
    x -= ylen;
    y -= yheight;
  }

  l->xmin = MIN(x, x + xlen);
  l->xmin = MIN(l->xmin, x + xlen + ylen);
  l->xmin = MIN(l->xmin, x + ylen);

  l->ymin = MIN(y, y + xheight);
  l->ymin = MIN(l->ymin, y + yheight);
  l->ymin = MIN(l->ymin, y + xheight + yheight);

  l->xmax = MAX(x, x + xlen);
  l->xmax = MAX(l->xmax, x + xlen + ylen);
  l->xmax = MAX(l->xmax, x + ylen);

  l->ymax = MAX(y, y + xheight);
  l->ymax = MAX(l->ymax, y + yheight);
  l->ymax = MAX(l->ymax, y + xheight + yheight);

}

/* ------------------------------------------------------------------------------ */
void process_strings(Graph g)
{
  String s;

  for(s = first(g->strings); s != nil(g->strings); s = next(s)) {
    process_label(s->s, g, 1);
  }
}

/* ------------------------------------------------------------------------------ */
void process_curve(Curve c, Graph g)
{
  if (c->bezier && (c->npts < 4 || (c->npts % 3 != 1))) {
    error_header();
    fprintf(stderr, "  Graph %d Curve %d:\n", g->num, c->num);
    fprintf(stderr, "  Curve has %d points\n", c->npts);
    fprintf(stderr, "  Bezier must have 3n + 1 points (n > 0)\n");
    exit(1);
  }
  c->marksize[0] = (!(c->marksize[0] < FSIG || c->marksize[0] > FSIG)) ?
                   4.0 : disttop(c->marksize[0], g->x_axis);
  c->marksize[1] = (!(c->marksize[1] < FSIG || c->marksize[1] > FSIG)) ?
                   4.0 : disttop(c->marksize[1], g->y_axis);
  if (c->marktype == 'o') c->marksize[1] = c->marksize[0];
  c->asize[0] = (!(c->asize[0] < FSIG || c->asize[0] > FSIG)) ?
                   6.0 : disttop(c->asize[0], g->x_axis);
  c->asize[1] = (!(c->asize[1] < FSIG || c->asize[1] > FSIG)) ?
                   2.0 : disttop(c->asize[1], g->y_axis) / 2.0;
  c->lmark->x = disttop(c->lmark->x, g->x_axis);
  c->lmark->y = disttop(c->lmark->y, g->y_axis);
  process_label(c->lmark, g, 0);
  if (!(c->parg < FSIG || c->parg > FSIG))
  {
      c->parg = 0.0;
  }
  if (!(c->aparg < FSIG || c->aparg > FSIG))
  {
      c->aparg = 0.0;
  }
  if (!(c->pparg < FSIG || c->pparg > FSIG))
  {
      c->pparg = 0.0;
  }
}

/* ------------------------------------------------------------------------------ */
void process_curves(Graph g)
{
  Curve c;
  for(c = first(g->curves); c != nil(g->curves); c = next(c)) {
    process_curve(c, g);
  }
}

/* ------------------------------------------------------------------------------ */
void process_extrema(Graph g)  /* This finds all the minval/maxvals for bbox calc */
{
  Curve c;
  String s;
  Axis xa, ya;

  xa = g->x_axis;
  ya = g->y_axis;

  g->xminval = 0.0;
  g->yminval = 0.0;
  g->xmaxval = xa->psize;
  g->ymaxval = ya->psize;

  if (xa->draw_axis_label) process_label_extrema(xa->label, g);
  if (ya->draw_axis_label) process_label_extrema(ya->label, g);
  if (xa->draw_hash_labels) process_label_extrema(xa->hl, g);
  if (ya->draw_hash_labels) process_label_extrema(ya->hl, g);

  if (xa->draw_hash_marks) {
      g->yminval = MIN(g->yminval, xa->draw_hash_marks_at);
      g->yminval = MIN(g->yminval,
                       xa->draw_hash_marks_at + HASH_DIR(xa) * HASH_SIZE);
      g->ymaxval = MAX(g->ymaxval, xa->draw_hash_marks_at);
      g->ymaxval = MAX(g->ymaxval,
                       xa->draw_hash_marks_at + HASH_DIR(xa) * HASH_SIZE);
  }
  if (ya->draw_hash_marks) {
      g->xminval = MIN(g->xminval, ya->draw_hash_marks_at);
      g->xminval = MIN(g->xminval,
                       ya->draw_hash_marks_at + HASH_DIR(ya) * HASH_SIZE);
      g->xmaxval = MAX(g->xmaxval, ya->draw_hash_marks_at);
      g->xmaxval = MAX(g->xmaxval,
                       ya->draw_hash_marks_at + HASH_DIR(ya) * HASH_SIZE);
  }
  process_label_extrema(g->title, g);

  if (g->legend->type == 'c') {
    for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
      process_label_extrema(c->l, g);
    }
  } else if (g->legend->type == 'u' && g->legend->anylines >= 0) {
    process_label_extrema(g->legend->l, g);
  }
  for(s = first(g->strings); s != nil(g->strings); s = next(s)) {
    process_label_extrema(s->s, g);
  }
}

/* ------------------------------------------------------------------------------ */
void process_label_extrema(Label l, Graph g)
{
  if (l->label == CNULL) return;
  g->yminval = MIN(g->yminval, l->ymin);
  g->ymaxval = MAX(g->ymaxval, l->ymax);
  g->xminval = MIN(g->xminval, l->xmin);
  g->xmaxval = MAX(g->xmaxval, l->xmax);
}

/* ------------------------------------------------------------------------------ */
void process_graph(Graph g)
{
  g->x_translate = intop(g->x_translate);
  g->y_translate = intop(g->y_translate);
  process_axis1(g->x_axis, g);
  process_axis1(g->y_axis, g);
  process_axis2(g->x_axis, g);
  process_axis2(g->y_axis, g);
  process_curves(g);
  process_legend(g);
  process_strings(g);
  process_title(g);
  process_extrema(g);
}

/* ------------------------------------------------------------------------------ */
void process_graphs(Graphs gs)
{
  Graphs the_g;
  Graph g;
  double diff, max_y, min_y, max_x, min_x;
  int do_bb, i;

  Pi = acos(-1.0);
  for (the_g = first(gs); the_g != nil(gs); the_g = next(the_g)) {
    for (g = first(the_g->g); g != nil(the_g->g); g = next(g)) process_graph(g);
    max_x = 0.0;
    min_x = 0.0;
    max_y = 0.0;
    min_y = 0.0;
    for (g = first(the_g->g); g != nil(the_g->g); g = next(g)) {
      max_y = MAX(max_y, g->y_translate + g->ymaxval);
      min_y = MIN(min_y, g->y_translate + g->yminval);
      max_x = MAX(max_x, g->x_translate + g->xmaxval);
      min_x = MIN(min_x, g->x_translate + g->xminval);
    }

    if (the_g->height >= 0.00) {
      the_g->height *= FCPI;
      if (the_g->height > max_y - min_y) {
        diff = (the_g->height - max_y + min_y) / 2.0;
        max_y += diff;
        min_y -= diff;
      } else {
        the_g->height = max_y - min_y;
      }
    } else {
      the_g->height = max_y - min_y;
    }
    if (the_g->width >= 0.00) {
      the_g->width *= FCPI;
      if (the_g->width > max_x - min_x) {
        diff = (the_g->width - max_x + min_x) / 2.0;
        max_x += diff;
        min_x -= diff;
      } else {
        the_g->width = max_x - min_x;
      }
    } else {
      the_g->width = max_x - min_x;
    }

    do_bb = 1;
    for (i = 0; i < 4; i++) do_bb = (do_bb && the_g->bb[i] == ISIG);
    if (do_bb) {
      the_g->bb[0] = (int) (min_x - 1.0);
      the_g->bb[1] = (int) (min_y - 1.0);
      the_g->bb[2] = (int) (max_x + 1.0);
      the_g->bb[3] = (int) (max_y + 1.0);
    }
  }
}

/* ------------------------------------------------------------------------------ */
/* End of file process.c */
