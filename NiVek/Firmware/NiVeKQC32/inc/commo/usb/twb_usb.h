/*
 * twb_usb.h
 *
 *  Created on: May 25, 2013
 *      Author: Kevin
 */

#ifndef TWB_USB_H_
#define TWB_USB_H_

#include "common/twb_common.h"

iOpResult_e TWB_USB_Init(void);

iOpResult_e TWB_USB_Handle_ConsoleIn(uint8_t *in, uint16_t len);
iOpResult_e TWB_USB_Handle_TelemetryIn(uint8_t *in, uint16_t len);
iOpResult_e TWB_USB_Write_Console_Out(uint8_t *out, uint16_t len);
iOpResult_e TWB_USB_Write_Debug_Out(uint8_t *out, uint16_t len);
iOpResult_e TWB_USB_Write_Telemetry_Out(uint8_t *out, uint16_t len);

#endif /* TWB_USB_H_ */
