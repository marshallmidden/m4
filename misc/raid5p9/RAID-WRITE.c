// #define FILENAME	"/dev/sdc"
#define FILENAME	"/dev/rsd2c"
// #define FILENAME	"THEFILE"

/*
Need a routine to test all of raid 5 parity 9 (8+1) write possibilties.

VDISKCREATE -v VID capacity physicalDisks raidType
							  3=raid5
							    stripe
							       mirrorDepth 
								 parity
								   maxRaids
								     threshold
									flags 2 = bay redundancy
									  minPD
vdiskcreate -v 59 5184 165,136,173,201,164,135,174,202,163 3 64 2 9 8 0 0 0

64 sectors per stripe.
Thus, 9*64 = one full round of raid5/9 cycling.
To get all permutations, that needs 9 rounds.

Thus CAPACITY is 9*64*9 for all permutations.
	      5184 sectors
	      We get 16384 as minimum.
Need to test that.
a) read blocks 1 sector at a time increasing by 1 sector.
b) read blocks 2 sector at a time increasing by 1 sector.
c) read blocks 3 sector at a time increasing by 1 sector.
	...
Max write is 2mb, which is 2097152, or 4096 sectors.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define SECTOR_SIZE	(512)
#define MAX_SECTOR	(16384)
#define MAX_BUFFER	(2*1024*1024)
static char buffer[MAX_BUFFER];

#define LL_BS		(SECTOR_SIZE/sizeof(unsigned long long))

int main()
{
    off_t sectors;
    off_t lba;
    int fd;
    off_t seeked;
    int i;
    int j;
    int count;
    ssize_t red;
    unsigned long long *wb = (unsigned long long *)buffer;

    fd = open(FILENAME, O_RDWR);
    if (fd < 0)
    {
    	perror("open:");
	exit(1);
    }

    for (sectors = 1; sectors <= MAX_BUFFER / SECTOR_SIZE ; sectors++)
    {
      fprintf(stderr, "\nwriting %llu sectors at block offset:\n", (unsigned long long)sectors);

      for (lba = 0; lba <= (MAX_SECTOR - sectors); lba++)
      {
	if ((lba % 50) == 0)
	{
	  fprintf(stderr, " %llu", (unsigned long long)lba);
	}

	seeked = lseek(fd, lba * SECTOR_SIZE, SEEK_SET);
	if (seeked < 0)
	{
	    perror("lseek:");
	    fprintf(stderr, "lba=%lld\n", lba);
	    exit(2);
	}

	count = (MAX_SECTOR - lba) / sectors;
	for (i = 0; i < count; i++)
	{
	    for (j = 0; j < sectors; j++)
	    {
	    	wb[j * LL_BS] = lba + i*sectors + j;
	    }
	    red = write(fd, buffer, sectors * SECTOR_SIZE);
	    if (red < 0)
	    {
		perror("write:");
		fprintf(stderr, "sectors=%llu, lba=%llu, count=%d\n", (unsigned long long)sectors, (unsigned long long)lba, count);
		exit(3);
	    }
	}
      }
    }

    fprintf(stderr, "Done!\n");
    exit(0);
}
