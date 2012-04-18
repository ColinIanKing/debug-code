#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static inline int rdrand16(uint16_t *val)
{
	uint16_t 	tmp;
	int ret;

        asm("rdrand %%ax;\n\
        mov $1,%%edx;\n\
        cmovae %%ax,%%dx;\n\
        mov %%edx,%1;\n\
        mov %%ax, %0;":"=r"(tmp),"=r"(ret)::"%ax","%dx");

        *val = tmp;
	return ret;
}

int main(int argc, char **argv)
{	
	uint16_t v;

	rdrand16(&v);
	printf("Rand16: 0x%hx\n", v);

	exit(EXIT_SUCCESS);
}
