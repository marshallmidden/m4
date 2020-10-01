#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
extern void *memrchr(const void *, int, size_t);

int main(int argc, char **argv)
{
	int lines = abs(atoi(argv[1]));
	char *fn = argv[2];
	char *start, *string;
	struct stat sb;
	int fd;

	fd = open(fn, O_RDONLY, 0);
	fstat(fd, &sb);
	start = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	string = start + sb.st_size;
	while (lines >= 0 && string > start) {
		string = memrchr(start, '\n', string - start);
		lines--;
	}
	string = (string == NULL) ? start : string+1;
	fwrite(string, sb.st_size - (string-start), 1, stderr);
	exit(0);
}
