#include "button.h"

void waitForButton(vuint32 *addr, int bit) {
	while(*addr & 1<<bit);
}
