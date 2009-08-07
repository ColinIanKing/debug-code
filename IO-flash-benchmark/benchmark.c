/*
 * Copyright (C) 2009 Canonical
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 *  Author Colin Ian King,  colin.king@canonical.com
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>

#define MEG		(1024*1024)
#define GIG		(1024*MEG)

#define MIN_SIZE	(512)
#define MAX_SIZE	(8*MEG)

#define ALIGNMENT 	(512)

int	drop_the_caches = 1;

void syntax(char **argv)
{
	fprintf(stderr, "%s: [-s size] [-f filename] [-S] [-D] [-C] [-m max_size]\n",argv[0]);
	fprintf(stderr, "\t-s size of data to read/write (in MB)\n");
	fprintf(stderr, "\t-m max size of read/write blocks to transfer (in MB)\n");
	fprintf(stderr, "\t-S use O_SYNC\n");
	fprintf(stderr, "\t-D use O_DIRECT\n");
	fprintf(stderr, "\t-C do not drop caches before each test\n");
	fprintf(stderr, "\t-f name of file to read/write, can be a raw device\n");
	exit(EXIT_FAILURE);
}

int is_not_device(char *filename)
{
	return (strncmp(filename, "/dev/", 5));
}

char *format_size(unsigned long size)
{
	static char buffer[256];

	if (size < 1024)
		sprintf(buffer, "%ldB", size);
	else 
		if (size < (MEG))
			sprintf(buffer, "%ldK", size / 1024);
		else
			sprintf(buffer, "%ldM", size / MEG);

	return buffer;
}

int write_settings(char *file, char *value)
{
	FILE *fp;
	int ret = 0;

	if ((fp = fopen(file, "w")) == NULL) {
		fprintf(stderr, "Cannot write %s to %s\n", file, value);
		ret = -1;
	}
	else {
		if (fprintf(fp, "%s", value) < 0) {
			fprintf(stderr, "Cannot write %s to %s\n", file, value);
			ret = -2;
		}
		fclose(fp);
	}
	return 0;
}

int drop_caches(void)
{
	int ret = 0;

	sync();
	sleep(1);
	sync();
	sleep(1);
	sync();
	if ((ret = write_settings("/proc/sys/vm/drop_caches","1")) < 0) {
		return ret;
	}
	sleep(1);
	if ((ret = write_settings("/proc/sys/vm/drop_caches","2")) < 0) {
		return ret;
	}
	sleep(1);
	if ((ret = write_settings("/proc/sys/vm/drop_caches","3")) < 0) {
		return ret;
	}
	sleep(1);

	return ret;
}

int benchmark_write_large_file(char *filename, int flags, long size, long blocksize, int repeats)
{
	char *buffer;
	char *alignedbuffer;	
	long usec,sec;	
	int fd;
	int i;
	int j;
	float secs = 0.0;
	float totalsecs = 0.0;
	struct timeval t1,t2;
	int not_device = is_not_device(filename);

	flags |= O_WRONLY | O_NOATIME;

	if ((buffer = malloc(blocksize + ALIGNMENT)) == NULL) {
		fprintf(stderr, "Cannot malloc %ld byte buffer\n",blocksize);
		return -1;
	}

	/* We need an aligned buffer for O_DIRECT I/O */
	alignedbuffer = (char *)((((long)buffer) & ~(ALIGNMENT-1)) + ALIGNMENT);

	if (not_device)
		flags |= O_CREAT;

	for (j=0;j<repeats;j++) {
		if (not_device)
			unlink(filename);
		if ((fd = open(filename, flags, S_IRUSR | S_IWUSR)) < 0) {
			fprintf(stderr, "Cannot creat %s\n",filename);
			free(buffer);
			return -1;
		}

		i = size / blocksize;
		gettimeofday(&t1, NULL);
		while (i-- > 0) {
			if (write(fd, alignedbuffer, blocksize) < 0) {	
				fprintf(stderr, "Write failure: %s\n",strerror(errno));
				free(buffer);
				close(fd);
				return -2;
			}
		}
		gettimeofday(&t2, NULL);
		close(fd);

		sec = t2.tv_sec - t1.tv_sec;
		usec = t2.tv_usec - t1.tv_usec;
	
		secs = (float)sec + (((float)usec)/1000000.0);
		totalsecs += secs;
	}
	printf("%s\t\t%5.2f\n",format_size(blocksize),(size>>20)/secs);

	if (not_device)
		unlink(filename);
	free(buffer);
	return 0;
}

