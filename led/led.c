#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
// For fopen
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// end for fopen

#define FALSE 0
#define TRUE 1

#define GPIOBASE_ADDRESS 0x56000000
#define GPGCON_OFFSET 0x60
#define GPGDAT_OFFSET 0x64 
#define GPGUP_OFFSET 0x68 
#define INPUT_PIN 0
#define OUTPUT_PIN 1
#define PIN_MASK 3
#define GPG10_MASK	0x00300000
#define GPG10_OUTPUT	0x00100000 // Sets bit to output
#define GPG11_MASK	0x00C00000
#define GPG11_OUTPUT	0x00400000 // Sets bit to input

#define LED_1_BIT 10
#define LED_2_BIT 11

unsigned int *GPIOBase;
unsigned int *GPGControlReg;
unsigned int *GPGDataReg;

void set_pin_direction(unsigned int *control_reg, unsigned int bit, unsigned int function);
void set_pin_data(unsigned int *data_reg, unsigned int bit, unsigned int on_off);

void print_usage()
{
	fprintf(stderr, "led 1|2 on|off\n");
}

int main(int argc, char *argv[])
{
	int led; // LED # in the register. LED 1 is GPG10, LED 2 is GPG11
	int on; // True if it should be on, false if off.

	if (argc < 3) {
		print_usage();
		exit(-1);
	}

	if (strcmp(argv[1], "1") == 0) led = LED_1_BIT;
	else if (strcmp(argv[1], "2") == 0) led = LED_2_BIT;
	else {
		fprintf(stderr, "The LED number must be 1 or 2, not \"%s\".\n", argv[1]);
		print_usage();
		exit(-1);
	}

	if (strcmp(argv[2], "on") == 0) on = TRUE;
	else if (strcmp(argv[2], "off") == 0) on = FALSE;
	else {
		fprintf(stderr, "Please specify on or off, not \"%s\".\n", argv[2]);
		print_usage();
		exit(-1);
	}

	printf("Turn LED %d %s\n", led, on?"off":"on");

	// Open memory
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
        	exit(-1);
	}
	GPIOBase = (unsigned int *) mmap((void *) 0, (size_t) 0x100,
		PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOBASE_ADDRESS);
	printf ("mmap returned %X\n", (unsigned int) GPIOBase);
	if (GPIOBase == MAP_FAILED) {
		perror("Memory mapping failed");
		exit(-1);
	}
	
	GPGControlReg = GPIOBase + GPGCON_OFFSET;
	GPGDataReg = GPIOBase + GPGDAT_OFFSET;

	printf("Setting pin direction\n");fflush(stdout);
	set_pin_direction(GPGControlReg, led, OUTPUT_PIN);
	printf("Setting pin data\n");fflush(stdout);
	set_pin_data(GPGDataReg, led, on);
	fflush(stdout);
}

#define RAW_DISPLAY 00
#define GPG_DISPLAY 01
void print_control_reg(int register_contents, int display_type)
{
	int i;
	printf("15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
	for (i = 15; i >= 0; i--)
	{
		int val = (register_contents >> i*2) & 0x3;
		switch (display_type) {

			case GPG_DISPLAY:
				switch (val) {
					case 0: printf(" I "); break;
					case 1: printf(" O "); break;
					case 2: printf("NT "); break;
					case 3: printf("OT "); break;
					default: printf("UN "); break;
				}
			break;

			default:
				switch (val) {
					case 0: printf("00 "); break;
					case 1: printf("01 "); break;
					case 2: printf("10 "); break;
					case 3: printf("11 "); break;
					default: printf("OV "); break;
				}
			break;
		}
	}
	printf("\n");
}

void print_data_reg(unsigned int register_contents)
{
	int i;
	printf("15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00\n");
	for (i = 15; i >= 0; i--)
	{
		int val = (register_contents >> i) & 0x1;
		printf(" %1d ", val);
	}
	printf("\n");
}


// bit = 0-15, function = INPUT_PIN, OUTPUT_PIN
void set_pin_direction(unsigned int *control_reg, unsigned int bit, unsigned int function)
{
	printf("Set pin %d to %d\n", bit, function);
	unsigned int shift_amount = bit * 2;
	unsigned int mask = PIN_MASK << shift_amount;
	unsigned int new_value = function << shift_amount;
	
	unsigned int current_register_contents = *control_reg;
	printf("Current control register: \n");
	print_control_reg(current_register_contents, GPG_DISPLAY);

	unsigned int new_register_contents = (current_register_contents & ~mask) | new_value;
	printf("New control register: \n");
	print_control_reg(new_register_contents, GPG_DISPLAY);

	*control_reg = new_register_contents;
	printf("Control register after writing: \n");
	print_control_reg(*control_reg, GPG_DISPLAY);
}

// bit = 0-15, on_off = 0 for off, anything else for on.
void set_pin_data(unsigned int *data_reg, unsigned int bit, unsigned int on_off)
{
	unsigned int shift_amount = bit;
	unsigned int mask = 1 << shift_amount;
	unsigned int new_value = on_off << shift_amount;
	
	unsigned int current_register_contents = *data_reg;
	printf("Current data:\n");
	print_data_reg(current_register_contents);

	unsigned int new_register_contents = (current_register_contents & ~mask) | new_value;
	printf("New data:\n");
	print_data_reg(new_register_contents);

	*data_reg = new_register_contents;
}
