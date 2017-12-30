/*
 * lsm303.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "sensors/lsm303_mag.h"
#include "twb_config.h"
#include "main.h"
#include "common/memory_mgmt.h"
#include "twb_eeprom.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_i2c.h"
#include "common/twb_debug.h"
#include "common/twb_timer.h"
#include "common/twb_math.h"
#include "sensors/snsr_main.h"
#include "Filters/Filter.h"
#include "sensors/lsm303_acc.h"

#include <math.h>

I2CJob_t *__writeCalibrationFactorsI2CJob;

/* Start the defines for the Magnetometer */
#define LSM303_Mag_Address 0x1E

#define LSM303_MAG_CRA_ADDR         0x00
#define LSM303_MAG_CRB_ADDR         0x01
#define LSM303_MAG_MR_ADDR          0x02

#define LSM303_MAG_OUT_X_H_ADDR     0x03
#define LSM303_MAG_OUT_X_L_ADDR     0x04
#define LSM303_MAG_OUT_Z_H_ADDR     0x05
#define LSM303_MAG_OUT_Z_L_ADDR     0x06
#define LSM303_MAG_OUT_Y_H_ADDR     0x07
#define LSM303_MAG_OUT_Y_L_ADDR     0x08

#define LSM303_MAG_CRA_DEFAULT 0b00011100
// D7 (Temperature Enable) => 0 => No temps
// D6, D5 (N/A)            => 00 => Set to 0 for correct operation
// D4, D3, D2 (Output Rate)=> 111 => 220 Hz
// D1, D0 (N/A)            => 00 => Set to 0 for correct operation

#define LSM303_MAG_CRB_DEFAULT 0b10000000
// D7, D6, D5 (Gain)       => 100 => +/4 4 Ga
// D4, D3, D2, D1, D0 (N/A)=> 00000 => Set to zero for proper operation


#define LSM303_MAG_MR_DEFAULT  0b00000000
// D7-D2 (N/A)             => 000000 => Set to zero for proper operation
// D1, D0 (Mode)           => 00 => Continuous operation (01 Single Conversion, 10 & 11 Sleep Mode)
/* ---------------------------------- */


#define __MATH_PI 3.141592f


I2CJob_t * LSM303MagI2CJob;

s16_vector_t __lsm303MagMin;
s16_vector_t __lsm303MagMax;

Filter_t __lsm303MagXFilter;
Filter_t __lsm303MagYFilter;
Filter_t __lsm303MagZFilter;

FilterSigned_t __lsm303CalibrationMagXFilter;
FilterSigned_t __lsm303CalibrationMagYFilter;
FilterSigned_t __lsm303CalibrationMagZFilter;


float _lsm303FullScaleRange;

void __twb_imu_lsm303_mag_setFullScaleRange(void){
	uint8_t setting = (SensorConfig->LSM303_MAG_CRB & 0b11100000 >> 5);
	switch(setting){
		case 0: _lsm303FullScaleRange = 1.3f; break;
		case 1: _lsm303FullScaleRange = 1.9f; break;
		case 2: _lsm303FullScaleRange = 2.5f; break;
		case 3: _lsm303FullScaleRange = 4.0f; break;
		case 4: _lsm303FullScaleRange = 4.7f; break;
		case 5: _lsm303FullScaleRange = 5.6f; break;
		case 6: _lsm303FullScaleRange = 8.1f; break;
	}
}

//Stuff we could care about for filtering on the LSM303
// REG1_A ODR3-ODR0
// REG2_A HPCF2-HPCF1
// REG4_A Full Scale Selection

// CRA_REG Gain Settings (magnometer seems to sometimes saturate the setting)

