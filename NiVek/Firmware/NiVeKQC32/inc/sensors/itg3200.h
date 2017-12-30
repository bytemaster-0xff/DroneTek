/*
 * itg3200.h
 *
 *  Created on: Oct 3, 2012
 *      Author: kevinw
 */

#ifndef ITG3200_H_
#define ITG3200_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_ITG3200_Init(void);
iOpResult_e TWB_IMU_ITG3200_ResetDefaults(void);
iOpResult_e TWB_IMU_ITG3200_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_ITG3200_Zero(uint8_t sampleSize, uint8_t pauseMS);
iOpResult_e TWB_IMU_ITG3200_IRQ(float deltaT);
iOpResult_e TWB_IMU_ITG3200_Read(float deltaT);
iOpResult_e TWB_IMU_ITG3200_EnableIRQ(void);
iOpResult_e TWB_IMU_ITG3200_DisableIRQ(void);
void TWB_IMU_ITG3200_SendDiag(void);

#define ITG_3200_WHOAMI 		0x00
#define ITG_3200_SMPLRT_DIV		0x15
#define ITG_3200_DLPF_FS		0x16
#define ITG_3200_INT_CFG		0x17
#define ITG_3200_INT_STATUS		0x1A
#define ITG_3200_TEMP_OUT_H		0x1B
#define ITG_3200_TEMP_OUT_L		0x1C
#define ITG_3200_GYRO_XOUT_H	0x1D
#define ITG_3200_GYRO_XOUT_L	0x1E
#define ITG_3200_GYRO_YOUT_H	0x1F
#define ITG_3200_GYRO_YOUT_L	0x20
#define ITG_3200_GYRO_ZOUT_H	0x21
#define ITG_3200_GYRO_ZOUT_L	0x22
#define ITG_3200_PWR_MGM		0x3E


#endif /* ITG3200_H_ */
