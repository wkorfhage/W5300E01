#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ntkn_w5300e01.h"
#include "button.h"
#include "gpio.h"
#include "keyboardread.h"
#include "charlib.h"

int isBlinking;

void *blink(void* args) {
	vuint32 *led = args;
	while(isBlinking) {
		led[DATA] ^= 1<<10;
		usleep(500000);
	}
}

char bbb[100];

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
	
	isBlinking = 1;
	pthread_t blinking;
	if (pthread_create(&blinking, NULL, blink, (void *)gpg)) {
		printf("pthread_create() failed\n");
		abort();
	}
	
	gpd[DATA] = 0;
	gpd[DATA] |= 1 << 10;	//stop counting
	gpd[DATA] &= ~(1 << 8);	//reset
	
	printf("now reply off\n");
	fgets(bbb, 100, stdin);
	
	gpd[DATA] |= 1 << 9;	//clock into reg.
	gpd[DATA] &= ~(1 << 9);
	gpd[DATA] |= 1 << 9;	//clock into reg.
	gpd[DATA] &= ~(1 << 9);
	
	int i;
	for (i=0; i<8; i++) {
		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= i << 12;
		
		printf("after reset, %04x\n", gpd[DATA]);
	}
	

	gpd[DATA] |= 1 << 8;
	
	printf("press button to make timer ready\n");
	fgets(bbb, 100, stdin);
	gpd[DATA] &= ~(1 << 10);	//start counting
	
	printf("press button to start counting\n");
	fgets(bbb, 100, stdin);
	
	printf("manually counting...\n");
	
//	gpd[DATA= |= 1 << 15;
	
	//manually count
	int xxx = 30;
	while(xxx--) {
		gpd[DATA] |= 1 << 11;
		gpd[DATA] &= ~(1 << 11);
		gpd[DATA] |= 1 << 11;
		gpd[DATA] &= ~(1 << 11);
	}
//	waitForButton(gpg+DATA, 8);
	//while (gpg[DATA] & 1 << 8);
	
	gpd[DATA] |= 1 << 9;
	gpd[DATA] &= ~(1 << 9);
	
	for (i = 0; i < 8; i++) {
		printf("press to display");
		fgets(bbb, 100, stdin);
		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= i << 12;

		
		printf("%04x\n", gpd[DATA]);
	}
	
	isBlinking = 0;
	gpg[DATA] |= 1 << 10;		//turn off led1
	
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
