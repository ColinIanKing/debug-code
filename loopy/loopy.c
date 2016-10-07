/*
 * Copyright (C) 2016 Canonical
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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/loop.h>

#define MAX_LOOP	8192

static int loop_create(const off_t len)
{
	int cfd, bfd, lfd, err;
	int devnr[MAX_LOOP];
	size_t i, n;
	char filename[PATH_MAX];
	const pid_t pid = getpid();

	cfd = open("/dev/loop-control", O_RDWR | O_CLOEXEC);
	if (cfd < 0) {
		fprintf(stderr, "can't open /dev/loop-control: %d (%s)\n",
			errno, strerror(errno));
		return -errno;
	}

	for (n = 0; n < MAX_LOOP; n++) {
		devnr[n] = ioctl(cfd, LOOP_CTL_GET_FREE);
		if (devnr[n] < 0) {
			fprintf(stderr, "ioctl LOOP_CTL_GET_FREE failed %d (%s)\n",
				errno, strerror(errno));
			break;
		}

		snprintf(filename, sizeof(filename), "backing-file-%d-%d", pid, devnr[n]);
		bfd = open(filename, O_RDWR | O_CLOEXEC | O_CREAT, S_IRUSR | S_IWUSR);
		if (bfd < 0) {
			fprintf(stderr, "can't create backing store file '%s': %d (%s)\n",
				filename, errno, strerror(errno));
			break;
		}

		snprintf(filename, sizeof(filename), "/dev/loop%d", devnr[n]);
		printf("Using %s\n", filename);

		lfd = open(filename, O_RDWR);
		if (lfd < 0) {
			fprintf(stderr, "open of loop device %s failed %d (%s)\n",
				filename, errno, strerror(errno));
			(void)close(bfd);
			break;
		}

		err = ioctl(lfd, LOOP_SET_FD, bfd);
		if (err < 0) {
			fprintf(stderr, "ioctl LOOP_SET_FD failed %d (%s)\n",
				errno, strerror(errno));
			(void)close(bfd);
			break;
		}
		(void)close(bfd);
		(void)close(lfd);
	}

	for (i = 0; i < n; i++) {
		snprintf(filename, sizeof(filename), "backing-file-%d-%d", pid, devnr[n]);
		bfd = open(filename, O_RDWR | O_CLOEXEC, S_IRUSR | S_IWUSR);
		if (bfd < 0) {
			fprintf(stderr, "can't re-open backing store file '%s': %d (%s)\n",
				filename, errno, strerror(errno));
		} else {
			snprintf(filename, sizeof(filename), "/dev/loop%d", devnr[i]);
			lfd = open(filename, O_RDWR);
			if (lfd >= 0) {
				err = ioctl(lfd, LOOP_CLR_FD, bfd);
				if (err < 0) {
					fprintf(stderr, "ioctl LOOP_CLR_FD failed %d (%s)\n",
						errno, strerror(errno));
				}
				(void)close(lfd);
			}
			(void)close(bfd);
retry:
			err = ioctl(cfd, LOOP_CTL_REMOVE, devnr[i]);
			if (err < 0) {
				if (errno == EBUSY) {
					usleep(2000);
					goto retry;
				}
				fprintf(stderr, "ioctl LOOP_CTL_REMOVE failed %d (%s)\n",
					errno, strerror(errno));
			}
		}
	}
	(void)close(cfd);

	return 0;
}
	
int main()
{
	return loop_create(65536);
}
