/*
 * itg3200.c
 *
 *  Created on: Oct 3, 2012
 *      Author: kevinw
 */

#include "twb_config.h"

#include "main.h"

#include "sensors/itg3200.h"
#include "sensors/snsr_main.h"

#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_i2c.h"
#include "common/twb_timer.h"
#include "common/twb_common.h"

#include "filters/Filter.h"

#include "twb_eeprom.h"

#define ITG3200_Address 0x68

#define ITG3200_SCALE_FACTOR 14.375
#define ITG3200_SCALE_INT_MATH 1437 //round to two decimal places.

#define DEFAULT_DLPF_FS 0b00011010 //Enable +/- 2000 full scale and 98Hz LPF
// D7, D6, D5 => N/A
// D4, D3 => FS_SEL => 11 => Only supported configuration, +/- 2000 Deg/Sec.
// D2, D1, D0 => DLP_CFG => 010 => 98 Hz, Internal Sample Rate 1KHz

#define DEFAULT_SMPLRT_DIV 0x04
// SMPLRT_DIV
// Not a bit mask field.
// We are not using LPF 256KHz so Internal Sample Rate will always be 1KHz
//
// Data Output Rate = (Internal Sample Rate) / (SMPLRT_DIV) + 1
//
// Default of 4 so 1KHz / (4 + 1) = 200Hz Update Rate.


#define ITG3200_DLPF_FS_ADDR 0x16
#define ITG3200_SMPLRT_DIV_ADDR 0x15

Filter_t __itg3200XFilter;
Filter_t __itg3200YFilter;
Filter_t __itg3200ZFilter;

s16_vector_t __itgOffset;

iOpResult_e TWB_IMU_ITG3200_Init() {
	assert_succeed(TWB_SE_ReadS16(ITG_EEPROM_OFFSET + 0, &__itgOffset.x));
	assert_succeed(TWB_SE_ReadS16(ITG_EEPROM_OFFSET + 2, &__itgOffset.y));
	assert_succeed(TWB_SE_ReadS16(ITG_EEPROM_OFFSET + 4, &__itgOffset.z))

	assert_succeed(TWB_SE_ReadU8(ITG_EEPROM_OFFSET + 6, &SensorConfig->ITG3200_Enabled));
	assert_succeed(TWB_SE_ReadU8(ITG_EEPROM_OFFSET + 7, &SensorConfig->ITG3200_DLPF_FS));
	assert_succeed(TWB_SE_ReadU8(ITG_EEPROM_OFFSET + 8, &SensorConfig->ITG3200_SMPLRT_DIV));
	assert_succeed(TWB_SE_ReadU8(ITG_EEPROM_OFFSET + 9, &SensorConfig->ITG3200_FilterOption));
	assert_succeed(TWB_SE_ReadU8(ITG_EEPROM_OFFSET + 10, &SensorConfig->ITG3200_SampleRate));

	GyroITG3200Sensor->SampleRate = SensorConfig->ITG3200_SampleRate;
	GyroITG3200Sensor->IsEnabled = SensorConfig->ITG3200_Enabled;

	__itg3200XFilter.FilterType = SensorConfig->ITG3200_FilterOption;
	__itg3200YFilter.FilterType = SensorConfig->ITG3200_FilterOption;
	__itg3200ZFilter.FilterType = SensorConfig->ITG3200_FilterOption;

	if(GyroITG3200Sensor->IsEnabled == Enabled){
		uint8_t outValue;
		assert_succeed(TWB_I2C_ReadRegisterValue(ITG3200_Address, 00, &outValue));

		assert_succeed(TWB_I2C_WriteRegister(ITG3200_Address, ITG3200_DLPF_FS_ADDR, SensorConfig->ITG3200_DLPF_FS));
		assert_succeed(TWB_I2C_WriteRegister(ITG3200_Address, ITG3200_SMPLRT_DIV_ADDR, SensorConfig->ITG3200_SMPLRT_DIV));

		TWB_Debug_Print("ITG3200-");
	}

	return OK;
}

