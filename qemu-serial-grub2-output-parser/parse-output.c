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


/*
 *  Cleans up ANSI VT escape codes from stream, for debugging QEMU EFI BIOS output
 */
#include <stdio.h>
#include <stdlib.h>

#define ESC	27

int parse_escape_codes(int ch)
{	
	while ((ch >= '0') && (ch <= '9')) {
		if ((ch = getc(stdin)) == EOF)
			return -1;
	}
	if (ch == ';') {
		do {
			if ((ch = getc(stdin)) == EOF)
				return -1;
		} while ((ch >= '0') && (ch <= '9'));
	}
	return 0;
}

int parse_escape_bracket(int ch)
{
	if (ch == '[') {
		if ((ch = getc(stdin)) == EOF)
			return -1;
		return parse_escape_codes(ch);
	}
	else {
		putc(ESC, stdout);
		putc(ch, stdout);
		return 0;
	}
}

int parse_escape(int ch)
{
	if (ch == ESC) {
		if ((ch = getc(stdin)) == EOF)
			return -1;
		return parse_escape_bracket(ch);
		
	}
	else {
		putc(ch, stdout);
		return 0;
	}
}

int main(int argc, char **argv)
{
	int ch;
	
	while ((ch = getc(stdin)) != EOF) {
		parse_escape(ch);
	}
	return 0;
}

