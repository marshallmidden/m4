/* #define	DEBUG_ENTRY */
/* #define	DEBUG_EXIT */
/* #define	DEBUG_LOTS */
/*
 * Copyright (c) 1997
 * Digital Equipment Corporation.  All rights reserved.
 * 
 * By downloading, installing, using, modifying or distributing this
 * software, you agree to the following:
 * 
 * 1. CONDITIONS. Subject to the following conditions, you may download,
 * install, use, modify and distribute this software in source and binary
 * forms:
 * 
 * a) Any source code, binary code and associated documentation
 * (including the online manual) used, modified or distributed must
 * reproduce and retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 
 * b) No right is granted to use any trade name, trademark or logo of
 * Digital Equipment Corporation.  Neither the "Digital Equipment
 * Corporation" name nor any trademark or logo of Digital Equipment
 * Corporation may be used to endorse or promote products derived from
 * this software without the prior written permission of Digital
 * Equipment Corporation.
 * 
 * 2.  DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY DIGITAL "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.IN NO EVENT SHALL DIGITAL BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Above copyright left intact, because code originated with "x2x". */

#define XLIB_ILLEGAL_ACCESS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>			/* for MAXHOSTNAMELEN */
#include <sys/types.h>			/* for select */
#include <sys/time.h>			/* for select */

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>			/* for selection */

#include "format.h"
/* ------------------------------------------------------------------------- */
/* Not defined in header files... */
extern Bool     XTestQueryExtension();
extern void     XTestGrabControl();
extern void     XTestFakeMotionEvent();
extern void     XTestFakeButtonEvent();
extern void     XTestFakeKeyEvent();
/* ------------------------------------------------------------------------- */
/* functions */
static Bool     ProcessMotionNotify();
static Bool     ProcessExpose();
static Bool     ProcessEnterNotify();
static Bool     ProcessButtonPress();
static Bool     ProcessButtonRelease();
static Bool     ProcessKeyEvent();
static Bool     ProcessConfigureNotify();
static Bool     ProcessClientMessage();
static Bool     ProcessSelectionRequest();
static Bool     ProcessPropertyNotify();
static Bool     ProcessSelectionNotify();
static Bool     ProcessSelectionClear();
static Bool     ProcessVisibility();
static Bool     ProcessMapping();
/* ------------------------------------------------------------------------- */
extern void	displayXA();
extern void	displayit();
/* ------------------------------------------------------------------------- */
/* text formatting instructions */
static Format   toDpyFormat[] = {
  FormatMeasureText,
  FormatSetLeft, 0,
  FormatSetTop, 0,
  FormatAddHalfTextX, 1,
  FormatAddHalfTextY, 3,
  FormatMultiLine, (Format) "unknown",		/* was FormatString */
  FormatAddHalfTextX, 1,
  FormatAddHalfTextY, 1
};

#define toDpyFormatLength (sizeof(toDpyFormat) / sizeof(Format))
/* ------------------------------------------------------------------------- */
/* indexes of values to be filled in at runtime */
#define toDpyLeftIndex    2
#define toDpyTopIndex     4
#define toDpyStringIndex 10
/* ------------------------------------------------------------------------- */
/* stuff for selection forwarding */
struct _dpyxtra {
/*   Display        *otherDpy; */
  int             sState;
  Atom            dpypAtom;
  Bool            pingInProg;
  Window          propWin;
/* selection variables (cut&paste) */
  Time            sTime;
  Display        *sDpy;
  XSelectionRequestEvent sEvt;
};

/* ------------------------------------------------------------------------- */
/* structures for recording state of buttons and keys */
struct _fakestr {
  struct _fakestr *pNext;
  int             type;
  unsigned int    thing;
};

#define FAKE_KEY    0
#define FAKE_BUTTON 1

#define N_BUTTONS   5

/* values for sState */
#define SELSTATE_OFF    0
#define SELSTATE_WAIT   1
#define SELSTATE_ON     2

/* State of connection to other displays */
#define XMANY_DISCONNECTED    0
#define XMANY_AWAIT_RELEASE   1
#define XMANY_CONNECTED       2
#define XMANY_CONN_RELEASE    3

/* special values for translated coordinates */
#define COORD_INCR     -1
#define COORD_DECR     -2
#define SPECIAL_COORD(COORD) (((COORD) < 0) ? (COORD) : 0)

/* ------------------------------------------------------------------------- */
struct trigger {
  Window          triggerwindow;
  int             MoveToX;
  int             MoveToY;
  Cursor          grabCursor;
};

#define	TRIGGER_TEXT	0
#define	TRIGGER_NORTH	1
#define	TRIGGER_SOUTH	2
#define	TRIGGER_EAST	3
#define	TRIGGER_WEST	4
#define TRIGGER_NUMBER	5

static unsigned char wrapdirection[TRIGGER_NUMBER] = {
  0, 0, 0, 1, 1				/* wrap east and west */
};

static int wrapdirections = 0;		/* number of direction trigger windows */

static char    *trigger_which[TRIGGER_NUMBER] = {
  "TRIGGER_TEXT",
  "TRIGGER_NORTH",
  "TRIGGER_SOUTH",
  "TRIGGER_EAST",
  "TRIGGER_WEST"
};

/* ------------------------------------------------------------------------- */
/* display information */
static struct {
/* stuff on "from" display */
  Window          root;
/* special things needed for each window. */
  int	 trigger_cnt;
  struct trigger  **trigger;		/* [trigger_cnt][TRIGGER_NUMBER] */
  int		  *trigger_screen;

/* unknown if unique or window specific */

/* selection forwarding info */
  struct _dpyxtra fromDpyXtra;
  struct _dpyxtra *screenDpyXtra;	/* screenDpyXtra[on_screen] */
  Display        *lastSelectDpy;
/* These are set when window notification (change) prompts it. */
  int             MoveToX;
  int             MoveToY;
  int             eventmask_trigger;
  int             eventmask_normal;
  int             eventmask_text;
  Window          windowfrom;
  Cursor          grabcursor;
/* unique things */
  int             on_to_screen;		/* current screen on, ordinal. */
  GC              textGC;
  Atom            wmpAtom;
  Atom            wmdwAtom;
  XFS            *font;
  int             twidth;		/* text window width */
  int             theight;		/* text window height */
  int             unreasonableDelta;
  unsigned int    inverseMap[N_BUTTONS + 1];	/* inverse of button mapping */
  int             mode;			/* state of connection */
  int             lastFromX;
  int             lastFromY;
  struct _fakestr *pFakeThings;		/* state of buttons and keys */
/* coordinate conversion stuff */
  int            *screen_nums;		/* screen number on device. */
  short         **xTables;		/* precalculated conversion tables */
  short         **yTables;
}               dpyInfo;

/* ------------------------------------------------------------------------- */
typedef int     (*HANDLER) ();		/* event handler function */

/* ------------------------------------------------------------------------- */
struct machine_list {
  char *name;
  struct machine_list *next;
  Window          propWin;		/* only one needed per machine. */
};

static struct machine_list *machine_list_first = NULL;
/* ------------------------------------------------------------------------- */
/* top-level variables */
static char    *programStr = "xmany";

static Display *fromDpy;

/* -------------------------------------------------- */
static char **display_to_names = NULL;		/* array of names */
static int   *display_is_from = NULL;		/* array of true/false */
static int      number_to = 0;
static Display **c_tDisplay;
/* -------------------------------------------------- */

static char    *defaultFN = "-*-times-bold-r-*-*-*-180-*-*-*-*-*-*";

static char    *pingStr = "PING";	/* atom for ping request */
static char    *geomStr = NULL;
static char     StringToPrint[BUFSIZ];

/* Set from command line options */
static char    *from_name = NULL;
static char    *from_host = NULL;
static int      from_host_lth = 0;
static char    *fontName = "-*-times-bold-r-*-*-*-180-*-*-*-*-*-*";
static Bool     opt_resurface = False;
static Bool     opt_wait = False;
static int      opt_triggerwidth = 2;
static int      opt_debug = 0;
/* Below are set with new -f file (parse_file) processing */
static int geom_x = 0;			/* geometry x & y */
static int geom_y = 0;

/* ------------------------------------------------------------------------- */
static int	t_w[TRIGGER_NUMBER];	/* set wrap directions allowed */

/* ------------------------------------------------------------------------- */
static char     debugstring[BUFSIZ];
static char  *event_type[LASTEvent] = {
  "Unknown 0",
  "Unknown 1",
  "KeyPress",
  "KeyRelease",
  "ButtonPress",
  "ButtonRelease",
  "MotionNotify",
  "EnterNotify",
  "LeaveNotify",
  "FocusIn",
  "FocusOut",
  "KeymapNotify",
  "Expose",
  "GraphicsExpose",
  "NoExpose",
  "VisibilityNotify",
  "CreateNotify",
  "DestroyNotify",
  "UnmapNotify",
  "MapNotify",
  "MapRequest",
  "ReparentNotify",
  "ConfigureNotify",
  "ConfigureRequest",
  "GravityNotify",
  "ResizeRequest",
  "CirculateNotify",
  "CirculateRequest",
  "PropertyNotify",
  "SelectionClear",
  "SelectionRequest",
  "SelectionNotify",
  "ColormapNotify",
  "ClientMessage",
  "MappingNotify"
};

/* ------------------------------------------------------------------------- */
static void     dpy_xtra(i, it)
int i;
struct _dpyxtra *it;
{
  if (opt_debug) {
    (void) fprintf(stderr, "  dpy_xtra(%d): sDpy(%d) propWin(%ld) pingInProg(%d) sState(%d) sTime(%ld)\n",
	i, (int)(it->sDpy), it->propWin, it->pingInProg, it->sState, it->sTime);
  }
}					/* dpy_xtra */

/* ------------------------------------------------------------------------- */
static void     display_dpyxtra()
{
  int		  i;

  (void) fprintf(stderr, "  dpyInfo.lastSelectDpy(%d)\n", (int)(dpyInfo.lastSelectDpy));
  dpy_xtra(-1, &(dpyInfo.fromDpyXtra));
  for (i = 0; i <  number_to; i++) {
    dpy_xtra(i, &(dpyInfo.screenDpyXtra[i]));
  }
}					/* display_dpyxtra */

/* ------------------------------------------------------------------------- */
void     displaydisplay(str, d)
char           *str;
Display        *d;
{
  int             cnt = 0;
  int		  i;

  if (opt_debug) {
    (void) fprintf(stderr, "%s (%d)", str, (int)d);
    if (d == fromDpy) {
      (void) fprintf(stderr, "fromDpy ");
      cnt++;
    }
    for (i = 0; i <  number_to; i++) {
      if (d == c_tDisplay[i]) {
	(void) fprintf(stderr, "display[%d](%s) ", i, display_to_names[i]);
	cnt++;
      }
    }
    if (cnt == 0) {
      (void) fprintf(stderr, "UNKNOWN");
    }
    (void) fprintf(stderr, "\n");
  }
}					/* displaydisplay */

/* ------------------------------------------------------------------------- */
void     displaywindow(str, w)
char           *str;
Window          w;
{
  int             cnt = 0;
  int             i;
  int             j;
  int		  l;
  struct machine_list *ml;

  if (opt_debug) {
    (void) fprintf(stderr, "%s (%ld)", str, w);
    if (w == dpyInfo.root) {
      (void) fprintf(stderr, "dpyInfo.root ");
      cnt++;
    }
    if (dpyInfo.trigger_cnt == 0) {
      l = 1;
    } else {
      l = dpyInfo.trigger_cnt;
    }
    for (j = 0; j < l; j++) {
      for (i = 0; i < TRIGGER_NUMBER; i++) {
	if (w == dpyInfo.trigger[j][i].triggerwindow) {
	  (void) fprintf(stderr, "%d:%s ", j, trigger_which[i]);
	  cnt++;
	}
      }
    }
    if (w == dpyInfo.windowfrom) {
      (void) fprintf(stderr, "dpyInfo.windowfrom ");
      cnt++;
    }
    if (w == dpyInfo.fromDpyXtra.propWin) {
      (void) fprintf(stderr, "dpyInfo.fromDpyXtra.propWin ");
      cnt++;
    }
    for (i = 0; i < number_to; i++) {
      if (display_is_from[i] != 1) {
	if (w == dpyInfo.screenDpyXtra[i].propWin) {
	  (void) fprintf(stderr, "dpyInfo.screenDpyXtra[%d].propWin ", i);
	  cnt++; 
	}
      }
    }
    for (ml = machine_list_first; ml != NULL; ml = ml->next) {
      if (ml->propWin == w) {
	(void) fprintf(stderr, "ml(%s) ", ml->name);
	cnt++; 
      }
    }
    if (cnt == 0) {
      (void) fprintf(stderr, "UNKNOWN");
    }
    (void) fprintf(stderr, "\n");
  }
}					/* displaywindow */

/* ------------------------------------------------------------------------- */
static void     Usage()
{
  printf("Usage: xmany [-from <DISPLAY>] -f file [options options]\n");
  printf("\t-from <DISPLAY>		keyboard and mouse control from\n");
  printf("\t-f file			parse file for layout of displays\n");
  printf("\t-font <FONTNAME>		font to use for text window\n");
  printf("\t-geometry <GEOMETRY>	geometry of text window\n");
  printf("\t-wait			wait for displays to become active\n");
  printf("\t-resurface			if must force resurface of displays\n");
  printf("\t-debug			print out debugging messages\n");
  exit(9);
}					/* Usage */

/* ------------------------------------------------------------------------- */
static void parse_file(str)
char *str;
{
  FILE	*f_f;				/* -f file */
  char buf[BUFSIZ];
  int l;
  char *p;
  char *x;
  int i;
  int first;
  int second;
  int linecnt = 0;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "parse_file\n"); */
