


#include "sensors/snsr_main.h"
#include "twb_config.h"
#include "common/memory_mgmt.h"
#include "twb_eeprom.h"
#include "sensors/mpu60x0.h"
#include "filters/Filter.h"
#include "common/twb_i2c.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"


#define MPU60x0_IRQ_ENABLED  0b00000001
#define MPU60x0_IRQ_NOT_ENABLED  0b00000000
// D7 - N/A
// D6 - Motion Enable
// D5 - N/A
// D4 - FIFO_Overflow (not used)
// D3 - I2C Mstr IRQ (not used)
// D2 - D1 N/A
// D0 - DATA_RDY_EN

#define MPU60x0_DLPF_ADDR 0x1A
#define MPU60x0_GYRO_CFG_ADDR 0x1B

#define MPU60x0_SMPLRT_DIV_ADDR 0x19
#define MPU60x0_FIFO_CONFIG_ADDR 0x23
#define MPU60x0_FIFO_ENABLE_ADDR 0x6A
#define MPU60x0_PWR_MGMT_1_ADDR 0x6B

#define MPU_60x0_INT_CFG_ADDR 0x38
#define MPU60x0_PWR_MGMT2_ADDR 0x6C
#define MPU_60x0_GYRO_XOUT_H    0x43

#define MPU_60x0_FIFO_COUNT_H 0x72
#define MPU_60x0_FIFO_COUNT_L 0x73

#define MPU_60x0_FIFO_DATA_BUFFER 0x74

#define MPU_60x0_INT_CFG 0x38;

#define MPU60x0_DEFAULT_GYRO_FS 0b00010000
// D7, D6, D5 - Self Test/ not used set to 0
// D4, D3 - Full Scale -> 10 -> 1000 degrees/second
// D2, D1, D0 - Not Used, keep at zero.

#define MPU60x0_DEFAULT_DLPF 0b00000010
// D7, D6 => N/A
// D5, D4, D3 => EXT_SYNC_SET (not used) => 000
// D2, D1, D0 => DLP_CFG => 010 => 98 Hz, Internal Sample Rate 1KHz

#define MPU60x0_DEFAULT_SMPL_RT_DIV 04
// SMPLRT_DIV
// Not a bit mask field.
// We are not using LPF 256KHz so Internal Sample Rate will always be 1KHz
//
// Data Output Rate = (Internal Sample Rate) / (SMPLRT_DIV) + 1
//
// Default of 4 so 1KHz / (4 + 1) = 200Hz Update Rate.

#define MPU60x0_DEFAULT_GYRO_FIFO_CONFIG  0b01110000
#define MPU60x0_DEFAULT_GYRO_FIFO_ENABLE  0b01000000
#define MPU60x0_DEFAULT_GYRO_FIFO_RESET   0b00000100
#define MPU60x0_DEFAULT_GYRO_FIFO_DISABLE 0b00000000

#define MPU60x0_MAX_SLOTS 6
#define MPU60x0_SLOT_SIZE 6

Filter_t __mpuGyro60x0XFilter;
Filter_t __mpuGyro60x0YFilter;
Filter_t __mpuGyro60x0ZFilter;

s16_vector_t __mpu60x0GyroOffset;
GryoExtendedData_t *__mpu60x0GyroExtendedData;
FiFoState_e MPU60x0GyroFiFoState = FiFoState_Idle;

#define MPU60x0_SLEEP_DISABLED 0b00000000

//Measured in LSB Per Degrees/Second
float __lsbSensitivity;

void __setMPUGyroLSBSensitivity(uint8_t fs_sel){
	switch((fs_sel & 0b00011000) >> 3){
		case 0: __lsbSensitivity = 131.0f; break;
		case 1: __lsbSensitivity = 65.5f; break;
		case 2: __lsbSensitivity = 32.8f; break;
		case 3: __lsbSensitivity = 16.4f; break;
	}
}

I2CJob_t *MPU60x0GyroI2CJob;

