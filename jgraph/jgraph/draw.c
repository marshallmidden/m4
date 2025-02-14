/* draw.c
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/draw.c,v $
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

#include "jgraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void draw_axis(Axis, Axis);
static void draw_label(Label);
static void draw_curves(Graph);
static void draw_curve(Curve, Graph);
static void draw_mark(double, double, Curve, Graph);
static void draw_arrow(double, double, double, double, Curve);
static void draw_legend(Graph);
static void draw_strings(Graph);
static void draw_graph(Graph);
static void draw_header(Graphs, int, int);
static void draw_footer(Graphs, int);

static char real_eof = EOF;

double ctop(double val, Axis axis)
{
  if (axis->is_lg) {
    if (val <= 0.0) {
      error_header();
      fprintf(stderr, 
              "Value of %f is at negative infinity with logrhythmic %c axis\n", 
              val, (axis->is_x) ? 'x' : 'y'); 
       exit(1);
    }
    return (log(val) / axis->logfactor - axis->logmin) * axis->factor;
  } else {
    return (val - axis->min) * axis->factor;
  }
}

double disttop(double val, Axis axis)
{
  if (axis->is_lg) {
    return FCPI * val;
  } else {
    return (val) * axis->factor;
  }
}

double intop(double val)
{
  return FCPI * val;
}

static void draw_axis(Axis a, Axis other)
{
  char orientation;
  Hash h;
  String s;

  orientation = (a->is_x) ? 'x' : 'y';
  setlinewidth(1.0);
  comment("Drawing Axis");
  if (a->grid_lines) {
    comment("Drawing Grid lines");
    gsave();
    setgray(a->gr_graytype, a->gr_gray);
    for (h = first(a->hash_lines); h != nil(a->hash_lines); h = next(h)) {
      if (h->major) {
        printline(h->loc, 0.0, h->loc, other->psize, orientation);
      }
    }
    grestore();
  }
  if (a->mgrid_lines) {
    comment("Drawing Minor Grid lines");
    gsave();
    setgray(a->mgr_graytype, a->mgr_gray);
    for (h = first(a->hash_lines); h != nil(a->hash_lines); h = next(h)) {
      if (!h->major) {
        printline(h->loc, 0.0, h->loc, other->psize, orientation);
      }
    }
    grestore();
  }
  gsave();
  setgray(a->graytype, a->gray);
  if (a->draw_axis_line) {
    printline(0.0, a->draw_at, a->psize, a->draw_at, orientation);
  }
  if (a->draw_hash_marks) {
    comment("Drawing Hash Marks");
    for (h = first(a->hash_lines); h != nil(a->hash_lines); h = next(h)) {
      printline(h->loc, a->draw_hash_marks_at, h->loc, 
                a->draw_hash_marks_at + (h->size * a->hash_scale), 
                orientation);
    }
  }
  if (a->draw_hash_labels) {
    comment("Drawing Hash Labels");
    for (s = first(a->hash_labels); s != nil(a->hash_labels); s = next(s)) {
      a->hl->label = s->s->label;
      if (a->is_x) {
        a->hl->x = s->s->x;
      } else {
        a->hl->y = s->s->y;
      }
      draw_label(a->hl);
    }
  }
  if (a->draw_axis_label) {
    comment("Drawing Axis Label");
    draw_label(a->label);
  }
  grestore();
  printf("\n");
}

static void draw_label(Label l)
{
  if (l->label == CNULL) return;
  comment(l->label);
  print_label(l);
}

static void set_clip(Graph g)
{
  comment("Setting Clip");
  printf("newpath\n");
  printf(" 0 0 moveto 0 %f lineto %f %f lineto %f 0 lineto\n",
           g->y_axis->psize, g->x_axis->psize,
           g->y_axis->psize, g->x_axis->psize);
  printf("  closepath clip newpath\n");
}

static void draw_curves(Graph g)
{
  Curve c;

  gsave();
  printf("\n");
  if (g->clip) set_clip(g);
  for(c = first(g->curves); c != nil(g->curves); c = next(c)) {
    draw_curve(c, g);
  }
  grestore();
  printf("\n");
}

static void draw_curve(Curve c, Graph g)
{
  Point p, px, py;
  int i, j;
  double this_x, this_y;
  double last_x = 0.0;
  double last_y = 0.0;
  double x, y;

  gsave();
  setgray(c->graytype, c->gray);
  if (c->clip) set_clip(g);
  if (first(c->xepts) != nil(c->xepts) ||
      first(c->yepts) != nil(c->yepts)) {
    comment("Drawing Epts");
    px = first(c->xepts);
    py = first(c->yepts);
    setlinewidth(c->linethick);
    setlinestyle('s', (Flist)0);
    for (p = first(c->pts); p != nil(c->pts); p = next(p)) {
      if (p->e == 'x') {
        x = ctop(p->x, g->x_axis);
        y = ctop(p->y, g->y_axis);
        print_ebar(x, y, ctop(px->x, g->x_axis), c->marksize[1]/2.0, 'x');
        px = next(px);
        print_ebar(x, y, ctop(px->x, g->x_axis), c->marksize[1]/2.0, 'x');
        px = next(px);
      } else if (p->e == 'y') {
        x = ctop(p->x, g->x_axis);
        y = ctop(p->y, g->y_axis);
        print_ebar(y, x, ctop(py->y, g->y_axis), c->marksize[0]/2.0, 'y');
        py = next(py);
        print_ebar(y, x, ctop(py->y, g->y_axis), c->marksize[0]/2.0, 'y');
        py = next(py);
      }
    }
  }

  comment("Drawing Curve");
  if (c->linetype != '0' || c->poly) {
    if (c->bezier) {
      i = 0;
      j = 0;
      if (c->poly) printf("newpath ");
      for (p = first(c->pts); p != nil(c->pts); p = next(p)) {
        if (j == 0 && i == 0) {
          start_line(ctop(p->x, g->x_axis), ctop(p->y, g->y_axis), c);
          j++;
        } else if (i != 0) {
          bezier_control(ctop(p->x, g->x_axis), ctop(p->y, g->y_axis));
        } else {
          bezier_end(ctop(p->x, g->x_axis), ctop(p->y, g->y_axis));
          j++;
        }
        if (!c->poly && j == 30 && i == 0) {
          end_line();
          p = prev(p);
          j = 0;
          i = 0;
        } else i = (i + 1) % 3;
      }
      if (j != 0) {
        if (c->poly) {
          printf("closepath ");
          setfill(0.0, 0.0, c->pfilltype, c->pfill, c->ppattern, c->pparg);
        }
        end_line();
      }
    } else {
      i = 0;
      if (c->poly) printf("newpath ");
      for (p = first(c->pts);
           p != nil(c->pts);
           p = next(p)) {
        if (i == 0) {
          start_line(ctop(p->x, g->x_axis), ctop(p->y, g->y_axis), c);
        } else {
          cont_line(ctop(p->x, g->x_axis), ctop(p->y, g->y_axis));
        } 
        if (!c->poly && i == 100 && next(p)) {
          end_line();
          p = prev(p);
          i = 0;
        } else i++;
      }
      if (i != 0) {
        if (c->poly) {
          printf("closepath ");
          setfill(0.0, 0.0, c->pfilltype, c->pfill, c->ppattern, c->pparg);
        }
        end_line();
      }
    }
  }
  comment("Drawing Curve points");
  i = 0;
  for (p = first(c->pts);
       p != nil(c->pts);
       p = next(p)) {
    this_x = ctop(p->x, g->x_axis);
    this_y = ctop(p->y, g->y_axis);
    if (!c->bezier || i == 0) draw_mark(this_x, this_y, c, g);
    if (p != first(c->pts)) {
      if (c->rarrows || (c->rarrow && p == last(c->pts))) {
        if (!c->bezier || i == 0) 
          draw_arrow(this_x, this_y, last_x, last_y, c);
      }
      if (c->larrows || (c->larrow && prev(p) == first(c->pts))) {
        if (!c->bezier || i == 1) 
          draw_arrow(last_x, last_y, this_x, this_y, c);
      }
    }
    last_x = this_x;  
    last_y = this_y;  
    i = (i + 1) % 3;
  }
  grestore();
  printf("\n");
}

static void draw_mark(double x, double y, Curve c, Graph g)
{
  Point p;
  double ms0, ms1, scx, scy, trx, try;
  int i, j;
  FILE *f;
  char ch;
  int done;
  char inp[1000];
  int bb[4];

  if (c->marktype == 'n') return;
  ms0 = c->marksize[0] / 2.0;
  ms1 = c->marksize[1] / 2.0;

  gsave();
  printf(" %f %f translate %f rotate\n", x, y, c->mrotate);

  switch (c->marktype) {
    case 'n': break;
    case 'E': if (c->eps == CNULL) break;
              f = fopen(c->eps, "r");
              if (f == NULL) {
                fprintf(stderr, "Error: eps file %s couldn't be opened\n",
                      c->eps);
                exit(1);
              }
              /* Get bbox */
              done = 0;
              while (!done && fgets(inp, 1000, f) != NULL) {
                if (strncmp("%%BoundingBox:", inp, 14) == 0) done = 1;
              }
              if (!done) {
                fprintf(stderr, "Error: Eps file '%s' has %s\n",
                        c->eps, "no bounding box");
                exit(1);
              }
              if (sscanf(inp+14, "%d %d %d %d", bb, bb+1, bb+2, bb+3) != 4) {
                fprintf(stderr, "Error: Eps file '%s': bad bounding box.\n", c->eps);
                exit(1);
              }
              if (bb[2] - bb[0] == 0) {
                scx = ms0;
                trx = 0.0;
              } else {
                scx = ms0 * 2.0/(double)(bb[2] - bb[0]);
                trx = -(double)(bb[2] - bb[0])/2.0 - bb[0];
              }
              if (bb[3] - bb[1] == 0) {
                scy = ms1;
                try = 0.0;
              } else {
                scy = ms1 * 2.0/(double)(bb[3] - bb[1]);
                try = -(double)(bb[3] - bb[1])/2.0 - bb[1];
              }
	      /* Don't scale if ms == 0 0 */
	      if (!(ms0 < 0.0 || ms0 > 0.0) && !(ms1 < 0.0 || ms1 > 0.0)) {
                scx = 1.0;
                scy = 1.0;
              }

              sprintf(inp, "Including eps file %s", c->eps);
              comment(inp);
              /* Use bbox to scale and translate */
                
              printf("%f %f scale %f %f translate\n", scx, scy, trx, try);
              /* Include the rest of the file */
              for (ch = getc(f); ch != real_eof; ch = getc(f)) putchar(ch);
              putchar('\n');
              fclose(f);
              break;
    case 'p': if (c->postscript == CNULL) break;
	      if (ms0 < 0.0 || ms0 > 0.0 || ms1 < 0.0 || ms1 > 0.0) {
                printf("%f %f scale\n", ms0, ms1);
              }
              if (!c->postfile) {
                printf("%s\n", c->postscript);
              } else {
                f = fopen(c->postscript, "r");
                if (f == NULL) {
                  fprintf(stderr, 
                          "Error: postscript file %s couldn't be opened\n",
                          c->postscript);
                  exit(1);
                }
                for (ch = getc(f); ch != real_eof; ch = getc(f)) putchar(ch);
                putchar('\n');
                fclose(f);
              }
              break;
    case 'c': printline(-ms0, 0.0, ms0, 0.0, 'x');
              printline(-ms1, 0.0, ms1, 0.0, 'y');
              break;
    case 'b': start_poly(-ms0, -ms1);
              cont_poly(ms0, -ms1);
              cont_poly(ms0, ms1);
              cont_poly(-ms0, ms1);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'd': start_poly(-ms0, 0.0);
              cont_poly(0.0, -ms1);
              cont_poly(ms0, 0.0);
              cont_poly(0.0, ms1);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'g': p = first(c->general_marks);
              if (p == nil(c->general_marks)) break;
              if (next(p) == nil(c->general_marks)) break;
              start_poly(p->x*ms0, p->y*ms1);
              for(p = next(p); p != nil(c->general_marks); p = next(p))
                cont_poly(p->x*ms0, p->y*ms1);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'G': i = 0;
              for (p = first(c->general_marks);
                   p != nil(c->general_marks);
                   p = next(p)) {
                if (i == 0) {
                  printf("%f %f moveto ", p->x*ms0, p->y*ms1);
                } else {
                  printf("%f %f lineto\n", p->x*ms0, p->y*ms1);
                }
                if (i == 100) {
                  printf("stroke\n");
                  p = prev(p);
                  i = 0;
                } else i++;
              }
              if (i != 0) printf("stroke\n");
              break;
    case 'B': i = 0;
              j = 0;
              for (p = first(c->general_marks);
                   p != nil(c->general_marks);
                   p = next(p)) {
                if (j == 0 && i == 0) {
                  printf("%f %f moveto ", p->x*ms0, p->y*ms1);
                  j++;
                } else if (i != 0) {
                  printf("%f %f ", p->x*ms0, p->y*ms1);
                } else {
                  printf("%f %f curveto\n", p->x*ms0, p->y*ms1);
                  j++;
                }
                if (j == 30 && i == 0) {
                  printf(" stroke\n");
                  p = prev(p);
                  j = 0;
                  i = 0;
                } else i = (i + 1) % 3;
              }
              if (j != 0) printf(" stroke\n");
              if (! ((i == 1) || (i == 0 && j == 0))) {
                fprintf(stderr, "Error: curve %d, %s\n", c->num,
                        "wrong number of points for bezier marktype\n");
                exit(1);
              }
              break;

    case 'Z': i = 0;
              j = 0;
              for (p = first(c->general_marks);
                   p != nil(c->general_marks);
                   p = next(p)) {
                if (i == 0 && j == 0) {
                  printf("newpath %f %f moveto ", p->x*ms0, p->y*ms1);
                  j++;
                } else if (i != 0) {
                  printf("%f %f ", p->x*ms0, p->y*ms1);
                } else {
                  printf("%f %f curveto\n", p->x*ms0, p->y*ms1);
                }
                i = (i + 1) % 3;
              }
              printf("closepath ");
              setfill(x, y, c->filltype, c->fill, c->pattern, c->parg);
              printf("stroke\n");

              if (i != 1) {
                fprintf(stderr, "Error: curve %d, %s\n", c->num,
                        "wrong number of points for bezier marktype\n");
                exit(1);
              }
              break;

    case 'x': printline(-ms0, -ms1, ms0, ms1, 'x');
              printline(-ms0, ms1, ms0, -ms1, 'x');
              break;
    case 'o': printellipse(x, y, ms0, ms0, 
                           c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'e': printellipse(x, y, ms0, ms1,
                           c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 't': start_poly(ms0, -ms1);
              cont_poly(0.0, ms1);
              cont_poly(-ms0, -ms1);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'X': start_poly(ms0, 0.0);
              cont_poly(-ms0, 0.0);
              cont_poly(-ms0, g->x_axis->draw_at - y);
              cont_poly(ms0, g->x_axis->draw_at - y);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'Y': start_poly(0.0, ms1);
              cont_poly(0.0, -ms1);
              cont_poly(g->y_axis->draw_at - x, -ms1);
              cont_poly(g->y_axis->draw_at - x, ms1);
              end_poly(x, y, c->filltype, c->fill, c->pattern, c->parg);
              break;
    case 'l': draw_label(c->lmark);
              break;
    default: error_header(); 
             fprintf(stderr, "Unknown mark: %c\n", c->marktype);
             break;
  }
  grestore();
}

