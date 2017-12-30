/*
 * Median.c
 *
 *  Created on: Nov 25, 2012
 *      Author: kevinw
 */

#include "Filters/Filter.h"
#include <string.h>
#include "common/twb_debug.h"

/*
	FilterOption_None = 0,
	FilterOption_Median_3_Sample = 1,
	FilterOption_Median_5_Sample = 2,
	FilterOption_Median_7_Sample = 3,
	FilterOption_Median_9_Sample = 4,
	FilterOption_Moving_Average_0_01 = 20,
	FilterOption_Moving_Average_0_1 = 21,
	FilterOption_Moving_Average_1 = 22,
	FilterOption_Moving_Average_2 = 23,
	FilterOption_Moving_Average_5 = 24,
	FilterOption_Moving_Average_10 = 25,
	FilterOption_Moving_Average_15 = 26,
	FilterOption_Moving_Average_20 = 27,
	FilterOption_Moving_Average_25 = 28,
	FilterOption_Moving_Average_33 = 29,
	FilterOption_Moving_Average_50 = 30,
*/

uint16_t __medianFilterTempHistory[MAX_HISTORY];
int16_t __medianFilterSignedTempHistory[MAX_HISTORY];
float __medianFilterFloatTempHistory[MAX_HISTORY];

void TWB_Filter_Median_Apply_Float(FilterFloat_t *filter, float currentValue){
	if(filter->FilterType == FilterOption_None){
		filter->Current = currentValue;
		return;
	}

	FilterOptions_t medianFilterType = filter->FilterType & 0x0F;
	FilterOptions_t movingAverageFilterType = filter->FilterType & 0xF0;

	if(medianFilterType != FilterOption_None){
		uint8_t maxSize = medianFilterType * 2 + 1;
		uint8_t idx = 0;
		float tmp;

		if(maxSize > 9){
			filter->CurrentSlot = 0;
			filter->History[filter->CurrentSlot++] = currentValue;
		}
		else
			filter->History[filter->CurrentSlot++] = currentValue;

		if(filter->CurrentSlot == maxSize){
			filter->CurrentSlot = 0;
			filter->IsInitialized = true;
		}

		memcpy(__medianFilterFloatTempHistory, filter->History, maxSize * sizeof(float));

		//TODO implement a faster sort
		//Simple, inefficient bubble sort, but we only have a max of 9 items
		while(idx < maxSize - 1){
			if(__medianFilterFloatTempHistory[idx] > __medianFilterFloatTempHistory[idx + 1]){
				tmp = __medianFilterFloatTempHistory[idx];
				__medianFilterFloatTempHistory[idx] = __medianFilterFloatTempHistory[idx + 1];
				__medianFilterFloatTempHistory[idx + 1] = tmp;
				idx = 0;
			}
			else
				idx++;
		}

		if(filter->IsInitialized == 0)
			filter->Current = __medianFilterFloatTempHistory[medianFilterType]; //FilterType just so happens to be in the middle of the array =D
		else
			filter->Current = currentValue;
	}
	else
		filter->Current = currentValue;

	//TODO: These need to really be compensated for by sample rate.
	if(movingAverageFilterType != FilterOption_None && filter->Previous != 0.0f){
		float currentFactor = 1.0f;
		float previousFactor = 0.0f;

		switch(movingAverageFilterType){
			case FilterOption_Moving_Average_0_01: previousFactor = 0.9999f; currentFactor = 0.0001f; break;
			case FilterOption_Moving_Average_0_1: previousFactor = 0.999f; currentFactor = 0.001f; break;
			case FilterOption_Moving_Average_1: previousFactor = 0.99f; currentFactor = 0.01f; break;
			case FilterOption_Moving_Average_2: previousFactor = 0.98f; currentFactor = 0.02f; break;
			case FilterOption_Moving_Average_5: previousFactor = 0.95f; currentFactor = 0.05f; break;
			case FilterOption_Moving_Average_10: previousFactor = 0.90f; currentFactor = 0.10f; break;
			case FilterOption_Moving_Average_15: previousFactor = 0.85f; currentFactor = 0.15f; break;
			case FilterOption_Moving_Average_20: previousFactor = 0.80f; currentFactor = 0.2f; break;
			case FilterOption_Moving_Average_30: previousFactor = 0.70f; currentFactor = 0.3f; break;
			case FilterOption_Moving_Average_40: previousFactor = 0.6f; currentFactor = 0.4f; break;
			case FilterOption_Moving_Average_50: previousFactor = 0.5f; currentFactor = 0.5f; break;
			case FilterOption_Moving_Average_60: previousFactor = 0.4f; currentFactor = 0.6f; break;
			case FilterOption_Moving_Average_70: previousFactor = 0.3f; currentFactor = 0.7f; break;
			case FilterOption_Moving_Average_80: previousFactor = 0.2f; currentFactor = 0.8f; break;
			case FilterOption_Moving_Average_90: previousFactor = 0.1f; currentFactor = 0.9f; break;
			default: previousFactor = 0.0f; currentFactor = 1.0f; break;
		}

		filter->Current = filter->Current * currentFactor + filter->Previous * previousFactor;
	}

	filter->Previous = filter->Current;
}