iOpResult_e __twb_imu_MPU60x0_Gyro_InitPeriph(void){
	uint8_t outValue;
	assert_succeed(TWB_I2C_ReadRegisterValue(MPU60x0_Address, MPU60x0_WHO_AM_I_ADDR, &outValue));

	if(outValue != MPU60x0_WHO_AM_I_VALUE){
		TWB_Debug_Print("No MPU60x0 Gyro\r\n");
		return FAIL;
	}

	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_SMPLRT_DIV_ADDR, SensorConfig->MPU60x0_GYR_SMPLRT_DIV));
	/*
	 * Although the DLPF is primarily for the MPU60x0 Acc,
	 * we need to init when we do the gyro as well, since
	 * the Acc might not be enabled.
	 *
	 * Worst case scenario, we write this value twice.
	 *
	 * */
	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_DLPF_ADDR, SensorConfig->MPU60x0_ACC_GYR_DLPF));
	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_GYRO_CFG_ADDR, SensorConfig->MPU60x0_GYRO_FS));
	assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_PWR_MGMT_1_ADDR, MPU60x0_SLEEP_DISABLED));

	TWB_Debug_Print("MPU60x0.Gyro-");

	return OK;
}

void __mpu60x0_gyro_calculateScaleValue(void);

iOpResult_e TWB_IMU_MPU60x0_Gyro_Init() {
	assert_succeed(TWB_SE_ReadS16(MPU60x0_GYR_EEPROM_OFFSET + 0, &__mpu60x0GyroOffset.x));
	assert_succeed(TWB_SE_ReadS16(MPU60x0_GYR_EEPROM_OFFSET + 2, &__mpu60x0GyroOffset.y));
	assert_succeed(TWB_SE_ReadS16(MPU60x0_GYR_EEPROM_OFFSET + 4, &__mpu60x0GyroOffset.z))

	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 6, &SensorConfig->MPU60x0_Gyro_Enabled));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 7, &SensorConfig->MPU60x0_GYR_SMPLRT_DIV));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 8, &SensorConfig->MPU60x0_GYRO_FS))
	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 9, &SensorConfig->MPU60x0_FIFO_Enable));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 10, &SensorConfig->MPU60x0_GyroFilterOption));
	assert_succeed(TWB_SE_ReadU8(MPU60x0_GYR_EEPROM_OFFSET + 11, &SensorConfig->MPU60x0_GyroSampleRate));

	GyroMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_GyroSampleRate;
	GyroMPU60x0Sensor->IsEnabled = SensorConfig->MPU60x0_Gyro_Enabled;

	GyroMPU60x0Sensor->StartZero = &TWB_IMU_MPU60x0_Gyro_Zero;
	GyroMPU60x0Sensor->CompleteZero = &TWB_IMU_MPU60x0_Gyro_CompleteZero;

	GyroMPU60x0Sensor->Read = &TWB_IMU_MPU60x0_Gyro_Read;
	GyroMPU60x0Sensor->HandleIRQ = &TWB_IMU_MPU60x0_Gyro_IRQ;
	GyroMPU60x0Sensor->ProcessData = &TWB_IMU_MPU60x0_Gyro_ProcessData;

	GyroMPU60x0Sensor->Start = &TWB_IMU_MPU60x0_Gyro_Start;
	GyroMPU60x0Sensor->Stop = &TWB_IMU_MPU60x0_Gyro_Stop;
	GyroMPU60x0Sensor->Pause = &TWB_IMU_MPU60x0_Gyro_Pause;
	GyroMPU60x0Sensor->Resume = &TWB_IMU_MPU60x0_Gyro_Resume;

	__mpuGyro60x0XFilter.FilterType = SensorConfig->MPU60x0_GyroFilterOption;
	__mpuGyro60x0YFilter.FilterType = SensorConfig->MPU60x0_GyroFilterOption;
	__mpuGyro60x0ZFilter.FilterType = SensorConfig->MPU60x0_GyroFilterOption;

	__setMPUGyroLSBSensitivity(SensorConfig->MPU60x0_GYRO_FS);

	if(SensorConfig->MPU60x0_Gyro_Enabled == Enabled)
		assert_succeed(__twb_imu_MPU60x0_Gyro_InitPeriph());

	MPU60x0GyroI2CJob = pm_malloc(sizeof(I2CJob_t));
	MPU60x0GyroI2CJob->OutBuffer = pm_malloc(1);
	MPU60x0GyroI2CJob->InBuffer = pm_malloc(MPU60x0_SLOT_SIZE * MPU60x0_MAX_SLOTS);

	__mpu60x0GyroExtendedData = pm_malloc(sizeof(GryoExtendedData_t));
	GyroMPU60x0Sensor->Extended = __mpu60x0GyroExtendedData;

	__mpu60x0_gyro_calculateScaleValue();

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_ResetDefaults(void){
	SensorConfig->MPU60x0_Gyro_Enabled = Disabled;
	SensorConfig->MPU60x0_ACC_GYR_DLPF = MPU60x0_DEFAULT_DLPF;
	SensorConfig->MPU60x0_GYR_SMPLRT_DIV = MPU60x0_DEFAULT_SMPL_RT_DIV;
	SensorConfig->MPU60x0_GYRO_FS = MPU60x0_DEFAULT_GYRO_FS;
	SensorConfig->MPU60x0_FIFO_Enable = Disabled;
	SensorConfig->MPU60x0_GyroFilterOption = FilterOption_None;
	SensorConfig->MPU60x0_GyroSampleRate = SampleRate_200Hz;

	__setMPUGyroLSBSensitivity(SensorConfig->MPU60x0_GYRO_FS);

	GyroMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_GyroSampleRate;
	GyroMPU60x0Sensor->IsEnabled = SensorConfig->MPU60x0_Gyro_Enabled;

	__mpu60x0GyroOffset.x = 0;
	__mpu60x0GyroOffset.y = 0;
	__mpu60x0GyroOffset.z = 0;

	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_GYR_EEPROM_OFFSET + 0,__mpu60x0GyroOffset.x));
	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_GYR_EEPROM_OFFSET + 2,__mpu60x0GyroOffset.y));
	assert_succeed(TWB_SE_WriteS16Pause(MPU60x0_GYR_EEPROM_OFFSET + 4,__mpu60x0GyroOffset.z));

	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 6, SensorConfig->MPU60x0_Gyro_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 7, SensorConfig->MPU60x0_GYR_SMPLRT_DIV));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 8, SensorConfig->MPU60x0_GYRO_FS));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 9, SensorConfig->MPU60x0_FIFO_Enable));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 10, SensorConfig->MPU60x0_GyroFilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(MPU60x0_GYR_EEPROM_OFFSET + 11, SensorConfig->MPU60x0_GyroSampleRate));

	return OK;
}


