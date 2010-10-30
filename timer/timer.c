/*
 * send3.c
 *
 *  Created on: Oct 29, 2010
 *      Author: lqu
 */

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <unistd.h>    /* for getopt */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <fcntl.h>

/* GPIO */
#define CON		0
#define DATA	1
#define UP		2
#define GPC_OFFSET	0x20/4
#define GPD_OFFSET	0x30/4

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef volatile uint8 vuint8;
typedef volatile uint16 vuint16;
typedef volatile uint32 vuint32;

/*
 * Write 1 byte to W5300, directly
 */
void dwrite8(uint8 which, vuint32 *gpio, uint8 offset, uint8 content) {

	uint8 tmp = 0;
	int i;
	for (i = 0; i < 8; i++) {
		tmp |= (content >> i & 1) << (7 - i);
	}
	gpio[CON] = 0x55555555; //GPIO output
	if (which == 1) {
		gpio[DATA] = 0xE800;
	} else if (which == 0) {
		gpio[DATA] = 0xE000;
	} else {
		printf("WRONG MODULE NUMBER\n");
		abort();
	}

	gpio[DATA] |= offset << 8; //set A0, A1, A2

	//	gpio[DATA] |= tmp;
	gpio[DATA] |= content;

	gpio[DATA] &= ~(1 << 14); //set WR to low
	gpio[DATA] |= 1 << 14; //set WR to high
}

/* 
 * Read 1 byte from W5300, directly
 */
uint8 dread8(uint8 which, vuint32 *gpio, uint8 offset) {
	uint8 data;
	gpio[CON] = 0x55550000; // GPIO input
	if (which == 1) {
		gpio[DATA] = 0xE800;	// the sender, 1
	} else if (which == 0) {
		gpio[DATA] = 0xE000;	// the receiver, 0
	} else {
		printf("WRONG W5300 NUMBER\n");
		abort();
	}
	gpio[DATA] |= offset << 8;

	gpio[DATA] &= ~(1 << 15);
	data = gpio[DATA] & 0x000000FF;
	gpio[DATA] |= 1 << 15;

	uint8 tmp = 0;
	int i;
	for (i = 0; i < 8; i++) {
		tmp |= (data >> i & 1) << (7 - i);
	}
	//	return tmp;
	return data;
}

/* 
 * Write 2 bytes to W5300, directly
 */
void dwrite16(uint8 which, vuint32 *gpio, uint8 offset, uint16 content) {
	// offset must be even
	// write bit 15~8 to offset 0
	dwrite8(which, gpio, offset, (content & 0xFF00) >> 8);
	// write bit 7~0 to offset 1
	dwrite8(which, gpio, offset + 1, (content & 0xFF));
}

/*
 * Read 2 bytes from W5300, directly
 */
uint16 dread16(uint8 which, vuint32 *gpio, uint8 offset) {
	unsigned char high, low;
	// offset must be even
	// read bit 15~8 first
	high = dread8(which, gpio, offset);
	low = dread8(which, gpio, offset + 1);

	uint16 result = (high << 8) | low;
	//	printf("read: %04X\n", result);
	return result;

}

/*
 * Write 16-bit to W5300 indirectly
 */
void iwrite(uint8 which, vuint32 *gpio, uint16 offset, uint16 content) {
	dwrite16(which, gpio, 2, offset);
	dwrite16(which, gpio, 4, content);
}

/*
 * Read 16-bit to W5300 indirectly
 */
uint16 iread(uint8 which, vuint32 *gpio, uint16 offset) {
	dwrite16(which, gpio, 2, offset);

	return dread16(which, gpio, 4);
}

void usage(char *s) {
	printf("Usage:\n");
	printf("%s <-h> <-d delay> <-f filename> <-m message> N\n", s);
	printf("-h	help\n");
	printf("-d	delay in seconds between runs\n");
	printf("-f	output to file\n");
	printf("-m	write a message to the first line of the file\n");
	printf("N repeat N times\n");
}