iOpResult_e  __twb_imu_lsm303_mag_init_Periph(void){
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_CRA_ADDR, SensorConfig->LSM303_MAG_CRA));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_CRB_ADDR, SensorConfig->LSM303_MAG_CRB));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_MR_ADDR, SensorConfig->LSM303_MAG_MR));
	TWB_Debug_Print("LSM303.MAG-");

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_Init(void){
	//By default, these all start out as -1 in the EEPROM, should be a good
	//starting point for the offset.
	LSM303MagI2CJob = pm_malloc(sizeof(I2CJob_t));
	LSM303MagI2CJob->OutBuffer = pm_malloc(1);
	LSM303MagI2CJob->InBuffer = pm_malloc(6);

	__writeCalibrationFactorsI2CJob = pm_malloc(sizeof(I2CJob_t));
	__writeCalibrationFactorsI2CJob->OutBuffer = pm_malloc(15); /* 12 for calibration, 1 for calibrated 2 for address */

	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 0, &SensorConfig->LSM303_MAG_Calibrated));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 1, &__lsm303MagMin.x));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 3, &__lsm303MagMin.y));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 5, &__lsm303MagMin.z));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 7, &__lsm303MagMax.x));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 9, &__lsm303MagMax.y));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 11, &__lsm303MagMax.z));

	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 13, &SensorConfig->LSM303_MAG_Enabled));
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 14, &SensorConfig->LSM303_MAG_CRA));
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 15, &SensorConfig->LSM303_MAG_CRB));
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 16, &SensorConfig->LSM303_MAG_MR));
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 17, &SensorConfig->LSM303_MAG_FilterOption));
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 18, &SensorConfig->LSM303_MAG_SampleRate));

	MagLSM303Sensor->SampleRate = SensorConfig->LSM303_MAG_SampleRate;
	MagLSM303Sensor->IsEnabled = SensorConfig->LSM303_MAG_Enabled;
 	__lsm303MagXFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;
	__lsm303MagYFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;
	__lsm303MagZFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;

	__lsm303CalibrationMagXFilter.FilterType = FilterOption_Median_9_Sample | FilterOption_Moving_Average_30;
	__lsm303CalibrationMagYFilter.FilterType = FilterOption_Median_9_Sample | FilterOption_Moving_Average_30;
	__lsm303CalibrationMagZFilter.FilterType = FilterOption_Median_9_Sample | FilterOption_Moving_Average_30;

	MagLSM303Sensor->ProcessData = &TWB_IMU_LSM303_Mag_ProcessData;
	MagLSM303Sensor->Read = &TWB_IMU_LSM303_Mag_Read;

	MagLSM303Sensor->Start = &TWB_IMU_LSM303_Mag_Start;
	MagLSM303Sensor->Stop = &TWB_IMU_LSM303_Mag_Stop;
	MagLSM303Sensor->Pause = &TWB_IMU_LSM303_Mag_Pause;
	MagLSM303Sensor->Resume = &TWB_IMU_LSM303_Mag_Resume;

	__twb_imu_lsm303_mag_setFullScaleRange();

	if(SensorConfig->LSM303_MAG_Enabled == Enabled)
		assert_succeed(__twb_imu_lsm303_mag_init_Periph());

	return OK;
}

