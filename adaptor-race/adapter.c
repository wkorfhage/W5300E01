#include "adapter.h"

/**
 *	initialize an adapter
 */
void adapter_init(Adapter *adpt) {
	adpt->name[0] = 0;
	adpt->position = -1;
	adpt->count = -1;
}

/**
 *	print an adapter
 */
void adapter_print(Adapter *adpt) {
	printf("Apater@[%p]: \nposition=%d, count=%x, name=%s\n", adpt, adpt->position, adpt->count, adpt->name);
}

/**
 *	sort a list of 4 adapters
 */
void adapter_sort(Adapter **list) {
	int i;
	for (i = 0; i < 3; i++) {
		int j;
		for (j=0; j<4-i-1; j++) {
			if (list[j]->count > list[j+1]->count) {
				Adapter *tmp = list[j];
				list[j] = list[j+1];
				list[j+1] = tmp;
			}
		}
	}
}

