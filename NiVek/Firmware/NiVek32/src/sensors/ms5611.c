/*
 * ms5611.c
 *
 *  Created on: Jul 28, 2013
 *      Author: Kevin
 */

#include "sensors/ms5611.h"

#include "main.h"
#include "filters/Filter.h"
#include "sensors/snsr_main.h"
#include "twb_eeprom.h"
#include "twb_config.h"
#include "common/twb_serial_eeprom.h"
#include "common/memory_mgmt.h"

#include "common/twb_debug.h"
#include "common/twb_i2c.h"


// registers of the device
#define MS561101BA_D1 0x40
#define MS561101BA_D2 0x50
#define MS561101BA_RESET 0x1E

// D1 and D2 result size (bytes)
#define MS561101BA_D1D2_SIZE 3

// OSR (Over Sampling Ratio) constants
#define MS561101BA_OSR_256 0x00
#define MS561101BA_OSR_512 0x02
#define MS561101BA_OSR_1024 0x04
#define MS561101BA_OSR_2048 0x06
#define MS561101BA_OSR_4096 0x08

#define MS5611_CMD_READ_TEMPERATURE_OSR_4096 0x58
#define MS5611_CMD_READ_PRESSURE_OSR_4096 0x48

#define MS561101BA_PROM_BASE_ADDR 0xA2

#define MS561101BA_CAL_SIZE 2
#define MS561101BA_CAL_LEN 6

#define MS5611_DEFAULT_ZERO_SCALER 1000.0f


int32_t __ms5611_defaultAltitudeZero;
float __ms5611_altitudeZero;

FilterFloat_t *__ms5611Filter;


uint32_t __ms5611_rawTemperature;
int32_t __ms5611_deltaTemperature;

uint32_t __ms5611_rawPressure;

float __ms5611_pressure;
float __ms5611_temperature;

float __ms5611_altitudeRaw;
float __ms5611_altitudeM;
float __ms5611_altitudeZero;

I2CJob_t *MS5611I2CJob;


typedef enum {
	MS5611_Uninitialized,
	MS5611_Idle,
	MS5611_PendingTemperatureConversion,
	MS5611_PendingPressureConversion,
	MS5611_PendingTemperatureRead,
	MS5611_PendingPressureRead,
} MS5611ReadStates_e;

MS5611ReadStates_e __ms5611ReadState;

uint8_t __ms5611_sensorReading = 0;

MS5611CalParams_t *__ms5611Calib;
#define CONVERSION_TIME 10000l /* conversion time in microseconds 10 MS */

#define MS5611_ID2C_ADDR 0x76

iOpResult_e reset(void) {
	assert_succeed(TWB_I2C_SendU8(MS5611_ID2C_ADDR, MS561101BA_RESET));

	/* Sleep 5ms after resetting */
	TWB_Timer_SleepMS(5);

	TWB_Debug_Print("\r\nAfter sleeping after reset\r\n");

	return OK;
}


#define kPa2AltPower 		0.1902949571836346f
#define PressureAtSeaLevel 	1013.25f
#define ScalingFactor 		44330

float __snsr_MS5611_GetAltitude(float pressure)
{
   float A = (pressure / PressureAtSeaLevel);

   return 44330.0f * (1.0f - powf(A, kPa2AltPower)) * 0.3048f;
}

