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

	vuint32 *gpc = gpio + 0x20 / 4;
	gpc[CON] = 0x55550000;
	gpc[DATA] = 0x00000000;

	vuint32 *gpd = gpio + 0x30 / 4;
	gpd[CON] = 0x55550000;
	gpd[DATA] = 0;			//reset
	gpd[DATA] = 5 << 8;		//start counting (~RESET and START_COUNTING set to 1)
	
	vuint32 *gpg = gpio + 0x60 / 4;
	gpg[CON] = 0; // EINT16 EINT17 (as input)

//	printf("Push either button to start...\n");

//	while (gpg[DATA] & 1 << 8 && gpg[DATA] & 1 << 9)
//		; //wait until a button is pushed

//	printf("Push both buttons to stop...\n");

//	while (1) {
//		gpc[DATA] = ((rand() & 0xFF) << 8) | ((rand() & 0xFF) << 10);
//		usleep(500000);
//		if (0 == (gpg[DATA] & (3 << 8))) {
//			break;
//		}
//	}
	
	int counter = 0;
	while (1) {
		gpc[DATA] = ((rand() & 0xFF) << 8);
	
		printf("%d buttons: %x\n", counter, gpc[DATA] & 0xF);
		
//		gpd[DATA] = 1 << 8;		//reset
//		gpd[DATA] = 3 << 10;	//start counting
		sleep(1);
		
		gpd[DATA] |= (1<<9);
		gpd[DATA] &= ~(1<<9);
		
		int i;
		for (i=0; i<8; i++) {
			gpd[DATA] &= ~(0xF << 12);
			gpd[DATA] |= i << 12;
			printf("counter-%d: %x\n", i, gpd[DATA] & 0xFF);
		}
		

		
	}
	return 0;
}
