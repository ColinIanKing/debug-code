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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>

int main(int argc, char **argv) {
	struct stat	statinfo;
	int		fd;
	int		num_blocks;
	int		block_size;
	int		i;

	if (argc != 2) {
		fprintf(stderr, "Syntax: %s filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (ioctl(fd, FIGETBSZ, &block_size) < 0) {
		fprintf(stderr, "Cannot get block size\n");
		close(fd);
		exit(EXIT_FAILURE);
	}

	if (fstat(fd, &statinfo) < 0) {
		fprintf(stderr, "Cannot stat %s\n", argv[1]);
		close(fd);
		exit(EXIT_FAILURE);
	}
	num_blocks = (statinfo.st_size + block_size - 1) / block_size;

	printf("File: %s: %d x %d byte blocks\n", 
		argv[1], num_blocks, block_size);

	for (i=0; i<num_blocks; i++) {
		int block = i;
		if (ioctl(fd, FIBMAP, &block)) {
			printf("ioctl failed: %s\n", strerror(errno));
		}
		printf("%10d\t", block);
	}
	close(fd);
	printf("\n");
	
	exit(EXIT_SUCCESS);
}
