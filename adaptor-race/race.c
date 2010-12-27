#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ntkn_w5300e01.h"
#include "button.h"
#include "gpio.h"
#include "keyboardread.h"
#include "charlib.h"

void *blink(void* args) {
	vuint32 *led = args;
	while(1) {
		led[DATA] ^= 1<<10;
		usleep(500000);
	}
}

int main() {
	vuint32 *gpio = get_gpio_base();
	vuint32 *gpc = gpio + GPC_OFFSET;
	vuint32 *gpd = gpio + GPD_OFFSET;
	vuint32 *gpg = gpio + GPG_OFFSET;
	
	gpc[CON] = 0x55550000;
	gpd[CON] = 0x55550000;
	gpg[CON] = 0x55500000;
	
	//led1 on
	gpg[DATA] &= ~(1 << 10);
	//led2 off
	gpg[DATA] |= 1 << 11;
	
	charinit();
	
	lcd_puts("Press Button SW3");
	
	pthread_t blinking;
	if (pthread_create(&blinking, NULL, blink, gpg)) {
		printf("pthread_create() failed\n");
		abort();
	}
	
	gpd[DATA] = 0;		//reset timer
	
	while (gpg[DATA] & 1 << 8);
	
	gpd[DATA] |= 1 << 8;
	gpd[DATA] |= 1 << 10;
	
	
//	waitForButton(gpg+DATA, 8);
	while (gpg[DATA] & 1 << 8);
	
	int x;
	for (x = 0; x < 8; x++) {
		
		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= x << 12;
		gpd[DATA] |= 1 << 9;
		gpd[DATA] &= ~(1 << 9);
		
		printf("%x\n", gpd[DATA]);
	}
	
	
	//pthread_kill(blinking, 9);
	gpg[DATA] = 1 << 10;		//turn off led1
	
	char buf[64];
	lcd_puts("type the name:");
	lcd_gotoxy(0, 1);
	lcd_putch('>');
	
	readFromUSBKeyboard("./kb0", buf, 64);
	printf("%s\n", buf);
	
	gpg[DATA] &= ~(1 << 10);	//turn on led1
	
	//run race
	
	return 0;
}
