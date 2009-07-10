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
#include <sys/io.h>

#define I8042_DATA_REG          0x60
#define I8042_STATUS_REG        0x64

extern void flash_led(void);

int main(int argc, char **argv)
{
	ioperm(I8042_STATUS_REG, 1, 1);
	ioperm(I8042_DATA_REG, 1, 1);

	for (;;) 
		flash_led();
}