static void draw_arrow(double x1, double y1, double x2, double y2, Curve c)
{
  double dx, dy;
  double ms0;
  double theta, ct, st;
  
  
  if (c->marktype == 'o') {
    dx = x1 - x2;
    dy = y1 - y2;
    if (!(dx < 0.0 || dx > 0.0) && !(dy < 0.0 || dy > 0.0)) return;

    ms0 = c->marksize[0] / 2.0;
    if (!(dx < 0.0 || dy > 0.0)) theta = asin(1.0); else theta = atan(dy/dx);
    if (theta < 0.0) theta = -theta;
    ct = cos(theta)*ms0;
    st = sin(theta)*ms0;
    x1 = x1 + ct*(dx > 0.0 ? -1.0 : 1.0);
    y1 = y1 + st*(dy > 0.0 ? -1.0 : 1.0);

    if ( ((x1 - x2 > 0) != (dx > 0)) || 
         ((y1 - y2 > 0) != (dy > 0)) ) return;
  }

  dx = x1 - x2;
  dy = y1 - y2;
  if (!(dx < 0.0 || dx > 0.0) && !(dy < 0.0 || dy > 0.0)) return;

  gsave();
  printf("%f %f translate %f %f atan rotate\n", x1, y1, dy, dx);
  start_poly(0.0, 0.0);
  cont_poly(-(c->asize[0]), (c->asize[1]));
  cont_poly(-(c->asize[0]), -(c->asize[1]));
  end_poly(0.0, 0.0, c->afilltype, c->afill, c->apattern, c->aparg);
  grestore();
  printf("\n");
}

