/*
 *  led.c
 *  
 *
 *  Created by Liang Qu on 12/3/10.
 *  Copyright 2010 NTKN. All rights reserved.
 *
 *  W5300E01 LED through GPIO
 *  Push either button to start random led light, push both to stop.
 *
 */

#include <stdio.h> /* for printf */
#include <stdlib.h> /* for exit */
#include <unistd.h> /* for getopt */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <fcntl.h>

#define CON		0
#define DATA	1
#define UP		2
#define GPC_OFFSET	0x20/4

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef volatile uint8 vuint8;
typedef volatile uint16 vuint16;
typedef volatile uint32 vuint32;

int main(int argc, char* argv[]) {

	int memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0) {
		perror("/dev/mem open failed");
		exit(-1);
	}

	vuint32 *gpio = mmap((void *) 0, (size_t) 0x400, PROT_READ | PROT_WRITE,
			MAP_SHARED, memfd, 0x56000000);
	if (gpio == MAP_FAILED) {
		perror("Memory mapping failed");
		exit(-1);
	}

	vuint32 *gpd = gpio + 0x30 / 4;
	gpd[CON] = 0x55550000;

	char buf[128];
	while (1) {
		printf("<enter> to display gpd[DATA], <pin> <0/1> to set\n");
		fgets(buf, 120, stdin);
		if (strcmp(buf, "\n") == 0) {
			printf("gpd[DATA] = %x\n", gpd[DATA]);
		} else {
			int pin, value;
			sscanf(buf, "%d %d", &pin, &value);
			printf("%d %d\n", pin, value);
			if (value) {
				gpd[DATA] |= 1 << pin;
			} else {
				gpd[DATA] &= ~(1 << pin);
			}
		}
	}

	gpd[DATA] = 0;			//reset
	gpd[DATA] = 5 << 8;		//start counting (~RESET and START_COUNTING set to 1)
	
	int t; 
	if (argc == 1) {
		t = 1;
	} else {
		t = atoi(argv[1]);
	}
	
	sleep(t);
	
	gpd[DATA] = 1 << 8;		//stop timer
	gpd[DATA] |= (1<<9);	//clock data into registers
	gpd[DATA] &= ~(1<<9);	//high, then low
	
	int i;
	for (i=0; i<8; i++) {
		gpd[DATA] &= ~(0xF << 12);
		gpd[DATA] |= i << 12;
		printf("counter-%d: %x\n", i, gpd[DATA]);
	}
	
	return 0;
	
}
