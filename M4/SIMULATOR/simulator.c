/*
 * Compile this program with pthreads:
 *   g++ -Wall -lpthread -o graphdb-simulator graphdb-simulator.cpp
 *
 * Before you run this program, you need to create the following directories:
 * ./mmap_set_0  	<-- fill it with data files named [0..N).dat
 * ./mmap_set_1  	<-- fill it with data files named [0..N).dat
 * ./mmap_live_data	<-- empty, created new data while running.
 *
 * The value of N is determined by the NUM_FILES compile time constant. The
 * size of the file is determined by the FILE_SIZE compile constant.
 */

/* ------------------------------------------------------------------------ */
//      Open NUM_FILES of size FILE_SIZE in directory FROM_FILES, via
//      NUM_READ_THREADS pthreads, and write those to directory TO_FILES.
//
//      If file exists, and size right, copy it to directory TO_FILES.
//      If file does not exist, create it and write random data to it,
//         then rewind, and write to directory TO_FILES.

/* ------------------------------------------------------------------------ */

#define __USE_GNU

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

/* ------------------------------------------------------------------------ */
// Number of files to create.
//                      200 * 1000 * 1000
// #define NUM_FILES    200000000ull
//                      36 * 1000 * 1000
#define NUM_FILES	36000000ull
// #define      NUM_FILES       1280000ull
// #define      NUM_FILES       101ull

/* FILE_SIZE of each individual file. */
#define	FILE_SIZE	4096
// #define	FILE_SIZE	16384

/* ------------------------------------------------------------------------ */
#define NUM_READ_THREADS     1000ull
// #define NUM_READ_THREADS     128ull
// #define NUM_READ_THREADS     7ull

/* ------------------------------------------------------------------------ */
// #define FROM_FILES	"input_files"
// #define TO_FILES	"output_files"
static char *FROM_FILES;
static char *TO_FILES;

/* ------------------------------------------------------------------------ */
// Directory layout:
//          files       1000    //         1,000
//      sub_dir         1000    //     1,000,000        Files in subdirectores.
//  FROM_FILES          Rest.   // 1,000,000,000        If 1000 directories in here.

#define FILES	1000			// Number of files in lowest level directory.
#define SUB_DIR	1000			// Number of directories in subdir.

/* ------------------------------------------------------------------------ */
#define	timeval_to_us(tv)	 (tv.tv_sec * 1000000 + tv.tv_usec)
#define get_user_time_ms(st,et)	((timeval_to_us(et.ru_utime) - timeval_to_us(st.ru_utime)) / 1000)
#define get_system_time_ms(st,et)  ((timeval_to_us(et.ru_stime) - timeval_to_us(st.ru_stime)) / 1000)

#define STRINGIFY(x)	#x
// #define m4_debug(...)        fprintf(stderr, __VA_ARGS__)
#define m4_debug(...)

/* ------------------------------------------------------------------------ */
static unsigned long long message_print_time = 100000;	// Starting 1/10th second

//                                            1000000           = 1 second.
static enum __rusage_who rusage = RUSAGE_CHILDREN;

/* ------------------------------------------------------------------------ */
static pthread_t thread_pid[NUM_READ_THREADS];
static unsigned int seedp_r[NUM_READ_THREADS];	/* Random number saved state for threads. */

