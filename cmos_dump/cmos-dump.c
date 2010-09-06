#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>

inline unsigned char cmos_read(int offset)
{
	unsigned char value;

	ioperm(0x70, 0x2, 1);
	ioperm(0x80, 0x1, 1);

	outb(offset, 0x70);
        outb(0, 0x80);  /* Small delay */
        value = inb(0x71);

	ioperm(0x80, 0x1, 0);
	ioperm(0x70, 0x2, 0);

	return value;
}

int main(int argc, char **argv)
{
	int i;
	unsigned long tmp;

	static char *cmos_shutdown_status[] = {
		"Power on or soft reset",
		"Memory size pass",
		"Memory test pass",
		"Memory test fail",

		"INT 19h reboot",		
		"Flush keyboard and jmp via 40h:67h",
		"Protected mode tests pass",
		"Protected mode tests fail",

		"Used by POST during protected-mode RAM test",
		"Int 15h (block move)",
		"Jmp via 40h:67h",
		"Used by 80386",
	};

	static char *floppy_disk[] = {
		"None",
		"360KB 5.25\" Drive",
		"1.2MB 5.25\" Drive",
		"720KB 3.5\" Drive",
		"1.44MB 3.5\" Drive",
		"2.88MB 3.5\" Drive",
		"Unknown",
		"Unknown"
	};

	static char *hard_disk[] = {
		"None",
		"Type 1",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Unknown",
		"Type 14",
		"Type 16-47"
	};

	static char *primary_display[] = {
		"BIOS selected",
		"CGA 40 column",
		"CGA 80 column",
		"Monochrome"
	};


	static char *divider[8] = {
		"unknown",
		"unknown",
		"32.768 KHz (default)",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
		"unknown",
	};

	static char *rate_selection[16] = { 
		"none",
		"unknown",
		"unknown",
		"122 microseconds (minimum)",

		"unknown",
		"unknown",
		"976.562 microseconds (default)",
		"unknown",
		
		"unknown",
		"unknown",
		"unknown",
		"unknown",

		"unknown",
		"unknown",
		"unknown",
		"500 milliseconds"
	};

	unsigned char data[0x80];

	if (geteuid() != 0) {
		fprintf(stderr, "Must be root to run %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (i=0;i<sizeof(data); i++)
		data[i] = cmos_read(i);

	printf("CMOS Memory Dump:\n");
	for (i=0;i<sizeof(data); i+= 8) {
		printf("  %2.2x: %2.2x %2.2x %2.2x %2.2x  %2.2x %2.2x %2.2x %2.2x\n",
			i, 
			data[i], data[i+1], data[i+2], data[i+3],
			data[i+4], data[i+5], data[i+6], data[i+7]);
	}
	printf("\n");

	printf("RTC Current Time: (CMOS 0x00..0x09)\n");
	printf("  RTC seconds:            %2.2x\n", data[0]);
	printf("  RTC minutes:            %2.2x\n", data[2]);
	printf("  RTC hours:              %2.2x\n", data[4]);
	printf("  RTC day of week:        %2.2x\n", data[6]);
	printf("  RTC date day:           %2.2x\n", data[7]);
	printf("  RTC date month:         %2.2x\n", data[8]);
	printf("  RTC date year:          %2.2x\n", data[9]);
	printf("\n");

	printf("RTC Alarm:\n");
	printf("  RTC seconds:            %2.2x\n", data[1]);
	printf("  RTC minutes:            %2.2x\n", data[3]);
	printf("  RTC hours:              %2.2x\n", data[5]);
	printf("\n");

	printf("Status Register A: (CMOS 0x0a): 0x%2.2x\n", data[10]);
	printf("  Rate freq:              %1.1x (%s)\n", data[10] & 0xf, rate_selection[data[10] & 0xf]);
	printf("  Timer freq divider:     %1.1x (%s)\n", (data[10] >> 4) & 0x7, divider[(data[10] >> 4) & 0x7]);
	printf("  Update in progress:     %1.1x\n", (data[10] >> 7) & 1);
	printf("\n");

	printf("Status Register B: (CMOS 0x0b): 0x%2.2x\n", data[11]);
	printf("  Daylight savings:       %1.1x (%s)\n", (data[11] >> 1) & 1, (data[11] >> 1) & 1 ? "Enabled" : "Disabled");
	printf("  24 Hour Clock:          %1.1x (%s)\n", (data[11] >> 2) & 1, (data[11] >> 1) & 1 ? "12 Hour" : "24 Hour");
	printf("  Square Wave:            %1.1x (%s)\n", (data[11] >> 3) & 1, (data[11] >> 2) & 1 ? "Enabled" : "Disabled");
	printf("  Update ended IRQ:       %1.1x (%s)\n", (data[11] >> 4) & 1, (data[11] >> 3) & 1 ? "Enabled" : "Disabled");
	printf("  Alarm IRQ:              %1.1x (%s)\n", (data[11] >> 5) & 1, (data[11] >> 5) & 1 ? "Enabled" : "Disabled");
	printf("  Periodic IRQ:           %1.1x (%s)\n", (data[11] >> 6) & 1, (data[11] >> 6) & 1 ? "Enabled" : "Disabled");
	printf("  Clock update cycle:     %1.1x (%s)\n", (data[11] >> 7) & 1, (data[11] >> 7) & 1 ? "Abort update in progress" : "Update normally");
	printf("\n");

	printf("Status Register C: (CMOS 0x0c): 0x%2.2x\n", data[12]);
	printf("  UF flag:                0x%1.1x\n", (data[12] >> 4) & 1);
	printf("  AF flag:                0x%1.1x\n", (data[12] >> 5) & 1);
	printf("  PF flag:                0x%1.1x\n", (data[12] >> 6) & 1);
	printf("  IRQF flag:              0x%1.1x\n", (data[12] >> 7) & 1);
	printf("\n");

	printf("Status Register D: (CMOS 0x0d): 0x%2.2x\n", data[13]);
	printf("  Valid CMOS RAM flag:    0x%1.1x (%s)\n", (data[13] >> 7) & 1, (data[13] >> 7) & 1 ? "Battery Good": "Battery Dead");
	printf("\n");

	printf("Diagnostic Status: (CMOS 0x0e): 0x%2.2x\n", data[14]);
	printf("  CMOS time status:       0x%1.1x (%s)\n", (data[14] >> 2) & 1, (data[14] >> 2) & 1 ? "Invalid": "Valid");
	printf("  Fixed disk init:        0x%1.1x (%s)\n", (data[14] >> 3) & 1, (data[14] >> 3) & 1 ? "Bad": "Good");
	printf("  Memory size check:      0x%1.1x (%s)\n", (data[14] >> 4) & 1, (data[14] >> 4) & 1 ? "Bad": "Good");
	printf("  Config info status:     0x%1.1x (%s)\n", (data[14] >> 5) & 1, (data[14] >> 5) & 1 ? "Invalid": "Valid");
	printf("  CMOS checksum status:   0x%1.1x (%s)\n", (data[14] >> 6) & 1, (data[14] >> 6) & 1 ? "Bad": "Good");
	printf("  CMOS power loss:        0x%1.1x (%s)\n", (data[14] >> 7) & 1, (data[14] >> 7) & 1 ? "Lost power": "Not lost power");
	printf("\n");
	printf("CMOS Shutdown Status: (CMOS 0x0f): 0x%2.2x (%s)\n\n", data[15],
			data[15] < 0xb ? cmos_shutdown_status[data[15]] : "Perform power-on reset");
	printf("Floppy Disk Type: (CMOS 0x10): 0x%2.2x\n", data[16]);
	printf("  Drive 0: %s\n", floppy_disk[((data[16] >> 4) & 0xf)]);
	printf("  Drive 1: %s\n", floppy_disk[((data[16] >> 0) & 0xf)]);
	printf("\n");

	printf("Hard Disk Type: (CMOS 0x12, Obsolete): 0x%2.2x\n", data[18]);
	printf("  Drive 0: %s\n", hard_disk[((data[18] >> 4) & 0xf)]);
	printf("  Drive 1: %s\n", hard_disk[((data[18] >> 0) & 0xf)]);
	printf("\n");

	printf("Installed H/W: (CMOS 0x14): 0x%2.2x\n", data[20]);
	printf("  Maths Coprocessor:      0x%1.1x (%s)\n", (data[20] >> 1) & 1, (data[20] >> 1) & 1 ? "Installed": "Not Installed");
	printf("  Keyboard:               0x%1.1x (%s)\n", (data[20] >> 2) & 1, (data[20] >> 2) & 1 ? "Installed": "Not Installed");
	printf("  Display Adaptor:        0x%1.1x (%s)\n", (data[20] >> 3) & 1, (data[20] >> 3) & 1 ? "Installed": "Not Installed");
	printf("  Primary Display:        0x%1.1x (%s)\n", (data[20] >> 4) & 3, primary_display[(data[20] >> 4) & 3]);
	printf("  Floppy Drives:          0x%2.2x (%d drives)\n", (data[20] >> 6) & 3, ((data[20] >> 6) & 3) + 1);
	printf("\n");

	tmp = ((data[22] << 8) | (data[21]));
	printf("Base Mem: (CMOS 0x16):\n");
	printf("  0x%2.2x%2.2x (%luK)\n", data[22], data[21], tmp);
	printf("\n");

	tmp = ((data[24] << 8) | (data[25]));
	printf("Extended Mem: (CMOS 0x18):\n");
	printf("  0x%2.2x%2.2x (%luK) %s\n", data[24], data[23], tmp, tmp > (16 * 1024) ? "[untrustworthy]" : "");
	printf("\n");

	printf("Hard Disk Extended Types (CMOS 0x19, 0x1a):\n");
	printf("  Hard Disk 0:            0x%2.2x\n", data[25]);
	printf("  Hard Disk 1:            0x%2.2x\n", data[26]);
	printf("\n");
	
	printf("CMOS Checksum:(CMOS 0x2e):0x%2.2x%2.2x\n", data[47], data[46]);
	printf("\n");

	printf("Extended Mem: (CMOS 0x30):0x%2.2x%2.2x\n", data[49], data[48]);
	printf("\n");

	printf("Century Date: (CMOS 0x32):%2.2x\n", data[50]);
	printf("\n");
	printf("POST Information Flag (CMOS 0x33):\n");
	printf("  POST cache test:        0x%1.1x %s\n", (data[51] >> 0) & 1, (data[51] >> 0) & 1 ? "Failed" : "Passed");
	printf("  BIOS size:              0x%1.1x %s\n", (data[51] >> 7) & 1, (data[51] >> 7) & 1 ? "128KB" : "64KB");
	printf("\n");

	exit(EXIT_SUCCESS);
}
