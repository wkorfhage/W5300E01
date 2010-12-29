
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
#include "keyboard.h"

static int keyboard_fd;

/**
 *	initializes the USB keyboard (open the device file)
 *	if successful, return 0, otherwise, return a non-zero
 */
int init_keyboard(const char* device) {
	keyboard_fd = open(device, O_RDONLY);

	return keyboard_fd<0;
}

/**
 *	close the device file, always succeeds
 */
int close_keyboard() {
	close(keyboard_fd);
	
	return 0;
}

/**
 *	returns the number of character written to the buffer
 *	not including the last '\0'
 */
int readFromUSBKeyboard(char* name, int length) {

	char buf[3];
	char x;
	
	int count = 0;
	
	//LCD should be initialized already
	
	//Enable the cursor, and make it blinking.
	lcd_set_cursor_type(3);
	
	// will need an extra position reserved for the '\0'
	// input > 40 will wrap around the LCD buffer
	while(count < length && count < 32 && read(keyboard_fd, buf, 3)) {
		
		/*	The third byte is the key-code, 'a' => 4, '1' => 30
		 *	releasing a key sends a zero.
		 */
		if (x = buf[2]) {	//assignment AND test	
			if (x == 40 || x == 88) {		//ENTER is 40
				break;	
			} else if (x == 42) {		//BACKSPACE is 42
				if (count > 0) {
					count--;
					ShiftCursor(0);
					lcd_putch(' ');
					ShiftCursor(0);
				}
				
				continue;
			}
			
			if (x >= 4 && x <= 4+26-1) {		// 'a' to 'z'
				x = x - 4 + 'a';
			} else if (x >= 30 && x <= 38) {	// '1' to '9'
				x = x - 30 + '1';
			} else if (x >= 89 && x <= 97) {	// '1' to '9'
				x = x - 89 + '1';
			} else if (x == 39 || x == 98){		// '0'
				x = '0';
			} else if (x == 84) {
				x = '/';
			} else if (x == 85) {
				x = '*';
			} else if (x == 86) {
				x = '-';
			} else if (x == 87) {
				x = '+';
			} else if (x == 99) {
				x = '.';
			} else {
				continue;
			}
			
			name[count++] = x;
			lcd_putch(x);
			
			printf("%c, %d\n", x, x);
		}		
		
	}
	
	name[count] = 0;
	
	//Disable the cursor.
	lcd_set_cursor_type(0);
	
	// printf("\nread from keyboard: %s\n", name);

	return count;
}

