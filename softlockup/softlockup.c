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
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>

void setsched(void)
{
	struct sched_param mysched;
	mysched.sched_priority = 99;
  
	if (sched_setscheduler(0, SCHED_FIFO, &mysched) == -1) {
    		fprintf(stderr,"Cannot set sched()\n");
    		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{	
	double d = 0.0001;

	if (getuid() != 0) {
		fprintf(stderr,"%s needs to be run as root\n",argv[0]);
    		exit(EXIT_FAILURE);

	}
	setsched();

	while (d > 0.0000001) {
		d+=0.000002;
	}

    	exit(EXIT_SUCCESS);
}
