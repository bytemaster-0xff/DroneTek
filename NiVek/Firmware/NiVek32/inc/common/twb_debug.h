/*
 * twb_debug.h
 *
 *  Created on: Oct 5, 2012
 *      Author: kevinw
 */

#ifndef TWB_DEBUG_H_
#define TWB_DEBUG_H_

#include "twb_common.h"

#define DEBUG_SIGNAL_PIN

typedef enum {
	ForwardDebugMessages_On = 0xFF,
	ForwardDebugMessages_Off = 0x00,
} ForwardDebugMessages_e;


iOpResult_e TWB_Debug_AddCh(char ch);
iOpResult_e TWB_Debug_Print(const char *strBuffer);
iOpResult_e TWB_Debug_PrintInt(const char *strBuffer, int value);
iOpResult_e TWB_Debug_Print2Int(const char *strBuffer, int value1, int value2);
iOpResult_e TWB_Debug_Init(void);
iOpResult_e TWB_Debug_PrintHexString(uint8_t byte);
iOpResult_e TWB_Debug_PrintHexNumb(unsigned long numb);
void TWB_Debug_HandleTXE(void);

iOpResult_e TWB_Debug_SetShouldForwardDebugOverUSB(ForwardDebugMessages_e debug_shouldForward);
iOpResult_e TWB_Debug_ReadDefaults(void);

void TWB_Debug_Toggle(void);
void TWB_Debug_TimerStart(void);
void TWB_Debug_TimerEnd(void);

#endif /* TWB_DEBUG_H_ */
