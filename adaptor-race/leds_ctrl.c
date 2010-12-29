#include <stdlib.h>

#include "ntkn_w5300e01.h"
#include "leds_ctrl.h"

//should run this as a seperate thread
void* led_run(void *arg) {
	vuint32 *base = arg;
	int i = 0;
	
	while (1) {
		*base &= ~(0xFF << 8);
		*base |= color << 8;
	
		if (blink && (i ^= 1)) {
			*base ^= blink << 8;
		}
		usleep (500000);
	}
}
