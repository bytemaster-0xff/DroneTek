/*
 * complementary.h
 *
 *  Created on: Jan 4, 2013
 *      Author: kevinw
 */

#ifndef COMPLEMENTARY_H_
#define COMPLEMENTARY_H_

#include "common/twb_common.h"
#include "filters/filter.h"
#include "sensors/snsr_main.h"

typedef enum {
	NoAxis = 0,
	PitchAxis = 10,
	RollAxis = 11,
	HeadingAxis = 12,
	AltitudeAxis = 13
} Axis_t;

typedef struct {
	float Period1;
	float Period2;
	Axis_t Axis;
	FilterFloat_t *Filter;
	Sensor_t *Sensor1;
	Sensor_t *Sensor2;
	Sensor_t *Sensor3;
} CompFilter_t;

iOpResult_e TWB_SNSR_Complementary_Init(void);
iOpResult_e TWB_SNSR_Complementary_ResetDefaults(void);
iOpResult_e TWB_SNSR_Complementary_UpdateSettings(uint16_t addr, uint8_t value);
iOpResult_e TWB_SNSR_Complementary_Process(float deltaT);

void TWB_SNSR_Complementary_SendDiag(void);

#endif /* COMPLEMENTARY_H_ */
