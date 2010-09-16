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

/*  Example of how to push keyboard scan codes into keyboard controller */

#include <unistd.h>
#include <stdlib.h>
#include <sys/io.h>

#define I8042_DATA_REG          0x60
#define I8042_STATUS_REG        0x64

void push_keycode(unsigned char code)
{
        while (inb(I8042_STATUS_REG) & 0x2) 
		;
	outb(0xd2, I8042_STATUS_REG);
        while (inb(I8042_STATUS_REG) & 0x2) 
		;
	outb(code, I8042_DATA_REG);
}

/*  keycodes in http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */
unsigned char hello[] = { 
	0x23, 0x12, 0x26, 0x26, 0x18, 
	0x39, 0x11, 0x18, 0x13, 0x20, 0
};

int main(int argc, char **argv)
{
	unsigned char *ptr = hello;

	ioperm(0x64, 1, 1);
	ioperm(0x60, 1, 1);

	for (ptr = hello; *ptr; ptr++) {
		push_keycode(*ptr);		/* Key down */
		usleep(1000);
		push_keycode(*ptr | 0x80);	/* Key up */
		usleep(10000);
	}

	ioperm(0x64, 1, 0);
	ioperm(0x60, 1, 0);
	
	exit(EXIT_SUCCESS);
}
