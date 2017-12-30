#include "stdio.h"
#include "common/twb_common.h"

#include "sensors/gps.h"
#include "sensors/snsr_main.h"
#include "sensors/bmp085.h"
#include "system/cli.h"
#include "commo/msg_parser.h"
#include "commo/wifi.h"

#include "common/twb_debug.h"
#include "common/twb_timer.h"

#include "flight/pwm_capture.h"

/* Handle the really important stuff */
void NMI_Handler(void){
	 if (RCC_GetITStatus(RCC_IT_CSS) != RESET)    {
		 /* At this stage: HSE, PLL are disabled (but no change on PLL config) and HSI        is selected as system clock source */
		 /* Enable HSE */
		 RCC_HSEConfig( RCC_HSE_ON);
		 /* Enable HSE Ready interrupt */
		 RCC_ITConfig(RCC_IT_HSERDY, ENABLE);
		 /* Enable PLL Ready interrupt */
		 RCC_ITConfig(RCC_IT_PLLRDY, ENABLE);        /* Clear Clock Security System interrupt pending bit */
		 RCC_ClearITPendingBit( RCC_IT_CSS);
	 }
}


extern char *__hexLookup;

void __sendChSync(const char ch){
	USART_SendData(USART1, ch);
	while(!(USART1->SR & 0x00000040));
}

void __printHexByteSync(uint8_t byte) {
	__sendChSync(__hexLookup[(byte >> 4)]);
	__sendChSync(__hexLookup[(byte & 0x0F)]);
}

void __printLongSync(unsigned long numb){
	__sendChSync('0');
	__sendChSync('x');

	__printHexByteSync((numb >> (3 * 8)) & 0xFF);
	__printHexByteSync((numb >> (2 * 8)) & 0xFF);
	__printHexByteSync((numb >> (1 * 8)) & 0xFF);
	__printHexByteSync(numb & 0xFF);
}

void __printStrSync(const char * str){
	char ch;
	while((ch = *str++) != '\0')
		__sendChSync(ch);
}

void __printRegister(const char *reg, unsigned long addr){
	  __printStrSync(reg);
	  __printStrSync(": ");
	  __printLongSync(addr);
	  __printStrSync("\r\n");
}

unsigned int __hf_stacked_pc;

//TODO: Should either reset or start flashing an LED.
void hard_fault_handler_c (unsigned int *hardfault_args){
	  USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	  __printStrSync("\r\nHARD FAULT\r\n");
	  __printStrSync("==================\r\n");

	  __printRegister("ro ", (unsigned long) hardfault_args[0]);
	  __printRegister("r1 ", (unsigned long) hardfault_args[1]);
	  __printRegister("r2 ", (unsigned long) hardfault_args[2]);
	  __printRegister("r3 ", (unsigned long) hardfault_args[3]);
	  __printRegister("r12", (unsigned long) hardfault_args[4]);

	  __printRegister("lr ", (unsigned long) hardfault_args[5]);
	  __printRegister("pc ", (unsigned long) hardfault_args[6]);
	  __printRegister("psr", (unsigned long) hardfault_args[7]);

	  __printStrSync("==================\r\n");


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef pwmPinConfig;
	pwmPinConfig.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_13;
	pwmPinConfig.GPIO_Mode = GPIO_Mode_OUT;
	pwmPinConfig.GPIO_Speed = GPIO_Speed_50MHz;
	pwmPinConfig.GPIO_OType = GPIO_OType_PP;
	pwmPinConfig.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOC, &pwmPinConfig);

	GPIO_WriteBit(GPIOC, GPIO_Pin_13,  SET);

	volatile uint32_t idx;

	while(1){
		idx = 168000 * 10;
		GPIO_WriteBit(GPIOC, GPIO_Pin_14,  SET);
    	while (idx-- > 0);

    	idx = 168000 * 10;
		GPIO_WriteBit(GPIOC, GPIO_Pin_14,  RESET);
		while (idx-- > 0);
	}
}

uint32_t isr;

