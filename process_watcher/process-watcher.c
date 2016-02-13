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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PROCESS_INFO_SIZE 1031
#define MAX_SAMPLES (60)		/* Default, run for 60 seconds */

typedef struct {
	pid_t pid;
	uint32_t usr;
	uint32_t sys;
	uint32_t previous_usr;
	uint32_t previous_sys;
	uint32_t total_usr;
	uint32_t total_sys;
	bool active;
	char name[128];
} process_info_t ;

typedef struct {
	uint32_t delta_usr;
	uint32_t delta_sys;
} sample_t;

static bool stop;

static inline size_t pid_hash(const pid_t pid)
{
	return ((size_t)pid * 3) % PROCESS_INFO_SIZE;
}

/*
 *  Sort based on total user and system time
 */
static int process_info_compare(const void *p1, const void *p2)
{
	process_info_t *item1 = (process_info_t*)p1;
	process_info_t *item2 = (process_info_t*)p2;

	return (item1->total_usr + item1->total_sys) <
	       (item2->total_usr + item2->total_sys);
}

static void syntax(char **argv)
{
	fprintf(stderr, "%s: [-s samples ] [-c] [-p]\n", argv[0]);
	fprintf(stderr, "    -s specifies number of 1 second samples to make\n");
	fprintf(stderr, "    -c print CPU usage summary\n");
	fprintf(stderr, "    -p process samples data dump\n");
}

static void handler(int dummy)
{
	printf("\nGot SIGINT\n");
	stop = true;
}


