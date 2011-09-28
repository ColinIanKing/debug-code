/*
 * Copyright (C) 2011 Canonical
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <asm/mtrr.h>
#include <fcntl.h>

#define LONGSZ	((int)(sizeof(long)<<1))

int main(int argc, char *argv[])
{
	struct mtrr_gentry gentry;
	int fd;

	static char *mtrr_type[] = {
		"Uncachable",
		"Write Combining",
		"Unknown",
		"Unknown",
		"Write Through",
		"Write Protect",
		"Write Back"
	};

	if ((fd = open("/proc/mtrr", O_RDONLY, 0)) < 0) {
		fprintf(stderr, "Cannot open /proc/mtrr!\n");
		exit(EXIT_FAILURE);
	}
		
	memset(&gentry, 0, sizeof(gentry));

	while (!ioctl(fd, MTRRIOC_GET_ENTRY, &gentry)) {
		if (gentry.size < 1) 
			printf("%u: Disabled\n", gentry.regnum);
		else
			printf("%u: 0x%*.*lx..0x%*.*lx %s\n", gentry.regnum,
				LONGSZ, LONGSZ, gentry.base, 
				LONGSZ, LONGSZ, gentry.base + gentry.size,
				mtrr_type[gentry.type]);
		gentry.regnum++;
	}
	close(fd);

	exit(EXIT_SUCCESS);
}
