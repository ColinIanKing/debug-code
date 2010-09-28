#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define cpuid(in, eax, ebx, ecx, edx)	\
  asm("cpuid":  "=a" (eax), 		\
		"=b" (ebx), 		\
		"=c" (ecx), 		\
		"=d" (edx) : "a" (in))

int main(int argc, char **argv)
{
	uint32_t  eax, ebx, ecx, edx;
	int cstate;

	eax = ebx = ecx = edx = 0;

	cpuid(5, eax, ebx, ecx, edx);
	if ((edx == 0) || (ecx & 1) == 0)
		printf("Cannot determine C states\n");
	else {
		printf("CPU has following C States:");
		for (cstate = 0; edx; cstate++, edx >>=4)
			if (edx & 0x07)
				printf(" C%d", cstate);
		printf("\n");
	}
	exit(EXIT_SUCCESS);
}
