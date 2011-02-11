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
#include <string.h>

#include <libx86.h>

typedef struct {
	uint8_t  VESA_signature[4];
	uint16_t VESA_version;
	uint32_t OEM_string;
        uint32_t capabilities_flags;
	uint32_t video_mode;
	uint16_t total_memory;
	uint16_t oem_version;
	uint32_t vendor_name;
	uint32_t product_name;
	uint32_t product_revision;
} __attribute__ ((packed)) VBE_info;

typedef struct {
	uint16_t mode_attr;
	uint8_t  window_attr1;
	uint8_t  window_attr2;
	uint16_t granularity;
	uint16_t size;
	uint16_t start_segment1;
	uint16_t start_segment2;
	uint32_t win_pos_func;
	uint16_t bytes_per_scan_line;
	uint16_t width;
	uint16_t height;
	uint8_t  char_width;
	uint8_t  char_height;
	uint8_t  number_of_planes;
	uint8_t  bits_per_pixel;
} __attribute__ ((packed)) VBE_modeinfo;

typedef struct {
	uint8_t reserved[0x12];
	uint16_t width;
	uint16_t height;
} __attribute__ ((packed)) VBE_mode_info;

static inline void * phys_to_virt(uint32_t ptr)
{
	uint32_t seg = ptr >> 16;
	uint32_t off = ptr & 0xffff;

	return (void *)((seg << 4) | off);
}

int getSVGAmode(uint32_t mode, void *data)
{
	struct LRMI_regs regs;

	memset(&regs,0,sizeof(struct LRMI_regs));
	regs.eax = 0x4f01;
	regs.ecx = mode;
	regs.es = ((unsigned long)data)>>4;
	regs.edi = ((unsigned long)data)&0xf;
	LRMI_int(0x10, &regs);

	return regs.eax & 0xff;
}

int main(int argc, char **argv)
{	
	struct LRMI_regs regs;
	VBE_info *vbe_info;
	VBE_modeinfo *vbe_modeinfo;
	int cont = 0;
	uint16_t *modes;
	int fd;

	LRMI_init();

	if ((vbe_info = LRMI_alloc_real(512)) == NULL) {
		fprintf(stderr, "Cannot allocate buffer from real memory pool.\n");
		exit(EXIT_FAILURE);
	}
	if ((vbe_modeinfo = LRMI_alloc_real(256)) == NULL) {
		fprintf(stderr, "Cannot allocate buffer from real memory pool.\n");
		exit(EXIT_FAILURE);
	}

	ioperm(0, 1024, 1);
   	iopl(3);

	memset(&regs,0,sizeof(struct LRMI_regs));
	regs.eax = 0x4f00;
	regs.es = ((unsigned long)vbe_info)>>4;
	regs.edi = ((unsigned long)vbe_info)&0xf;
	LRMI_int(0x10, &regs);

	if (regs.flags & 1) {
		fprintf(stderr, "Failed!\n");
		exit(EXIT_FAILURE);
	}

	printf("BUF: %p\n", vbe_info);
	printf("SIG: %c%c%c%c\n",
		vbe_info->VESA_signature[0],
		vbe_info->VESA_signature[1],
		vbe_info->VESA_signature[2],
		vbe_info->VESA_signature[3]);
	printf("Version: %4.4x\n", vbe_info->VESA_version);
	printf("%s\n", (char*)phys_to_virt(vbe_info->OEM_string));
	printf("capabilities: %8.8x\n", vbe_info->capabilities_flags);
	printf("Total memory: %dK\n", vbe_info->total_memory * 64);
	printf("OEM Version %4.4x\n", vbe_info->oem_version);
	printf("Vendor Name: %s\n", (char*)phys_to_virt(vbe_info->vendor_name));
	printf("Product Name: %s\n", (char*)phys_to_virt(vbe_info->product_name));
	printf("Revision: %s\n", (char*)phys_to_virt(vbe_info->product_revision));

	modes = (uint16_t*)phys_to_virt(vbe_info->video_mode);
	while (*modes != 0xffff) {
		if (getSVGAmode(*modes, vbe_modeinfo)) {
			printf("Mode: %4.4x: %d x %d x %d %s\n", *modes, vbe_modeinfo->width, vbe_modeinfo->height, vbe_modeinfo->bits_per_pixel,
				vbe_modeinfo->mode_attr & 16 ? "Graphics" : "Text");
		}
		modes++;
	}
	modes++;
	
	LRMI_free_real(vbe_modeinfo);
	LRMI_free_real(vbe_info);

	exit(EXIT_SUCCESS);
}
