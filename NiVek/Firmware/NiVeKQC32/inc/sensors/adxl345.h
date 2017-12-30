#ifndef __ADXL345_H
#define __ADXL345_H

#include "common/twb_common.h"

iOpResult_e TWB_IMU_ADXL345_Init(void);
iOpResult_e TWB_IMU_ADXL345_ResetDefaults(void);
iOpResult_e TWB_IMU_ADXL345_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_ADXL345_Zero(uint8_t count, uint8_t pauseMs);

iOpResult_e TWB_IMU_ADXL345_Read(float deltaT);
void TWB_IMU_ADXL345_SendDiag(void);;

#endif
