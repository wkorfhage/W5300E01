#ifndef __NTKN_ADAPTER__
#define __NTKN_ADAPTER__

typedef struct {
	char on;	//whether an adapter is plugged in
	char name[32];
	unsigned int count;
	int position;
} Adapter;

void adapter_init(Adapter *adpt);

void adapter_sort(Adapter **list);

#endif
