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

#include <iostream>
#include <string>
#include <sys/mman.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
/* ------------------------------------------------------------------------ */
using namespace std;
/* ------------------------------------------------------------------------ */
// static const unsigned int NUM_FILES = 2500;
// static const unsigned int FILE_SIZE = 1048576 * 50;	// 10MB FILE_SIZE
static const unsigned int NUM_FILES = 200000000;	// 200,000,000
static const unsigned int FILE_SIZE = 4096;		// one block.

static const string SET_0 = "./mmap_set_0";
static const string SET_1 = "./mmap_set_1";
static const string LIVE_DATA = "./mmap_live_data";

//-- static const unsigned int NUM_READ_THREADS = 25;
//-- static const unsigned int NUM_WRITE_THREADS = 10;
static const unsigned int NUM_READ_THREADS = 64;
static const unsigned int NUM_WRITE_THREADS = 64;

static unsigned long long message_print_time = 100000;	// Starting 1/10th second
//					      1000000      = 1 second.

static pthread_rwlock_t addr_lock = PTHREAD_RWLOCK_INITIALIZER;
/* ------------------------------------------------------------------------ */
vector <unsigned long *>open_and_mmap_files_from_dir(string dname)
{
    vector <unsigned long *>addresses(NUM_FILES);

    for (unsigned int i = 0; i < NUM_FILES; ++i)
    {
	ostringstream   os;

	os << dname
	   << "/"
	   << i
	   << ".dat";

	const char     *filename = os.str().c_str();
	int             fd = open(filename, O_RDONLY);

	if (fd == -1)
	{
	    cerr << "Unable to open " << filename << ". Error code: " << errno << endl;
	    exit(1);
	}

	unsigned long  *data = (unsigned long *)mmap(0, FILE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	if ((void *)data == MAP_FAILED)
	{
	    cerr << "Could not mmap " << filename << ". Error code: " << errno << endl;
	    exit(1);
	}
	close(fd);
	addresses[i] = data;
    }
    return(addresses);
}	/* End of >open_and_mmap_files_from_dir */

/* ------------------------------------------------------------------------ */
unsigned long long timeval_to_us(struct timeval tv)
{
    unsigned long long ret = tv.tv_sec * 1000000;
    return(ret + tv.tv_usec);
}	/* End of timeval_to_us */

/* ------------------------------------------------------------------------ */
unsigned long get_user_time_ms(struct rusage start_time, struct rusage end_time)
{
    return((timeval_to_us(end_time.ru_utime) - timeval_to_us(start_time.ru_utime)) / 1000);
}	/* End of get_user_time_ms */

/* ------------------------------------------------------------------------ */
unsigned long get_system_time_ms(struct rusage start_time, struct rusage end_time)
{
    return((timeval_to_us(end_time.ru_stime) - timeval_to_us(start_time.ru_stime)) / 1000);
}	/* End of get_system_time_ms */

/* ------------------------------------------------------------------------ */
string get_time_string()
{
    time_t          raw_time;
    char            buffer[80];
    struct tm      *timeinfo;

    time(&raw_time);
    timeinfo = localtime(&raw_time);
    strftime(buffer, 80, "%D %T %Z", timeinfo);

    return(string(buffer));
}	/* End of get_time_string */

/* ------------------------------------------------------------------------ */
void open_mmap_read_close(const char *filename)
{
    int             fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
	cerr << "Unable to open " << filename << ". Error code: " << errno << endl;
	exit(1);
    }

    unsigned long  *data = (unsigned long *)mmap(0, FILE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED)
    {
	cerr << "Could not mmap "
	     << filename
	     << ". Error code: "
	     << errno
	     << endl;
	exit(1);
    }

    unsigned int    num_ints = FILE_SIZE / sizeof(unsigned long);
    int             read_bytes = 0;
    unsigned long   tmp = 0;

    for (unsigned int i = 0; i < num_ints; ++i)
    {
	tmp = data[i];
	read_bytes += sizeof(unsigned long);
    }

    if (munmap(data, FILE_SIZE) == -1)
    {
	cerr << "Cound not unmap file"
	     << endl;
    }

    cerr << get_time_string()
         << " Opened, mmapped, and closed file: "
	 << string(filename)
	 << ". Read: "
	 << read_bytes
	 << " bytes."
	 << endl;
    close(fd);
}	/* End of open_mmap_read_close */

