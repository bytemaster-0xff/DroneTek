/*
 * esc.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */


#include "flight/pwm_source.h"
#include "commo/message_ids.h"
#include "common/twb_debug.h"
#include "flight/ctrl_loop.h"
#include "common/twb_serial_eeprom.h"

#define PWM_REV1_HACK

/*This file provides functionality for driving PWM devices such as ESC's and Servo's
 * The following are mappings on REVB of the NiVek boards
 *
 * REAR - PC9 - TIM3_CH4 - Starboard Rear
 * RIGHT - PC8 - TIM3_CH3 - Starboard Front
 * LEFT - PC7 - TIM3_CH2 - Port Rear
 * FRONT - PC6 - TIM3_CH1 - Port Front
 *
 * AUX1 - PB3 - TIM2_CH2
 * AUX2 - PB10 - TIM2_CH3
 *
 * For reference the following are input channels
 * PITCH - PB4 - TIM3_CH1
 * Roll -  PB5 - TIM3_CH2
 * Yaw - PB6 - TIM16_CH1
 * Throttle - PB7 - TIM17_CH1
 * 
 * Sonar Echo PC5 Rise/Fall
 *
 * AUX1 - PC11
 * AUX2 - PC12
 *
 */

/*
Full Range = 400 for PWM
Full Range incoming for motor is 255

Therefore int math would be:
	([FULL_RANGE_PWM] * [MOTOR_SET_POINT]) / [FULL_SCALE_MOTOR];

	Or for 75% power (Input = 192/255)
	(400 * 192) / 255;

	400 * 192 will overflow a uint16

	So scale down 400 / 255 to 25/16 which is equivelent to 400 / 256.
*/

#define PWM_RANGE_INT_MATH_NUMERATOR 25
#define PWM_RANGE_INT_MATH_DENOMENATOR 16

uint8_t __escLeftTargetPower = 0;
uint8_t __escRearTargetPower = 0;
uint8_t __escFrontTargetPower = 0;
uint8_t __escRightTargetPower = 0;

#define PWM_Period 						8000 //20ms

#define PWM_MinDutyCycle 			PWM_Period / 20 //1ms or 400
#define PWM_MaxDutyCycle 			PWM_Period / 10 //2ms or 800

uint16_t __pwmRangeScaler = 0;

#define PWM_PSC_CPU_SCALER 1.75

//240 is   20ms Period or 50 Hz
//120 is   10ms Period or 100 Hz
// 60 is    5ms Period or 200 Hz
// 45 is 3.75ms Period or 300 Hz
// 30 is  2.5ms Period or 400 Hz

/*
Period	Prescaler	Hz	ms	Total Counts			Cnt Per MS	Cnt Per FS
8000	120	50	20	48000000	0		400		800		1
8000	60	100	10	48000000	0		800		1600	2
8000	40	150	6.7	48000000	0		1200	2400	3
8000	30	200	5	48000000	0		1600	3200	4
8000	24	250	4	48000000	0		2000	4000	5
8000	20	300	3.3	48000000	0		2400	4800	6
8000	17	353	2.8	48008000	-8000	2824	5648	7.06  Close enough for 350 Hz
8000	15	400	2.5	48000000	0		3200	6400	8
*/
uint16_t __pwm_MapUpdateRate(uint8_t kUpdateRate){
	TWB_Debug_PrintInt("Setting ESC Update Rate (1-50Hz, 8-400Hz): ", kUpdateRate);

	switch(kUpdateRate){
			case FiftyHz:
				__pwmRangeScaler = 1;
				return (uint16_t)(120 * PWM_PSC_CPU_SCALER); //50Hz - 20 ms period
			case HundredHz:
				__pwmRangeScaler = 2;
				return (uint16_t)(60 * PWM_PSC_CPU_SCALER); //100Hz - 10 ms period
			case OneFiftyHz:
				__pwmRangeScaler = 3;
				return (uint16_t)(40 * PWM_PSC_CPU_SCALER); //150Hz - 6.6 ms period
			case TwoHundredHz:
				__pwmRangeScaler = 4;
				return (uint16_t)(30 * PWM_PSC_CPU_SCALER); //200Hz - 5.0 ms period
			case TwoFiftyHz:
				__pwmRangeScaler = 5;
				return (uint16_t)(24 * PWM_PSC_CPU_SCALER); //250Hz - 4.0 ms period
			case ThreeHundredHz:
				__pwmRangeScaler = 6;
				return (uint16_t)(20 * PWM_PSC_CPU_SCALER); //300Hz - 3.3 ms Period
			case ThreeFiftyHz:
				__pwmRangeScaler = 7;
				return (uint16_t)(17 * PWM_PSC_CPU_SCALER); //353Hz - 2.8 ms Period
			case FourHundredHz:
				__pwmRangeScaler = 8;
				return (uint16_t)(15 * PWM_PSC_CPU_SCALER); //400Hz - 2.5 ms Period
	}

	return -1;
}

