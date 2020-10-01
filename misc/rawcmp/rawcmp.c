/* This is the equivalent of "cmp file1 file2 bs=", but it uses 3 circular buffers */
/* to read each of the data files into.  This does not do any conversions whatsoever! */
/*   -v option lists the block number:characters read in block. */
/*   -fill option fills the input block. */
/* if "ef=" is specified, then stdout and stderr are written the same. */
/* if "-20' is given, then 20 circular buffers are use, specify nn. */
/* A file name of '-' means to use standard file descriptor. */

/* Exit status of 1 means that it failed for reason -- see printed message. */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

/* Default number of circular buffers. */
#define	DEFAULTBUFFERS	3

static struct timeval limited;		/* select timeout value */
static struct timeval *timelimit;
static int      maxfd;			/* max file descriptor for select */

static long     full_records = 0;	/* number full records read. */
static long     partial_records = 0;	/* number partial records read. */
static int	verbose = 0;		/* not verbose mode. */
static int	fillinput = 0;		/* do not fill input blocks. */
static int	nbuffers = DEFAULTBUFFERS;	/* number of buffers */
static int	usestderr = 0;		/* do not use stderr */

/* ------------------------------------------------------------------------- */
static void stats()
{
  if (usestderr == 0) {
    (void) fprintf(stderr, "%ld full, and %ld partial records processed\n",
		 full_records, partial_records);
  }
}					/* end of stats */

/* ------------------------------------------------------------------------- */
static void terminate(code)
int	code;
{
  stats();
  exit(code);
}					/* end of terminate */

