/*
 * twb_common.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "common/twb_common.h"

EdgeType_t TWB_GetEdgeType(uint16_t last, uint16_t this, uint16_t pin){
	FlagStatus lastPinStatus = last & pin; //
	FlagStatus thisPinStatus = this & pin;

	if(lastPinStatus == thisPinStatus)
		return EdgeTypeSame;

	if(lastPinStatus != 0x00)
		return EdgeTypeFall;

	return EdgeTypeRise;
}

float minf(float a, float b){
	return (a < b) ? a : b;
}
