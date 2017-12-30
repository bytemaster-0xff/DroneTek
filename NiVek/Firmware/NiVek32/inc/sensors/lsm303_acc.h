/*
 * lsm303.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef LSM303_ACC_H_
#define LSM303_ACC_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_LSM303_Acc_Init(void);
iOpResult_e TWB_IMU_LSM303_Acc_ResetDefaults(void);
iOpResult_e TWB_IMU_LSM303_Acc_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_LSM303_Acc_Start(void);
iOpResult_e TWB_IMU_LSM303_Acc_Pause(void);
iOpResult_e TWB_IMU_LSM303_Acc_Resume(void);
iOpResult_e TWB_IMU_LSM303_Acc_Stop(void);


iOpResult_e TWB_IMU_LSM303_Acc_Zero(uint8_t points, float pauseMS);
iOpResult_e TWB_IMU_LSM303_Acc_CompleteZero(u16_vector_t *cal);

iOpResult_e  TWB_IMU_LSM303_Acc_ProcessData(void);
iOpResult_e TWB_IMU_LSM303_Acc_Read(float deltaT);
iOpResult_e TWB_IMU_LSM303_Acc_IRQ(void);
iOpResult_e TWB_IMU_LSM303_EnableIRQ(void);
iOpResult_e TWB_IMU_LSM303_DisableIRQ(void);
void TWB_IMU_LSM303_Acc_SendDiag(void);

extern float __lsm303AccX,__lsm303AccY, __lsm303AccZ;

#endif /* LSM303_H_ */
