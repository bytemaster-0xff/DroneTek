/*
 * rc_in.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */
#include <math.h>

#include "flight/pwm_capture.h"
#include "flight/ctrl_loop.h"

#include "twb_eeprom.h"
#include "twb_config.h"

#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "common/twb_leds.h"

#include "commo/commo.h"

PWMIn_t *PWMIn;

//Counts when an edge went either high or low.
uint16_t __rcInRise[6];
uint16_t __rcInFall[6];



iOpResult_e __pwm_capture_readCalibration(void);

#define RC_IN_COUNT_MAX 40000

iOpResult_e __pwm_capture_initTimeBase(void){
	RCC_APB1PeriphClockCmd(TMR_PWM_CAPTURE_PERIPH , ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 30; //Gives us 0.025ms per tick
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = RC_IN_COUNT_MAX;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TMR_PWM_CAPTURE, &TIM_TimeBaseStructure);

	return OK;
}

iOpResult_e __pwm_capture_initPorts(void){
	GPIO_InitTypeDef pwmCaptureConfig;
	/*Enable input pins for PWM Capture, Port B - 4,5,6,7 & Port C - 11, 12 */
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE);

	pwmCaptureConfig.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	//pwmCaptureConfig.GPIO_Pin = GPIO_Pin_5;
	pwmCaptureConfig.GPIO_Speed = GPIO_Speed_50MHz;
	pwmCaptureConfig.GPIO_Mode = GPIO_Mode_IN;
	pwmCaptureConfig.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &pwmCaptureConfig);

#ifndef DEBUG_SIGNAL_PIN
    //Pins are re-purposed to toggle with TWB_Debug_StartTimer
    pwmCaptureConfig.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOC, &pwmCaptureConfig);
#endif

    /*Enable sys config clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Connect the ports to the EXTI0 Line */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource6);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);

#ifndef DEBUG_SIGNAL_PIN
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource11);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource12);
#endif

    return OK;
}

TIM_TypeDef *__pwmTimer;

iOpResult_e TWB_PWM_Capture_Init(void){
	assert_succeed(__pwm_capture_initPorts());
    assert_succeed(__pwm_capture_readCalibration());
    assert_succeed(__pwm_capture_initTimeBase());

    PWMIn->Pitch = 0;
    PWMIn->Roll = 0;
    PWMIn->Yaw = 0;
    PWMIn->Throttle = 0;
    PWMIn->Aux1 = 0;
    PWMIn->Aux2 = 0;

    for(uint8_t idx = 0; idx < 6; ++idx){
    	__rcInFall[idx] = 0;
    	__rcInRise[idx] = 0;
    }

	WiFiPitch = 0;
	WiFiRoll = 0;
	WiFiYaw = 0;
	WiFiThrottle = 0;

	__pwmTimer = TMR_PWM_CAPTURE;

	TIM_Cmd(TMR_PWM_CAPTURE, ENABLE);

	TWB_Debug_Print("PWM Capture: OK\r\n");

	return OK;
}

uint16_t __pwmCapture_processRadioChannel(uint8_t port);
int8_t __pwmCapture_getStickPosition(uint16_t current, uint16_t min, uint16_t idle, uint16_t max);

void TWB_PWM_RCWatchDogExpire(void){
	TWB_LEDS_SetState(LED_RC_Connected, LED_Off);
}