/* ======================================================================== */
static void initialize(void)
{
    unsigned long long i;
    unsigned int    j;
    struct stat     st = { 0 };
    int             serrno;

    /* Initialize random number generator. */
    j = time(0);
    for (i = 0; i < NUM_READ_THREADS; i++)
    {
	seedp_r[i] = j;
    }

    /* Assume if directory exists, that it really is a directory. */
    if (stat(FROM_FILES, &st) == -1)
    {
m4_debug("%s:%u:%s mkdir(%s,...)\n", __FILE__, __LINE__, __func__, FROM_FILES);
	if (mkdir(FROM_FILES, 0700) < 0)
	{
	    serrno = errno;
	    if (serrno != EEXIST)
	    {
		fprintf(stderr, "%s:%u:%s mkdir(%s,...) error %d '%s'\n", __FILE__, __LINE__,
			__func__, FROM_FILES, serrno, strerror(serrno));
		exit(1);
	    }
	}
    }
    if (stat(TO_FILES, &st) == -1)
    {
m4_debug("%s:%u:%s mkdir(%s,...)\n", __FILE__, __LINE__, __func__, TO_FILES);
	if (mkdir(TO_FILES, 0700) < 0)
	{
	    serrno = errno;
	    if (serrno != EEXIST)
	    {
		fprintf(stderr, "%s:%u:%s mkdir(%s,...) error %d '%s'\n", __FILE__, __LINE__,
			__func__, TO_FILES, serrno, strerror(serrno));
		exit(1);
	    }
	}
    }
}					/* End of initialize */

/* ------------------------------------------------------------------------ */
static char    *get_time_string(void)
{
    time_t          raw_time;
    char            buffer[80];
    struct tm       ti;

    time(&raw_time);
    (void)localtime_r(&raw_time, &ti);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H-%M-%S", &ti);

    return (strndup(buffer, sizeof(buffer)));
}					/* End of get_time_string */

/* ------------------------------------------------------------------------ */
static void check_mkdir1(unsigned long long thread, const char *dir,
			 unsigned long long which_dir)
{
    char            filename[PATH_MAX];
    struct stat     st = { 0 };
    int             serrno;

    snprintf(filename, PATH_MAX, "%s/%lld", dir, which_dir);
    if (stat(filename, &st) == -1)
    {
m4_debug("%4lld %s:%u:%s mkdir(%s,...)\n", thread, __FILE__, __LINE__, __func__, filename);
	if (mkdir(filename, 0700) < 0)
	{
	    serrno = errno;
	    if (serrno != EEXIST)
	    {
		fprintf(stderr, "%4lld %s:%u:%s mkdir(%s,...) error %d '%s'\n", thread,
			__FILE__, __LINE__, __func__, filename, serrno, strerror(serrno));
		exit(1);
	    }
	}
    }
}					/* End of check_mkdir1 */

/* ------------------------------------------------------------------------ */
static void check_mkdir2(unsigned long long thread, const char *dir,
			 unsigned long long which_dir, unsigned long long which_subdir)
{
    char            filename[PATH_MAX];
    struct stat     st = { 0 };
    int             serrno;

    snprintf(filename, PATH_MAX, "%s/%lld/%lld", dir, which_dir, which_subdir);
    if (stat(filename, &st) == -1)
    {
m4_debug("%4lld %s:%u:%s mkdir(%s,...)\n", thread, __FILE__, __LINE__, __func__, filename);
	if (mkdir(filename, 0700) < 0)
	{
	    serrno = errno;
	    if (serrno != EEXIST)
	    {
		fprintf(stderr, "%4lld %s:%u:%s mkdir(%s,...) error %d '%s'\n", thread,
			__FILE__, __LINE__, __func__, filename, serrno, strerror(serrno));
		exit(1);
	    }
	}
    }
}					/* End of check_mkdir2 */

/* ------------------------------------------------------------------------ */
static int create_from_file(unsigned long long thread, unsigned long long which_dir,
			    unsigned long long which_subdir,
			    unsigned long long which_file,
			    char *buffer,
			    unsigned long long *f_last_dir, unsigned long long *f_last_subdir)
{
    int             from_fd;
    char            filename[PATH_MAX];
    int            *p = (int *)buffer;
    int             i;
    int             l = FILE_SIZE / sizeof(int);
    int             serrno = errno;

    /* Save stat system call for number of FILES in subdirectory of directory of "dir" */
    if (*f_last_dir != which_dir)
    {
	*f_last_dir = which_dir;
	*f_last_subdir = which_subdir;
	check_mkdir1(thread, FROM_FILES, which_dir);
	check_mkdir2(thread, FROM_FILES, which_dir, which_subdir);
    }
    else if (*f_last_subdir != which_subdir)	/* only subcomponent missing */
    {
	*f_last_subdir = which_subdir;
	check_mkdir2(thread, FROM_FILES, which_dir, which_subdir);
    }

    for (i = 0; i < l; i++)
    {
	*p++ = rand();
    }

    snprintf(filename, PATH_MAX, "%s/%lld/%lld/%lld.dat", FROM_FILES, which_dir,
	     which_subdir, which_file);
m4_debug("%4lld %s:%u:%s creating file '%s'\n", thread, __FILE__, __LINE__, __func__, filename);
    from_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0700);
    if (from_fd < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s open(%s,...) error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, filename, serrno, strerror(serrno));
	exit(1);
    }