/* #endif */
  if ((f_f = fopen(str, "r")) == NULL) {
    (void) perror("can not open file");
    exit(30);
  }
  while (fgets(buf, BUFSIZ, f_f) != NULL) {
    linecnt++;
    l = strlen(buf);
    for (i = 0; i < l; i++) {
      if (buf[i] == '#') {		/* comment */
	buf[i] = '\0';
	l = i;
	break;
      }
    }
    while (l > 0) {
/* toss trailing new-line, return, space, tab */
      if (buf[l-1] == '\n' || buf[l-1] == '\r' ||
	  buf[l-1] == ' ' || buf[l-1] == '\t') {
	buf[l-1] = '\0';
	l--;
	continue;
      }
      break;
    }
    if (l == 0) {
      continue;				/* blank line. */
    }
    if (l < 7) {
      (void) fprintf(stderr, "parsing file %s/%d too short (%s)\n",
			str, linecnt, buf);
      continue;
    }
    for (i = 0; i < l; i++) {
      if (buf[i] == ' ' || buf[i] == '\t') {	/* white space */
	buf[i] = '\0';
	i++;
	break;
      }
    }
    if (buf[i-1] != '\0') {
      (void) fprintf(stderr, "parsing file %s/%d: line did not have two arguments\n", str, linecnt);
      (void) fprintf(stderr, "  line=(%s)\n",buf);
      continue;
    }
    p = buf + i;
    while (*p == ' ' || *p == '\t') {	/* delete white space before arg */
      p++;
    }
/* ---------- */
    if (strcmp(buf, "-from") == 0) {
      if (from_name != NULL) {
	(void) fprintf(stderr, "parsing file %s/%d: -from already specified (%s)  new (%s)\n",
				str, linecnt, from_name, p);
	exit(31);
      }
      from_name = strdup(p);
      if (from_name == NULL) {
	(void) fprintf(stderr, "parsing file %s/%d: strdup failed\n", str, linecnt);
	exit(32);
      }
      continue;
    }
/* ---------- */
    if (strcmp(buf, "-geom") == 0) {
      if (geom_x != 0 || geom_y != 0) {
	(void) fprintf(stderr, "parsing file %s/%d: -geom already specified (%dx%d)  new (%s)\n",
				str, linecnt, geom_x, geom_y, p);
	exit(31);
      }
      x = index(p, 'x');
      if (x == NULL) {
	(void) fprintf(stderr, "parsing file %s/%d: -geom (%s) does not have x\n", str, linecnt, p);
	exit(33);
      }
      *x = '\0';
      x++;
      geom_x = strtol(p, (char **)NULL, 10);
      geom_y = strtol(x, (char **)NULL, 10);
      if (geom_x <= 0 || geom_x > 10 ||
	  geom_y <= 0 || geom_y > 10) {
	(void) fprintf(stderr, "parsingfile %s/%d: -geom (%sx%s) invalid (%dx%d)\n",
				str, linecnt, p, x, geom_x, geom_y);
	exit(34);
      }
      number_to = geom_x * geom_y;
      display_to_names = (char **) malloc(sizeof(char *) * number_to);
      for (i = 0; i <  number_to; i++) {
	display_to_names[i] = NULL;		/* initialize to nothing */
      }
      continue;
    }
/* ---------- */
    if (strcmp(buf, "-wrap") == 0) {
      while (*p != '\0') {
	switch (*p) {
	  case 'n':
	    wrapdirection[TRIGGER_NORTH] = 1;
	    break;
	  case 's':
	    wrapdirection[TRIGGER_SOUTH] = 1;
	    break;
	  case 'e':
	    wrapdirection[TRIGGER_EAST] = 1;
	    break;
	  case 'w':
	    wrapdirection[TRIGGER_WEST] = 1;
	    break;
	  default:
	    (void) fprintf(stderr, "parsingfile %s/%d: -wrap invalid character (%c)\n",
				    str, linecnt, *p);
	}
	p++;
      }
      continue;
    }
/* ---------- */
    if (strcmp(buf, "-nowrap") == 0) {
      while (*p != '\0') {
	switch (*p) {
	  case 'n':
	    wrapdirection[TRIGGER_NORTH] = 0;
	    break;
	  case 's':
	    wrapdirection[TRIGGER_SOUTH] = 0;
	    break;
	  case 'e':
	    wrapdirection[TRIGGER_EAST] = 0;
	    break;
	  case 'w':
	    wrapdirection[TRIGGER_WEST] = 0;
	    break;
	  default:
	    (void) fprintf(stderr, "parsingfile %s/%d: -nowrap invalid character (%c)\n",
				    str, linecnt, *p);
	}
	p++;
      }
      continue;
    }
/* ---------- */
    if (isdigit(buf[0]) != 0) {
      x = index(buf, ',');
      if (x == NULL) {
	(void) fprintf(stderr, "parsing file %s/%d: (%s) (%s) does not have ,\n",
				str, linecnt, buf, p);
	exit(35);
      }
      *x = '\0';
      x++;
      first = strtol(buf, (char **)NULL, 10);
      second = strtol(x, (char **)NULL, 10);
      if (first < 0 || first >= geom_x ||
	  second < 0 || second >= geom_y) {
	(void) fprintf(stderr, "parsing file %s/%d: -geom (%dx%d) incompatible with (%d,%d)\n",
				str, linecnt, geom_x, geom_y, first, second);
	exit(36);
      }
      i = first + (second * geom_x);
      if (display_to_names[i] != NULL) {
	(void) fprintf(stderr, "parsing file %s/%d: (%d,%d) already defined as (%s)\n",
				str, linecnt, first, second, display_to_names[i]);
	exit(37);
      }
      display_to_names[i] = strdup(p);
      continue;
    }
/* ---------- */
    (void) fprintf(stderr, "parsing file %s/%d: unrecognized line (%s) (%s)\n", str, linecnt, buf, p);
    exit(39);
  }
  if (display_to_names == NULL) {
    (void) fprintf(stderr, "parsing file %s/%d: Did not get a -geom\n", str, linecnt);
    exit(40);
  }
  for (i = 0; i <  number_to; i++) {
    if (display_to_names[i] == NULL) {
      (void) fprintf(stderr, "parsing file %s/%d: Did not get a definition for %d,%d\n",
			      str, linecnt, i % geom_x, i / geom_x);
      exit(41);
    }
  }
/*   if (opt_debug != 0) { */
/*     (void) fprintf(stderr, "wrapping nsew = %d %d %d %d\n", */
/* 	wrapdirection[TRIGGER_NORTH], wrapdirection[TRIGGER_SOUTH], */
/* 	wrapdirection[TRIGGER_EAST], wrapdirection[TRIGGER_WEST]); */
/*   } */
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting parse_file\n\n"); */
/* #endif */
}					/* end of parse_file */

/* ------------------------------------------------------------------------- */
/* use standard X functions to parse the command line */
static char     defaulthostname[MAXHOSTNAMELEN + 1];


static void     ParseCommandLine(argc, argv)
int             argc;
char          **argv;
{
  int             arg;
  char           *displayenv;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "ParseCommandLine\n"); */
/* #endif */
  for (arg = 1; arg < argc; ++arg) {
    if (strcasecmp(argv[arg], "-from") == 0) {
      if (++arg >= argc || from_name != NULL) {
	(void) fprintf(stderr, "no argument after -from\n");
	Usage();
      }
      from_name = argv[arg];
    } else if (strcasecmp(argv[arg], "-f") == 0) {
      if (++arg >= argc) {
	(void) fprintf(stderr, "no argument after -f\n");
	Usage();
      }
      parse_file(argv[arg]);
    } else if (strcasecmp(argv[arg], "-debug") == 0) {
      opt_debug++;
    } else if (strcasecmp(argv[arg], "-font") == 0) {
      if (++arg >= argc) {
	(void) fprintf(stderr, "no argument after -font\n");
	Usage();
      }
      fontName = argv[arg];
    } else if (strcasecmp(argv[arg], "-geometry") == 0) {
      if (++arg >= argc || geomStr != NULL) {
	(void) fprintf(stderr, "no argument after -geometry\n");
	Usage();
      }
      geomStr = argv[arg];
    } else if (strcasecmp(argv[arg], "-wait") == 0) {
      opt_wait = True;
    } else if (strcasecmp(argv[arg], "-resurface") == 0) {
      opt_resurface = True;
    } else if (strcasecmp(argv[arg], "-triggerw") == 0 ||
	       strcasecmp(argv[arg], "-triggerwidth") == 0) {
      if (++arg >= argc) {
	(void) fprintf(stderr, "no argument after -triggerwidth\n");
	Usage();
      }
      opt_triggerwidth = atoi(argv[arg]);
    } else {
      (void) fprintf(stderr, "unknown argument (%s)\n", argv[arg]);
      Usage();
    }
  }
  if (from_name == NULL) {
    displayenv = getenv("DISPLAY");	/* if set, use display */
    if (displayenv == NULL) {
      if (gethostname(defaulthostname, sizeof(defaulthostname)) != 0) {
	perror("gethostname:");
	exit(6);
      }
      (void) strlcat(defaulthostname, ":0.0", sizeof(defaulthostname));
      displayenv = defaulthostname;
    }
    from_name = displayenv;		/* default from us. */
  }
  if (display_to_names == NULL) {
    (void) fprintf(stderr, "Must have at least one display to connect to.\n");
    exit(7);
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ParseCommandLine\n\n"); */
/* #endif */
}					/* ParseCommandLine */

/* ------------------------------------------------------------------------- */
/* call the library to check for the test extension */

static Bool     CheckTestExtension(dpy)
Display        *dpy;
{
  int             eventb;
  int             errorb;
  int             vmajor;
  int             vminor;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "CheckTestExtension\n"); */
/* #endif */
  return(XTestQueryExtension(dpy, &eventb, &errorb, &vmajor, &vminor));
}					/* CheckTestExtension */

/* ------------------------------------------------------------------------- */
static void     RefreshPointerMapping(dpy)
Display        *dpy;
{
  int             buttCtr;
  unsigned char   buttonMap[N_BUTTONS];
  int             nButtons;

#ifdef DEBUG_ENTRY
(void) fprintf(stderr, "RefreshPointerMapping\n");
#endif
  if (dpy == c_tDisplay[dpyInfo.on_to_screen]) {/* only care about current to display */
/* straightforward mapping */
    for (buttCtr = 1; buttCtr <= N_BUTTONS; ++buttCtr) {
      dpyInfo.inverseMap[buttCtr] = buttCtr;
    }
    nButtons = MIN(N_BUTTONS, XGetPointerMapping(dpy, buttonMap, N_BUTTONS));
    for (buttCtr = 0; buttCtr < nButtons; ++buttCtr) {
      if (buttonMap[buttCtr] <= N_BUTTONS) {
	dpyInfo.inverseMap[buttonMap[buttCtr]] = buttCtr + 1;
      }
    }
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting RefreshPointerMapping\n\n"); */
/* #endif */
}					/* RefreshPointerMapping */

/* ------------------------------------------------------------------------- */
static void set_t_w(screen_cnt)
int screen_cnt;
{
/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "set_t_w\n"); */
/* #endif */
  if (screen_cnt < geom_x && 				/* at top */
       wrapdirection[TRIGGER_NORTH] == 0) {
    t_w[TRIGGER_NORTH] = 1;		/* Do not wrap. */
  } else {
    t_w[TRIGGER_NORTH] = 0;		/* wrap. */
  }
  if (screen_cnt >= ((geom_y - 1) * geom_x) && 	/* at bottom */
       wrapdirection[TRIGGER_SOUTH] == 0) {
    t_w[TRIGGER_SOUTH] = 1;		/* Do not wrap. */
  } else {
    t_w[TRIGGER_SOUTH] = 0;		/* wrap. */
  }
  if ((screen_cnt % geom_x) == (geom_x - 1) && 	/* at right */
    wrapdirection[TRIGGER_EAST] == 0) {
    t_w[TRIGGER_EAST] = 1;		/* Do not wrap. */
  } else {
    t_w[TRIGGER_EAST] = 0;		/* wrap. */
  }
  if ((screen_cnt % geom_x) == 0 &&			/* at left */ 
       wrapdirection[TRIGGER_WEST] == 0) {
    t_w[TRIGGER_WEST] = 1;		/* Do not wrap. */
  } else {
    t_w[TRIGGER_WEST] = 0;		/* wrap. */
  }
/*   if (opt_debug != 0) { */
/*     (void) fprintf(stderr, "set_t_w: nsew (screen_cnt=%d) = %d %d %d %d\n", */
/* 			    screen_cnt, */
/* 			    t_w[TRIGGER_NORTH], t_w[TRIGGER_SOUTH], */
/* 			    t_w[TRIGGER_EAST], t_w[TRIGGER_WEST]); */
/*   } */
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting set_t_w\n\n"); */
/* #endif */
}			/* end of set_t_w */

/* ------------------------------------------------------------------------- */
static void     InitDpyInfo()
{
  Screen         *fromScreen;
  long            black;
  long            white;
  int             fromHeight;
  int             fromWidth;
  int             toHeight;
  int             toWidth;
  Window          rret;
  Window          propWin;
  short          *xTable;
  short          *yTable;
  int             counter;
  int             twidth = 100;
  int             theight = 100;	/* default text dimensions */
  int             xoff;
  int             yoff;			/* window offsets */
  unsigned int    width;
  unsigned int    height;		/* window width, height */
  int             geomMask;		/* mask returned by parse */
  int             gravMask;
  int             xret;
  int             yret;
  unsigned int    wret;
  unsigned int    hret;
  unsigned int    bret;
  unsigned int    dret;
  XSetWindowAttributes xswa;
  XSizeHints     *xsh;
  int             eventmask;
  char           *windowName;
  int             tmp;
  int             screen_cnt;
  Pixmap          nullPixmap;
  XColor          dummyColor;
  int             i;
  int             j;
  int		  l;
  int		  done_from_once = 0;	/* NOT DONE YET -- was 0 */
  int found;
  char *dindex;
  int dlth;
  int mlth;
  struct machine_list *ml;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "InitDpyInfo\n"); */
/* #endif */
/* Initialize From state. */
  fromScreen = XDefaultScreenOfDisplay(fromDpy);
  black = XBlackPixelOfScreen(fromScreen);
  white = XWhitePixelOfScreen(fromScreen);
  fromHeight = XHeightOfScreen(fromScreen);
  fromWidth = XWidthOfScreen(fromScreen);
  dpyInfo.root = XDefaultRootWindow(fromDpy);
  dpyInfo.mode = XMANY_DISCONNECTED;	/* not in any "to" display yet. */
  dpyInfo.unreasonableDelta = fromWidth / 2;	/* only can go .5 of distance */
  dpyInfo.pFakeThings = NULL;
