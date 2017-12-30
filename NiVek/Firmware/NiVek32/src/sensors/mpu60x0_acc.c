/*
 * mpu60x0_acc.c
 *
 *  Created on: Jan 12, 2013
 *      Author: kevinw
 */


#include "sensors/snsr_main.h"
#include "twb_config.h"
#include "twb_eeprom.h"
#include "common/memory_mgmt.h"
#include "common/twb_i2c.h"
#include "sensors/mpu60x0.h"
#include "filters/Filter.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_math.h"

#define MPU60x0_DLPF_ADDR 0x1A
#define MPG60x0_ACC_CFG_ADDR 0x1C
#define MPU60x0_SMPLRT_DIV_ADDR 0x19
#define MPU60x0_FIFO_ENABLE_ADDR 0x23
#define MPU60x0_PWR_MGMT1_ADDR 0x6B

#define MPU_60x0_ACC_XOUT_H 0x3B

#define MPU60x0_DEFAULT_ACC_FS 0b00000000
// D7, D6, D5 - Self Test/ not used set to 0
// D4, D3 - 00 +/ 2G
// D2, D1, D0 - Not Used, keep at zero.

#define MPU60x0_SLEEP_DISABLED 0b00000000


FilterSigned_t __mpuAcc60x0XFilter;
FilterSigned_t __mpuAcc60x0YFilter;
FilterSigned_t __mpuAcc60x0ZFilter;

s16_vector_t __mpu60x0AccOffset;

uint16_t __mpu60x0AccScaler;

I2CJob_t *MPU60x0AccI2CJob;

iOpResult_e __twb_imu_mpu60x0_Acc_InitPerip(void){
	uint8_t outValue;
	assert_succeed(TWB_I2C_ReadRegisterValue(MPU60x0_Address, MPU60x0_WHO_AM_I_ADDR, &outValue));

	if(outValue != MPU60x0_WHO_AM_I_VALUE){
		TWB_Debug_Print("No MPU60x0 Acc\r\n");
		return FAIL;
	}

	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPG60x0_ACC_CFG_ADDR, SensorConfig->MPU60x0_ACC_FS));
	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_DLPF_ADDR, SensorConfig->MPU60x0_ACC_GYR_DLPF));

	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_PWR_MGMT1_ADDR, MPU60x0_SLEEP_DISABLED));

	TWB_Debug_Print("MPU60x0.Acc-");

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Init(void) {
	assert_succeed(TWB_SE_ReadS16(MPU60x0_ACC_EEPROM_OFFSET + 0, &__mpu60x0AccOffset.x));
	assert_succeed(TWB_SE_ReadS16(MPU60x0_ACC_EEPROM_OFFSET + 2, &__mpu60x0AccOffset.y));
	assert_succeed(TWB_SE_ReadS16(MPU60x0_ACC_EEPROM_OFFSET + 4, &__mpu60x0AccOffset.z))

	assert_succeed(TWB_SE_ReadU8(MPU60x0_ACC_EEPROM_OFFSET + 6, &SensorConfig->MPU60x0_Acc_Enabled));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_ACC_EEPROM_OFFSET + 7, &SensorConfig->MPU60x0_ACC_FS));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_ACC_EEPROM_OFFSET + 8, &SensorConfig->MPU60x0_ACC_GYR_DLPF));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_ACC_EEPROM_OFFSET + 9, &SensorConfig->MPU60x0_AccFilterOption));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_ACC_EEPROM_OFFSET + 10, &SensorConfig->MPU60x0_AccSampleRate));

	AccMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_AccSampleRate;
	AccMPU60x0Sensor->IsEnabled = SensorConfig->MPU60x0_Acc_Enabled;

	AccMPU60x0Sensor->StartZero = &TWB_IMU_MPU60x0_Acc_Zero;
	AccMPU60x0Sensor->CompleteZero = &TWB_IMU_MPU60x0_Acc_CompleteZero;

	AccMPU60x0Sensor->ProcessData = &TWB_IMU_MPU60x0_Acc_ProcessData;

	AccMPU60x0Sensor->Read = &TWB_IMU_MPU60x0_Acc_Read;
	AccMPU60x0Sensor->HandleIRQ = &TWB_IMU_MPU60x0_Acc_IRQ;

	AccMPU60x0Sensor->Start = &TWB_IMU_MPU60x0_Acc_Start;
	AccMPU60x0Sensor->Stop = &TWB_IMU_MPU60x0_Acc_Stop;
	AccMPU60x0Sensor->Pause = &TWB_IMU_MPU60x0_Acc_Pause;
	AccMPU60x0Sensor->Resume = &TWB_IMU_MPU60x0_Acc_Resume;

	__mpuAcc60x0XFilter.FilterType = SensorConfig->MPU60x0_AccFilterOption;
	__mpuAcc60x0YFilter.FilterType = SensorConfig->MPU60x0_AccFilterOption;
	__mpuAcc60x0ZFilter.FilterType = SensorConfig->MPU60x0_AccFilterOption;

	if(SensorConfig->MPU60x0_Acc_Enabled == Enabled)
		assert_succeed(__twb_imu_mpu60x0_Acc_InitPerip());

	MPU60x0AccI2CJob = pm_malloc(sizeof(I2CJob_t));
	MPU60x0AccI2CJob->OutBuffer = pm_malloc(1);
	MPU60x0AccI2CJob->InBuffer = pm_malloc(6);

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_ResetDefaults(void){
	SensorConfig->MPU60x0_Acc_Enabled = Disabled;
	SensorConfig->MPU60x0_ACC_FS = MPU60x0_DEFAULT_ACC_FS;
	SensorConfig->MPU60x0_AccFilterOption = FilterOption_None;
	SensorConfig->MPU60x0_AccSampleRate = SampleRate_100Hz;

	AccMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_AccSampleRate;
	AccMPU60x0Sensor->IsEnabled = SensorConfig->MPU60x0_Acc_Enabled;

	__mpu60x0AccOffset.x = 0;
	__mpu60x0AccOffset.y = 0;
	__mpu60x0AccOffset.z = 0;

	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_ACC_EEPROM_OFFSET + 0,__mpu60x0AccOffset.x));
	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_ACC_EEPROM_OFFSET + 2,__mpu60x0AccOffset.y));
	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_ACC_EEPROM_OFFSET + 4,__mpu60x0AccOffset.z));

	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_ACC_EEPROM_OFFSET + 6, SensorConfig->MPU60x0_Acc_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_ACC_EEPROM_OFFSET + 7, SensorConfig->MPU60x0_ACC_FS));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_ACC_EEPROM_OFFSET + 8, SensorConfig->MPU60x0_ACC_GYR_DLPF));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_ACC_EEPROM_OFFSET + 9, SensorConfig->MPU60x0_AccFilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_ACC_EEPROM_OFFSET + 10, SensorConfig->MPU60x0_AccSampleRate));

	return OK;
}



