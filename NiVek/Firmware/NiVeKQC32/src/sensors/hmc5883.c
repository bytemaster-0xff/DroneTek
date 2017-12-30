/*
 * hmc5883.c
 *
 *  Created on: Nov 22, 2012
 *      Author: kevinw
 */

#include <math.h>

#include "sensors/hmc5883.h"

#include "twb_eeprom.h"

#include "filters/Filter.h"
#include "twb_config.h"
#include "main.h"

#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_timer.h"
#include "common/twb_i2c.h"

#include "sensors/snsr_main.h"

s16_vector_t __hmcMagMin;
s16_vector_t __hmcMagMax;

Filter_t _hmc5883XFilter;
Filter_t _hmc5883YFilter;
Filter_t _hmc5883ZFilter;

#define __MATH_PI 3.141592

#define HMC5883_ADDR 0x1E

#define HMC5883_CRA_DEFAULT 0b01111000  //Average 8 samples per measurement, 75 Hz Output Rate
//  D7 N/A
//  D6, D5 (Samples Averaged) => 11,  8 Samples
//  D4, D3, D2 (Data Output Rate) > 110, 75Hz
//  D1, D0 (Measurement Mode) => 00, Normal Operation

#define HMC5883_CRB_DEFAULT 0b10000000  // +/- 4.0 Ga
//  D7, D6, D5 (Gain) => 100 => +/- 4.0 Ga, Resolution 2.27 mG/Lsb
//  D4, D3, D2, D1, D0 - Bits cleared for correct operation.

#define HMC5883_MODE_DEFAULT 0b00000000 //Only use lowest two bits, these are 00 for continuous measurements.
//  D7-D2 (N/A) keep default to 0
//  D1, D0 (Mode Select Bits) => 00 => Continous mode of operation, data available at Data Output Rate.

#define HMC5883_CRA_ADDR 0x00
#define HMC5883_CRB_ADDR 0x01
#define HMC5883_MODE_ADDR 0x02
#define HMC5883_ID_REG1_ADDR 0x0A

#define HMC5883_DATE_X_MSB 0x03
#define HMC5883_DATE_X_LSB 0x04
#define HMC5883_DATE_Z_MSB 0x05
#define HMC5883_DATE_Z_LSB 0x06
#define HMC5883_DATE_Y_MSB 0x07
#define HMC5883_DATE_Y_LSB 0x08

#define HMC588_MIN_OUTPUT -2048
#define HMC588_MAX_OUTPUT 2047

iOpResult_e TWB_IMU_HMC5883_Init(void){
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 0, &SensorConfig->HMC5883_Calibrated));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 1, &__hmcMagMin.x));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 3, &__hmcMagMin.y));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 5, &__hmcMagMin.z));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 7, &__hmcMagMax.x));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 9, &__hmcMagMax.y));
	assert_succeed(TWB_SE_ReadS16(HMC5883_EEPROM_OFFSET + 11, &__hmcMagMax.z));

	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 13, &SensorConfig->HMC5883_Enabled));
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 14, &SensorConfig->HMC5883_CRA));
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 15, &SensorConfig->HMC5883_CRB));
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 16, &SensorConfig->HMC5883_MODE));
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 17, &SensorConfig->HMC5883_FilterOption));
	assert_succeed(TWB_SE_ReadU8(HMC5883_EEPROM_OFFSET + 18, &SensorConfig->HMC5883_SampleRate));

	MagHMC5883Sensor->SampleRate = SensorConfig->HMC5883_SampleRate;
	MagHMC5883Sensor->IsEnabled = SensorConfig->HMC5883_Enabled;

	_hmc5883XFilter.FilterType = SensorConfig->HMC5883_FilterOption;
	_hmc5883YFilter.FilterType = SensorConfig->HMC5883_FilterOption;
	_hmc5883ZFilter.FilterType = SensorConfig->HMC5883_FilterOption;

	if(SensorConfig->HMC5883_Enabled == Enabled){
		uint8_t outValue = 0x00;
		assert_succeed(TWB_I2C_ReadRegisterValue(HMC5883_ADDR, HMC5883_ID_REG1_ADDR, &outValue));

		if(outValue != 0b01001000){
			TWB_Debug_Print("!!Could not configure HMC5883.\r\n");
			return FAIL;
		}

		//Now init the chip.
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_CRA_ADDR, SensorConfig->HMC5883_CRA));
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_CRB_ADDR, SensorConfig->HMC5883_CRB));
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_MODE_ADDR, SensorConfig->HMC5883_MODE));

		TWB_Debug_Print("HMC5883-");
	}

	return OK;
}