void TWB_Filter_Median_Apply(Filter_t *filter, uint16_t currentValue){
	if(filter->FilterType == FilterOption_None){
		filter->Current = currentValue;
		return;
	}

	FilterOptions_t medianFilterType = filter->FilterType & 0x0F;
	FilterOptions_t movingAverageFilterType = filter->FilterType & 0xF0;

	if(medianFilterType != FilterOption_None){
		uint8_t maxSize = medianFilterType * 2 + 1;
		uint8_t idx = 0;
		uint16_t tmp;

		if(maxSize > 9){
			filter->CurrentSlot = 0;
			filter->History[filter->CurrentSlot++] = currentValue;
		}
		else
			filter->History[filter->CurrentSlot++] = currentValue;

		if(filter->CurrentSlot == maxSize){
			filter->CurrentSlot = 0;
			filter->IsInitialized = true;
		}

		memcpy(__medianFilterTempHistory, filter->History, maxSize * sizeof(uint16_t));

		//TODO implement a faster sort
		//Simple, inefficient bubble sort, but we only have a max of 9 items
		while(idx < maxSize - 1){
			if(__medianFilterTempHistory[idx] > __medianFilterTempHistory[idx + 1]){
				tmp = __medianFilterTempHistory[idx];
				__medianFilterTempHistory[idx] = __medianFilterTempHistory[idx + 1];
				__medianFilterTempHistory[idx + 1] = tmp;
				idx = 0;
			}
			else
				idx++;
		}

		if(filter->IsInitialized)
			currentValue = __medianFilterTempHistory[medianFilterType]; //FilterType just so happens to be in the middle of the array =D

	}

	//TODO: These need to really be compensated for by sample rate.
	if(movingAverageFilterType != FilterOption_None && filter->Previous != 0){
		volatile float currentFactor = 1.0f;
		volatile float previousFactor = 0.0f;

		switch(movingAverageFilterType){
			case FilterOption_Moving_Average_0_01: previousFactor = 0.9999f; currentFactor = 0.0001f; break;
			case FilterOption_Moving_Average_0_1: previousFactor = 0.999f; currentFactor = 0.001f; break;
			case FilterOption_Moving_Average_1: previousFactor = 0.99f; currentFactor = 0.01f; break;
			case FilterOption_Moving_Average_2: previousFactor = 0.98f; currentFactor = 0.02f; break;
			case FilterOption_Moving_Average_5: previousFactor = 0.95f; currentFactor = 0.05f; break;
			case FilterOption_Moving_Average_10: previousFactor = 0.90f; currentFactor = 0.10f; break;
			case FilterOption_Moving_Average_15: previousFactor = 0.85f; currentFactor = 0.15f; break;
			case FilterOption_Moving_Average_20: previousFactor = 0.80f; currentFactor = 0.2f; break;
			case FilterOption_Moving_Average_30: previousFactor = 0.70f; currentFactor = 0.3f; break;
			case FilterOption_Moving_Average_40: previousFactor = 0.6f; currentFactor = 0.4f; break;
			case FilterOption_Moving_Average_50: previousFactor = 0.5f; currentFactor = 0.5f; break;
			case FilterOption_Moving_Average_60: previousFactor = 0.4f; currentFactor = 0.6f; break;
			case FilterOption_Moving_Average_70: previousFactor = 0.3f; currentFactor = 0.7f; break;
			case FilterOption_Moving_Average_80: previousFactor = 0.2f; currentFactor = 0.8f; break;
			case FilterOption_Moving_Average_90: previousFactor = 0.1f; currentFactor = 0.9f; break;

			default: previousFactor = 0.0f; currentFactor = 1.0f; break;
		}

		volatile uint16_t part1 = (uint16_t)((float)currentValue * currentFactor);
		volatile uint16_t part2 = (uint16_t)((float)filter->Previous * previousFactor);

		currentValue = (uint16_t)(part1 + part2);
	}

	filter->Current = currentValue;
	filter->Previous = filter->Current;
}

