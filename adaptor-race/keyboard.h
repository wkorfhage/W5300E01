
/*
 *  keyboardread.h
 *  
 *
 *  Created by Liang Qu on 12/3/10.
 *  Copyright 2010 NTKN. All rights reserved.
 *
 *  Display prompt on LCD, read from USB keyboard, then display content in console.
 *
 */

#ifndef __NTKN_KEYPAD__
#define __NTKN_KEYPAD__


int init_keyboard(const char* device);
int close_keyboard();
int readFromUSBKeyboard(char* name, int length);

#endif
