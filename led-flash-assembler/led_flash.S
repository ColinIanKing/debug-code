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

#define I8042_DATA_REG          $0x60
#define I8042_STATUS_REG        $0x64

wait:	movl	$0x1000, %eax
wait1: 	subl    $1, %eax
	cmpl    $0, %eax
	jne     wait1
	ret

delay:
	movl    $0x07878787, %eax
delay1: subl    $1, %eax
	cmpl    $0, %eax
	jne     delay1
	ret

.globl flash_led

flash_led:
	inb	I8042_STATUS_REG, %al
	testb	$0x02, %al
	jne	flash_led
	movb	$0xed, %al
	outb	%al, I8042_DATA_REG
	call	wait
flash_led1:
	inb	I8042_STATUS_REG, %al
	testb	$0x02, %al
	jne	flash_led1
	call 	wait
	movb	$0x05, %al
	outb	%al, I8042_DATA_REG
	call	delay

	inb	I8042_STATUS_REG, %al
	testb	$0x02, %al
	jne	flash_led
	movb	$0xed, %al
	outb	%al, I8042_DATA_REG
	call	wait
flash_led2:
	inb	I8042_STATUS_REG, %al
	testb	$0x02, %al
	jne	flash_led2
	call 	wait
	movb	$0x00, %al
	outb	%al, I8042_DATA_REG
	call	delay
	call	delay
	ret
