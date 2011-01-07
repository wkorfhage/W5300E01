#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/kdev_t.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>

#include "ntkn_w5300e01.h"
#include "button.h"
#include "gpio.h"
#include "keyboard.h"
#include "charlib.h"
#include "leds_ctrl.h"
#include "adapter.h"
#include "rtc.h"

int i, j, n;
int skip;
char buf[256];
char logfile[] = "/mnt/jffs2/race.log";

Adapter adapters[4];
Adapter *adp[4];

vuint32 *gpio, *gpc, *gpd, *gpg;

struct sockaddr_in sad; /* structure to hold an IP address     */
int clientSocket; /* socket descriptor                   */
struct hostent *ptrh; /* pointer to a host table entry       */

char *host = "192.168.1.2"; /* pointer to host name	*/
int port = 5000; /* protocol port number	*/

int time_from_keyboard(char *buf) {
	char local_buf[128];
	int year, month, date, hh, mm, ss;
	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("YY.MM.DD.hh.mm.ss");
	lcd_gotoxy(0, 1);
	lcd_putch('>');
	readFromUSBKeyboard(local_buf, 32);
	fprintf(stderr, "time_from_keyboard(): %s\n", local_buf);
	sscanf(local_buf, "%d.%d.%d.%d.%d.%d", &year, &month, &date, &hh, &mm, &ss);
	sprintf(buf, "20%d-%d-%d %d:%d:%d.", year, month, date, hh, mm, ss);
	fprintf(stderr, "time_from_keyboard(): %s\n", buf);

	return 0;
}

int time_from_server(char *buf) {

	/* Create a socket. */
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (clientSocket < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}

	/* determine the server's address */
	memset((char *) &sad, 0, sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet     */
	sad.sin_port = htons((u_short)port);
	ptrh = gethostbyname(host); /* Convert host name to equivalent IP address and copy to sad. */
	if (((char *) ptrh) == NULL) {
		fprintf(stderr, "invalid host: %s\n", host);
		exit(1);
	}
	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Send the sentence to the server  */
	n = sendto(clientSocket, buf, strlen(buf) + 1, 0, (struct sockaddr *) &sad,
			sizeof(struct sockaddr));
	fprintf(stderr, "Client sent %d bytes to the server\n", n);

	fd_set rfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(clientSocket, &rfds);
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	retval = select(clientSocket + 1, &rfds, NULL, NULL, &tv);

	if (retval) {
		n = recvfrom(clientSocket, buf, strlen(buf) + 1, 0,
				(struct sockaddr *) &sad, NULL);
		fprintf(stderr, "time from server===: %s\n", buf);
		close(clientSocket);
		return 0;
	} else {
		fprintf(stderr, "error connecting to server, timeout!");
		close(clientSocket);
		return -1;
	}

}

void send_record(const char* record) {

	/* Create a socket. */

	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (clientSocket < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}

	/* determine the server's address */

	memset((char *) &sad, 0, sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet     */
	sad.sin_port = htons((u_short)port);
	ptrh = gethostbyname(host); /* Convert host name to equivalent IP address and copy to sad. */
	if (((char *) ptrh) == NULL) {
		fprintf(stderr, "invalid host: %s\n", host);
		exit(1);
	}
	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Send the sentence to the server  */
	n = sendto(clientSocket, record, strlen(record) + 1, 0,
			(struct sockaddr *) &sad, sizeof(struct sockaddr));
	fprintf(stderr, "Client sent %d bytes to the server\n", n);
	/* Close the socket. */

	close(clientSocket);
}

void run_race() {
	gpd[DATA] = 0;
	gpd[DATA] |= 1 << 10; //stop counting, COUNT high
	gpd[DATA] &= ~(1 << 8); //reset, RESET low
	usleep(500000); //wait for the resistors to discharge

	gpd[DATA] |= 1 << 8; //enable, RESET high
	gpd[DATA] &= ~(1 << 10); //start counting, COUNT low

	sleep(1); //wait for the counters to be stable

	gpd[DATA] |= 1 << 9;
	gpd[DATA] &= ~(1 << 9);

	for (i = 0; i < 4; i++) {
		int c = 0;

		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2 * i) << 12;
		c |= gpd[DATA] & 0xFF;

		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2 * i + 1) << 12;
		c |= (gpd[DATA] & 0xFF) << 8;

		adapters[i].on = gpc[DATA] & (1 << (i + 4));
		adapters[i].count = c;
	}

	//relay off, touching the registers should not burn me now
	gpd[DATA] |= 1 << 10;

}

