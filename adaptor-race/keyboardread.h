
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

#ifndef __KEYBOARDREAD_NTKN__
#define __KEYBOARDREAD_NTKN__


int init_keyboard(const char* device);
int close_keyboard();
int readFromUSBKeyboard(char* name, int length);

#endif
