/*
 * lsm303.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "sensors/lsm303_acc.h"

#include "main.h"

#include "twb_config.h"
#include "twb_eeprom.h"


#include "common/twb_serial_eeprom.h"
#include "common/twb_i2c.h"
#include "common/twb_debug.h"
#include "common/twb_timer.h"
#include "common/twb_math.h"
#include "common/memory_mgmt.h"

#include "sensors/snsr_main.h"
#include "Filters/Filter.h"

#include <math.h>


I2CJob_t *LSM303AccI2CJob;


/* Start the defines for the Accelerometer */
#define LSM303_Acc_Address 0x19

#define LSM303_ACC_CRGA1_ADDR       0x20
#define LSM303_ACC_CRGA2_ADDR       0x21
#define LSM303_ACC_CRGA3_ADDR       0x22
#define LSM303_ACC_CRGA4_ADDR       0x23
#define LSM303_ACC_CRGA5_ADDR       0x24

#define LSM303_ACC_CRGA1_DEFAULT 0b01110111 //Start out with a 100Hz data rate and enable all three channels.
// D7-D4 (Data Rate)         => 0111 => 400 Hz
// D3 (Low Power Mode)       => 0 => Normal Operation
// D2, D1, D0 (Zen, Yen, Xen)=> 111 => Enable the channels

#define LSM303_ACC_CRGA2_DEFAULT 0b10000000
// D7, D6 (HPF Mode)         => 10 => Normal Mode
// D5, D4 (HPF Config)       => 00 => Cutoff Frequency Selection (need to play with this, couldn't find mapping for values)
// D3 (FDS)                  => 0 => Internal Filter Bypassed (1 => Data from internal filter sent to output register)
// D2-D0 not really interesting
// D2 (HPF For Click)        => 0 => Not using Click
// D1 (HPF For AOI On IRQ2)  => 0 => Nope
// D0 (HPF For AOI On IRQ1)  => 0 => Nope

#define LSM303_ACC_CRGA3_DEFAULT 0b00000000
// Configures IRQ sources.

#define LSM303_ACC_CRGA4_DEFAULT 0b00000001
// D7 (Block Data Update)    => 0 => Continuous Output
// D6 (Endian)               => 0 => data LSB @ lower address
// D5, D4 (Fullscale)        => 00 => +/2G Full Scale
// D3  High Resolution Mode  => 0 => High resolution disable (too much noise to make this worth while)
// D2, D1 (N/A)              => 00 => Keep 0 for normal operation
// D0 SIM                    => 1 => 3 Wire interface

#define LSM303_ACC_CRGA5_DEFAULT 0b00000000 // Used for IRQ and FIFO configuration


#define LSM303_ACC_CRGA6_DEFAULT 0b00000000 // Used for IRQ and FIFO configuration


#define LSM303_ACC_INT1_CFG_A 0b

#define LSM303_ACC_FIFO_CTRL_REG_A_ADDR 0x2E
#define LSM303_ACC_FIFO_SRC_REG_A_ADDR  0x2F

#define LSM303_ACC_CRGA5_FIFO_ENABLED      0b01000000
// D7, BOOT                  => 0 => Normal Mode
// D6, FIFO Enable           => 1 => Enabled
// D5, D4 N/A                => 00 => Not Used
// D3, LIR INT1 Latch        => 0 => Disabled, but potentially turn on, cleared when read INT1_SRC
// D2, D4D INT1 4D Enable    => 0 => 4D Detection is enabled on INT1, see Table 29 in DM for more details
// D1, LIR INT2 Latch        => 0 => Disabled, INT2 not used.
// D0, D4D INT2 4D Enable    => 0 => See table 29 in DM
#define LSM303_ACC_CRGA5_FIFO_DISABLED   0b00000000


#define LSM303_ACC_FIFO_CTRL_REG_A_NO_FIFO 0b00000000
// D7, D6 FIFO Mode          => 00 => NO FIFO
// D5     Trigger Selection  => 0  => Trigger NOT linked to INT1
// D4,D3,D2,D1,D0 WTM Value  =>  00000 => Default Value 0 (N/A)

#define LSM303_ACC_FIFO_CTRL_REG_A_FIFO 0b10100100
// D7, D6 FIFO Mode          => 10 => Stream Mode
// D5     Trigger Selection  => 1  => Trigger linked to INT1
// D4 D3,D2,D1,D0 WTM Value  => 00100 => 4 Samples

#define LSM303_ACC_FIFO_IRQ_WTM_CFG_ENABLE  0b00000100