iOpResult_e TWB_IMU_HMC5883_ResetDefaults(void){
	__hmcMagMin.x = 10000;
	__hmcMagMin.y = 10000;
	__hmcMagMin.z = 10000;
	__hmcMagMax.x = -10000;
	__hmcMagMax.y = -10000;
	__hmcMagMax.z = -10000;

	SensorConfig->HMC5883_Calibrated = NotCalibrated;

	SensorConfig->HMC5883_Enabled = Disabled;
	SensorConfig->HMC5883_CRA = HMC5883_CRA_DEFAULT;
	SensorConfig->HMC5883_CRB = HMC5883_CRB_DEFAULT;
	SensorConfig->HMC5883_MODE = HMC5883_MODE_DEFAULT;
	SensorConfig->HMC5883_FilterOption = FilterOption_None;
	SensorConfig->HMC5883_SampleRate = SampleRate_50Hz;

	MagHMC5883Sensor->SampleRate = SensorConfig->HMC5883_SampleRate;
	MagHMC5883Sensor->IsEnabled = SensorConfig->HMC5883_Enabled;

	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 0, SensorConfig->HMC5883_Calibrated));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 1, __hmcMagMin.x));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 3, __hmcMagMin.y));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 5, __hmcMagMin.z));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 7, __hmcMagMax.x));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 9, __hmcMagMax.y));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 11, __hmcMagMax.z));

	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 13, SensorConfig->HMC5883_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 14, SensorConfig->HMC5883_CRA));
	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 15, SensorConfig->HMC5883_CRB));
	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 16, SensorConfig->HMC5883_MODE));
	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 17, SensorConfig->HMC5883_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 18, SensorConfig->HMC5883_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_HMC5883_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
	case 13:
		SensorConfig->HMC5883_Enabled = value;
		if(SensorConfig->HMC5883_Enabled == Enabled){
			assert_succeed(TWB_IMU_HMC5883_Init());
		}
		else
			MagHMC5883Sensor->IsEnabled = Disabled;
		break;
	case 14:
		SensorConfig->HMC5883_CRA = value;
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_CRA_ADDR, SensorConfig->HMC5883_CRA));
		break;
	case 15:
		SensorConfig->HMC5883_CRB = value;
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_CRB_ADDR, SensorConfig->HMC5883_CRB));
		break;
	case 16:
		SensorConfig->HMC5883_MODE = value;
		assert_succeed(TWB_I2C_WriteRegister(HMC5883_ADDR, HMC5883_MODE_ADDR, SensorConfig->HMC5883_MODE));
		break;
	case 17:
		SensorConfig->HMC5883_FilterOption = value;
		_hmc5883XFilter.FilterType = value;
		_hmc5883YFilter.FilterType = value;
		_hmc5883ZFilter.FilterType = value;

		_hmc5883XFilter.IsInitialized = false;
		_hmc5883YFilter.IsInitialized = false;
		_hmc5883ZFilter.IsInitialized = false;

		_hmc5883XFilter.CurrentSlot = 0;
		_hmc5883YFilter.CurrentSlot = 0;
		_hmc5883ZFilter.CurrentSlot = 0;
		break;

	case 18:
		SensorConfig->HMC5883_SampleRate = value;
		MagHMC5883Sensor->SampleRate = SensorConfig->HMC5883_SampleRate;
		break;
	}

	return OK;
}


iOpResult_e TWB_IMU_HMC5883_BeginCalibration(void){
	SensorConfig->HMC5883_Calibrated = NotCalibrated;

	__hmcMagMin.x = 10000;
	__hmcMagMin.y = 10000;
	__hmcMagMin.z = 10000;
	__hmcMagMax.x = -10000;
	__hmcMagMax.y = -10000;
	__hmcMagMax.z = -10000;

	return OK;
}

