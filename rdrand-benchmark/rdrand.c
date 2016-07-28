/*
 * Copyright (C) 2010-2012 Canonical
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
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

volatile int start_test;
volatile int end_test;

typedef struct {
	uint64_t usec;		/* duration of test in microseconds */
	uint64_t iter;		/* number of iterations taken */
	uint16_t thread_num;	/* thread instance number */
	pthread_t thread;	/* pthread info */
} info_t;

#define ITERATIONS	(100000000)

#define cpuid(in, eax, ebx, ecx, edx)   \
  asm("cpuid":  "=a" (eax),             \
                "=b" (ebx),             \
                "=c" (ecx),             \
                "=d" (edx) : "a" (in))


inline uint32_t rdrand16(void)
{
        uint32_t        ret;

        asm volatile("1:;\n\
        rdrand %0;\n\
        jnc 1b;\n":"=r"(ret));

        return ret;
}

inline uint32_t rdrand32(void)
{
        uint32_t        ret;

        asm volatile("1:;\n\
        rdrand %0;\n\
        jnc 1b;\n":"=r"(ret));

        return ret;
}

inline uint64_t rdrand64(void)
{
        uint64_t        ret;

        asm volatile("1:;\n\
        rdrand %0;\n\
        jnc 1b;\n":"=r"(ret));

        return ret;
}

void *test64(void *private)
{
	struct timeval tv1, tv2;
	uint64_t usec1;
	uint64_t usec2;
	info_t *info = (info_t *)private;
	register uint64_t i;

	end_test = false;

	while (!start_test)
		;

	if (info->thread_num == 0) {
		gettimeofday(&tv1, NULL);

		for (i = 0; i < info->iter; i++)
			rdrand64();

		end_test = true;
		gettimeofday(&tv2, NULL);

	} else {
		gettimeofday(&tv1, NULL);

		for (i = 0; !end_test; i += 32) {
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
			rdrand64();
		}

		gettimeofday(&tv2, NULL);
	}

	usec1 = (tv1.tv_sec * 1000000) + tv1.tv_usec;
	usec2 = (tv2.tv_sec * 1000000) + tv2.tv_usec;
	
	info->iter = i;
	info->usec = usec2 - usec1;

	return NULL;
}

void test(uint32_t threads)
{
	uint32_t i;
	uint64_t usec = 0;
	uint64_t iter = 0;
	info_t info[threads];
	void *ret;
	double nsec;

	start_test = false;

	for (i = 0; i < threads; i++) {
		info[i].thread_num = i;
		info[i].iter = ITERATIONS;
		pthread_create(&info[i].thread, NULL, test64, &info[i]);
	}

	start_test = true;

	for (i = 0; i < threads; i++)
		pthread_join(info[i].thread, &ret);

	for (i = 0; i < threads; i++) {
		usec += info[i].usec;
		iter += info[i].iter;
	}
	usec /= threads;
	nsec = 1000.0 * (double)usec / iter;
		
	printf("%" PRIu16 "\t%8.3f\t%8.3f\t  %12.7f\n", threads, nsec, 1000.0 / nsec, 64.0 / nsec);
}

int main(int argc, char **argv)
{	
	uint32_t eax, ebx, ecx, edx = 0;
	uint32_t i;
	uint32_t cpus = sysconf(_SC_NPROCESSORS_ONLN);

	/* Intel CPU? */
	cpuid(0, eax, ebx, ecx, edx);
	if (!((memcmp(&ebx, "Genu", 4) == 0) &&
	      (memcmp(&edx, "ineI", 4) == 0) &&
	      (memcmp(&ecx, "ntel", 4) == 0))) {
		fprintf(stderr, "Not a recognised Intel CPU.\n");
		exit(EXIT_FAILURE);
	}
	/* ..and supports rdrand? */
	cpuid(1, eax, ebx, ecx, edx);
	if (!(ecx & 0x40000000)) {
		fprintf(stderr, "CPU does not support rdrand.\n");
		exit(EXIT_FAILURE);
	}

	printf("Exericising 64 bit rdrands:\n");
	printf("Threads\trdrand\t\tmillion rdrands\t  billion bits\n");
	printf("\tduration (ns)\tper second\t  per second\n");
	for (i = 1; i <= cpus; i++) 
		test(i);

	exit(EXIT_SUCCESS);
}