bool TWB_IMU_MPU60x0_Gyro_Zero(uint8_t sampleSize, float pauseMS){
	assert_succeed(TWB_Sensor_Calibrate(MPU60x0_Address, MPU_60x0_GYRO_XOUT_H, MPU60x0_GYR_EEPROM_OFFSET + 0, sampleSize, pauseMS, &__mpu60x0GyroOffset, true));
	//Since we are integrating, we need to reset the output values.
	GyroMPU60x0Sensor->X = 0;
	GyroMPU60x0Sensor->Y = 0;
	GyroMPU60x0Sensor->Z = 0;

	return false;
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_CompleteZero(u16_vector_t *cal) {
	__mpu60x0GyroOffset.x = cal->x;
	__mpu60x0GyroOffset.y = cal->y;
	__mpu60x0GyroOffset.z = cal->z;

	return OK;
}



iOpResult_e TWB_IMU_MPU60x0_Gyro_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			SensorConfig->MPU60x0_Gyro_Enabled = value;
			GyroMPU60x0Sensor->IsEnabled = value;

			if(SensorConfig->MPU60x0_Gyro_Enabled == Enabled)
				assert_succeed(__twb_imu_MPU60x0_Gyro_InitPeriph());

			break;
		case 7:
			SensorConfig->MPU60x0_GYR_SMPLRT_DIV = value;
			assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_SMPLRT_DIV_ADDR, SensorConfig->MPU60x0_GYR_SMPLRT_DIV));
			__mpu60x0_gyro_calculateScaleValue();
			break;
		case 8:
			SensorConfig->MPU60x0_GYRO_FS = value;
			assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_GYRO_CFG_ADDR, SensorConfig->MPU60x0_GYRO_FS));
			__mpu60x0_gyro_calculateScaleValue();
			break;
		case 9:
			SensorConfig->MPU60x0_FIFO_Enable = value;
			break;
		case 10:
			SensorConfig->MPU60x0_GyroFilterOption = value;
			__mpuGyro60x0XFilter.FilterType = value;
			__mpuGyro60x0YFilter.FilterType = value;
			__mpuGyro60x0ZFilter.FilterType = value;
			break;
		case 11:
			SensorConfig->MPU60x0_GyroSampleRate = value;
			GyroMPU60x0Sensor->SampleRate = SensorConfig->MPU60x0_GyroSampleRate;
			break;
	}

	return OK;
}



