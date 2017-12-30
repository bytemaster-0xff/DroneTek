/*
 * nivek_system.h
 *
 *  Created on: Jul 14, 2013
 *      Author: Kevin
 */

#ifndef NIVEK_SYSTEM_H_
#define NIVEK_SYSTEM_H_

#include "twb_common.h"
#include "commo.h"
#include "commo/wifi.h"
#include "common/twb_debug.h"

iOpResult_e TWB_Sys_Init(void);
iOpResult_e TWB_Sys_ReadName(void);
iOpResult_e TWB_Sys_HandleMessage(TWB_Commo_Message_t *msg);
iOpResult_e TWB_Sys_RestoreDefaults(void);

iOpResult_e TWB_Sys_Get_ShouldForwardDebugToConsole(ForwardDebugMessages_e *shoudForward);
iOpResult_e TWB_Sys_Set_ShouldForwardDebugToConsole(ForwardDebugMessages_e shoudForward);

iOpResult_e TWB_Sys_SetBaudRate(BaudRates_e baud);
iOpResult_e TWB_Sys_GetBaudRate(BaudRates_e *baud);

extern uint8_t StartupPhase;

#define MSG_PING 100 //Ping constant for both directions
#define MSG_WELCOME_PING 101 //Ping constant for both directions

#define MSG_SET_NAME 200 // Set a new name for NiVek.
#define MSG_GET_NAME 201 // Get the name of NiVek

#define MSG_NIVEK_NAME 200 // Outgoing message type to include NiVek's Name


/*	 Revision History
 *
 * 		0.32 	8/12/2013	Fix yaw PID controller
 *
 */

#define FIRMWARE_VERSION 0.34f

#define NIVEK_NAME_OFFSET SYSTEM_AREA_OFFSET + 0x00
#define NIVEK_NAME_SIZE   24

#define BAUD_RATE_ADDRESS NIVEK_NAME_OFFSET + NIVEK_NAME_SIZE
#define FORWARD_DEBUG_MSG_VIA_USB_ADDRESS NIVEK_NAME_OFFSET + BAUD_RATE_ADDRESS


#endif /* NIVEK_SYSTEM_H_ */
