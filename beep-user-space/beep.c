#include <unistd.h>
#include <sys/io.h> 
#include <stdlib.h>

static void beep(unsigned int hz)
{
        unsigned int enable;

        if (!hz) {
                enable = 0x00;          /* Turn off speaker */
        } else {
                unsigned short div = 1193181/hz;
                outb(0xb6, 0x43);       /* Ctr 2, squarewave, load, binary */	
		usleep(1);
                outb(div, 0x42);        /* LSB of counter */
		usleep(1);
                outb(div >> 8, 0x42);   /* MSB of counter */
		usleep(1);
                enable = 0x03;          /* Turn on speaker */
        }
        inb(0x61);              /* Dummy read of System Control Port B */
	usleep(1);
        outb(enable, 0x61);     /* Enable timer 2 output to speaker */
	usleep(1);
}

int main(int argc, char **argv)
{
	int freq;

	ioperm(0x42, 4, 1);
	ioperm(0x61, 4, 1);

	for (freq=440;freq<880; freq+=10) {
		beep(freq);
		usleep(100000);
	}
	beep(0);

	exit(EXIT_SUCCESS);
}
