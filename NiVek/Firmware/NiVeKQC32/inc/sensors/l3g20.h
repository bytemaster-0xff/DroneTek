#ifndef L3G20_H_
#define L3G20_H_

#include "common/twb_common.h"

iOpResult_e TWB_IMU_L3GD20_Init(void);
iOpResult_e TWB_IMU_L3GD20_ResetDefaults(void);
iOpResult_e TWB_IMU_L3GD20_UpdateSetting(uint16_t addr, uint8_t value);

iOpResult_e TWB_IMU_L3GD20_Start(void);
iOpResult_e TWB_IMU_L3GD20_Pause(void);
iOpResult_e TWB_IMU_L3GD20_Resume(void);
iOpResult_e TWB_IMU_L3GD20_Stop(void);

iOpResult_e TWB_IMU_L3GD20_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_L3GD20_CompleteZero(u16_vector_t *cal);
iOpResult_e TWB_IMU_L3GD30_ProcessData(void);
iOpResult_e TWB_IMU_L3GD20_Read(float deltaT);

iOpResult_e TWB_IMU_L3GD20_IRQ(void);

iOpResult_e TWB_IMU_L3GD20_EnableIRQ(void);
iOpResult_e TWB_IMU_L3GD20_DisableIRQ(void);
void TWB_IMU_L3GD20_SendDiag(void);

#endif
