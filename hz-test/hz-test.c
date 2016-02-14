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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_TASKS	64

static pid_t *cpu_pids;

static void cpu_consume_kill(void)
{
	int i;
	siginfo_t info;

	for (i=0;i<MAX_TASKS;i++) {
		if (cpu_pids[i] != 0) {
			kill(cpu_pids[i], SIGUSR1);
			waitid(P_PID, cpu_pids[i], &info, WEXITED);
		}
	}
}

static void cpu_consume_sighandler(int dummy)
{
	exit(0);
}

static void cpu_sigint_handler(int dummy)
{
	cpu_consume_kill();
	exit(0);
}

static void cpu_consume_cycles(void)
{
	signal(SIGUSR1, cpu_consume_sighandler);

	float dummy = 0.000001;
	unsigned long long i = 0;

	while (dummy > 0.0) {
		dummy += 0.0000037;
		i++;
	}
}


void cpu_consume_complete(void)
{
	cpu_consume_kill();
	free(cpu_pids);	
}

int cpu_consume_start(void)
{
	int i;

	if ((cpu_pids = (pid_t*)calloc(MAX_TASKS, sizeof(pid_t))) == NULL)
		return -1;

	signal(SIGINT, cpu_sigint_handler);

	for (i=0;i<MAX_TASKS;i++) {
		pid_t pid;

		pid = fork();
		switch (pid) {
		case 0: /* Child */
			cpu_consume_cycles();	
			break;
		case -1:
			/* Went wrong */
			cpu_consume_complete();
			return -1;
		default:
			cpu_pids[i] = pid;
			break;
		}
	}
	return 0;
}

typedef unsigned long long u64;
typedef unsigned long      u32;

static inline u64 rdtsc(void)
{
	if (sizeof(long) == sizeof(u64)) {
		u32 lo, hi;
        	asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
		return ((u64)(hi) << 32) | lo;
	}
	else {
		u64 tsc;
        	asm volatile("rdtsc" : "=A" (tsc));
		return tsc;
	}
}


void calc_mean_and_stddev(double *values, int len, double *mean, double *stddev)
{
	int i;
	double total = 0.0;

	for (i=0;i<len;i++) {
		total += values[i];
	}
	*mean = (total / (double)len);

	total = 0.0;
	for (i=0;i<len;i++) {
		double d;
	
		d = values[i] - *mean;
		d *= d;
		total += d;
	}
	*stddev = sqrt(total / (double)len);
}

typedef struct {
	int user;
	int nice;
	int sys;
	int idle;
	int iowait;
	int irq;
	int softirq;
	int ctxt;
	int irqs;
} stats;

int read_sys_stats(stats *info)
{
	FILE *fp;
	char buffer[4096];
	stats stat;

	fp = fopen("/proc/stat", "r");
	if (fp == NULL)	
		return 0;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (strncmp(buffer, "cpu ", 4) == 0) {
			char cpu[10];
			sscanf(buffer, "%s %d %d %d %d %d %d %d",
				cpu, &stat.user, &stat.nice, &stat.sys, &stat.idle, &stat.iowait, &stat.irq, &stat.softirq);
		}	
		if (strncmp(buffer, "ctx", 3) == 0) {
			char ctxt[10];
			sscanf(buffer, "%s %d",
				ctxt, &stat.ctxt);
		}
		if (strncmp(buffer, "intr", 4) == 0) {
			int irq;
			char *ptr = buffer+4;

			stat.irqs = 0;

			while (*ptr == ' ') {
				while (*ptr == ' ')
					ptr++;
				irq = atoi(ptr);
				while (isdigit(*ptr))
					ptr++;
				stat.irqs += irq;
			}
		}
	}
	fclose(fp);

	*info = stat;

	return 1;
}

