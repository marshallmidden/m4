/* jgraph.h
 * James S. Plank
 
Jgraph - A program for plotting graphs in postscript.

 * $Source: /Users/plank/src/jgraph/RCS/jgraph.h,v $
 * $Revision: 8.4 $
 * $Date: 2012/10/15 15:54:18 $
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

#include "list.h"
#include "prio_list.h"
#include <stdlib.h>

#define PPI 120
#define FPPI 120.0
#define CPI 72.0
#define FCPI 72.0
#define CNULL ((char *)0)
#define GMNULL ((Point)0)
#define FSIG -10010.0
#define ISIG -11111111
#define HASH_SIZE 5.0
#define MHASH_SIZE 2.0

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))

typedef struct point {
  struct point *flink;
  struct point *blink;
  double x;
  double y;
  char e;          /* 'x' for x_ebars, 'y' for y_ebars, 'p' for neither */
} *Point;

typedef struct flist {
  struct flist *flink;
  struct flist *blink;
  double f;
} *Flist;

typedef struct label {
  char *label;
  double x;
  double y;
  double rotate;
  const char *font;
  double fontsize;
  char hj;
  char vj;
  double gray[3];
  char graytype;
  double linesep;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  int nlines;
} *Label;

typedef struct curve {
  struct curve *flink;
  struct curve *blink;
  int num;
  Label l;
  Label lmark;
  Point pts;
  Point yepts;
  Point xepts;
  int npts;
  Point general_marks;
  double marksize[2];
  double fill[3];
  double gray[3];
  double afill[3];
  double pfill[3];
  double linethick;
  double mrotate;
  Flist gen_linetype;
  char graytype ;
  char filltype ;
  char afilltype;
  char pfilltype;
  char pattern;
  double parg;
  char apattern;
  double aparg;
  char ppattern;
  double pparg;
  char marktype;
  char linetype;
  char *postscript;
  char *eps;
  int postfile;
  int rarrow;
  int larrow;
  int rarrows;
  int larrows;
  int bezier;
  int poly;
  double asize[2];
  int clip;
} *Curve;

typedef struct string {
  struct string *flink;
  struct string *blink;
  int num;
  Label s;
} *String;

typedef struct hash {
  struct hash *flink;
  struct hash *blink;
  double loc;
  double size;
  int major;
} *Hash;

typedef struct deflt {
  double rotate;
  double fontsize;
  Point general_marks;
  double fill;
  double linethick;
  double marksize[2];
  char *font;
  char hj;
  char vj;
  char marktype;
} *Default;
  
typedef struct axis {
  Label label;
  Label hl;
  double max;
  double min;
  double pmax;
  double pmin;
  double logmin;
  double logfactor;
  double size;
  double psize;
  double factor;
  double hash_interval;
  double hash_start;
  double hash_scale;
  double log_base;
  double draw_hash_marks_at;
  double draw_hash_labels_at;
  double draw_at;
  double gray[3];
  char graytype;
  double gr_gray[3];
  char gr_graytype;
  double mgr_gray[3];
  char mgr_graytype;
  char hash_format;
  int grid_lines;
  int mgrid_lines;
  int draw_hash_labels;
  int draw_axis_line;
  int draw_hash_marks;
  int draw_axis_label;
  int auto_hash_labels;
  int auto_hash_marks;
  int minor_hashes;
  int precision;
  int start_given;
  String hash_labels;
  Hash hash_lines;
  int is_x;
  int is_lg;
} *Axis;

typedef struct legend {
  double linelength;
  double linebreak;
  int anylines;
  double midspace;
  char type; /* 'n' = off, 'u' = userdefined (use Label), 'c' = custom */
  Label l;
} *Legend;

