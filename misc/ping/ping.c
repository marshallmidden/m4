#define IPHEAD				/* IPHEAD mods: raw socket returns entire packet, including IP
					   header.  Required for SunOS4 and DEC-3100 */

/*
 *                      P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *      Mike Muuss
 *      U. S. Army Ballistic Research Laboratory
 *      December, 1983
 *
 * Target System -
 *      4.2 BSD with MIT and BRL fixes to /sys/netinet/ip_icmp.c et.al.
 *
 * Status -
 *      Public Domain.  Distribution Unlimited.
 *
 * Bugs -
 *      Divide by zero if no packets return.
 *      More statistics could always be gathered.
 *      This program has to run SUID to ROOT to access the ICMP socket.

------------------------------------------------------------------------------
 HISTORY @ University of Minnesota
 05/17/94	M.Midden Changing % packets lost to be floating number.
 09/28/91	M.Midden added -A option, and changed exit status to be:
		0 if device is up, and fast (if -u option used).
                1 if error in command/ip address/etc.
                2 if device dropped all packets.
                3 if box dropped at least one packet, but got some.
                4 if box responded slowly

 09/4/91	M.Midden added -S, -N options.
 ??/??/??	Craig Finseth put it up.
------------------------------------------------------------------------------
 */
#ifndef lint
static char     RCSid[] = "@(#)$Header: /home/m4/.CVS/src/ping/ping.c,v 1.1.1.1 2002/05/03 23:32:34 m4 Exp $ (BRL)";
#endif

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>

#define PACKETSIZE 56			/* old kernel bug needs this */
#define MAXPACKET 32768
int             packetsize = PACKETSIZE;
u_char          packet[MAXPACKET];

#define DEBUG 1
#define VERBOSE 2
#define UPTEST 4
#define SILENT 8
#define NUMBER 16
#define STATUS 32
#define NOEXTRA 64
#define AVERAGE 128
int             options = 0;

extern int      errno;

int             s;			/* Socket file descriptor */
struct hostent *hp;			/* Pointer to host info */
struct timezone tz;			/* leftover */

struct sockaddr whereto;		/* Who to ping */
unsigned        maxping = 65535 * 256;	/* How many times to ping */
int             lastseqcheck = 0;	/* last packet sequence transmitted */
int             fastping = 0;		/* if non-zero don't wait between pings */

char           *hostname;
char           *hostinetaddr;

int             ntransmitted = 0;	/* sequence # for outbound packets = #sent */
int             ident;

int             nreceived = 0;		/* # of packets we got back */
int             pingdelay = 1;		/* time in seconds between pings */
long            tmin = 999999999;
long            tmax = 0;
long            tsum = 0;		/* sum of all times, for doing average */
__sighandler_t  finish(void);




/*
 *      usage - print short help and exit
 */
void            usage()
{
  printf("Usage:  ping [-n][-c n][-d][-u][-s][-v][-p n][-t n] host\n");
  printf("-c n  - number of times to 'ping' host\n");
  printf("-d    - debug mode\n");
  printf("-n    - host must be an ip address, will not be verified\n");
  printf("-p n  - set packet size\n");
  printf("-t n  - set timeout in seconds\n");
  printf("-u    - determine if host is active, implies -c 1\n");
  printf("-s    - does its work silently, use with -u\n");
  printf("-S    - return status variable set (-s also does this)\n");
  printf("-N    - do not print out wrongly received icmp packets\n");
  printf("-A    - print out averages\n");
  printf("-v    - verbose mode\n");
  printf("host  - name or IP number of host\n");
  exit(1);
}	/* end of usage */

/*
 *                      C A T C H E R
 *
 * This routine causes another PING to be transmitted, and then
 * schedules another SIGALRM for 'pingdelay' seconds from now.
 *
 * Bug -
 *      Our sense of time will slowly skew (ie, packets will not be launched
 *      exactly at 1-second intervals).  This does not affect the quality
 *      of the delay and loss statistics.
 */
__sighandler_t catcher(void)
{
/*    if (nreceived == ntransmitted) */
  if (maxping && 
    !((nreceived > 0) && (nreceived == ntransmitted) && (options & UPTEST))) {
    if (ntransmitted > nreceived && options & UPTEST) {	/* if send one, but it was slow */
      options &= ~UPTEST;		/* turn off updown test. */
    }
    if (fastping) {			/* check for missing response */
      if (lastseqcheck == ntransmitted) {	/* missing more than one second, reping */
	--maxping;
	pinger();
      }
      lastseqcheck = ntransmitted;
    }
    else {				/* reping */
      --maxping;
      pinger();
    }

    if (pingdelay)
      alarm(pingdelay);
  }
  else
    finish();				/* end of ping sequence */
}	/* end of catcher */