m4_debug("%4lld %s:%u:%s write(%s,,%d)\n", thread, __FILE__, __LINE__, __func__, filename, FILE_SIZE);
    l = write(from_fd, buffer, FILE_SIZE);
    if (l < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s write(%s,...) error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, filename, serrno, strerror(serrno));
	exit(1);
    }
    else if (l != FILE_SIZE)
    {
	fprintf(stderr, "%4lld %s:%u:%s Write returned %d instead of %d bytes!\n", thread,
		__FILE__, __LINE__, __func__, l, FILE_SIZE);
	exit(1);
    }
m4_debug("%4lld %s:%u:%s fsync(%s)\n", thread, __FILE__, __LINE__, __func__, filename);
    i = fsync(from_fd);
    if (i < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s fsync %s  error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, filename, serrno, strerror(serrno));
	exit(1);
    }

// m4_debug("%4lld %s:%u:%s lseek(%s,0)\n", thread, __FILE__, __LINE__, __func__, filename);
    i = lseek(from_fd, 0, SEEK_SET);
    if (i < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s lseek(%s,0,SEEK_SET) error %d '%s'\n", thread,
		__FILE__, __LINE__, __func__, filename, serrno, strerror(serrno));
	exit(1);
    }

    return (from_fd);
}					/* End of create_from_file */

/* ------------------------------------------------------------------------ */
static int create_to_file(unsigned long long thread, unsigned long long which_dir,
			  unsigned long long which_subdir, unsigned long long which_file,
			  unsigned long long *t_last_dir, unsigned long long *t_last_subdir)
{
    int             to_fd;
    char            filename[PATH_MAX];
    int             serrno;

    /* Save stat system call for number of FILES in subdirectory of directory of "dir" */
    if (*t_last_dir != which_dir)
    {
	*t_last_dir = which_dir;
	*t_last_subdir = which_subdir;
	check_mkdir1(thread, TO_FILES, which_dir);
	check_mkdir2(thread, TO_FILES, which_dir, which_subdir);
    }
    else if (*t_last_subdir != which_subdir)     /* only subcomponent missing */
    {
	*t_last_subdir = which_subdir;
	check_mkdir2(thread, TO_FILES, which_dir, which_subdir);
    }

    snprintf(filename, PATH_MAX, "%s/%lld/%lld/%lld.dat", TO_FILES, which_dir,
	     which_subdir, which_file);
m4_debug("%4lld %s:%u:%s creating file '%s'\n", thread, __FILE__, __LINE__, __func__, filename);
    to_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (to_fd < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s open(%s,...) error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, filename, serrno, strerror(serrno));
	exit(1);
    }

    return (to_fd);
}					/* End of create_to_file */