/* ------------------------------------------------------------------------ */
void           *read_randomly_from_addresses(void *addrs)
{
    struct rusage   r_start_time;
    struct rusage   r_end_time;
    struct timeval  start_time;
    struct timeval  end_time;
    unsigned long long start;
    unsigned long long end;
    unsigned long   tmp = 0;
    vector <unsigned long *>*mapped_addrs = (vector <unsigned long *>*)addrs;

    while (true)
    {
	int             lock_rc = 0;
	while ((lock_rc = pthread_rwlock_rdlock(&addr_lock)) == EBUSY) ;
	if (lock_rc != 0)
	{
	    cerr << get_time_string()
	         << " Failed to acquire read lock for thread "
		 << (unsigned int)pthread_self()
		 << ". Error: "
		 << lock_rc
		 << ". Exiting."
		 << endl;
	    return(addrs);
	}

	unsigned int    file_no = rand() % mapped_addrs->size();
	unsigned int    file_sz = FILE_SIZE / sizeof(unsigned long);
	unsigned int    offset = 0;
	unsigned int    rd_size = rand() % (file_sz - offset);
	getrusage(RUSAGE_THREAD, &r_start_time);
	gettimeofday(&start_time, NULL);

	for (unsigned int i = 0; i < rd_size; i++)
	{
	    tmp = (*mapped_addrs)[file_no][offset + i];
	}

	lock_rc = pthread_rwlock_unlock(&addr_lock);

	if (lock_rc != 0)
	{
	    cerr << get_time_string()
	         << " WARN: Could not release read lock in thread "
		 << (unsigned int)pthread_self()
		 << ". Error "
		 << lock_rc
		 << endl;
	}

	getrusage(RUSAGE_THREAD, &r_end_time);
	gettimeofday(&end_time, NULL);

	start = timeval_to_us(start_time);
	end = timeval_to_us(end_time);

	if ((end - start) > message_print_time)
	{
	    message_print_time *= 1.1;
	    // Print diagnostics if memory access took too long.
	    cerr << get_time_string()
	         << " usr: "
		 << get_user_time_ms(r_start_time, r_end_time)
		 << " ms sys: "
		 << get_system_time_ms(r_start_time, r_end_time)
		 << " ms elapsed time: "
		 << (end - start) / 1000
		 << " ms   message_limiting: "
		 << message_print_time
		 << endl;
	}
	int up = usleep(20000);
	if (up != 0)
	{
	    cerr << get_time_string()
	         << " usleep error: "
		 << errno
		 << endl;
	}
    }
    return(0);
}	/* End of read_randomly_from_addresses */

