
/*
 *  keyboardread.c
 *  
 *
 *  Created by Liang Qu on 12/3/10.
 *  Copyright 2010 NTKN. All rights reserved.
 *
 *  Display prompt on LCD, read from USB keyboard, then display content in console.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ntkn_w5300e01.h"
#include "charlib.h"
#include "keyboardread.h"

int readFromUSBKeyboard(const char* device, char* name, int length) {

	int fd = open(device, O_RDONLY);
	char buf[3];
	char x;
	
	int count = 0;
	
	if (charinit()) 
	{
		perror("Init didn't work: ");
		exit(-1);
	}
	
	lcd_puts("type the name:");
	lcd_gotoxy(0, 1);
	lcd_putch('>');
	
	while(read(fd, buf, 3)) {
		if (count == 32) {
			break;
		}
		
		/*	The third byte is the key-code, 'a' => 4, '1' => 30
		 *	releasing a key sends a zero.
		 */
		if (x = buf[2]) {	//assignment AND test		
			if (x == 40) {		//ENTER is 40
				break;	
			} else if (x == 42) {		//BACKSPACE is 42
				if (count > 0) {
					count--;
					ShiftCursor(0);
					lcd_putch(' ');
					ShiftCursor(0);
				}
			}
			
			if (x >= 4 && x <= 4+26-1) {	// 'a' to 'z'
				x = x - 4 + 'a';
				name[count++] = x;
				lcd_putch(x);
			} else if (x >= 30 && x <= 38) {	// '1' to '9'
				x = x - 30 + '1';
				name[count++] = x;
				lcd_putch(x);
			} else if (x == 39){			// '0'
				x = '0';
				name[count++] = x;
				lcd_putch(x);
			}
			

			// printf("%c", x);
		}		
		
	}
	
	if (count >= length) {
		count = length - 1;
	}
	name[count] = 0;
	// printf("\nread from keyboard: %s\n", name);
	lcd_puts("bye!");
	close(fd);
	return 0;
}

int main(int argc, char* argv[]) {
	char buf[20];
	readFromUSBKeyboard("./kb0", buf, 20);
	printf("%s\n", buf);
	return 0;
}
