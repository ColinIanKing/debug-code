/*
 * Copyright (C) 2017 Canonical, Ltd.
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

static sigjmp_buf jmp_env;

static void write_oom_adjustment(const char *path, const char *str)
{
	int fd;

	if ((fd = open(path, O_WRONLY)) >= 0) {
		ssize_t ret = write(fd, str, strlen(str));
		(void)ret;
		(void)close(fd);
	}
}

static inline void set_oom_adjustment(void)
{
	const bool high_priv = (getuid() == 0) && (geteuid() == 0);

	write_oom_adjustment("/proc/self/oom_adj", high_priv ? "-17" : "-16");
	write_oom_adjustment("/proc/self/oom_score_adj", high_priv ? "-1000" : "0");
}

static void sigbus_handler(int dummy)
{
	(void)dummy;

	siglongjmp(jmp_env, 1);
}

int main(int argc, char **argv)
{
	static struct sigaction new_action;
	size_t page_size = sysconf(_SC_PAGESIZE);
	size_t sz = page_size * 4;
	page_size = (page_size <= 0) ? 4096: page_size;

	if (geteuid()) {
		fprintf(stderr, "eatpages must be run with root privileges\n");
		exit(EXIT_FAILURE);
	}

	if (sigsetjmp(jmp_env, 1))
		return EXIT_FAILURE;

	new_action.sa_handler = sigbus_handler;
	new_action.sa_flags = 0;
	if (sigaction(SIGBUS, &new_action, NULL) < 0)
		return EXIT_FAILURE;
	new_action.sa_handler = SIG_IGN;
	if (sigaction(SIGCHLD, &new_action, NULL) < 0)
		return EXIT_FAILURE;

	printf("eatpages will now mark pages as poisoned and consume memory..\n");

	set_oom_adjustment();
	(void)sync();

	if (fork())
		_exit(0);
	if (fork())
		_exit(0);

	printf("eatpages running in background (PID %d)\n", getpid());

	(void)setsid();
	(void)close(0);
	(void)close(1);
	(void)close(2);
	
	for (;;) {
		void *buf = mmap(NULL, sz, PROT_READ,
				MAP_ANONYMOUS | MAP_SHARED, -1, 0);
		if (buf == MAP_FAILED) {
			if (sz > page_size)
				sz >>= 1;
			continue;
		}
		if (sigsetjmp(jmp_env, 1)) {
			(void)munmap((void *)buf, sz);
			continue;
		}
		(void)madvise(buf, sz, MADV_HWPOISON);
		(void)munmap((void *)buf, sz);
	}
	return EXIT_SUCCESS;
}
