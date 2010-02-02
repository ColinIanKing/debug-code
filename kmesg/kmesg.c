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

/*
 *  Author Colin Ian King,  colin.king@canonical.com
 */

#include <sys/klog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define POLL_NEXT_BUSY	(5000)
#define POLL_NEXT_IDLE	(50000)

int main(int argc, char **argv)
{
	int sz,n;
	char *buffer;
	int fd;

	if (argc < 2) {
		fprintf(stderr, "Syntax: %s logfile\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_WRONLY | O_APPEND | O_CREAT | O_SYNC, 00666);
	if (fd < 0) {
		fprintf(stderr,"Cannot open log file %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	sz = klogctl(10, NULL, 0);
	if ((buffer = malloc(sz+1)) == NULL) {
		fprintf(stderr,"Cannot alloc %d bytes for buffer\n",sz);
		exit(EXIT_FAILURE);
	}
	
	for (;;) {
		if ((n = klogctl(4, buffer, sz)) > 0) {	
			char *ptr = buffer;
			while (n > 0) {
				int written = write(fd, ptr, n);
				if (written > 0) {
					ptr += written;
					n -= written;	
				}
				else {
					break;
				}
			}
			usleep(POLL_NEXT_BUSY);
		}
		else {
			usleep(POLL_NEXT_IDLE);
		}
	}
	exit(EXIT_SUCCESS);
}
