/* char_lcd.c: A Linux Character-LCD driver for W5300E01-ARM board. */
/*
	Copyright 2008 WIZnet Co.,Ltd.

	This software may be used and distributed according to the terms of
	the GNU General Public License (GPL), incorporated herein by reference.
	Drivers based on or derived from this code fall under the GPL and must
	retain the authorship, copyright and license notice.  This file is not
	a complete program and may only be used when the entire operating
	system is licensed under the GPL.
*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "charlib.h"

#include <time.h>

// For usleep
#include <unistd.h>


#define BUSY		0x80




unsigned char *LcdCommandW;
unsigned char *LcdCommandR;
unsigned char *LcdDataW;
unsigned char *LcdDataR;
int fd;
struct timespec mytime;

void print_time(struct timespec t)
{
	printf("%ld.%ld", (long) t.tv_sec, t.tv_nsec);
}

void get_and_print_time(const char *label)
{
    if (clock_gettime(CLOCK_REALTIME, &mytime) != 0) {
	perror("Couldn't get clock: ");
    }
    printf ("%s ", label);
    print_time(mytime);
    printf ("\n");
}

int charinit()
{
    if (clock_getres(CLOCK_REALTIME, &mytime) != 0) {
	return 1;
    }
    printf ("Clock resolution is ");
    print_time(mytime);
    printf ("\n");

    fd = open("/dev/mem", O_RDWR);
    printf ("mem opens with fd %d\n", fd);
    if (fd < 0) {
	return 1;
    }
    LcdCommandW = (unsigned char *) mmap((void *) 0, (size_t) 4, 
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, LCD_BASEADDR);
    printf ("mmap returned %X\n", (int) LcdCommandW);
    if (LcdCommandW == MAP_FAILED) return 1;

    LcdCommandR = LcdCommandW+1;
    LcdDataW = LcdCommandW+2;
    LcdDataR = LcdCommandW+3;

    return 0;
}

void mdelay(int i)
{
    usleep(i*1000);
}

char lcd_ready(void)
{
  mdelay(2);
  return 1;
}

void lcd_clrscr(void)       
{
  lcd_ready();
  *LcdCommandW = (unsigned char) 0x01;
}

char lcd_init(void)
{
  if((char)-1 ==lcd_ready()) return 0;

  *LcdCommandW = (unsigned char) 0x38;
  mdelay(50);
  *LcdCommandW = (unsigned char) 0x0C;
  mdelay(50);

  lcd_clrscr();
  lcd_gotoxy(0,0);
  return 1;
}

void lcd_gotoxy(unsigned char x,	unsigned char y)
{
  lcd_ready();
  switch(y)
  {
    case 0 : *LcdCommandW = (unsigned char) (0x80 + x); break;
    case 1 : *LcdCommandW = (unsigned char) (0xC0 + x); break;
    case 2 : *LcdCommandW = (unsigned char) (0x94 + x); break;
    case 3 : *LcdCommandW = (unsigned char) (0xD4 + x); break;
  }
}

char * lcd_puts(char* str)
{
  printf("Sending \"%s\" to LCD\n", str);fflush(stdout);
  unsigned char i;

  lcd_clrscr();
  for (i=0; str[i] != '\0'; i++){
    lcd_ready();
    *LcdDataW = (unsigned char) str[i];
  }
  return str;
}

void lcd_putch(char ch)
{
  lcd_ready();
  *LcdDataW = (unsigned char) ch;
}

void lcd_set_cursor_type(unsigned char type)	
{
  lcd_ready();
  switch(type) 
  {
    //No Cursor 
    case 0 : *LcdCommandW = (unsigned char) 0x0C; break;  
             // Normal Cursor 
    case 1 : *LcdCommandW = (unsigned char) 0x0E; break; 
             // No Cursor | Blink
    case 2 : *LcdCommandW = (unsigned char) 0x0D; break; 
             // Normal Cursor | Blink 
    case 3 : *LcdCommandW = (unsigned char) 0x0F; break; 
  }
}

void ShiftCursor(unsigned char dir)	
{
  lcd_ready();
  if(dir)
    *LcdCommandW = (unsigned char) 0x14;
  else
    *LcdCommandW = 0x10;
}

void lcd_home_cursor(void)       
{
  lcd_ready();
  *LcdCommandW = 0x2;
}