void TWB_PWM_ProcessRCIn(void){
	if(__rcInFall[PWM_In_Pitch] > 0){
		if(__rcInRise[PWM_In_Pitch] > 0)
			PWMIn->Pitch = __pwmCapture_getStickPosition(__pwmCapture_processRadioChannel(PWM_In_Pitch), PWMIn->CalMinPitch, PWMIn->CalIdlePitch, PWMIn->CalMaxPitch);
		else
			__rcInFall[PWM_In_Pitch] = 0;

		TWB_Timer_Reset(RCWatchDog);
		TWB_LEDS_SetState(LED_RC_Connected, LED_On);
	}

	if(__rcInFall[PWM_In_Roll] > 0){
		if(__rcInRise[PWM_In_Roll] > 0)
			PWMIn->Roll = __pwmCapture_getStickPosition(__pwmCapture_processRadioChannel(PWM_In_Roll), PWMIn->CalMinRoll, PWMIn->CalIdleRoll, PWMIn->CalMaxRoll);
		else
			__rcInFall[PWM_In_Roll] = 0;

		TWB_Timer_Reset(RCWatchDog);
		TWB_LEDS_SetState(LED_RC_Connected, LED_On);
	}

	if( __rcInFall[PWM_In_Yaw] > 0){
		if(__rcInRise[PWM_In_Yaw] > 0)
			PWMIn->Yaw =  __pwmCapture_getStickPosition(__pwmCapture_processRadioChannel(PWM_In_Yaw), PWMIn->CalMinYaw, PWMIn->CalIdleYaw, PWMIn->CalMaxYaw);
		else
			__rcInFall[PWM_In_Yaw] = 0;

		TWB_Timer_Reset(RCWatchDog);
		TWB_LEDS_SetState(LED_RC_Connected, LED_On);
	}

	if(__rcInFall[PWM_In_Throttle] > 0){
		if(__rcInRise[PWM_In_Throttle] > 0){
			int16_t delta = __pwmCapture_processRadioChannel(PWM_In_Throttle) - PWMIn->CalMinThrottle;
			if(delta < 5)
				PWMIn->Throttle = 0;
			else{
				int16_t range = PWMIn->CalMaxThrottle - PWMIn->CalMinThrottle;
				if(range == 0)
					PWMIn->Throttle = 0;
				else if(delta > range)
					PWMIn->Throttle = 255;
				else
					PWMIn->Throttle = (uint8_t)((delta * 255) / range);
			}

			__rcInFall[PWM_In_Throttle] = 0;
			__rcInRise[PWM_In_Throttle] = 0;
		}
		else
			__rcInFall[PWM_In_Throttle] = 0;

		TWB_Timer_Reset(RCWatchDog);
		TWB_LEDS_SetState(LED_RC_Connected, LED_On);
	}

	if(__rcInRise[PWM_In_Aux1] > 0 && __rcInFall[PWM_In_Aux1] > 0)
		PWMIn->Aux1 = __pwmCapture_processRadioChannel(PWM_In_Aux1);

	if(__rcInRise[PWM_In_Aux2] > 0 && __rcInFall[PWM_In_Aux2] > 0)
		PWMIn->Aux2 = __pwmCapture_processRadioChannel(PWM_In_Aux2);
}

iOpResult_e TWB_RCIN_BeginCalibration(void){
	SystemStatus->GPIOState = GPIOAppState_RCCalibration;
	TWB_Timer_Enable(SendPWMInData);

	TWB_Debug_Print("Begin PWM In Calibration\r\n");

	return OK;
}

void TWB_RCIN_QueueMsg(void){
	TWB_Commo_SendMessage(MOD_System, CMD_PWMInData, (uint8_t *)PWMIn, sizeof(PWMIn_t));
}

iOpResult_e TWB_RCIN_CancelCalibration(void){
	__pwm_capture_readCalibration();
	TWB_Timer_Disable(SendPWMInData);
	SystemStatus->GPIOState = GPIOAppState_Idle;

	TWB_Debug_Print("Canceled PWM In Calibration\r\n");

	return OK;
}

//TODO: Might be nice to validate calibration here.
iOpResult_e TWB_RCIN_FinalizeCalibration(void){
	TWB_Timer_Disable(SendPWMInData);
	SystemStatus->GPIOState = GPIOAppState_Idle;

	PWMIn->IsCalibrated = Calibrated;
	assert_succeed(TWB_SE_WriteU8(PWM_IN_CALIBRATION + 0, PWMIn->IsCalibrated));

	TWB_Commo_SendMessage(MOD_System, CMD_CalibrationCompleted, (uint8_t *)PWMIn, sizeof(PWMIn_t));

	TWB_Debug_Print("Done PWM In Calibration\r\n");

	return OK;
}


uint16_t __pwmCapture_processRadioChannel(uint8_t port){
	uint16_t value = (__rcInFall[port] < __rcInRise[port]) ?  (RC_IN_COUNT_MAX + __rcInFall[port]) - __rcInRise[port] : __rcInFall[port] - __rcInRise[port];

	if(value > 0){
		switch(port){
		case PWM_In_Pitch: PWMIn->Radio1Raw = value; break;
		case PWM_In_Roll: PWMIn->Radio2Raw = value; break;
		case PWM_In_Yaw: PWMIn->Radio3Raw = value; break;
		case PWM_In_Throttle: PWMIn->Radio4Raw = value; break;
		case PWM_In_Aux1: PWMIn->Radio5Raw = value; break;
		case PWM_In_Aux2: PWMIn->Radio6Raw = value; break;
		}
	}

	__rcInRise[port] = 0;
	__rcInFall[port] = 0;

	return value;
}