/* window init structures */
  xsh = XAllocSizeHints();
  dpyInfo.wmpAtom = XInternAtom(fromDpy, "WM_PROTOCOLS", True);
  dpyInfo.wmdwAtom = XInternAtom(fromDpy, "WM_DELETE_WINDOW", True);
  dpyInfo.font = NULL;
/* Set to number of displays (monitors) */
  if (number_to < 2 || dpyInfo.trigger_cnt == 0) {
    for (i = 0; i < TRIGGER_NUMBER; i++) {
      wrapdirections += wrapdirection[i];
    }
  } else {
    wrapdirections = 1;			/* will always wrap between screens */
  }
  if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
    l = 1;
  } else {
    l = dpyInfo.trigger_cnt;
  }
  dpyInfo.trigger = (struct trigger **) malloc(sizeof(struct trigger *) * l);
  dpyInfo.trigger_screen = (int *) malloc(sizeof(int) * l);
  for (j = 0; j < l; j++) {
    dpyInfo.trigger_screen[j] = -1;	/* for from_screen, where are we. */
    dpyInfo.trigger[j] = (struct trigger *) 
		    malloc(sizeof(struct trigger) * TRIGGER_NUMBER);
    for (i = 0; i < TRIGGER_NUMBER; i++)  {
      dpyInfo.trigger[j][i].triggerwindow = (unsigned int)(-1);
    }
  }
  eventmask = KeyPressMask | KeyReleaseMask | PropertyChangeMask;
  if (opt_resurface == True) {			/* get visibility events */
    eventmask |= VisibilityChangeMask;
  }
  dpyInfo.eventmask_normal = eventmask;
  dpyInfo.eventmask_trigger = eventmask | EnterWindowMask;
  dpyInfo.windowfrom = NULL;			/* Set to none. */
  if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) { /* force text window */
/* Normal window for text: do size grovelling */
    dpyInfo.trigger[0][TRIGGER_TEXT].grabCursor = XCreateFontCursor(fromDpy, XC_exchange);
    eventmask |= StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask;
    dpyInfo.eventmask_text = eventmask;
/* Determine size of text for small window */
    if (((dpyInfo.font = XLoadQueryFont(fromDpy, fontName)) != NULL) ||
	((dpyInfo.font = XLoadQueryFont(fromDpy, defaultFN)) != NULL) ||
	((dpyInfo.font = XLoadQueryFont(fromDpy, "fixed")) != NULL)) {
/* Have a font */
      StringToPrint[0] = '\0';
      (void) strlcpy(StringToPrint, "  ", sizeof(StringToPrint));
      (void) strlcat(StringToPrint, display_to_names[0], sizeof(StringToPrint));
      for (i = 1; i <  number_to; i++) {
	if ((i % geom_x) == 0) {
	  (void) strlcat(StringToPrint, "\n  ", sizeof(StringToPrint));
	} else {
	  (void) strlcat(StringToPrint, "  ", sizeof(StringToPrint));
	}
	display_to_names[i] = XDisplayName(display_to_names[i]);
	(void) strlcat(StringToPrint, display_to_names[i], sizeof(StringToPrint)); }
      toDpyFormat[toDpyStringIndex] = (Format) StringToPrint;
      formatText(NULL, (Window)NULL, NULL, dpyInfo.font, toDpyFormatLength, toDpyFormat,
		 &twidth, &theight);
      dpyInfo.textGC = XCreateGC(fromDpy, dpyInfo.root, 0, NULL);
      XSetState(fromDpy, dpyInfo.textGC, black, white, GXcopy, AllPlanes);
      XSetFont(fromDpy, dpyInfo.textGC, dpyInfo.font->fid);
      dpyInfo.trigger[0][TRIGGER_TEXT].MoveToX = -1;	/* special, means leave alone */
      dpyInfo.trigger[0][TRIGGER_TEXT].MoveToY = -1;	/* special, means leave alone */
    }
/* Determine size of window */
    xoff = yoff = 0;
    width = twidth;
    height = theight;
    geomMask = XParseGeometry(geomStr, &xoff, &yoff, &width, &height);
    switch (gravMask = (geomMask & (XNegative | YNegative))) {
      case (XNegative | YNegative):
	xsh->win_gravity = SouthEastGravity;
	break;
      case XNegative:
	xsh->win_gravity = NorthEastGravity;
	break;
      case YNegative:
	xsh->win_gravity = SouthWestGravity;
	break;
      default:
	xsh->win_gravity = NorthWestGravity;
	break;
    }
    if (gravMask) {
      XGetGeometry(fromDpy, dpyInfo.root, &rret, &xret, &yret, &wret,
		   &hret, &bret, &dret);
      if ((geomMask & (XValue | XNegative)) == (XValue | XNegative)) {
	xoff = wret - width + xoff;
      }
      if ((geomMask & (YValue | YNegative)) == (YValue | YNegative)) {
	yoff = hret - height + yoff;
      }
    }
    if (opt_debug != 0) {
      (void) fprintf(stderr, "InitDpyInfo textwindow(%d,%d)\n", (int)fromDpy, (int)dpyInfo.root);
    }
    dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow = XCreateSimpleWindow(fromDpy, dpyInfo.root, xoff, yoff,
								      width, height, 0, black, white);
    xsh->x = xoff;
    xsh->y = yoff;
    xsh->base_width = width;
    xsh->base_height = height;
    xsh->flags = (PPosition | PBaseSize | PWinGravity);
    XSetWMNormalHints(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow, xsh);
    windowName = (char *) malloc(strlen(programStr) + from_host_lth - 1 + 2);
    strcpy(windowName, programStr);
    strcat(windowName, " ");
    strncat(windowName, from_host, from_host_lth - 1);
    XStoreName(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow, windowName);
    XSetIconName(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow, windowName);
    free(windowName);
    XSetWMProtocols(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow, &(dpyInfo.wmdwAtom), 1);
    if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
      i = TRIGGER_TEXT;
    } else {
      (void) fprintf(stderr, "Should not be here.\n");
      i = -1;
      exit(10);
    }
    dpyInfo.windowfrom = dpyInfo.trigger[0][i].triggerwindow;
    dpyInfo.MoveToX = dpyInfo.trigger[0][i].MoveToX;
    dpyInfo.MoveToY = dpyInfo.trigger[0][i].MoveToY;
    dpyInfo.grabcursor = dpyInfo.trigger[0][i].grabCursor;
  }
  if (dpyInfo.font) {			/* paint text */
/* position text */
    dpyInfo.twidth = twidth;
    dpyInfo.theight = theight;
    tmp = (width - twidth) / 2;
    toDpyFormat[toDpyLeftIndex] = MAX(0, tmp);
    tmp = (height - theight) / 2;
    toDpyFormat[toDpyTopIndex] = MAX(0, tmp);
    formatText(fromDpy, dpyInfo.windowfrom, &(dpyInfo.textGC), dpyInfo.font,
	       toDpyFormatLength, toDpyFormat, NULL, NULL);
  }
/* Set up arrays for number of "to" screens. */
  dpyInfo.screenDpyXtra = (struct _dpyxtra *) malloc(sizeof(struct _dpyxtra) * number_to);
  dpyInfo.screen_nums = (int *) malloc(sizeof(int) * number_to);
  dpyInfo.xTables = (short **) malloc(sizeof(short *) * number_to);
  dpyInfo.yTables = (short **) malloc(sizeof(short *) * number_to);
  j = 0;
  xsh->flags = (PPosition | PBaseSize | PWinGravity);
  nullPixmap = XCreatePixmap(fromDpy, dpyInfo.root, 1, 1, 1);
  xswa.background_pixel = black;
  xswa.override_redirect = True;
  windowName = (char *) malloc(strlen(programStr) + 1);
  strcpy(windowName, programStr);
  dpyInfo.lastSelectDpy = fromDpy;	/* last selection from key/mouse. */
  for (screen_cnt = 0; screen_cnt < number_to; screen_cnt++) {
    set_t_w(screen_cnt);
    if (display_is_from[screen_cnt] == 1) {	/* if on same machine */
      dpyInfo.trigger_screen[j] = screen_cnt;
/* Initialize various windows */
      if (t_w[TRIGGER_EAST] == 0) {		/* east */
	l = TRIGGER_EAST;
	dpyInfo.trigger[j][l].MoveToX = opt_triggerwidth + 1;
	dpyInfo.trigger[j][l].MoveToY = -1;	/* leave alone */
	dpyInfo.trigger[j][l].triggerwindow = XCreateWindow(fromDpy, dpyInfo.root,
	    fromWidth - opt_triggerwidth, 0, opt_triggerwidth, fromHeight, 0, 0,
	    InputOutput, 0, CWBackPixel | CWOverrideRedirect, &xswa);
	if (dpyInfo.windowfrom == NULL) {
	  dpyInfo.windowfrom = dpyInfo.trigger[j][l].triggerwindow;
	}
      }
      if (t_w[TRIGGER_WEST] == 0) {		/* west */
	l = TRIGGER_WEST;
	dpyInfo.trigger[j][l].MoveToX = fromWidth - opt_triggerwidth - 1;
	dpyInfo.trigger[j][l].MoveToY = -1;	/* special, means leave alone */
	dpyInfo.trigger[j][l].triggerwindow = XCreateWindow(fromDpy, dpyInfo.root,
	    0, 0, opt_triggerwidth, fromHeight, 0, 0,
	    InputOutput, 0, CWBackPixel | CWOverrideRedirect, &xswa);
	if (dpyInfo.windowfrom == NULL) {
	  dpyInfo.windowfrom = dpyInfo.trigger[j][l].triggerwindow;
	}
      }
      if (t_w[TRIGGER_NORTH] == 0) {		/* north */
	l = TRIGGER_NORTH;
	dpyInfo.trigger[j][l].MoveToX = -1;	/* special, -1 means leave */
	dpyInfo.trigger[j][l].MoveToY = fromHeight - opt_triggerwidth - 1;
	dpyInfo.trigger[j][l].triggerwindow = XCreateWindow(fromDpy, dpyInfo.root,
	    0, 0, fromWidth, opt_triggerwidth, 0, 0,
	    InputOutput, 0, CWBackPixel | CWOverrideRedirect, &xswa);
	if (dpyInfo.windowfrom == NULL) {
	  dpyInfo.windowfrom = dpyInfo.trigger[j][l].triggerwindow;
	}
      }
      if (t_w[TRIGGER_SOUTH] == 0) {		/* south */
	l = TRIGGER_SOUTH;
	dpyInfo.trigger[j][l].MoveToX = -1;	/* special, -1 means leave */
	dpyInfo.trigger[j][l].MoveToY = opt_triggerwidth + 1;
	dpyInfo.trigger[j][l].triggerwindow = XCreateWindow(fromDpy, dpyInfo.root,
	    0, fromHeight - opt_triggerwidth, fromWidth, opt_triggerwidth, 0, 0,
	    InputOutput, 0, CWBackPixel | CWOverrideRedirect, &xswa);
	if (dpyInfo.windowfrom == NULL) {
	  dpyInfo.windowfrom = dpyInfo.trigger[j][l].triggerwindow;
	}
      }
/* all the 4 sides */
      for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
	if (dpyInfo.trigger[j][l].triggerwindow != (unsigned int)(-1)) {
	  dpyInfo.trigger[j][l].grabCursor = XCreatePixmapCursor(fromDpy,
		  nullPixmap, nullPixmap, &dummyColor, &dummyColor, 0, 0);
	  XSetWMNormalHints(fromDpy, dpyInfo.trigger[j][l].triggerwindow, xsh);
	  XStoreName(fromDpy, dpyInfo.trigger[j][l].triggerwindow, windowName);
	  XSetIconName(fromDpy, dpyInfo.trigger[j][l].triggerwindow, windowName);
	  XSetWMProtocols(fromDpy, dpyInfo.trigger[j][l].triggerwindow,
			  &(dpyInfo.wmdwAtom), 1);
	}
      }
    }
/* conversion stuff */
    dpyInfo.screen_nums[screen_cnt] = DefaultScreen(c_tDisplay[screen_cnt]);
/* for each screen, set up conversion for x/y screen/mouse movement */
/* construct table lookup for screen coordinate conversion */
    toWidth = XWidthOfScreen(XScreenOfDisplay(c_tDisplay[screen_cnt], dpyInfo.screen_nums[screen_cnt]));
    toHeight = XHeightOfScreen(XScreenOfDisplay(c_tDisplay[screen_cnt], dpyInfo.screen_nums[screen_cnt]));
    dpyInfo.xTables[screen_cnt] = xTable =
	(short *) malloc(sizeof(short) * fromWidth);
    dpyInfo.yTables[screen_cnt] = yTable =
	(short *) malloc(sizeof(short) * fromHeight);
    for (counter = 0; counter < fromHeight; ++counter) {
/* vertical conversion table */
      yTable[counter] = (counter * toHeight) / fromHeight;
    }
/* horizontal conversion table entries */
    for (counter = 0; counter < fromWidth; ++counter) {
      xTable[counter] = (counter * toWidth) / fromWidth;
    }
/* adjustment for boundaries */
    for (counter = 1; counter <= opt_triggerwidth; counter++) {
      if (t_w[TRIGGER_EAST] == 0) {
	xTable[fromWidth - counter] = COORD_INCR;
      }
      if (t_w[TRIGGER_WEST] == 0) {
	xTable[counter-1] = COORD_DECR;
      }
      if (t_w[TRIGGER_NORTH] == 0) {
	yTable[counter-1] = COORD_DECR;
      }
      if (t_w[TRIGGER_SOUTH] == 0) {
	yTable[fromHeight - counter] = COORD_INCR;
      }
    }