/*
 *                      M A I N
 */
main(argc, argv)
char           *argv[];
{
  struct protoent *pp;
  char           *hostarg = (char *) NULL;
  int             i;
  struct sockaddr_in from;
  int             argi = 1;
  struct sockaddr_in *to = (struct sockaddr_in *) & whereto;

  if (argi >= argc) {
    usage();
  }

  while (argi < argc) {
    if (argv[argi][0] == '-') {		/* process command line flag */
      if (!strcmp(argv[argi], "-c")) {
	maxping = atoi(argv[++argi]);
      }
      else if (!strcmp(argv[argi], "-d")) {
	options |= DEBUG;
      }
      else if (!strcmp(argv[argi], "-n")) {
	options |= NUMBER;
      }
      else if (!strcmp(argv[argi], "-u")) {
	options |= UPTEST;
	maxping = 1;
      }
      else if (!strcmp(argv[argi], "-v")) {
	options |= VERBOSE;
      }
      else if (!strcmp(argv[argi], "-p")) {
	packetsize = atoi(argv[++argi]);
      }
      else if (!strcmp(argv[argi], "-s")) {
	options |= SILENT;
      }
      else if (!strcmp(argv[argi], "-S")) {
	options |= STATUS;
      }
      else if (!strcmp(argv[argi], "-N")) {
	options |= NOEXTRA;
      }
      else if (!strcmp(argv[argi], "-A")) {
	options |= AVERAGE;
      }
      else if (!strcmp(argv[argi], "-t")) {
	pingdelay = atoi(argv[++argi]);
      }
      else {
	fprintf(stderr, "%s: bad argument:%s\n",
		argv[0], argv[argi]);
	usage();
      }
    }
    else {
      hostarg = argv[argi];
    }

    ++argi;
  }

  if (options & SILENT)
    options &= ~VERBOSE;

  if (hostarg == (char *) NULL) {
    fprintf(stderr, "%s: host name not specified\n", argv[0]);
    usage();
  }

  bzero((char *) &whereto, sizeof(struct sockaddr));
  if (((hostarg[0] >= '0') && (hostarg[0] <= '9')) ||
      (options & NUMBER))
    hp = (struct hostent *) NULL;
  else
    hp = gethostbyname(hostarg);

  if (hp) {
    bcopy(hp->h_addr, (char *) &to->sin_addr, hp->h_length);
    to->sin_family = hp->h_addrtype;
  }
  else {
    if (options & NUMBER)
      hp = (struct hostent *) NULL;
    else {				/* look up IP address in host table */
      to->sin_addr.s_addr = inet_addr(hostarg);
      hp = gethostbyaddr(&to->sin_addr, 4, AF_INET);
    }

    to->sin_family = AF_INET;
    to->sin_addr.s_addr = inet_addr(hostarg);
    if (to->sin_addr.s_addr == -1) {
      printf("ping: unknown host %s\n", hostarg);
      exit(1);
    }
  }
  if (hp == NULL)
    hostname = "unknown";
  else
    hostname = hp->h_name;
  hostinetaddr = inet_ntoa(to->sin_addr);
  to->sin_port = 0;

  if ((options & VERBOSE) && hp) {
    printf("host name: %s (%s)\nalias:", hostname, hostinetaddr);
    i = 0;
    while (hp->h_aliases[i] != 0) {
      printf("\t(%s)\n", hp->h_aliases[i++]);
    }
    if (i == 0)
      putchar('\n');
    printf("address type: %c\n", '@' + hp->h_addrtype);
    printf("address length: %d\n", hp->h_length);
  }

  ident = getpid() & 0xFFFF;

  pp = getprotobyname("ICMP");
  if (pp == (struct protoent *) NULL) {	/* fatal error */
    perror("ping: getprotobyname");
    exit(1);
  }

  while ((s = socket(AF_INET, SOCK_RAW, pp->p_proto)) < 0) {
    perror("ping: socket");
    sleep(5);
  }

  setreuid(geteuid(), geteuid());

  if (packetsize < (ICMP_MINLEN + sizeof(struct timeval)))
    packetsize = ICMP_MINLEN + sizeof(struct timeval);

  if (((!(options & UPTEST)) || (options & VERBOSE)) &&
      (!(options & SILENT))) {
    printf("PING %s (%s): %d data bytes\n",
	   hostname, hostinetaddr, packetsize);
    printf("none     0");
  }
  else if (!(options & SILENT))
    printf("%s (%s) is ", hostname, hostinetaddr);
  fflush(stdout);

  setlinebuf(stdout);

  signal(SIGINT, (__sighandler_t)finish);

  if (pingdelay <= 0) {
    pingdelay = 1;
    fastping = 1;
    lastseqcheck = ntransmitted;
  }

  if (pingdelay)
    signal(SIGALRM, (__sighandler_t)catcher);
  catcher();				/* start things going */

  for (;;) {
    int             len = sizeof(packet);
    int             fromlen = sizeof(from);
    int             cc;

 /* cc = recvfrom(s, buf, len, flags, from, fromlen) */
    if ((cc = recvfrom(s, packet, len, 0, (struct sockaddr *)&from, &fromlen)) < 0) {	/* error */
      if (errno == EINTR)
	continue;
      perror("ping: recvfrom");
      continue;
    }
    pr_pack(packet, cc, &from, "from");
    if ((maxping == 0) && (ntransmitted == nreceived))
      finish();
    fflush(stdout);

    if (fastping && maxping) {
      --maxping;
      pinger();
    }
  }
 /* NOT REACHED */
}	/* end of main */

