#include <unistd.h>
#include <stdlib.h>
#include <sys/io.h>

int main(int argc, char **argv)
{
	ioperm(0x42, 4, 1);
        ioperm(0x61, 4, 1);
        ioperm(0x80, 1, 1);

	beep_440hz();
	sleep(5);
	beep_off();

	exit(EXIT_SUCCESS);
}
