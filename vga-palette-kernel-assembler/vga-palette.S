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


.globl setred

setred:
	movb	$0x00, %bl
setred1:
	movw	$0x3c8, %dx
	mov	%bl, %al
	outb	%al, %dx
	movw 	$0x3c9, %dx
	movb	$63, %al
	outb	%al, %dx
	movb	$00, %al
	outb	%al, %dx
	outb	%al, %dx
	addb	$1, %bl
	cmpb	$0x40, %bl
	jne	setred1
	ret

.globl setgreen
setgreen:
	movb	$0x00, %bl
setgreen1:
	movw	$0x3c8, %dx
	mov	%bl, %al
	outb	%al, %dx
	movw 	$0x3c9, %dx
	movb	$00, %al
	outb	%al, %dx
	movb	$63, %al
	outb	%al, %dx
	movb	$00, %al
	outb	%al, %dx
	addb	$1, %bl
	cmpb	$0x40, %bl
	jne	setgreen1
	ret

.globl setblue
setblue:
	movb	$0x00, %bl
setblue1:
	movw	$0x3c8, %dx
	mov	%bl, %al
	outb	%al, %dx
	movw 	$0x3c9, %dx
	movb	$00, %al
	outb	%al, %dx
	outb	%al, %dx
	movb	$63, %al
	outb	%al, %dx
	addb	$1, %bl
	cmpb	$0x40, %bl
	jne	setblue1
	ret

.globl	setreg
setreg:
	movb	%al, %cl
	movb	$0x00, %bl
setreg1:
	movw	$0x3c8, %dx
	mov	%bl, %al
	outb	%al, %dx
	movw 	$0x3c9, %dx
	movb	%cl, %al
	outb	%al, %dx
	outb	%al, %dx
	outb	%al, %dx
	addb	$1, %bl
	cmpb	$0x40, %bl
	jne	setreg1
	ret

delay:
	movl    $0x1e1e1e1e, %eax
loop1s:
	subl    $1, %eax
	cmpl    $0, %eax
	jne     loop1s
	ret

.globl	setblack
setblack:
	movb	$0,%al
	jmp	setreg

.globl	setwhite
setwhite:
	movb	$63,%al
	jmp	setreg

.globl flash
flash:
	call setblack
	call delay
	call setred
	call delay
	call setgreen
	call delay
	call setblue
	call delay
	call setwhite
	call delay
	