/* ------------------------------------------------------------------------- */
int             main(argc, argv)
int             argc;
char          **argv;
{
  size_t          block_size = 512;	/* buffer size to use, default 512 */
  char          **buffer;		/* for multiple buffer reading */
  int            *read_chars;		/* number of characters in buffer */
  int            *buffer_empty;		/* anything in buffer */
  int            *stderr_empty;		/* anything in buffer */
  int             ifd1;			/* input file descriptor */
  int             ifd2;			/* input file descriptor */
  char           *input_file1 = NULL;	/* input file name/device */
  char           *input_file2 = NULL;	/* input file name/device */
  int             iwhich;		/* current input buffer location */
  char           *ibuff;		/* used for input reading */
  int             owhich;		/* current output buffer location */
  char           *obuff;		/* used for output writing */
  char           *stderr_file = NULL;	/* output file name/device */
  int             ewhich;		/* current output buffer location */
  char           *ebuff;		/* used for output writing */
  int             efd;			/* stderr file descriptor */
  int		  input_done = 0;	/* flag, non-zero means done. */
  int		  output_done = 0;	/* flag, non-zero means done. */
  int		  stderr_done = 0;	/* flag, non-zero means done. */
  fd_set          ibits;		/* input select fd's */
  fd_set          obits;		/* output select fd's */
  int             n;			/* select return */
  int             tmp;
  int             temp1;
  int             temp2;
  int             cnt;			/* counter */
  char           *argon;		/* char on while processing argument */
  int             return_status = 0;	/* program exit status (main loop). */
  int		  block = 1;		/* blocks read (for verbose mode). */

/* Parse the arguments. */
  for (tmp = 1; tmp < argc; tmp++) {	/* process arguments */
    if (strncmp(argv[tmp], "bs=", 3) == 0) {	/* buffer size */
      argon = argv[tmp] + 3;		/* move past the "bs=" characters. */
      block_size = 0;			/* zero the block size */
/* Get number, possibly followed by one of '[kKbB]'. */
      while (*argon >= '0' && *argon <= '9') {
/* Shift old number by 10 (decimal), and new number added. */
	block_size = (block_size * 10) + (*argon++ - '0');
      }
      if (*argon == 'g' || *argon == 'G') {
	block_size *= 1024*1024*1024;	/* multiply by one G. */
	argon++;
      } else if (*argon == 'm' || *argon == 'M') {
	block_size *= 1024*1024;	/* multiply by one M. */
	argon++;
      }else if (*argon == 'k' || *argon == 'K') {
	block_size *= 1024;		/* multiply by one K. */
	argon++;
      } else if (*argon == 'b' || *argon == 'B') {
	block_size *= 512;		/* multiple by one disk block */
	argon++;
      }
      if (*argon != '\0') {		/* If not at end of number... */
	(void) fprintf(stderr, "%s: Bad char in bs= number parsing: %c\n", argv[0], *argon);
	exit(1);
      }
    } else if (strncmp(argv[tmp], "ef=", 3) == 0) {	/* stderr file */
      stderr_file = argv[tmp]+3;
    } else if (strncmp(argv[tmp], "-v", 2) == 0) {	/* verbose mode */
      verbose++;
    } else if (strncmp(argv[tmp], "-fill", 5) == 0) {	/* fill input mode */
      fillinput++;
    } else if (*argv[tmp] == '-') {			/* possible number */
      argon = argv[tmp] + 1;		/* move past the '-' character. */
      nbuffers = 0;
      while (*argon >= '0' && *argon <= '9') {
/* Shift old number by 10 (decimal), and new number added. */
	nbuffers = (nbuffers * 10) + (*argon++ - '0');
      }
      if (*argon != '\0') {
	(void) fprintf(stderr, "%s: bad character for number of buffers: %c\n", argv[0], *argon);
	exit(1);
      }
    } else if (input_file1 == (char*)NULL) {
      input_file1 = argv[tmp];
    } else if (input_file2 == (char*)NULL) {
      input_file2 = argv[tmp];
    } else {
      (void) fprintf(stderr, "%s: bad argument: %s\n", argv[0], argv[tmp]);
      exit(1);
    }
  }					/* end of for() all arguments */

/* Arguments parsed, open and check file names. */
  if (input_file1 == NULL) {
    (void) fprintf(stderr, "%s: Did not pass in two input files.\n", argv[0]);
    exit(1);
  }
  if (input_file1 != NULL && strcmp(input_file1,"-") != 0) { /* open input */
    ifd1 = open(input_file1, 0);
    if (ifd1 < 0) {			/* if error, quit */
      perror(input_file1);
      exit(1);
    }
  } else {
    ifd1 = dup(0);			/* or stdin */
    if (ifd1 < 0) {			/* if error, quit */
      perror(argv[0]);
      exit(1);
    }
  }
  if (input_file2 != NULL && strcmp(input_file2,"-") != 0) { /* open output */
    ifd2 = open(input_file2, 0);
    if (ifd2 < 0) {			/* if error, quit */
      perror(output_file);
      exit(1);
    }
  } else {
    ifd2 = dup(1);			/* or stdout */
    if (ifd2 < 0) {			/* if error, quit */
      perror(argv[0]);
      exit(1);
    }
  }
  if (stderr_file != NULL && strcmp(stderr_file,"-") != 0) { /* open stderr */
    efd = creat(stderr_file, 0666);
    if (efd < 0) {			/* if error, quit */
      perror(stderr_file);
      exit(1);
    }
    usestderr = 1;
    stderr_done = 0;
  } else if (stderr_file != NULL && strcmp(stderr_file, "-") == 0) {
    efd = dup(2);			/* or stderr */
    if (efd < 0) {			/* if error, quit */
      perror(argv[0]);
      exit(1);
    }
    usestderr = 1;
    stderr_done = 0;
  } else {
    efd = 0;				/* nope */
    usestderr = 0;
    stderr_done = 1;
  }

/* Initialize the circular buffers. */
  buffer = (char **)malloc((unsigned) nbuffers);
  if (buffer == NULL) {
    (void) fprintf(stderr, "malloc of buffer (%d) failed.\n", nbuffers);
    exit(1);
  }
  read_chars = (int *) malloc((unsigned) nbuffers * (sizeof(int)));
  if (read_chars == NULL) {
    (void) fprintf(stderr, "malloc of read_chars (%d) failed.\n", nbuffers*sizeof(int));
    exit(1);
  }
  buffer_empty = (int *) malloc((unsigned) nbuffers * (sizeof(int)));
  if (buffer_empty == NULL) {
    (void) fprintf(stderr, "malloc of buffer_empty (%d) failed.\n", nbuffers*sizeof(int));
    exit(1);
  }
  stderr_empty = (int *) malloc((unsigned) nbuffers * (sizeof(int)));
  if (stderr_empty == NULL) {
    (void) fprintf(stderr, "malloc of stderr_empty (%d) failed.\n", nbuffers*sizeof(int));
    exit(1);
  }
  for (tmp = 0; tmp < nbuffers; tmp++) {/* get the buffer */
    buffer_empty[tmp] = 0;		/* nothing in buffer */
    stderr_empty[tmp] = 0;		/* nothing in buffer */
    read_chars[tmp] = 0;		/* nothing in buffer */
    buffer[tmp] = valloc(block_size);	/* Get aligned memory block. */
    if (buffer[tmp] == (char *) NULL) {	/* if error */
      (void) fprintf(stderr, "%s: valloc failed\n", argv[0]);
      exit(1);
    }
  }

  if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
    (void) signal(SIGINT, terminate);
  }

