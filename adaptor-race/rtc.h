#ifndef __NTKN_RTC__
#define __NTKN_RTC__

int init_rtc();
int read_rtc(char *buf, int length);
int set_rtc(const char* buf, int length);

#endif