iOpResult_e TWB_IMU_HMC5883_EndCalibration(void){
	SensorConfig->HMC5883_Calibrated = Calibrated;

	assert_succeed(TWB_SE_WriteU8Pause(HMC5883_EEPROM_OFFSET + 0, SensorConfig->HMC5883_Calibrated));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 1, __hmcMagMin.x));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 3, __hmcMagMin.y));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 5, __hmcMagMin.z));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 7, __hmcMagMax.x));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 9, __hmcMagMax.y));
	assert_succeed(TWB_SE_WriteS16Pause(HMC5883_EEPROM_OFFSET + 11, __hmcMagMax.z));

	return OK;
}

iOpResult_e TWB_IMU_HMC5883_Read(float deltaT) {
	assert_succeed(TWB_I2C_ReadRegisterValues(HMC5883_ADDR, HMC5883_DATE_X_MSB, __commonSensorBuffer, 6));

	TWB_Filter_Median_Apply(&_hmc5883XFilter,(int16_t)(__commonSensorBuffer[0] << 8 | __commonSensorBuffer[1]));
	TWB_Filter_Median_Apply(&_hmc5883YFilter,(int16_t)(__commonSensorBuffer[2] << 8 | __commonSensorBuffer[3]));
	TWB_Filter_Median_Apply(&_hmc5883ZFilter,(int16_t)(__commonSensorBuffer[4] << 8 | __commonSensorBuffer[5]));

	if(SystemStatus->SnsrState == SensorAppState_MagCalibration){
		if((int16_t)_hmc5883XFilter.Current < __hmcMagMin.x) __hmcMagMin.x = (int16_t)_hmc5883XFilter.Current;
		if((int16_t)_hmc5883XFilter.Current > __hmcMagMax.x) __hmcMagMax.x = (int16_t)_hmc5883XFilter.Current;

		if((int16_t)_hmc5883YFilter.Current < __hmcMagMin.y) __hmcMagMin.y = (int16_t)_hmc5883YFilter.Current;
		if((int16_t)_hmc5883YFilter.Current > __hmcMagMax.y) __hmcMagMax.y = (int16_t)_hmc5883YFilter.Current;

		if((int16_t)_hmc5883ZFilter.Current < __hmcMagMin.z) __hmcMagMin.z = (int16_t)_hmc5883ZFilter.Current;
		if((int16_t)_hmc5883ZFilter.Current > __hmcMagMax.z) __hmcMagMax.z = (int16_t)_hmc5883ZFilter.Current;
	}
	else{
		/*vector_t m;

		m.x = (float)(SensorData->RawMagX_HMC5883 - __hmcMagMin.x);
		m.y = (float)(SensorData->RawMagY_HMC5883 - __hmcMagMin.y);
		m.z = (float)(SensorData->RawMagZ_HMC5883 - __hmcMagMin.z);

		m.x = m.x / (__hmcMagMax.x - __hmcMagMin.x) * 2 - 1.0;
		m.y = m.y / (__hmcMagMax.y - __hmcMagMin.y) * 2 - 1.0;
		m.z = m.z / (__hmcMagMax.z - __hmcMagMin.z) * 2 - 1.0;

		if(__hmcMagMax.x == __hmcMagMin.x)
			SensorData->Heading_HMC5883 = 0;
		else{
			SensorData->Heading_HMC5883 = (int16_t)(360 * atan2(m.y ,m.x) / __MATH_PI);
			if(SensorData->Heading_HMC5883 < 0)
				SensorData->Heading_HMC5883 += 360;
		}
		//Eventually will use this as the heading.
		MagHMC5883Sensor->X = MATH
		*/
	}

	return OK;
}

void TWB_IMU_HMC5883_SendDiag(void){
	_commonDiagBuffer->SensorId = MagHMC5883Sensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)_hmc5883XFilter.Current;
	_commonDiagBuffer->RawY = (int16_t)_hmc5883YFilter.Current;
	_commonDiagBuffer->RawZ = (int16_t)_hmc5883ZFilter.Current;

	_commonDiagBuffer->X = (int16_t)MagHMC5883Sensor->X;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