iOpResult_e __imu_ms5611_readCalibration(void) {
	reset();

	/* MS5611 will only clock back individual U16's we can just read the array at once */
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (0 * MS561101BA_CAL_SIZE), &__ms5611Calib->c1));
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (1 * MS561101BA_CAL_SIZE), &__ms5611Calib->c2));
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (2 * MS561101BA_CAL_SIZE), &__ms5611Calib->c3));
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (3 * MS561101BA_CAL_SIZE), &__ms5611Calib->c4));
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (4 * MS561101BA_CAL_SIZE), &__ms5611Calib->c5));
	assert_succeed(TWB_I2C_ReadU16RegisterValue(MS5611_ID2C_ADDR, MS561101BA_PROM_BASE_ADDR + (5 * MS561101BA_CAL_SIZE), &__ms5611Calib->c6));

	TWB_Debug_Print("\r\nMS5611 Calibration Data\r\n");
	TWB_Debug_PrintInt("C1: ", __ms5611Calib->c1);
	TWB_Debug_PrintInt("C2: ", __ms5611Calib->c2);
	TWB_Debug_PrintInt("C3: ", __ms5611Calib->c3);
	TWB_Debug_PrintInt("C4: ", __ms5611Calib->c4);
	TWB_Debug_PrintInt("C5: ", __ms5611Calib->c5);
	TWB_Debug_PrintInt("C6: ", __ms5611Calib->c6);
	TWB_Debug_Print("-------\r\n");

	return OK;
}


iOpResult_e __twb_imu_ms5611_init_Periph(void){
	__imu_ms5611_readCalibration();
	return OK;
}

iOpResult_e TWB_IMU_MS5611_Init(void) {
	__ms5611Calib = (MS5611CalParams_t *)pm_malloc(sizeof(MS5611CalParams_t));

	assert_succeed(TWB_SE_ReadU8(MS5611_EEPROM_OFFSET + 0, &SensorConfig->MS5611_Enabled));
	assert_succeed(TWB_SE_ReadU8(MS5611_EEPROM_OFFSET + 1, &SensorConfig->MS5611_OverSamplingRate));
	assert_succeed(TWB_SE_ReadU8(MS5611_EEPROM_OFFSET + 2, &SensorConfig->MS5611_FilterOption));
	assert_succeed(TWB_SE_ReadU8(MS5611_EEPROM_OFFSET + 3, &SensorConfig->MS5611_SampleRate));

	if(SensorConfig->MS5611_SampleRate < SampleRate_500Hz)
		SensorConfig->MS5611_SampleRate = SampleRate_500Hz;

		if(SensorConfig->MS5611_SampleRate > SampleRate_1Hz)
			SensorConfig->MS5611_SampleRate = SampleRate_1Hz;

	assert_succeed(TWB_SE_ReadS32(MS5611_EEPROM_OFFSET + 4, &__ms5611_defaultAltitudeZero));

	TWB_Debug_PrintInt("\r\nMS5611 Zero -> ", __ms5611_defaultAltitudeZero);

	__ms5611_altitudeZero = (float)__ms5611_defaultAltitudeZero / MS5611_DEFAULT_ZERO_SCALER;

	__ms5611_altitudeRaw = 0.0f;
	__ms5611_altitudeM = 0.0f;

	__ms5611_pressure = 0;
	__ms5611_temperature = 0;

	__ms5611ReadState = MS5611_Uninitialized;

	MS5611I2CJob = (I2CJob_t * )pm_malloc(sizeof(I2CJob_t));
	MS5611I2CJob->Address = MS5611_ID2C_ADDR;
	MS5611I2CJob->OutBuffer = pm_malloc(sizeof(2));
	MS5611I2CJob->InBuffer = pm_malloc(sizeof(3));

	__ms5611Filter = pm_malloc(sizeof(FilterFloat_t));
	__ms5611Filter->FilterType = SensorConfig->MS5611_FilterOption;
	__ms5611Filter->IsInitialized = false;
	__ms5611Filter->Current = 0.0f;
	__ms5611Filter->CurrentSlot = 0;
	__ms5611Filter->Previous = 0.0f;

	if(SensorConfig->MS5611_Enabled == Enabled)
		__twb_imu_ms5611_init_Periph();

	PressureMS5611Sensor->SampleRate = SensorConfig->MS5611_SampleRate;
	PressureMS5611Sensor->IsEnabled = SensorConfig->MS5611_Enabled;

	PressureMS5611Sensor->ProcessData = &TWB_IMU_MS5611_ProcessData;
	PressureMS5611Sensor->Read = &TWB_IMU_MS5611_Read;

	PressureMS5611Sensor->StartZero = &TWB_IMU_MS5611_Zero;

	PressureMS5611Sensor->Start = &TWB_IMU_MS5611_Start;
	PressureMS5611Sensor->Stop = &TWB_IMU_MS5611_Stop;
	PressureMS5611Sensor->Pause = &TWB_IMU_MS5611_Pause;
	PressureMS5611Sensor->Resume = &TWB_IMU_MS5611_Resume;

	return OK;
}