int8_t __pwmCapture_getStickPosition(uint16_t current, uint16_t min, uint16_t idle, uint16_t max){
	int16_t delta = (int16_t)(current - idle);
	if(delta == 0)
		return 0;

	if(delta < idle){
		int16_t range = idle - min;
		if(range == 0)
			return 0;

		return (int8_t) ((delta * 90) / range);
	}

	int16_t range = max - idle;
	if(range == 0)
		return 0;

	return -(int8_t) ((delta * 90) / range);
}

iOpResult_e __pwm_capture_readCalibration(void){
    //Grab calibration values from persistent storage.
	assert_succeed(TWB_SE_ReadU8(PWM_IN_CALIBRATION, &PWMIn->IsCalibrated));

	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 2, &PWMIn->CalMinThrottle));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 4, &PWMIn->CalMaxThrottle));

	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 6, &PWMIn->CalMinYaw));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 8, &PWMIn->CalIdleYaw));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 10, &PWMIn->CalMaxYaw));

	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 12, &PWMIn->CalMinPitch));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 14, &PWMIn->CalIdlePitch));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 16, &PWMIn->CalMaxPitch));

	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 18, &PWMIn->CalMinRoll));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 20, &PWMIn->CalIdleRoll));
	assert_succeed(TWB_SE_ReadU16(PWM_IN_CALIBRATION + 22, &PWMIn->CalMaxRoll));

	return OK;
}

iOpResult_e TWB_RCIN_CalibrateStick(uint8_t setting) {
	uint16_t counts = 0;
	switch(setting){
		case RC_IN_ThrottleMin: //Min
			PWMIn->CalMinThrottle = PWMIn->Radio4Raw;
			counts = PWMIn->CalMinThrottle;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 2, PWMIn->CalMinThrottle));

			break;
		case RC_IN_ThrottleMax: //Max
			PWMIn->CalMaxThrottle = PWMIn->Radio4Raw;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 4, PWMIn->CalMaxThrottle));
			counts = PWMIn->CalMaxThrottle;
			break;

		/*---------------------------*/
		case RC_IN_Yaw_Port: // Min
			PWMIn->CalMinYaw = PWMIn->Radio3Raw;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 6, PWMIn->CalMinYaw));
			counts = PWMIn->CalMinYaw;
			break;
		case RC_IN_Yaw_Idle: // 0
			PWMIn->CalIdleYaw = PWMIn->Radio3Raw;
			counts = PWMIn->CalIdleYaw;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 8, PWMIn->CalIdleYaw));
			break;
		case RC_IN_Yaw_Starboard: // Max
			PWMIn->CalMaxYaw = PWMIn->Radio3Raw;
			counts = PWMIn->CalMaxYaw;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 10, PWMIn->CalMaxYaw));
			break;

		/*---------------------------*/
		case RC_IN_Pitch_Up: // Min
			PWMIn->CalMinPitch = PWMIn->Radio1Raw;
			counts = PWMIn->CalMinPitch;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 12, PWMIn->CalMinPitch));

			break;
		case RC_IN_Pitch_Idle: // 0
			PWMIn->CalIdlePitch = PWMIn->Radio1Raw;
			counts = PWMIn->CalIdlePitch;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 14, PWMIn->CalIdlePitch));
			break;
		case RC_IN_Pitch_Down: //Max
			PWMIn->CalMaxPitch = PWMIn->Radio1Raw;
			counts = PWMIn->CalMaxPitch;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 16, PWMIn->CalMaxPitch));
			break;

		/*---------------------------*/
		case RC_IN_Roll_Left: // Min
			PWMIn->CalMinRoll = PWMIn->Radio2Raw;
			counts = PWMIn->CalMinRoll;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 18, PWMIn->CalMinRoll));
			break;
		case RC_IN_Roll_Idle: // 0
			PWMIn->CalIdleRoll = PWMIn->Radio2Raw;
			counts = PWMIn->CalIdleRoll;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 20, PWMIn->CalIdleRoll));
			break;
		case RC_IN_Roll_Right: // Max
			PWMIn->CalMaxRoll = PWMIn->Radio2Raw;
			counts = PWMIn->CalMaxRoll;
			assert_succeed(TWB_SE_WriteU16(PWM_IN_CALIBRATION + 22, PWMIn->CalMaxRoll));
			break;
	}

	uint8_t out[3];
	out[0] = setting;
	out[1] = counts >> 8;
	out[2] = counts & 0xFF;

	TWB_Commo_SendMessage(MOD_FlightControls, CMD_RCChannel, out, 3);

	return OK;
}