static void draw_legend(Graph g)
{
  Curve c;
  Legend l;
  double x = 0.0;
  double y;
  char tmpmktype;

  l = g->legend;
  comment("Drawing legend");
  if (l->type == 'n' || l->anylines < 0) return;
  gsave();
  if (l->type == 'u') {
    printf("%f %f translate %f rotate\n", l->l->x, l->l->y, l->l->rotate);
  }
  for (c = first(g->curves); c != nil(g->curves); c = next(c)) {
    if (c->l->label != CNULL) {
      gsave();
      setgray(c->graytype, c->gray);
      y = (c->l->ymax + c->l->ymin) / 2.0;
      if (l->anylines) {
        if (c->linetype != '0' && (l->linelength < 0 || l->linelength > 0)) {
          if (l->type == 'c' && c->l->hj == 'r') {
            x = c->l->x + l->midspace;
          } else {
            x = c->l->x - l->midspace - l->linelength;
          }
          start_line(x, y, c); 
          cont_line(x+l->linelength, y);
          end_line();
        }
        tmpmktype = c->marktype;
        c->marktype = 'n';
        if (c->larrows || c->larrow) draw_arrow(x, y, x+l->linelength, y, c);
        if (c->rarrows || c->rarrow) draw_arrow(x+l->linelength, y, x, y, c);
        c->marktype = tmpmktype;
        if (l->type == 'c' && c->l->hj == 'r') {
          x = c->l->x + l->midspace + l->linelength / 2.0;
        } else {
          x = c->l->x - l->midspace - l->linelength / 2.0;
        }
      } else if (l->type == 'c' && c->l->hj == 'r') {
        x = c->l->x + l->midspace;
      } else {
        x = c->l->x - l->midspace;
      }
      if (c->marktype == 'X' || c->marktype == 'Y') {
        char old;
        old = c->marktype;
        c->marktype = 'b'; 
        draw_mark(x, y, c, g); 
        c->marktype = old;
      } else {
        draw_mark(x, y, c, g);
      }
      grestore();
      printf("\n");
      draw_label(c->l);
    }
  }
  grestore();
  printf("\n");
}

