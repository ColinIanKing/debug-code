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


#define ENABLE		$0x03

#define PIT_CONTROL	$0x61
#define PIT_COUNTER	$0x42

/*
 *  Wait a small amount by writing to port $80
 */
#define TINY_WAIT		\
	movb	$0, %al;	\
	outb	%al, $0x80;	

/*
 *  Turn beep off
 */
.globl	beep_off
beep_off:
	inb     PIT_CONTROL, %al;       # Dummy read of System Control Port B
	TINY_WAIT
	movb	$0, %al;
	outb	%al, PIT_CONTROL;
	TINY_WAIT
	ret;

/*
 *  Play a tone specified by %al in Hz
 */
beep_on:
	movl	%eax, %edi		# eax carries the tone freq, 440HZ=A
	movl	$1193181, %ebx		# convert from HZ to PIT clock freq
	movl	%ebx, %eax	
	movl	$0, %edx
	divl	%edi
	movl	%eax, %ebx		# ebx saved for later

	movb	$0xb6, %al;
	outb	%al, $0x43; 		# CTR 2, Squarewave, load
	TINY_WAIT

	movb	%bl, %al;
	outb	%al, PIT_COUNTER	# LSB of counter
	TINY_WAIT

	shrw	$0x8, %bx
	movl	%ebx, %eax		# MSB of counter
	outb	%al, PIT_COUNTER

	inb     PIT_CONTROL, %al;       # Dummy read of System Control Port B
	TINY_WAIT

        movb    ENABLE, %al;        	# Enable timer 2 output speaker
        outb    %al, PIT_CONTROL;       
	TINY_WAIT
	ret

/*
 *  Play 440Hz tone, which is A above middle C
 */
.globl	beep_440hz			# A above middle C
beep_440hz:
	movl	$440, %eax
	call	beep_on
