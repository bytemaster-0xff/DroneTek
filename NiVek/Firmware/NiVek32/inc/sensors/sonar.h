/*
 * hcsr04.h
 *
 *  Created on: Oct 28, 2012
 *      Author: kevinw
 */

#ifndef SONAR_H_
#define SONAR_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_Sonar_Init(void);
iOpResult_e TWB_IMU_Sonar_ResetDefaults(void);
iOpResult_e TWB_IMU_Sonar_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_Sonar_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_IMU_Sonar_Read(float deltaT);
iOpResult_e TWB_IMU_SONAR_IRQ(void);
iOpResult_e TWB_IMU_SONAR_ProcessData(void);
void TWB_IMU_SONAR_HandleIRQ(EdgeType_t edge);
void TWB_IMU_SONAR_SendDiag(void);


iOpResult_e TWB_IMU_Sonar_Start(void);
iOpResult_e TWB_IMU_Sonar_Stop(void);
iOpResult_e TWB_IMU_Sonar_Pause(void);
iOpResult_e TWB_IMU_Sonar_Resume(void);

#endif /* HCSR04_H_ */
