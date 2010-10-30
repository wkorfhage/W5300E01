#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

/* GPIO */
#define CON		0
#define DATA	1
#define UP		2
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


void setMac(uint8 which, volatile unsigned int *gpio, unsigned short *addr) {
	iwrite(which, gpio, 0x08, addr[0]);
	iwrite(which, gpio, 0x0a, addr[1]);
	iwrite(which, gpio, 0x0c, addr[2]);
}

void getMac(uint8 which, volatile unsigned int *gpio, unsigned short *addr) {
	addr[0] = iread(which, gpio, 0x08);
	addr[1] = iread(which, gpio, 0x0a);
	addr[2] = iread(which, gpio, 0x0c);
}

void setGW(uint8 which, volatile unsigned int *gpio, unsigned short *addr) {
	iwrite(which, gpio, 0x10, addr[0]);
	iwrite(which, gpio, 0x12, addr[1]);
}

void setSUBNET(uint8 which, volatile unsigned int *gpio, unsigned short *addr) {
	iwrite(which, gpio, 0x14, addr[0]);
	iwrite(which, gpio, 0x16, addr[1]);
}

void setIP(uint8 which, volatile unsigned int *gpio, unsigned short *addr) {
	iwrite(which, gpio, 0x18, addr[0]);
	iwrite(which, gpio, 0x1A, addr[1]);
}

int main(int argc, char* argv[]) {
	long i;
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

	vuint32 *gpd = gpio + GPD_OFFSET;

	printf("GPD is : %08X\n", gpd);
	printf("GPDCON: %08X\n", gpd[CON]);
	printf("GPDDATA: %08X\n", gpd[DATA]);

	// hardware reset
	gpd[CON] = 0x55555555;
	gpd[DATA] = ~(1 << 13);
	gpd[DATA] = 0xFFFF;

	printf("Hardware reset: %04X\n", dread16(0, gpd, 0));
	dwrite16(0, gpd, 0, 0x3801);
	printf("MR after change: %04X\n", dread16(0, gpd, 0));
	printf("ID: %04X\n", iread(0, gpd, 0xfe));

	printf("Hardware reset: %04X\n", dread16(1, gpd, 0));
	dwrite16(1, gpd, 0, 0x3801);
	printf("MR after change: %04X\n", dread16(1, gpd, 0));
	printf("ID: %04X\n", iread(1, gpd, 0xfe));

	unsigned short mac0[3] = { 0x0008, 0xDCA0, 0x0001 };
	unsigned short mac1[3] = { 0x0008, 0xDCA0, 0x0002 };

	setMac(0, gpd, mac0);
	setMac(1, gpd, mac1);

	unsigned short gateway[2] = { 0xC0A8, 0x0101 };
	setGW(0, gpd, gateway);

	unsigned short subnet[2] = { 0xFFFF, 0xFF00 };
	setSUBNET(0, gpd, subnet);

	unsigned short ip0[2] = { 0xC0A8, 0x0105 };
	unsigned short ip1[2] = { 0xC0A8, 0x0106 };

	setIP(0, gpd, ip0);
	setIP(1, gpd, ip1);

	//	unsigned short tx[4] = { 64, 0x0000, 0x0000, 0x0000 };
	//	for (i = 0; i < 4; i++) {
	//		iwrite(gpd, 0x20 + i * 2, tx[i]);
	//	}
	//
	//	unsigned short rx[4] = { 64, 0x0000, 0x0000, 0x0000 };
	//	for (i = 0; i < 4; i++) {
	//		iwrite(gpd, 0x28 + i * 2, rx[i]);
	//	}

	//		 Memory Type Register
	//	iwrite(0, gpd, 0x30, 0x00FF);

	for (i = 0; i < 0x40; i += 2) {
		printf("read[%02X]: %04X\n", i, iread(0, gpd, i));
	}
	for (i = 0; i < 0x40; i += 2) {
		printf("read[%02X]: %04X\n", i, iread(1, gpd, i));
	}
}

