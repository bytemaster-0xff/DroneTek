#ifndef TWB_MEMORY_MGMT_H_
#define TWB_MEMORY_MGMT_H_

#include "twb_common.h"

void *pm_malloc(uint16_t size);
char * createStr(const char *str);
void pm_printHeapSize(void);
#endif
