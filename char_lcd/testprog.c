#include "charlib.h"
#include <stdio.h>

void print_usage()
{
	fprintf(stderr, "testprog \"string to print\"\n");
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		print_usage();
		exit(-1);
	}

	if (charinit()) 
	{
		perror("Init didn't work: ");
		exit(-1);
	}
	lcd_home_cursor();
	lcd_puts(argv[1]);

	return 0;
}