void TWB_Motors_QueueMsg(void) {
	TWB_Commo_SendMessage(MOD_System, MSG_SYSTEM_MOTOR_STATUS_ID, (uint8_t *)MotorStatus, sizeof(MotorStatus_t));
}

iOpResult_e TWB_PWM_SetESCPower(Motors_t channel, uint8_t value){
	switch(channel){
		case ESC_Front: __escFrontTargetPower = value; break;
		case ESC_Left: __escLeftTargetPower = value; break;
		case ESC_Right: __escRightTargetPower = value; break;
		case ESC_Rear: __escRearTargetPower = value; break;
	}

	return OK;
}


TIM_TypeDef *_timer3;
TIM_TypeDef *_timer14;

void __pwm_RefreshDutyCycles(void){
	TMR_PWM_OUT_PRIMARY->CCR1 = (((PWM_RANGE_INT_MATH_NUMERATOR * (uint16_t)MotorStatus->ESC_Front) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle) * __pwmRangeScaler;
	TMR_PWM_OUT_PRIMARY->CCR2 = (((PWM_RANGE_INT_MATH_NUMERATOR * (uint16_t)MotorStatus->ESC_Left) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle) * __pwmRangeScaler;
	TMR_PWM_OUT_PRIMARY->CCR3 = (((PWM_RANGE_INT_MATH_NUMERATOR * (uint16_t)MotorStatus->ESC_Right) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle) * __pwmRangeScaler;
	TMR_PWM_OUT_PRIMARY->CCR4 = (((PWM_RANGE_INT_MATH_NUMERATOR * (uint16_t)MotorStatus->ESC_Rear) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle) * __pwmRangeScaler;

#ifdef PWM_REV1_HACK
	TMR_PWM_OUT_AUX->CCR3 = (((PWM_RANGE_INT_MATH_NUMERATOR * (uint16_t)MotorStatus->ESC_Rear) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle) * __pwmRangeScaler;
#endif
}


iOpResult_e TWB_PWM_SetNewESCUpdateRate(uint8_t newUpdateRate){
	PIDConstantsConfig->ESCUpdateRate = newUpdateRate;
	TMR_PWM_OUT_PRIMARY->PSC = __pwm_MapUpdateRate(newUpdateRate);

#ifdef PWM_REV1_HACK
	TMR_PWM_OUT_AUX->PSC = __pwm_MapUpdateRate(newUpdateRate);
#endif

	__pwm_RefreshDutyCycles();

	return OK;
}


GPIO_TypeDef *_GPIOC;

iOpResult_e TWB_PWM_Init_PWM_IO(void){
	GPIO_InitTypeDef pwmPinConfig;
	pwmPinConfig.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	pwmPinConfig.GPIO_Mode = GPIO_Mode_AF;
	pwmPinConfig.GPIO_Speed = GPIO_Speed_50MHz;
	pwmPinConfig.GPIO_OType = GPIO_OType_PP;
	pwmPinConfig.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOC, &pwmPinConfig);

	pwmPinConfig.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10;
	GPIO_Init(GPIOB, &pwmPinConfig);

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, TMR_PWM_OUT_PRIMARY_GPIO_AF);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, TMR_PWM_OUT_PRIMARY_GPIO_AF);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, TMR_PWM_OUT_PRIMARY_GPIO_AF);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, TMR_PWM_OUT_PRIMARY_GPIO_AF);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, TMR_PWM_OUT_AUX_GPIO_AF);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, TMR_PWM_OUT_AUX_GPIO_AF);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, TMR_PWM_OUT_AUX_GPIO_AF);

	_GPIOC = GPIOC;

	return OK;
}


iOpResult_e TWB_PWM_Init_TimeBase(void){
	TIM_TimeBaseInitTypeDef  pwmTimeBase;
	uint16_t scaler;

	//Start out by setting the ESC's at 50Hz, for some reason need to do this for turnigy ECS, then when we ARM it, set the proper rate.
	scaler = __pwm_MapUpdateRate(FiftyHz);

	__pwm_RefreshDutyCycles();

	pwmTimeBase.TIM_Prescaler = scaler;
	pwmTimeBase.TIM_CounterMode = TIM_CounterMode_Up;
	pwmTimeBase.TIM_ClockDivision = 0;
	pwmTimeBase.TIM_Period = PWM_Period;
	pwmTimeBase.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TMR_PWM_OUT_PRIMARY, &pwmTimeBase);

	pwmTimeBase.TIM_Prescaler =  (uint16_t)(120 * PWM_PSC_CPU_SCALER);
	pwmTimeBase.TIM_CounterMode = TIM_CounterMode_Up;
	pwmTimeBase.TIM_Period = PWM_Period;
	pwmTimeBase.TIM_ClockDivision = 0;
	pwmTimeBase.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TMR_PWM_OUT_AUX, &pwmTimeBase);

	return OK;
}


