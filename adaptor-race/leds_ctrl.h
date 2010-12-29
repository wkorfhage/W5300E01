#ifndef __NTKN_LEDS_CTRL__
#define __NTKN_LEDS_CTRL__

#include "ntkn_w5300e01.h"

#define OFF				0
#define GREEN			1
#define RED				2
#define ORANGE			3
#define BLINK_GREEN		4
#define BLINK_RED		5
#define BLINK_ORANGE	6

uint8 color;
uint8 blink;

void* led_run(void *arg);

#endif
