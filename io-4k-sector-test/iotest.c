/*
 * Copyright (C) 2010 Canonical
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

#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#define BLOCKSIZE	4096
#define ALIGNMENT	4096

int main(int argc, char **argv)
{
	off64_t start = 0ULL;
	off64_t offset = start;
	off64_t last_offset = offset;
	char *buffer;
	char *alignedbuffer;
	int fd;
	unsigned long long count = 0;
	unsigned long long i;
	struct timeval t1, t2;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s path\n", argv[0]);
		fprintf(stderr, "  where path is filename or name of block device\n");
		exit(EXIT_FAILURE);
	}

	if ((buffer = malloc(BLOCKSIZE + ALIGNMENT)) == NULL) {
                fprintf(stderr, "Cannot malloc %d byte buffer\n",BLOCKSIZE);
		exit(EXIT_FAILURE);
        }

        /* aligned buffer for O_DIRECT I/O */
        alignedbuffer = (char *)((((long)buffer) & ~(ALIGNMENT-1)) + ALIGNMENT);

	if ((fd = open(argv[1], O_RDWR | O_CREAT, S_IRWXU)) < 0) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	printf("Write test:\n");

	gettimeofday(&t1, NULL);

	for (;;) {
		register off64_t *ptr = (off64_t *)alignedbuffer;
		register int j;

		for (j=0;j<BLOCKSIZE;j+=sizeof(off64_t)) {
			*ptr++ = offset;
		}
		if (lseek64(fd, offset, SEEK_SET) < 0) {
			printf("Seek to offset %lld failed\n", (unsigned long long) offset);
			break;
		}
		if (write(fd, alignedbuffer, BLOCKSIZE) != BLOCKSIZE) {
			printf("Write to offset %lld failed\n", (unsigned long long)offset);
			break;
		}

		if ((count & 0xffff) == 0) {
			gettimeofday(&t2, NULL);
			unsigned long long usec1,usec2, usecs;
			double secs;
			double rate = 0.0;
		
			usec1 = (t1.tv_sec * 1000000) + t1.tv_usec;
			usec2 = (t2.tv_sec * 1000000) + t2.tv_usec;
			usecs = usec2 - usec1;
			secs = (usecs / 1000000.0);
			if (secs > 0.0)
				rate = (offset - last_offset) / secs;
				
			t1 = t2;
			last_offset = offset;

			printf("Written to offset %llx (%lld MB) @ %7.3f MB/sec\n", (unsigned long long)offset, (unsigned long long)offset / (1024 * 1024), rate / (1024.0 * 1024.0));
			fflush(stdout);
		}
		count++;
		offset+=BLOCKSIZE;
	}

	printf("Read test:\n");

	last_offset = offset = start;
	for (i=0;i<count;i++) {
		off64_t *ptr = (off64_t *)alignedbuffer;
		register int j;
		register int bad = 0;

		if (lseek64(fd, offset, SEEK_SET) < 0) {
			printf("Seek to offset %lld failed\n", (unsigned long long)offset);
			break;
		}
		if (read(fd, alignedbuffer, BLOCKSIZE) != BLOCKSIZE) {
			printf("Read at offset %lld failed\n", (unsigned long long)offset);
			break;
		}
		for (j=0;j<BLOCKSIZE;j+=sizeof(off64_t), ptr++) {
			if (*ptr != offset) {
				printf("Data at offset %lld was 0x%llx, should be 0x%llx\n",
				(unsigned long long)(offset + j), (unsigned long long)*ptr, (unsigned long long)offset);
				bad++;
			}
		}
		if (bad)
			break;
		if ((i & 0xffff) == 0) {
			gettimeofday(&t2, NULL);
			unsigned long long usec1,usec2, usecs;
			double secs;
			double rate = 0.0;
		
			usec1 = (t1.tv_sec * 1000000) + t1.tv_usec;
			usec2 = (t2.tv_sec * 1000000) + t2.tv_usec;
			usecs = usec2 - usec1;
			secs = (usecs / 1000000.0);
			if (secs > 0.0)
				rate = (offset - last_offset) / secs;
				
			t1 = t2;
			last_offset = offset;

			printf("Read from offset %llx (%lld MB) @ %7.3f MB/sec\n", (unsigned long long)offset, (unsigned long long)offset / (1024 * 1024), rate / (1024.0 * 1024.0));
			fflush(stdout);
		}
		offset+=BLOCKSIZE;
	}

	close(fd);
	free(buffer);

	exit(EXIT_SUCCESS);
}
