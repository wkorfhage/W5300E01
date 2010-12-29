#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kdev_t.h>
#include <sys/socket.h>
#include <netdb.h>

#include "ntkn_w5300e01.h"
#include "button.h"
#include "gpio.h"
#include "keyboard.h"
#include "charlib.h"
#include "leds_ctrl.h"
#include "adapter.h"
#include "rtc.h"

int i, j, n;
char buf[256];

Adapter adapters[4];
Adapter *adp[4];

struct  sockaddr_in sad; /* structure to hold an IP address     */
int     clientSocket;    /* socket descriptor                   */ 
struct  hostent  *ptrh;  /* pointer to a host table entry       */

char    *host = "192.168.1.2";	/* pointer to host name	*/
int     port = 5000;			/* protocol port number	*/  

void send_record(const char* record) {
	
	/* Create a socket. */
	
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (clientSocket < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}
	
	/* determine the server's address */
	
	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET;           /* set family to Internet     */
	sad.sin_port = htons((u_short)port);
	ptrh = gethostbyname(host); /* Convert host name to equivalent IP address and copy to sad. */
	if ( ((char *)ptrh) == NULL ) {
		fprintf(stderr,"invalid host: %s\n", host);
		exit(1);
	}
	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);
	
	/* Send the sentence to the server  */
	n=sendto(clientSocket, record, strlen(record)+1, 0, (struct sockaddr *) &sad, sizeof(struct sockaddr));
	printf("Client sent %d bytes to the server\n", n);
	/* Close the socket. */
	
	close(clientSocket);
}