//USART1 - Connected to debug port and GPS
void USART1_IRQHandler(void){
	if((USART1->SR & USART_SR_RXNE) == USART_SR_RXNE){
		TWB_GPS_HandleRX_IRQ(USART1->DR);
	}
	else if((USART1->SR & USART_SR_ORE) == USART_SR_ORE){
		TWB_GPS_HandleRX_Overrun();
	}
	else
		if((USART1->SR & USART_SR_TXE) == USART_SR_TXE)
			TWB_Debug_HandleTXE();

	//For now we just stream out any debug info on USART1,
}

//TODO: Currently we have no command mode with the UART device.
//USART2 - Connected to XBee Soccket.
void USART2_IRQHandler(void){
	//There will be about a 500 millisecond delay after we switch
	//from to the command mode until it becomes active.
	//The only time we will send messages will be in the Ready state
	//therefore no new messages will come in while in the PendingCommandMode state
	//but if a message is in process, we want to continue sending it.
	//
	//While in the command mode, ignore any incoming messages from the WiFly

	isr =USART2->SR;

	if(WiFiState == WiFi_CLIMode){
		if((isr & USART_SR_RXNE) == USART_SR_RXNE)
			TWB_CLI_HandleRX(USART2->DR);
		else if((isr & USART_SR_ORE) == USART_SR_ORE)
			TWB_CLI_HandleRX_Overrun();
		else if((isr & USART_SR_TXE) == USART_SR_TXE)
			TWB_CLI_HandleTXE();
	}
	else if(WiFiState == WiFi_Ready ||
	   WiFiState == WiFi_PendingCommandMode){

		if((isr & USART_SR_RXNE) == USART_SR_RXNE)
			TWB_MsgParser_HandleRX(USART2->DR);
		else if((isr & USART_SR_ORE) == USART_SR_ORE)
			TWB_MsgParser_HandleRX_Overrun();
		else if((isr & USART_SR_TXE) == USART_SR_TXE)
			TWB_MsgParser_HandleTXE();

	}
	else{
		if((isr & USART_SR_RXNE) == USART_SR_RXNE)
			TWB_WiFi_HandleRX(USART2->DR);
		else if((isr & USART_SR_ORE) == USART_SR_ORE)
			TWB_MsgParser_HandleRX_Overrun();
		else if((isr & USART_SR_TXE) == USART_SR_TXE)
			TWB_WiFi_HandleTXE();
	}
}

/* For Rev II boards, here are the associated IRQ's we care about
 *
 * Pin 1 - AVAILABLE
 * Pin 2, Port B, Sonar Echo
 * Pin 3, Port C, BMP085
 * Pin 4, Port B Pitch/RC1
 * Pin 5, Port B Roll/RC2
 * Pin 6, Port B Yaw/RC3
 * Pin 7, Port B Throttle/RC4
 * Pin 8 - AVAILABLE
 * Pin 9 Port B ITG3200
 * Pin 10, Port C L3GD20 IRQ
 * Pin 11, Port C Aux1
 * Pin 12, Port C Aux2
 * Pin 13, Available
 */
void EXTI0_IRQHandler(void){

}

void EXTI1_IRQHandler(void){

}

void EXTI4_IRQHandler(void){
	volatile uint16_t thisCount = (uint16_t)TMR_PWM_CAPTURE->CNT;

	if((GPIOB->IDR & GPIO_Pin_4) == GPIO_Pin_4)
		__rcInRise[PWM_In_Pitch] = thisCount;
	else
		__rcInFall[PWM_In_Pitch] = thisCount;

	EXTI_ClearITPendingBit(EXTI_Line4);
}

uint16_t thisCount;