/* always create propWin for events from c_tDisplay[screen_cnt] */
    if (display_is_from[screen_cnt] != 1 || done_from_once == 0) {
      ml = NULL;
      if (display_is_from[screen_cnt] != 1) {
	dindex = index(display_to_names[screen_cnt], ':');
	if (dindex == NULL) {
	  dlth = strlen(display_to_names[screen_cnt]);
	} else {
	  dlth = dindex - display_to_names[screen_cnt];
	}
	for (ml = machine_list_first; ml != NULL; ml = ml->next) {
	  mlth = strlen(ml->name);
	  if (mlth == dlth && strncmp(display_to_names[screen_cnt], ml->name, mlth) == 0) {
	    break;
	  }
	}
	if (ml == NULL) {
	  (void) fprintf(stderr, "PROBLEM with machine_list_first (%s)\n",
		  display_to_names[screen_cnt]);
	  exit(1);
	}
	found = 0;
	if (ml->propWin == 0) {
	  if (opt_debug != 0) {
	    (void) fprintf(stderr, "InitDpyInfo[%d] XCreateWindow(%d,%d)\n", screen_cnt, (int)c_tDisplay[screen_cnt], (int)XDefaultRootWindow(c_tDisplay[screen_cnt]));
	  }
	  ml->propWin = propWin = XCreateWindow(c_tDisplay[screen_cnt],
		    XDefaultRootWindow(c_tDisplay[screen_cnt]),
		    0, 0, 1, 1, 0, 0, InputOutput, CopyFromParent, 0, NULL);
	} else {
	  found = 1;
	  propWin = ml->propWin;		/* use previous */
	}
      } else {
	found = 0;
	propWin = dpyInfo.windowfrom;		/* should be already set */
      }
      if (opt_debug != 0) {
	(void) fprintf(stderr, "found=%d, ml=%-8.8x, propWin=%d, display_is_from=%d\n", found, (int)ml, (int)propWin,display_is_from[screen_cnt]);
      }
      dpyInfo.screenDpyXtra[screen_cnt].propWin = propWin;
/* initialize pointer mapping */
      RefreshPointerMapping(c_tDisplay[screen_cnt]);
      dpyInfo.screenDpyXtra[screen_cnt].sState = SELSTATE_ON;		/* was OFF */
      dpyInfo.screenDpyXtra[screen_cnt].dpypAtom = 
			XInternAtom(c_tDisplay[screen_cnt], pingStr, False);
      dpyInfo.screenDpyXtra[screen_cnt].pingInProg = False;
      dpyInfo.screenDpyXtra[screen_cnt].sTime = 0;
      dpyInfo.screenDpyXtra[screen_cnt].sDpy = NULL;
/*       dpyInfo.screenDpyXtra[screen_cnt].sEvt = NULL; */
      if (display_is_from[screen_cnt] == 1) {
	done_from_once = 1;
	dpyInfo.screenDpyXtra[screen_cnt].sState = SELSTATE_ON;
	dpyInfo.fromDpyXtra.sState = SELSTATE_ON;
	dpyInfo.fromDpyXtra.dpypAtom = dpyInfo.screenDpyXtra[screen_cnt].dpypAtom;
	dpyInfo.fromDpyXtra.pingInProg = False;
	dpyInfo.fromDpyXtra.sTime = 0;
	dpyInfo.fromDpyXtra.sDpy = NULL;
/* 	dpyInfo.fromDpyXtra.sEvt = NULL; */
	dpyInfo.fromDpyXtra.propWin = dpyInfo.windowfrom;
/* 	dpyInfo.fromDpyXtra.propWin = propWin; */
      }
      if (found == 0) {
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "InitDpyInfo[%d] XSelectInput(%d,%d,0x%x)\n", screen_cnt, (int)c_tDisplay[screen_cnt], (int)propWin, (int)PropertyChangeMask);
	}
	XSelectInput(c_tDisplay[screen_cnt], propWin, PropertyChangeMask);
	if (display_is_from[screen_cnt] == 0) {	/* not from display, clear selection */
	  if (opt_debug != 0) {
	    (void) fprintf(stderr, "InitDpyInfo[%d] XSetSelectionOwner (%d,XA_PRIMARY,%d,YYY)\n", screen_cnt, (int)c_tDisplay[screen_cnt], (int)propWin);
	  }
	  XSetSelectionOwner(c_tDisplay[screen_cnt], XA_PRIMARY, propWin, CurrentTime);
	}
      }
    } else {				/* set to fromDpyXtra. */
      dpyInfo.screenDpyXtra[screen_cnt].propWin = dpyInfo.fromDpyXtra.propWin;
      dpyInfo.screenDpyXtra[screen_cnt].sState = SELSTATE_ON;
      dpyInfo.screenDpyXtra[screen_cnt].dpypAtom = dpyInfo.fromDpyXtra.dpypAtom;
      dpyInfo.screenDpyXtra[screen_cnt].pingInProg = False;
      dpyInfo.screenDpyXtra[screen_cnt].sTime = 0;
      dpyInfo.screenDpyXtra[screen_cnt].sDpy = NULL;
/*       dpyInfo.screenDpyXtra[screen_cnt].sEvt = NULL; */
      found = 0;
    }
    if (display_is_from[screen_cnt] == 1) {		/* if same machine */
      for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
	if (dpyInfo.trigger[j][l].triggerwindow != (unsigned int)(-1)) {
	  if (found == 0) {
	    if (opt_debug != 0) {
	      (void) fprintf(stderr, "InitDpyInfo[%d] XSelectInput (2)(fromDpy[%d],%d,0x%x)\n", screen_cnt, (int)fromDpy, (int)(dpyInfo.trigger[j][l].triggerwindow), dpyInfo.eventmask_trigger);
	    }
	    XSelectInput(fromDpy, dpyInfo.trigger[j][l].triggerwindow,
			 dpyInfo.eventmask_trigger);
	  }
	  if (opt_debug != 0) {
	    (void) fprintf(stderr, "InitDpyInfo[%d] XMapRaised (2)(fromDpy[%d],%d)\n", screen_cnt, (int)fromDpy, (int)(dpyInfo.trigger[j][l].triggerwindow));
	  }
	  XMapRaised(fromDpy, dpyInfo.trigger[j][l].triggerwindow);
	}
      }
      j++;
    }
    if (display_is_from[screen_cnt] == 0) {	/* if not on same machine */
      if (opt_debug != 0) {
	(void) fprintf(stderr, "InitDpyInfo[%d] XTestGrabControl(%d)\n", screen_cnt, (int)c_tDisplay[screen_cnt]);
      }
      XTestGrabControl(c_tDisplay[screen_cnt], True);	/* impervious to grabs */
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr, "XTestGrabControl worked\n"); */
/* #endif */
    } else {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "InitDpyInfo[%d] not doing XTestGrabControl(%d) (on same machine)\n", screen_cnt, (int)c_tDisplay[screen_cnt]);
      }
    }
  }
  if (dpyInfo.windowfrom == NULL) {
    (void) fprintf(stderr, "dpyInfo.windowfrom not set, WARNING!\n");
  }
  if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
    if (dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow != (unsigned int)(-1)) {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "InitDpyInfo[%d] XSelectInput text(fromDpy[%d],%d,0x%x)\n", screen_cnt, (int)fromDpy, (int)(dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow), dpyInfo.eventmask_text);
      }
      XSelectInput(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow,
		  dpyInfo.eventmask_text);
/* if only one window, don't bother changing selection. */
/*       XSetSelectionOwner(fromDpy, XA_PRIMARY, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow, CurrentTime); */
      XMapRaised(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow);
    }
  }
  free(windowName);
  XFree((char *) xsh);
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting InitDpyInfo\n\n"); */
/* #endif */
}					/* InitDpyInfo */

/* ------------------------------------------------------------------------- */
static void     registerevent(w)
Window          w;
{
  if (w == (Window)(-1)) {
    (void) fprintf(stderr, "registerevent: window == -1?\n");
    return;
  }
/*   if (opt_debug) { */
/*     (void) fprintf(stderr, "registerevent fromDpy %d  window=%d\n", (int)fromDpy, (int)w); */
/*   } */
  XSaveContext(fromDpy, w, MotionNotify, (XPointer) ProcessMotionNotify);
  XSaveContext(fromDpy, w, Expose, (XPointer) ProcessExpose);
  XSaveContext(fromDpy, w, EnterNotify, (XPointer) ProcessEnterNotify);
  XSaveContext(fromDpy, w, ButtonPress, (XPointer) ProcessButtonPress);
  XSaveContext(fromDpy, w, ButtonRelease, (XPointer) ProcessButtonRelease);
  XSaveContext(fromDpy, w, KeyPress, (XPointer) ProcessKeyEvent);
  XSaveContext(fromDpy, w, KeyRelease, (XPointer) ProcessKeyEvent);
  XSaveContext(fromDpy, w, ConfigureNotify, (XPointer) ProcessConfigureNotify);
  XSaveContext(fromDpy, w, ClientMessage, (XPointer) ProcessClientMessage);
/*  XSaveContext(fromDpy, w, ClientMessage, (XPointer) ProcessClientMessage); */
/*  XSaveContext(fromDpy, w, ClientMessage, (XPointer) ProcessClientMessage); */
  if (opt_resurface == True) {
    XSaveContext(fromDpy, w, VisibilityNotify, (XPointer) ProcessVisibility);
  }
  XSaveContext(fromDpy, w, SelectionRequest, (XPointer)ProcessSelectionRequest);
/*   if (opt_debug) { */
/*     (void) fprintf(stderr, "register ProcessPropertyNotify %d %d\n", (int)fromDpy, (int)w); */
/*   } */
  XSaveContext(fromDpy, w, PropertyNotify, (XPointer) ProcessPropertyNotify);
  XSaveContext(fromDpy, w, SelectionNotify, (XPointer) ProcessSelectionNotify);
  XSaveContext(fromDpy, w, SelectionClear, (XPointer) ProcessSelectionClear);
/* (void) fprintf(stderr,"XSaveContext(%d,%d,SelectionClear...\n",fromDpy,w); */
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting registerevent\n\n"); */
/* #endif */
}					/* registerevent */

/* ------------------------------------------------------------------------- */
static void     RegisterEventHandlers()
{
  int		  i;
  int		  l;
  Display         *d;
  Window          w;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "RegisterEventHandlers\n"); */
/* #endif */
  if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
    if (dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow != (unsigned int)(-1)) {
      registerevent(dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow);
    }
  } else {
    for (i = 0; i < dpyInfo.trigger_cnt; i++) {
      for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
	if (dpyInfo.trigger[i][l].triggerwindow != (unsigned int)(-1)) {
	  registerevent(dpyInfo.trigger[i][l].triggerwindow);
	}
      }
    }
  }
  for (i = 0; i < number_to; i++) {
    if (display_is_from[i] == 0) {	/* not from host */
      d = c_tDisplay[i];
      w = dpyInfo.screenDpyXtra[i].propWin;
      XSaveContext(d, None, MappingNotify, (XPointer) ProcessMapping);
      XSaveContext(d, w, SelectionRequest, (XPointer) ProcessSelectionRequest);
/*       if (opt_debug) { */
/* 	(void) fprintf(stderr, "register ProcessPropertyNotify toDpy %d %d\n", (int)d, (int)w); */
/*       } */
      XSaveContext(d, w, PropertyNotify, (XPointer) ProcessPropertyNotify);
      XSaveContext(d, w, SelectionNotify, (XPointer) ProcessSelectionNotify);
      XSaveContext(d, w, SelectionClear, (XPointer) ProcessSelectionClear);
/*       w = RootWindow(c_tDisplay[i], 0); */
    }
  }
  XSaveContext(fromDpy, None, MappingNotify, (XPointer) ProcessMapping);
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting RegisterEventHandlers\n\n"); */
/* #endif */
}					/* RegisterEventHandlers */

/* ------------------------------------------------------------------------- */
static Bool     ProcessEvent(dpy)
Display        *dpy;
{
  XEvent          ev;
  XAnyEvent      *pEv = (XAnyEvent *) & ev;
  HANDLER         handler;

/*   if (opt_debug) { */
/*     (void) fprintf(stderr, "in ProcessEvent\n"); */
/*   } */
  XNextEvent(dpy, &ev);
  handler = 0;
  if ((!XFindContext(dpy, pEv->window, pEv->type, (XPointer *) & handler)) ||
      (!XFindContext(dpy, None, pEv->type, (XPointer *) & handler))) {
/* have handler */
    return((*handler) (dpy, &ev));
  }
  if (pEv->type == MapNotify) {
    return(False);			/* ignore MapNotify events */
  }
  if (opt_debug) {
    (void) sprintf(debugstring, "ProcessEvent %d (%s) window %ld display (%s)\n  ",
		   pEv->type, event_type[pEv->type], pEv->window,
		   pEv->display->display_name);
    (void) displaydisplay(debugstring, dpy);
    (void) displaywindow("  ", pEv->window);
    (void) fprintf(stderr, "no event handler\n");
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessEvent\n\n"); */
/* #endif */
  return(False);
}					/* ProcessEvent */

/* ------------------------------------------------------------------------- */
static void     DoXMANY()
{
  int             nfds;
  fd_set         *fdset;
  int             fromConn;
  int             toConn;
  int		  i;
  int		  flag;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "DoXMANY\n"); */
/* #endif */
/* set up displays */
  InitDpyInfo();

  RegisterEventHandlers();

/* set up for select */
  nfds = getdtablesize();
  fdset = (fd_set *) malloc(sizeof(fd_set));
/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "DoXMANY, XConnectionNumber(fromDpy)\n"); */
/* #endif */
  fromConn = XConnectionNumber(fromDpy);
  while (True) {
/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "DoXMANY while(forever)\n"); */
/* #endif */
    flag = 0;
    if (XPending(fromDpy)) {
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"pending event, from\n"); */
/* #endif */
      flag = 1;
      if (ProcessEvent(fromDpy)) {
	break;
      }
    }
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"checking to displays\n"); */
/* #endif */
    for (i = 0; i <  number_to; i++) {
      if (display_is_from[i] != 1) {	/* if not on same machine */
	if (XPending(c_tDisplay[i])) {
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"pending event, to (%d)\n", i); */
/* #endif */
	  flag = 1;
	  if (ProcessEvent(c_tDisplay[i])) {
	    break;
	  }
	}
      }
    }
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"no events, or events handled\n"); */
/* #endif */
    if (flag == 0) {
      FD_ZERO(fdset);
      FD_SET(fromConn, fdset);
      for (i = 0; i <  number_to; i++) {
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"setting toConn for %d (%d)\n", i, (int)c_tDisplay[i]); */
/* #endif */
	toConn = XConnectionNumber(c_tDisplay[i]);
	FD_SET(toConn, fdset);
      }
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"before select\n"); */
/* #endif */
      select(nfds, fdset, NULL, NULL, NULL);
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr,"after select\n"); */
/* #endif */
    }
  }
  free(fdset);
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting DoXMANY\n\n"); */
/* #endif */
}					/* DoXMANY */