iOpResult_e TWB_PWM_Source_Init(void){
	assert_succeed(TWB_SE_ReadU8(K_ESC_UPDATE_RATE, &PIDConstantsConfig->ESCUpdateRate));

	if(PIDConstantsConfig->ESCUpdateRate == 0)
		PIDConstantsConfig->ESCUpdateRate = 8;

	assert_succeed(TWB_PWM_Init_PWM_IO());

	uint16_t idlePulse = (uint16_t) (((uint32_t) 5 * (PWM_Period - 1)) / 100);

	RCC_APB1PeriphClockCmd(TMR_PWM_OUT_PRIMARY_PERIPH, ENABLE);
	RCC_APB1PeriphClockCmd(TMR_PWM_OUT_AUX_PERIPH, ENABLE);

	assert_succeed(TWB_PWM_Init_TimeBase());

	/* Channel 1, 2, 3 and 4 Configuration in PWM mode */
	TIM_OCInitTypeDef  pwmOCInit;
	pwmOCInit.TIM_OCMode = TIM_OCMode_PWM1;
	pwmOCInit.TIM_OutputState = TIM_OutputState_Enable;
	pwmOCInit.TIM_OutputNState = TIM_OutputNState_Enable;
	pwmOCInit.TIM_OCPolarity = TIM_OCPolarity_High;
	pwmOCInit.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	pwmOCInit.TIM_OCIdleState = TIM_OCIdleState_Reset;
	pwmOCInit.TIM_OCNIdleState = TIM_OCIdleState_Set;
	pwmOCInit.TIM_Pulse = idlePulse;

	TIM_OC1Init(TMR_PWM_OUT_PRIMARY, &pwmOCInit);
	TIM_OC2Init(TMR_PWM_OUT_PRIMARY, &pwmOCInit);
	TIM_OC3Init(TMR_PWM_OUT_PRIMARY, &pwmOCInit);
	TIM_OC4Init(TMR_PWM_OUT_PRIMARY, &pwmOCInit);

	TIM_OC3Init(TMR_PWM_OUT_AUX, &pwmOCInit);
	TIM_OC2Init(TMR_PWM_OUT_AUX, &pwmOCInit);

	TIM_Cmd(TMR_PWM_OUT_AUX, ENABLE);
	TIM_Cmd(TMR_PWM_OUT_PRIMARY, ENABLE);

	_timer3 = TMR_PWM_OUT_PRIMARY;

	TWB_Debug_Print("PWM Source:  OK\r\n");

	return OK;
}

void TWB_PWM_ManagePower() {
	uint16_t dutyCycle = 0;

	if(__escLeftTargetPower != MotorStatus->ESC_Left){
		if(__escLeftTargetPower < MotorStatus->ESC_Left)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)--MotorStatus->ESC_Left) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;
		else if (__escLeftTargetPower > MotorStatus->ESC_Left)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)++MotorStatus->ESC_Left) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;

		dutyCycle = dutyCycle * __pwmRangeScaler;
		TMR_PWM_OUT_PRIMARY->CCR2 = dutyCycle;
	}

	if(__escRearTargetPower != MotorStatus->ESC_Rear){
		if(__escRearTargetPower < MotorStatus->ESC_Rear)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)--MotorStatus->ESC_Rear) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;
		else if (__escRearTargetPower > MotorStatus->ESC_Rear)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)++MotorStatus->ESC_Rear) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;

		dutyCycle = dutyCycle * __pwmRangeScaler;

		TMR_PWM_OUT_PRIMARY->CCR4 = dutyCycle;

#ifdef PWM_REV1_HACK
		TMR_PWM_OUT_AUX->CCR4 = dutyCycle;
#endif
	}

	if(__escFrontTargetPower != MotorStatus->ESC_Front){
		if(__escFrontTargetPower < MotorStatus->ESC_Front)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)--MotorStatus->ESC_Front) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;
		else if (__escFrontTargetPower > MotorStatus->ESC_Front)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)++MotorStatus->ESC_Front) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;

		dutyCycle = dutyCycle * __pwmRangeScaler;

		TMR_PWM_OUT_PRIMARY->CCR1 = dutyCycle;
	}

	if(__escRightTargetPower != MotorStatus->ESC_Right){
		if(__escRightTargetPower < MotorStatus->ESC_Right)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)--MotorStatus->ESC_Right) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;
		else if (__escRightTargetPower > MotorStatus->ESC_Right)
			dutyCycle = ((PIDConstants->ThrottleSensitivity * (uint16_t)++MotorStatus->ESC_Right) / PWM_RANGE_INT_MATH_DENOMENATOR) + PWM_MinDutyCycle;

		dutyCycle = dutyCycle * __pwmRangeScaler;
		TMR_PWM_OUT_PRIMARY->CCR3 = dutyCycle;
	}
}