iOpResult_e TWB_IMU_MS5611_ResetDefaults(void){
	SensorConfig->MS5611_Enabled = Disabled;
	SensorConfig->MS5611_OverSamplingRate = Disabled;
	SensorConfig->MS5611_FilterOption = FilterOption_None;
	SensorConfig->MS5611_SampleRate = SampleRate_100Hz;

	__ms5611_defaultAltitudeZero = 0;

	assert_succeed(TWB_SE_WriteU32Pause(MS5611_EEPROM_OFFSET + 4, __ms5611_defaultAltitudeZero));

	return OK;
}

iOpResult_e TWB_IMU_MS5611_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 0:
		SensorConfig->MS5611_Enabled = value;
		PressureMS5611Sensor->IsEnabled = value;
		if(SensorConfig->MS5611_Enabled == Enabled)
			assert_succeed(__twb_imu_ms5611_init_Periph());

		break;
	case 1:
		SensorConfig->MS5611_OverSamplingRate = value;
		break;
	case 2:
		SensorConfig->MS5611_FilterOption = value;
		__ms5611Filter->FilterType = value;

		__ms5611Filter->IsInitialized = false;
		__ms5611Filter->Current = 0.0f;
		__ms5611Filter->CurrentSlot = 0;
		__ms5611Filter->Previous = 0.0f;

		break;
	case 3:
		SensorConfig->MS5611_SampleRate = value;
		PressureMS5611Sensor->SampleRate = SensorConfig->MS5611_SampleRate;
		break;
	}

	return OK;
}

bool TWB_IMU_MS5611_Zero(uint8_t sampleSize, float pauseMS){
	__ms5611_altitudeZero = __ms5611_altitudeRaw;

	__ms5611_defaultAltitudeZero = (int32_t)(__ms5611_altitudeRaw * MS5611_DEFAULT_ZERO_SCALER);

	TWB_Debug_PrintInt("\r\nMS5611 Zero -> ", __ms5611_defaultAltitudeZero);

	I2CZeroJob->Address = EEPROM_I2C_ADDR;
	I2CZeroJob->OutBufferSize = 6;
	I2CZeroJob->InBufferSize = 0;
	I2CZeroJob->Callback = 0x00;

	I2CZeroJob->OutBuffer[0] = (MS5611_EEPROM_OFFSET + 4) >> 8;
	I2CZeroJob->OutBuffer[1] = (MS5611_EEPROM_OFFSET + 4) & 0xFF;
	I2CZeroJob->OutBuffer[2] = __ms5611_defaultAltitudeZero >> 24;
	I2CZeroJob->OutBuffer[3] = __ms5611_defaultAltitudeZero >> 16;
	I2CZeroJob->OutBuffer[4] = __ms5611_defaultAltitudeZero >> 8;
	I2CZeroJob->OutBuffer[5] = __ms5611_defaultAltitudeZero & 0xFF;

	TWB_I2C_QueueAsyncJob(I2CZeroJob);

	return true;
}


iOpResult_e __ms5611_readRawTemperature(void) {
	if(MS5611I2CJob->Result == OK)
		__ms5611_rawTemperature = MS5611I2CJob->InBuffer[0] << 16 | MS5611I2CJob->InBuffer[1] << 8 | MS5611I2CJob->InBuffer[2];
	else
		TWB_Debug_Print("t");

	return OK;
}

int32_t _temp_sub;

