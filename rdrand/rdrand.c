#include <stdint.h>

/* From http://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide/ */

#include "rdrand.h"

static inline int __rdrand16(uint16_t *val)
{
	uint16_t 	tmp;
	int 		ret;

        asm("rdrand %%ax;\n\
        mov $1,%%edx;\n\
        cmovae %%ax,%%dx;\n\
        mov %%edx,%1;\n\
        mov %%ax, %0;":"=r"(tmp),"=r"(ret)::"%ax","%dx");

        *val = tmp;
	return ret;
}

static inline int __rdrand32(uint32_t *val)
{
	uint32_t 	tmp;
	int 		ret;

        asm("rdrand %%eax;\n\
        mov $1,%%edx;\n\
        cmovae %%eax,%%edx;\n\
        mov %%edx,%1;\n\
        mov %%eax, %0;":"=r"(tmp),"=r"(ret)::"%eax","%edx");

        *val = tmp;
	return ret;
}

static inline int __rdrand64(uint64_t *val)
{
	uint64_t	tmp;
	int 		ret;

        asm("rdrand %%rax;\n\
        mov $1,%%edx;\n\
        cmovae %%rax,%%rdx;\n\
        mov %%edx,%1;\n\
        mov %%rax, %0;":"=r"(tmp),"=r"(ret)::"%rax","%rdx");

        *val = tmp;
	return ret;
}

void rdrand16(uint16_t *val)
{
        while (__rdrand16(val) == 0)
                ;
}

void rdrand32(uint32_t *val)
{
        while (__rdrand32(val) == 0)
                ;
}

void rdrand64(uint64_t *val)
{
        while (__rdrand64(val) == 0)
                ;
}