typedef struct graph {
  struct graph *flink;
  struct graph *blink;
  int num;
  double xminval;
  double yminval;
  double xmaxval;
  double ymaxval;
  double x_translate;
  double y_translate;
  Axis x_axis;
  Axis y_axis;
  Curve curves;
  Legend legend;
  String strings;
  Label title;
  int clip;
  int border;
  Default def;
} *Graph;

typedef struct graphs {
  struct graphs *flink;
  struct graphs *blink;
  Graph g;
  double height;
  double width;
  int bb[4]; /* Bounding box */
  char *preamble; 
  char *epilogue; 
  int prefile;
  int epifile;
  int page;
} *Graphs;

/* draw.c */
extern double ctop(double, Axis);
extern double disttop(double, Axis);
extern double intop(double);
extern double ptoc(double, Axis);
extern double ptodist(double, Axis);
extern void draw_graphs(Graphs, int, int);

/* Stuff defined in jgraph.c */
extern Curve new_line(Curve, int);
extern Curve new_curve(Curve, int);
extern Curve get_curve(Curve, int);
extern Graph new_graph(Graph, int);
extern void new_graphs(Graphs);
extern Graph get_graph(Graph, int);
extern String new_string(String, int);
extern String get_string(String, int);
extern Label new_label(void);
extern const char *MARKTYPESTRS[];
extern const char MARKTYPES[];
extern int NMARKTYPES;
extern int NORMALMARKTYPES;
extern const char *PATTERNS[];
extern const char PTYPES[];
extern int NPATTERNS;

/* printline.c */
extern void setlinewidth(double);
extern void comment(const char *);
extern void gsave(void);
extern void setgray(char, double *);
extern void printline(double, double, double, double, char);
extern void grestore(void);
extern void print_label(Label);
extern void setlinestyle(char, Flist);
extern void print_ebar(double, double, double, double, char);
extern void start_line(double, double, Curve);
extern void bezier_end(double, double);
extern void bezier_control(double, double);
extern void setfill(double, double, char, double *, char, double);
extern void end_line(void);
extern void cont_line(double, double);
extern void start_poly(double, double);
extern void cont_poly(double, double);
extern void end_poly(double, double, char, double *, char, double);
extern void printellipse(double, double, double, double, char, double *, char, double);
extern void setfont(const char *, double);
extern void set_comment(int c);

/* prio_list.c */

typedef int Boolean;

/* A prioirity list is any list with the first three fields being flink, 
 * blink and prio.  Use the routines of list.c to do everything except
 * insertion */
 
typedef struct prio_list {
  struct prio_list *flink;
  struct prio_list *blink;
  int prio;
} *Prio_list;

extern void prio_insert(Prio_list, Prio_list, Boolean);

/* token.c */
extern void set_input_file(char *);
extern void error_header(void);
extern int gettokenchar(void);
extern void ungettokenchar(void);
extern int gettoken(char *);
extern void get_comment(void);
extern int getstring(char *);
extern int getint(int *);
extern int getdouble(double *);
extern char *getmultiline(void);
extern char *getlabel(void);
extern int getsystemstring(void);
extern void rejecttoken(void);

/* edit.c */
extern void edit_graphs(Graphs);

/* process.c */
extern void copy_label(Label, Label);
extern void process_title(Graph);
extern void process_legend(Graph);
extern double find_reasonable_hash_interval(Axis) ;
extern double find_reasonable_hash_start(Axis);
extern int find_reasonable_precision(Axis);
extern int find_reasonable_minor_hashes(Axis);
extern void process_axis1(Axis a, Graph);
extern void process_axis2(Axis a, Graph);
extern void process_label(Label, Graph, int);
extern void process_label_max_n_mins(Label, double, double);
extern void process_strings(Graph);
extern void process_curve(Curve, Graph);
extern void process_curves(Graph);
extern void process_extrema(Graph);
extern void process_label_extrema(Label, Graph);
extern void process_graph(Graph);
extern void process_graphs(Graphs);

/* show.c */
extern void show_graphs(Graphs);

/* End of file jgraph.h */