iOpResult_e TWB_IMU_ITG3200_ResetDefaults(void){
	SensorConfig->ITG3200_Enabled = Disabled;
	SensorConfig->ITG3200_DLPF_FS = DEFAULT_DLPF_FS;
	SensorConfig->ITG3200_SMPLRT_DIV = DEFAULT_SMPLRT_DIV;
	SensorConfig->ITG3200_FilterOption = FilterOption_None;
	SensorConfig->ITG3200_SampleRate = SampleRate_200Hz;
	GyroITG3200Sensor->SampleRate = SensorConfig->ITG3200_SampleRate;
	GyroITG3200Sensor->IsEnabled = SensorConfig->ITG3200_Enabled;

	__itgOffset.x = 0;
	__itgOffset.y = 0;
	__itgOffset.z = 0;

	assert_succeed(TWB_SE_WriteS16Pause(ITG_EEPROM_OFFSET + 0, __itgOffset.x));
	assert_succeed(TWB_SE_WriteS16Pause(ITG_EEPROM_OFFSET + 2, __itgOffset.y));
	assert_succeed(TWB_SE_WriteS16Pause(ITG_EEPROM_OFFSET + 4, __itgOffset.z));

	assert_succeed(TWB_SE_WriteU8Pause(ITG_EEPROM_OFFSET + 6, SensorConfig->ITG3200_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(ITG_EEPROM_OFFSET + 7, SensorConfig->ITG3200_DLPF_FS));
	assert_succeed(TWB_SE_WriteU8Pause(ITG_EEPROM_OFFSET + 8, SensorConfig->ITG3200_SMPLRT_DIV));
	assert_succeed(TWB_SE_WriteU8Pause(ITG_EEPROM_OFFSET + 9, SensorConfig->ITG3200_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(ITG_EEPROM_OFFSET + 10, SensorConfig->ITG3200_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_ITG3200_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			SensorConfig->ITG3200_Enabled = value;
			if(SensorConfig->ITG3200_Enabled == Enabled){
				assert_succeed(TWB_IMU_ITG3200_Init());
			}
			else
				GyroITG3200Sensor->IsEnabled = Disabled;

		break;
		case 7:
			SensorConfig->ITG3200_DLPF_FS = value;
			assert_succeed(TWB_I2C_WriteRegister(ITG3200_Address, ITG3200_DLPF_FS_ADDR, SensorConfig->ITG3200_DLPF_FS));

			break;
		case 8:
			SensorConfig->ITG3200_SMPLRT_DIV = value;
			assert_succeed(TWB_I2C_WriteRegister(ITG3200_Address, ITG3200_SMPLRT_DIV_ADDR, SensorConfig->ITG3200_SMPLRT_DIV));
			break;
		case 9:
			SensorConfig->ITG3200_FilterOption = value;
			__itg3200XFilter.FilterType = value;
			__itg3200YFilter.FilterType = value;
			__itg3200ZFilter.FilterType = value;

			__itg3200XFilter.CurrentSlot = 0;
			__itg3200YFilter.CurrentSlot = 0;
			__itg3200ZFilter.CurrentSlot = 0;

			__itg3200XFilter.IsInitialized = false;
			__itg3200YFilter.IsInitialized = false;
			__itg3200ZFilter.IsInitialized = false;

			break;
		case 10:
			SensorConfig->ITG3200_SampleRate = value;
			GyroITG3200Sensor->SampleRate = SensorConfig->ITG3200_SampleRate;
			break;

	}

	return OK;
}


bool TWB_IMU_ITG3200_Zero(uint8_t sampleSize, uint8_t pauseMS){
	assert_succeed(TWB_Sensor_Calibrate(ITG3200_Address, ITG_3200_GYRO_XOUT_H, ITG_EEPROM_OFFSET, sampleSize, pauseMS, &__itgOffset, true));

	return false;
}


iOpResult_e TWB_IMU_ITG3200_IRQ(float ticks){
	assert_succeed(TWB_I2C_ReadRegisterValues(ITG3200_Address, ITG_3200_GYRO_XOUT_H, __commonSensorBuffer, 6));

	TWB_Filter_Median_Apply(&__itg3200XFilter,((int16_t)(__commonSensorBuffer[0] << 8 | __commonSensorBuffer[1])));
	TWB_Filter_Median_Apply(&__itg3200YFilter,((int16_t)(__commonSensorBuffer[2] << 8 | __commonSensorBuffer[3])));
	TWB_Filter_Median_Apply(&__itg3200ZFilter,((int16_t)(__commonSensorBuffer[4] << 8 | __commonSensorBuffer[5])));

	//These only apply the rate, not the accumulative angle (which we will also need).
	GyroITG3200Sensor->X = (int16_t)((__itg3200XFilter.Current - __itgOffset.x) / 1437);
	GyroITG3200Sensor->Y = (int16_t)((__itg3200YFilter.Current - __itgOffset.y) / 1437);
	GyroITG3200Sensor->Z = (int16_t)((__itg3200ZFilter.Current - __itgOffset.z) / 1437);

	return OK;
}

iOpResult_e TWB_IMU_ITG3200_Read(float ticks){
	return TWB_IMU_ITG3200_IRQ(ticks);
}

iOpResult_e TWB_IMU_ITG3200_EnableIRQ(){
	// ACTL 1XXXXXXX Active Low
	// OPEN X0XXXXXX Push Pull
	// LTCH XX1XXXXX Latch until IRQ Cleared
	// CLRM XXX1XXXX Clear when any register cleared
	// NA   XXXX-XXX IRQ When Device Ready
	//ITGRD XXXXX0XX IRQ When Device Ready
	//DRDY  XXXXXX-X IRQ When Data is ready
	//DRDY  XXXXXX01 IRQ When Data is ready
	//      10110001 -> 0XB1 (Enable IRQ, latch til clear)
	//      10010001 -> 0X91 (Enable IRQ, 50uS puls)
	//      10110000 -> 0X90 (Disable IRQ)

	return TWB_I2C_WriteRegister(ITG3200_Address, ITG_3200_INT_CFG, 0x91);
}

iOpResult_e TWB_IMU_ITG3200_DisableIRQ(){
	return TWB_I2C_WriteRegister(ITG3200_Address, ITG_3200_INT_CFG, 0xB0);
}

void TWB_IMU_ITG3200_SendDiag(void){
	_commonDiagBuffer->SensorId = GyroITG3200Sensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)(__itg3200XFilter.Current - __itgOffset.x);
	_commonDiagBuffer->RawY = (int16_t)(__itg3200YFilter.Current - __itgOffset.y);
	_commonDiagBuffer->RawZ = (int16_t)(__itg3200ZFilter.Current - __itgOffset.z);

	_commonDiagBuffer->X = (int16_t)GyroITG3200Sensor->X;
	_commonDiagBuffer->Y = (int16_t)GyroITG3200Sensor->Y;
	_commonDiagBuffer->Z = (int16_t)GyroITG3200Sensor->Z;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
