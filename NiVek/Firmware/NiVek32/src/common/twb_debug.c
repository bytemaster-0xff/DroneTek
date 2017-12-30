/*
 * twb_debug.c
 *
 *  Created on: Oct 5, 2012
 *      Author: kevinw
 *
 *      USART1 - TX OUT -> Debug Console
 *      USART1 - RX IN  -> Input from GPS

 */


#include <stdlib.h>
#include "twb_debug.h"
#include "nivek_system.h"
#include "stdio.h"

#include "string.h"
#include "memory_mgmt.h"
#include "twb_timer.h"
#include "twb_strings.h"
//#include "usbd_conf.h"
//#include "usbd_cdc_core.h"
#include "misc.h"
#include "stm32f4xx_exti.h"


#define DEBUG_USART            USART1

#define DEBUG_USART_GPIO_PORT       GPIOA
#define DEBUG_USART_GPIO_SPEED      GPIO_Speed_50MHz
#define DEBUG_USART_TX_PIN          GPIO_Pin_9
#define DEBUG_USART_RX_PIN          GPIO_Pin_10
#define DEBUG_USART1_AF             GPIO_AF_0
#define DEBUG_USART_BAUD_RATE 		115200

bool __debugReady = false;
ForwardDebugMessages_e __debug_shouldForward;

//For now we use PC4 as a pin to toggle to measure algorithms

#define DEBUG_PULSE_PORT GPIOC
#define DEBUG_PULSE_PIN GPIO_Pin_11


void TWB_Debug_TimerStart(void) {
	GPIO_WriteBit(DEBUG_PULSE_PORT, DEBUG_PULSE_PIN,  Bit_SET);
}

void TWB_Debug_TimerEnd(void) {
	GPIO_WriteBit(DEBUG_PULSE_PORT, DEBUG_PULSE_PIN,  Bit_RESET);
}

uint8_t __lastDebugToggle = 0;

void TWB_Debug_Toggle(void){
	if(__lastDebugToggle)
		TWB_Debug_TimerStart();
	else
		TWB_Debug_TimerEnd();

	__lastDebugToggle = !__lastDebugToggle;
}

#define DEBUG_BUFFER_LEN 4096

uint16_t __dbg_BufferHead = 0;
uint16_t __dbg_BufferTail = 0;
char *__dbg_Buffer;//[DEBUG_BUFFER_LEN];

