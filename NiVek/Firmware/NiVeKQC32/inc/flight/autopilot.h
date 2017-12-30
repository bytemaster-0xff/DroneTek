/*
 * autopilot.h
 *
 *  Created on: Jul 4, 2013
 *      Author: Kevin
 */

#ifndef AUTOPILOT_H_
#define AUTOPILOT_H_

#include "common/twb_common.h"
#include "sensors/fusion/complementary.h"

iOpResult_e TWB_Autopilot_DetermineAltitude(CompFilter_t *filter);
iOpResult_e TWB_AutoPilot_Init(void);
iOpResult_e TWB_AutoPilot_ResetDefaults(void);
iOpResult_e TWB_AutoPilot_Exec(float deltaT);
iOpResult_e TWB_AutoPilot_UpdateSettings(uint16_t addr, uint8_t value);
iOpResult_e TWB_AutoPilot_Arm(void);
iOpResult_e TWB_AutoPilot_Safe(void);

iOpResult_e TWB_AutoPilot_TransitionToFlightMode(void);
iOpResult_e TWB_AutoPilot_TransitionFromFlightMode(void);

void TWB_AutoPilot_QueueAltitudeMsg(void);

#endif /* AUTOPILOT_H_ */