iOpResult_e TWB_IMU_MPU60x0_Acc_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			SensorConfig->MPU60x0_Acc_Enabled = value;
			AccMPU60x0Sensor->IsEnabled = value;
			if(AccMPU60x0Sensor->IsEnabled == Enabled)
				assert_succeed(__twb_imu_mpu60x0_Acc_InitPerip());

			break;
		case 7:
			SensorConfig->MPU60x0_ACC_FS = value;
			assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPG60x0_ACC_CFG_ADDR, SensorConfig->MPU60x0_ACC_FS));
			break;
		case 8:
			SensorConfig->MPU60x0_ACC_GYR_DLPF = value;
			assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_DLPF_ADDR, SensorConfig->MPU60x0_ACC_GYR_DLPF));
			break;
		case 9:
			SensorConfig->MPU60x0_AccFilterOption = value;
			__mpuAcc60x0XFilter.FilterType = value;
			__mpuAcc60x0YFilter.FilterType = value;
			__mpuAcc60x0ZFilter.FilterType = value;
			break;
		case 10:
			SensorConfig->MPU60x0_AccSampleRate = value;
			AccMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_AccSampleRate;
			break;
	}

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Start(void){
	__mpuAcc60x0XFilter.CurrentSlot = 0;
	__mpuAcc60x0YFilter.CurrentSlot = 0;
	__mpuAcc60x0ZFilter.CurrentSlot = 0;

	__mpuAcc60x0XFilter.IsInitialized = false;
	__mpuAcc60x0YFilter.IsInitialized = false;
	__mpuAcc60x0ZFilter.IsInitialized = false;
	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Pause(void){

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Resume(void){
	__mpuAcc60x0XFilter.CurrentSlot = 0;
	__mpuAcc60x0YFilter.CurrentSlot = 0;
	__mpuAcc60x0ZFilter.CurrentSlot = 0;

	__mpuAcc60x0XFilter.IsInitialized = false;
	__mpuAcc60x0YFilter.IsInitialized = false;
	__mpuAcc60x0ZFilter.IsInitialized = false;

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Stop(void){

	return OK;
}


bool TWB_IMU_MPU60x0_Acc_Zero(uint8_t sampleSize, float pauseMS){
	TWB_Sensor_Calibrate(MPU60x0_Address, MPU_60x0_ACC_XOUT_H, MPU60x0_ACC_EEPROM_OFFSET + 6, sampleSize, pauseMS, &__mpu60x0AccOffset, true);

	return false;
}

iOpResult_e TWB_IMU_MPU60x0_Acc_CompleteZero(u16_vector_t *cal) {
	__mpu60x0AccOffset.x = cal->x;
	__mpu60x0AccOffset.y = cal->y;
	__mpu60x0AccOffset.z = cal->z;

	return OK;
}


void __imu_mpu60x0_acc_dataReady(void *job){
	AccMPU60x0Sensor->DataReady = DataReady;
}


float _mpuAccX, _mpuAccY, _mpuAccZ;

iOpResult_e TWB_IMU_MPU60x0_Acc_ProcessData(void){
	TWB_Filter_Median_Apply_Signed(&__mpuAcc60x0XFilter,(int16_t)(MPU60x0AccI2CJob->InBuffer[0] << 8 | MPU60x0AccI2CJob->InBuffer[1]));
	TWB_Filter_Median_Apply_Signed(&__mpuAcc60x0YFilter,(int16_t)(MPU60x0AccI2CJob->InBuffer[2] << 8 | MPU60x0AccI2CJob->InBuffer[3]));
	TWB_Filter_Median_Apply_Signed(&__mpuAcc60x0ZFilter,(int16_t)(MPU60x0AccI2CJob->InBuffer[4] << 8 | MPU60x0AccI2CJob->InBuffer[5]));

	_mpuAccX = (__mpuAcc60x0XFilter.Current / 16384.0f);
	_mpuAccY = (__mpuAcc60x0XFilter.Current / 16384.0f);
	_mpuAccZ = (__mpuAcc60x0XFilter.Current / 16384.0f);

	_mpuAccX = TWB_MATH_LimitOneToMinusOne(_mpuAccX);
	_mpuAccY = TWB_MATH_LimitOneToMinusOne(_mpuAccY);
	_mpuAccZ = TWB_MATH_LimitOneToMinusOne(_mpuAccZ);

	AccMPU60x0Sensor->X = (float)(360.0f * twb_asinf(_mpuAccX) / 6.28318f);
	AccMPU60x0Sensor->Y = (float)(360.0f * twb_asinf(_mpuAccY) / 6.28318f);
	AccMPU60x0Sensor->Z = (float)(360.0f * twb_asinf(_mpuAccZ) / 6.28318f);

	return OK;
}

iOpResult_e __twb_imu_MPU60x0_Acc_Read(void){
	MPU60x0AccI2CJob->Address = MPU60x0_Address;
	MPU60x0AccI2CJob->OutBuffer[0] = MPU_60x0_ACC_XOUT_H;
	MPU60x0AccI2CJob->OutBufferSize = 1;
	MPU60x0AccI2CJob->InBufferSize = 6;
	MPU60x0AccI2CJob->Callback = &__imu_mpu60x0_acc_dataReady;

	return TWB_I2C_QueueAsyncJob(MPU60x0AccI2CJob);
}

iOpResult_e TWB_IMU_MPU60x0_Acc_Read(float deltaT){
	return  __twb_imu_MPU60x0_Acc_Read();
}

iOpResult_e TWB_IMU_MPU60x0_Acc_IRQ(){
	return  __twb_imu_MPU60x0_Acc_Read();
}

void TWB_IMU_MPU60x0_Acc_SendDiag(void){
	_commonDiagBuffer->SensorId = AccMPU60x0Sensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)(__mpuAcc60x0XFilter.Current - __mpu60x0AccOffset.x);
	_commonDiagBuffer->RawY = (int16_t)(__mpuAcc60x0YFilter.Current - __mpu60x0AccOffset.y);
	_commonDiagBuffer->RawZ = (int16_t)(__mpuAcc60x0ZFilter.Current - __mpu60x0AccOffset.z);

	_commonDiagBuffer->X = (int16_t)(AccMPU60x0Sensor->X * 10.0f);
	_commonDiagBuffer->Y = (int16_t)(AccMPU60x0Sensor->Y * 10.0f);
	_commonDiagBuffer->Z = (int16_t)(AccMPU60x0Sensor->Z * 10.0f);

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}