#define LSM303_ACC_DRDY_IRQ_CFG_ENABLE      0b00010000
#define LSM303_ACC_NO_IRQ			        0b00000000

#define LSM303_ACC_INT1_CFG_A_ADDR      0x30
#define LSM303_ACC_INT1_SRC_A_ADDR      0x31
#define LSM303_ACC_INT1_THS_A_ADDR      0x32

#define LSM303_ACC_OUT_X_L_ADDR         0x28
#define LSM303_ACC_OUT_X_H_ADDR         0x29
#define LSM303_ACC_OUT_Y_L_ADDR         0x2A
#define LSM303_ACC_OUT_Y_H_ADDR         0x2B
#define LSM303_ACC_OUT_Z_L_ADDR         0x2C
#define LSM303_ACC_OUT_Z_H_ADDR         0x2D
/* ---------------------------------- */

#define __MATH_PI 3.141592
#define LSM_303_OUT_BUFFER_SIZE 1    /* Will only send out reg address or 1 byte */
#define LSM_303_MAX_FIFO_SIZE 20      /* Cap the maximum number of slots read at once to 6 */
#define LSM_303_MAX_FIFO_SLOT_SIZE 6 /* Two bytes for each X, Y, Z */

s16_vector_t __lsm303acc_offset;
s16_vector_t __lsm303acc_currentRaw;

FilterSigned_t __lsm303AccXFilter;
FilterSigned_t __lsm303AccYFilter;
FilterSigned_t __lsm303AccZFilter;

FiFoState_e LSM303AccFiFoState = FiFoState_Idle;

void TWB_IMU_LSM303_ReadComplete(iOpResult_e result);

iOpResult_e __twb_imu_lsm303_acc_init_periph(void){
	TWB_Debug_Print("LSM303.ACC-");

	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA1_ADDR, SensorConfig->LSM303_ACC_REG1));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA2_ADDR, SensorConfig->LSM303_ACC_REG2));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, SensorConfig->LSM303_ACC_REG3)); //This just clears all the IRQ's for the periph.
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA4_ADDR, SensorConfig->LSM303_ACC_REG4));

	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_FIFO_CTRL_REG_A_ADDR, LSM303_ACC_FIFO_CTRL_REG_A_NO_FIFO));

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_Init(void){
	LSM303AccI2CJob = pm_malloc(sizeof(I2CJob_t));
	LSM303AccI2CJob->OutBuffer = pm_malloc(LSM_303_OUT_BUFFER_SIZE); //Will always just send out one byte for the register address.
	LSM303AccI2CJob->InBuffer = pm_malloc(LSM_303_MAX_FIFO_SLOT_SIZE * LSM_303_MAX_FIFO_SIZE);
	LSM303AccI2CJob->Callback = null;

	assert_succeed(TWB_SE_ReadS16(LSM303_ACC_EEPROM_OFFSET + 0, &__lsm303acc_offset.x));
	assert_succeed(TWB_SE_ReadS16(LSM303_ACC_EEPROM_OFFSET + 2, &__lsm303acc_offset.y));
	assert_succeed(TWB_SE_ReadS16(LSM303_ACC_EEPROM_OFFSET + 4, &__lsm303acc_offset.z));

	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 6, &SensorConfig->LSM303_ACC_Enabled));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 7, &SensorConfig->LSM303_ACC_REG1));

	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 8, &SensorConfig->LSM303_ACC_REG2));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 9, &SensorConfig->LSM303_ACC_REG3));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 10, &SensorConfig->LSM303_ACC_REG4));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 11, &SensorConfig->LSM303_ACC_FIFO_Enable));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 12, &SensorConfig->LSM303_ACC_FilterOption));
	assert_succeed(TWB_SE_ReadU8(LSM303_ACC_EEPROM_OFFSET + 13, &SensorConfig->LSM303_ACC_SampleRate));

	AccLSM303Sensor->IsEnabled = SensorConfig->LSM303_ACC_Enabled;
	AccLSM303Sensor->SampleRate = SensorConfig->LSM303_ACC_SampleRate;

	AccLSM303Sensor->StartZero = &TWB_IMU_LSM303_Acc_Zero;
	AccLSM303Sensor->CompleteZero = &TWB_IMU_LSM303_Acc_CompleteZero;

	AccLSM303Sensor->ProcessData = &TWB_IMU_LSM303_Acc_ProcessData;

	AccLSM303Sensor->Read = &TWB_IMU_LSM303_Acc_Read;
	AccLSM303Sensor->HandleIRQ = &TWB_IMU_LSM303_Acc_IRQ;

	AccLSM303Sensor->Start = &TWB_IMU_LSM303_Acc_Start;
	AccLSM303Sensor->Pause = &TWB_IMU_LSM303_Acc_Pause;
	AccLSM303Sensor->Resume = &TWB_IMU_LSM303_Acc_Resume;
	AccLSM303Sensor->Stop = &TWB_IMU_LSM303_Acc_Stop;

	__lsm303AccXFilter.FilterType = SensorConfig->LSM303_ACC_FilterOption;
	__lsm303AccYFilter.FilterType = SensorConfig->LSM303_ACC_FilterOption;
	__lsm303AccZFilter.FilterType = SensorConfig->LSM303_ACC_FilterOption;

	__lsm303AccYFilter.IsInitialized = false;
	__lsm303AccYFilter.IsInitialized = false;
	__lsm303AccZFilter.IsInitialized = false;

	__lsm303AccYFilter.MovingAverageInitialized = false;
	__lsm303AccYFilter.MovingAverageInitialized = false;
	__lsm303AccZFilter.MovingAverageInitialized = false;

	if(AccLSM303Sensor->IsEnabled == Enabled)
		assert_succeed(__twb_imu_lsm303_acc_init_periph());

	return OK;
}