int main(int argc, char **argv) {
	int delay = 5;
	const char* filename = NULL;
	const char* message = NULL;
	int repeat = 1;
	int c;
	while ((c = getopt(argc, argv, "hd:f:m:")) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			abort();
		case 'd':
			delay = atoi(optarg);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'm':
			message = optarg;
			break;
		default:
			usage(argv[0]);
			abort();
		}
	}

	if (optind == argc - 1) {
		repeat = atoi(argv[argc - 1]);
	}

	int memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd < 0) {
		perror("/dev/mem open failed");
		exit(-1);
	}

	volatile unsigned int *gpio = mmap((void *) 0, (size_t) 0x400, PROT_READ
			| PROT_WRITE, MAP_SHARED, memfd, 0x56000000);
	if (gpio == MAP_FAILED) {
		perror("Memory mapping failed");
		exit(-1);
	}

	vuint32 *gpc = gpio + GPC_OFFSET;
	vuint32 *gpd = gpio + GPD_OFFSET;

	uint16 id = iread(0, gpd, 0xfe);
	if (id != 0x5300) {
		printf("ID register value is wrong: %04X, need to reset\n", id);
		abort();
	}
	id = iread(1, gpd, 0xfe);
	if (id != 0x5300) {
		printf("ID register value is wrong: %04X, need to reset\n", id);
		abort();
	}

	/*
	 * Open output file
	 */
	FILE *filep;
	if (filename != NULL) {
		char outstr[200];
		time_t t;
		struct tm *tmp;
		t = time(NULL);
		tmp = localtime(&t);
		if (tmp == NULL) {
			perror("localtime");
			exit(EXIT_FAILURE);
		}
		if (strftime(outstr, sizeof(outstr), "%m%d%C%y%H%M%S", tmp) == 0) {
			fprintf(stderr, "strftime returned 0");
			exit(EXIT_FAILURE);
		}

		char name[128];
		strcpy(name, filename);
		int len = strlen(filename);
		sprintf(name + len, "%s", "_");
		sprintf(name + len + 1, "%s", outstr);
		filep = fopen(name, "w");

		if (message != NULL) {
			fprintf(filep, "%s\n", message);
		}

		printf("=====================\n");
		printf("delay: %d second\n", delay);
		printf("file: %s\n", name);
		printf("message: \"%s\"\n", message);
		printf("repeat: %d times\n", repeat);
		printf("=====================\n");

	}

	/*
	 * setup sending socket, SOCKET_5
	 */
	iwrite(1, gpd, 0x300, 0x2); //MR: UDP mode
	iwrite(1, gpd, 0x30A, 5000); //source port
	iwrite(1, gpd, 0x302, 0x1); //Command: open
	while (0xFF & iread(1, gpd, 0x302))
		;

	while ((0xFF & iread(1, gpd, 0x308)) != 0x22)
		//wait for UDP status (0x22)
		;

	iwrite(1, gpd, 0x312, 5000); //dest port

	iwrite(1, gpd, 0x314, 192 << 8 | 168); //write dest IP
	iwrite(1, gpd, 0x316, 1 << 8 | 5);

	long count = 0;
	int i;
	for (i = 0; i < 8 * 1024 / 2; i++) {
		iwrite(1, gpd, 0x32E, count++); //write packet content
	}

	iwrite(1, gpd, 0x320, 0);
	iwrite(1, gpd, 0x322, 2); //write packet size

	int loop;
	for (loop = 0; loop < repeat; loop++) {

		/*
		 * setup receiving socket, SOCKET_1, port 0x1388/5000
		 */
		iwrite(0, gpd, 0x240, 0x2); //MR: UDP mode
		iwrite(0, gpd, 0x24A, 5000); //source port (0x1388)

		/*
		 * set up BRDYR_3, thus start the timer
		 */
		iwrite(0, gpd, 0x6E, 0x0001); //buffer depth, set to 1 byte
		iwrite(0, gpd, 0x6C, 0x0081); //enable, RX, low, socket 1


		iwrite(0, gpd, 0x242, 0x1); //Command: open

		iwrite(1, gpd, 0x302, 0x20); //SEND command
		/*
		 * start timer
		 */
		gpc[CON] = 0x00005555;
		gpc[DATA] = 0; //reset
		gpc[DATA] = 0x3 << 3; //start


		while (!iread(0, gpd, 0x26A))
			//wait for packets
			;


		//timer has already stopped by here
		gpc[DATA] &= ~(1 << 4); //turn off relay

		// clock data into register
		gpc[DATA] |= 1 << 5; //bring high
		gpc[DATA] &= ~(1 << 5); //then low

		// counter 000
		gpc[DATA] &= ~7;
		gpc[DATA] |= 0;
		uint8 counter0 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 001
		gpc[DATA] &= ~7;
		gpc[DATA] |= 1;
		uint8 counter1 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 010
		gpc[DATA] &= ~7;
		gpc[DATA] |= 2;
		uint8 counter2 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 011
		gpc[DATA] &= ~7;
		gpc[DATA] |= 3;
		uint8 counter3 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 100
		gpc[DATA] &= ~7;
		gpc[DATA] |= 4;
		uint8 counter4 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 101
		gpc[DATA] &= ~7;
		gpc[DATA] |= 5;
		uint8 counter5 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 110
		gpc[DATA] &= ~7;
		gpc[DATA] |= 6;
		uint8 counter6 = (gpc[DATA] & 0xFF00) >> 8;

		// counter 111
		gpc[DATA] &= ~7;
		gpc[DATA] |= 7;
		uint8 counter7 = (gpc[DATA] & 0xFF00) >> 8;
		//time now
		char outstr[200];
		time_t t;
		struct tm *tmp;
		t = time(NULL);
		tmp = localtime(&t);
		if (tmp == NULL) {
			perror("localtime");
			exit(EXIT_FAILURE);
		}
		if (strftime(outstr, sizeof(outstr), "%m_%d_%C%y_%H_%M_%S", tmp) == 0) {
			fprintf(stderr, "strftime returned 0");
			exit(EXIT_FAILURE);
		}

		uint32 timer1 = ((counter3 << 24) | (counter2 << 16) | (counter1 << 8)
				| counter0) / 4;
		uint32 timer2 = ((counter7 << 24) | (counter6 << 16) | (counter5 << 8)
				| counter4) / 4;
		printf("%d\t%d\tmicroseconds\n", timer1, timer2);

		if (filename != NULL) {
			fprintf(filep, "%s\t%d\t%d\n", outstr, timer1, timer2);
			fflush(filep);
		}

		iwrite(0, gpd, 0x242, 0x40); //command: RECV
		while (0xFF & iread(0, gpd, 0x242))
			;
		iwrite(0, gpd, 0x242, 0x10); //command: CLOSE
		while (0xFF & iread(0, gpd, 0x242))
			;
		while (0xFF & iread(0, gpd, 0x248))
			//wait for the status to be closed
			;

		sleep(delay);
	}

	if (filename != NULL) {
		fclose(filep);
	}

	return 0;
}