iOpResult_e __ms5611_processTemperature(void) {
	_temp_sub = ((int32_t)__ms5611Calib->c5) << 8;

	__ms5611_deltaTemperature =  __ms5611_rawTemperature - _temp_sub;
	__ms5611_temperature = (2000 + (__ms5611_deltaTemperature * (__ms5611Calib->c6 * 1.19209e-07))) / 100.0f;

	InternalState->Temperature = __ms5611_temperature;

	return OK;
}

iOpResult_e __ms5611_readRawPressure(void) {
	if(MS5611I2CJob->Result == OK)
		__ms5611_rawPressure = MS5611I2CJob->InBuffer[0] << 16 | MS5611I2CJob->InBuffer[1] << 8 | MS5611I2CJob->InBuffer[2];
	else
		TWB_Debug_Print("p");

	return OK;
}

iOpResult_e __ms5611_processPressure(void) {
	int64_t off  = ((uint32_t)__ms5611Calib->c2 <<16) + (((int64_t)__ms5611_deltaTemperature * __ms5611Calib->c4) >> 7);
	int64_t sens = ((uint32_t)__ms5611Calib->c1 <<15) + (((int64_t)__ms5611_deltaTemperature * __ms5611Calib->c3) >> 8);

	float unfilteredPressure = ((( (__ms5611_rawPressure * sens ) >> 21) - off) >> 15) / 100.0f;


	TWB_Filter_Median_Apply_Float(__ms5611Filter, unfilteredPressure);

	__ms5611_pressure = __ms5611Filter->Current;

	InternalState->Pressure_hPa = __ms5611_pressure;

	__ms5611_altitudeRaw = __snsr_MS5611_GetAltitude(__ms5611_pressure);
	__ms5611_altitudeM = __ms5611_altitudeRaw - __ms5611_altitudeZero;

	AltitudeData->MS5611 = (int16_t)(__ms5611_altitudeM * 10.0f);

	PressureMS5611Sensor->Z = __ms5611_altitudeM;
	PressureMS5611Sensor->X = __ms5611_altitudeRaw;
	PressureMS5611Sensor->Y = __ms5611_temperature;

	return OK;
}


/*
 * Next set of code will be used to manage the process of
 * communicating with the MS5611, code above this will do
 * the actual conversions
 */
void TWB_IMU_MS5611_I2C_Completed(void *i2cJob){
	PressureMS5611Sensor->DataReady = DataReady;
}

iOpResult_e __imu_ms5611_startTemperatureConversion(void){
	__ms5611ReadState = MS5611_PendingTemperatureConversion;
	MS5611I2CJob->OutBuffer[0] =MS5611_CMD_READ_TEMPERATURE_OSR_4096;
	MS5611I2CJob->OutBufferSize = 1;
	MS5611I2CJob->InBufferSize = 0;
	MS5611I2CJob->Callback = null;

	return TWB_I2C_QueueAsyncJob(MS5611I2CJob);
}

iOpResult_e __imu_ms5611_startPressureConversion(void){
	__ms5611ReadState = MS5611_PendingPressureConversion;

	MS5611I2CJob->OutBuffer[0] = MS5611_CMD_READ_PRESSURE_OSR_4096;
	MS5611I2CJob->OutBufferSize = 1;
	MS5611I2CJob->InBufferSize = 0;
	MS5611I2CJob->Callback = null;

	return TWB_I2C_QueueAsyncJob(MS5611I2CJob);
}

iOpResult_e __imu_ms5611_readTemperature(void) {
	__ms5611_readRawTemperature();
	__ms5611_processTemperature();

	__ms5611_sensorReading = 0;
	__imu_ms5611_startPressureConversion();
	return OK;
}