/*
 *                      P I N G E R
 *
 * Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
pinger()
{
  static u_char   outpack[MAXPACKET];
  register struct icmp *icp = (struct icmp *) outpack;
  int             i;
  int             cc;
  register struct timeval *tp = (struct timeval *) & outpack[ICMP_MINLEN];
  register u_char *datap = &outpack[ICMP_MINLEN + sizeof(struct timeval)];

  icp->icmp_type = ICMP_ECHO;
  icp->icmp_code = 0;
  icp->icmp_cksum = 0;
  icp->icmp_seq = ++ntransmitted;
  icp->icmp_id = ident;			/* ID */

  cc = packetsize;
  if (cc > MAXPACKET)
    cc = MAXPACKET;

  gettimeofday(tp, &tz);

  for (i = ICMP_MINLEN + sizeof(struct timeval); i < packetsize; i++)
    *datap++ = i & 0xff;

 /* Compute ICMP checksum here */
  icp->icmp_cksum = in_cksum(icp, cc);

 /* cc = sendto(s, msg, len, flags, to, tolen) */
  i = sendto(s, outpack, cc, 0, &whereto, sizeof(struct sockaddr));

  if (options & DEBUG)
    p_packet(outpack, cc, &whereto, "to");

  if (i < 0 || i != cc) {
    if (i < 0)
      perror("sendto");
    printf("ping: wrote %s %d chars, ret=%d\n",
	   hostname, cc, i);
    fflush(stdout);
  }

  if ((!(options & UPTEST)) && (!(options & SILENT))) {
    printf("\r\t");
    if (options & VERBOSE)
      printf("snd#");
    printf("%4d", ntransmitted);
    fflush(stdout);
  }
}	/* end of pinger */

/*
 *                      P R _ T Y P E
 *
 * Convert an ICMP "type" field to a printable string.
 */
char           *pr_type(t)
register int    t;
{
  static char    *ttab[] = {
    "Echo Reply",
    "ICMP 1",
    "ICMP 2",
    "Dest Unreachable",
    "Source Quence",
    "Redirect",
    "ICMP 6",
    "ICMP 7",
    "Echo",
    "ICMP 9",
    "ICMP 10",
    "Time Exceeded",
    "Parameter Problem",
    "Timestamp",
    "Timestamp Reply",
    "Info Request",
    "Info Reply"
  };

  if (t < 0 || t > 16)
    return ("OUT-OF-RANGE");

  return (ttab[t]);
}	/* end of pr_type */

/*
 *                      P R _ C O D E _ U R
 *
 * Convert an ICMP unreachable "code" field to a printable string.
 */
char           *pr_code_ur(c)
register int    c;
{
  static char    *urctab[] = {
  "net",
  "host",
  "proto",
  "port",
  "need frag",
  "src rt fail"
  };

  if (c < 0 || c > ICMP_UNREACH_SRCFAIL)
    return ("OUT-OF-RANGE");

  return (urctab[c]);
}	/* end of pr_code_ur */