void test_cpu_loads(void)
{
#define CPU_LOAD_SAMPLES	60
	int i;
	stats s1,s2;

	double values[9][CPU_LOAD_SAMPLES];

	char *tags[] = {
		"User Time",
		"Niced Time",
		"System Time",
		"Idle Time",
		"I/O Wait Time",
		"IRQ Time",
		"Soft IRQ Time",
		"Context Switches/sec",
		"IRQs/sec"
	};

	read_sys_stats(&s1);
	sleep(1);

	for (i=0;i<CPU_LOAD_SAMPLES;i++) {
		read_sys_stats(&s2);
		double total;

		/*
		printf("usr: %d, niced %d, sys: %d, idle: %d, iowait: %d, irq: %d, softirq: %d, ctxt: %d, irqs: %d\n",
			s2.user - s1.user,
			s2.nice - s1.nice,
			s2.sys  - s1.sys,
			s2.idle - s1.idle,
			s2.iowait - s1.iowait,
			s2.irq  - s1.irq,
			s2.softirq - s1.softirq,	
			s2.ctxt - s1.ctxt,		
			s2.irqs - s1.irqs);
		*/

		total = (double)((s2.user - s1.user) +
			 	(s2.nice - s1.nice) +
			 	(s2.sys  - s1.sys) +
			 	(s2.idle - s1.idle) +
			 	(s2.iowait - s1.iowait) +
			 	(s2.irq  - s1.irq) +
			 	(s2.softirq - s1.softirq));
		total = total / 100.0;
	
		values[0][i] = (double)(s2.user - s1.user) / total;
		values[1][i] = (double)(s2.nice - s1.nice) / total;
		values[2][i] = (double)(s2.sys - s1.sys) / total;
		values[3][i] = (double)(s2.idle - s1.idle) / total;
		values[4][i] = (double)(s2.iowait - s1.iowait) / total;
		values[5][i] = (double)(s2.irq - s1.irq) / total;
		values[6][i] = (double)(s2.softirq - s1.softirq) / total;
		values[7][i] = (double)(s2.ctxt - s1.ctxt) / total;
		values[8][i] = (double)(s2.irqs - s1.irqs) / total;

		s1 = s2;
		sleep(1);
	}

	for (i=0;i<9;i++) {
		double mean, stddev;
		calc_mean_and_stddev(values[i], CPU_LOAD_SAMPLES, &mean, &stddev);
		printf("%20.20s: Mean: %8.2f%s (StdDev %f)\n", tags[i], mean, i < 7 ? "%" : " ", stddev);
	}
}

void test_cpu_usage(void)
{
#define CPU_SAMPLES	60
	int i;
	u64 t1,t2;
	double values[CPU_SAMPLES];
	double mean, stddev;

	printf("Calculating loops per 1,000,000,000 TSC ticks\n");

	for (i=0;i<CPU_SAMPLES;i++) {
		register u64 count = 0;

		t1 = rdtsc();
		t2 = t1 + 1000000000ULL;
		while (rdtsc() < t2) {
			count++;
		}
		values[i] = (double)count;
	}
	calc_mean_and_stddev(values, CPU_SAMPLES, &mean, &stddev);

	printf("Loops per billion TSC ticks (%d tests): Mean: %14.2f, StdDev: %14.2f\n\n",
			CPU_SAMPLES, mean, stddev);
}


void test_clock_jitter(void)
{
#define SAMPLES	25
	long long int us;

	printf("Delay (us)           Mean            StdDev                Mean           StdDev         Deviation\n");
	printf("                   TSC ticks       TSC ticks            Clock Time      Clock Time       from Clock\n");

	for (us=100;us<=10000000;us *= 10) {
		int j;
		struct timespec req;
		double tsc_timings[SAMPLES];	
		double tv_timings[SAMPLES];
		double tsc_mean, tv_mean;
		double tsc_stddev, tv_stddev;
		double accuracy;
		u64 t1,t2;

		for (j=0;j<SAMPLES;j++) {
			req.tv_sec = us / 1000000;
			req.tv_nsec = (us * 1000) % 1000000000;
			struct timeval tv1,tv2;
			
			gettimeofday(&tv1, NULL);
			t1 = rdtsc();
			nanosleep(&req, NULL);
			t2 = rdtsc();
			gettimeofday(&tv2, NULL);

			tsc_timings[j] = (double)(t2 - t1);
			tv_timings[j] = (double)
				(((tv2.tv_sec - tv1.tv_sec) * 1000000) +
				 (tv2.tv_usec - tv1.tv_usec));
		}

		calc_mean_and_stddev(tsc_timings, SAMPLES, &tsc_mean, &tsc_stddev);
		calc_mean_and_stddev(tv_timings, SAMPLES, &tv_mean, &tv_stddev);
	
		accuracy = fabs((double)(us) - tv_mean) / ((double)(us)) * 100.0;

		printf("%10lld\t%14.3f\t%14.3f\t\t%12.3f\t%12.3f\t%8.3f%%\n",
			us, tsc_mean, tsc_stddev, tv_mean, tv_stddev, accuracy);
	}
		
}

int main(int argc, char **argv)
{
	cpu_consume_start();
	test_cpu_loads();
	test_cpu_usage();
	cpu_consume_complete();
	test_clock_jitter();
	
	exit(EXIT_SUCCESS);
}