int main(int argc, char **argv)
{
	bool cpu_summary = false;
	bool process_summary = false;
	int32_t sample;
	int i;
	int opt;
	int32_t max_samples = MAX_SAMPLES;
	uint32_t active = 0;

	process_info_t *process_info;
	sample_t **samples;

	while ((opt = getopt(argc, argv, "cps:")) != -1) {
		switch (opt) {
		case 's':
			max_samples = atoi(optarg);
			break;
		case 'c':
			cpu_summary = true;
			break;
		case 'p':
			process_summary = 1;
			break;
		default:
			syntax(argv);
			exit(EXIT_FAILURE);
		}
	}


	/* Defualt to do at least something! */
	if (!cpu_summary && !process_summary)
		cpu_summary = true;

	/* Allocate tables */
	process_info = (process_info_t *)malloc(PROCESS_INFO_SIZE * sizeof(process_info_t));
	if (!process_info) {
		fprintf(stderr, "Out of memory allocating process data\n");
		exit(EXIT_FAILURE);
	}
	samples = (sample_t **)malloc(max_samples * sizeof(sample_t *));
	if (!samples) {
		fprintf(stderr, "Out of memory allocating samples data\n");
		exit(EXIT_FAILURE);
	}
	for (i =0; i < max_samples; i++) {
		samples[i] = malloc(PROCESS_INFO_SIZE * sizeof(sample_t));
		if (!samples[i]) {
			fprintf(stderr, "Out of memory allocating samples data\n");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < PROCESS_INFO_SIZE; i++)
		process_info[i].pid = -1;

	signal(SIGINT, handler);

	/* Gather data */
	for (sample= -1; (!stop) && (sample < max_samples); sample++) {
		uint32_t total_usr = 0;
		uint32_t total_sys = 0;
		DIR *dir;
		struct dirent *dirent;

		dir = opendir("/proc");

		while ((dirent = readdir(dir)) != NULL) {
			/* Get pid data only */
			if (isdigit(dirent->d_name[0])) {
				char filename[256];
				char buffer[4096];
				int fd;
				pid_t pid = atoi(dirent->d_name);
				size_t index = pid_hash(pid);

				while (process_info[index].pid != pid) {
					if (process_info[index].pid == -1)
						break;
					index++;
					if (index >= PROCESS_INFO_SIZE)
						index = 0;
				}
				process_info[index].pid = pid;
				sprintf(filename,"/proc/%s/stat", dirent->d_name);

				if ((fd = open(filename, O_RDONLY)) < 0)
					continue;

				if (read(fd, buffer, sizeof(buffer)) > 0) {
					uint32_t usr, sys;
					char name[256];

					if (sscanf(buffer,"%*d %s %*s %*d %*d %*d %*d %*d %*d %*d %*d "
						"%*d %*d %" SCNu32 " %" SCNu32, name, &usr, &sys) == 3) {
						uint32_t delta_usr, delta_sys;

						strncpy(process_info[index].name,name,sizeof(process_info[index].name));
						process_info[index].previous_usr = process_info[index].usr;
						process_info[index].previous_sys = process_info[index].sys;
						process_info[index].usr = usr;
						process_info[index].sys = sys;

						delta_usr = process_info[index].usr - process_info[index].previous_usr;
						delta_sys = process_info[index].sys - process_info[index].previous_sys;

						total_usr += delta_usr;
						total_sys += delta_sys;

						if (sample != -1) {
							samples[sample][index].delta_usr = delta_usr;
							samples[sample][index].delta_sys = delta_sys;

							if ((delta_usr + delta_sys) > 0)
								process_info[index].active = true;
						}
					}
				}
				(void)close(fd);
			}
		}
		(void)closedir(dir);

		if (sample > -1) {
			fprintf(stderr,"%" PRId32 " of %" PRId32 "\r", sample, max_samples);
			fflush(stderr);
		}
		sleep(1);
	}

	if (stop)
		max_samples = sample;

	for (i = 0; i < PROCESS_INFO_SIZE; i++) {
		if (process_info[i].active)
			active++;
	}

	printf("Total of %" PRIu32 " active processes\n", active);

	if (process_summary) {
		printf("\nPer Second Sample Summary:\n\n");

		for (i = 0; i < PROCESS_INFO_SIZE; i++) {
			if (process_info[i].active)
				printf("%-5d      ",process_info[i].pid);
		}
		printf("\n");
		for (i = 0; i < PROCESS_INFO_SIZE; i++) {
			if (process_info[i].active > 0) {
				char *ptr = process_info[i].name;
				while (*ptr) {
					if (*ptr == ')')
						*ptr = '\0';
					ptr++;
				}
				printf("%-8.8s . ",&process_info[i].name[1]);
			}
		}
		printf("\n");
		for (i = 0; i < PROCESS_INFO_SIZE; i++) {
			if (process_info[i].active > 0)
				printf("%3s  %3s   ", "usr", "sys");
		}
		printf("\n");

		for (sample = 0; sample < max_samples; sample++) {
			for (i = 0; i < PROCESS_INFO_SIZE; i++) {
				if (process_info[i].active > 0)
					printf("%-3" PRIu32 " %-3" PRIu32 "   ",
						samples[sample][i].delta_usr,
						samples[sample][i].delta_sys);
			}
			printf("\n");
		}
	}

	if (cpu_summary) {
		uint32_t grand_total_usr = 0;
		uint32_t grand_total_sys = 0;
		pid_t mypid = getpid();

		for (i = 0;i < PROCESS_INFO_SIZE; i++) {
			if (process_info[i].active > 0) {
				for (sample=0;sample<max_samples;sample++) {
					process_info[i].total_usr += samples[sample][i].delta_usr;
					process_info[i].total_sys += samples[sample][i].delta_usr;
				}
				grand_total_usr += process_info[i].total_usr;
				grand_total_sys += process_info[i].total_sys;
			}
		}
		qsort(process_info, PROCESS_INFO_SIZE, sizeof(process_info_t), process_info_compare);

		printf("\nTotal CPU Usage Summary:\n\n");

		printf(" PID\t%% USR\t%% USR\t%% SYS\t%% SYS\tPROCESS\n");
		printf("\t(share)\t(CPU)\t(share)\t(CPU)\n");
		for (i = 0; i < PROCESS_INFO_SIZE; i++) {
			if (process_info[i].active > 0) {
				printf("%5d\t%6.2f\t%6.2f\t%6.2f\t%6.2f\t%s %s\n",
					process_info[i].pid,
					((float)process_info[i].total_usr/(float)grand_total_usr)*100.0,
					((float)process_info[i].total_usr/(float)(max_samples*100))*100.0,
					((float)process_info[i].total_sys/(float)grand_total_sys)*100.0,
					((float)process_info[i].total_sys/(float)(max_samples*100))*100.0,
					&process_info[i].name[1],
					mypid == process_info[i].pid ? "(Monitoring overhead)" : "");
			}
		}
		printf("\t%6.2f\t%6.2f\t%6.2f\t%6.2f\n",
			100.0,((float)grand_total_usr/(float)(max_samples*100))*100.0,
			100.0,((float)grand_total_sys/(float)(max_samples*100))*100.0);
	}

	for (i = 0; i < max_samples; i++)
		free(samples[i]);
	free(samples);
	free(process_info);

	exit(EXIT_SUCCESS);
}
