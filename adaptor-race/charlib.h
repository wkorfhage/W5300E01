#ifndef _CHAR_LCD_H
#define _CHAR_LCD_H

#include <linux/types.h>

#define LCD_MAX_COL		16
#define LCD_MAX_ROW		2

#define LCD_BASEADDR		0x18000000 // CS3 = 0x18000000

/* LCD Command Read/Write Register */
//#define LcdCommandW (*((unsigned char*)LCD_BASEADDR))	
//#define LcdCommandR (*((unsigned char*)(LCD_BASEADDR+1)))	

/* LCD Data Read/Write Register */
//#define LcdDataW    (*((unsigned char*)(LCD_BASEADDR+2)))	
//#define LcdDataR    (*((unsigned char*)(LCD_BASEADDR+3)))	

int  init(void);
char lcd_ready(void);			// Check for LCD to be ready
void lcd_clrscr(void);			// Clear LCD. 
char lcd_init(void);			// LCD Init
void lcd_gotoxy(unsigned char x, unsigned char y);	// Output character string in current Cursor.
char* lcd_puts(char* str);		// Output character stream in current Cursor.
void lcd_putch(char ch);		// Output 1 character in current Cursor.
void lcd_home_cursor(void);		// Move Cursor first Column.
void lcd_set_cursor_type(unsigned char type);	// Decide Cursor type.
void lcd_shitf_cursor(unsigned char dir);	// Shift to Left and Right current Cursor.

#endif
