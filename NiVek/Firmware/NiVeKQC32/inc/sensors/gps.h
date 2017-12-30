/*
 * gps.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef GPS_H_
#define GPS_H_

#include "common/twb_common.h"

iOpResult_e TWB_GPS_Geo_Init(void);
iOpResult_e TWB_GPS_Alt_Init(void);
iOpResult_e TWB_GPS_Heading_Init(void);

iOpResult_e TWB_GPS_SetHomeLocation(void);

void TWB_GPS_QueueMsg(void);
void TWB_GPS_HandleRX_IRQ(uint8_t ch);
void TWB_GPS_HandleRX_Overrun(void);

iOpResult_e TWB_GPS_Geo_ResetDefaults(void);
iOpResult_e TWB_GPS_Alt_ResetDefaults(void);
iOpResult_e TWB_GPS_Heading_ResetDefaults(void);


iOpResult_e TWB_GPS_Geo_Update_Setting(uint16_t addr, uint8_t value);
iOpResult_e TWB_GPS_Alt_Update_Setting(uint16_t addr, uint8_t value);
iOpResult_e TWB_GPS_Heading_Update_Setting(uint16_t addr, uint8_t value);

#endif /* GPS_H_ */
