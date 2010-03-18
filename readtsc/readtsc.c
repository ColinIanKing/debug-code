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

int main(int argc, char **argv)
{
	printf("%llx\n",rdtsc());

	exit(EXIT_SUCCESS);
}
