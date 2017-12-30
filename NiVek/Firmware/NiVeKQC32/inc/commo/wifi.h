/*
 * wifi.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "common/twb_common.h"

typedef enum {
	Baud_9600 = 9,
	Baud_115200 = 1,
	Baud_38400 = 3,
	Baud_57600 = 5,
	Baud_230400 = 2,

} BaudRates_e;

typedef enum {
	WiFi_Ready,
	WiFi_PendingCommandMode,
	WiFi_PauseBeforeCommandString,
	WiFi_BeforeCommand,
	WiFi_SendCommand,
	WiFi_SendingCommand,
	WiFi_CommandSent,
	WiFi_ExitCommandMode,
	WiFi_CLIMode,

} WiFiStates_t;
extern WiFiStates_t WiFiState;

iOpResult_e  TWB_WiFi_Init(void);
void TWB_WiFi_Reboot(void);
void TWB_WiFi_Timer_Tick(void);

void TWB_WiFi_PerformActions(void);

void TWB_WiFi_HandleTXE(void);
void TWB_WiFi_HandleRX(uint8_t);

void TWB_WiFi_SetBaud(BaudRates_e rate);
BaudRates_e TWB_WiFi_GetBaudRate(void);

void TWB_WiFi_RestoreDefaults(void);

#endif /* WIFI_H_ */
