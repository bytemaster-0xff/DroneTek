/*
 * itg60x0.h
 *
 *  Created on: Dec 6, 2012
 *      Author: kevinw
 */

#ifndef MPU60X0_H_
#define MPU60X0_H_

#include "common/twb_common.h"

#define MPU60x0_Address 0x68
#define MPU60x0_WHO_AM_I_ADDR 0x75
#define MPU60x0_WHO_AM_I_VALUE 0x68


iOpResult_e TWB_IMU_MPU60x0_Acc_Init(void);
iOpResult_e TWB_IMU_MPU60x0_Gyro_Init(void);

iOpResult_e TWB_IMU_MPU60x0_Gyro_ResetDefaults(void);
iOpResult_e TWB_IMU_MPU60x0_Acc_ResetDefaults(void);

iOpResult_e TWB_IMU_MPU60x0_Gyro_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_IMU_MPU60x0_Acc_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_MPU60x0_Acc_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_MPU60x0_Acc_CompleteZero(u16_vector_t *cal);

iOpResult_e TWB_IMU_MPU60x0_Gyro_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_MPU60x0_Gyro_CompleteZero(u16_vector_t *cal);

iOpResult_e TWB_IMU_MPU60x0_Gyro_IRQ(void);
iOpResult_e TWB_IMU_MPU60x0_Acc_IRQ(void);

iOpResult_e TWB_IMU_MPU60x0_Acc_Read(float deltaT);
iOpResult_e TWB_IMU_MPU60x0_Gyro_Read(float deltaT);

iOpResult_e TWB_IMU_MPU60x0_Acc_ProcessData(void);
iOpResult_e TWB_IMU_MPU60x0_Gyro_ProcessData(void);

iOpResult_e TWB_IMU_MPU60x0_Acc_Start(void);
iOpResult_e TWB_IMU_MPU60x0_Acc_Stop(void);
iOpResult_e TWB_IMU_MPU60x0_Acc_Pause(void);
iOpResult_e TWB_IMU_MPU60x0_Acc_Resume(void);

iOpResult_e TWB_IMU_MPU60x0_Gyro_Start(void);
iOpResult_e TWB_IMU_MPU60x0_Gyro_Stop(void);
iOpResult_e TWB_IMU_MPU60x0_Gyro_Pause(void);
iOpResult_e TWB_IMU_MPU60x0_Gyro_Resume(void);

void TWB_IMU_MPU60x0_Acc_SendDiag(void);
void TWB_IMU_MPU60x0_Gyro_SendDiag(void);

#endif /* ITG60X0_H_ */
