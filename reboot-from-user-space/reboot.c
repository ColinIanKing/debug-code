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
#include <unistd.h> 
#include <sys/io.h> 
#include <stdlib.h>

#define KEYB_PORT	0x64
#define PCI_RESET_PORT	0xcf9

#define RESET_METHOD_KEYB	1
#define RESET_METHOD_PCI	2

/* Twiddle this setting to select method to reboot with */
#define RESET_METHOD		RESET_METHOD_PCI

int main(int argc, char **argv)
{
	printf("Reboot using ports writes\n");

	printf("Syncing discs!\n");
	sync();
	sleep(1);
	sync();
	sleep(1);

	printf("Rebooting!\n");

	switch (RESET_METHOD) {
	case RESET_METHOD_KEYB:
		if (ioperm(KEYB_PORT, 0x1, 1) < 0) {
			fprintf(stderr,"Cannot access port!\n");
			exit(EXIT_FAILURE);
		}
		outb(0xfe, KEYB_PORT);
		break;
	case RESET_METHOD_PCI:
		if (ioperm(PCI_RESET_PORT, 0x1, 1) < 0) {
			fprintf(stderr,"Cannot access port!\n");
			exit(EXIT_FAILURE);
		}
		outb(0x02, PCI_RESET_PORT);
		usleep(10);
		outb(0x04, PCI_RESET_PORT);
		break;
	default:
		fprintf(stderr,"Unknown reset method!\n");
		break;
	}
	printf("Reboot failed :-(\n");

	exit(EXIT_FAILURE);
}