static void draw_strings(Graph g)
{
  String s;

  comment("Drawing strings");
  for (s = first(g->strings); s != nil(g->strings); s = next(s))
    draw_label(s->s);
}

static void draw_graph(Graph g)
{
  comment("Drawing New Graph");
  printf("%f %f translate\n", g->x_translate, g->y_translate);
  if (g->border) {
    printline(0.0, 0.0, 0.0, g->y_axis->psize, 'x');
    printline(0.0, 0.0, 0.0, g->x_axis->psize, 'y');
    printline(g->x_axis->psize, 0.0, g->x_axis->psize, g->y_axis->psize, 'x');
    printline(g->y_axis->psize, 0.0, g->y_axis->psize, g->x_axis->psize, 'y');
  }
  draw_axis(g->x_axis, g->y_axis);
  draw_axis(g->y_axis, g->x_axis);
  draw_label(g->title);
  draw_curves(g);
  draw_legend(g);
  draw_strings(g);
  printf("%f %f translate\n", - g->x_translate, - g->y_translate);
}

void draw_graphs(Graphs gs, int pp, int landscape)
{
  Graphs gs_p;
  Graph g;

  for (gs_p = first(gs); gs_p != nil(gs); gs_p = next(gs_p)) {
    draw_header(gs_p, pp, landscape);
    for (g = first(gs_p->g); g != nil(gs_p->g); g = next(g)) {
      draw_graph(g);
    }
    draw_footer(gs_p, pp);
  }
}

