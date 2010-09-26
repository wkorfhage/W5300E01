#include <stdio.h>
#include "gpio.h"

#define FALSE 0
#define TRUE 1

#define LED_1_BIT 10
#define LED_2_BIT 11

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
	
	unsigned int *gpio_base = get_gpio_base();


	printf ("gpio_base is %X\n", (unsigned int) gpio_base);
	if (gpio_base == MAP_FAILED) {
		perror("Memory mapping failed");
		exit(-1);
	}
	
	printf("Setting pin direction\n");fflush(stdout);
	set_pin_direction(&gpio_base[GPGCON_OFFSET], led, FUNCTION_OUTPUT);
	printf("Setting pin data\n");fflush(stdout);
	// Note that LED goes on when the output goes low.
	set_pin_data(&gpio_base[GPGDAT_OFFSET], led, !on);
	fflush(stdout);

	gpio_close();
}