iOpResult_e  TWB_IMU_LSM303_Acc_ResetDefaults(void){
	//Start Defaults for LSM303 Accelerometer
	SensorConfig->LSM303_ACC_Enabled = Enabled;
	SensorConfig->LSM303_ACC_REG1 = LSM303_ACC_CRGA1_DEFAULT;
	SensorConfig->LSM303_ACC_REG2 = LSM303_ACC_CRGA2_DEFAULT;
	SensorConfig->LSM303_ACC_REG3 = LSM303_ACC_CRGA3_DEFAULT;
	SensorConfig->LSM303_ACC_REG4 = LSM303_ACC_CRGA4_DEFAULT;
	SensorConfig->LSM303_ACC_FIFO_Enable = Enabled;
	SensorConfig->LSM303_ACC_FilterOption = FilterOption_Median_5_Sample | FilterOption_Moving_Average_30;
	SensorConfig->LSM303_ACC_SampleRate = SampleRate_200Hz;

	AccLSM303Sensor->IsEnabled = SensorConfig->LSM303_ACC_Enabled;
	AccLSM303Sensor->SampleRate = SensorConfig->LSM303_ACC_SampleRate;

	__lsm303acc_offset.x = 0;
	__lsm303acc_offset.y = 0;
	__lsm303acc_offset.z = 0;

	assert_succeed(TWB_SE_WriteU16Pause(LSM303_ACC_EEPROM_OFFSET + 0, __lsm303acc_offset.x));
	assert_succeed(TWB_SE_WriteU16Pause(LSM303_ACC_EEPROM_OFFSET + 2, __lsm303acc_offset.y));
	assert_succeed(TWB_SE_WriteU16Pause(LSM303_ACC_EEPROM_OFFSET + 4, __lsm303acc_offset.z));

	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 6, SensorConfig->LSM303_ACC_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 7, SensorConfig->LSM303_ACC_REG1));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 8, SensorConfig->LSM303_ACC_REG2));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 9, SensorConfig->LSM303_ACC_REG3));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 10, SensorConfig->LSM303_ACC_REG4));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 11, SensorConfig->LSM303_ACC_FIFO_Enable));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 12, SensorConfig->LSM303_ACC_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(LSM303_ACC_EEPROM_OFFSET + 13, SensorConfig->LSM303_ACC_SampleRate));

	return OK;
}

iOpResult_e __lsmacc303_EnableFifo(void){
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_FIFO_CTRL_REG_A_ADDR, LSM303_ACC_FIFO_CTRL_REG_A_FIFO));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA5_ADDR, LSM303_ACC_CRGA5_FIFO_ENABLED));

	return OK;
}

