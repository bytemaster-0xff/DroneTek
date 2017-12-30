#include "sensors/bmp085.h"
#include "stdlib.h"

#include "main.h"
#include "filters/Filter.h"
#include "sensors/snsr_main.h"
#include "twb_eeprom.h"
#include "twb_config.h"

#include "common/twb_serial_eeprom.h"
#include "common/memory_mgmt.h"

#include "common/twb_debug.h"
#include "common/twb_i2c.h"

#define BMP085_CHIP_ID			0x55
#define BOSCH_PRESSURE_BMP085	85

#define BMP085_I2C_ADDR		(0xEE>>1)

#define BMP085_PROM_START__ADDR		0xaa
#define BMP085_PROM_DATA__LEN		  22

BMP085Calib_t __bmp085Calib;

I2CJob_t *BMPI2CJob;

long __bmp085ParamB5;

FilterFloat_t *__bmp085Filter;

float __bmp085_pressure;
float __bmp085_temperature;

float __bmp085_altitudeRaw;
float __bmp085_altitudeM;
float __bmp085_altitudeZero;

int16_t __bmp085_defaultAltitudeZero;

uint32_t _bmp085_RawTemperature;

#define BMP_085_DEFAULT_ZERO_SCALER 10.0f

typedef enum {
	BMP085_Idle,
	BMP085_PendingTemperature,
	BMP085_PendingPressure
} BMP085ReadStates_e;

BMP085ReadStates_e __bmp085ReadState;


void TWB_IMU_BMP085_I2C_Completed(void *job);

iOpResult_e bmp085_get_cal_param(void){
  unsigned char data[22];
  assert_succeed(TWB_I2C_ReadRegisterValues(BMP085_I2C_ADDR, BMP085_PROM_START__ADDR, data, BMP085_PROM_DATA__LEN));

  __bmp085Calib.ac1 =  (data[0] <<8) | data[1];
  __bmp085Calib.ac2 =  (data[2] <<8) | data[3];
  __bmp085Calib.ac3 =  (data[4] <<8) | data[5];
  __bmp085Calib.ac4 =  (data[6] <<8) | data[7];
  __bmp085Calib.ac5 =  (data[8] <<8) | data[9];
  __bmp085Calib.ac6 = (data[10] <<8) | data[11];

  __bmp085Calib.b1 =  (data[12] <<8) | data[13];
  __bmp085Calib.b2 =  (data[14] <<8) | data[15];

  __bmp085Calib.mb =  (data[16] <<8) | data[17];
  __bmp085Calib.mc =  (data[18] <<8) | data[19];
  __bmp085Calib.md =  (data[20] <<8) | data[21];

  /*
  TWB_Debug_Print("BMP085 Calibration Data\r\n");
  TWB_Debug_PrintInt("AC1: ", __bmp085Calib.ac1);
  TWB_Debug_PrintInt("AC2: ", __bmp085Calib.ac2);
  TWB_Debug_PrintInt("AC3: ", __bmp085Calib.ac3);
  TWB_Debug_PrintInt("AC4: ", __bmp085Calib.ac4);
  TWB_Debug_PrintInt("AC5: ", __bmp085Calib.ac5);
  TWB_Debug_PrintInt("AC6: ", __bmp085Calib.ac6);

  TWB_Debug_PrintInt("B1: ", __bmp085Calib.b1);
  TWB_Debug_PrintInt("B2: ", __bmp085Calib.b2);

  TWB_Debug_PrintInt("MB: ", __bmp085Calib.mb);
  TWB_Debug_PrintInt("MC: ", __bmp085Calib.mc);
  TWB_Debug_PrintInt("MD: ", __bmp085Calib.md);
  TWB_Debug_Print("-------\r\n");
  */
  return OK;
}

iOpResult_e __twb_imu_bmp085_init_Periph(void){
	//TODO: Compare Chip ID
	uint8_t data;
	assert_succeed(TWB_I2C_ReadRegisterValue(BMP085_I2C_ADDR, BMP085_CHIP_ID__REG, &data));
	assert_succeed(TWB_I2C_ReadRegisterValue(BMP085_I2C_ADDR, BMP085_VERSION_REG, &data));
	assert_succeed(bmp085_get_cal_param());

	TWB_Debug_Print("BMP085-");

	return OK;
}

