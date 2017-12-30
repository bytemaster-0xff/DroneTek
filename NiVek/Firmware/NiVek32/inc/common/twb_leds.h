#ifndef __TWB_LEDS_H
#define __TWB_LEDS_H

#include "twb_common.h"


typedef enum {
	LED_WiFi_Connected = 0,
	LED_RC_Connected = 1,
	LED_Armed = 2,
	LED_Snsr_Fault = 3,
	LED_Snsr_Online = 4,
} SystemLedNames_t;

#define LED_COUNT 5

typedef enum {
	LED_On = 0,
	LED_Off = 1,
	LED_FastBlink = 2, //10Hz
	LED_SlowBlink = 3, //2Hz
} SystemLedStates_t;

typedef struct{
	SystemLedNames_t Led;
	SystemLedStates_t State;
	GPIO_TypeDef* Port;
	uint16_t Pin;

} SystemLeds_t;

iOpResult_e TWB_LEDS_Init(void);

void TWB_LEDS_SetState(SystemLedNames_t led, SystemLedStates_t state);
void TWB_LEDS_UpdateFastBlink(void);
void TWB_LEDS_UpdateSlowBlink(void);

#endif