void TWB_Debug_HandleTXE(void){
	if(__dbg_BufferTail == __dbg_BufferHead)
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	else{
		USART_SendData(USART1, __dbg_Buffer[__dbg_BufferHead++]);
		if(__dbg_BufferHead == DEBUG_BUFFER_LEN)
			__dbg_BufferHead = 0;

		if(__dbg_BufferTail == __dbg_BufferHead)
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		else
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
}

iOpResult_e TWB_Debug_AddCh(char ch){
	char twoChs[2];
	twoChs[0] = ch;
	twoChs[1] = 0x00;
	return TWB_Debug_Print(twoChs);
}

iOpResult_e __debug_SendOverUSB(const char *strBuffer){
	/*char ch;
	while((ch = *strBuffer++) != '\0'){
		 USB_Debug_Buffer->Buffer[USB_Debug_Buffer->PtrIn++] = ch;

		 /* To avoid buffer overflow */
		/* if (USB_Debug_Buffer->PtrIn == USB_BUFFER_SIZE)
			 USB_Debug_Buffer->PtrIn = 0;
	  }*/

	   return OK;
}

void __debug_addCh(char ch){
	__dbg_Buffer[__dbg_BufferTail++] = ch;
	if(__dbg_BufferTail == DEBUG_BUFFER_LEN)
		__dbg_BufferTail = 0;

	if(__dbg_BufferTail == __dbg_BufferHead){
		__dbg_BufferTail = 0;
		__dbg_BufferHead = 0;
	}
}

iOpResult_e __debug_SendOverUART(const char *strBuffer){
	char ch;
	while((ch = *strBuffer++) != '\0')
		__debug_addCh(ch);

	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

	return OK;
}

iOpResult_e TWB_Debug_SetShouldForwardDebugOverUSB(ForwardDebugMessages_e debug_shouldForward){
	__debug_shouldForward = debug_shouldForward;

	return TWB_Sys_Set_ShouldForwardDebugToConsole(__debug_shouldForward);
}

iOpResult_e TWB_Debug_Print(const char *strBuffer){
	if(!__debugReady)
		return OK;

	if(strBuffer[0] == 0x00) //Nothing to do.
		return OK;

	//CLR_Debug::Printf(strBuffer);

/*	if(__debug_shouldForward == ForwardDebugMessages_On)
		__debug_SendOverUSB(strBuffer);*/

	__debug_SendOverUART(strBuffer);

	return OK;
}

uint8_t __debug_Temp_IntBuffer[10];

//Yeah, I know that's what printf is for, but using it adds about 20K to my code
//base. Wrong answer!  KDW 12/2/2012
iOpResult_e TWB_Debug_PrintInt(const char *strBuffer, int value) {
	TWB_Debug_Print(strBuffer);
	twb_itoa(value, __debug_Temp_IntBuffer, 10);
	TWB_Debug_Print((const char *)__debug_Temp_IntBuffer);
	TWB_Debug_Print("\r\n");

	return OK;
}

iOpResult_e TWB_Debug_Print2Int(const char *strBuffer, int value1, int value2) {
	TWB_Debug_Print(strBuffer);
	twb_itoa(value1, __debug_Temp_IntBuffer, 10);
	TWB_Debug_Print((const char *)__debug_Temp_IntBuffer);
	TWB_Debug_Print(", ");
	twb_itoa(value2, __debug_Temp_IntBuffer, 10);
	TWB_Debug_Print((const char *)__debug_Temp_IntBuffer);

	TWB_Debug_Print("\r\n");

	return OK;
}

char *__hexLookup = "0123456789ABCDEF";

iOpResult_e TWB_Debug_PrintHexString(uint8_t byte) {
	__debug_Temp_IntBuffer[0] = __hexLookup[(byte >> 4)];
	__debug_Temp_IntBuffer[1] = __hexLookup[(byte & 0x0F)];
	__debug_Temp_IntBuffer[2] = 0x00;
	TWB_Debug_Print((const char *)__debug_Temp_IntBuffer);

	return OK;
}

iOpResult_e TWB_Debug_PrintHexNumb(unsigned long numb){
	TWB_Debug_AddCh('0');
	TWB_Debug_AddCh('x');

	TWB_Debug_PrintHexString((numb >> (3 * 8)) & 0xFF);
	TWB_Debug_PrintHexString((numb >> (2 * 8)) & 0xFF);
	TWB_Debug_PrintHexString((numb >> (1 * 8)) & 0xFF);
	TWB_Debug_PrintHexString(numb & 0xFF);

	return OK;
}

void __twb_debug_setBaudRate(uint32_t baud){
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
}

void __twb_debug_sendStringSync(const char * str){
	while(*str){
		USART1->DR = *str;
		while((USART1->SR & USART_SR_TXE) != USART_SR_TXE) {};
		str++;
	}
}

iOpResult_e TWB_Debug_Init(void) {
	NVIC_InitTypeDef nvicInit;

	__dbg_Buffer = pm_malloc((size_t)DEBUG_BUFFER_LEN);
	if(__dbg_Buffer == NULL)
		return OutOfMemory;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//For now we are tapping the TRG pin on the sonar to show us clocks.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//The GPS and debug service share the same port.
	//Need to start out at 9600 then send command to change baud to 115200 synchronously.
	//then change the baud to 115200
	__twb_debug_setBaudRate(9600);
	/* Enable USART */

	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

	nvicInit.NVIC_IRQChannel = USART1_IRQn;

	nvicInit.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);

	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

	USART_Cmd(USART1, ENABLE);

	__twb_debug_sendStringSync("$PMTK251,115200*1F\r\n");

	/* Sleep an extra 20MS to make sure everything was sent properly */
	TWB_Timer_SleepMS(20);

	/* Send out these chars sync */
	__twb_debug_setBaudRate(115200);

#ifdef DEBUG_SIGNAL_PIN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

	__debugReady = true;

	return OK;
}

iOpResult_e TWB_Debug_ReadDefaults(void){
	return TWB_Sys_Get_ShouldForwardDebugToConsole(&__debug_shouldForward);
}
