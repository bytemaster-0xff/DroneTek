#include <string.h>

#include "common/memory_mgmt.h"
#include "common/twb_debug.h"

#define HEAPSIZE 0x8000 //Start out with a roughly 32K heap

uint16_t __heapPtr = 0;
uint8_t __customHeap[HEAPSIZE];

/* Poor mans malloc, only used to allocate
 * memory, no free
 */
void *pm_malloc(uint16_t size){
	while(__heapPtr % 4 != 0)
		__heapPtr++;

	void *block = &__customHeap[__heapPtr];
	if(size + __heapPtr > HEAPSIZE)
		return 0;

	__heapPtr += size;
	return block;
}

char * createStr(const char *str){
	uint8_t len = strlen(str) + 1;

	char *out = pm_malloc(len); //Add null terminator.
	for(uint8_t idx = 0; idx <= len; ++idx){
		if(idx == len)
			out[idx] = 0x00;
		else
			out[idx] = str[idx];
	}

	return out;
}

void pm_printHeapSize(void){
	TWB_Debug_PrintInt("MEMORY ALLOCATED: ", __heapPtr);
}



