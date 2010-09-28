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

/*
 *  Author Colin Ian King,  colin.king@canonical.com
 */


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