/* ------------------------------------------------------------------------ */
static void copy_from_to(unsigned long long thread,
			 unsigned long long which_number,
			 char *buffer,
			 unsigned long long *f_last_dir, unsigned long long *f_last_subdir,
			 unsigned long long *t_last_dir, unsigned long long *t_last_subdir)
{
    struct rusage   r_start_time;
    struct rusage   r_end_time;
    struct timeval  start_time;
    struct timeval  end_time;
    unsigned long long start;
    unsigned long long end;

    int             up;
    int             tsp;

    int             from_fd = -1;
    int             to_fd = -1;
    char            from_file_name[PATH_MAX];
    char            to_file_name[PATH_MAX];
    unsigned long long which;
    unsigned long long which_dir;
    unsigned long long which_subdir;
    unsigned long long which_file;
    char           *gts;
    int             j;
    int             serrno;

    /* Create file names like: ./input_files/56/22/123.dat */
    which_file = which_number % FILES;	// ./xyz/0/0/0.dat
    which = which_number / FILES;
    which_subdir = which % SUB_DIR;	// ./xyz/0/0
    which_dir = which / SUB_DIR;	// ./xyz/0

    snprintf(from_file_name, PATH_MAX, "%s/%lld/%lld/%lld.dat",
	     FROM_FILES, which_dir, which_subdir, which_file);
    snprintf(to_file_name, PATH_MAX, "%s/%lld/%lld/%lld.dat",
	     TO_FILES, which_dir, which_subdir, which_file);

m4_debug("%4lld %s:%u:%s opening file '%s'\n", thread, __FILE__, __LINE__, __func__, from_file_name);
    from_fd = open(from_file_name, O_RDONLY);
    if (from_fd < 0)
    {
	from_fd = create_from_file(thread, which_dir, which_subdir, which_file,
				   buffer, f_last_dir, f_last_subdir);
    }

//     getrusage(RUSAGE_THREAD, &r_start_time);
    getrusage(rusage, &r_start_time);
    gettimeofday(&start_time, NULL);

    /* Do copy. */
m4_debug("%4lld %s:%u:%s opening file '%s'\n", thread, __FILE__, __LINE__, __func__, to_file_name);
    to_fd = open(to_file_name, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (to_fd < 0)
    {
	to_fd = create_to_file(thread, which_dir, which_subdir, which_file, t_last_dir, t_last_subdir);
    }

m4_debug("%4lld %s:%u:%s read(%s,,%d)\n", thread, __FILE__, __LINE__, __func__, from_file_name, FILE_SIZE);
    j = read(from_fd, buffer, FILE_SIZE);
    if (j < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s read(%s,...) error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, from_file_name, serrno, strerror(serrno));
	exit(1);
    }
    else if (j != FILE_SIZE)
    {
	fprintf(stderr, "%4lld %s:%u:%s Read returned %d instead of %d bytes for '%s'!\n",
		thread, __FILE__, __LINE__, __func__, j, FILE_SIZE, from_file_name);
	exit(1);
    }
m4_debug("%4lld %s:%u:%s close(%s)\n", thread, __FILE__, __LINE__, __func__, from_file_name);
    j = close(from_fd);
    if (j < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s close %s  error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, from_file_name, serrno, strerror(serrno));
	exit(1);
    }

m4_debug("%4lld %s:%u:%s write(%s,,%d)\n", thread, __FILE__, __LINE__, __func__, to_file_name, FILE_SIZE);
    j = write(to_fd, buffer, FILE_SIZE);
    if (j < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s write(%s,...) error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, to_file_name, serrno, strerror(serrno));
	exit(1);
    }
    else if (j != FILE_SIZE)
    {
	fprintf(stderr, "%4lld %s:%u:%s Write returned %d instead of %d bytes!\n", thread,
		__FILE__, __LINE__, __func__, j, FILE_SIZE);
	exit(1);
    }
m4_debug("%4lld %s:%u:%s fsync(%s)\n", thread, __FILE__, __LINE__, __func__, to_file_name);
    j = fsync(to_fd);
    if (j < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s fsync %s  error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, to_file_name, serrno, strerror(serrno));
	exit(1);
    }
