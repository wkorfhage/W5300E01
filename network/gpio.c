#include <stdio.h>
#include <stdlib.h>

// For open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// end for open

// For close
#include <unistd.h>
// end for close

#include "gpio.h"

#define GPIOBASE_ADDRESS 0x56000000

int memfd;

/**
    Get a pointer to the base of the GPIO registers.

    @returns 0 on success, non-zero on failure. Errno is set if failed.
*/
unsigned int *get_gpio_base()
{
	// Open memory
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0) {
        	exit(-1);
	}
	gpio_base = (unsigned int *) mmap((void *) 0, (size_t) 0x100,
		PROT_READ | PROT_WRITE, MAP_SHARED, memfd, GPIOBASE_ADDRESS);
	printf ("mmap returned %X\n", (unsigned int) gpio_base);
	if (gpio_base == MAP_FAILED) {
		perror("Memory mapping failed");
		exit(-1);
	}
	return gpio_base;
}

/**
   Clean up resources before exiting. Don't use the memory after calling this.

   @returns 0 on success, non-zero on failure. Errno is set if failed.
*/
int gpio_close()
{
	return close(memfd);
}

/**
    Print the contents of a control register

    @passed register_contents - the 32 bit register contents
    @passed display_type - how to interpret the contents.

*/
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

/**
    Print the contents of a data register

    @passed register_contents - a 32-bit number, the lower 15 bits of which are the data.

*/
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
/**
    Set the function of an io pin

    @passed control_reg The address of the control register.
    @passed bit - which I/O bit this controls (0, 1, 2, .., 15). Note that this is not the
	bit offset in the control register. For instance, output bit 1 is actually controlled
	by bits 2 and 3 of the register. In this case, you pass in 1.
    @passed function What function to apply to the pins, as defined below. These are 2-bit
	numbers.
*/
void set_pin_direction(unsigned int *control_reg, unsigned int bit, unsigned int function)
{
	const unsigned char PIN_MASK = 3;
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
/**
    Set the data in a register.

    @passed data_reg A pointer to the control register address
    @passed bit Which bit to set (0-15)
    @passed bit_value 1 for on, 0 for off.
*/
void set_pin_data(unsigned int *data_reg, unsigned int bit, unsigned int bit_value)
{
	unsigned int shift_amount = bit;
	unsigned int mask = 1 << shift_amount;
	unsigned int new_value = bit_value << shift_amount;
	
	unsigned int current_register_contents = *data_reg;
	printf("Current data:\n");
	print_data_reg(current_register_contents);

	unsigned int new_register_contents = (current_register_contents & ~mask) | new_value;
	printf("New data:\n");
	print_data_reg(new_register_contents);

	*data_reg = new_register_contents;
}