/* The main reading and writing loop. */
  iwhich = 0;				/* Starting buffer for reading. */
  owhich = 0;				/* Starting buffer for writing. */
  ewhich = 0;				/* Starting buffer for writing. */

  while (input_done == 0 || output_done == 0 || stderr_done == 0) {
/* (void) fprintf(stderr, "start of loop, %d %d %d whichs=%d %d %d\n", input_done,output_done,stderr_done,iwhich,owhich,ewhich); */
    FD_ZERO(&ibits);			/* nothing to read for select, yet */
    FD_ZERO(&obits);			/* nothing to write for select, yet */
    cnt = 0;				/* count for possible error. */
    if (input_done == 0 &&
      buffer_empty[iwhich] == 0 &&
      ((usestderr != 0 && stderr_empty[iwhich] == 0) || usestderr == 0)) {
      FD_SET(ifd1, &ibits);		/* set to read */
      cnt++;
    }
    if (buffer_empty[owhich] != 0 && output_done == 0) {	/* if something in write buffer */
      FD_SET(ifd2, &ibits);		/* set to read */
      cnt++;
    }
    if (usestderr == 1) {
      if (stderr_empty[ewhich] != 0 && stderr_done == 0) {	/* if something in write buffer */
	FD_SET(efd, &obits);		/* set to write buffer */
	cnt++;
/* (void) fprintf(stderr, "write to stderr (%d)\n", efd); */
      }
    }
    if (cnt == 0) {			/* huh? */
      if (usestderr == 0) {
	(void) fprintf(stderr, "HUH, nothing to read or write???\n");
      }
    }
    maxfd = 1;
    if (ifd1 >= maxfd) {		/* maximum file descriptor for select */
      maxfd = ifd1 + 1;
    }
    if (ifd2 >= maxfd) {			/* maximum file descriptor for select */
      maxfd = ifd2 + 1;
    }
    if (efd >= maxfd) {			/* maximum file descriptor for select */
      maxfd = efd + 1;
    }
/*     limited.tv_sec = 20; */		/* 20 second timeout (slow devices) */
    limited.tv_sec = 60;		/* 60 second timeout (slow devices) */
    limited.tv_usec = 0;
    timelimit = &limited;		/* Some systems clobber this variable */

    n = select(maxfd, &ibits, &obits, (fd_set *) 0, timelimit);
/* (void) fprintf(stderr, "%d = select(%d,%-8.8x,%-8.8x,...)\n", n, maxfd, (int)ibits.fds_bits[0],(int)obits.fds_bits[0]); */
    if (n < 0) {			/* First check for error. */
      if (errno == EINTR) {
	continue;			/* if interrupted, loop */
      }
      perror("select error, ignored");
      continue;				/* ignore other errors(broken systems)*/
    }
    if (n == 0) {			/* select timeout */
      if (usestderr == 0) {
	(void) fprintf(stderr, "select timeout\n");
      }
      continue;				/* Nothing to do upon a timeout. */
    }
    cnt = 0;
/* INPUT ---------- */
    if (FD_ISSET(ifd1, &ibits)) {	/* If correct input bit set. */
      cnt++;				/* count did something */
      ibuff = buffer[iwhich] + read_chars[iwhich];
      temp1 = (int) block_size - read_chars[iwhich];
      temp2 = read(ifd1, ibuff, temp1);
/* (void)fprintf(stderr,"%d = read(%d, ibuff, %d)\n", temp2, ifd1, temp1); */
      if (temp2 <= -1) {	/* if read error */
	if (usestderr == 0) {
	  (void) fprintf(stderr, "%s: ", argv[0]);
	  perror("read");
	}
	return_status = 1;
	break;
      }
      read_chars[iwhich] += temp2;
      if (verbose != 0) {		/* print how many character read. */
	if (fillinput == 0) {
	  (void) fprintf(stderr, "%d:%d\n", block, temp2);
	} else {
	  (void) fprintf(stderr, "%d:%d -> %d\n",
				block, temp2, read_chars[iwhich]);
	}
      }
      if (temp2 == 0) {	/* end of file reached */
/* (void)fprintf(stderr,"eof reached\n"); */
	input_done = 1;
      }
      if (fillinput == 0 || read_chars[iwhich] == (int)block_size || temp2 == 0) {
	if (read_chars[iwhich] != (int)block_size) {
/* (void)fprintf(stderr,"partial record increment\n"); */
	  partial_records++;		/* number of partial records */
	} else {
/* (void)fprintf(stderr,"full record increment\n"); */
	  full_records++;		/* number of full records */
	}
	buffer_empty[iwhich] = 1;	/* something in buffer */
	stderr_empty[iwhich] = 1;	/* something in buffer */
	iwhich++;			/* to next buffer, circularly */
	block++;
	if (iwhich >= nbuffers) {	/* wrap if necessary */
	  iwhich = 0;
	}
      } else {
/* (void)fprintf(stderr,"partial record read\n"); */
      }
    }					/* end of reading section */
/* OUTPUT ---------- */
    if (FD_ISSET(ifd2, &ibits)) {	/* If correct input bit set. */
      cnt++;				/* count did something */
      obuff = buffer[owhich];
/* I know that the following will not work for network protocols. */
/* (void)fprintf(stderr,"writing characters to stdout\n"); */
#---      if (write(ifd2, obuff, (int) read_chars[owhich]) != read_chars[owhich]) {
#---	perror("write stdout");
#---	return_status = 1;
#---	break;
#---      }
      if (usestderr == 1 && owhich == ewhich) {
/* (void)fprintf(stderr,"do not clear read count\n"); */
	;				/* Don't clear count. */
      } else {
/* (void)fprintf(stderr,"clear read count\n"); */
	read_chars[owhich] = 0;		/* mark nothing in buffer */
      }
      buffer_empty[owhich] = 0;
      owhich++;				/* to next buffer, circularly */
      if (owhich >= nbuffers) {		/* wrap if necessary */
	owhich = 0;
      }
    }					/* end of writing section */
/* STDERR ---------- */
    if (usestderr == 1 && FD_ISSET(efd, &obits)) {
      cnt++;				/* count did something */
      ebuff = buffer[ewhich];
/* I know that the following will not work for network protocols. */
/* (void)fprintf(stderr,"writing characters to stderr\n"); */
      if (write(efd, ebuff, (int) read_chars[ewhich]) != read_chars[ewhich]) {
	perror("write stderr");
	return_status = 1;
	break;
      }
      if (owhich == ewhich) {
/* (void)fprintf(stderr,"do not clear read count\n"); */
	;				/* Don't clear count. */
      } else {
/* (void)fprintf(stderr,"clear read count\n"); */
	read_chars[ewhich] = 0;		/* mark nothing in buffer */
      }
      stderr_empty[ewhich] = 0;
      ewhich++;				/* to next buffer, circularly */
      if (ewhich >= nbuffers) {		/* wrap if necessary */
	ewhich = 0;
      }
    }					/* end of writing section */
/* ------------------------------------------------------------------------ */
    if (cnt != n) {			/* Check did what select told us. */
      if (usestderr == 0) {
	(void) fprintf(stderr, "bits set (%d) not equal to %d\n", cnt, n);
      }
    }
/* ------------------------------------------------------------------------ */
    if (input_done ==1) {
      if (read_chars[owhich]==0 && buffer_empty[owhich]==0 ) {
	output_done = 1;
      }
      if (usestderr == 1 &&
	  read_chars[ewhich]==0 && stderr_empty[ewhich] == 0) {
	stderr_done = 1;
      }
    }
  }					/* end of while forever */
/* Exit from program, printing out statistics first. */
  stats();
  exit(return_status);			/* DONE! */
}					/* end of main */

/* End of file fdd.c */
