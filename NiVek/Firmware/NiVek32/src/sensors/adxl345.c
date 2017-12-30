/*
 * adxl345.c
 *
 *  Created on: Nov 22, 2012
 *      Author: kevinw
 */

#include "sensors/adxl345.h"
#include "twb_config.h"
#include "common/twb_i2c.h"
#include "common/twb_debug.h"
#include "twb_eeprom.h"
#include "main.h"
#include "common/twb_serial_eeprom.h"
#include "sensors/snsr_main.h"
#include "filters/Filter.h"
#include "commo/commo.h"

//#define ADXL345_ADDR 0x1D
#define ADXL345_ADDR 0x53

#define ADXL345_X0  0x32
#define ADXL345_X1  0x33
#define ADXL345_Y0  0x34
#define ADXL345_Y1  0x35
#define ADXL345_Z0  0x36
#define ADXL345_Z1  0x37

#define ADXL345_BW_RATE_ADDR      0x2C
#define ADXL345_POWER_CTL_ADDR    0x2D
#define ADXL345_DATA_FORMAT_ADDR  0x31

#define ADXL345_BW_RATE_DEFAULT 0b00001011
// D7, D6, D5 (N/A)   => 000 => Set to zero for normal operation
// D4 Low Power       => 0 => 0 for normal operation (not much point saving uA's when the ESC's suck up > 1 amp.
// D3-D0 Data Rate    => 1011 => 200Hz Operating in normal operation

#define ADXL345_POWER_CTL 0b00001000 // 1 in fourth position sets the device to normal operating mode.
// D7, D6 (N/A)       => 00 => Set to zero for normal operation
// D5 (Link)          => 0 => Really used for tap/activity detection
// D3 (Measure)       => 1 => Put in measurement mode.
// D2 (Sleep)         => 0 => Keep out of sleep mode.
// D1, D0 (Wakeup)    => 00 => Sample frequence in sleep mode.

#define ADXL345_DATA_FORMAT_DEFAULT 0b00001000
// D7 (Self Test)     => 0 => Nope
// D7 (SPI Bit)       => 0 => N/A for I2C
// D6 (INT_Invert)    => 0 => For now ADXL345 not IRQ driven
// D4 (N/A)           => 0 => Set to zero for normal operation
// D3 (Full Res.)     => 1 => Yup go for full resolution
// D2 (Justify)       => 0 => Right justify with sign extension
// D1, D0 (Range Bits)=> 00 => +/- 2G

s16_vector_t __adxlAcc_offset;
s16_vector_t __adxlAcc_currentRaw;

Filter_t _adxlXFilter;
Filter_t _adxlYFilter;
Filter_t _adxlZFilter;

iOpResult_e TWB_IMU_ADXL345_Init(void) {
	assert_succeed(TWB_SE_ReadS16(ADXL345_EEPROM_OFFSET + 0, &__adxlAcc_offset.x));
	assert_succeed(TWB_SE_ReadS16(ADXL345_EEPROM_OFFSET + 2, &__adxlAcc_offset.y));
	assert_succeed(TWB_SE_ReadS16(ADXL345_EEPROM_OFFSET + 4, &__adxlAcc_offset.z));

	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 6, &SensorConfig->ADXL345_Enabled));
	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 7, &SensorConfig->ADXL345_BW_RATE));
	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 8, &SensorConfig->ADXL345_DATA_FORMAT));
	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 9, &SensorConfig->ADXL345_FIFO_Enable));
	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 10, &SensorConfig->ADXL345_FilterOption));
	assert_succeed(TWB_SE_ReadU8(ADXL345_EEPROM_OFFSET + 11, &SensorConfig->ADXL345_SampleRate));

	AccADXL345Sensor->SampleRate = SensorConfig->ADXL345_SampleRate;
	AccADXL345Sensor->IsEnabled = SensorConfig->ADXL345_Enabled;

	_adxlXFilter.FilterType = SensorConfig->ADXL345_FilterOption;
	_adxlYFilter.FilterType = SensorConfig->ADXL345_FilterOption;
	_adxlZFilter.FilterType = SensorConfig->ADXL345_FilterOption;

	if(SensorConfig->ADXL345_Enabled == Enabled){
		uint8_t outValue = 0x00;
		assert_succeed(TWB_I2C_ReadRegisterValue(ADXL345_ADDR, 0x00, &outValue));

		if(outValue != 0xE5){
			TWB_Debug_Print("!!Could not configure ADXL345.\r\n");
			return FAIL;
		}

		assert_succeed(TWB_I2C_WriteRegister(ADXL345_ADDR, ADXL345_BW_RATE_ADDR, SensorConfig->ADXL345_BW_RATE));
		assert_succeed(TWB_I2C_WriteRegister(ADXL345_ADDR, ADXL345_DATA_FORMAT_ADDR, SensorConfig->ADXL345_DATA_FORMAT));
		assert_succeed(TWB_I2C_WriteRegister(ADXL345_ADDR, ADXL345_POWER_CTL_ADDR, ADXL345_POWER_CTL));

		TWB_Debug_Print("ADXL345-");
	}

	return OK;
}