p_packet(icp, cc, from, dir)
register struct icmp *icp;
int             cc;
struct sockaddr_in *from;
char           *dir;
{
  register long  *lp = (long *) icp;
  register int    i;

  from->sin_addr.s_addr = ntohl(from->sin_addr.s_addr);

  printf("\n%d bytes %s %s:\n", cc, dir, inet_ntoa(from->sin_addr));
  printf("icmp_type=%X (%s), ", icp->icmp_type, pr_type(icp->icmp_type));
  printf("icmp_code=%X", icp->icmp_code);
  if (icp->icmp_type == ICMP_UNREACH)
    printf(" (%s), ", pr_code_ur(icp->icmp_code));
  else
    printf(", ");
  printf("icmp_cksum=%X\n", icp->icmp_cksum);
  printf("icmp_id=%04X, icmp_seq=%04X\n",
	 icp->icmp_id, icp->icmp_seq);
  for (i = 0; i < cc; i += sizeof(long))
    printf("x%2.2X: x%8.8X\n", i, *lp++);
}	/* end of p_packet */


/*
 *                      P R _ P A C K
 *
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
pr_pack(icp, cc, from, dir)
register struct icmp *icp;
int             cc;
struct sockaddr_in *from;
char           *dir;
{
  register long  *lp = (long *) icp;
  register int    i;
  struct timeval  tv;
  struct timeval *tp = (struct timeval *) & packet[ICMP_MINLEN];
  long            triptime;
#if !defined(IPHEAD)
  register u_char *datap =
  &packet[ICMP_MINLEN + sizeof(struct timeval)];
#else
  register u_char *datap;
  u_char         *start;

  start = (u_char *) ((long *) packet + (packet[0] & 0xF));
  icp = (struct icmp *) start;
  tp = (struct timeval *) (start + ICMP_MINLEN);
  datap = start + (ICMP_MINLEN + sizeof(struct timeval));
#endif

  from->sin_addr.s_addr = ntohl(from->sin_addr.s_addr);
  gettimeofday(&tv, &tz);

  if (((icp->icmp_type != ICMP_ECHOREPLY) && !(options&NOEXTRA))
	|| (options & DEBUG)) {
    p_packet(icp, cc, from, dir);
  }

  if (icp->icmp_id != ident)
    return;				/* 'Twas not our ECHO */

  for (i = ICMP_MINLEN + sizeof(struct timeval); i < packetsize; i++)
    if (*datap++ != (i & 0xff)) {	/* bad data in packet */
      if (options & (VERBOSE | DEBUG))
	printf("Bad data in received packet, byte %x, %x != %x.\n",
	       i, *--datap, i & 0xff);
      return;
    }

  tvsub(&tv, tp);
  triptime = tv.tv_sec * 1000000 + (tv.tv_usec);
  tsum += triptime;
  if (triptime < tmin)
    tmin = triptime;
  if (triptime > tmax)
    tmax = triptime;

  if (options & VERBOSE) {
    printf(" %d bytes from %s: ", cc, hostname);
    printf("rec#%4d,", icp->icmp_seq);
    printf("time=%4d ms\n", triptime / 1000);
  }
  else if ((!(options & UPTEST)) && (!(options & SILENT))) {
 /* printf("\n%4d",icp->icmp_seq); */
    printf("\r%4d", nreceived + 1);
    fflush(stdout);
  }

  nreceived++;
}	/* end of pr_pack */


/*
 *                      I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers.
 */
