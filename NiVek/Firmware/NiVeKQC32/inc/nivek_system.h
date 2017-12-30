/*
 * nivek_system.h
 *
 *  Created on: Jul 14, 2013
 *      Author: Kevin
 */

#ifndef NIVEK_SYSTEM_H_
#define NIVEK_SYSTEM_H_

#include "common/twb_common.h"
#include "commo/commo.h"
#include "commo/wifi.h"
#include "common/twb_debug.h"

iOpResult_e TWB_Sys_Init(void);
iOpResult_e TWB_Sys_ReadName(void);
iOpResult_e TWB_Sys_HandleMessage(TWB_Commo_Message_t *msg);
iOpResult_e TWB_Sys_RestoreDefaults(uint8_t version);

iOpResult_e TWB_Sys_Get_ShouldForwardDebugToConsole(ForwardDebugMessages_e *shoudForward);
iOpResult_e TWB_Sys_Set_ShouldForwardDebugToConsole(ForwardDebugMessages_e shoudForward);
iOpResult_e TWB_Sys_Set_SerialNumber(uint32_t serialNumber);
iOpResult_e TWB_Sys_Set_Address(uint8_t address);
iOpResult_e TWB_Sys_Set_NiVekName(char *name);
iOpResult_e TWB_WiFi_Set_Baud(BaudRates_e baud);
iOpResult_e TWB_Sys_Get_BaudRate(BaudRates_e *baud);

extern uint8_t StartupPhase;

#define MSG_PING 100 //Ping constant for both directions
#define MSG_WELCOME_PING 101 //Ping constant for both directions

#define MSG_SET_NAME 200 // Set a new name for NiVek.
#define MSG_GET_NAME 201 // Get the name of NiVek
#define MSG_SET_SERIAL_NUMBER 210 // Get the name of NiVek
#define MSG_GET_SERIAL_NUMBER 211 // Get the name of NiVek
#define MSG_SET_ADDRESS 220 // Get the name of NiVek

#define MSG_NIVEK_NAME 200 // Outgoing message type to include NiVek's Name
#define MSG_NIVEK_SERIAL_NUMBER 201 // Outgoing message type to include NiVek's Name

/*	 Revision History
 *
 * 		0.32 	8/12/2013	Fix yaw PID controller
 *
 */

#define FIRMWARE_VERSION 0.41f

#define NIVEK_NAME_OFFSET SYSTEM_AREA_OFFSET + 0x00
#define NIVEK_NAME_SIZE   24
#define NIVEK_SERIAL_NUMBER NIVEK_NAME_OFFSET + NIVEK_NAME_SIZE
#define BAUD_RATE_ADDRESS NIVEK_SERIAL_NUMBER + 2
#define FORWARD_DEBUG_MSG_VIA_USB_ADDRESS BAUD_RATE_ADDRESS + 1
#define NIVEK_ADDRESS FORWARD_DEBUG_MSG_VIA_USB_ADDRESS + 1



#endif /* NIVEK_SYSTEM_H_ */