iOpResult_e TWB_IMU_MPU60x0_Gyro_Start(void){
	if(SensorConfig->MPU60x0_FIFO_Enable == Enabled){
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_CONFIG_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_CONFIG));
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_ENABLE_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_ENABLE));
	}

	if(SensorConfig->MPU60x0_GyroSampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_ENABLE_ADDR, MPU60x0_IRQ_ENABLED));

	__mpuGyro60x0XFilter.CurrentSlot = 0;
	__mpuGyro60x0YFilter.CurrentSlot = 0;
	__mpuGyro60x0ZFilter.CurrentSlot = 0;

	__mpuGyro60x0XFilter.IsInitialized = false;
	__mpuGyro60x0YFilter.IsInitialized = false;
	__mpuGyro60x0ZFilter.IsInitialized = false;

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_Pause(void){
	if(SensorConfig->MPU60x0_FIFO_Enable == Enabled)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_CONFIG_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_DISABLE));

	if(SensorConfig->MPU60x0_GyroSampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU_60x0_INT_CFG_ADDR, MPU60x0_IRQ_NOT_ENABLED));

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_Resume(void){
	if(SensorConfig->MPU60x0_FIFO_Enable == Enabled){
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_CONFIG_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_CONFIG));
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_ENABLE_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_RESET));
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_ENABLE_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_ENABLE));
	}

	if(SensorConfig->MPU60x0_GyroSampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU_60x0_INT_CFG_ADDR, MPU60x0_IRQ_ENABLED));

	__mpuGyro60x0XFilter.CurrentSlot = 0;
	__mpuGyro60x0YFilter.CurrentSlot = 0;
	__mpuGyro60x0ZFilter.CurrentSlot = 0;

	__mpuGyro60x0XFilter.IsInitialized = false;
	__mpuGyro60x0YFilter.IsInitialized = false;
	__mpuGyro60x0ZFilter.IsInitialized = false;

	return OK;
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_Stop(void){
	if(SensorConfig->MPU60x0_FIFO_Enable == Enabled)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU60x0_FIFO_ENABLE_ADDR, MPU60x0_DEFAULT_GYRO_FIFO_DISABLE));

	if(SensorConfig->MPU60x0_GyroSampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(MPU60x0_Address, MPU_60x0_INT_CFG_ADDR, MPU60x0_IRQ_NOT_ENABLED));

	return OK;
}

void __twb_imu_mpu60x0_gyro_RegisterRead(void *job){
	GyroMPU60x0Sensor->DataReady = DataReady;
}

void __twb_imu_mpu60x0_gyro_QueueRead(void *job){
	MPU60x0GyroFiFoState = FiFoState_ReadQueue;
	GyroMPU60x0Sensor->DataReady = DataReady;
}

void __twb_imu_mpu60x0_gyro_QueueSizeRead(void *job){
	MPU60x0GyroFiFoState = FiFoState_ReadQueueSize;
	GyroMPU60x0Sensor->DataReady = DataReady;
}

void __twb_imu_mpu60x0_gyro_QueueFiFoRead(void){
	uint16_t sampleCount = (uint16_t)(MPU60x0GyroI2CJob->InBuffer[0] << 8 | MPU60x0GyroI2CJob->InBuffer[1]);

	MPU60x0GyroI2CJob->Address = MPU60x0_Address;
	MPU60x0GyroI2CJob->OutBufferSize = 1;
	MPU60x0GyroI2CJob->OutBuffer[0] = MPU_60x0_FIFO_DATA_BUFFER;
	MPU60x0GyroI2CJob->InBufferSize = MPU60x0_SLOT_SIZE * sampleCount;
	MPU60x0GyroI2CJob->Callback = &__twb_imu_mpu60x0_gyro_QueueRead;

	TWB_I2C_QueueAsyncJob(MPU60x0GyroI2CJob);

	MPU60x0GyroFiFoState = FiFoState_ReadingQueue;
}

float __mpu60x0_getDataRateSeconds(void){
	float dataRateSeconds = 0.00f;

	if(SensorConfig->MPU60x0_FIFO_Enable){
		uint8_t dlpf = SensorConfig->MPU60x0_ACC_GYR_DLPF & 0b00000111;
		float gyroOutputRate;
		if(dlpf == 0) /* DLPF is turned off gyro sample rate is 8KHz */
			gyroOutputRate = 8000.0f;
		else
			gyroOutputRate = 1000.0f;

		dataRateSeconds = 1.0f / (gyroOutputRate / (float)(SensorConfig->MPU60x0_GYR_SMPLRT_DIV + 1));
	}
	else
		dataRateSeconds = TWB_Timer_GetMSFromSampleRate(SensorConfig->MPU60x0_GyroSampleRate) / 1000.0f;

	return dataRateSeconds;
}

