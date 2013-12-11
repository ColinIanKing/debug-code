/*
 * Copyright (C) 2013 Canonical
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
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define MSR_SMI_COUNT	(0x00000034)

static inline int tty_height(void)
{
#ifdef TIOCGWINSZ
	int fd = 0;
	struct winsize ws;

	if (isatty(fd) &&
           (ioctl(fd, TIOCGWINSZ, &ws) != -1) &&
           (0 < ws.ws_row) &&
           (ws.ws_row == (size_t)ws.ws_row))
		return ws.ws_row;
#endif
	return 25; 
}

static inline bool cpu_has_msr(void)
{
	uint32_t edx;

	asm("cpuid" : "=d" (edx));

	return edx & (1 << 5);
}

static int readmsr(const int cpu, const uint32_t reg, uint64_t *val)
{
	char buffer[PATH_MAX];
	uint64_t value = 0;
	int fd;
	int ret;

	*val = ~0;
	snprintf(buffer, sizeof(buffer), "/dev/cpu/%d/msr", cpu);
	if ((fd = open(buffer, O_RDONLY)) < 0) {
		if (system("modprobe msr") < 0)
			return -1;
		if ((fd = open(buffer, O_RDONLY)) < 0)
			return -1;
	}
	ret = pread(fd, &value, 8, reg);
	(void)close(fd);
	if (ret < 0)
		return -1;

	*val = value;
	return 0;
}

static void heading(void)
{
	printf("  Time        SMIs\n");
}

int main(void)
{
	int row = 2;

	if ((getuid() !=0 ) || (geteuid() != 0)) {
		fprintf(stderr, "Need to run as root.\n");
		exit(EXIT_FAILURE);
	}

	if (!cpu_has_msr()) {
		fprintf(stderr, "CPU does not have MSRs\n");
		exit(EXIT_FAILURE);
	}

	heading();
	while (true) {
		uint64_t smicount;
		struct tm tm;
		time_t now;
		int h = tty_height();

		(void)time(&now);
		(void)localtime_r(&now, &tm);
		row++;

		if (row >= h) {
			heading();
			row = 2;
		}
		if (readmsr(0, MSR_SMI_COUNT, &smicount) < 0) {
			fprintf(stderr, "MSR read failed\n");
			exit(EXIT_FAILURE);
		}
		printf("%2.2d:%2.2d:%2.2d %9" PRIu64 "\n", 
			tm.tm_hour, tm.tm_min, tm.tm_sec, smicount);
		sleep(1);
	}
}