/* ------------------------------------------------------------------------- */
int             main(argc, argv)
int             argc;
char          **argv;
{
  char           *findex;
  int 		  i;
  int		  cnt;
  int		  from_on_screen = 0;
  char *dindex;
  int mlth;
  int dlth;
  struct machine_list *ml;
  int found;
  int machine_cnt;

  XrmInitialize();
  ParseCommandLine(argc, argv);
/* Convert to real name. */
  from_name = XDisplayName(from_name);
  findex = index(from_name, ':');
  if (findex == NULL) {
    from_host_lth = strlen(from_name);
  } else {
    from_host_lth = findex - from_name;
  }
/* add : to name */
  (void) asprintf(&from_host, "%*.*s:", from_host_lth,from_host_lth, from_name);
  from_host_lth++;	/* add in : */
/* Convert to real name(s). */
  cnt = 0;
  machine_cnt = 0;
  dpyInfo.trigger_cnt = 0;
  display_is_from = (int *) malloc(sizeof(int) * number_to);
  for (i = 0; i <  number_to; i++) {
    display_to_names[i] = XDisplayName(display_to_names[i]);
    if (strcasecmp(display_to_names[i], from_name) == 0) {
      cnt++;
      if (cnt > 1) {
	(void) fprintf(stderr, "from display must only be in list once.\n");
	exit(1);
      }
      from_on_screen = i;
    }
    if (strncasecmp(display_to_names[i], from_host, from_host_lth) == 0) {
      machine_cnt++;
      display_is_from[i] = 1;		/* is really from host. */
      dpyInfo.trigger_cnt++;
    } else {
      display_is_from[i] = 0;		/* is another host */
    }
/* Create a list of machines, for cut & paste. */
    dindex = index(display_to_names[i], ':');
    if (dindex == NULL) {
      dlth = strlen(display_to_names[i]);
    } else {
      dlth = dindex - display_to_names[i];
    }
    found = 0;
    for (ml = machine_list_first; ml != NULL; ml = ml->next) {
      mlth = strlen(ml->name);
      if (mlth == dlth && strncmp(display_to_names[i], ml->name, mlth) == 0) {
	if (display_to_names[i][mlth] == ':') {
	  found = 1;
	  break;
	}
      }
    }
    if (found == 0) {			/* add machine one time only. */
      ml = (struct machine_list *) malloc(sizeof(struct machine_list));
      ml->next = machine_list_first;
      ml->name = (char *) malloc(dlth+1);
      ml->propWin = 0;			/* flag not initialized */
      (void) strncpy(ml->name, display_to_names[i], dlth);
      ml->name[dlth] = '\0';		/* make sure null terminated */
      machine_list_first = ml;
    }
  }
  found = 0;
  for (i = 0; i < TRIGGER_NUMBER; i++) {
    found += wrapdirection[i];
  }
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr, "found=%d number_to=%d from_on_screen(machine_cnt)=%d\n",found,number_to,machine_cnt); */
/* #endif */
  if (machine_cnt == 1 && number_to == 1 && found == 0) {
    (void) fprintf(stderr, "no reason to use this program, and it would fail.\n");
    exit(1);
  }
  if (opt_debug != 0) {			/* list machines involved - cut&paste */
    for (ml = machine_list_first; ml != NULL; ml = ml->next) {
      (void) fprintf(stderr, "Machine=%s\n", ml->name);
    }
  }
/* Need to check if a from_host match, and if so, is any other of those
   mentioned twice -- error */
  if (dpyInfo.trigger_cnt > 1) {
    for (i = 0; i <  (number_to - 1); i++) {
      if (display_is_from[i] == 0) {	/* not from host */
	for (cnt = (i + 1); cnt <  number_to; cnt++) {
	  if (strcasecmp(display_to_names[i], display_to_names[cnt]) == 0) {
	    (void) fprintf(stderr, "error: display %d,%d and %d,%d match\n",
			  i % geom_x, i / geom_x, cnt % geom_x, cnt / geom_x);
	    exit(1);
	  }
	}
      }
    }
  }
/* Open From Display */
  while ((fromDpy = XOpenDisplay(from_name)) == NULL) {
    if (opt_wait == False) {
      (void) fprintf(stderr, "%s - error: can not open from display %s\n",
	      programStr, from_name);
      exit(2);
    }
    sleep(1);
  }
/* Open To Displays */
  c_tDisplay = (Display **) malloc(sizeof(Display *) * number_to);
  for (i = 0; i < number_to; i++) {
    if (strcasecmp(from_name, display_to_names[i]) == 0) {
      c_tDisplay[i] = fromDpy;
    } else {
      while ((c_tDisplay[i] = XOpenDisplay(display_to_names[i])) == NULL) {
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "XOpenDisplay failed (%s)\n", display_to_names[i]);
	}
	if (opt_wait == False) {
	  (void) fprintf(stderr, "%s - error: can not open to display %s\n",
		  programStr, display_to_names[i]);
	  exit(3);
	}
	sleep(1);
      }
    }
/*   (void) fprintf(stderr,"to display: default_screen=%d, nscreens=%d\n",DefaultScreen(c_tDisplay[i]),ScreenCount(c_tDisplay[i])); */
/* do not check TEST extension if on same machine as from */
    if (display_is_from[i] == 0) {	/* not from host */
      if (CheckTestExtension(c_tDisplay[i]) == 0) {
	fprintf(stderr,
		"%s - error: display %s does not support the test extension\n",
		programStr, display_to_names[i]);
	exit(4);
      }
    }
  }
  dpyInfo.on_to_screen = from_on_screen;
  DoXMANY();				/* Many loop */
/* Shut down displays gracefully. */
  XCloseDisplay(fromDpy);
  for (i = 0; i <  number_to; i++) {
    XCloseDisplay(c_tDisplay[i]);
  }
  exit(0);
}					/* main */

/* ------------------------------------------------------------------------- */
static void     DoConnect()
{
  int j;
  int l;

  if (opt_debug != 0) {
    (void) fprintf(stderr, "in DoConnect\n");
  }
  dpyInfo.mode = XMANY_CONNECTED;
/* unset "other" window triggers -- this one set below. */
  for (j = 0; j < dpyInfo.trigger_cnt; j++) {
    for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
      if (dpyInfo.trigger[j][l].triggerwindow != (unsigned int)(-1) &&
          dpyInfo.trigger[j][l].triggerwindow != dpyInfo.windowfrom ) {
/* 	if (opt_debug != 0) { */
/* 	  (void) fprintf(stderr, "DoConnect XSelectInput(fromDpy[%d],%d,0x%x)\n", (int)fromDpy, (int)(dpyInfo.trigger[j][l].triggerwindow), dpyInfo.eventmask_normal); */
/* 	} */
	XSelectInput(fromDpy, dpyInfo.trigger[j][l].triggerwindow,
		   dpyInfo.eventmask_normal);
      }
    }
  }
/* make work. */
  XGrabPointer(fromDpy, dpyInfo.windowfrom, True,
	       PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
	       GrabModeAsync, GrabModeAsync,
	       None, dpyInfo.grabcursor, CurrentTime);
  XGrabKeyboard(fromDpy, dpyInfo.windowfrom, True,
		GrabModeAsync, GrabModeAsync,
		CurrentTime);
/*   if (opt_debug != 0) { */
/*     (void) fprintf(stderr, "DoConnect XSelectInput(fromDpy[%d],%d,0x%x)\n", (int)fromDpy, (int)(dpyInfo.windowfrom), (int)(dpyInfo.eventmask_normal | PointerMotionMask)); */
/*   } */
  XSelectInput(fromDpy, dpyInfo.windowfrom,
	       dpyInfo.eventmask_normal | PointerMotionMask);
  XFlush(fromDpy);
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting DoConnect\n\n");
  }
}					/* DoConnect */

/* ------------------------------------------------------------------------- */
static void     FakeAction(type, thing, bDown)
int             type;
unsigned int    thing;
Bool            bDown;
{
  struct _fakestr **ppFake;
  struct _fakestr *pFake;

/* #ifdef DEBUG_ENTRY */
/* (void) fprintf(stderr, "FakeAction\n"); */
/* #endif */
/* find the associated button, or the last record, whichever comes first */
  for (ppFake = &(dpyInfo.pFakeThings);
       (*ppFake && (((*ppFake)->type != type) || ((*ppFake)->thing != thing)));
       ppFake = &((*ppFake)->pNext)) {
    ;
  }
  if (bDown) {				/* key down */
    if (*ppFake == NULL) {		/* need a new record */
      pFake = (struct _fakestr *) malloc(sizeof(struct _fakestr));
      pFake->pNext = NULL;		/* always at the end of the list */
      pFake->type = type;
      pFake->thing = thing;
      *ppFake = pFake;
    }
  } else {				/* key up */
    if (*ppFake != NULL) {		/* get rid of the record */
/* splice out of the list */
      pFake = *ppFake;
      *ppFake = pFake->pNext;
      free(pFake);
    }
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting FakeAction\n\n"); */
/* #endif */
}					/* FakeAction */

/* ------------------------------------------------------------------------- */
static void     FakeThingsUp()
{
  struct _fakestr *pFake;
  struct _fakestr *pNext;
  unsigned int    type;
  KeyCode         keycode;
  int cnt = 0;

/*   if (opt_debug != 0) { */
/*     (void) fprintf(stderr, "in FakeThingsUp\n"); */
/*   } */
  if (dpyInfo.pFakeThings) {		/* everything goes up */
    for (pFake = dpyInfo.pFakeThings; pFake; pFake = pNext) {
      type = pFake->type;
      if (type == FAKE_KEY) {		/* key goes up */
	if ((keycode = XKeysymToKeycode(c_tDisplay[dpyInfo.on_to_screen], pFake->thing))) {
	  XTestFakeKeyEvent(c_tDisplay[dpyInfo.on_to_screen], keycode, False, 0);
	  cnt++;
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr, "FakeThingsUp: XTestFakeKeyEvent  screen %d\n",dpyInfo.on_to_screen); */
/* #endif */
	}
      } else {				/* button goes up */
	XTestFakeButtonEvent(c_tDisplay[dpyInfo.on_to_screen], pFake->thing, False, 0);
	cnt++;
/* #ifdef DEBUG_LOTS */
/* (void) fprintf(stderr, "FakeThingsUp: XTestFakeButtonEvent  screen %d\n",dpyInfo.on_to_screen); */
/* #endif */
      }
/* flush everything at once */
      if (cnt > 0) {
	XFlush(c_tDisplay[dpyInfo.on_to_screen]);
      }
/* get next and free current */
      pNext = pFake->pNext;
      free(pFake);
    }
    dpyInfo.pFakeThings = NULL;
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting FakeThingsUp\n\n"); */
/* #endif */
}					/* FakeThingsUp */

/* ------------------------------------------------------------------------- */
static void     DoDisconnect()
{
  int j;
  int l;

  if (opt_debug != 0) {
    (void) fprintf(stderr, "in DoDisconnect\n");
  }
  dpyInfo.mode = XMANY_DISCONNECTED;
/* Set other window triggers, this window done below. */
  for (j = 0; j < dpyInfo.trigger_cnt; j++) {
    for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
      if (dpyInfo.trigger[j][l].triggerwindow != (unsigned int)(-1) &&
	  dpyInfo.trigger[j][l].triggerwindow != dpyInfo.windowfrom) {
/* 	if (opt_debug != 0) { */
/* 	  (void) fprintf(stderr, "DoDisconnect XSelectInput(fromDpy[%d],%d,0x%x)\n", (int)fromDpy, (int)(dpyInfo.trigger[j][l].triggerwindow), dpyInfo.eventmask_trigger); */
/* 	} */
	XSelectInput(fromDpy, dpyInfo.trigger[j][l].triggerwindow,
		   dpyInfo.eventmask_trigger);
      }
    }
  }
/* Make work */
  XUngrabKeyboard(fromDpy, CurrentTime);
  XUngrabPointer(fromDpy, CurrentTime);
/*   if (opt_debug != 0) { */
/*     (void) fprintf(stderr, "DoDisconnect XSelectInput(fromDpy[%d],%d,0x%x)\n", (int)fromDpy, (int)(dpyInfo.windowfrom), dpyInfo.eventmask_trigger); */
/*   } */
  XSelectInput(fromDpy, dpyInfo.windowfrom, dpyInfo.eventmask_trigger);
  if (dpyInfo.fromDpyXtra.sState == SELSTATE_ON) {
    XSetSelectionOwner(fromDpy, XA_PRIMARY, dpyInfo.fromDpyXtra.propWin, CurrentTime);
    if (opt_debug != 0) {
      (void) displaydisplay("XSetSelectionOwner XA_PRIMARY: ", fromDpy);
      (void) displaywindow("  ", dpyInfo.fromDpyXtra.propWin);
      (void) displayXA(fromDpy, dpyInfo.fromDpyXtra.propWin);
    }
  }
  XFlush(fromDpy);
/* force normal state on to display: */
  FakeThingsUp();
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting DoDisconnect\n\n");
  }
}					/* DoDisconnect */