static void draw_header(Graphs gs, int pp, int landscape)
{
  FILE *f;
  char c;

  if (gs->page == 1) printf("%%!PS-Adobe-2.0 EPSF-1.2\n");
  printf("%%%%Page: %d %d\n", gs->page, gs->page);
  if (landscape) {
    printf("%%%%BoundingBox: %d %d %d %d\n", gs->bb[1], gs->bb[0], 
            gs->bb[3], gs->bb[2]);
  } else {
    printf("%%%%BoundingBox: %d %d %d %d\n", gs->bb[0], gs->bb[1], 
            gs->bb[2], gs->bb[3]);
  }

  printf("%%%%EndComments\n");
  if (landscape) {
    printf("-90 rotate\n");
  }
  if (pp) {
    if (landscape) {
      printf("%f 0 translate\n", -(11.0 * FCPI));
      printf("%f %f translate\n",
        (((11.0 * FCPI) - (gs->bb[2] - gs->bb[0])) / 2.0) - gs->bb[0],     
        (((8.5 * FCPI) - (gs->bb[3] - gs->bb[1])) / 2.0) - gs->bb[1]);     
    } else {
      printf("%f %f translate\n",
        (((8.5 * FCPI) - (gs->bb[2] - gs->bb[0])) / 2.0) - gs->bb[0],     
        (((11.0 * FCPI) - (gs->bb[3] - gs->bb[1])) / 2.0) - gs->bb[1]);     
    }
  } else if (landscape) {
    printf("%f 0 translate\n", (double) (-gs->bb[2] - gs->bb[0]));
  }
  printf("1 setlinecap 1 setlinejoin\n");
  printf("0.700 setlinewidth\n");
  printf("0.00 setgray\n");

  printf("\n");
  printf("/Jrnd { exch cvi exch cvi dup 3 1 roll idiv mul } def\n");

  printf("/JDEdict 8 dict def\n");
  printf("JDEdict /mtrx matrix put\n");
  printf("/JDE {\n");
  printf("  JDEdict begin\n");
  printf("  /yrad exch def\n");
  printf("  /xrad exch def\n");
  printf("  /savematrix mtrx currentmatrix def\n");
  printf("  xrad yrad scale\n");
  printf("  0 0 1 0 360 arc\n");
  printf("  savematrix setmatrix\n");
  printf("  end\n");
  printf("} def\n");

  printf("/JSTR {\n");
  printf("  gsave 1 eq { gsave 1 setgray fill grestore } if\n");
  printf("    exch neg exch neg translate \n");
  printf("    clip                        \n");
  printf("    rotate                      \n");
  printf("    4 dict begin\n");
  printf("      pathbbox  /&top exch def\n");
  printf("                /&right exch def\n");
  printf("                /&bottom exch def\n");
  printf("                &right sub /&width exch def\n");
  printf("      newpath\n");
  printf("      currentlinewidth mul round dup               \n");
  printf("      &bottom exch Jrnd exch &top             \n");
  printf("      4 -1 roll currentlinewidth mul setlinewidth  \n");
  printf("      { &right exch moveto &width 0 rlineto stroke } for    \n");
  printf("    end\n");
  printf("  grestore\n");
  printf("  newpath\n");
  printf("} bind def\n");

  gsave();
  setfont("Times-Roman", 9.00);
  if (gs->preamble != CNULL) {
    if (gs->prefile) {
      f = fopen(gs->preamble, "r");
      if (f == NULL) {
        fprintf(stderr, "Error: preamble file %s couldn't be opened\n",
                gs->preamble);
        exit(1);
      }
      for (c = getc(f); c != real_eof; c = getc(f)) putchar(c);
      putchar('\n');
      fclose(f);
    } else {
      printf("%s\n", gs->preamble);
    }
  }
}

static void draw_footer(Graphs gs, int pp)
{
  FILE *f;
  char c;

  if (gs->epilogue != CNULL) {
    if (gs->epifile) {
      f = fopen(gs->epilogue, "r");
      if (f == NULL) {
        fprintf(stderr, "Error: epilogue file %s couldn't be opened\n",
                gs->epilogue);
        exit(1);
      }
      for (c = getc(f); c != real_eof; c = getc(f)) putchar(c);
      putchar('\n');
      fclose(f);
    } else {
      printf("%s\n", gs->epilogue);
    }
  }
  grestore();
  if (pp) printf("showpage\n"); else printf("\n");
}