iOpResult_e  TWB_IMU_LSM303_Mag_ResetDefaults(void){
	__lsm303MagMin.x = 10000;
	__lsm303MagMin.y = 10000;
	__lsm303MagMin.z = 10000;
	__lsm303MagMax.x = -10000;
	__lsm303MagMax.y = -10000;
	__lsm303MagMax.z = -10000;

	SensorConfig->LSM303_MAG_Enabled = Enabled;
	SensorConfig->LSM303_MAG_Calibrated = NotCalibrated;
	SensorConfig->LSM303_MAG_CRA = LSM303_MAG_CRA_DEFAULT;
	SensorConfig->LSM303_MAG_CRB = LSM303_MAG_CRB_DEFAULT;
	SensorConfig->LSM303_MAG_MR = LSM303_MAG_MR_DEFAULT;
	SensorConfig->LSM303_MAG_FilterOption = FilterOption_Median_9_Sample | FilterOption_Moving_Average_30;

	SensorConfig->LSM303_MAG_SampleRate = SampleRate_100Hz;

	MagLSM303Sensor->SampleRate = SensorConfig->LSM303_MAG_SampleRate;
	MagLSM303Sensor->IsEnabled = SensorConfig->LSM303_MAG_Enabled;

	__lsm303MagXFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;
	__lsm303MagYFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;
	__lsm303MagZFilter.FilterType = SensorConfig->LSM303_MAG_FilterOption;

	//TODO : Could potentially just write an array for these values.
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 0, SensorConfig->LSM303_MAG_Calibrated));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 1, __lsm303MagMin.x));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 3, __lsm303MagMin.y));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 5, __lsm303MagMin.z));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 7, __lsm303MagMax.x));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 9, __lsm303MagMax.y));
	assert_succeed(TWB_SE_WriteS16Pause(LSM303_MAG_EEPROM_OFFSET + 11, __lsm303MagMax.z));

	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 13, SensorConfig->LSM303_MAG_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 14, SensorConfig->LSM303_MAG_CRA));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 15, SensorConfig->LSM303_MAG_CRB));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 16, SensorConfig->LSM303_MAG_MR));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 17, SensorConfig->LSM303_MAG_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_MAG_EEPROM_OFFSET + 18, SensorConfig->LSM303_MAG_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_UpdateSetting(uint16_t addr, uint8_t value) {
	switch(addr){
	case 13:
		SensorConfig->LSM303_MAG_Enabled = value;
		MagLSM303Sensor->IsEnabled = value;
		if(SensorConfig->LSM303_MAG_Enabled == Enabled)
			assert_succeed(__twb_imu_lsm303_mag_init_Periph());

		break;
	case 14:
		SensorConfig->LSM303_MAG_CRA = value;
		assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_CRA_ADDR, SensorConfig->LSM303_MAG_CRA));
		break;
	case 15:
		SensorConfig->LSM303_MAG_CRB = value;
		assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_CRB_ADDR, SensorConfig->LSM303_MAG_CRB));
		__twb_imu_lsm303_mag_setFullScaleRange();
		break;
	case 16:
		SensorConfig->LSM303_MAG_MR = value;
		assert_succeed(TWB_I2C_WriteRegister(LSM303_Mag_Address, LSM303_MAG_MR_ADDR, SensorConfig->LSM303_MAG_MR));
		break;
	case 17:
		SensorConfig->LSM303_MAG_FilterOption = value;
		__lsm303MagXFilter.FilterType = value;
		__lsm303MagYFilter.FilterType = value;
		__lsm303MagZFilter.FilterType = value;

		break;
	case 18:
		SensorConfig->LSM303_MAG_SampleRate = value;
		MagLSM303Sensor->SampleRate = SensorConfig->LSM303_MAG_SampleRate;
		break;
	}

	return OK;

}

iOpResult_e TWB_IMU_LSM303_Mag_Start(void){
	__lsm303MagXFilter.IsInitialized = false;
	__lsm303MagYFilter.IsInitialized = false;
	__lsm303MagZFilter.IsInitialized = false;

	__lsm303MagXFilter.CurrentSlot = 0;
	__lsm303MagYFilter.CurrentSlot = 0;
	__lsm303MagZFilter.CurrentSlot = 0;

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_Stop(void) {

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_Pause(void) {

	return OK;
}
iOpResult_e TWB_IMU_LSM303_Mag_Resume(void) {
	__lsm303MagXFilter.IsInitialized = false;
	__lsm303MagYFilter.IsInitialized = false;
	__lsm303MagZFilter.IsInitialized = false;

	__lsm303MagXFilter.CurrentSlot = 0;
	__lsm303MagYFilter.CurrentSlot = 0;
	__lsm303MagZFilter.CurrentSlot = 0;

	return OK;
}

//Upon starting magnetic calibration, set these values
//to be max and mins so as we go through the calibration phase
// the device will write out the correct values.
iOpResult_e TWB_IMU_LSM303_Mag_BeginCalibration(){
	__lsm303CalibrationMagXFilter.IsInitialized = false;
	__lsm303CalibrationMagYFilter.IsInitialized = false;
	__lsm303CalibrationMagZFilter.IsInitialized = false;
	__lsm303CalibrationMagXFilter.CurrentSlot = 0;
	__lsm303CalibrationMagYFilter.CurrentSlot = 0;
	__lsm303CalibrationMagZFilter.CurrentSlot = 0;

	__lsm303MagMin.x = 10000;
	__lsm303MagMin.y = 10000;
	__lsm303MagMin.z = 10000;

	__lsm303MagMax.x = -10000;
	__lsm303MagMax.y = -10000;
	__lsm303MagMax.z = -10000;

//	SensorConfig->LSM303_MAG_Calibrated = NotCalibrated;

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_CancelCalibration(void){
	assert_succeed(TWB_SE_ReadU8(LSM303_MAG_EEPROM_OFFSET + 0, &SensorConfig->LSM303_MAG_Calibrated));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 1, &__lsm303MagMin.x));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 3, &__lsm303MagMin.y));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 5, &__lsm303MagMin.z));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 7, &__lsm303MagMax.x));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 9, &__lsm303MagMax.y));
	assert_succeed(TWB_SE_ReadS16(LSM303_MAG_EEPROM_OFFSET + 11, &__lsm303MagMax.z));

	return OK;
}

