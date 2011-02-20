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

char buf[256];
char time_buf[32];
char logfile[] = "/mnt/jffs2/race.log";
char calibrefile[] = "/mnt/jffs2/offset.clb";

Adapter adapters[4];
Adapter *adp[4];
int offset[4];
unsigned long count_sum[4];

vuint32 *gpio, *gpc, *gpd, *gpg;

struct sockaddr_in sad; /* structure to hold an IP address     */
int clientSocket; /* socket descriptor                   */
struct hostent *ptrh; /* pointer to a host table entry       */

char *host = "50.16.187.107"; /* pointer to host name	*/
int port = 5000; /* protocol port number	*/

int time_from_keyboard(char *buf) {
	int year, month, date, hh, mm, ss;

	readFromUSBKeyboard(buf, 32);
	fprintf(stderr, "time_from_keyboard(): %s\n", buf);
	sscanf(buf, "%d.%d.%d.%d.%d.%d", &year, &month, &date, &hh, &mm, &ss);
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
	int n = sendto(clientSocket, buf, strlen(buf) + 1, 0,
			(struct sockaddr *) &sad, sizeof(struct sockaddr));
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
	int n = sendto(clientSocket, record, strlen(record) + 1, 0,
			(struct sockaddr *) &sad, sizeof(struct sockaddr));
	fprintf(stderr, "Client sent %d bytes to the server\n", n);
	/* Close the socket. */

	close(clientSocket);
}

void run_race() {
	gpd[DATA] = 0;
	gpd[DATA] |= 1 << 10; //stop counting, COUNT high
	gpd[DATA] &= ~(1 << 8); //reset, RESET low
	usleep(20000); //wait for the resistors to discharge

	gpd[DATA] |= 1 << 8; //enable, RESET high
	gpd[DATA] &= ~(1 << 10); //start counting, COUNT low

	sleep(1); //wait for the counters to be stable

	gpd[DATA] |= 1 << 9;
	gpd[DATA] &= ~(1 << 9);

	int i;
	for (i = 0; i < 4; i++) {
		int c = 0;

		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2 * i) << 12;
		c |= gpd[DATA] & 0xFF;

		gpd[DATA] &= ~(7 << 12);
		gpd[DATA] |= (2 * i + 1) << 12;
		c |= (gpd[DATA] & 0xFF) << 8;

		adapters[i].on = gpc[DATA] & (1 << (i + 4));
		adapters[i].count = c + offset[i];
	}

	//relay off, touching the registers should not burn me now
	gpd[DATA] |= 1 << 10;

}