iOpResult_e __lsmacc303_DisableFifo(void){
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_FIFO_CTRL_REG_A_ADDR, LSM303_ACC_FIFO_CTRL_REG_A_NO_FIFO));
	assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA5_ADDR, LSM303_ACC_CRGA5_FIFO_DISABLED));

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			SensorConfig->LSM303_ACC_Enabled = value;
			AccLSM303Sensor->IsEnabled = value;
			if(SensorConfig->LSM303_ACC_Enabled == Enabled)
				__twb_imu_lsm303_acc_init_periph();

			break;
		case 7:
			SensorConfig->LSM303_ACC_REG1 = value;
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA1_ADDR, SensorConfig->LSM303_ACC_REG1));
			break;
		case 8:
			SensorConfig->LSM303_ACC_REG2 = value;
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA2_ADDR, SensorConfig->LSM303_ACC_REG2));
			break;
		case 10:
			SensorConfig->LSM303_ACC_REG4 = value;
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA4_ADDR, SensorConfig->LSM303_ACC_REG4));
			break;
		case 11:
			/* Enable the FIFO, when the sensors are re-enabled, we will configure the periph. */
			SensorConfig->LSM303_ACC_FIFO_Enable = value;
			break;
		case 12:
			SensorConfig->LSM303_ACC_FilterOption = value;
			__lsm303AccXFilter.FilterType = value;
			__lsm303AccYFilter.FilterType = value;
			__lsm303AccZFilter.FilterType = value;

			__lsm303AccXFilter.IsInitialized = false;
			__lsm303AccYFilter.IsInitialized = false;
			__lsm303AccZFilter.IsInitialized = false;

			__lsm303AccXFilter.CurrentSlot = 0;
			__lsm303AccYFilter.CurrentSlot = 0;
			__lsm303AccZFilter.CurrentSlot = 0;

			break;
		case 13:
			/* Set the sample rate here, when we restart the sensor, it will send the I2C to reconfigure it. */
			SensorConfig->LSM303_ACC_SampleRate = value;
			AccLSM303Sensor->SampleRate = SensorConfig->L3GD20_SampleRate;

			break;
	}

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_Start(void) {
	if(SensorConfig->LSM303_ACC_FIFO_Enable)
		__lsmacc303_EnableFifo();

	if(SensorConfig->LSM303_ACC_SampleRate == SampleRate_IRQ){
		//If Fifo is enabled, enable the IRQ on watermark.
		if(SensorConfig->LSM303_ACC_FIFO_Enable){
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_FIFO_IRQ_WTM_CFG_ENABLE ));
		}
		else{ //Otherwise enable it on the Data Ready 2 event.
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_DRDY_IRQ_CFG_ENABLE ));
		}
	}

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_Pause(void) {
	if(SensorConfig->LSM303_ACC_FIFO_Enable)
		__lsmacc303_DisableFifo();

	if(SensorConfig->LSM303_ACC_SampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_NO_IRQ));

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_Resume(void){
	if(SensorConfig->LSM303_ACC_FIFO_Enable)
		__lsmacc303_EnableFifo();

	if(SensorConfig->LSM303_ACC_SampleRate == SampleRate_IRQ){
		//If Fifo is enabled, enable the IRQ on watermark.
		if(SensorConfig->LSM303_ACC_FIFO_Enable){
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_FIFO_IRQ_WTM_CFG_ENABLE ));
		}
		else{ //Otherwise enable it on the Data Ready 2 event.
			assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_DRDY_IRQ_CFG_ENABLE ));
		}
	}

	return OK;
}

iOpResult_e TWB_IMU_LSM303_Acc_Stop(void) {
	if(SensorConfig->LSM303_ACC_FIFO_Enable)
		__lsmacc303_DisableFifo();

	if(SensorConfig->LSM303_ACC_SampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(LSM303_Acc_Address, LSM303_ACC_CRGA3_ADDR, LSM303_ACC_NO_IRQ));

	return OK;
}

bool TWB_IMU_LSM303_Acc_Zero(uint8_t sampleSize, float pauseMS){
	TWB_Sensor_Calibrate(LSM303_Acc_Address, LSM303_ACC_OUT_X_L_ADDR | 0x80,LSM303_ACC_EEPROM_OFFSET + 0x00, sampleSize, pauseMS, &__lsm303acc_offset, false);

	return false;
}

iOpResult_e TWB_IMU_LSM303_Acc_CompleteZero(u16_vector_t *cal){
	__lsm303acc_offset.x = cal->x;
	__lsm303acc_offset.y = cal->y;
	__lsm303acc_offset.z = cal->z;

	return OK;
}

int32_t tmpX, tmpY, tmpZ;
float _accX, _accY, _accZ;
float __lsm303AccX,__lsm303AccY, __lsm303AccZ;

void __twb_imu_lsm303_acc_RegisterRead(void *i2cjob){
	AccLSM303Sensor->DataReady = DataReady;
}