int benchmark_read_large_file(char *filename, int flags, long size, long blocksize, int repeats)
{
	char *buffer;
	char *alignedbuffer;	
	struct timeval t1,t2;
	long usec,sec;	
	int fd;
	int i,j;
	float secs = 0.0;
	float totalsecs = 0.0;
	int not_device = is_not_device(filename);

	flags |= O_WRONLY | O_NOATIME;

	if ((buffer = malloc(blocksize + ALIGNMENT)) == NULL) {
		fprintf(stderr, "Cannot malloc %ld byte buffer\n",blocksize);
		return -1;
	}

	/* We need an aligned buffer for O_DIRECT I/O */
	alignedbuffer = (char *)((((long)buffer) & ~(ALIGNMENT-1)) + ALIGNMENT);

	if (not_device)
		flags |= O_CREAT;

	for (j=0;j<repeats;j++) {
		if (not_device)
			unlink(filename);
		if ((fd = open(filename, flags, S_IRUSR | S_IWUSR)) < 0) {
			fprintf(stderr, "Cannot creat %s\n",filename);
			free(buffer);
			return -1;
		}

		i = size / blocksize;
		while (i-- > 0) {
			if (write(fd, alignedbuffer, blocksize) < 0) {	
				fprintf(stderr, "Write failure: %s\n",strerror(errno));
				free(buffer);
				return -2;
			}
		}
		sync();
		close(fd);
		if ((fd = open(filename, O_RDONLY)) < 0) {
			fprintf(stderr, "Cannot open %s\n",filename);
			free(buffer);
			return -1;
		}

		i = size / blocksize;
		gettimeofday(&t1, NULL);
		while (i-- > 0) {
			if (read(fd, alignedbuffer, blocksize) < 0) {	
				fprintf(stderr, "Read failure: %s\n",strerror(errno));
				free(buffer);
				close(fd);
				return -2;
			}
		}
		gettimeofday(&t2, NULL);
		close(fd);

		sec = t2.tv_sec - t1.tv_sec;
		usec = t2.tv_usec - t1.tv_usec;
	
		secs = (float)sec + (((float)usec)/1000000.0);
		totalsecs += secs;
	}
	printf("%s\t\t%5.2f\n",format_size(blocksize),(size>>20)/secs);

	if (not_device)
		unlink(filename);
	free(buffer);
	return 0;
}


int main(int argc, char **argv)
{
	long sz = MAX_SIZE / MEG;
	long max_sz = MAX_SIZE / MEG;
	long bs;
	char *filename = "rawdata.dat";
	int opt;
	int flags = 0;

	while ((opt = getopt(argc, argv, "SDf:s:m:C")) != -1) {
		switch (opt) {
		case 'C':
			drop_the_caches = 0;
			break;
		case 'S':
			flags |= O_SYNC;
			break;
		case 'D':
			flags |= O_DIRECT;
			break;
		case 'f':
			filename = optarg;
			break;
		case 's':
			sz = atol(optarg);
			break;
		case 'm':
			max_sz = atol(optarg);
			break;
		default:
			syntax(argv);
			exit(EXIT_FAILURE);
		}
	}
	if (sz < max_sz) 
		sz = max_sz;

	max_sz *= MEG;

	if (drop_the_caches) 
		if (getuid() != 0) {
			fprintf(stderr, "Cannot drop caches - run as sudo\n");
			exit(EXIT_FAILURE);
		}

	
	printf("Performing test on %s, write/read %ld MB of data\n",filename,sz);
	if (flags & (O_SYNC | O_DIRECT))
		printf("Using %s %s\n", flags & O_SYNC ? "O_SYNC" : "",
				        flags & O_DIRECT ? "O_DIRECT" : "");
	printf("Block Size\tWrite Rate\n");
	printf("(Bytes)\t\tMB/s\n");
	for (bs=MIN_SIZE; bs<=max_sz;bs<<=1) {
		if (drop_the_caches)
			drop_caches();

		if (benchmark_write_large_file(filename, flags, sz * MEG, bs, 4) < 0) {
			exit(EXIT_FAILURE);
		}
	}
	printf("\n");
	printf("Block Size\tRead Rate\n");
	printf("(Bytes)\t\tMB/s\n");
	for (bs=MIN_SIZE; bs<=max_sz;bs<<=1) {
		if (drop_the_caches)
			drop_caches();

		if (benchmark_read_large_file(filename, flags, sz * MEG, bs, 4) < 0) {
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
}