float __mpu60x0DataRateSeconds;
float __mpu60x0LSBSensitivity;
float __mpu60x0Factor;

void __mpu60x0_gyro_calculateScaleValue(void){
	uint8_t fsSetting = SensorConfig->MPU60x0_GYRO_FS >> 3 & 0b00000011;

	float fullScaleDPS = 0;

	switch(fsSetting){
		case 0b00: fullScaleDPS = 250.0f; break;
		case 0b01: fullScaleDPS = 500.0f; break;
		case 0b10: fullScaleDPS = 1000.0f; break;
		case 0b11: fullScaleDPS = 2000.0f; break;
	}

	__mpu60x0DataRateSeconds = __mpu60x0_getDataRateSeconds();

	/*
	 * Each register read is a 16 bit signed number
	 * this full scale for +/- is 1/2 of a 16 bit
	 * number or 0x8000;
	 */
	float fullScale = (float)0x8000;

	//The LSB is with full scale count / full scale DPS
	//EX: 32768 / 2000 = Each bit represents a rate of 16.384 BPS
	__mpu60x0LSBSensitivity = fullScale / fullScaleDPS;

	//This allows us to use for multiplication rather than division later ont.
	__mpu60x0LSBSensitivity = 1.0f / __mpu60x0LSBSensitivity;

	//Since we are reading the values at data rate predetermined by
	//the device when it stuffs a value into the FIFO, we don't have
	//to accurately measure timing in this sensor.  If we have a data rate
	//set at .001315 seconds that means that the angular distance each
	//integration period will be lsbSensivity * raw value * period
	//
	// Let's say our LSB value from the sensor is 300
	// Let's say our internal sample rate is 760 or .001315 seconds
	// Therefore our calculation to determine the distance for the angle is:
	// (300 (raw count) / 16.385(lsb)) * .00135 (period)
	// or
	// Factor = (LSB) / Period
	//  And our integration value over the period is 300 * (Factor)
	//
	// In the case of our sample our factor is ~ 0.0000803
	//
	// Bottom line is for each "slot" in the FIFO buffer each LSB represents
	// approximately 0.00008 degrees of movement.
	__mpu60x0Factor = __mpu60x0DataRateSeconds * __mpu60x0LSBSensitivity;
}

void __twb_imu_mpu60x0_gyro_ProcessSlot(uint8_t slotIndex){
	uint8_t slotOffset = MPU60x0_SLOT_SIZE * slotIndex;

	TWB_Filter_Median_Apply(&__mpuGyro60x0XFilter,(int16_t)(MPU60x0GyroI2CJob->InBuffer[slotOffset + 0] << 8 | MPU60x0GyroI2CJob->InBuffer[slotOffset + 1]));
	TWB_Filter_Median_Apply(&__mpuGyro60x0YFilter,(int16_t)(MPU60x0GyroI2CJob->InBuffer[slotOffset + 2] << 8 | MPU60x0GyroI2CJob->InBuffer[slotOffset + 3]));
	TWB_Filter_Median_Apply(&__mpuGyro60x0ZFilter,(int16_t)(MPU60x0GyroI2CJob->InBuffer[slotOffset + 4] << 8 | MPU60x0GyroI2CJob->InBuffer[slotOffset + 5]));

	float deltaX = ((float)(int16_t)(__mpuGyro60x0XFilter.Current - __mpu60x0GyroOffset.x)) * __mpu60x0Factor;
	float deltaY = ((float)(int16_t)(__mpuGyro60x0YFilter.Current - __mpu60x0GyroOffset.y)) * __mpu60x0Factor;
	float deltaZ = ((float)(int16_t)(__mpuGyro60x0ZFilter.Current - __mpu60x0GyroOffset.z)) * __mpu60x0Factor;

	__mpu60x0GyroExtendedData->DeltaT += __mpu60x0DataRateSeconds;

	__mpu60x0GyroExtendedData->DeltaX += deltaX;
	__mpu60x0GyroExtendedData->DeltaY += deltaY;
	__mpu60x0GyroExtendedData->DeltaZ += deltaZ;

	GyroMPU60x0Sensor->Y += deltaX;
	GyroMPU60x0Sensor->X += deltaY;
	GyroMPU60x0Sensor->Z += deltaZ;
}