/* ------------------------------------------------------------------------- */
static void     StartGetSelection(dpy, pDpyXtra)
Display        *dpy;
struct _dpyxtra *pDpyXtra;
{
  if (opt_debug != 0) {
    (void) fprintf(stderr, "in StartGetSelection\n");
    (void) displayXA(dpy, pDpyXtra->propWin);
  }
  if (pDpyXtra->pingInProg == False) {
/* Get current server time, by appending zerl length data to a property using */
/* XChangeProperty, which generates a PropertyNotify event with the time. */
    XChangeProperty(dpy, pDpyXtra->propWin, pDpyXtra->dpypAtom, XA_PRIMARY,
		    8, PropModeAppend, NULL, 0);
/*     if (opt_debug != 0) { */
/*       (void) fprintf(stderr, "after XChangeProperty\n"); */
/*       (void) displayXA(dpy, pDpyXtra->propWin); */
/*     } */
    pDpyXtra->pingInProg = True;
  } else {
    if (opt_debug != 0) {
      (void) fprintf(stderr, "pingInProg != False\n");
    }
  }
  if (opt_debug != 0) {
#ifdef DEBUG_EXIT
      (void) fprintf(stderr, "exiting StartGetSelection\n\n");
#endif
  }
}					/* StartGetSelection */

/* ------------------------------------------------------------------------- */
static void     SendSelectionNotify(pSelReq)
XSelectionRequestEvent *pSelReq;
{
  XSelectionEvent sendEv;

  if (opt_debug != 0) {
    (void) displaydisplay("in SendSelectionNotify: ", pSelReq->display);
  }
  sendEv.type = SelectionNotify;
  sendEv.display = pSelReq->display;
  sendEv.requestor = pSelReq->requestor;
  sendEv.selection = pSelReq->selection;
  sendEv.target = pSelReq->target;
  sendEv.property = pSelReq->property;
  sendEv.time = pSelReq->time;
  XSendEvent(pSelReq->display, pSelReq->requestor, False, 0,
	     (XEvent *) & sendEv);
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting SendSelectionNotify: requestor=%ld\n\n" ,pSelReq->requestor);
  }
}					/* SendSelectionNotify */

/* ------------------------------------------------------------------------- */
/* Note: ProcessMotionNotify is sometimes called from inside xmany to */
/* simulate motion event. */
/* Any new references to pEv fields must be checked carefully. */

/*ARGSUSED*/
static Bool     ProcessMotionNotify(unused, pEv)
Display        *unused;
XMotionEvent   *pEv;			/* caution: might be pseudo-event. */
{
  int             screen_cnt;
  int             toX;
  int             toY;
  int             fromX;
  int             fromY;
  int             delta;
  int		  tmpx;
  int		  tmpy;
  int		  gottawarp = 0;

#ifdef DEBUG_ENTRY
(void) fprintf(stderr, "ProcessMotionNotify\n");
#endif
/* find the screen */
  screen_cnt = dpyInfo.on_to_screen;
  fromX = pEv->x_root;
  fromY = pEv->y_root;
/* check to make sure the cursor is still on the from screen */
  if (pEv->same_screen == 0) {			/* multiple screens */
    gottawarp = 1;
    if (opt_debug != 0) {
      (void) fprintf(stderr, "pEv->same_screen=%d  from X,Y=(%d,%d)\n", pEv->same_screen,fromX,fromY);
    }
/* We can't stop cursor from moving to new screen on a machine with */
/* multiple screens and screen wrapping turned on. */
/* What we must do is determine which direction the cursor went, and set */
/* toX and toY as appropriate.  Then we must move the cursor back to the */
/* display it was at. */
    toY = dpyInfo.yTables[screen_cnt][fromY];
    toX = dpyInfo.xTables[screen_cnt][fromX];
/*     toX = (dpyInfo.lastFromX < fromX) ? COORD_DECR : COORD_INCR; */
    delta = dpyInfo.lastFromX - fromX;
#ifdef DEBUG_LOTS
    if (opt_debug != 0) {
      (void) fprintf(stderr,"X delta = %d\n",delta);
    }
#endif
    if (delta < 0 && (-delta) > dpyInfo.unreasonableDelta) {
      fromX = 0;
      toX = dpyInfo.xTables[screen_cnt][fromX];
    } else if (delta > dpyInfo.unreasonableDelta) {
      fromX = XWidthOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth;
      toX = dpyInfo.xTables[screen_cnt][fromX];
    }
/*     toY = (dpyInfo.lastFromY < fromY) ? COORD_DECR : COORD_INCR; */
    delta = dpyInfo.lastFromY - fromY;
#ifdef DEBUG_LOTS
    if (opt_debug != 0) {
      (void) fprintf(stderr,"Y delta = %d\n",delta);
    }
#endif
    if (delta < 0 && (-delta) > dpyInfo.unreasonableDelta) {
      fromY = 0;
      toY = dpyInfo.yTables[screen_cnt][fromY];
    } else if (delta > dpyInfo.unreasonableDelta) {
      fromY = XHeightOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth - 1;
      toY = dpyInfo.yTables[screen_cnt][fromY];
    }
/*     if (SPECIAL_COORD(toX) == 0 && SPECIAL_COORD(toY) == 0) { */
/*       if (opt_debug != 0) { */
/* 	(void) fprintf(stderr, "Uh, don't know why we are here?\n"); */
/*       } */
/*     } */
    (void) fprintf(stderr, "to X,Y=(%d,%d) screen_cnt=%d last x,y(%d,%d) new x,y(%d,%d)\n",toX,toY,screen_cnt,dpyInfo.lastFromX,dpyInfo.lastFromY,fromX,fromY);
  } else {
    toX = dpyInfo.xTables[screen_cnt][fromX];
    toY = dpyInfo.yTables[screen_cnt][fromY];
/* sanity check motion: necessary for nondeterminism surrounding warps */
    delta = dpyInfo.lastFromX - fromX;
    if (delta < 0) {
      delta = -delta;
    }
    if (delta > dpyInfo.unreasonableDelta) {
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessMotionNotify (unreasonable X delta)\n\n"); */
/* #endif */
      return(False);
    }
    delta = dpyInfo.lastFromY - fromY;
    if (delta < 0) {
      delta = -delta;
    }
    if (delta > dpyInfo.unreasonableDelta) {
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessMotionNotify (unreasonable Y delta)\n\n"); */
/* #endif */
      return(False);
    }
  }
  if (SPECIAL_COORD(toX) != 0) {	/* special coordinate in X direction */
    tmpx = screen_cnt % geom_x;
/*     if (opt_debug != 0) { */
/*       (void) fprintf(stderr, "SPECIAL_COORD(toX) geom x=%d,y=%d screen_cnt=%d,tmpx=%d last x,y(%d,%d) new x,y(%d,%d)\n",geom_x,geom_y,screen_cnt,tmpx,dpyInfo.lastFromX,dpyInfo.lastFromY,fromX,fromY); */
/*     } */
    set_t_w(screen_cnt);
    if (toX == COORD_INCR && t_w[TRIGGER_EAST] == 0) {
      if ((tmpx + 1) < geom_x) {
	screen_cnt++;				/* simple go right */
      } else {
	screen_cnt = (screen_cnt + 1 - geom_x);	/* wrap to the left */
      }
/*       if (opt_debug != 0) { */
/* 	(void) fprintf(stderr, "x incrementing(trigger_east), new screen_cnt=%d, is_from=%d\n",screen_cnt,display_is_from[screen_cnt]); */
/*       } */
      fromX = opt_triggerwidth;
      toX = dpyInfo.xTables[screen_cnt][fromX];
      toY = dpyInfo.yTables[screen_cnt][fromY];
      if (display_is_from[screen_cnt] == 0) {	/* different machine */
/*	DoConnect(); */
      } else {
	fromX = toX;			/* AHA, FIX THIS */
/* 	if (display_is_from[screen_cnt] == 0) { */
	  XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
	  XFlush(fromDpy);
	  DoDisconnect();
/* 	} */
      }
    } else if (t_w[TRIGGER_WEST] == 0) {	/* DECR */
      if ((tmpx - 1) >= 0) {		/* simple go to left */
	screen_cnt--;
      } else {
	screen_cnt = (screen_cnt - 1 + geom_x);
      }
/*       if (opt_debug != 0) { */
/* 	(void) fprintf(stderr, "x decrementing(trigger_west), new screen_cnt=%d, is_from=%d\n",screen_cnt,display_is_from[screen_cnt]); */
/*       } */
      fromX = XWidthOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth - 1;
      toX = dpyInfo.xTables[screen_cnt][fromX];
      toY = dpyInfo.yTables[screen_cnt][fromY];
      if (display_is_from[screen_cnt] == 0) {	/* different machine */
/*	DoConnect(); */
      } else {
	fromX = toX;			/* AHA, FIX THIS */
/* 	if (display_is_from[screen_cnt] == 0) { */
	  XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
	  XFlush(fromDpy);
	  DoDisconnect();
/* 	} */
      }
    } else if (toX == COORD_INCR) {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "x incrementing, t_w says don't\n");
      }
      fromX = opt_triggerwidth;
      toX = dpyInfo.xTables[screen_cnt][fromX];
    } else {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "x decrementing, t_w says don't\n");
      }
      fromX = XWidthOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth - 1;
      toX = dpyInfo.xTables[screen_cnt][fromX];
      toY = dpyInfo.yTables[screen_cnt][fromY];
    }
    dpyInfo.on_to_screen = screen_cnt;
    XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
    XFlush(fromDpy);
  }
  if (SPECIAL_COORD(toY) != 0) {	/* special coordinate in Y direction */
    tmpx = screen_cnt % geom_x;
    tmpy = screen_cnt / geom_x;
/*     if (opt_debug != 0) { */
/*       (void) fprintf(stderr, "SPECIAL_COORD(toY) geom x=%d,y=%d screen_cnt=%d,tmp x=%d,y=%d last,new(%d,%d)\n",geom_x,geom_y,screen_cnt,tmpx,tmpy,dpyInfo.lastFromY,fromY); */
/*     } */
    set_t_w(screen_cnt);
    if (toY == COORD_INCR && t_w[TRIGGER_SOUTH] == 0) {
      if ((tmpy + 1) < geom_y) {	/* simple go up */
	screen_cnt = screen_cnt + geom_x;
      } else {
	screen_cnt = tmpx;		/* really a wrap from bottom to top */
      }
/*       if (opt_debug != 0) { */
/* 	(void) fprintf(stderr, "y incrementing(trigger_south), new screen_cnt=%d, is_from=%d\n",screen_cnt,display_is_from[screen_cnt]); */
/*       } */
      fromY = opt_triggerwidth;
      toY = dpyInfo.yTables[screen_cnt][fromY];
      toX = dpyInfo.yTables[screen_cnt][fromX];
      if (display_is_from[screen_cnt] == 0) {	/* different machine */
/*	DoConnect(); */
      } else {
	fromY = toY;			/* AHA, FIX THIS */
/* 	if (display_is_from[screen_cnt] == 0) { */
	  XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
	  XFlush(fromDpy);
	  DoDisconnect();
/* 	} */
      }
    } else if (t_w[TRIGGER_NORTH] == 0) {	/* DECR */
      if (tmpy > 0) {			/* simple go down */
	screen_cnt = screen_cnt - geom_x;
      } else {
	screen_cnt = ((geom_y-1) * geom_x) +tmpx;
      }
/*       if (opt_debug != 0) { */
/* 	(void) fprintf(stderr, "y decrementing(trigger_north), new screen_cnt=%d, is_from=%d\n",screen_cnt,display_is_from[screen_cnt]); */
/*       } */
      fromY = XHeightOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth - 1;
      toY = dpyInfo.yTables[screen_cnt][fromY];
      toX = dpyInfo.yTables[screen_cnt][fromX];
      if (display_is_from[screen_cnt] == 0) {	/* different machine */
/*	DoConnect(); */
      } else {
	fromY = toY;			/* AHA, FIX THIS */
/* 	if (display_is_from[screen_cnt] == 0) { */
 	  XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
	  XFlush(fromDpy);
	  DoDisconnect();
/* 	} */
      }
    } else if (toY == COORD_INCR) {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "y incrementing, t_w says don't\n");
      }
      fromY = opt_triggerwidth;
      toY = dpyInfo.xTables[screen_cnt][fromY];
      toX = dpyInfo.yTables[screen_cnt][fromX];
    } else {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "y decrementing, t_w says don't\n");
      }
      fromY = XHeightOfScreen(XDefaultScreenOfDisplay(fromDpy)) - opt_triggerwidth - 1;
      toY = dpyInfo.xTables[screen_cnt][fromY];
      toX = dpyInfo.yTables[screen_cnt][fromX];
    }
    dpyInfo.on_to_screen = screen_cnt;
    XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
    XFlush(fromDpy);
  }
  if (gottawarp == 1) {
    XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, fromX, fromY);
    XFlush(fromDpy);
  }
  dpyInfo.lastFromX = fromX;
  dpyInfo.lastFromY = fromY;
  XTestFakeMotionEvent(c_tDisplay[dpyInfo.on_to_screen], dpyInfo.screen_nums[screen_cnt], toX, toY, 0);
  XFlush(c_tDisplay[dpyInfo.on_to_screen]);
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessMotionNotify\n\n"); */
/* #endif */
  return(False);
}					/* ProcessMotionNotify */

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessExpose(dpy, pEv)
Display        *dpy;
XExposeEvent   *pEv;
{
  int i;
  int l;

#ifdef DEBUG_ENTRY
  if (opt_debug != 0) {
    (void) fprintf(stderr, "ProcessExpose\n");
  }
#endif
  if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
    if (dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow != (unsigned int)(-1)) {
      XClearWindow(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow);
      if (dpyInfo.font) {
	formatText(fromDpy, dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow,
		   &(dpyInfo.textGC), dpyInfo.font,
		   toDpyFormatLength, toDpyFormat, NULL, NULL);
      }
    }
  } else {
    for (i = 0; i < dpyInfo.trigger_cnt; i++) {
      for (l = TRIGGER_TEXT+1; l < TRIGGER_NUMBER; l++) {
	if (dpyInfo.trigger[i][l].triggerwindow != (unsigned int)(-1)) {
	  XClearWindow(fromDpy, dpyInfo.trigger[i][l].triggerwindow);
	}
      }
    }
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessExpose\n\n");
  }
#endif
  return(False);
}					/* ProcessExpose */

