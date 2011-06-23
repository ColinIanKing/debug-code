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

#define WAKE_STATUS_MASK	0x8000
#define SLEEP_TYPE_MASK		0x1c00
#define SLEEP_TYPE_SHIFT	0x0a
#define SLEEP_ENABLE_MASK	0x2000
#define SLEEP_ENABLE_SHIFT	0x0d

#define S5_STATE		0x7


void portperm(uint32_t port, uint32_t width, const char *name)
{
	if (port > 0) {
		if (ioperm(port, width,1) < 0) {
			fprintf(stderr, "Cannot get access to %s\n", name);
			exit(EXIT_FAILURE);
		}
	}
}

void port_write(uint32_t reg_a, uint32_t reg_b, uint32_t val)
{
	outl(val, reg_a);
	if (reg_b)
		outl(val, reg_b);
}

uint32_t port_read(uint32_t reg_a, uint32_t reg_b) 
{
	uint32_t val;

	val = inl(reg_a);
	if (reg_b)
		val |= inl(reg_b);

	return val;
}

int main(int argc, char **argv)
{
	uint32_t PM1a_EVT_BLK;
	uint32_t PM1b_EVT_BLK;
	uint32_t PM1a_CNT_BLK;
	uint32_t PM1b_CNT_BLK;
	uint32_t val;

	if (argc != 5) {
		fprintf(stderr, "Syntax: %s PM1a_EVT_BLK PM1b_EVT_BLK PM1a_CNT_BLK PM1b_CNT_BLK\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	sscanf(argv[1], "%i", &PM1a_EVT_BLK);
	sscanf(argv[2], "%i", &PM1b_EVT_BLK);
	sscanf(argv[3], "%i", &PM1a_CNT_BLK);
	sscanf(argv[4], "%i", &PM1b_CNT_BLK);

	printf("PM1a_EVT_BLK = 0x%x\n", PM1a_EVT_BLK);
	printf("PM1b_EVT_BLK = 0x%x\n", PM1b_EVT_BLK);
	printf("PM1a_CNT_BLK = 0x%x\n", PM1a_CNT_BLK);
	printf("PM1b_CNT_BLK = 0x%x\n", PM1b_CNT_BLK);

	portperm(PM1a_EVT_BLK, 4, "PM1a_EVT_BLK");
	portperm(PM1b_EVT_BLK, 4, "PM1b_EVT_BLK");
	portperm(PM1a_CNT_BLK, 4, "PM1a_CNT_BLK");
	portperm(PM1b_CNT_BLK, 4, "PM1b_CNT_BLK");

	/* Step 1, if already set then clear WAKE_STATUS by
	   writing 1 to that bit */

	val = port_read(PM1a_EVT_BLK, PM1b_EVT_BLK);
	val &= WAKE_STATUS_MASK;
	if (val)
		port_write(PM1a_EVT_BLK, PM1b_EVT_BLK, val);

	/* Step 2, clear SLP_EN and SLP_TYP */
	val = port_read(PM1a_CNT_BLK, PM1b_CNT_BLK);
	val &= ~(SLEEP_ENABLE_MASK | SLEEP_TYPE_MASK);
	val |= (S5_STATE << SLEEP_TYPE_SHIFT);
	port_write(PM1a_CNT_BLK, PM1b_CNT_BLK, val);

	/* Step 3, set SLP_EN */
	val |= (SLEEP_ENABLE_MASK);
	port_write(PM1a_CNT_BLK, PM1b_CNT_BLK, val);

	/* Should be reset by now */
	sleep(1);

	exit(EXIT_FAILURE);
}