void __twb_imu_lsm303_acc_QueueRead(void *i2cjob){
	LSM303AccFiFoState = FiFoState_ReadQueue;
	AccLSM303Sensor->DataReady = DataReady;
}

void __twb_imu_lsm303_acc_QueueSizeRead(void *i2cjob){
	LSM303AccFiFoState = FiFoState_ReadQueueSize;
	AccLSM303Sensor->DataReady = DataReady;
}

void __twb_imu_lsm303_acc_QueueFiFoRead(void){
	uint8_t sampleCount = LSM303AccI2CJob->InBuffer[0] & 0b00011111 ;

	if(LSM_303_MAX_FIFO_SIZE < sampleCount){
		TWB_Debug_PrintInt("ACC TOO BIG: ", sampleCount);
		sampleCount = LSM_303_MAX_FIFO_SIZE;
	}

	LSM303AccI2CJob->Address = LSM303_Acc_Address;
	LSM303AccI2CJob->State = Pending;
	LSM303AccI2CJob->OutBufferSize = 1;
	LSM303AccI2CJob->OutBuffer[0] = LSM303_ACC_OUT_X_L_ADDR | 0x80;
	LSM303AccI2CJob->InBufferSize = LSM_303_MAX_FIFO_SLOT_SIZE * sampleCount;
	LSM303AccI2CJob->Callback = &__twb_imu_lsm303_acc_QueueRead;

	TWB_I2C_QueueAsyncJob(LSM303AccI2CJob);

	LSM303AccFiFoState = FiFoState_ReadingQueue;
}

void __twb_imu_lsm303_acc_ProcessSlot(uint8_t slotIndex){
	TWB_Filter_Median_Apply_Signed(&__lsm303AccXFilter,((int16_t)(LSM303AccI2CJob->InBuffer[1 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE] << 8 | LSM303AccI2CJob->InBuffer[0 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE])));
	TWB_Filter_Median_Apply_Signed(&__lsm303AccYFilter,((int16_t)(LSM303AccI2CJob->InBuffer[3 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE] << 8 | LSM303AccI2CJob->InBuffer[2 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE])));
	TWB_Filter_Median_Apply_Signed(&__lsm303AccZFilter,((int16_t)(LSM303AccI2CJob->InBuffer[5 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE] << 8 | LSM303AccI2CJob->InBuffer[4 + slotIndex * LSM_303_MAX_FIFO_SLOT_SIZE])));
}

float _resultF;

void __twb_imu_lsm303_acc_CalcAttitude(void){
	__lsm303AccX = __lsm303AccXFilter.Current - __lsm303acc_offset.x;
	__lsm303AccY = __lsm303AccYFilter.Current - __lsm303acc_offset.y;
	__lsm303AccZ = __lsm303AccZFilter.Current - __lsm303acc_offset.z;

	_accX = ((__lsm303AccXFilter.Current - __lsm303acc_offset.x) / 16384.0f);
	_accY = ((__lsm303AccYFilter.Current - __lsm303acc_offset.y) / 16384.0f);
	_accZ = ((__lsm303AccZFilter.Current) / 16384.0f);

	_accX = TWB_MATH_LimitOneToMinusOne(_accX);
	_accY = TWB_MATH_LimitOneToMinusOne(_accY);
	_accZ = TWB_MATH_LimitOneToMinusOne(_accZ);

	AccLSM303Sensor->X = (float)(360.0f * twb_asinf(_accX) / 6.28318f);
	AccLSM303Sensor->Y = (float)(360.0f * twb_asinf(_accY) / 6.28318f);
	AccLSM303Sensor->Z = (float)(360.0f * twb_asinf(_accZ) / 6.28318f);

	if(AccLSM303Sensor->Y > 45.0f) /*Q1 -> Q3*/
		AccLSM303Sensor->Y = 90.0f - AccLSM303Sensor->Z;
	else if(AccLSM303Sensor->Y < -45.0f) /*-Q1 -> -Q3 */
		AccLSM303Sensor->Y = -90.0f + AccLSM303Sensor->Z;
	else if(AccLSM303Sensor->Z < 0.0f){ /* Q3, Q4, -Q3, -Q4 */
		if(AccLSM303Sensor->Y > 0.0f)
			AccLSM303Sensor->Y = 180.0f - AccLSM303Sensor->Y;
		else
			AccLSM303Sensor->Y = -180.0f - AccLSM303Sensor->Y;
	}

	if(AccLSM303Sensor->X > 45.0f)
		AccLSM303Sensor->X = 90 - AccLSM303Sensor->Z;
	else if(AccLSM303Sensor->X < -45.0f)
			AccLSM303Sensor->X = -90 + AccLSM303Sensor->Z;
	else if(AccLSM303Sensor->Z < 0.0f){ /* Q3, Q4, -Q3, -Q4 */
		if(AccLSM303Sensor->X > 0.0f)
			AccLSM303Sensor->X = 180.0f - AccLSM303Sensor->X;
		else
			AccLSM303Sensor->X = -180.0f - AccLSM303Sensor->X;
	}
}