void TWB_Filter_Median_Apply_Signed(FilterSigned_t *filter, int16_t currentValue){
	if(filter->FilterType == FilterOption_None){
		filter->Current = currentValue;
		return;
	}

	volatile FilterOptions_t medianFilterType = filter->FilterType & 0x0F;
	volatile FilterOptions_t movingAverageFilterType = filter->FilterType & 0xF0;

	if(medianFilterType != FilterOption_None){
		uint8_t maxSize = medianFilterType * 2 + 1;
		uint8_t idx = 0;
		int16_t tmp;

		if(maxSize > 9){
			filter->CurrentSlot = 0;
			filter->History[filter->CurrentSlot++] = currentValue;
		}
		else
			filter->History[filter->CurrentSlot++] = currentValue;

		if(filter->CurrentSlot == maxSize){
			filter->CurrentSlot = 0;
			filter->IsInitialized = true;
		}

		memcpy(__medianFilterSignedTempHistory, filter->History, maxSize * sizeof(int16_t));

		//TODO implement a faster sort
		//Simple, inefficient bubble sort, but we only have a max of 9 items
		while(idx < maxSize - 1){
			if(__medianFilterSignedTempHistory[idx] > __medianFilterSignedTempHistory[idx + 1]){
				tmp = __medianFilterSignedTempHistory[idx];
				__medianFilterSignedTempHistory[idx] = __medianFilterSignedTempHistory[idx + 1];
				__medianFilterSignedTempHistory[idx + 1] = tmp;
				idx = 0;
			}
			else
				idx++;
		}

		if(filter->IsInitialized){
			volatile uint16_t slotIndex = medianFilterType;
			currentValue = __medianFilterSignedTempHistory[slotIndex]; //FilterType just so happens to be in the middle of the array =D
		}

	}

		//TODO: These need to really be compensated for by sample rate.
	if(movingAverageFilterType != FilterOption_None && filter->MovingAverageInitialized == true){
		volatile float currentFactor = 1.0f;
		volatile float previousFactor = 0.0;

		switch(movingAverageFilterType){
			case FilterOption_Moving_Average_0_01: previousFactor = 0.9999f; currentFactor = 0.0001f; break;
			case FilterOption_Moving_Average_0_1: previousFactor = 0.999f; currentFactor = 0.001f; break;
			case FilterOption_Moving_Average_1: previousFactor = 0.99f; currentFactor = 0.01f; break;
			case FilterOption_Moving_Average_2: previousFactor = 0.98f; currentFactor = 0.02f; break;
			case FilterOption_Moving_Average_5: previousFactor = 0.95f; currentFactor = 0.05; break;
			case FilterOption_Moving_Average_10: previousFactor = 0.90f; currentFactor = 0.10f; break;
			case FilterOption_Moving_Average_15: previousFactor = 0.85f; currentFactor = 0.15f; break;
			case FilterOption_Moving_Average_20: previousFactor = 0.80f; currentFactor = 0.2f; break;
			case FilterOption_Moving_Average_30: previousFactor = 0.70f; currentFactor = 0.3f; break;
			case FilterOption_Moving_Average_40: previousFactor = 0.6f; currentFactor = 0.4f; break;
			case FilterOption_Moving_Average_50: previousFactor = 0.5f; currentFactor = 0.5f; break;
			case FilterOption_Moving_Average_60: previousFactor = 0.4f; currentFactor = 0.6f; break;
			case FilterOption_Moving_Average_70: previousFactor = 0.3f; currentFactor = 0.7f; break;
			case FilterOption_Moving_Average_80: previousFactor = 0.2f; currentFactor = 0.8f; break;
			case FilterOption_Moving_Average_90: previousFactor = 0.1f; currentFactor = 0.9f; break;
			default: previousFactor = 0.0f; currentFactor = 1.0f; break;
		}

		volatile int16_t part1 = (int16_t)((float)currentValue * currentFactor);
		volatile int16_t part2 = (int16_t)((float)filter->Previous * previousFactor);

		currentValue = (part1 + part2);
	}

	filter->Current = currentValue;
	filter->Previous = filter->Current;

	filter->MovingAverageInitialized = true;
}
