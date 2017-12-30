#include "commo/wifi.h"

#include "commo/msg_parser.h"
#include "common/twb_timer.h"
#include "common/twb_debug.h"

#include "common/twb_common.h"
#include "nivek_system.h"

#include <stdio.h>
#include "string.h"

/*
 * WiFi is really whatever devices is being driven by the onboard serial port
 *
 * For using the 3DR Radio Modem, the following are used:
 *
 * From ATI5
 *
 * SO: Format = 25
 * S1: Serial Speed = 57
 * S2: Air Speed = 64
 * S3: NetID = 67
 * S4: TXPower = 20
 * S5: ECC = 1
 * S6: MACLINK = 1
 * S7: OPPRESEND = 1
 * S8: Min Freq = 915000
 * S9: Max Freq = 928000
 * S10: Num Channels = 50
 * S11: Duty Cycle = 100
 * S12: LBT_RSSI = 0
 * S13: Manchester = 0
 * S14: RTSCTS = 0

 *
 */

WiFiStates_t WiFiState;

const char *RebootSequence[] = {"$$$","\r","reboot\r"};
const uint8_t RebootLength = 3;

const char *EnableAPMode[] = {"$$$","\r","set wlan join 7\r","set wlan ssid NiVek"};
const uint8_t EnableAPModeLength = 5;

const char *CloseSequence[] = {"$$$","\r","close\r","exit\r"};
const uint8_t CloseLength = 3;

char __wiFiCommandBuffer[16];
uint8_t __wiFiCommandLength;
uint8_t __wiFiCommandIdx;
uint8_t __commandIdx;
uint8_t __commandLength;

uint8_t _wifiCLIEcho = 0x00;

const char **Commands;

#define WIFI_RESET_PORT GPIOB
#define WIFI_RESET_PIN GPIO_Pin_0

uint32_t __wifiResolveBaudRate(BaudRates_e rate){
	switch(rate){
		case Baud_9600: return 9600;
		case Baud_38400: return 38400;
		case Baud_57600: return 57600;
		case Baud_115200: return 115200;
		case Baud_230400: return 230400;
	}

	return 230400;
}

USART_InitTypeDef __wiFi_Config;

BaudRates_e _currentBaudRate;

iOpResult_e  TWB_WiFi_Init(void){
	NVIC_InitTypeDef nvicInit;

	TWB_Sys_Get_BaudRate(&_currentBaudRate);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_USART2);  //CTS
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_USART2);  //RTS
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);  //TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);  //RX

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


	GPIO_InitTypeDef uartIO;
	uartIO.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;
	uartIO.GPIO_Mode = GPIO_Mode_AF;
	uartIO.GPIO_OType = GPIO_OType_PP;
	uartIO.GPIO_Speed = GPIO_Speed_50MHz;
	uartIO.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &uartIO);

	GPIO_InitTypeDef wifiResetPort;
	wifiResetPort.GPIO_Pin = WIFI_RESET_PIN;
	wifiResetPort.GPIO_Mode = GPIO_Mode_OUT;
	wifiResetPort.GPIO_Speed = GPIO_Speed_50MHz;
	wifiResetPort.GPIO_OType = GPIO_OType_PP;
	wifiResetPort.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_WriteBit(WIFI_RESET_PORT, WIFI_RESET_PIN,  Bit_SET);
	GPIO_Init(WIFI_RESET_PORT, &wifiResetPort);

	__wiFi_Config.USART_BaudRate = __wifiResolveBaudRate(_currentBaudRate);
	__wiFi_Config.USART_WordLength = USART_WordLength_8b;
	__wiFi_Config.USART_StopBits = USART_StopBits_1;
	__wiFi_Config.USART_Parity = USART_Parity_No;
	__wiFi_Config.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//	config.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
	__wiFi_Config.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART2, &__wiFi_Config);

	nvicInit.NVIC_IRQChannel = USART2_IRQn;
	nvicInit.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART2, ENABLE);

/*	WiFiState = WiFi_Ready; */

	TWB_Debug_Print("WiFi:        OK\r\n");

	return OK;
}

BaudRates_e TWB_WiFi_GetBaudRate(void){
	return _currentBaudRate;
}

void TWB_WiFi_SetBaud(BaudRates_e baudRate) {
	_currentBaudRate = baudRate;

	TWB_Sys_Set_BaudRate(baudRate);

	__wiFi_Config.USART_BaudRate = __wifiResolveBaudRate(_currentBaudRate);

	USART_Init(USART2, &__wiFi_Config);
}


void TWB_WiFi_Timer_Tick(void){
	GPIO_WriteBit(WIFI_RESET_PORT, WIFI_RESET_PIN,  Bit_SET);
	TWB_Debug_Print("WiFly Rebooted\r\n");

	TWB_Timer_Disable(WiFiActionTimer);
}

void TWB_WiFi_Reboot(void) {
	TWB_Debug_Print("Rebooting WiFly\r\n");
	GPIO_WriteBit(WIFI_RESET_PORT, WIFI_RESET_PIN,  Bit_RESET);

	TWB_Timer_Enable(WiFiActionTimer);
}


void TWB_WiFi_HandleTXE(void) {
	if(__wiFiCommandIdx == __wiFiCommandLength){
		USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		WiFiState = WiFi_CommandSent;

	}
	else{
		uint8_t ch = __wiFiCommandBuffer[__wiFiCommandIdx++];
		USART2->DR = (ch & (uint16_t)0x01FF);
	}
}

void TWB_WiFi_HandleRX(uint8_t ch) {
	//TODO: At some point it might be nice to do some handshaking with the
	//conversation coming from the WiFly.
	TWB_Debug_AddCh(ch);
}

void __wifi_AddCmd(char *cmd){
	strcpy(__wiFiCommandBuffer, cmd);
	__wiFiCommandLength = strlen(cmd);
	__wiFiCommandIdx = 0;
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

//TODO: Might need to add back in at some point
void TWB_WiFi_PerformActions(){
	//If our timeout hasn't happened, just return
	if(!WiFiCmds->IsExpired)
		return;

	//This will only fire when we are not waiting for
	//a timer to expire.
	switch(WiFiState){
		case WiFi_PendingCommandMode:
		case WiFi_SendCommand:
			WiFiState = WiFi_SendingCommand;
			__wifi_AddCmd((char *)Commands[__commandIdx++]);

			break;
		case WiFi_SendingCommand: break;

		case WiFi_CommandSent:
			if(__commandIdx == __commandLength){
				WiFiState = WiFi_ExitCommandMode;
				WiFiCmds->TimeOutMS = 500;
				TWB_Timer_Enable(WiFiCmds);
			}
			else{
				WiFiState = WiFi_SendCommand;
				if(__commandIdx == 1){
					WiFiCmds->TimeOutMS = 300;
					TWB_Timer_Enable(WiFiCmds);
				}
				else{
					WiFiCmds->TimeOutMS = 50;
					TWB_Timer_Enable(WiFiCmds);
				}
			}
			break;

		case WiFi_ExitCommandMode:
			WiFiState = WiFi_Ready;
			break;
		default: break;
	}
}

//TODO: Might need to add back in at some point
void AddCommands(char **commands, uint8_t len){
	__commandIdx = 0;
	//Commands = commands;
	__commandLength = len;

	WiFiState = WiFi_PendingCommandMode;
	//Wait 1000ms before switching to the command mode and sending the message
	//TWB_Watchdog_Set(WiFiSTM, 300);
}

void TWB_WiFi_CloseConnection(void) {
	TWB_Debug_Print("Close Connection\r\n");
//	AddCommands(CloseSequence,4);
}