iOpResult_e  TWB_IMU_LSM303_Acc_ProcessData(void){
	if(LSM303AccFiFoState == FiFoState_ReadQueueSize)
		__twb_imu_lsm303_acc_QueueFiFoRead();
	else if(LSM303AccFiFoState == FiFoState_ReadQueue){
		uint8_t sampleCount = LSM303AccI2CJob->InBufferSize / LSM_303_MAX_FIFO_SLOT_SIZE;
		for(uint8_t slotIndex = 0; slotIndex < sampleCount; ++slotIndex)
			__twb_imu_lsm303_acc_ProcessSlot(slotIndex);

		LSM303AccFiFoState = FiFoState_Idle;
		__twb_imu_lsm303_acc_CalcAttitude();
	}
	else{
		/*
		 * Only Other Option is a simple register
		 * read, so just read in first set of X, Y, Z
		 */
		__twb_imu_lsm303_acc_ProcessSlot(0);
		__twb_imu_lsm303_acc_CalcAttitude();

		LSM303AccFiFoState = FiFoState_Idle;
	}

	return OK;
}

/*
 * Primary starting point for
 * doing a read from the gyro.
 */
iOpResult_e __twb_imu_lsm303_acc_Read(void){
	if(SensorConfig->LSM303_ACC_FIFO_Enable == Enabled){
		LSM303AccI2CJob->Address = LSM303_Acc_Address;
		LSM303AccI2CJob->OutBufferSize = 1;
		LSM303AccI2CJob->State = Pending;
		LSM303AccI2CJob->OutBuffer[0] = LSM303_ACC_FIFO_SRC_REG_A_ADDR;
		LSM303AccI2CJob->InBufferSize = 1; /* Only read one character, the size of the queue */
		LSM303AccI2CJob->Callback = &__twb_imu_lsm303_acc_QueueSizeRead;

		LSM303AccFiFoState = FiFoState_ReadingQueueSize;
	}
	else{
		LSM303AccI2CJob->Address = LSM303_Acc_Address;
		LSM303AccI2CJob->OutBufferSize = 1;
		LSM303AccI2CJob->State = Pending;
		LSM303AccI2CJob->OutBuffer[0] = LSM303_ACC_OUT_X_L_ADDR | 0x80;
		LSM303AccI2CJob->InBufferSize = 6; /* Read X, Y & Z */
		LSM303AccI2CJob->Callback = &__twb_imu_lsm303_acc_RegisterRead;

		LSM303AccFiFoState = FiFoState_Idle;
	}

	return TWB_I2C_QueueAsyncJob(LSM303AccI2CJob);
}

void TWB_IMU_LSM303_ReadComplete(iOpResult_e result){
	AccLSM303Sensor->DataReady = DataReady;
}

iOpResult_e TWB_IMU_LSM303_Acc_IRQ(void) {
	return __twb_imu_lsm303_acc_Read();
}

iOpResult_e TWB_IMU_LSM303_Acc_Read(float deltaT) {
	return __twb_imu_lsm303_acc_Read();
}

void TWB_IMU_LSM303_Acc_SendDiag(void){
	_commonDiagBuffer->SensorId = AccLSM303Sensor->SensorId;

	_commonDiagBuffer->RawX = (uint16_t)(__lsm303AccXFilter.Current - __lsm303acc_offset.x);
	_commonDiagBuffer->RawY = (uint16_t)(__lsm303AccYFilter.Current - __lsm303acc_offset.y);
	_commonDiagBuffer->RawZ = (uint16_t)(__lsm303AccZFilter.Current);

	_commonDiagBuffer->X = (int16_t)AccLSM303Sensor->X * 10.0f;
	_commonDiagBuffer->Y = (int16_t)AccLSM303Sensor->Y * 10.0f;
	_commonDiagBuffer->Z = (int16_t)AccLSM303Sensor->Z * 10.0f;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
