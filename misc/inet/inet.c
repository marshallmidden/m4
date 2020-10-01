/* Inet program, who knows where we got it... */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#if	BSD >= 43
#include <arpa/inet.h>
#else	/* BSD <= 42 */
 /* 4.2 and SUN arpa/inet.h misdeclare inet_addr. */
extern unsigned long inet_addr();
#endif	/* BSD <= 42 */
#include <ctype.h>
#include <arpa/nameser.h>
#include <resolv.h>
#ifndef NAMESERVER_PORT
#define NAMESERVER_PORT 53
#endif

/* ------------------------------------------------------------------------- */
extern char    *inet_ntoa();
// extern char    *index(char *, char);
#include <strings.h>
// extern int	fprintf();
// extern int	printf();
// extern void	bcopy(char *, char *, int);
// extern void	bzero(char *, int);

extern int      errno;
extern int      h_errno;
/* ------------------------------------------------------------------------- */
char           *progname = "(Unknown)";


char           *h_errors[] = {
  "no error", "HOST_NOT_FOUND", "TRY_AGAIN", "NO_RECOVERY", "NO_DATA"
};

/* ------------------------------------------------------------------------- */
int             main(argc, argv)
int             argc;
char           *argv[];
{
  int    i;
  unsigned long   in;
  struct sockaddr_in inet_sin;
  struct hostent *hp;
  char  *p;
  char           *pport;
  int             ishost = 0;
  int             altns = 0;
  int		  nameonly = 0;
  int		  iponly = 0;
  int		  flag;

  if (argc > 0)
    progname = argv[0];
  if (argc < 2)
    goto Usage;
  if (res_init() < 0) {
    (void) fprintf(stderr, "%s: res_init() failed.\n", progname);
    exit(1);
  }
  hp = NULL;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
	case 'v':			/* use virtual circuit */
	  _res.options |= RES_USEVC;
	  break;
	case 'd':			/* enable resolver debug output */
	  _res.options |= RES_DEBUG;
	  break;
	case 's':
	  if (*(p = &(argv[i][2])) == '\0')
	    p = argv[++i];
	  if ((pport = index(p, ':')) != NULL)
	    *pport++ = '\0';
	  bzero((char *)(&inet_sin), sizeof(inet_sin));
	  inet_sin.sin_family = AF_INET;
	  if ((inet_sin.sin_addr.s_addr = inet_addr(p)) == (unsigned int)-1) {
	    hp = gethostbyname(p);
	    if (hp == NULL) {
	      (void) fprintf(stderr,
			     "%s: unknown host\n", p);
	      exit(2);
	    }
	    bcopy(hp->h_addr, (char *)(&inet_sin.sin_addr),
		  hp->h_length);
	  }
	  if (pport != NULL)
	    inet_sin.sin_port = htons(atoi(pport));
	  if (inet_sin.sin_port == 0)
	    inet_sin.sin_port = htons(NAMESERVER_PORT);
	  if (altns == 0) {
	    altns++;
	    _res.nscount = 0;
	  }
	  if (_res.nscount >= MAXNS) {
	    (void) fprintf(stderr,
			   "Can only specify %d name servers\n",
			   MAXNS);
	    exit(3);
	  }
	  _res.nsaddr_list[_res.nscount++] = inet_sin;
	  break;
	case 'h':
	  ishost++;
	  break;
	case 'n':
	  nameonly = 1;
	  iponly = 0;
	  break;
	case 'i':
	  iponly = 1;
	  nameonly = 0;
	  break;
	default:
	Usage:
	  (void) fprintf(stderr, "\
Usage: %s [-d] [-v] [-h] [-s nameserver[:port]] host-name-or-address ...\n\
	-d Enable debug (display queries and responses)\n\
	-v Use virtual circuit (TCP rather than UDP)\n\
	-h Force lookup as host name even if it looks like IP address\n\
	-n Print out names, not IP address(es)\n\
	-i Print out IP address(es), not names\n\
	-s Ask that nameserver rather than those in /etc/resolv.conf.\n\
",
			 progname);
	  exit(1);
      }
      continue;
    }
    if (ishost || isalpha(argv[i][0])) {
      hp = gethostbyname(argv[i]);
    } else if ((in = inet_addr(argv[i])) != (unsigned int)-1) {
      hp = gethostbyaddr((char *)(&in), sizeof(in), AF_INET);
    }
    if (nameonly == 0 && iponly == 0) {
      printf("%s: ", argv[i]);
    }
    if (hp != NULL) {
      if (nameonly == 1) {		/* if only name, print it */
	printf("%s", hp->h_name);
      } else {
	flag = 0;			/* if need to print a leading space */
	if (iponly == 0) {		/* if both */
	  printf("host %s <=> ", hp->h_name);
	}
	while (*hp->h_addr_list != NULL) {
	  bcopy(hp->h_addr_list[0], (char *) &in, hp->h_length);
	  inet_sin.sin_addr.s_addr = in;
	  if (flag != 0) {
	    printf(" ");
	  }
	  printf("%s", inet_ntoa(inet_sin.sin_addr));
	  hp->h_addr_list++;
	  flag = 1;
	}
      }
      printf("\n");
    } else {
      printf("unknown (%s)\n", h_errors[h_errno]);
    }
  }
  return(0);
}

/* ------------------------------------------------------------------------- */
/* End of file inet.c */