iOpResult_e TWB_IMU_BMP085_Init(void){
	assert_succeed(TWB_SE_ReadU8(BMP085_EEPROM_OFFSET + 0, &SensorConfig->BMP085_Enabled));
	assert_succeed(TWB_SE_ReadU8(BMP085_EEPROM_OFFSET + 1, &SensorConfig->BMP085_NumberSamples));
	assert_succeed(TWB_SE_ReadU8(BMP085_EEPROM_OFFSET + 2, &SensorConfig->BMP085_OverSamplingRate));
	assert_succeed(TWB_SE_ReadU8(BMP085_EEPROM_OFFSET + 3, &SensorConfig->BMP085_FilterOption));
	assert_succeed(TWB_SE_ReadU8(BMP085_EEPROM_OFFSET + 4, &SensorConfig->BMP085_SampleRate));

	assert_succeed(TWB_SE_ReadS16(BMP085_EEPROM_OFFSET + 5, &__bmp085_defaultAltitudeZero));

	TWB_Debug_PrintInt("Default Zero -> ", __bmp085_defaultAltitudeZero);

	__bmp085_altitudeZero = (float)__bmp085_defaultAltitudeZero / BMP_085_DEFAULT_ZERO_SCALER;

	PressureBMP085Sensor->SampleRate = SensorConfig->BMP085_SampleRate;
	PressureBMP085Sensor->IsEnabled = SensorConfig->BMP085_Enabled;

	PressureBMP085Sensor->ProcessData = &TWB_IMU_BMP085_ProcessData;
	PressureBMP085Sensor->Read = &TWB_IMU_BMP085_Read;
	PressureBMP085Sensor->HandleIRQ = &TWB_IMU_BMP085_IRQ;

	PressureBMP085Sensor->StartZero = &TWB_IMU_BMP085_Zero;

	PressureBMP085Sensor->Start = &TWB_IMU_BMP085_Start;
	PressureBMP085Sensor->Stop = &TWB_IMU_BMP085_Stop;
	PressureBMP085Sensor->Pause = &TWB_IMU_BMP085_Pause;
	PressureBMP085Sensor->Resume = &TWB_IMU_BMP085_Resume;

	__bmp085Filter = pm_malloc(sizeof(FilterFloat_t));
	__bmp085Filter->FilterType = SensorConfig->BMP085_FilterOption;
	__bmp085Filter->IsInitialized = false;
	__bmp085Filter->Current = 0.0f;
	__bmp085Filter->CurrentSlot = 0;
	__bmp085Filter->Previous = 0.0f;

	BMPI2CJob = pm_malloc(sizeof(I2CJob_t));
	BMPI2CJob->OutBuffer = pm_malloc(sizeof(2));
	BMPI2CJob->InBuffer = pm_malloc(sizeof(3));

	if(PressureBMP085Sensor->IsEnabled == Enabled)
		assert_succeed(__twb_imu_bmp085_init_Periph());

	return OK;
}

bool TWB_IMU_BMP085_Zero(uint8_t sampleSize, float pauseMS){
	__bmp085_altitudeZero = __bmp085_altitudeRaw;

	__bmp085_defaultAltitudeZero = (int16_t)(__bmp085_altitudeRaw * BMP_085_DEFAULT_ZERO_SCALER);

	I2CZeroJob->Address = EEPROM_I2C_ADDR;
	I2CZeroJob->OutBufferSize = 4;
	I2CZeroJob->InBufferSize = 0;
	I2CZeroJob->Callback = 0x00;

	I2CZeroJob->OutBuffer[0] = (BMP085_EEPROM_OFFSET + 5) >> 8;
	I2CZeroJob->OutBuffer[1] = (BMP085_EEPROM_OFFSET + 5) & 0xFF;
	I2CZeroJob->OutBuffer[2] = __bmp085_defaultAltitudeZero >> 8;
	I2CZeroJob->OutBuffer[3] = __bmp085_defaultAltitudeZero & 0xFF;

	TWB_I2C_QueueAsyncJob(I2CZeroJob);

	return true;
}