int main(int argc, char *argv[]) {

	fprintf(stderr, "initializing...\n");
	for (i = 0; i < 4; i++) {
		adp[i] = adapters + i;
		adp[i]->position = i;
	}

	gpio = get_gpio_base();
	gpc = gpio + GPC_OFFSET;
	gpd = gpio + GPD_OFFSET;
	gpg = gpio + GPG_OFFSET;

	gpc[CON] = 0x55550000;
	gpd[CON] = 0x55550000;
	gpg[CON] = 0x55500000;

	pthread_t led_ctrl_threads;
	color = 0xFF;
	blink = 0x00;

	if (pthread_create(&led_ctrl_threads, NULL, led_run, (void *) (gpc + DATA))) {
		fprintf(stderr, "pthread_create() failed\n");
		abort();
	}

	charinit();

	init_rtc();
	read_rtc(buf, 100);
	fprintf(stderr, "%s\n", buf);

	//TODO check USB
	i = mknod("/dev/kb0", S_IFCHR, MKDEV(252, 0));
	fprintf(stderr, "mknod returned %d\n", i);
	while (init_keyboard("/dev/kb0")) {
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Must Connect");
		lcd_gotoxy(0, 1);
		lcd_puts("USB Keyboard");
		sleep(1);
	}

	//TODO check network
	if (argc > 1) {
		/* Extract host-name from command-line argument */
		host = argv[1]; /* if host argument specified   */
	}

	if (argc > 2) {
		/* Extract port number  from command-line argument */
		port = atoi(argv[2]); /* convert to binary            */
	}

	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Connecting...");
	sprintf(buf, "Tell me the time");
	if (time_from_server(buf) != 0) {
		//failed to connect to server
		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Connect Failed");
		lcd_gotoxy(0, 1);
		lcd_puts("Input Time ");
		readFromUSBKeyboard(buf, 1);
		time_from_keyboard(buf);
		set_rtc(buf, 100);
	} else { //succeeded
		set_rtc(buf, 100);
	}

	//open log file for writing
	FILE *logfp = fopen(logfile, "a+");
	fprintf(stderr, "log file is: %x\n", logfp);

	/* 
	 *	Initial setup
	 *	connect adapters, input names
	 */
	for (i = 0; i < 4; i++) {
		//initialize adapters
		adapter_init(adapters + i);

		color &= ~(3 << (i * 2)); //clear led[i]
		blink &= ~(3 << (i * 2));
		color |= 2 << (i * 2); //RED
		blink |= 2 << (i * 2); //BLINKING

		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Install Power Supply");

		//		int done = 0;

		//wait for button OR plug
		while (gpc[DATA] & (1 << i)) {
			gpd[DATA] &= ~(1 << 10); //relay on
			usleep(10000);
			adapters[i].on = gpc[DATA] & (1 << (i + 4));
			gpd[DATA] |= 1 << 10; //relay off
			usleep(100000);

			if (adapters[i].on) {
				break;
			}

			//			done = (gpc[DATA] & 0xF) != 0xF;
			//			printf("%x, %x\n", gpc[DATA], done);
			//			if (done) {
			//				break;
			//			}
		}

		//		if (done) {
		//			break;
		//		}

		if (adapters[i].on) {

		} else {
			adapters[i].name[0] = 0;
			color &= ~(3 << (i * 2)); //clear led[i]
			blink &= ~(3 << (i * 2)); //NO BLINKING
			color |= 2 << (i * 2); //RED

			continue;
		}

		color |= 3 << (i * 2); //ORANGE
		blink |= 3 << (i * 2); //BLINKING

		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("type its name:");
		lcd_gotoxy(0, 1);
		lcd_putch('>');
		while (readFromUSBKeyboard(adapters[i].name, 20) == 0)
			;
		fprintf(stderr, "%s\n", adapters[i].name);

		color &= ~(3 << (i * 2));
		color |= 1 << (i * 2); //GREEN
		blink &= ~(3 << (i * 2)); //NO BLINKING

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
	run_race();

	lcd_clrscr();
	lcd_gotoxy(0, 0);
	lcd_puts("Done!");

	/**
	 *	sort()
	 */
	adapter_sort(adp);

	fprintf(stderr, "after sorting\n");
	for (i = 0; i < 4; i++) {
		adapter_print(adp[i]);
	}

	/**
	 *	change leds
	 */
	blink = 0; //NO BLINK
	color = 0; //OFF
	for (i = 0; i < 4; i++) {
		if (i < 2 && adp[i]->on) { //fast and connected, green
			color |= 1 << (2 * (adp[i]->position));
		} else { //slow, red
			color |= 2 << (2 * (adp[i]->position));
		}
	}

	sleep(1);

	for (i = 0; i < 4; i++) {
		int min_count = adp[0]->count;
		read_rtc(buf + 20, 20);
		sprintf(buf, "%s, %s, %d, %d, %d", buf+20, adp[i]->name, adp[i]->count, adp[i]->count - min_count, i);
		send_record(buf);
		fprintf(logfp, "%s\n", buf);
		fflush(logfp);
	}

	while (1) {

		/**
		 *	Replace the slower 2 power supplies
		 */

		for (j = 2; j < 4; j++) {
			i = adp[j]->position;

			color &= ~(3 << (i * 2)); //clear led[i]
			blink &= ~(3 << (i * 2));
			color |= 2 << (i * 2); //RED
			blink |= 2 << (i * 2); //BLINKING

			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Replace Power Supply");

			//wait for removal
			skip = 0;
			//if it's on, wait until it's removed, or hit button to skip
			while (!skip) {
				//if button pressed, skip
				skip = ((gpc[DATA] & (1 << i)) == 0);
				//				printf("in 1st while()\n");

				gpd[DATA] &= ~(1 << 10);
				usleep(10000);

				adapters[i].on = gpc[DATA] & (1 << (i + 4));
				gpd[DATA] |= 1 << 10; //relay off
				usleep(100000);

				//power supply plugged, continue
				if (adapters[i].on == 0) {
					break;
				}

			}

			usleep(100000);

			//wait to be plugged, or button pressed, or hit button to skip
			while (!skip) {
				//if button pressed, skip
				skip = (gpc[DATA] & (1 << i) == 0);
				//				printf("in 2nd loop\n");
				gpd[DATA] &= ~(1 << 10);
				usleep(10000);

				adapters[i].on = gpc[DATA] & (1 << (i + 4));
				gpd[DATA] |= 1 << 10; //relay off
				usleep(100000);

				//power supply plugged, continue
				if (adapters[i].on) {
					break;
				}

			}

			if (!skip) {

			} else {
				adapters[i].name[0] = 0;
				color &= ~(3 << (i * 2)); //clear led[i]
				blink &= ~(3 << (i * 2)); //NO BLINKING
				color |= 2 << (i * 2); //RED

				continue;
			}

			color |= 3 << (i * 2); //ORANGE
			blink |= 3 << (i * 2); //BLINKING

			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Type Its Name:");
			lcd_gotoxy(0, 1);
			lcd_putch('>');
			while (readFromUSBKeyboard(adapters[i].name, 20) == 0)
				;
			fprintf(stderr, "%s\n", adapters[i].name);

			color &= ~(3 << (i * 2));
			color |= 1 << (i * 2); //GREEN
			blink &= ~(3 << (i * 2)); //NO BLINKING

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
		run_race();

		lcd_clrscr();
		lcd_gotoxy(0, 0);
		lcd_puts("Done!");

		/**
		 *	sort()
		 */
		adapter_sort(adp);

		fprintf(stderr, "after sorting\n");
		for (i = 0; i < 4; i++) {
			adapter_print(adp[i]);
		}

		/**
		 *	change leds
		 */
		blink = 0; //NO BLINK
		color = 0; //OFF
		for (i = 0; i < 4; i++) {
			if (i < 2 && adp[i]->on) { //fast and connected, green
				color |= 1 << (2 * (adp[i]->position));
			} else { //slow, red
				color |= 2 << (2 * (adp[i]->position));
			}
		}

		sleep(1);

		for (i = 0; i < 4; i++) {
			int min_count = adp[0]->count;
			read_rtc(buf + 20, 20);
			sprintf(buf, "%s, %s, %d, %d, %d", buf+20, adp[i]->name, adp[i]->count, adp[i]->count - min_count, i);
			send_record(buf);
			fprintf(logfp, "%s\n", buf);
			fflush(logfp);
		}

	}

	//should never reach here
	return 0;

}