/* ------------------------------------------------------------------------- */
static Bool     ProcessEnterNotify(dpy, pEv)
Display        *dpy;
XCrossingEvent *pEv;
{
  XMotionEvent    xmev;
  int             tmpx;
  int             tmpy;
  int             i;
  int             j;
  int             l;
  int             m;

#ifdef DEBUG_ENTRY
(void) fprintf(stderr, "ProcessEnterNotify\n");
#endif
  if ((pEv->mode == NotifyNormal) &&
      (dpyInfo.mode == XMANY_DISCONNECTED) &&
      (dpy == fromDpy)) {
    if (dpyInfo.trigger_cnt == 0 || wrapdirections == 0) {
      l = 1;
    } else {
      l = dpyInfo.trigger_cnt;
    }
    for (j = 0; j < l; j++) {
      for (i = 0; i < TRIGGER_NUMBER; i++) {
	if (pEv->window == dpyInfo.trigger[j][i].triggerwindow) {
	  dpyInfo.windowfrom = dpyInfo.trigger[j][i].triggerwindow;
	  dpyInfo.MoveToX = dpyInfo.trigger[j][i].MoveToX;
	  dpyInfo.MoveToY = dpyInfo.trigger[j][i].MoveToY;
	  dpyInfo.grabcursor = dpyInfo.trigger[j][i].grabCursor;
	  m = dpyInfo.trigger_screen[j];
	  if (m < 0) {
	    (void) fprintf(stderr, "ouch, trigger_screen not set\n");
	  } else {
	    tmpx = m % geom_x;
	    tmpy = m / geom_x;
	    switch (i) {
	      case TRIGGER_TEXT:
		break;			/* do not understand what to do */
	      case TRIGGER_NORTH:
		if (tmpy > 0) {
		  dpyInfo.on_to_screen = m - geom_x;
		} else {
		  dpyInfo.on_to_screen = ((geom_y-1) * geom_x) +tmpx;
		}
		break;
	      case TRIGGER_SOUTH:
		if ((tmpy+1) < geom_y) {
		  dpyInfo.on_to_screen = m + geom_x;
		} else {
		  dpyInfo.on_to_screen = tmpx;
		}
		break;
	      case TRIGGER_EAST:
		if ((tmpx+1) < geom_x) {
		  dpyInfo.on_to_screen = m + 1;
		} else {
		  dpyInfo.on_to_screen = (m + 1 - geom_x);
		}
		break;
	      case TRIGGER_WEST:
		if ((tmpx-1) >= 0) {
		  dpyInfo.on_to_screen = m - 1;
		} else {
		  dpyInfo.on_to_screen = (m - 1 + geom_x);
		}
		break;
	    }
	  }
/* (void) fprintf(stderr,"found at j=%d  i=%d (%s)  t_s=%d  on_to_screen=%d\n", j,i,trigger_which[i],dpyInfo.trigger_screen[j],dpyInfo.on_to_screen); */
	  break;
	}
      }
      if (i >= 0 && i < TRIGGER_NUMBER) {
/* (void) fprintf(stderr,"found break # 2 at j=%d  i=%d (%s)\n", j,i,trigger_which[i]); */
	break;
      }
    }
    if (j < 0 || j >= l) {
      return(False);
    }
/* Fix where to go */
    if (dpyInfo.MoveToX == -1) {
      tmpx = pEv->x_root;
    } else {
      tmpx = dpyInfo.MoveToX;
    }
    if (dpyInfo.MoveToY == -1) {
      tmpy = pEv->y_root;
    } else {
      tmpy = dpyInfo.MoveToY;
    }
    if (display_is_from[dpyInfo.on_to_screen] == 0) {	/* different machine */
      DoConnect();
      XWarpPointer(fromDpy, None, dpyInfo.root, 0, 0, 0, 0, tmpx, tmpy);
    }
    xmev.x_root = dpyInfo.lastFromX = tmpx;
    xmev.y_root = dpyInfo.lastFromY = tmpy;
    xmev.same_screen = True;
    ProcessMotionNotify(NULL, &xmev);
  } else {
/*       (void) fprintf(stderr, "already connected to display, ignore\n"); */
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessEnterNotify\n\n"); */
/* #endif */
  return(False);
}					/* ProcessEnterNotify */

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessButtonPress(dpy, pEv)
Display        *dpy;
XButtonEvent   *pEv;
{
  int             state;
  unsigned int    toButton = 0;

  if (opt_debug != 0) {
    (void) fprintf(stderr, "in ProcessButtonPress\n");
  }
  switch (dpyInfo.mode) {
    case XMANY_DISCONNECTED:
      dpyInfo.mode = XMANY_AWAIT_RELEASE;
      break;
    case XMANY_CONNECTED:
      if (pEv->button <= N_BUTTONS) {
	toButton = dpyInfo.inverseMap[pEv->button];
      }
      XTestFakeButtonEvent(c_tDisplay[dpyInfo.on_to_screen], toButton, True, 0);
      XFlush(c_tDisplay[dpyInfo.on_to_screen]);
      FakeAction(FAKE_BUTTON, toButton, True);
/* NOTDONEYET?  Do we ignore if no text? */
/* check if more than one button pressed */
      state = pEv->state;
      switch (pEv->button) {
	case Button1:
	  state &= ~Button1Mask;
	  break;
	case Button2:
	  state &= ~Button2Mask;
	  break;
	case Button3:
	  state &= ~Button3Mask;
	  break;
	case Button4:
	  state &= ~Button4Mask;
	  break;
	case Button5:
	  state &= ~Button5Mask;
	  break;
	default:
	  if (opt_debug != 0) {
	    (void) fprintf(stderr, "  unknown button %d\n", pEv->button);
	  }
	  break;
      }
      if (state) {			/* then more than one button pressed */
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  awaiting button release before disconnecting\n");
	}
	dpyInfo.mode = XMANY_CONN_RELEASE;
      }
      break;
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessButtonPress\n\n"); */
/* #endif */
  return(False);
}					/* ProcessButtonPress */

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessButtonRelease(dpy, pEv)
Display        *dpy;
XButtonEvent   *pEv;
{
  int             state;
  XMotionEvent    xmev;
  unsigned int    toButton = 0;

  if (opt_debug != 0) {
    (void) fprintf(stderr, "in ProcessButtonRelease\n");
  }
/* (void) fprintf(stderr,"ProcessButtonRelease (%d)\n", dpyInfo.mode); */
  if ((dpyInfo.mode == XMANY_CONNECTED) ||
      (dpyInfo.mode == XMANY_CONN_RELEASE)) {
    if (pEv->button <= N_BUTTONS) {
      toButton = dpyInfo.inverseMap[pEv->button];
    }
    XTestFakeButtonEvent(c_tDisplay[dpyInfo.on_to_screen], toButton, False, 0);
    XFlush(c_tDisplay[dpyInfo.on_to_screen]);
    FakeAction(FAKE_BUTTON, toButton, False);
  }
/* NOTDONEYET */
  if ((dpyInfo.mode == XMANY_AWAIT_RELEASE) ||
      (dpyInfo.mode == XMANY_CONN_RELEASE)) {
/* make sure that all buttons are released */
    state = pEv->state;
    switch (pEv->button) {
      case Button1:
	state &= ~Button1Mask;
	break;
      case Button2:
	state &= ~Button2Mask;
	break;
      case Button3:
	state &= ~Button3Mask;
	break;
      case Button4:
	state &= ~Button4Mask;
	break;
      case Button5:
	state &= ~Button5Mask;
	break;
      default:
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  unknown button %d\n", pEv->button);
	}
	break;
    }
    if (state == 0) {			/* all buttons up: (dis)connect */
      if (dpyInfo.mode == XMANY_AWAIT_RELEASE) {	/* connect */
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  Connect\n");
	}
	dpyInfo.windowfrom = dpyInfo.trigger[0][TRIGGER_TEXT].triggerwindow;
	dpyInfo.MoveToX = dpyInfo.trigger[0][TRIGGER_TEXT].MoveToX;
	dpyInfo.MoveToY = dpyInfo.trigger[0][TRIGGER_TEXT].MoveToY;
	dpyInfo.grabcursor = dpyInfo.trigger[0][TRIGGER_TEXT].grabCursor;
	DoConnect();
	xmev.x_root = dpyInfo.lastFromX = pEv->x_root;
	xmev.y_root = dpyInfo.lastFromY = pEv->y_root;
	xmev.same_screen = True;
	ProcessMotionNotify(NULL, &xmev);
      } else {				/* disconnect */
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  DisConnect\n");
	}
	DoDisconnect();
      }
    }
  } else {
/*     if (opt_debug != 0) { */
/*       (void) fprintf(stderr, "  no more processing on button release\n"); */
/*     } */
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessButtonRelease\n\n"); */
/* #endif */
  return(False);
}					/* ProcessButtonRelease */

/*                              "1234567890123456" */
/*                              "0123456789012345" */
/* static char KILLTHISTHING[] = "kill this thing"; */
static char KILLTHISTHING[] = "               ";
static char KILLLTH = sizeof(KILLTHISTHING);
/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessKeyEvent(dpy, pEv)
Display        *dpy;
XKeyEvent      *pEv;
{
  KeyCode         keycode;
  KeySym          keysym;
  Bool            bPress;

#ifdef DEBUG_ENTRY
  if (opt_debug != 0) {
    (void) fprintf(stderr, "ProcessKeyEvent\n");
  }
#endif
  keysym = XKeycodeToKeysym(fromDpy, pEv->keycode, 0);
  bPress = (pEv->type == KeyPress);
  if ((keycode = XKeysymToKeycode(c_tDisplay[dpyInfo.on_to_screen], keysym))) {
    if (bPress && keysym < 0xff) {
      (void)bcopy(KILLTHISTHING+1,KILLTHISTHING,KILLLTH - 2);
      KILLTHISTHING[KILLLTH-2] = keysym;
/* (void) fprintf(stderr, "keycode=0x%-4.4x, keysym=%-4.4x (%s)[%d] {%d}\n", keycode, keysym,KILLTHISTHING,KILLLTH,pEv->type); */
    }
    XTestFakeKeyEvent(c_tDisplay[dpyInfo.on_to_screen], keycode, bPress, 0);
    XFlush(c_tDisplay[dpyInfo.on_to_screen]);
  }
  FakeAction(FAKE_KEY, (unsigned int)keysym, bPress);
  if (strcmp(KILLTHISTHING,"kill this thing") == 0) {
    XTestFakeKeyEvent(c_tDisplay[dpyInfo.on_to_screen], keycode, !bPress, 0);
    XFlush(c_tDisplay[dpyInfo.on_to_screen]);
    (void) fprintf(stderr, "string typed = 'kill this thing', exiting\n");
/*     sleep(2); */
    exit(0);
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessKeyEvent\n\n");
  }
#endif
  return(False);
}					/* ProcessKeyEvent */

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessConfigureNotify(dpy, pEv)
Display        *dpy;
XConfigureEvent *pEv;
{
#ifdef DEBUG_ENTRY
  if (opt_debug != 0) {
    (void) fprintf(stderr, "ProcessConfigureNotify\n");
  }
#endif
  if (dpyInfo.font) {
/* reposition text */
    toDpyFormat[toDpyLeftIndex] = MAX(0, ((pEv->width - dpyInfo.twidth) / 2));
    toDpyFormat[toDpyTopIndex] = MAX(0, ((pEv->height - dpyInfo.theight) / 2));
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessConfigureNotify\n\n");
  }
#endif
  return(False);
}					/* ProcessConfigureNotify */

/* ------------------------------------------------------------------------- */

/*ARGSUSED*/
static Bool     ProcessClientMessage(dpy, pEv)
Display        *dpy;
XClientMessageEvent *pEv;
{
  if (opt_debug != 0) {
    (void) fprintf(stderr, "in ProcessClientMessage\n");
  }
/* terminate if atoms match. */
  return(((Atom)(pEv->message_type) == dpyInfo.wmpAtom) &&
	  ((Atom)(pEv->data.l[0]) == dpyInfo.wmdwAtom));
}					/* ProcessClientMessage */

/* ------------------------------------------------------------------------- */
static Bool     ProcessSelectionRequest(dpy, pEv)
Display        *dpy;
XSelectionRequestEvent *pEv;
{
  struct _dpyxtra *pDpyXtra = NULL;
  Display        *otherDpy;
  struct _dpyxtra *oDpyXtra;
  int i;
/*   int fromone = 0; */

  if (opt_debug != 0) {
    displaydisplay("in ProcessSelectionRequest: ", dpy);
/*     displaydisplay("  pEv->display: ", pEv->display); */
/*     (void) displaywindow("  pEv->owner: ", pEv->owner); */
/*     (void) displaywindow("  pEv->requestor: ", pEv->requestor); */
    (void) fprintf(stderr, "  type=%d  serial=%lu  send_event=", pEv->type, pEv->serial);
    if (pEv->send_event == 0) {
      (void) fprintf(stderr, "false ");
    } else {
      (void) fprintf(stderr, "true ");
    }
    (void) fprintf(stderr, "time=%ld\n", pEv->time);
/*     displayit(pEv->display, pEv->selection, "pEv->selection"); */
/*     displayit(pEv->display, pEv->target, "pEv->target"); */
/*     displayit(pEv->display, pEv->property, "pEv->property"); */
/*     (void) displayXA(dpy, pEv->requestor); */
  }
  if (opt_debug != 0) {
    display_dpyxtra();
  }
  if (dpy == fromDpy) {
    pDpyXtra = &(dpyInfo.fromDpyXtra);
  } else {
    for (i = 0; i <  number_to; i++) {
      if (dpy == c_tDisplay[i]) {
	pDpyXtra = &(dpyInfo.screenDpyXtra[i]);
	break;
      }
    }
    if (pDpyXtra == NULL) {
      if (opt_debug != 0) {
	(void) fprintf(stderr,"ProcessSelectionRequest unexpected display[%d]\n", (int)dpy);
      }
#ifdef DEBUG_EXIT
      if (opt_debug != 0) {
	(void) fprintf(stderr, "exiting ProcessSelectionRequest\n\n");
      }
#endif
      return(False);
    }
  }
  if ((pDpyXtra->sState != SELSTATE_ON) ||
      (pEv->selection != XA_PRIMARY) ||
      (pEv->target > XA_LAST_PREDEFINED)) {	/* bad request */
    if (opt_debug != 0) {
      (void) fprintf(stderr, "  bad requext, pass-thru?\n");
      (void) fprintf(stderr, "  sState=%d == %d, selection=%ld == %ld, target=%ld <= %ld\n",
		pDpyXtra->sState,SELSTATE_ON,pEv->selection,XA_PRIMARY,
		pEv->target,XA_LAST_PREDEFINED);
    }
    pEv->property = None;
    SendSelectionNotify(pEv);		/* tell dpy that a selection notify has happened (pass thru)? */
  } else {
/*     if (dpy == fromDpy) { */
      otherDpy = dpyInfo.lastSelectDpy;
      oDpyXtra = NULL;
/* NOT DONE YET???? */
      for (i = 0; i< number_to; i++) {	/* send to all other machines */
	if (otherDpy == c_tDisplay[i]) {
	  if (otherDpy == fromDpy) {
	    oDpyXtra = &(dpyInfo.fromDpyXtra);
	  } else {
	    oDpyXtra = &dpyInfo.screenDpyXtra[i];
	  }
	  break;
	}
      }
      if (oDpyXtra == NULL) {
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "problems with finding display.\n");
	}
	return(False);
      }
