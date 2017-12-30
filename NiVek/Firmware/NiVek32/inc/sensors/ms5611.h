#ifndef MS5611_H_
#define MS5611_H_

#include "common/twb_common.h"

typedef struct {
	uint16_t c1;
	uint16_t c2;
	uint16_t c3;
	uint16_t c4;
	uint16_t c5;
	uint16_t c6;

} MS5611CalParams_t;


iOpResult_e TWB_IMU_MS5611_Init(void);
iOpResult_e TWB_IMU_MS5611_ResetDefaults(void);
iOpResult_e TWB_IMU_MS5611_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_MS5611_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_IMU_MS5611_Read(float deltaT);
iOpResult_e TWB_IMU_MS5611_ProcessData(void);

iOpResult_e TWB_IMU_MS5611_Start(void);
iOpResult_e TWB_IMU_MS5611_Stop(void);
iOpResult_e TWB_IMU_MS5611_Pause(void);
iOpResult_e TWB_IMU_MS5611_Resume(void);

void TWB_IMU_MS5611_SendDiag(void);

#endif
