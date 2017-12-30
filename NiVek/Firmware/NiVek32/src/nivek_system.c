/*
 * nivek_system.c
 *
 *  Created on: Jul 14, 2013
 *      Author: Kevin
 */


#include "nivek_system.h"

#include <string.h>
#include "common/twb_debug.h"
#include "common/twb_leds.h"
#include "twb_eeprom.h"
#include "common/twb_serial_eeprom.h"
#include "commo/message_ids.h"
#include "memory_mgmt.h"
#include "commo/wifi.h"

uint8_t *NiVekName = null;
uint8_t StartupPhase = 255;

iOpResult_e TWB_Sys_Init(void){
	TWB_SE_ReadBuffer(NIVEK_NAME_OFFSET, NiVekName, NIVEK_NAME_SIZE);
	TWB_Debug_Print("Name: ");
	TWB_Debug_Print((const char *)NiVekName);
	TWB_Debug_Print("\r\n");

	return OK;
}

iOpResult_e TWB_Sys_Get_ShouldForwardDebugToConsole(ForwardDebugMessages_e *shoudForward){
	assert_succeed(TWB_SE_ReadU8(FORWARD_DEBUG_MSG_VIA_USB_ADDRESS, shoudForward));
	return OK;
}

iOpResult_e TWB_Sys_Set_ShouldForwardDebugToConsole(ForwardDebugMessages_e shoudForward){
	assert_succeed(TWB_SE_WriteU8(FORWARD_DEBUG_MSG_VIA_USB_ADDRESS, shoudForward));
	return OK;
}


iOpResult_e TWB_Sys_GetBaudRate(BaudRates_e *baud){
	assert_succeed(TWB_SE_ReadU8(BAUD_RATE_ADDRESS, baud));
	return OK;
}

iOpResult_e TWB_Sys_SetBaudRate(BaudRates_e baud){
	assert_succeed(TWB_SE_WriteU8(BAUD_RATE_ADDRESS, baud));
	return OK;
}

iOpResult_e TWB_Sys_RestoreDefaults(void){
	for(uint8_t idx = 0; idx < NIVEK_NAME_SIZE;++idx)
		NiVekName[idx] = 0x00;

	assert_succeed(TWB_SE_WriteBufferPause(NIVEK_NAME_OFFSET, NiVekName, NIVEK_NAME_SIZE));
	assert_succeed(TWB_SE_WriteU8Pause(BAUD_RATE_ADDRESS, Baud_230400));
	assert_succeed(TWB_SE_WriteU8Pause(FORWARD_DEBUG_MSG_VIA_USB_ADDRESS, ForwardDebugMessages_Off));

	return OK;
}

iOpResult_e TWB_Sys_HandleMessage(TWB_Commo_Message_t *msg) {
	switch(msg->TypeId){
		case MSG_PING:
		case MSG_WELCOME_PING:
			//If we were previously disconnected make sure
			//Send a welcome ping and set connection status.
			if(msg->TypeId == MSG_WELCOME_PING){
				TWB_Debug_Print("WiFi Connected, send welcome ping\r\n");
				TWB_Commo_SendMessage(MOD_System, MSG_PING, null, 0);
			}

			//Along with the Ping comes settings for control inputs.
			WiFiThrottle = msg->MsgBuffer[0];
			WiFiPitch = msg->MsgBuffer[1];
			WiFiRoll = msg->MsgBuffer[2];
			WiFiYaw = msg->MsgBuffer[3];

			TWB_Timer_Enable(WiFiInWatchDog);
			WiFiIsConnected = WiFiConnected;
			TWB_LEDS_SetState(LED_WiFi_Connected, LED_On);

			if(WiFiIsConnected == WiFiDisconnected)
				TWB_Debug_Print("\r\nTelemetry Connected\r\n\r\n");

		break;
		case MSG_SET_NAME:
			memcpy(NiVekName, msg->MsgBuffer, NIVEK_NAME_SIZE);

			TWB_Debug_Print("NiVek's new name: ");
			TWB_Debug_Print((const char *)NiVekName);
			TWB_Debug_Print("\r\n");
			assert_succeed(TWB_SE_WriteBuffer(NIVEK_NAME_OFFSET, msg->MsgBuffer, NIVEK_NAME_SIZE));
			break;
		case MSG_GET_NAME:
			TWB_Commo_SendMessage(MOD_System, MSG_NIVEK_NAME, NiVekName, NIVEK_NAME_SIZE);

			break;
	}

	return OK;
}