iOpResult_e TWB_IMU_ADXL345_ResetDefaults(void) {
	__adxlAcc_offset.x = 0;
	__adxlAcc_offset.y = 0;
	__adxlAcc_offset.z = 0;

	SensorConfig->ADXL345_Enabled = Disabled;
	SensorConfig->ADXL345_BW_RATE = ADXL345_BW_RATE_DEFAULT;
	SensorConfig->ADXL345_DATA_FORMAT = ADXL345_DATA_FORMAT_DEFAULT;
	SensorConfig->ADXL345_FIFO_Enable = Disabled;
	SensorConfig->ADXL345_FilterOption = FilterOption_None;
	SensorConfig->ADXL345_SampleRate = SampleRate_100Hz;

	AccADXL345Sensor->SampleRate = SensorConfig->ADXL345_SampleRate;
	AccADXL345Sensor->IsEnabled = SensorConfig->ADXL345_Enabled;

	assert_succeed(TWB_SE_WriteS16Pause(ADXL345_EEPROM_OFFSET + 0, __adxlAcc_offset.x));
	assert_succeed(TWB_SE_WriteS16Pause(ADXL345_EEPROM_OFFSET + 2, __adxlAcc_offset.y));
	assert_succeed(TWB_SE_WriteS16Pause(ADXL345_EEPROM_OFFSET + 4, __adxlAcc_offset.z));

	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 6, SensorConfig->ADXL345_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 7, SensorConfig->ADXL345_BW_RATE));
	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 8, SensorConfig->ADXL345_DATA_FORMAT));
	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 9, SensorConfig->ADXL345_FIFO_Enable));
	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 10, SensorConfig->ADXL345_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(ADXL345_EEPROM_OFFSET + 11, SensorConfig->ADXL345_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_ADXL345_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			SensorConfig->ADXL345_Enabled = value;

			if(SensorConfig->ADXL345_Enabled == Enabled){
				assert_succeed(TWB_IMU_ADXL345_Init());
			}
			else
				AccADXL345Sensor->IsEnabled = Disabled;
		break;
		case 7:
			SensorConfig->ADXL345_BW_RATE = value;
			assert_succeed(TWB_I2C_WriteRegister(ADXL345_ADDR, ADXL345_BW_RATE_ADDR, SensorConfig->ADXL345_BW_RATE));
			break;
		case 8:
			SensorConfig->ADXL345_DATA_FORMAT = value;
			assert_succeed(TWB_I2C_WriteRegister(ADXL345_ADDR, ADXL345_DATA_FORMAT_ADDR, SensorConfig->ADXL345_DATA_FORMAT));
			break;
		case 9:
			SensorConfig->ADXL345_FilterOption = value;
			_adxlXFilter.FilterType = value;
			_adxlYFilter.FilterType = value;
			_adxlZFilter.FilterType = value;

			_adxlXFilter.IsInitialized = false;
			_adxlYFilter.IsInitialized = false;
			_adxlZFilter.IsInitialized = false;

			_adxlXFilter.CurrentSlot= 0;
			_adxlYFilter.CurrentSlot= 0;
			_adxlZFilter.CurrentSlot= 0;
			break;
		case 10:
			SensorConfig->ADXL345_SampleRate = value;
			AccADXL345Sensor->SampleRate = SensorConfig->ADXL345_SampleRate;
			break;
	}

	return OK;
}


//TODO
bool TWB_IMU_ADXL345_Zero(uint8_t sampleSize, uint8_t pauseMs) {
	assert_succeed(TWB_Sensor_Calibrate(ADXL345_ADDR, ADXL345_X0, ADXL345_EEPROM_OFFSET + 0x00, sampleSize, pauseMs, &__adxlAcc_offset, true));

	return false;
}


iOpResult_e TWB_IMU_ADXL345_Read(float deltaT){
	assert_succeed(TWB_I2C_ReadRegisterValues(ADXL345_ADDR, ADXL345_X0, __commonSensorBuffer, 6));

	TWB_Filter_Median_Apply(&_adxlXFilter, (__commonSensorBuffer[1] << 8 | __commonSensorBuffer[0]) /*- __adxlAcc_offset.x*/);
	TWB_Filter_Median_Apply(&_adxlYFilter, (__commonSensorBuffer[3] << 8 | __commonSensorBuffer[2]) /*- __adxlAcc_offset.y*/);
	TWB_Filter_Median_Apply(&_adxlZFilter, (__commonSensorBuffer[5] << 8 | __commonSensorBuffer[4]) /*- __adxlAcc_offset.z*/);

	AccADXL345Sensor->X = _adxlXFilter.Current;
	AccADXL345Sensor->Y = _adxlYFilter.Current;
	AccADXL345Sensor->Z = _adxlZFilter.Current;

	return OK;
}

void TWB_IMU_ADXL345_SendDiag(void){
	_commonDiagBuffer->SensorId = AccADXL345Sensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)_adxlXFilter.Current;
	_commonDiagBuffer->RawY = (int16_t)_adxlYFilter.Current;
	_commonDiagBuffer->RawZ = (int16_t)_adxlZFilter.Current;

	_commonDiagBuffer->X = (int16_t)AccADXL345Sensor->X;
	_commonDiagBuffer->Y = (int16_t)AccADXL345Sensor->Y;
	_commonDiagBuffer->Z = (int16_t)AccADXL345Sensor->Z;


	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
