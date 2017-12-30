#ifndef FILTER_H
#define FILTER_H

#include "twb_config.h"

#define MAX_HISTORY 9

//Code smell here, using a bit more RAM than necessary.

typedef struct {
	uint8_t CurrentSlot;
	FilterOptions_t FilterType;
	uint16_t History[MAX_HISTORY];
	uint16_t Current;
	uint16_t Previous;
	uint8_t IsInitialized;
} Filter_t;

typedef struct {
	uint8_t CurrentSlot;
	FilterOptions_t FilterType;
	int16_t History[MAX_HISTORY];
	int16_t Current;
	int16_t Previous;
	uint8_t IsInitialized;
	uint8_t MovingAverageInitialized;
} FilterSigned_t;

typedef struct {
	uint8_t CurrentSlot;
	FilterOptions_t FilterType;
	float History[MAX_HISTORY];
	float Current;
	float Previous;
	uint8_t IsInitialized;
} FilterFloat_t;

//Size per structure 22;

void TWB_Filter_Median_Apply_Float(FilterFloat_t *filter, float current);
void TWB_Filter_Median_Apply_Signed(FilterSigned_t *filter, int16_t current);
void TWB_Filter_Median_Apply(Filter_t *filter, uint16_t current);

#endif
