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
#include <unistd.h>
#include <stdlib.h>
#include <sys/io.h>

int main(int argc, char **argv)
{
	ioperm(0x42, 4, 1);
        ioperm(0x61, 4, 1);
        ioperm(0x80, 1, 1);

	beep_440hz();
	sleep(5);
	beep_off();

	exit(EXIT_SUCCESS);
}