void EXTI9_5_IRQHandler(void){
	 thisCount = (uint16_t)TMR_PWM_CAPTURE->CNT;

	if(((EXTI->PR & EXTI_Line5) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_Line5) != (uint32_t)RESET)){
		if((GPIOB->IDR & GPIO_Pin_5) == GPIO_Pin_5)
			__rcInRise[PWM_In_Roll] = thisCount;
		else
			__rcInFall[PWM_In_Roll] = thisCount;

		EXTI_ClearITPendingBit(EXTI_Line5);
	}

	if(((EXTI->PR & EXTI_Line6) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_Line6) != (uint32_t)RESET)){
		if((GPIOB->IDR & GPIO_Pin_6) == GPIO_Pin_6)
			__rcInRise[PWM_In_Yaw] = thisCount;
		else
			__rcInFall[PWM_In_Yaw] = thisCount;

		EXTI_ClearITPendingBit(EXTI_Line6);
	}

	if(((EXTI->PR & EXTI_Line7) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_Line7) != (uint32_t)RESET)){
		if((GPIOB->IDR & GPIO_Pin_7) == GPIO_Pin_7)
			__rcInRise[PWM_In_Throttle] = thisCount;
		else
			__rcInFall[PWM_In_Throttle] = thisCount;

		EXTI_ClearITPendingBit(EXTI_Line7);
	}

	if(((EXTI->PR & EXTI_Line8) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_Line8) != (uint32_t)RESET)){
		if((GPIOA->IDR & GPIO_Pin_8) == 0x00){
			if(GyroITG3200Sensor->IsEnabled)
				GyroITG3200Sensor->DataReady = DataReady;

			if(GyroMPU60x0Sensor->IsEnabled)
				GyroMPU60x0Sensor->DataReady = DataReady;
		}

		EXTI_ClearITPendingBit(EXTI_Line8);
	}

	if(((EXTI->PR & EXTI_Line9) != (uint32_t)RESET) && ((EXTI->IMR & EXTI_Line9) != (uint32_t)RESET)){

		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}


void EXTI15_10_IRQHandler(void){
	volatile uint16_t thisCount = (uint16_t)TMR_PWM_CAPTURE->CNT;

	if(((EXTI->PR & EXTI_Line10) != (uint32_t)RESET) &&	((EXTI->IMR & EXTI_Line10) != (uint32_t)RESET)){
		if((GPIOC->IDR & GPIO_Pin_10) == 0x00)
			GyroL3GD20Sensor->DataReady = DataReady;

		EXTI_ClearITPendingBit(EXTI_Line10);
	}

	if(((EXTI->PR & EXTI_Line11) != (uint32_t)RESET) &&	((EXTI->IMR & EXTI_Line11) != (uint32_t)RESET)){
		if((GPIOC->IDR & GPIO_Pin_11) == GPIO_Pin_11)
			__rcInRise[PWM_In_Aux1] = thisCount;
		else
			__rcInFall[PWM_In_Aux1] = thisCount;

		EXTI_ClearITPendingBit(EXTI_Line11);
	}

	if(((EXTI->PR & EXTI_Line12) != (uint32_t)RESET) &&	((EXTI->IMR & EXTI_Line12) != (uint32_t)RESET)){
		if((GPIOC->IDR & GPIO_Pin_12) == GPIO_Pin_11)
			__rcInRise[PWM_In_Aux2] = thisCount;
		else
			__rcInFall[PWM_In_Aux2] = thisCount;

		EXTI_ClearITPendingBit(EXTI_Line12);
	}

	if(((EXTI->PR & EXTI_Line13) != (uint32_t)RESET) &&	((EXTI->IMR & EXTI_Line13) != (uint32_t)RESET)){
		if((GPIOB->IDR & GPIO_Pin_13) == 0x00)
			AccLSM303Sensor->DataReady = DataReady;

		EXTI_ClearITPendingBit(EXTI_Line13);
	}

	if(((EXTI->PR & EXTI_Line14) != (uint32_t)RESET) &&	((EXTI->IMR & EXTI_Line14) != (uint32_t)RESET)){
		if((GPIOB->IDR & GPIO_Pin_14) == 0x00)
			AccADXL345Sensor->DataReady = DataReady;

		EXTI_ClearITPendingBit(EXTI_Line14);
	}
}

void RCC_IRQHandler(void){
	if(RCC_GetITStatus(RCC_IT_HSERDY) != RESET){
		RCC_ClearITPendingBit(RCC_IT_HSERDY);

    if (RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET){
    		RCC_PLLCmd(ENABLE);
    	}
	}

	if(RCC_GetITStatus(RCC_IT_PLLRDY) != RESET){
		RCC_ClearITPendingBit(RCC_IT_PLLRDY);

		if (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != RESET){
			RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		}
	}
}

/*
//#ifdef USE_USB_OTG_FS
void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}*/
//#endif

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS
void OTG_HS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line20);
}
#endif

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}
#endif


void SVC_Handler(void) {}
void PendSV_Handler(void) {}
