#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "rdrand.h"

#define ITERATIONS	100000000

#define cpuid(in, eax, ebx, ecx, edx)   \
  asm("cpuid":  "=a" (eax),             \
                "=b" (ebx),             \
                "=c" (ecx),             \
                "=d" (edx) : "a" (in))

int main(int argc, char **argv)
{	
	uint64_t r64;
	uint64_t i;
	unsigned long long usec1;
	unsigned long long usec2;
	struct timeval tv1, tv2;
	double nsec;
	uint32_t eax, ebx, ecx, edx = 0;

	cpuid(0, eax, ebx, ecx, edx);
	if (!((memcmp(&ebx, "Genu", 4) == 0) &&
	      (memcmp(&edx, "ineI", 4) == 0) &&
	      (memcmp(&ecx, "ntel", 4) == 0))) {
		fprintf(stderr, "Not a recognised Intel CPU.\n");
		exit(EXIT_FAILURE);
	}
	cpuid(1, eax, ebx, ecx, edx);
	if (!(ecx & 0x40000000)) {
		fprintf(stderr, "CPU does not support rdrand.\n");
		exit(EXIT_FAILURE);
	}
	
	gettimeofday(&tv1, NULL);

	for (i = 0; i < ITERATIONS; i++)
		rdrand64(&r64);

	gettimeofday(&tv2, NULL);

	usec1 = (tv1.tv_sec * 1000000) + tv1.tv_usec;
	usec2 = (tv2.tv_sec * 1000000) + tv2.tv_usec;
	nsec = 1000.0 * (double)(usec2 - usec1) / ITERATIONS;

	printf("%g nsec per rdrand64 (%.6f million rdrand64 per second)\n",
		nsec, 1000.0 / nsec);

	exit(EXIT_SUCCESS);
}