/* ------------------------------------------------------------------------ */
void           *create_data_and_update_pool(void *pool)
{
    vector <unsigned long *>*addresses = (vector <unsigned long *>*)pool;

    while (true)
    {
	// Create a new .dat file. Name will be Epoch time in microseconds.
	ostringstream   fname;
	timeval         tv;
	gettimeofday(&tv, NULL);
	unsigned long long time = timeval_to_us(tv);
	fname << LIVE_DATA
	      << "/"
	      << (unsigned int)pthread_self()
	      << "-"
	      << time
	      << ".dat";
	ofstream        fout;
	fout.open(fname.str().c_str(), ios::trunc | ios::binary);

	// Populate the file in random sized increments, with random waits --
	// up to 10 ms -- between updates. This simulates our write traffic.
	unsigned int    num_written = 0;
	const unsigned int num_ints = FILE_SIZE / sizeof(unsigned long);

	while (num_written < (num_ints - 1))
	{
	    unsigned int    wr_size = rand() % (num_ints - num_written);

	    for (unsigned int i = 0; i < wr_size; i++)
	    {
		unsigned long   data = (unsigned long)rand();
		fout.write(reinterpret_cast <const char *>(&data), sizeof(data));
		++num_written;
	    }
	    int tsp = rand() % 100000;
	    int up = usleep(tsp);
	    if (up != 0)
	    {
		cerr << get_time_string()
		     << " usleep error: "
		     << errno
		     << endl;
	    }
	}
	fout.close();
	cerr << get_time_string()
	     << " Finished writing "
	     << fname.str()
	     << endl;

	// Reopen the file in readonly mode.
	int             fd = open(fname.str().c_str(), O_RDONLY);
	if (fd == -1)
	{
	    cerr << get_time_string()
	         << " Could not open "
		 << fname.str()
		 << " for reading. Error code: "
		 << errno
		 << endl;
	    exit(1);
	}


	unsigned long  *base = (unsigned long *)mmap(NULL, FILE_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
	if ((void *)base == MAP_FAILED)
	{
	    cerr << get_time_string()
	         << " Could not mmap "
		 << fname.str()
		 << " for reading. Error code: "
		 << errno
		 << endl;
	    exit(1);
	}
	close(fd);

	// Lock the table and update it.
	int             lock_rc = 0;
	while ((lock_rc = pthread_rwlock_wrlock(&addr_lock)) == EBUSY) ;

	if (lock_rc != 0)
	{
	    cerr << get_time_string()
	         << " Could not acquire write lock while trying to insert file: "
		 << fname.str()
		 << ". Return value: "
		 << lock_rc
		 << endl;
	    return(pool);
	}

	unsigned int    file_no = rand() % addresses->size();
	unsigned long  *old_addr = (*addresses)[file_no];
	// Add the new file
	(*addresses)[file_no] = base;

	// Unmap the old file.
	if (munmap(old_addr, FILE_SIZE) != 0)
	{
	    cerr << get_time_string()
	         << " Warn: could not mmap file at offset "
		 << file_no
		 << ". Error code: "
		 << errno << endl;
	}

	lock_rc = pthread_rwlock_unlock(&addr_lock);
	if (lock_rc != 0)
	{
	    cerr << get_time_string()
	         << " Could not unlock write lock for file: "
		 << fname.str()
		 << ". Aborting"
		 << endl;
	    exit(1);
	}
	cerr << get_time_string()
	     << " Successfully added "
	     << fname.str()
	     << " to working set at index "
	     << file_no
	     << endl;
    }
    return(pool);
}	/* End of create_data_and_update_pool */

/* ------------------------------------------------------------------------ */
int main()
{
    srand(time(0));

    for (unsigned int i = 0; i < NUM_FILES; ++i)
    {
	ostringstream   os;
	os << SET_0
	   << "/"
	   << i
	   << ".dat";
	const char     *filename = os.str().c_str();
	open_mmap_read_close(filename);
    }

    vector <unsigned long *>mapped_addrs = open_and_mmap_files_from_dir(SET_1);
    vector <pthread_t> read_threads(NUM_READ_THREADS);
    vector <pthread_t> write_threads(NUM_WRITE_THREADS);
    void           *res;
    pthread_rwlock_init(&addr_lock, NULL);

    cerr << get_time_string()
         << " Starting read threads.. "
	 << endl;
    for (unsigned int i = 0; i < NUM_READ_THREADS; ++i)
    {
	pthread_create(&read_threads[i], 0, &read_randomly_from_addresses, (void *)&mapped_addrs);
    }

    cerr << get_time_string()
         << " Starting write threads.. "
	 << endl;

    for (unsigned int i = 0; i < NUM_WRITE_THREADS; ++i)
    {
	pthread_create(&write_threads[i], 0, &create_data_and_update_pool, (void *)&mapped_addrs);
    }

    pthread_join(write_threads[0], &res);
    pthread_join(read_threads[0], &res);

    for (vector <unsigned long *>::iterator i = mapped_addrs.begin();
	 i != mapped_addrs.end();
	 ++i)
    {
	munmap(*i, FILE_SIZE);
    }
    sleep(5000000);
    return(0);
}

/* ------------------------------------------------------------------------ */
