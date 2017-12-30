/*
 * NetMFInit.c
 *
 *  Created on: Sep 8, 2013
 *      Author: Kevin
 */


#include "NetMFInit.h"

extern "C" {
#include "common\twb_timer.h"
#include "common\twb_leds.h"
#include "common\twb_i2c.h"
#include "common\memory_mgmt.h"
#include "stm32f4xx_gpio.h"
#include "main.h"

uint32_t SystemCoreClock = 168000000;

void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void SysTick_Handler(void);
void TIM7_IRQHandler(void);

void TWB_CLI_Init() {};
void TWB_USB_Init() {};

}

extern uint32_t ARM_Vectors[84];  // the interrupt vector table



iOpResult_e __twb_bindISR(uint32_t Irq_Index, void ISR()){
	ARM_Vectors[Irq_Index + 16] = (uint32_t)ISR; // exception = irq + 16
    return OK;
}

iOpResult_e __twb_initISRs(void){
	__twb_bindISR(I2C1_EV_IRQn, I2C1_EV_IRQHandler);
	__twb_bindISR(I2C1_ER_IRQn, I2C1_ER_IRQHandler);
	__twb_bindISR(SysTick_IRQn, SysTick_Handler);
	__twb_bindISR(TIM7_IRQn, TIM7_IRQHandler);

    __DMB(); // asure table is written

    return OK;
}

iOpResult_e TWB_NetMFInit(void) {
	__twb_initISRs();

	TWB_LEDS_Init();
	TWB_LEDS_SetState(LED_Snsr_Fault, LED_On);
	TWB_LEDS_SetState(LED_Snsr_Fault, LED_FastBlink);

	if (SysTick_Config(SystemCoreClock / 1000)){
		while(true){
			uint32_t idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  Bit_SET);
			while (idx-- > 0);

			idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  Bit_RESET);
			while (idx-- > 0);
		}
	}

	__twb_main_AllocateMemory();

	SystemStatus->SystemState = SystemStartingUp;
	/*
	 * Without setting this all IRQ pre-emption is done via
	 * sub priorities.  Setting this to four allows for exclusive
	 * use of PreemptionPiority for the IRQ's.
	 */


	if(__initMain() != OK){
		while(true){
			uint32_t idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  Bit_SET);
			while (idx-- > 0);

			idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  Bit_RESET);
			while (idx-- > 0);
		}
	}

	TWB_LEDS_SetState(LED_Snsr_Fault, LED_Off);

	pm_printHeapSize();

	/* Everything is ready to go at this point
	 * So start the main loop
	 */
	__startMainLoop();



	return OK;
}
