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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_KIDS	8

int pids[MAX_KIDS];

void reap(void)
{
	int i;
	siginfo_t info;
	static int reaped = 0;

	if (reaped == 0) {
		printf("\nKilling processes!\n");
		reaped++;
		for (i=0;i<MAX_KIDS;i++) {
			if (pids[i]) {
				kill(pids[i], SIGKILL);
				waitid(P_PID, pids[i], &info, WEXITED);
				pids[i] = 0;
			}
		}
	}
}

void burncycles1(void)
{
	double f = 0.00001;

	signal(SIGINT, SIG_IGN);

	while (f > 0.0L) 
		f += 0.00009233;
}

void burncycles2(void)
{
	float f = 0.00001;

	signal(SIGINT, SIG_IGN);

	while (f > 0.0L) 
		f += 0.00013;
}

void burncycles3(void)
{
	signal(SIGINT, SIG_IGN);

	for (;;) {
	}
}

void burncycles4(void)
{
	register int x;

	signal(SIGINT, SIG_IGN);

	for (x=0;;) {
		x ^= random();
	}
}

void (*funcs[])(void) = {
	burncycles1, burncycles2, burncycles3, burncycles4,
};

void handler(int dummy)
{
	reap();
}

int main(int argc, char **argv)
{
	int i;

	for (i=0;i<MAX_KIDS;i++) {
		int pid = fork();
		if (pid == 0)
			funcs[i % (sizeof(funcs)/sizeof(*funcs))]();
		else
			pids[i] = pid;
	}
	signal(SIGINT, handler);
	
	sleep(100000);	/* Zzzzz */

	exit(EXIT_SUCCESS);
}
