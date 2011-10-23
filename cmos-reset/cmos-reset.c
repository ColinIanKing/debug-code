/*
 * Copyright (C) 2011 Canonical
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
#include <stdint.h>
#include <unistd.h>
#include <sys/io.h>

#define CMOS_CHECKSUM_HI (0x2e)
#define CMOS_CHECKSUM_LO (0x2f)

static inline uint8_t cmos_read(uint8_t addr)
{
	outb(addr, 0x70);	/* specify address to read */
	outb(0, 0x80);		/* tiny delay */
	return inb(0x71);	/* read value */
}

static inline void cmos_write(uint8_t addr, uint8_t val)
{
	outb(addr, 0x70);	/* specify address to write */
	outb(0, 0x80);		/* tiny delay */
	outb(val, 0x71);	/* write value */
}

static inline void cmos_invert(uint8_t addr)
{
	cmos_write(addr, 255 ^ cmos_read(addr));
}

int main(int argc, char **argv)
{
	if (ioperm(0x70, 2, 1) < 0) {
		fprintf(stderr, "ioperm failed on ports 0x70 and 0x71\n");
		exit(1);
	}
	if (ioperm(0x80, 1, 1) < 0) {
		fprintf(stderr, "ioperm failed on port 0x80\n");
		exit(1);
	}
	if (iopl(3) < 0) {
		fprintf(stderr, "iopl failed\n");
		exit(1);
	}

	asm("cli");
	/* Invert CMOS checksum, high and low bytes*/
	cmos_invert(CMOS_CHECKSUM_HI);
	cmos_invert(CMOS_CHECKSUM_LO);
	asm("sti");

	(void)iopl(0);
	(void)ioperm(0x80, 1, 0);
	(void)ioperm(0x70, 2, 0);

	exit(0);
}