void __imu_LSM303_MagCalWritten(void *result){
	SystemStatus->SnsrState = SensorAppState_Online;
}

iOpResult_e TWB_IMU_LSM303_Mag_EndCalibration(void){
	SystemStatus->SnsrState = SensorAppState_FinishingCalibration;

	SensorConfig->LSM303_MAG_Calibrated = Calibrated;

	__writeCalibrationFactorsI2CJob->OutBufferSize = 15;
	__writeCalibrationFactorsI2CJob->InBufferSize = 0;

	__writeCalibrationFactorsI2CJob->OutBuffer[0]= (uint8_t)(LSM303_MAG_EEPROM_OFFSET >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[1]= (uint8_t)(LSM303_MAG_EEPROM_OFFSET & 0xFF);

	__writeCalibrationFactorsI2CJob->OutBuffer[2]= SensorConfig->LSM303_MAG_Calibrated;

	__writeCalibrationFactorsI2CJob->OutBuffer[3]= (uint8_t)(__lsm303MagMin.x >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[4]= (uint8_t)(__lsm303MagMin.x & 0xFF);
	__writeCalibrationFactorsI2CJob->OutBuffer[5]= (uint8_t)(__lsm303MagMin.y >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[6]= (uint8_t)(__lsm303MagMin.y & 0xFF);
	__writeCalibrationFactorsI2CJob->OutBuffer[7]= (uint8_t)(__lsm303MagMin.z >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[8]= (uint8_t)(__lsm303MagMin.z & 0xFF);

	__writeCalibrationFactorsI2CJob->OutBuffer[9]= (uint8_t)(__lsm303MagMax.x >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[10]= (uint8_t)(__lsm303MagMax.x & 0xFF);
	__writeCalibrationFactorsI2CJob->OutBuffer[11]= (uint8_t)(__lsm303MagMax.y >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[12]= (uint8_t)(__lsm303MagMax.y & 0xFF);
	__writeCalibrationFactorsI2CJob->OutBuffer[13]= (uint8_t)(__lsm303MagMax.z >> 8);
	__writeCalibrationFactorsI2CJob->OutBuffer[14]= (uint8_t)(__lsm303MagMax.z & 0xFF);
	__writeCalibrationFactorsI2CJob->Callback = null;

	assert_succeed(TWB_SE_QueueAsyncWrite(__writeCalibrationFactorsI2CJob));

	return OK;
}




float _tmpX;
float _tmpY;
float _tmpZ;

int16_t _rawX;
int16_t _rawY;
int16_t _rawZ;

int16_t _deltaX;
int16_t _deltaY;
int16_t _deltaZ;

uint16_t _rangeX;
uint16_t _rangeY;
uint16_t _rangeZ;

float _ratioX;
float _ratioY;
float _ratioZ;
float _fheading;
float _fatanresult;
float _fratio;

void __twb_imu_lsm303_mag_I2CReadComplete(void *i2cjob) {
	MagLSM303Sensor->DataReady = DataReady;
}


vector_t N;
vector_t E;

vector_t from;

vector_t m;

vector_t acc;

vector_t acc_norm;

float _xComponent;
float _yComponent;
float _zComponent;


float _otherHeading;

float cosRoll, sinRoll, cosPitch, sinPitch;
float rollRadians, pitchRadians;

float Xh;
float Yh;

float tempHeading;

float tempArcTan;
float tanRatio;

/* Algorithm adapted from:
 * https://www.loveelectronics.co.uk/Tutorials/13/tilt-compensated-compass-arduino-tutorial
 */

float __MagPitch;
float __MagRoll;
float __MagDenom;
float accxnorm;
float accynorm;


float magxcomp;
float magycomp;

float __imu_lsm303_mag_getTiltHeading(void){
	//TWB_Debug_TimerStart();
  // see appendix A in app note AN3192

//  pitchRadians = toRadians(InternalState->Pitch);
//  rollRadians = toRadians(InternalState->Roll);

	  // We cannot correct for tilt over 40 degrees with this algorthem, if the board is tilted as such, return 0.
	/*if(rollRadians > 0.78f || rollRadians < -0.78f || pitchRadians > 0.78f || pitchRadians < -0.78f)
	{
		_otherHeading=0;
		return -1;
	}*/

	__MagPitch =  -toRadians(InternalState->Pitch);
	__MagRoll = toRadians(InternalState->Roll);

    // Some of these are used twice, so rather than computing them twice in the algorithm we pre-compute them before hand.
    cosRoll = twb_cosf(__MagRoll);
    sinRoll = twb_sinf(__MagRoll);
    cosPitch = twb_cosf(__MagPitch);
    sinPitch = twb_sinf(__MagPitch);

    float magxcomp = _ratioX*twb_cosf(__MagPitch)+_ratioZ*twb_sinf(__MagPitch);
    float magycomp = _ratioX*twb_sinf(__MagRoll) * twb_sinf(__MagPitch)+_ratioY*twb_cosf(__MagRoll)-_ratioZ*twb_sinf(__MagRoll)*twb_cosf(__MagPitch);

    tempArcTan = twb_atan2f(magycomp, magxcomp);
    tempHeading = toDegrees(tempArcTan);

	if (tempHeading < 0)
		tempHeading += 360.0f;

    //TWB_Debug_TimerEnd();

	return tempHeading;
}



iOpResult_e TWB_IMU_LSM303_Mag_ProcessData(void){
	/* At this point, our buffer is full with data from the I2C Read */

	_rawX = (int16_t)(LSM303MagI2CJob->InBuffer[0] << 8 | LSM303MagI2CJob->InBuffer[1]);
	_rawY = (int16_t)(LSM303MagI2CJob->InBuffer[4] << 8 | LSM303MagI2CJob->InBuffer[5]);
	_rawZ = (int16_t)(LSM303MagI2CJob->InBuffer[2] << 8 | LSM303MagI2CJob->InBuffer[3]);


	if(SystemStatus->SnsrState == SensorAppState_MagCalibration){
		TWB_Filter_Median_Apply_Signed(&__lsm303CalibrationMagXFilter, _rawX);
		TWB_Filter_Median_Apply_Signed(&__lsm303CalibrationMagYFilter, _rawY);
		TWB_Filter_Median_Apply_Signed(&__lsm303CalibrationMagZFilter, _rawZ);

		if(__lsm303CalibrationMagXFilter.IsInitialized == true
			&& __lsm303CalibrationMagYFilter.IsInitialized == true
			&& __lsm303CalibrationMagZFilter.IsInitialized == true){

			if(__lsm303CalibrationMagXFilter.Current < __lsm303MagMin.x) __lsm303MagMin.x = __lsm303CalibrationMagXFilter.Current;
			if(__lsm303CalibrationMagXFilter.Current > __lsm303MagMax.x) __lsm303MagMax.x = __lsm303CalibrationMagXFilter.Current;

			if(__lsm303CalibrationMagYFilter.Current < __lsm303MagMin.y) __lsm303MagMin.y = __lsm303CalibrationMagYFilter.Current;
			if(__lsm303CalibrationMagYFilter.Current > __lsm303MagMax.y) __lsm303MagMax.y = __lsm303CalibrationMagYFilter.Current;

			if(__lsm303CalibrationMagZFilter.Current < __lsm303MagMin.z) __lsm303MagMin.z = __lsm303CalibrationMagZFilter.Current;
			if(__lsm303CalibrationMagZFilter.Current > __lsm303MagMax.z) __lsm303MagMax.z = __lsm303CalibrationMagZFilter.Current;

			_rawX = __lsm303CalibrationMagXFilter.Current;
			_rawY = __lsm303CalibrationMagYFilter.Current;
			_rawZ = __lsm303CalibrationMagZFilter.Current;
		}
	}
	else{
		if(SensorConfig->LSM303_MAG_Calibrated == Calibrated){
			_deltaX = _rawX - __lsm303MagMin.x;
			_deltaY = _rawY - __lsm303MagMin.y;
			_deltaZ = _rawZ - __lsm303MagMin.z;

			if(_deltaX < 0) _deltaX = 0;
			if(_deltaY < 0) _deltaY = 0;
			if(_deltaZ < 0) _deltaZ = 0;

			/* delta's will always be positive */
			TWB_Filter_Median_Apply(&__lsm303MagXFilter,(uint16_t)_deltaX);
			TWB_Filter_Median_Apply(&__lsm303MagYFilter,(uint16_t)_deltaY);
			TWB_Filter_Median_Apply(&__lsm303MagZFilter,(uint16_t)_deltaZ);

			_tmpX = (float)(__lsm303MagXFilter.Current);
			_tmpY = (float)(__lsm303MagYFilter.Current);
			_tmpZ = (float)(__lsm303MagZFilter.Current);

			_rangeX = __lsm303MagMax.x - __lsm303MagMin.x;
			_rangeY = __lsm303MagMax.y - __lsm303MagMin.y;
			_rangeZ = __lsm303MagMax.z - __lsm303MagMin.z;

			_ratioX = _tmpX / _rangeX * 2.0f - 1.0f;
			_ratioY = _tmpY / _rangeY * 2.0f - 1.0f;
			_ratioZ = _tmpZ / _rangeZ * 2.0f - 1.0f;

			if(__lsm303MagMax.x == __lsm303MagMin.x)
				MagLSM303Sensor->Z = 0.0f;
			else{
				if(_ratioX == 0 && _ratioY == 0)
					MagLSM303Sensor->Z = -90.0f;
				else
					MagLSM303Sensor->Z = __imu_lsm303_mag_getTiltHeading();
			}
		}
		else
			MagLSM303Sensor->Z = 0.0f;
	}

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Mag_Read(float deltaT) {
	LSM303MagI2CJob->Address = LSM303_Mag_Address;
	LSM303MagI2CJob->OutBuffer[0] = LSM303_MAG_OUT_X_H_ADDR | 0x80;
	LSM303MagI2CJob->OutBufferSize = 1;
	LSM303MagI2CJob->InBufferSize = 6;
	LSM303MagI2CJob->Callback = &__twb_imu_lsm303_mag_I2CReadComplete;

	return TWB_I2C_QueueAsyncJob(LSM303MagI2CJob);
}

void TWB_IMU_LSM303_Mag_SendDiag(void){
	_commonDiagBuffer->SensorId = MagLSM303Sensor->SensorId;

	_commonDiagBuffer->X = (int)(_ratioX * 100.0f);
	_commonDiagBuffer->Y = (int)(_ratioY * 100.0f);
	_commonDiagBuffer->Z = (int)(_ratioZ * 100.0f);

	_commonDiagBuffer->RawX = _rawX;
	_commonDiagBuffer->RawY = _rawY;
	_commonDiagBuffer->RawZ = _rawZ;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