/*     } else { */
/*       otherDpy = fromDpy; */
/*       oDpyXtra = &(dpyInfo.fromDpyXtra); */
/*     } */
    if (opt_debug != 0) {
      displaydisplay("  otherDpy: ", otherDpy);
    }
    StartGetSelection(otherDpy, oDpyXtra);	/* get started sending data */
/* ---------- */
    if (oDpyXtra->sDpy) {		/* if one already present, delete */
      oDpyXtra->sEvt.property = None;
      SendSelectionNotify(&(oDpyXtra->sEvt));
    }
    oDpyXtra->sDpy = otherDpy;
    oDpyXtra->sEvt = *pEv;
    dpyInfo.lastSelectDpy = otherDpy;		/* try saving just last one */
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessSelectionRequest\n\n");
  }
#endif
  return(False);
}					/* ProcessSelectionRequest */

/* ------------------------------------------------------------------------- */
static Bool     ProcessPropertyNotify(dpy, pEv)
Display        *dpy;
XPropertyEvent *pEv;
{
  struct _dpyxtra *pDpyXtra = NULL;
  int i;

  if (opt_debug != 0) {
    (void) displaydisplay("in ProcessPropertyNotify: ", dpy);
    (void) displayXA(dpy, pEv->window);
  }
/* (void) fprintf(stderr,"dpy=%d  dpyInfo.on_to_screen=%d  pEv->atom=%d\n", */
/* 	(int)dpy, dpyInfo.on_to_screen, (int)(pEv->atom)); */
/* (void) fprintf(stderr,"fromDpy=%d  -Atom=%d\n", (int)fromDpy, (int)(dpyInfo.fromDpyXtra.dpypAtom)); */
  if (dpy == fromDpy) {
    pDpyXtra = &(dpyInfo.fromDpyXtra);
  } else {
    for (i = 0; i <  number_to; i++) {
      if (dpy == c_tDisplay[i]) {
	pDpyXtra = &(dpyInfo.screenDpyXtra[i]);
	break;
      }
    }
    if (pDpyXtra == NULL) {
      if (opt_debug != 0) {
	(void) fprintf(stderr,"unexpected display[%d] with Atom=%d\n", (int)dpy, (int)(pEv->atom));
      }
#ifdef DEBUG_EXIT
      if (opt_debug != 0) {
	(void) fprintf(stderr, "exiting ProcessPropertyNotify\n\n");
      }
#endif
      return(False);
    }
  }
  if (pEv->atom == pDpyXtra->dpypAtom) {	/* acking a ping */
    if (opt_debug != 0) {
      (void) fprintf(stderr, "  acking a ping, sState=%d\n", pDpyXtra->sState);
      display_dpyxtra();
    }
    pDpyXtra->pingInProg = False;	/* not acking for next time */
    if (pDpyXtra->sState == SELSTATE_WAIT) {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "  SELSTATE_WAIT\n");
      }
      pDpyXtra->sState = SELSTATE_ON;
      XSetSelectionOwner(dpy, XA_PRIMARY, pDpyXtra->propWin, pEv->time);
      if (opt_debug != 0) {
	(void) displayXA(dpy, pDpyXtra->propWin);
      }
    } else if (dpy == pDpyXtra->sDpy) {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "  dpy == pDpyXtra->sDpy\n");
      }
      if (pDpyXtra->sTime == pEv->time) {
/* oops, need to ensure uniqueness */
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  try for another time stamp\n");
	}
	StartGetSelection(dpy, pDpyXtra);	/* try for another time stamp */
      } else {
	if (opt_debug != 0) {
	  (void) fprintf(stderr, "  XConvertSelection\n");
	}
	pDpyXtra->sTime = pEv->time;
/* send selectionrequestion to current selection owner, saying property to */
/* store data in, format to convert to, property to place information in, */
/* window wanting information */
	XConvertSelection(dpy, pDpyXtra->sEvt.selection, pDpyXtra->sEvt.target,
			  XA_PRIMARY, pDpyXtra->propWin, pEv->time);
	if (opt_debug != 0) {
	  (void) displayXA(dpy, pDpyXtra->propWin);
	}
      }
    } else {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "  Uh, why are we here?\n");
      }
    }
  } else {
    if (opt_debug != 0) {
      (void) fprintf(stderr, "  do nothing\n");
    }
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessPropertyNotify\n\n");
  }
#endif
  return(False);
}					/* ProcessPropertyNotify */

/* ------------------------------------------------------------------------- */
static Bool     ProcessSelectionNotify(dpy, pEv)
Display        *dpy;
XSelectionEvent *pEv;
{
  Atom            type;
  int             format;
  unsigned long   nitems;
  unsigned long   after;
  unsigned char  *prop;
  Bool            success;
  XSelectionRequestEvent *pSelReq;
  struct _dpyxtra *oDpyXtra = NULL;
  int i;

#define DEFAULT_PROP_SIZE 1024L

  if (opt_debug != 0) {
    (void) displaydisplay("in ProcessSelectionNotify: ", dpy);
    (void) displayXA(dpy, pEv->requestor);
  }
  if (dpy == fromDpy) {
    oDpyXtra = &(dpyInfo.fromDpyXtra);
  } else {
    for (i = 0; i< number_to; i++) {	/* find display that is StartGetSelection-ing */
      if (dpyInfo.screenDpyXtra[i].sDpy == dpy) {
	oDpyXtra = &(dpyInfo.screenDpyXtra[i]);
	break;
      }
    }
    if (oDpyXtra == NULL) {
      (void) fprintf(stderr, "can't find display that asked for selection\n");
#ifdef DEBUG_EXIT
      if (opt_debug != 0) {
	(void) fprintf(stderr, "exiting ProcessSelectionNotify\n\n");
      }
#endif
      return(False);
    }
  }
  if (opt_debug != 0) {
    displaydisplay("  otherDpy: ", oDpyXtra->sDpy);
    display_dpyxtra();
  }
  if (oDpyXtra->sTime == pEv->time) {
    success = False;
/* corresponding select */
    if (XGetWindowProperty(dpy, pEv->requestor, XA_PRIMARY, 0L,
			   DEFAULT_PROP_SIZE, True, AnyPropertyType,
			   &type, &format, &nitems, &after, &prop) == Success) {
      if ((type != None) && (format != None) && (nitems != 0) &&
	  (prop != None) && (type <= XA_LAST_PREDEFINED)) {	/* known type */
	if (after == 0L) {		/* got everything */
	  success = True;
	} else {			/* try to get everything */
	  XFree(prop);
	  success = ((XGetWindowProperty(dpy, pEv->requestor, XA_PRIMARY, 0L,
			 DEFAULT_PROP_SIZE + after + 1, True, AnyPropertyType,
			 &type, &format, &nitems, &after, &prop) == Success) &&
		     (type != None) && (format != None) && (nitems != 0) &&
		     (after == 0L) && (prop != None));
	  if (opt_debug != 0) {
	    (void) fprintf(stderr, "2nd XGetWindowProperty, a little long\n");
	  }
	}
      } else {
 	if (opt_debug != 0) {
 	  (void) fprintf(stderr, "not known type for XA_PRIMARY\n");
 	}
      }
    } else {
      if (opt_debug != 0) {
	(void) fprintf(stderr, "1st XGetWindowProperty failed?\n");
      }
    }
    pSelReq = &(oDpyXtra->sEvt);
    if (success) {			/* send bits to requesting dpy/window */
      XChangeProperty(pSelReq->display, pSelReq->requestor, pSelReq->property,
		      type, format, PropModeReplace, prop, nitems);
      if (opt_debug != 0) {
	(void) displayXA(pSelReq->display, pSelReq->requestor);
      }
      XFree(prop);
      SendSelectionNotify(pSelReq);
    } else {
      pSelReq->property = None;
      SendSelectionNotify(pSelReq);
    }
    oDpyXtra->sDpy = NULL;
  } else {
    if (opt_debug != 0) {
      (void) fprintf(stderr, "  do nothing -- time wrong?\n");
    }
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessSelectionNotify\n\n");
  }
#endif
  return(False);
}					/* ProcessSelectionNotify */

/* ------------------------------------------------------------------------- */
static Bool     ProcessSelectionClear(dpy, pEv)
Display        *dpy;
XSelectionClearEvent *pEv;
{
  Display        *otherDpy;
  struct _dpyxtra *pDpyXtra = NULL;
  struct _dpyxtra *oDpyXtra;
  int i;
  int fromone = 0;		/* NOT DONE YET -- used to be 0 */

  if (opt_debug != 0) {
    displaydisplay("in ProcessSelectionClear: ", dpy);
    displaydisplay("  pEv->display: ", pEv->display);
    (void) displaywindow("  pEv->window: ", pEv->window);
/*     (void) fprintf(stderr, "serial=%lu send_event=", pEv->serial); */
/*     if (pEv->send_event == 0) { */
/*       (void) fprintf(stderr, "false "); */
/*     } else { */
/*       (void) fprintf(stderr, "true "); */
/*     } */
/*     (void) fprintf(stderr, "time=%ld selection=%-8.8x\n", pEv->time, (unsigned int)pEv->selection); */
    (void) displayXA(dpy,pEv->window);
  }
  if (pEv->selection == XA_PRIMARY) {
/* track primary selection */
    if (dpy == fromDpy) {
      pDpyXtra = &(dpyInfo.fromDpyXtra);
      fromone = 1;
    } else {
      for (i = 0; i <  number_to; i++) {
	if (dpy == c_tDisplay[i]) {
	  pDpyXtra = &(dpyInfo.screenDpyXtra[i]);
	  break;
	}
      }
      if (pDpyXtra == NULL) {
	if (opt_debug != 0) {
	  (void) fprintf(stderr,"ProcessSelectionClear unexpected display[%d]\n", (int)dpy);
#ifdef DEBUG_EXIT
	  (void) fprintf(stderr, "exiting ProcessSelectionClear\n\n");
#endif
	}
	return(False);
      }
    }
    if (opt_debug != 0) {
      display_dpyxtra();
    }
    pDpyXtra->sState = SELSTATE_OFF;
    for (i = 0; i< number_to; i++) {	/* send to all other machines */
      if (dpy != c_tDisplay[i]) {
	otherDpy = c_tDisplay[i];
	if (otherDpy == fromDpy) {
	  oDpyXtra = &(dpyInfo.fromDpyXtra);
	} else {
	  oDpyXtra = &(dpyInfo.screenDpyXtra[i]);
	}
	if (opt_debug != 0) {
	  displaydisplay("  otherDpy: ", otherDpy);
	}
	if (otherDpy != fromDpy || fromone == 0) {
	  oDpyXtra->sState = SELSTATE_WAIT;
	  StartGetSelection(otherDpy, oDpyXtra);	/* get started */
	  if (oDpyXtra->sDpy) {
	    oDpyXtra->sEvt.property = None;
	    if (opt_debug != 0) {
	      (void) fprintf(stderr, "Before SendSelection\n");
	    }
	    SendSelectionNotify(&(oDpyXtra->sEvt));
	    if (opt_debug != 0) {
	      (void) fprintf(stderr, "Before SendSelection\n");
	    }
	    oDpyXtra->sDpy = NULL;
/* This above might be a little wrong. */
	  }
	  if (otherDpy == fromDpy) {
	    fromone = 1;
	  }
	}
      }
    }
    dpyInfo.lastSelectDpy = dpy;		/* try saving just last one */
  } else {
    if (opt_debug != 0) {
      (void) fprintf(stderr, "  selection not XA_PRIMARY (%ld)\n", pEv->selection);
    }
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessSelectionClear\n\n");
  }
#endif
  return(False);
}					/* ProcessSelectionClear */

/* ------------------------------------------------------------------------- */
/* process a visibility event */

static Bool     ProcessVisibility(dpy, pEv)
Display        *dpy;
XVisibilityEvent *pEv;
{
  if (opt_debug != 0) {
    (void) fprintf(stderr, "in ProcessVisibility\n");
  }
/* Might want to qualify, based on other messages.  */
/* Otherwise, this code might cause a loop if two windows decide */
/* to fight it out for the top of the stack */
  if (pEv->state != VisibilityUnobscured) {
    XRaiseWindow(dpy, pEv->window);
  }
#ifdef DEBUG_EXIT
  if (opt_debug != 0) {
    (void) fprintf(stderr, "exiting ProcessVisibility\n\n");
  }
#endif
  return(False);
}					/* ProcessVisibility */

/* ------------------------------------------------------------------------- */
/* process a keyboard mapping event */

static Bool     ProcessMapping(dpy, pEv)
Display        *dpy;
XMappingEvent  *pEv;
{
  if (opt_debug != 0) {
    (void) fprintf(stderr, "in ProcessMapping\n");
  }
  switch (pEv->request) {
    case MappingModifier:
    case MappingKeyboard:
      XRefreshKeyboardMapping(pEv);
      break;
    case MappingPointer:
      RefreshPointerMapping(dpy);
      break;
  }
/* #ifdef DEBUG_EXIT */
/* (void) fprintf(stderr, "exiting ProcessMapping\n\n"); */
/* #endif */
  return(False);
}					/* ProcessMapping */

/* ------------------------------------------------------------------------- */