int main(int argc, char *argv[]) {
	
	
	printf("initializing...\n");
	for (i=0; i<4; i++) {
		adp[i] = adapters + i;
		adp[i]->position = i;
	}
	
	vuint32 *gpio = get_gpio_base();
	vuint32 *gpc = gpio + GPC_OFFSET;
	vuint32 *gpd = gpio + GPD_OFFSET;
	vuint32 *gpg = gpio + GPG_OFFSET;
	
	gpc[CON] = 0x55550000;
	gpd[CON] = 0x55550000;
	gpg[CON] = 0x55500000;
	
	
	pthread_t led_ctrl_threads;
	color = 0xFF;
	blink = 0x00;
	
	if (pthread_create(&led_ctrl_threads, NULL, led_run, (void *)(gpc+DATA))) {
		printf("pthread_create() failed\n");
		abort();
	}
	
	charinit();
	
	init_rtc();
	read_rtc(buf, 100);
	printf("%s\n", buf);
	
	set_rtc("12-28-10 23:21:33 2", 100);
	read_rtc(buf, 100);
	printf("%s\n", buf);
	
	//TODO check USB
	i = mknod("/dev/kb0", S_IFCHR, MKDEV(252, 0));
	printf("mknod returned %d\n", i);
	while (init_keyboard("/dev/kb0")) {
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Must Connect");
		lcd_gotoxy(0, 1);
		lcd_puts("USB Keyboard");
		sleep(1);
	}
	
	
	//TODO check network
	
	/* 
	 *	connect adapters, input names
	 */
	for (i=0; i<4; i++) {
		color &= ~(3 << (i*2));	//clear led[i]
		blink &= ~(3 << (i*2));	
		color |= 2 << (i*2);	//RED
		blink |= 2 << (i*2);	//BLINKING
		
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Install P.S.");
		lcd_gotoxy(0, 1);
		lcd_puts("then hit Btn");
		
		while ((gpc[DATA] & (1 << i)) == 0);
		
		color |= 3 << (i*2);	//ORANGE
		blink |= 3 << (i*2);	//BLINKING
		
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("type its name:");
		lcd_gotoxy(0, 1);
		lcd_putch('>');
		while(readFromUSBKeyboard(adp[i]->name, 20) == 0)
			;
		printf("%s\n", adp[i]->name);
		
		color &= ~(3 << (i*2));	
		color |= 1 << (i*2);	//GREEN
		blink &= ~(3 << (i*2));	//NO BLINKING
		
	}
	
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Press a Key to");
	lcd_gotoxy(0, 1);
	lcd_puts("Start Race");
	readFromUSBKeyboard(buf, 1);
	
	
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Running...");
	
	
	/*
	 *	start the timer, run the race
	 */
	
	gpd[DATA] = 0;
	gpd[DATA] |= 1 << 10;	//stop counting, COUNT high
	gpd[DATA] &= ~(1 << 8);	//reset, RESET low
	usleep(500000);				//wait for the registers to discharge
	
	//	gpd[DATA] |= 1 << 9;	//clock into reg.
	//	gpd[DATA] &= ~(1 << 9);
	//	gpd[DATA] |= 1 << 9;	//clock into reg.
	//	gpd[DATA] &= ~(1 << 9);
	
	//	for (i=0; i<8; i++) {
	//		gpd[DATA] &= ~(7 << 12);
	//		gpd[DATA] |= i << 12;
	//		
	//		printf("after reset, %04x\n", gpd[DATA]);
	//	}
	
	gpd[DATA] |= 1 << 8;	//enable, RESET high
	gpd[DATA] &= ~(1 << 10);	//start counting, COUNT low
	
	//	printf("manually counting...\n");
	//	for (i=0; i<30; i++) {	
	//		gpd[DATA] |= 1 << 11;
	//		gpd[DATA] &= ~(1 << 11);
	//		gpd[DATA] |= 1 << 11;
	//		gpd[DATA] &= ~(1 << 11);
	//	}
	//	waitForButton(gpg+DATA, 8);
	//	while (gpg[DATA] & 1 << 8);
	
	/**
	 *	read if connected
	 */
//	while (1) {
//		printf("%x\n", gpc[DATA]);
//	}
	
	sleep(1);		//wait for the counters to be stable
	
	gpd[DATA] |= 1 << 9;
	gpd[DATA] &= ~(1 << 9);
	
	for (i = 0; i < 4; i++) {
		int c = 0;
		
		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2*i) << 12;
		c |= gpd[DATA] & 0xFF;
		
		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2*i+1) << 12;
		c |= (gpd[DATA] & 0xFF) << 8;
		
		adp[i]->count = c;
	}
	
	//relay off, touching the registers should not burn me now
	gpd[DATA] |= 1 << 10;
	
	
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Done!");
	
	/**
	 *	sort()
	 */
	adapter_sort(adp);
	
	printf("after sorting\n");
	for (i=0; i<4; i++) {
		adapter_print(adp[i]);
	}
	
	/**
	 *	change leds
	 */
	blink = 0;	//NO BLINK
	color = 0;	//OFF
	for (i=0; i<4; i++) {
		if (i<2) {
			color |= 1 << (2 * (adp[i]->position));
		} else {
			color |= 2 << (2 * (adp[i]->position));
		}
	}
	
	sleep (1);
	
	if (argc > 1) {
		/* Extract host-name from command-line argument */
		host = argv[1];         /* if host argument specified   */
	}
	
	if (argc > 2) {
		/* Extract port number  from command-line argument */
		port = atoi(argv[2]);   /* convert to binary            */
	}
	
	for (i=0; i<4; i++) {
		int min_count = adp[0]->count;
		sprintf(buf, " , %s, %d, %d, %d", adp[i]->name, adp[i]->count, adp[i]->count - min_count, i);
		send_record(buf);
	}
	
	
	
	while (1) {
		
	/**
	 *	Replace power supplies
	 */
	
		for (j=2; j<4; j++) {
			i = adp[j]->position;
			
			color &= ~(3 << (i*2));	//clear led[i]
			blink &= ~(3 << (i*2));	
			color |= 2 << (i*2);	//RED
			blink |= 2 << (i*2);	//BLINKING
			
			lcd_clrscr();	
			lcd_gotoxy(0, 0);
			lcd_puts("Replace P.S.");
			lcd_gotoxy(0, 1);
			lcd_puts("Then Hit Button");
			
			while ((gpc[DATA] & (1 << i)) == 0);
			
			color |= 3 << (i*2);	//ORANGE
			blink |= 3 << (i*2);	//BLINKING
			
			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Type Its Name:");
			lcd_gotoxy(0, 1);
			lcd_putch('>');
			while(readFromUSBKeyboard(adp[i]->name, 20) == 0)
				;
			printf("%s\n", adp[i]->name);
			
			color &= ~(3 << (i*2));	
			color |= 1 << (i*2);	//GREEN
			blink &= ~(3 << (i*2));	//NO BLINKING
			
		}
	
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Press a Key to");
		lcd_gotoxy(0, 1);
		lcd_puts("Start Race");
		readFromUSBKeyboard(buf, 1);
		
		
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("running...");
		
		
		/*
		 *	start the timer, run the race
		 */
		
		gpd[DATA] = 0;
		gpd[DATA] |= 1 << 10;	//stop counting, COUNT high
		gpd[DATA] &= ~(1 << 8);	//reset, RESET low
		usleep(500000);				//wait for the registers to discharge
		
		//	gpd[DATA] |= 1 << 9;	//clock into reg.
		//	gpd[DATA] &= ~(1 << 9);
		//	gpd[DATA] |= 1 << 9;	//clock into reg.
		//	gpd[DATA] &= ~(1 << 9);
		
		//	for (i=0; i<8; i++) {
		//		gpd[DATA] &= ~(7 << 12);
		//		gpd[DATA] |= i << 12;
		//		
		//		printf("after reset, %04x\n", gpd[DATA]);
		//	}
		
		gpd[DATA] |= 1 << 8;	//enable, RESET high
		gpd[DATA] &= ~(1 << 10);	//start counting, COUNT low
		
		//	printf("manually counting...\n");
		//	for (i=0; i<30; i++) {	
		//		gpd[DATA] |= 1 << 11;
		//		gpd[DATA] &= ~(1 << 11);
		//		gpd[DATA] |= 1 << 11;
		//		gpd[DATA] &= ~(1 << 11);
		//	}
		//	waitForButton(gpg+DATA, 8);
		//	while (gpg[DATA] & 1 << 8);
		
		/**
		 *	read if connected
		 */
		//	while (1) {
		//		printf("%x\n", gpc[DATA]);
		//	}
		
		sleep(1);		//wait for the counters to be stable
		
		gpd[DATA] |= 1 << 9;
		gpd[DATA] &= ~(1 << 9);
		
		for (i = 0; i < 4; i++) {
			int c = 0;
			
			gpd[DATA] &= ~(7 << 12);
			gpd[DATA] |= (2*i) << 12;
			c |= gpd[DATA] & 0xFF;
			
			gpd[DATA] &= ~(7 << 12);
			gpd[DATA] |= (2*i+1) << 12;
			c |= (gpd[DATA] & 0xFF) << 8;
			
			adp[i]->count = c;
		}
		
		//relay off, touching the registers should not burn me now
		gpd[DATA] |= 1 << 10;
		
		
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Done!");
		
		/**
		 *	sort()
		 */
		adapter_sort(adp);
		
		printf("after sorting\n");
		for (i=0; i<4; i++) {
			adapter_print(adp[i]);
		}
		
		/**
		 *	change leds
		 */
		blink = 0;	//NO BLINK
		color = 0;	//OFF
		for (i=0; i<4; i++) {
			if (i<2) {
				color |= 1 << (2 * (adp[i]->position));
			} else {
				color |= 2 << (2 * (adp[i]->position));
			}
		}
		
		sleep (1);
		
		if (argc > 1) {
			/* Extract host-name from command-line argument */
			host = argv[1];         /* if host argument specified   */
		}
		
		if (argc > 2) {
			/* Extract port number  from command-line argument */
			port = atoi(argv[2]);   /* convert to binary            */
		}
		
		for (i=0; i<4; i++) {
			int min_count = adp[0]->count;
			sprintf(buf, " , %s, %d, %d, %d", adp[i]->name, adp[i]->count, adp[i]->count - min_count, i);
			send_record(buf);
		}
		
	}
	
	//should never reach here
	return 0;
	
}