iOpResult_e  TWB_IMU_MPU60x0_Gyro_ProcessData(void){
	if(MPU60x0GyroFiFoState == FiFoState_ReadQueueSize)
		__twb_imu_mpu60x0_gyro_QueueFiFoRead();
	else if(MPU60x0GyroFiFoState == FiFoState_ReadQueueSize){
		uint8_t sampleCount = MPU60x0GyroI2CJob->InBufferSize / MPU60x0_SLOT_SIZE;
		for(uint8_t slotIndex = 0; slotIndex < sampleCount; ++slotIndex)
			__twb_imu_mpu60x0_gyro_ProcessSlot(slotIndex);

		MPU60x0GyroFiFoState = FiFoState_Idle;
	}
	else{
		/*
		 * Only Other Option is a simple register
		 * read, so just read in first set of X, Y, Z
		 */
		__twb_imu_mpu60x0_gyro_ProcessSlot(0);

		MPU60x0GyroFiFoState = FiFoState_Idle;
	}

	if(MPU60x0GyroFiFoState == FiFoState_Idle){
		__mpu60x0GyroExtendedData->RateXDPS += ((float)(int16_t)(__mpuGyro60x0XFilter.Current - __mpu60x0GyroOffset.x)) * __mpu60x0LSBSensitivity;
		__mpu60x0GyroExtendedData->RateYDPS += ((float)(int16_t)(__mpuGyro60x0YFilter.Current - __mpu60x0GyroOffset.y)) * __mpu60x0LSBSensitivity;
		__mpu60x0GyroExtendedData->RateZDPS += ((float)(int16_t)(__mpuGyro60x0ZFilter.Current - __mpu60x0GyroOffset.z)) * __mpu60x0LSBSensitivity;
	}
	return OK;
}


iOpResult_e __twb_imu_mpu60x0_gyro_Read(void) {
	if(SensorConfig->LSM303_ACC_FIFO_Enable == Enabled){
		MPU60x0GyroI2CJob->Address = MPU60x0_Address;
		MPU60x0GyroI2CJob->OutBufferSize = 1;
		MPU60x0GyroI2CJob->OutBuffer[0] = MPU_60x0_FIFO_COUNT_H;
		MPU60x0GyroI2CJob->InBufferSize = 2; /* Only read one character, the size of the queue */
		MPU60x0GyroI2CJob->Callback = &__twb_imu_mpu60x0_gyro_QueueSizeRead;

		MPU60x0GyroFiFoState = FiFoState_ReadingQueueSize;
	}
	else{
		MPU60x0GyroI2CJob->Address = MPU60x0_Address;
		MPU60x0GyroI2CJob->OutBufferSize = 1;
		MPU60x0GyroI2CJob->OutBuffer[0] = MPU_60x0_GYRO_XOUT_H;
		MPU60x0GyroI2CJob->InBufferSize = 6; /* Read X, Y & Z */
		MPU60x0GyroI2CJob->Callback = &__twb_imu_mpu60x0_gyro_RegisterRead;

		MPU60x0GyroFiFoState = FiFoState_Idle;
	}

	return TWB_I2C_QueueAsyncJob(MPU60x0GyroI2CJob);
}


//Ticks comes in at qty of 10 microseconds, or .01 milliseconds or .000001 seconds
//Formula for integration of the Gyro
//  Counts from axis * LSB Sensitivity * Seconds
iOpResult_e TWB_IMU_MPU60x0_Gyro_Read(float deltaT){
	return __twb_imu_mpu60x0_gyro_Read();
}

iOpResult_e TWB_IMU_MPU60x0_Gyro_IRQ(){
	return __twb_imu_mpu60x0_gyro_Read();
}


void TWB_IMU_MPU60x0_Gyro_SendDiag(void){
	_commonDiagBuffer->SensorId = GyroMPU60x0Sensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)(__mpuGyro60x0XFilter.Current - __mpu60x0GyroOffset.x);
	_commonDiagBuffer->RawY = (int16_t)(__mpuGyro60x0YFilter.Current - __mpu60x0GyroOffset.y);
	_commonDiagBuffer->RawZ = (int16_t)(__mpuGyro60x0ZFilter.Current - __mpu60x0GyroOffset.z);

	_commonDiagBuffer->X = (int16_t)GyroMPU60x0Sensor->X;
	_commonDiagBuffer->Y = (int16_t)GyroMPU60x0Sensor->Y;
	_commonDiagBuffer->Z = (int16_t)GyroMPU60x0Sensor->Z;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}



