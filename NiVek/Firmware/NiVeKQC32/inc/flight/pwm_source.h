/*
 * esc.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef ESC_H_
#define ESC_H_

#include "common/twb_common.h"

iOpResult_e TWB_PWM_Source_Init(void);

typedef enum {
	FiftyHz = 1,
	HundredHz = 2,
	OneFiftyHz = 3,
	TwoHundredHz = 4,
	TwoFiftyHz = 5,
	ThreeHundredHz = 6,
	ThreeFiftyHz = 7,
	FourHundredHz = 8
} ESCUpdateRate_t;


iOpResult_e TWB_PWM_SetNewESCUpdateRate(ESCUpdateRate_t rate);
void TWB_PWM_ManagePower(void); //Update interval in 1/10 of ms.
iOpResult_e TWB_PWM_SetESCPower(Motors_t channel, uint8_t value);
void TWB_Motors_QueueMsg(void);

#endif /* ESC_H_ */
