#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ntkn_w5300e01.h"
#include "rtc.h"

static vuint32 *rtc;
static rtc_fd;

int init_rtc() {
	// Open memory
	int rtc_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (rtc_fd < 0) {
		printf("cannot open /dev/mem for RTC");
		exit(-1);
	}
	
	rtc = (vuint32 *) mmap((void *) 0, (size_t) 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, rtc_fd, 0x57000000);
	printf ("mmap returned %X\n", (unsigned int) rtc);
	if (rtc == MAP_FAILED) {
		perror("GPIO Memory mapping failed for RTC");
		exit(-1);
	}
	
	rtc += 0x40 / 4;
	
	rtc[0] = 1; //enable

}

int read_rtc(char *buf, int length) {
	int month, date, year;
	int hh, mm, ss;
	int weekday;
	
	date = rtc[0x3c / 4];
	month = rtc[0x44 / 4];
	year = rtc[0x48 / 4];
	weekday = rtc[0x40 / 4];
	ss = rtc[0x30 / 4];
	mm = rtc[0x34 / 4];
	hh = rtc[0x38 / 4];
	
	printf("%x-%x-%x (%x) %x:%x:%x\n", month, date, year, weekday, hh, mm, ss);
	
	sprintf(buf, "%x-%x-%x (%x) %x:%x:%x\n", month, date, year, weekday, hh, mm, ss);
	
	return 0;
}

int set_rtc(const char* buf, int length) {

	int month, date, year;
	int hh, mm, ss;
	int weekday;
	
	sscanf(buf, "%d-%d-%d %d:%d:%d %d", &month, &date, &year, &hh, &mm, &ss, &weekday);
	rtc[0x3c / 4] = date / 10 << 4 | date % 10;
	rtc[0x44 / 4] = month / 10 << 4 | month % 10;
	rtc[0x48 / 4] = (year % 100 / 10) << 4 | year % 10;
	
	rtc[0x30 / 4] = ss / 10 << 4 | ss % 10;
	rtc[0x34 / 4] = mm / 10 << 4 | mm % 10;
	rtc[0x38 / 4] = hh / 10 << 4 | hh % 10;
	
	rtc[0x40 / 4] = weekday;
	
}

int close_rtc() {
	close(rtc_fd);
	return 0;
}
