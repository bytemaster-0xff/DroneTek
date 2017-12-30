/*
 * hmc5883.h
 *
 *  Created on: Nov 22, 2012
 *      Author: kevinw
 */

#ifndef HMC5883_H_
#define HMC5883_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_HMC5883_Init(void);

iOpResult_e TWB_IMU_HMC5883_ResetDefaults(void);
iOpResult_e TWB_IMU_HMC5883_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_IMU_HMC5883_BeginCalibration(void);
iOpResult_e TWB_IMU_HMC5883_EndCalibration(void);

iOpResult_e TWB_IMU_HMC5883_Read(float deltaT);

void TWB_IMU_HMC5883_SendDiag(void);


#endif /* HMC5883_H_ */