iOpResult_e TWB_IMU_BMP085_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
	case 0:
		SensorConfig->BMP085_Enabled = value;
		PressureBMP085Sensor->IsEnabled = value;
		if(SensorConfig->BMP085_Enabled == Enabled)
			assert_succeed(__twb_imu_bmp085_init_Periph());

		break;
	case 1:
		SensorConfig->BMP085_NumberSamples = value;
		break;
	case 2:
		SensorConfig->BMP085_OverSamplingRate = value;
		break;
	case 3:
		SensorConfig->BMP085_FilterOption = value;
		__bmp085Filter->FilterType = value;

		__bmp085Filter->IsInitialized = false;
		__bmp085Filter->Current = 0.0f;
		__bmp085Filter->CurrentSlot = 0;
		__bmp085Filter->Previous = 0.0f;

		break;
	case 4:
		SensorConfig->BMP085_SampleRate = value;
		PressureBMP085Sensor->SampleRate = SensorConfig->BMP085_SampleRate;
		break;
	}

	return OK;
}

/* Conversion Times
 *
 * Temperature 			4.5 ms
 * 		Over Sample 0 		4.5 ms
 * 		Over Sample 1 		7.5 ms
 * 		Over Sample 2 		13.5 ms
 * 		Over Sample 3 		25.5 ms*
 *
 *   So with Temperature and Sample each time we have 31 ms max update rate would be 33Hz
 */