int main(int argc, char *argv[]) {

	fprintf(stderr, "initializing...\n");
	int i;
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
	color = 0xAA;
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
		lcd_puts("Connection Failed");
		lcd_gotoxy(0, 1);
		lcd_puts("Input Time Manually");
		lcd_gotoxy(0, 2);
		lcd_puts("YY.MM.DD.hh.mm.ss");
		lcd_gotoxy(0, 3);
		lcd_puts(">");

		time_from_keyboard(buf);
		lcd_clrscr();
		set_rtc(buf, 100);
	} else { //succeeded
		set_rtc(buf, 100);
	}

	//open log file for writing
	FILE *logfp = fopen(logfile, "a+");
	fprintf(stderr, "log file is: %x\n", logfp);

	//open calibration file for reading

	FILE *calibrefp = fopen(calibrefile, "r");
	if (calibrefp) {
		//reverse the order: 1 2 3 4 -->> 3 2 1 0
		fscanf(calibrefp, "%d %d %d %d", offset + 3, offset + 2, offset + 1,
				offset);
		fclose(calibrefp);
	}

	for (i = 0; i < 4; i++) {
		printf("Offset ====== %d\n", offset[i]);
	}

	/* 
	 *	Initial setup
	 *	connect adapters, input names
	 */

	for (i = 0; i < 4; i++) {
		adapter_init(adapters + i);
	}

	int existence = 0;
	while (1) {
		gpd[DATA] &= ~(1 << 10); //relay on
		usleep(10000);
		existence = (gpc[DATA] & 0xF0) >> 4;
		gpd[DATA] |= 1 << 10; //relay off

		printf("existence: %x\n", existence);

		/*
		 * clear the names of the absent ones
		 */
		for (i = 0; i < 4; i++) {
			if ((existence & (1 << i)) == 0) {
				color &= ~(3 << i * 2);
				adapters[i].name[0] = 0;
			} else if ((existence & (1 << i)) && adapters[i].name[0]) {
				//on and has a name
				//				color &= ~(3 << i * 2);
				//				color |= 1 << i * 2;
			} else {
				//on but waiting for a name
				color |= 3 << i * 2;
			}
		}

		/*
		 * input missing names
		 * 'i' is the smallest index that's present and waiting for a name
		 */
		i = 0;
		while (((existence & (1 << i)) == 0 || (adapters[i].name[0]))) {
			i++;
			if (i == 4) {
				break;
			}
		}

		if (i < 4) {
			color |= 3 << i * 2; //ORANGE
			blink |= 3 << i * 2; //BLINKING
			lcd_clrscr();
			lcd_gotoxy(0, 0);
			lcd_puts("Input the Name:");
			lcd_gotoxy(0, 1);
			lcd_putch('>');
			while (readFromUSBKeyboard(adapters[i].name, 20) == 0)
				;
			color &= ~(3 << i * 2);
			color |= (1 << i * 2);
			blink &= ~(3 << i * 2);

			continue; //check for more inputs
		}

		/*
		 * check for GO button
		 */
		lcd_gotoxy(0, 0);
		lcd_puts("Install or Change");
		lcd_gotoxy(0, 1);
		lcd_puts("Power Supplies");
		lcd_gotoxy(0, 2);
		lcd_puts("Press Any Button");
		lcd_gotoxy(0, 3);
		lcd_puts("To Run Race...");
		//		printf("button: %x\n", gpc[DATA]);

		if ((gpc[DATA] & 0xF) != 0xF) {
			do { //repeat if the input is not a valid number
				lcd_clrscr();
				lcd_gotoxy(0, 0);
				lcd_puts("How many to run?");
				lcd_gotoxy(0, 1);
				lcd_puts(">");
				readFromUSBKeyboard(buf, 8);
			} while (buf[0] < '0' || buf[0] > '9');
			int run_count = atoi(buf);

			//reset count_sum
			for (i = 0; i < 4; i++) {
				count_sum[i] = 0;
			}

			for (i = 0; i < run_count; i++) {
				sprintf(buf, "%d/%d", i+1, run_count);
				lcd_clrscr();
				lcd_gotoxy(0, 0);
				lcd_puts("Running Race");
				lcd_gotoxy(0, 1);
				lcd_puts(buf);
				run_race();

				int j;
				for (j = 0; j < 4; j++) {
					count_sum[j] += adapters[j].count;
				}

				/**
				 *	sort() individual
				 */
				adapter_sort(adp);

				fprintf(stderr, "after sorting (individual)\n");
				for (j = 0; j < 4; j++) {
					adapter_print(adp[j]);
				}

				read_rtc(time_buf, 20);
				for (j = 0; (j < 4) && (adp[j]->name[0] != 0); j++) {
					//send to the server
					sprintf(buf, "%s, %s, %d, %d, %d", time_buf, adp[j]->name, adp[j]->count, adp[j]->count - adp[0]->count, j);
					send_record(buf);

					//write to the log file
					fprintf(logfp, "%s\n", buf);
					fflush(logfp);
				}

			}

			/**
			 *	calculate average and the sort() again
			 */

			for (i = 0; i < 4; i++) {
				adapters[i].count = count_sum[i] / run_count;
			}

			adapter_sort(adp);

			fprintf(stderr, "after sorting (AVERAGE)\n");
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

			lcd_clrscr();

			int min_count = adp[0]->count;

			//write result to LCD
			for (i = 3; i >= 0; i--) {
				if (adapters[i].name[0] == 0) {
					sprintf(buf, "%d: -", 4-i);
				} else if (&adapters[i] == adp[0]) {
					sprintf(buf, "%d: 0 (%d us)", 4 - i, adapters[i].count/5);
				} else {
					sprintf(buf, "%d: +%d", 4 - i, (adapters[i].count - min_count)/5);
				}
				lcd_gotoxy(0, 3 - i);
				lcd_puts(buf);
			}

			lcd_gotoxy(17, 2);
			lcd_puts("Hit");
			lcd_gotoxy(17, 3);
			lcd_puts("Btn");
			//wait for button
			while ((gpc[DATA] & 0xF) == 0xF) {
				;
			}
			lcd_clrscr();
		} // end of running race

		usleep(200000);
	}

	//should never reach here
	return 0;

}
