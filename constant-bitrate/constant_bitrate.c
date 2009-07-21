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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void syntax(char *name)
{
	fprintf(stderr,"Syntax: %s fromfilename tofilename bitrate packetsize\n",name);
	fprintf(stderr,"   filenames may be - to indicate stdin or stdout\n");
}

int main(int argc, char **argv)
{
	char run = ' ';
	char *buffer;

	int ps ;
	int delay ;
	int br ;
	int fdin,fdout ;
	int counter = 0;
	int inbufsize ;
	unsigned long totalbits = 0;

	float secs1,secs2;
	float currentbitrate ;

	struct timeval tv1,tv2;	
	struct timezone tz;

	if (argc < 5) {	
		syntax(argv[0]);
		exit(EXIT_FAILURE);
	}
	br = atoi(argv[3]);
	if (br < 1) {
		fprintf(stderr, "Bitrate too low!\n");
		exit(EXIT_FAILURE);
	}

	ps = atoi(argv[4]);
	if (ps < 1) {
		fprintf(stderr, "Packet size too low!\n");
		exit(EXIT_FAILURE);
	}

	if ((buffer = malloc(ps)) == NULL) {
		fprintf(stderr,"Cannot malloc buffer of %d bytes\n",ps);
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1],"-") == 0)
		fdin = fileno(stdin);
	else {
		if ((fdin = open(argv[1],O_RDONLY)) == -1) {
			fprintf(stderr,"Cannot open input file %s\n",argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	
	if (strcmp(argv[2],"-") == 0)
		fdout = fileno(stdout);
	else {
		if ((fdout = open(argv[2],O_RDWR | O_CREAT,  S_IWUSR | S_IRUSR)) == -1) {
			fprintf(stderr,"Cannot open file %s\n",argv[2]);
			exit(EXIT_FAILURE);
		}
	}
	fprintf(stderr,"Input %s, Ouput %s\n", fdin == 0 ? "stdin" : argv[1], fdout == 1 ? "stdout" :  argv[2]);
	fprintf(stderr,"Bitrate: %d bits / sec\n",br);

	gettimeofday(&tv1,&tz);	
	secs1 = ((float)tv1.tv_usec / 1000000.0);
	{
		float d = (float) ps * 8.0 * 1000000.0 ;
		delay = (int) (d / (float)br);
	}
	inbufsize = 0;
	for (;;) {	
		while (inbufsize < ps) {
			int sz = ps - inbufsize;
			int n = read(fdin,buffer,sz);
			if (n < 0) {
				fprintf(stderr,"Read: EOF\n");
				exit(EXIT_FAILURE);
			}
			inbufsize += n ;
		}
		if (write(fdout,buffer,ps) < 0) {
			fprintf(stderr,"Write failure\n");
			exit(EXIT_FAILURE);
		}
		inbufsize = 0 ;

		if (delay > 0) 
			usleep(delay);	

		totalbits += (8*ps);
		gettimeofday(&tv2,&tz);
		secs2 = (float)(tv2.tv_sec-tv1.tv_sec) + ((float)tv2.tv_usec / 1000000.0);
		currentbitrate = ((float)totalbits) / (secs2 - secs1);

		if (currentbitrate > (float)br) {
			run = 'O' ;
			delay += (delay >> 2) + 10000;
			if (delay < 0) {		
				delay = 0;
			}
		}
		else {
			run = 'U' ;
			delay -= ((delay >> 2) + 10000);
			if (delay < 0) {
				delay = 0 ;
			}
		}
		counter+=delay;
		if (counter>500000) {
			fprintf(stderr,"Bitrate %10.2f %c %10ld %12.5f PSize: %10d Delay: %10d\r",currentbitrate,run,totalbits,secs2-secs1,ps,delay);
			fflush(stderr); 
			counter=0;
		}
	}
}
