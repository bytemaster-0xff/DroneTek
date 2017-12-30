
#include "common/twb_debug.h"
#include "common/twb_leds.h"

SystemLeds_t LEDS[LED_COUNT];

iOpResult_e TWB_LEDS_Init(void){
	uint8_t idx = 0;

	LEDS[LED_WiFi_Connected].Led = LED_WiFi_Connected;
	LEDS[LED_WiFi_Connected].Pin = GPIO_Pin_15;
	LEDS[LED_WiFi_Connected].Port = GPIOA;
	LEDS[LED_WiFi_Connected].State = LED_Off;

	LEDS[LED_RC_Connected].Led = LED_RC_Connected;
	LEDS[LED_RC_Connected].Pin = GPIO_Pin_12;
	LEDS[LED_RC_Connected].Port = GPIOB;
	LEDS[LED_RC_Connected].State = LED_Off;

	LEDS[LED_Armed].Led = LED_Armed;
	LEDS[LED_Armed].Pin = GPIO_Pin_15;
	LEDS[LED_Armed].Port = GPIOB;
	LEDS[LED_Armed].State = LED_Off;

	LEDS[LED_Snsr_Fault].Led = LED_Snsr_Fault;
	LEDS[LED_Snsr_Fault].Pin = GPIO_Pin_14;
	LEDS[LED_Snsr_Fault].Port = GPIOC;
	LEDS[LED_Snsr_Fault].State = LED_Off;

	LEDS[LED_Snsr_Online].Led = LED_Snsr_Online;
	LEDS[LED_Snsr_Online].Pin = GPIO_Pin_13;
	LEDS[LED_Snsr_Online].Port = GPIOC;
	LEDS[LED_Snsr_Online].State = LED_Off;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	for(idx = 0; idx < LED_COUNT; ++idx){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

		GPIO_InitTypeDef pwmPinConfig;
		pwmPinConfig.GPIO_Pin = LEDS[idx].Pin;
		pwmPinConfig.GPIO_Mode = GPIO_Mode_OUT;
		pwmPinConfig.GPIO_Speed = GPIO_Speed_50MHz;
		pwmPinConfig.GPIO_OType = GPIO_OType_PP;
		pwmPinConfig.GPIO_PuPd = GPIO_PuPd_UP ;
		GPIO_Init(LEDS[idx].Port, &pwmPinConfig);
	}

	for(idx = 0; idx < LED_COUNT; ++idx)
		GPIO_WriteBit(LEDS[idx].Port, LEDS[idx].Pin,  Bit_SET);

	TWB_Debug_Print("LED Manager: OK\r\n");

	return OK;

}

void TWB_LEDS_UpdateFastBlink(void){
	for(uint8_t idx = 0; idx < LED_COUNT; ++idx){
		BitAction currentState =(LEDS[idx].Port->ODR & LEDS[idx].Pin) == LEDS[idx].Pin;
		if(LEDS[idx].State == LED_FastBlink)
			GPIO_WriteBit(LEDS[idx].Port, LEDS[idx].Pin,  !currentState);
	}
}

void TWB_LEDS_UpdateSlowBlink(void){
	for(uint8_t idx = 0; idx < LED_COUNT; ++idx){
		BitAction currentState =(LEDS[idx].Port->ODR & LEDS[idx].Pin) == LEDS[idx].Pin;
		if(LEDS[idx].State == LED_SlowBlink)
			GPIO_WriteBit(LEDS[idx].Port, LEDS[idx].Pin,  !currentState);
	}
}

void TWB_LEDS_SetState(SystemLedNames_t led, SystemLedStates_t state){
	if(LEDS[led].State != state){
		LEDS[led].State = state;
		GPIO_WriteBit(LEDS[led].Port, LEDS[led].Pin,  state != LED_Off ? Bit_RESET : Bit_SET);
	}
}