iOpResult_e TWB_IMU_BMP085_ResetDefaults(void){
	SensorConfig->BMP085_Enabled = Disabled;
	SensorConfig->BMP085_NumberSamples = 1;
	SensorConfig->BMP085_OverSamplingRate = 3;
	SensorConfig->BMP085_FilterOption = FilterOption_None;
	SensorConfig->BMP085_SampleRate = SampleRate_5Hz;

	PressureBMP085Sensor->SampleRate = SensorConfig->BMP085_SampleRate;
	PressureBMP085Sensor->IsEnabled = SensorConfig->BMP085_Enabled;

	__bmp085_defaultAltitudeZero = 0;

	assert_succeed(TWB_SE_WriteU8Pause(BMP085_EEPROM_OFFSET + 0, SensorConfig->BMP085_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(BMP085_EEPROM_OFFSET + 1, SensorConfig->BMP085_NumberSamples));
	assert_succeed(TWB_SE_WriteU8Pause(BMP085_EEPROM_OFFSET + 2, SensorConfig->BMP085_OverSamplingRate));
	assert_succeed(TWB_SE_WriteU8Pause(BMP085_EEPROM_OFFSET + 3, SensorConfig->BMP085_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(BMP085_EEPROM_OFFSET + 4, SensorConfig->BMP085_SampleRate));
	assert_succeed(TWB_SE_WriteS32Pause(BMP085_EEPROM_OFFSET + 5, __bmp085_defaultAltitudeZero));

	return OK;
}

iOpResult_e TWB_IMU_BMP085_Start(void) {
	__bmp085Filter->IsInitialized = false;
	__bmp085Filter->CurrentSlot = 0;

	return OK;
}

iOpResult_e TWB_IMU_BMP085_Stop(void) {
	return OK;
}

iOpResult_e TWB_IMU_BMP085_Pause(void) {

	return OK;
}
iOpResult_e TWB_IMU_BMP085_Resume(void) {
	__bmp085Filter->IsInitialized = false;
	__bmp085Filter->CurrentSlot = 0;

	return OK;
}

iOpResult_e __snsr_bmp085_startTemperatureConversion(void);
iOpResult_e __snsr_bmp085_startPressureConversion(void);


/* Step 1 Request a read from the Temperature via Async I2C */
iOpResult_e TWB_IMU_BMP085_Read(float deltaT){
	assert_succeed(__snsr_bmp085_startTemperatureConversion());

	return OK;
}

iOpResult_e __snsr_bmp085_startTemperatureConversion(void){
	BMPI2CJob->Address = BMP085_I2C_ADDR;
	BMPI2CJob->OutBuffer[0] = BMP085_CTRL_MEAS_REG;
	BMPI2CJob->OutBuffer[1] = BMP085_T_MEASURE;

	BMPI2CJob->OutBufferSize = 2;
	BMPI2CJob->InBufferSize = 0;
	BMPI2CJob->Callback = null;

	TWB_I2C_QueueAsyncJob(BMPI2CJob);

	__bmp085ReadState = BMP085_PendingTemperature;

	return OK;
}

iOpResult_e __snsr_bmp085_startPressureConversion(void){
	BMPI2CJob->Address = BMP085_I2C_ADDR;
	BMPI2CJob->OutBuffer[0] = BMP085_CTRL_MEAS_REG;
	BMPI2CJob->OutBuffer[1] = BMP085_P_MEASURE + (SensorConfig->BMP085_OverSamplingRate<<6);

	BMPI2CJob->OutBufferSize = 2;
	BMPI2CJob->InBufferSize = 0;
	BMPI2CJob->Callback = null;

	TWB_I2C_QueueAsyncJob(BMPI2CJob);

	__bmp085ReadState = BMP085_PendingPressure;

	return OK;
}

/*
 * Step 2 BMP085 signals the IRQ that conversion is completed
 *        mark data ready to be read.
 */
void EXTI3_IRQHandler(void){
	if((GPIOC->IDR & GPIO_Pin_3) == GPIO_Pin_3)
		PressureBMP085Sensor->IRQAsserted = IRQAsserted;

	EXTI_ClearITPendingBit(EXTI_Line3);
}

/*
 * At EOC start an I2C job to read in the temperature and/or pressure.
 */
iOpResult_e TWB_IMU_BMP085_IRQ(void){
	BMPI2CJob->Address = BMP085_I2C_ADDR;
	BMPI2CJob->OutBuffer[0] = BMP085_ADC_OUT_MSB_REG;
	BMPI2CJob->OutBufferSize = 1;

	BMPI2CJob->InBufferSize = __bmp085ReadState == BMP085_PendingTemperature ? 2 : 3; //Ready in 2 bytes for temperature and 3 for pressure
	BMPI2CJob->Callback = &TWB_IMU_BMP085_I2C_Completed;

	TWB_I2C_QueueAsyncJob(BMPI2CJob);

   return OK;
}

/*
 * Step 4- Read has been completed, this comes in an
 *         I2C IRQ, so signal the sensor that data
 *         us ready.
 */
void TWB_IMU_BMP085_I2C_Completed(void *i2cJob){
	PressureBMP085Sensor->DataReady = DataReady;
}

iOpResult_e __snsr_BMP085_ProcessTemperature(void){
   _bmp085_RawTemperature = (uint32_t)((BMPI2CJob->InBuffer[0] << 8) | BMPI2CJob->InBuffer[1]);

	long x1,x2;

	x1 = (((long) _bmp085_RawTemperature - (long) __bmp085Calib.ac6) * (long)__bmp085Calib.ac5) >> 15;
	x2 = ((long) __bmp085Calib.mc << 11) / (x1 + __bmp085Calib.md);
	__bmp085ParamB5 = x1 + x2;

//	__bmp085ParamB5 = 4000;

	uint32_t  bmp_085_temperature = ((__bmp085ParamB5 + 8) >> 4);  // temperature in 0.1°C
	__bmp085_temperature = bmp_085_temperature * 0.1f;


	__bmp085ReadState = BMP085_PendingPressure;

	return OK;
}

#define kPa2AltPower 		0.1902949571836346f
#define PressureAtSeaLevel 	1013.25f
#define ScalingFactor 		44330

float __snsr_BMP085_GetAltitude(float pressure)
{
   float A = (pressure / PressureAtSeaLevel);

   return 44330.0f * (1.0f - powf(A, kPa2AltPower)) * 0.3048f;
}

iOpResult_e __snsr_BMP085_ProcessPressure(void) {
   uint32_t up = ((BMPI2CJob->InBuffer[0] << 16) | (BMPI2CJob->InBuffer[1] << 8) | BMPI2CJob->InBuffer[2]) >> (8-SensorConfig->BMP085_OverSamplingRate);

   uint32_t bmp085_pressure;

   int x1, x2, x3, b3, b6;
   unsigned int b4, b7;

   /*
    * Oh joy, a bunch of calculations that I have no idea what they do...other than what the MFG told
    * us they need to, they seem to work so oh-well.
    */

   b6 = __bmp085ParamB5 - 4000;
   // Calculate B3
   x1 = (__bmp085Calib.b2 * (b6 * b6)>>12)>>11;
   x2 = (__bmp085Calib.ac2 * b6)>>11;
   x3 = x1 + x2;
   b3 = (((((int)__bmp085Calib.ac1)*4 + x3) << SensorConfig->BMP085_OverSamplingRate) + 2)>>2;

   // Calculate B4
   x1 = (__bmp085Calib.ac3 * b6)>>13;
   x2 = (__bmp085Calib.b1 * ((b6 * b6)>>12))>>16;
   x3 = ((x1 + x2) + 2)>>2;
   b4 = (__bmp085Calib.ac4 * (uint32_t)(x3 + 32768))>>15;

   b7 = ((uint32_t)(up - b3) * (50000>>SensorConfig->BMP085_OverSamplingRate));
   if (b7 < 0x80000000)
	   bmp085_pressure = (b7<<1)/b4;
   else
	   bmp085_pressure = (b7/b4)<<1;

   x1 = (bmp085_pressure>>8) * (bmp085_pressure>>8);
   x1 = (x1 * 3038)>>16;
   x2 = (-7357 * bmp085_pressure)>>16;

   bmp085_pressure += (x1 + x2 + 3791)>>4;

   float unfilteredPressure = bmp085_pressure * 0.01f;

   TWB_Filter_Median_Apply_Float(__bmp085Filter, unfilteredPressure);

   __bmp085_pressure = __bmp085Filter->Current;

   InternalState->Pressure_hPa = __bmp085_pressure;
   InternalState->Temperature = __bmp085_temperature;

   __bmp085_altitudeRaw = __snsr_BMP085_GetAltitude(__bmp085_pressure);
   __bmp085_altitudeM = __bmp085_altitudeRaw - __bmp085_altitudeZero;

   AltitudeData->BMP085 = (int16_t)(__bmp085_altitudeM * 10.0f);

   PressureBMP085Sensor->Z = __bmp085_altitudeM;
   PressureBMP085Sensor->X = __bmp085_altitudeRaw;
   PressureBMP085Sensor->Y = __bmp085_temperature;

   __bmp085ReadState = BMP085_Idle;

   return OK;
}

iOpResult_e TWB_IMU_BMP085_ProcessData(void){
	if(__bmp085ReadState == BMP085_PendingTemperature){
		assert_succeed(__snsr_BMP085_ProcessTemperature());
		__snsr_bmp085_startPressureConversion();
	}
	else if(__bmp085ReadState == BMP085_PendingPressure)
		assert_succeed(__snsr_BMP085_ProcessPressure());

	return OK;
}


void TWB_IMU_BMP085_SendDiag(void){
	_commonDiagBuffer->SensorId = PressureBMP085Sensor->SensorId;

	_commonDiagBuffer->RawX = 0;
	_commonDiagBuffer->RawY = 0;
	_commonDiagBuffer->RawZ = 0;

	_commonDiagBuffer->X = (int16_t)(__bmp085_altitudeRaw * 10.0f);
	_commonDiagBuffer->Y = (int16_t)((__bmp085_pressure - 1000.0f) * 100.0f);
	_commonDiagBuffer->Z = (int16_t)(__bmp085_temperature * 10.0f);

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