iOpResult_e __imu_ms5611_startTemperatureRead(void) {
	__ms5611ReadState = MS5611_PendingTemperatureRead;

	MS5611I2CJob->OutBuffer[0] = 0x00;
	MS5611I2CJob->OutBufferSize = 1;
	MS5611I2CJob->InBufferSize = 3;
	MS5611I2CJob->Callback = &TWB_IMU_MS5611_I2C_Completed;

	return TWB_I2C_QueueAsyncJob(MS5611I2CJob);
}

iOpResult_e __imu_ms5611_readPressure(void) {
	__ms5611_readRawPressure();
	__ms5611_processPressure();

	if(__ms5611_sensorReading++ == 3)
		__imu_ms5611_startTemperatureConversion();
	else
		__imu_ms5611_startPressureConversion();

	return OK;
}

iOpResult_e __imu_ms5611_startPressureRead(void) {
	__ms5611ReadState = MS5611_PendingPressureRead;

	MS5611I2CJob->OutBuffer[0] = 0x00;
	MS5611I2CJob->OutBufferSize = 1;
	MS5611I2CJob->InBufferSize = 3;
	MS5611I2CJob->Callback = &TWB_IMU_MS5611_I2C_Completed;

	return TWB_I2C_QueueAsyncJob(MS5611I2CJob);
}

/*
 * So the MS5611 has a conversion time of 10ms at OSR of 4096 (get cleanest from device)
 * So our process is as follows
 *
 * 1) The first time through we send a command to get temperature (don't read any registers)
 * 2) Next time we read that temperature, once that is available, we read it and send a
 *    command to start pressure reading.
 * 3) We read the pressure, then, send another command to read the pressure.
 * 4) Repeat 3 for 99 cycles, then perform a cycle to read the temperature
 * 5) We then switch back to reading pressure.  This way we get 99 pressure readings a second
 *    but also get a temperature witch is probably much more static.
 *
 */



/*
 * Get's called at the sample rate
 * - First time through, we start a temperature conversion, when that is complete it will start a pressure conversion.
 * - Next time through it will start a read from I2C of the temperature, then in the ProcessData method it will take those values and calculate temperature
 *   it will then start a pressure conversion.
 *
 */
iOpResult_e TWB_IMU_MS5611_Read(float deltaT){
	switch(__ms5611ReadState){
		case MS5611_Uninitialized: __imu_ms5611_startTemperatureConversion(); break;
		case MS5611_PendingTemperatureConversion:  __imu_ms5611_startTemperatureRead(); break;
		case MS5611_PendingPressureConversion: __imu_ms5611_startPressureRead(); break;

		default: /*NOP*/ break;
	}

	return OK;
}

/*
 * This get's called once the I2C job completes it operates on the "MainThread" so
 * we can actually parse and do something with the raw data;
 */
iOpResult_e TWB_IMU_MS5611_ProcessData(void){
	switch(__ms5611ReadState){
		case MS5611_PendingTemperatureRead:  __imu_ms5611_readTemperature(); break;
		case MS5611_PendingPressureRead: __imu_ms5611_readPressure(); break;

		default: /*NOP*/ break;
	}

	return OK;
}


void TWB_IMU_MS5611_SendDiag(void){
	_commonDiagBuffer->SensorId = PressureMS5611Sensor->SensorId;

	_commonDiagBuffer->RawX = 0;
	_commonDiagBuffer->RawY = 0;
	_commonDiagBuffer->RawZ = 0;

	_commonDiagBuffer->X = (int16_t)(__ms5611_altitudeRaw * 10.0f);
	_commonDiagBuffer->Y = (int16_t)((__ms5611_pressure - 1000.0f) * 100.0f);
	_commonDiagBuffer->Z = (int16_t)(__ms5611_temperature * 10.0f);

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}

iOpResult_e TWB_IMU_MS5611_Start(void){
	return OK;
}

iOpResult_e TWB_IMU_MS5611_Stop(void){
	return OK;
}

iOpResult_e TWB_IMU_MS5611_Pause(void){
	return OK;
}

iOpResult_e TWB_IMU_MS5611_Resume(void){
	return OK;
}