m4_debug("%4lld %s:%u:%s close(%s)\n", thread, __FILE__, __LINE__, __func__, to_file_name);
    j = close(to_fd);
    if (j < 0)
    {
	serrno = errno;
	fprintf(stderr, "%4lld %s:%u:%s close %s  error %d '%s'\n", thread, __FILE__,
		__LINE__, __func__, to_file_name, serrno, strerror(serrno));
	exit(1);
    }

    gettimeofday(&end_time, NULL);
    start = timeval_to_us(start_time);
    end = timeval_to_us(end_time);

    if ((end - start) > message_print_time)
    {
//      getrusage(RUSAGE_THREAD, &r_end_time);
	getrusage(rusage, &r_end_time);
	message_print_time *= 1.1;
	// Print diagnostics if memory access took too long.
	gts = get_time_string();
	fprintf(stderr,
		"%4lld %s user: %ld ms  sys: %ld ms  time %lld seconds  message_limiting: %lld\n",
		thread, gts, get_user_time_ms(r_start_time, r_end_time),
		get_system_time_ms(r_start_time, r_end_time), (end - start) / 1000,
		message_print_time);
	free(gts);
    }
    tsp = rand() % 100000;
    up = usleep(tsp);
    if (up != 0)
    {
	gts = get_time_string();
	fprintf(stderr, "%4lld %s usleep error: %d\n", thread, gts, errno);
	free(gts);
    }
}					/* End of copy_from_to */

/* ------------------------------------------------------------------------ */
static void    *read_threads(void *arg)
{
    unsigned long long s;
    unsigned long long e;
    unsigned long long i;
    char           *buffer;
    unsigned long long thread;
    unsigned long long f_last_dir = -1;
    unsigned long long f_last_subdir = -1;
    unsigned long long t_last_dir = -1;
    unsigned long long t_last_subdir = -1;

    thread = (unsigned long long)arg;
    s = (thread * NUM_FILES) / NUM_READ_THREADS;
    e = ((thread + 1) * NUM_FILES) / NUM_READ_THREADS;

m4_debug("arg=%lld NUM_FILES(%lld) NUM_READ_THREADS(%lld)  s=%lld e=%lld\n", thread, NUM_FILES, NUM_READ_THREADS, s, e);

    buffer = (char *)malloc(FILE_SIZE);

    for (i = s; i < e; i++)
    {
	copy_from_to(thread, i, buffer, &f_last_dir, &f_last_subdir, &t_last_dir, &t_last_subdir);
    }
    free(buffer);
    return (arg);
}					/* End of read_threads */

/* ------------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    char           *gts;
    unsigned long long i;
    long            which_thread[NUM_READ_THREADS];
    void           *res;
    struct rusage   r_start_time;
    struct rusage   r_end_time;
    struct timeval  start_time;
    struct timeval  end_time;
    unsigned long long start;
    unsigned long long end;

    if (argc != 3)
    {
	fprintf(stderr, "Need two arguments, input and output directories.\n");
	exit(1);
    }

    FROM_FILES = argv[1];
    TO_FILES = argv[2];

    getrusage(RUSAGE_SELF, &r_start_time);
    gettimeofday(&start_time, NULL);

    initialize();

    gts = get_time_string();
    fprintf(stderr, "%s Starting read threads...\n", gts);
    free(gts);

    for (i = 0; i < NUM_READ_THREADS; ++i)
    {
	which_thread[i] = i;
	pthread_create(&thread_pid[i], NULL, &read_threads, (void *)which_thread[i]);
    }

    gts = get_time_string();
    fprintf(stderr, "%s Wait for threads to finish their tasks...\n", gts);
    free(gts);

    for (i = 0; i < NUM_READ_THREADS; ++i)
    {
	pthread_join(thread_pid[i], &res);
    }

    getrusage(RUSAGE_SELF, &r_end_time);
    gettimeofday(&end_time, NULL);
    start = timeval_to_us(start_time);
    end = timeval_to_us(end_time);

    gts = get_time_string();
    fprintf(stderr, "%s user: %ld ms  sys: %ld ms  real time: %4.2f seconds\n",
	    gts, get_user_time_ms(r_start_time, r_end_time),
	    get_system_time_ms(r_start_time, r_end_time), (end - start) / 1000000.0);
    free(gts);

    return (0);
}					/* End of main */

/* ------------------------------------------------------------------------ */
