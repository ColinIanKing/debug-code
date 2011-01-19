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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sched.h>

#include <libx86.h>

typedef struct {
	uint32_t base_lo;
	uint32_t base_hi;
	uint32_t length_lo;
	uint32_t length_hi;
	uint32_t type;
} e820_t;

int main(int argc, char **argv)
{	
	struct LRMI_regs regs;
	e820_t *e820;
	char *str;
	int cont = 0;
        cpu_set_t mask, oldset;

        /* First, go to the right cpu */

        sched_getaffinity(0, sizeof(oldset), &oldset);

        CPU_ZERO(&mask);
        CPU_SET(0, &mask);
        sched_setaffinity(0, sizeof(mask), &mask);

	LRMI_init();

	if ((e820 = LRMI_alloc_real(sizeof(e820_t))) == NULL) {
		fprintf(stderr, "Cannot allocate e820 buffer from real memory pool.\n");
		exit(EXIT_FAILURE);
	}

	printf("Base Address     Length           Type\n");

	do {
		memset(&regs,0,sizeof(struct LRMI_regs));

		regs.eax = 0xe820;
		regs.ebx = cont;
		regs.ecx = 20;
		regs.edx = ('S' << 24) | 
		   	   ('M' << 16) |
                   	   ('A' << 8) |
                   	   ('P');
		regs.es = ((unsigned long)e820)>>4;
		regs.edi = ((unsigned long)e820)&0xf;
		LRMI_int(0x15, &regs);

		if (regs.flags & 1)
			break;

		cont = regs.ebx;
	
		switch (e820->type) {
		case 1:	
			str = "RAM";
			break;
		case 2:
			str = "Reserved";
			break;
		case 3:
			str = "ACPI Reclaim";
			break;
		case 4:
			str = "ACPI NVS memory";
			break;
		case 5:
			str = "Unusable";
			break;
		case 6:
			str = "Disabled";
			break;
		default:
			str = "Unknown";
			break;
		}

		printf("%8.8x%8.8x %8.8x%8.8x %s\n",
			e820->base_hi, e820->base_lo,
			e820->length_hi, e820->length_lo,
			str);

	} while (cont != 0);
	LRMI_free_real(e820);

	exit(EXIT_SUCCESS);
}
