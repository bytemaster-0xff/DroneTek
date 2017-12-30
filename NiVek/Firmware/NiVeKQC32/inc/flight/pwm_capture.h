/*
 * rc_in.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef RC_IN_H_
#define RC_IN_H_

#include "common/twb_common.h"

typedef enum PWM_Input {
	PWM_In_Pitch = 0,
	PWM_In_Roll = 1,
	PWM_In_Yaw = 2,
	PWM_In_Throttle = 3,
	PWM_In_Aux1 = 4,
	PWM_In_Aux2 = 5
} PWM_Input_t;

iOpResult_e TWB_PWM_Capture_Init(void);

iOpResult_e TWB_RCIN_CalibrateStick(uint8_t setting);

iOpResult_e TWB_RCIN_BeginCalibration(void);
iOpResult_e TWB_RCIN_CancelCalibration(void);
iOpResult_e TWB_RCIN_FinalizeCalibration(void);

void TWB_PWM_RCWatchDogExpire(void);

void TWB_PWM_CaptureRise(PWM_Input_t port);
void TWB_PWM_CaptureFall(PWM_Input_t port);

void TWB_RCIN_QueueMsg(void);

void TWB_PWM_ProcessRCIn(void);

extern uint16_t __rcInRise[];
extern uint16_t __rcInFall[];

#define SEND_PWM_UPDATE_RATE_MS 500

#endif /* RC_IN_H_ */
