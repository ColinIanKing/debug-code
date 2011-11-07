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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Real Mode IDT */
#define INT_VEC_START		(0x00000000)
#define INT_VEC_END  		(0x000003ff)
#define INT_VEC_SIZE		(INT_VEC_END - INT_VEC_START)

/* Legacy BIOS Option ROM region */
#define BIOS_ROM_REGION_START	(0x000c0000)
#define BIOS_ROM_REGION_END  	(0x000fffff)
#define BIOS_ROM_REGION_SIZE	(BIOS_ROM_REGION_END - BIOS_ROM_REGION_START)

#define EFI_SUPPORT		0x0001
#define VGA_SUPPORT		0x0002

bool maybeVGAROM(uint8_t *optROM, int length)
{
	int i;

	for (i=0; i<length-2; i++) {
		if ((*(optROM+i+0) == 'V') &&
		    (*(optROM+i+1) == 'G') &&
		    (*(optROM+i+2) == 'A')) {
			return true;
		}
	}
	return false;
}

int main(int argc, char **argv)
{
	uint32_t *intVec;
	uint8_t  *optROM;
	uint32_t int10hVec;
	int fd;
	int i;
	struct stat buf;
	int flag = 0;

	if (geteuid() != 0) {
		fprintf(stderr, "Need to run with root privilege.\n");
		exit(1);
	}

	if (stat("/sys/firmware/efi", &buf) != -1)
		flag |= EFI_SUPPORT;

	if ((fd  = open("/dev/mem", O_RDONLY)) < 0) {
		fprintf(stderr, "failed to open /dev/mem\n");
		exit(1);
	}

        if ((optROM = mmap(NULL, BIOS_ROM_REGION_SIZE, PROT_READ, MAP_PRIVATE, fd, BIOS_ROM_REGION_START)) == MAP_FAILED) {
		fprintf(stderr, "failed to mmap option ROM region\n");
		exit(1);
	}
	if ((intVec = mmap(NULL, INT_VEC_SIZE, PROT_READ, MAP_PRIVATE, fd, INT_VEC_START)) == MAP_FAILED) {
		fprintf(stderr, "failed to mmap realmode IDT\n");
		exit(1);
	}

	/* Get Int 10h vector from segment/offset realmode address */
	int10hVec = (intVec[0x10] & 0xffff) | ((intVec[0x10] & 0xffff0000)>> 12);
	/* printf("Int 10h @ 0x%x\n", int10hVec); */

	/* Look for any BIOS option ROMs that the int 10 vector jumps to */
	for (i=0; i<BIOS_ROM_REGION_SIZE; i+= 512) {
		if ((*(optROM+i) == 0x55) && (*(optROM+i+1) == 0xaa)) {
			int length = *(optROM+i+2) << 9;
			int ROMstart = BIOS_ROM_REGION_START+i;
			int ROMend = BIOS_ROM_REGION_START+i+length;

			if ((ROMstart <= int10hVec) && (int10hVec <= ROMend)) {
				printf("Found option ROM: %x..%x (%d bytes)%s\n",
					BIOS_ROM_REGION_START+i,
					BIOS_ROM_REGION_START+i+length,
					length,
					maybeVGAROM(optROM+i, length) ? 
						" (looks like a VGA ROM)" : "");
				flag |= VGA_SUPPORT;
				break;
			}	
		}
	}

	switch (flag) {
	case 0:
		/* Unlikely */
		printf("Legacy BIOS firmware does not have a video option ROM\n");
		break;
	case VGA_SUPPORT:
		printf("Legacy BIOS firmware has video option ROM with Int 10h support\n");
		break;
	case EFI_SUPPORT:
		printf("UEFI firmware seems to have no CSM support\n");
		break;
	case (EFI_SUPPORT | VGA_SUPPORT):
		printf("UEFI firmware seems to have CSM support with Int 10h support\n");
		break;
	}

	munmap(optROM, BIOS_ROM_REGION_SIZE);
	munmap(intVec, INT_VEC_SIZE);

	exit(0);
}