in_cksum(addr, len)
u_short        *addr;
int             len;
{
#ifndef vax
  register int    cnt = len;
  register u_short *ptr = addr;
  register int    sum = 0;


  while ((cnt -= 2) >= 0)
    sum += *ptr++;
  if (cnt == -1)
    sum += *ptr & 0xFF00;
  sum = (sum >> 16) + (sum & 0xFFFF);
  return (~sum & 0xFFFF);
#else
/*
 * Checksum routine for Internet Protocol family headers (VAX Version).
 *
 * Shamelessly pilfered from /sys/vax/in_cksum.c, with all the MBUF stuff
 * ripped out.
 */
  register int    nleft = len;		/* on vax, (user mode), r11 */
  register int    xxx;			/* on vax, (user mode), r10 */
  register u_short *w = addr;		/* on vax, known to be r9 */
  register int    sum = 0;		/* on vax, known to be r8 */


 /* Force to long boundary so we do longword aligned memory operations.  It
    is too hard to do byte adjustment, do only word adjustment. */
  if (((int) w & 0x2) && nleft >= 2) {
    sum += *w++;
    nleft -= 2;
  }
 /* Do as much of the checksum as possible 32 bits at a time. In fact, this
    loop is unrolled to make overhead from branches &c small.
 
 We can do a 16 bit ones complement sum 32 bits at a time because the 32 bit
    register is acting as two 16 bit registers for adding, with carries from
    the low added into the high (by normal carry-chaining) and carries from
    the high carried into the low on the next word by use of the adwc
    instruction.  This lets us run this loop at almost memory speed.
 
 Here there is the danger of high order carry out, and we carefully use adwc. */
  while ((nleft -= 32) >= 0) {
#undef ADD
    asm("clrl r0");			/* clears carry */
#define ADD             asm("adwc (r9)+,r8;");
    ADD;
    ADD;
    ADD;
    ADD;
    ADD;
    ADD;
    ADD;
    ADD;
    asm("adwc $0,r8");
  }
  nleft += 32;
  while ((nleft -= 8) >= 0) {
    asm("clrl r0");
    ADD;
    ADD;
    asm("adwc $0,r8");
  }
  nleft += 8;
 /* Now eliminate the possibility of carry-out's by folding back to a 16 bit
    number (adding high and low parts together.)  Then mop up trailing words
    and maybe an odd byte. */
  {
    asm("ashl $-16,r8,r0; addw2 r0,r8");
    asm("adwc $0,r8; movzwl r8,r8");
  }
  while ((nleft -= 2) >= 0) {
    asm("movzwl (r9)+,r0; addl2 r0,r8");
  }
  if (nleft == -1) {
    sum += *(u_char *) w;
  }

 /* Add together high and low parts of sum and carry to get cksum. Have to be
    careful to not drop the last carry here. */
  {
    asm("ashl $-16,r8,r0; addw2 r0,r8; adwc $0,r8");
    asm("mcoml r8,r8; movzwl r8,r8");
  }
  return (sum);
#endif
}	/* end of in_cksum */

/*
 *                      T V S U B
 *
 * Subtract 2 timeval structs:  out = out - in.
 *
 * Out is assumed to be >= in.
 */
tvsub(out, in)
register struct timeval *out,
               *in;
{
  if ((out->tv_usec -= in->tv_usec) < 0) {
    out->tv_sec--;
    out->tv_usec += 1000000;
  }
  out->tv_sec -= in->tv_sec;
}	/* end of tvsub */

/*
 *                      F I N I S H
 *
 * Print out statistics, and give up.
 * Heavily buffered STDIO is used here, so that all the statistics
 * will be written with 1 sys-write call.  This is nice when more
 * than one copy of the program is running on a terminal;  it prevents
 * the statistics output from becomming intermingled.
 */
__sighandler_t finish(void)
{
  if ((!(options & UPTEST)) && (!(options & SILENT))) {
    printf("\n---- %s (%s) PING Statistics ----\n",
	   hostname, hostinetaddr);
    printf("%d packets transmitted, ", ntransmitted);
    printf("%d packets received, ", nreceived);
    if (ntransmitted) {
      int per;

      per = (((ntransmitted - nreceived) * 100000) / ntransmitted);
      printf("%d.%3.3d%% packet loss\n", per/1000, per % 1000);
    }
    if (nreceived && !(options & AVERAGE)) {
      printf("round-trip  min/avg/max = %d/%d/%d ms\n",
	     tmin / 1000,
	     tsum / 1000 / nreceived,
	     tmax / 1000);
    }
    goto almost_done;
  }
  if (options & AVERAGE) {
    if (nreceived) {
      printf("Sent:%d  Received:%d  Min/Avg/Max = %d/%d/%d ms\n",
	     ntransmitted, nreceived,
	     tmin / 1000,
	     tsum / 1000 / nreceived,
	     tmax / 1000);
    }
    else {
      printf("Sent:%d  Received:0  Min/Avg/Max = 0/0/0 ms\n",
	     ntransmitted);
    }
    goto almost_done;
  }

  if (!(options & SILENT)) {
    if (nreceived)
      printf("up\n");
    else
      printf("down\n");
  }
  else {
    goto exit_status;
  }

almost_done:
  fflush(stdout);
  if (options & STATUS) {
exit_status:
    if ((nreceived == ntransmitted) && (nreceived == 1)) {	/* ok, and quick up */
      exit(0);
    }
    else if (nreceived == 0) {				/* nothing returned */
      exit(2);
    }
    else if (nreceived != ntransmitted) {		/* packet dropped */
      exit(3);
    }
    else {						/* was slow */
      exit(4);
    }
  }
  else {				/* otherwise, just ok */
    exit(0);
  }
}	/* end of finish */
