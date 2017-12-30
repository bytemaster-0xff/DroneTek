/*
 * lsm303.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef LSM303_MAG_H_
#define LSM303_MAG_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_LSM303_Mag_Init(void);
iOpResult_e TWB_IMU_LSM303_Mag_ResetDefaults(void);
iOpResult_e TWB_IMU_LSM303_Mag_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_LSM303_Mag_BeginCalibration(void);
iOpResult_e TWB_IMU_LSM303_Mag_CancelCalibration(void);
iOpResult_e TWB_IMU_LSM303_Mag_EndCalibration(void);

iOpResult_e TWB_IMU_LSM303_Mag_ProcessData(void);

iOpResult_e TWB_IMU_LSM303_Mag_Start(void);
iOpResult_e TWB_IMU_LSM303_Mag_Stop(void);
iOpResult_e TWB_IMU_LSM303_Mag_Pause(void);
iOpResult_e TWB_IMU_LSM303_Mag_Resume(void);

iOpResult_e TWB_IMU_LSM303_Mag_Read(float deltaT);
void TWB_IMU_LSM303_Mag_SendDiag(void);

#endif /* LSM303_H_ */
