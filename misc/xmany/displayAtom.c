#include <stdio.h>

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

/* ------------------------------------------------------------------------- */
extern void displaydisplay();
extern void displaywindow();
/* ------------------------------------------------------------------------- */
void displayit(dpy, prop, str)
Display *dpy;
Atom prop;
char *str;
{
  Atom actual_type;
  int actual_format;
  unsigned long nitems;
  unsigned long leftover;
  unsigned char *data;
  char *name;

  name = XGetAtomName(dpy, prop);
  /* req_type was XA_STRING */
  if (XGetWindowProperty(dpy, RootWindow(dpy, 0), prop,
			 0L, 10000000L, False, AnyPropertyType,
		  	 &actual_type, &actual_format, &nitems, &leftover,
			 &data) != Success) {
    (void) fprintf(stderr, "  %s (%s): XGetWindowProperty error?\n", str, name);
    return;
  }
  (void) fprintf(stderr, "  %s (%s): type=%lu, format=%d, nitems=%lu, leftover=%lu",
		 str, name, actual_type, actual_format, nitems, leftover);
  if (data == NULL) {
    (void) fprintf(stderr, "-- no data\n");
    return;
  }
  if (actual_type == XA_STRING && actual_format == 8) {
    (void) fprintf(stderr, "  data = (%s)\n", data);
  } else {
    (void) fprintf(stderr, "  -- actual_type!=%lu or actual_format!=8\n", 
		XA_STRING);
  }
  XFree(data);
}					/* displayit */

/* ------------------------------------------------------------------------- */
void displayXA(dpy, w)
Display *dpy;
Window w;
{
  displaydisplay("  display: ", dpy);
  displaywindow ("  Window: ", w);
/*   displayit(dpy, XA_PRIMARY, "XA_PRIMARY"); */
/*   displayit(dpy, XA_SECONDARY, "XA_SECONDARY"); */
/*   displayit(dpy, XA_ARC, "XA_ARC"); */
/*   displayit(dpy, XA_ATOM, "XA_ATOM"); */
/*   displayit(dpy, XA_CARDINAL, "XA_CARDINAL"); */
  displayit(dpy, XA_CUT_BUFFER0, "XA_CUT_BUFFER0");
/*   displayit(dpy, XA_CUT_BUFFER1, "XA_CUT_BUFFER1"); */
/*   displayit(dpy, XA_CUT_BUFFER2, "XA_CUT_BUFFER2"); */
/*   displayit(dpy, XA_CUT_BUFFER3, "XA_CUT_BUFFER3"); */
/*   displayit(dpy, XA_CUT_BUFFER4, "XA_CUT_BUFFER4"); */
/*   displayit(dpy, XA_CUT_BUFFER5, "XA_CUT_BUFFER5"); */
/*   displayit(dpy, XA_CUT_BUFFER6, "XA_CUT_BUFFER6"); */
/*   displayit(dpy, XA_CUT_BUFFER7, "XA_CUT_BUFFER7"); */
/*   displayit(dpy, XA_CUT_BUFFER7, "XA_CUT_BUFFER7"); */
/*   displayit(dpy, XA_DRAWABLE, "XA_DRAWABLE"); */
/*   displayit(dpy, XA_STRING, "XA_STRING"); */
/*   displayit(dpy, XA_WINDOW, "XA_WINDOW"); */
/*   displayit(dpy, XA_WM_CLIENT_MACHINE, "XA_WM_CLIENT_MACHINE"); */
/*   displayit(dpy, XA_WM_NAME, "XA_WM_NAME"); */
}					/* displayXA */

/* ------------------------------------------------------------------------- */
