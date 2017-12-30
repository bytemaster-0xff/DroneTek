/*
 * l3g20.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "twb_config.h"

#include "twb_eeprom.h"

#include "main.h"

#include "Filters/Filter.h"

#include "sensors/l3g20.h"
#include "sensors/snsr_main.h"

#include "common/twb_i2c.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_timer.h"
#include "common/memory_mgmt.h"
#include "common/twb_debug.h"

#define L3G20ADDR 0x6A

/*************************
    L3G4200D Registers
*************************/
#define L3GD20_WHO_AM_I 0x0F
#define L3GD20_CTRL_REG1 0x20
#define L3GD20_CTRL_REG2 0x21
#define L3GD20_CTRL_REG3 0x22
#define L3GD20_CTRL_REG4 0x23
#define L3GD20_CTRL_REG5 0x24
#define L3GD20_REFERENCE 0x25
#define L3GD20_OUT_TEMP 0x26
#define L3GD20_STATUS_REG 0x27
#define L3GD20_OUT_X_L 0x28
#define L3GD20_OUT_X_H 0x29
#define L3GD20_OUT_Y_L 0x2A
#define L3GD20_OUT_Y_H 0x2B
#define L3GD20_OUT_Z_L 0x2C
#define L3GD20_OUT_Z_H 0x2D
#define L3GD20_FIFO_CTRL_REG 0x2E
#define L3GD20_FIFO_SRC_REG 0x2F
#define L3GD20_INT1_CFG 0x30
#define L3GD20_INT1_SRC 0x31
#define L3GD20_INT1_TSH_XH 0x32
#define L3GD20_INT1_TSH_XL 0x33
#define L3GD20_INT1_TSH_YH 0x34
#define L3GD20_INT1_TSH_YL 0x35
#define L3GD20_INT1_TSH_ZH 0x36
#define L3GD20_INT1_TSH_ZL 0x37
#define L3GD20_INT1_DURATION 0x38

#define L3GD20_ADDR 0x6A

FilterSigned_t _l3gd20XFilter;
FilterSigned_t _l3gd20YFilter;
FilterSigned_t _l3gd20ZFilter;

#define L3GD_CTRL_REG1_DEFAULT 0b11011111
//  D7, D6 (DR1, DR0 Output Data Rate) => 11 => 760 Hz
//  D5, D4 (BW1, BW0 Bandwidth Filter) => 01 => 25 //Not exactly sure what this value does.
//  D3 (PD Power Down) => 1 => Not Powered Down
//  D2, D1, D0 (Zen, Yen, Xen) => 111 => Enable All channels.
//
#define L3GD_CTRL_REG2_DEFAULT 0b00100000
//  D7, D6 (N/A) set to zero for proper operation
//  D5, D4 (HPM1, HPM0 High Pass Filter Mode) => 10 => Normal Mode of Operation
//  D3, D2, D1, D0 (High pass cut-off filter) => 0000 => Dependent on ODR

#define L3GD_CTRL_REG3_DEFAULT 0b00000000
//  D7 (Int 1 Enable)            => 0 => Not using Int1
//  D6 (Int 1 on Boot)           => 0 => Nope
//  D5 (Int 1 High/Low)          => 0 => Don't care, not using this line.
//  D4 (Push Pull/Open Drain)    => 0 => Push Pull
//  D3 (Data Ready on Int2)      => 0 => Will use, but will set via code after startup.
//  D2 (FIFO Water mark on Int2) => 0 => Nope
//  D1 (FIFO Overrun on Int2)    => 0 => Nope
//  D0 (FIFO empty on Int2)      => 0 => Nope

#define L3GD_CTRL_REG4_DEFAULT 0b00110001
//  D7 (Block Data Update) => 0 => Continuous (for now)
//  D6 (Big/Little Endian) => 0 => Data LSB @ lower address
//  D5, D4 (Full Scale)    => 11 => 2000 dps (other options 00 250 dps, 01 500 dps)
//  D3 (N/A)               => 0 => N/A,
//  D2, D1 (N/A)           => 00 => keep at zero for proper operation>
//  D0 (Interface Type)    => 1 => 3 Wire

#define L3GD_CTRL_REG5_DEFAULT 0b000000000
//  D7 (Reboot Mry)        => 0 => No idea, but 0 was normal operation.
//  D6 (FIFO Enable)       => 0 => Disable (for now)
//  D5 (N/A)               => 0 => Not applicable
//  D4 (HPF Enable)        => 0 => Disable HPF
//  D3, D2 (INT Selection) => 0 => (not sure about, default was 0)
//  D1, D0 (Output Select) => 0 => (not sure about, default was 0)
//  See Figure 18 in data sheet for more details.


//FiFo Enable FIFO_CTRL_REG
//================================================================================================
//  Enables FIFO Mode and Threshold for watermark
//  Total FIFO size is 32 slots for each X, Y and Z
//  FIFO_CTRL_REG 0x2E
//
//  D7, D6, D5 - (FM2, FM1, FM0)
//
//  0,  0,  0  - Bypass Mode - Readings always get written tot registers
//  0,  0,  1  - FIFO Mode - Start filling FIFO until full, then stops
//  0,  1,  0  - Stream Mode - Start filling FIFO until full, then start replacing oldest data.
//  0,  1,  1  - Stream-to-FIFO mode - once an IRQ occurs it switches to FIFO mode
//  1,  0,  0  - Bypass to Stream Mode - once a trigger occurs, switches from Bypass to Stream
//
//  WTM4-WTM0 - Threshold of bytes for watermark to trigger IRQ on Int2
//
//  To operate in FIFO mode, we are going to set the ODR (output data rate to 760Hz)
//  This means we are grabbing data at period of 1.31 milliseconds
//
//  Really want to update the Gyro at approximately 200Hz or 5MS, that means if we set our
//  FIFI water mark to 4 samples, we should get an IRQ every 5.24 milliseconds or at approximately
//  190 Hz, which should be adequate.
//
//  So for setting a watermark of four samples we have the following in WTM4-WTM0
//
//  D4, D3, D2, D1, D0
//   0,  0,  1,  0,  0 => 0x04
//
//  Bottom line, use Stream Mode, watermark at 4 samples
#define L3GD20_FIFO_CONFIG 0b01000100
#define L3GD20_FIFO_BYPASS 0b00000000

//
//  Next up, enabling IRQ when the watermark has been exceeded
//
//  CTRL_REG3 0x22
//
//  D7 - I1_Int1 IRQ ENable on Int1 (not used)  0
//  D6 - I2_Boot Not used                       0
//  D5 - H_Lactive H/L active for IRQ 1         0
//  D4 - PP_OD Push-Pull/ Open Drain            0
//  D3 - I2_DRDY  (Not used for WTM)            0
//  D2 - I2_WTM (This is the only IRQ we want)  1
//  D1 - I2_ORun Not used                       0
//  D0 - I2_Empty Not used                      0
//
//  Since we are using 4 samples and pulling every 5.24ms, we have a considerable
//  amount of time to pull and process the samples, we really never should drop any
//  samples.  Also in the IRQ, we are just going to assume that we just have
//  four samples ready for us.  Don't want to add the overhead of doing
//  a read for the status register.

// Pin to OR/XOR to enable/disable IRQ for WTM
#define L3GD20_FIFO_IRQ_ENABLE 0b00000100 //Applied to CTRL_REG3

#define L3GD20_DRDY_IRQ_ENABLE 0b00001000 //Applied to CTRL_REG3


// Pin to OR/XOR to enable/disable FIFO
#define L3GD20_FIFO_ENABLE 0b01000000 //Applied to CTRL_REG5

FiFoState_e L3GD20FiFoState = FiFoState_Idle;

I2CJob_t *L3GD20GryoI2CJob;
#define L3GD20_OUT_BUFFER_SIZE 1
#define L3GD20_FIFO_SLOT_SIZE 6
#define L3GD20_MAX_FIFO_SIZE 6

iOpResult_e __enableL3GD20FiFo(void){
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG5, L3GD20_FIFO_ENABLE));

	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_FIFO_CTRL_REG, L3GD20_FIFO_CONFIG));

	// TODO: When setting CTRL5, probably need to take into account the default value


	return OK;
}

iOpResult_e __disableL3GD20FiFo(void){
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_FIFO_CTRL_REG, L3GD20_FIFO_BYPASS));
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD_CTRL_REG3_DEFAULT));

	//Disable FIFO
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_FIFO_CTRL_REG, L3GD_CTRL_REG5_DEFAULT));

	return OK;
}

void __l3gd20_calculateScaleValue(void);

GryoExtendedData_t *__l3gdGyroExtendedData;
s16_vector_t __l3gd20Offset;

iOpResult_e __twb_imu_L3GD20_InitPeriph(void){

	uint8_t outValue = 0x00;
	assert_succeed(TWB_I2C_ReadRegisterValue(L3GD20_ADDR, 0x0F, &outValue));

	if(outValue != 0xD4){
		TWB_Debug_Print("!!Could not configure L3GD20.\r\n");
		return FAIL;
	}

	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_FIFO_CTRL_REG, L3GD20_FIFO_BYPASS));

	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG1, SensorConfig->L3GD20_REG1));
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG2, SensorConfig->L3GD20_REG2));
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG4, SensorConfig->L3GD20_REG4));
	assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG5, SensorConfig->L3GD20_REG5));

	TWB_Debug_Print("L3GD20-");

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_Init(void) {
	L3GD20GryoI2CJob = pm_malloc(sizeof(I2CJob_t));
	L3GD20GryoI2CJob->OutBuffer = pm_malloc(L3GD20_OUT_BUFFER_SIZE); //Will always just send out one byte for the register address.
	L3GD20GryoI2CJob->InBuffer = pm_malloc(L3GD20_FIFO_SLOT_SIZE * L3GD20_MAX_FIFO_SIZE);

	assert_succeed(TWB_SE_ReadS16(L3GD_EEPROM_OFFSET, &__l3gd20Offset.x));
	assert_succeed(TWB_SE_ReadS16(L3GD_EEPROM_OFFSET + 2, &__l3gd20Offset.y));
	assert_succeed(TWB_SE_ReadS16(L3GD_EEPROM_OFFSET + 4, &__l3gd20Offset.z));

	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 6, &SensorConfig->L3GD20_Enabled));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 7, &SensorConfig->L3GD20_REG1));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 8, &SensorConfig->L3GD20_REG2));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 9, &SensorConfig->L3GD20_REG3));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 10, &SensorConfig->L3GD20_REG4));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 11, &SensorConfig->L3GD20_REG5))
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 12, &SensorConfig->L3GD20_FIFO_Enable));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 13, &SensorConfig->L3GD20_FilterOption));
	assert_succeed(TWB_SE_ReadU8(L3GD_EEPROM_OFFSET + 14, &SensorConfig->L3GD20_SampleRate));

	GyroL3GD20Sensor->SampleRate = SensorConfig->L3GD20_SampleRate;
	GyroL3GD20Sensor->IsEnabled = SensorConfig->L3GD20_Enabled;

	GyroL3GD20Sensor->StartZero = &TWB_IMU_L3GD20_Zero;
	GyroL3GD20Sensor->CompleteZero = TWB_IMU_L3GD20_CompleteZero;

	GyroL3GD20Sensor->ProcessData = &TWB_IMU_L3GD30_ProcessData;

	GyroL3GD20Sensor->Read = &TWB_IMU_L3GD20_Read;
	GyroL3GD20Sensor->HandleIRQ = &TWB_IMU_L3GD20_IRQ;

	GyroL3GD20Sensor->Start = &TWB_IMU_L3GD20_Start;
	GyroL3GD20Sensor->Pause = &TWB_IMU_L3GD20_Pause;
	GyroL3GD20Sensor->Resume = &TWB_IMU_L3GD20_Resume;
	GyroL3GD20Sensor->Stop = &TWB_IMU_L3GD20_Stop;

	_l3gd20XFilter.FilterType = SensorConfig->L3GD20_FilterOption;
	_l3gd20YFilter.FilterType = SensorConfig->L3GD20_FilterOption;
	_l3gd20ZFilter.FilterType = SensorConfig->L3GD20_FilterOption;

	__l3gdGyroExtendedData = pm_malloc(sizeof(GryoExtendedData_t));
	GyroL3GD20Sensor->Extended = __l3gdGyroExtendedData;
	__l3gd20_calculateScaleValue();

	if(SensorConfig->L3GD20_Enabled == Enabled)
		assert_succeed(__twb_imu_L3GD20_InitPeriph());

	return OK;
}


iOpResult_e TWB_IMU_L3GD20_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
		case 6:
			GyroL3GD20Sensor->IsEnabled = value;
			SensorConfig->L3GD20_Enabled = value;

			if(SensorConfig->L3GD20_Enabled == Enabled){
				assert_succeed(__twb_imu_L3GD20_InitPeriph());

				if(SensorConfig->L3GD20_SampleRate == SampleRate_IRQ){
					if(SensorConfig->L3GD20_FIFO_Enable == Enabled){
						assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_FIFO_IRQ_ENABLE));
					}
					else{
						assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_DRDY_IRQ_ENABLE));
					}
				}

			}

			break;
		case 7:
			SensorConfig->L3GD20_REG1 = value;
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG1, SensorConfig->L3GD20_REG1));
			__l3gd20_calculateScaleValue();
			break;
		case 8:
			SensorConfig->L3GD20_REG2 = value;
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG2, SensorConfig->L3GD20_REG2));
			break;
		case 9:
			SensorConfig->L3GD20_REG3 = value;
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, SensorConfig->L3GD20_REG3));
			break;
		case 10:
			SensorConfig->L3GD20_REG4 = value;
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG4, SensorConfig->L3GD20_REG4));
			__l3gd20_calculateScaleValue();
			break;
		case 11:
			SensorConfig->L3GD20_REG5 = value;
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG5, SensorConfig->L3GD20_REG5));
			break;
		case 12:
			SensorConfig->L3GD20_FIFO_Enable = value;
			break;
		case 13:
			SensorConfig->L3GD20_FilterOption = value;
			_l3gd20XFilter.FilterType = value;
			_l3gd20YFilter.FilterType = value;
			_l3gd20ZFilter.FilterType = value;

			_l3gd20XFilter.CurrentSlot = 0;
			_l3gd20YFilter.CurrentSlot = 0;
			_l3gd20ZFilter.CurrentSlot = 0;

			_l3gd20XFilter.IsInitialized = false;
			_l3gd20YFilter.IsInitialized = false;
			_l3gd20ZFilter.IsInitialized = false;

			break;
		case 14:
			SensorConfig->L3GD20_SampleRate = value;
			GyroL3GD20Sensor->SampleRate = SensorConfig->L3GD20_SampleRate;
			break;

	}

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_ResetDefaults(void){
	SensorConfig->L3GD20_Enabled = Enabled;
	SensorConfig->L3GD20_REG1 = L3GD_CTRL_REG1_DEFAULT;
	SensorConfig->L3GD20_REG2 = L3GD_CTRL_REG2_DEFAULT;
	SensorConfig->L3GD20_REG3 = L3GD_CTRL_REG3_DEFAULT;
	SensorConfig->L3GD20_REG4 = L3GD_CTRL_REG4_DEFAULT;
	SensorConfig->L3GD20_REG5 = L3GD_CTRL_REG5_DEFAULT;
	SensorConfig->L3GD20_FilterOption = FilterOption_Median_5_Sample | FilterOption_Moving_Average_30;
	SensorConfig->L3GD20_SampleRate = SampleRate_200Hz;
	SensorConfig->L3GD20_FIFO_Enable = Enabled;

	GyroL3GD20Sensor->SampleRate = SensorConfig->L3GD20_SampleRate;
	GyroL3GD20Sensor->IsEnabled = SensorConfig->L3GD20_Enabled;

	__l3gd20Offset.x = 0;
	__l3gd20Offset.y = 0;
	__l3gd20Offset.z = 0;

	assert_succeed(TWB_SE_WriteU16Pause(L3GD_EEPROM_OFFSET + 0, __l3gd20Offset.x));
	assert_succeed(TWB_SE_WriteU16Pause(L3GD_EEPROM_OFFSET + 2, __l3gd20Offset.y));
	assert_succeed(TWB_SE_WriteU16Pause(L3GD_EEPROM_OFFSET + 4, __l3gd20Offset.z));

	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 6, SensorConfig->L3GD20_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 7, SensorConfig->L3GD20_REG1));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 8, SensorConfig->L3GD20_REG2));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 9, SensorConfig->L3GD20_REG3));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 10, SensorConfig->L3GD20_REG4));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 11, SensorConfig->L3GD20_REG5));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 12, SensorConfig->L3GD20_FIFO_Enable));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 13, SensorConfig->L3GD20_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(L3GD_EEPROM_OFFSET + 14, SensorConfig->L3GD20_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_Start(void){
	if(SensorConfig->L3GD20_FIFO_Enable)
		__enableL3GD20FiFo();

	if(SensorConfig->L3GD20_SampleRate == SampleRate_IRQ){
		if(SensorConfig->L3GD20_FIFO_Enable == Enabled){
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_FIFO_IRQ_ENABLE));
		}
		else{
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_DRDY_IRQ_ENABLE));
		}
	}


	_l3gd20XFilter.CurrentSlot = 0;
	_l3gd20YFilter.CurrentSlot = 0;
	_l3gd20ZFilter.CurrentSlot = 0;

	_l3gd20XFilter.IsInitialized = false;
	_l3gd20YFilter.IsInitialized = false;
	_l3gd20ZFilter.IsInitialized = false;

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_Pause(void){
	if(SensorConfig->L3GD20_FIFO_Enable == Enabled)
		__disableL3GD20FiFo();

	if(SensorConfig->L3GD20_SampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD_CTRL_REG3_DEFAULT));

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_Resume(void){
	if(SensorConfig->L3GD20_FIFO_Enable == Enabled)
		__enableL3GD20FiFo();

	if(SensorConfig->L3GD20_SampleRate == SampleRate_IRQ){
		if(SensorConfig->L3GD20_FIFO_Enable == Enabled){
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_FIFO_IRQ_ENABLE));
		}
		else{
			assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD20_DRDY_IRQ_ENABLE));
		}
	}

	_l3gd20XFilter.CurrentSlot = 0;
	_l3gd20YFilter.CurrentSlot = 0;
	_l3gd20ZFilter.CurrentSlot = 0;

	_l3gd20XFilter.IsInitialized = false;
	_l3gd20YFilter.IsInitialized = false;
	_l3gd20ZFilter.IsInitialized = false;

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_Stop(void){
	if(SensorConfig->L3GD20_SampleRate == SampleRate_IRQ)
		assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, L3GD20_CTRL_REG3, L3GD_CTRL_REG3_DEFAULT));

	return OK;
}

bool TWB_IMU_L3GD20_Zero(uint8_t sampleSize, float pauseMS){
	TWB_Sensor_Calibrate(L3G20ADDR, L3GD20_OUT_X_L | 0x80, L3GD_EEPROM_OFFSET + 0x00, sampleSize, pauseMS, &__l3gd20Offset, false);

	GyroL3GD20Sensor->X = 0;
	GyroL3GD20Sensor->Y = 0;
	GyroL3GD20Sensor->Z = 0;

	return false;
}

iOpResult_e TWB_IMU_L3GD20_CompleteZero(u16_vector_t *cal) {
	__l3gd20Offset.x = cal->x;
	__l3gd20Offset.y = cal->y;
	__l3gd20Offset.z = cal->z;

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_EnableIRQ(void) {
	//assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, CTRL_REG3, SensorConfig->L3GD20_REG3));

	return OK;
}

iOpResult_e TWB_IMU_L3GD20_DisableIRQ(void) {
	//assert_succeed(TWB_I2C_WriteRegister(L3GD20_ADDR, CTRL_REG3, SensorConfig->L3GD20_REG3));

	return OK;
}


float __l3gd20Factor = 0.00008;
float __l3gd20DataRateSeconds;
float __l3gd20LSBSensitivity;

float __l3gd20_getDataRateSeconds(void){
	float dataRateSeconds = 0.00f;

	if(SensorConfig->L3GD20_FIFO_Enable){
		uint8_t dataRate = SensorConfig->L3GD20_REG1 >> 6 & 0b00000011;
		switch(dataRate){
			case 0b00: dataRateSeconds = 0.010526; break;
			case 0b01: dataRateSeconds = 0.005263; break;
			case 0b10: dataRateSeconds = 0.002631; break;
			case 0b11: dataRateSeconds = 0.001316; break;
		}
	}
	else
		dataRateSeconds = TWB_Timer_GetMSFromSampleRate(SensorConfig->L3GD20_SampleRate) / 1000.0f;

	return dataRateSeconds;
}

/*
 * Things that are used to calculate the scaler
 * value that is applied against the raw
 * output from the gyro.
 */
void __l3gd20_calculateScaleValue(void){
	uint8_t fsSetting = SensorConfig->L3GD20_REG4 >> 4 & 0b00000011;


	float fullScaleDPS = 0;

	switch(fsSetting){
		case 0b00: fullScaleDPS = 250.0f; break;
		case 0b01: fullScaleDPS = 500.0f; break;
		case 0b10:
		case 0b11: fullScaleDPS = 2000.0f; break;
	}

	__l3gd20DataRateSeconds = __l3gd20_getDataRateSeconds();

	/*
	 * Each register read is a 16 bit signed number
	 * this full scale for +/- is 1/2 of a 16 bit
	 * number or 0x8000;
	 */
	float fullScale = (float)0x8000;

	//The LSB is with full scale count / full scale DPS
	//EX: 32768 / 2000 = Each bit represents a rate of 16.384 BPS
	__l3gd20LSBSensitivity = fullScale / fullScaleDPS;

	//This allows us to use for multiplication rather than division later ont.
	__l3gd20LSBSensitivity = 1.0f / __l3gd20LSBSensitivity;

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
	__l3gd20Factor = __l3gd20DataRateSeconds * __l3gd20LSBSensitivity;
}

void __twb_imu_l3gd20_ProcessSlot(uint8_t slotIndex){
	uint8_t slotOffset = L3GD20_FIFO_SLOT_SIZE * slotIndex;

	TWB_Filter_Median_Apply_Signed(&_l3gd20XFilter,((int16_t)(L3GD20GryoI2CJob->InBuffer[1 + slotOffset] << 8 | L3GD20GryoI2CJob->InBuffer[0 + slotOffset])));
	TWB_Filter_Median_Apply_Signed(&_l3gd20YFilter,((int16_t)(L3GD20GryoI2CJob->InBuffer[3 + slotOffset] << 8 | L3GD20GryoI2CJob->InBuffer[2 + slotOffset])));
	TWB_Filter_Median_Apply_Signed(&_l3gd20ZFilter,((int16_t)(L3GD20GryoI2CJob->InBuffer[5 + slotOffset] << 8 | L3GD20GryoI2CJob->InBuffer[4 + slotOffset])));

	float deltaX = ((float)(int16_t)(_l3gd20XFilter.Current - __l3gd20Offset.x)) * __l3gd20Factor;
	float deltaY = ((float)(int16_t)(_l3gd20YFilter.Current - __l3gd20Offset.y)) * __l3gd20Factor;
	float deltaZ = ((float)(int16_t)(_l3gd20ZFilter.Current - __l3gd20Offset.z)) * __l3gd20Factor;

	__l3gdGyroExtendedData->DeltaT += __l3gd20DataRateSeconds;

	__l3gdGyroExtendedData->DeltaX += deltaX;
	__l3gdGyroExtendedData->DeltaY -= deltaY;
	__l3gdGyroExtendedData->DeltaZ -= deltaZ;

	/* The Gyro and Accelerotmeter are 90 degress offset */

	__l3gdGyroExtendedData->RateYDPS = ((float)(int16_t)(_l3gd20XFilter.Current - __l3gd20Offset.x)) * __l3gd20LSBSensitivity;
	__l3gdGyroExtendedData->RateXDPS = -((float)(int16_t)(_l3gd20YFilter.Current - __l3gd20Offset.y)) * __l3gd20LSBSensitivity;
	__l3gdGyroExtendedData->RateZDPS = -((float)(int16_t)(_l3gd20ZFilter.Current - __l3gd20Offset.z)) * __l3gd20LSBSensitivity;

	/* Yes, we know these are mapped to complement, just has to do with orientation of the chips on the board */
	GyroL3GD20Sensor->Y += deltaX;
	GyroL3GD20Sensor->X -= deltaY;
	GyroL3GD20Sensor->Z -= deltaZ;

	if(GyroL3GD20Sensor->Z < 0.0f)
		GyroL3GD20Sensor->Z += 360.0f;
	else if(GyroL3GD20Sensor->Z > 360.0f)
		GyroL3GD20Sensor->Z -= 360.0f;
}

void __twb_imu_l3gd20_RegisterRead(void *i2cjob){
	GyroL3GD20Sensor->DataReady = DataReady;
}

void __twb_imu_l3gd20_QueueRead(void *i2cjob){
	L3GD20FiFoState = FiFoState_ReadQueue;
	GyroL3GD20Sensor->DataReady = DataReady;
}

void __twb_imu_l3gd20_QueueSizeRead(void *i2cjob){
	L3GD20FiFoState = FiFoState_ReadQueueSize;
	GyroL3GD20Sensor->DataReady = DataReady;
}

void __twb_imu_l3gd20_QueueFiFoRead(void){
	uint8_t sampleCount = L3GD20GryoI2CJob->InBuffer[0] & 0b00011111;

	L3GD20GryoI2CJob->Address = L3G20ADDR;
	L3GD20GryoI2CJob->OutBufferSize = 1;
	L3GD20GryoI2CJob->OutBuffer[0] = L3GD20_OUT_X_L | 0x80;
	L3GD20GryoI2CJob->InBufferSize = L3GD20_FIFO_SLOT_SIZE * sampleCount;
	L3GD20GryoI2CJob->Callback = &__twb_imu_l3gd20_QueueRead;

	TWB_I2C_QueueAsyncJob(L3GD20GryoI2CJob);

	L3GD20FiFoState = FiFoState_ReadingQueue;
}

iOpResult_e  TWB_IMU_L3GD30_ProcessData(){
	if(SensorConfig->L3GD20_FIFO_Enable == Disabled){
		__twb_imu_l3gd20_ProcessSlot(0);
	}
	else if(L3GD20FiFoState == FiFoState_Idle){
		__l3gdGyroExtendedData->RateXDPS += ((float)(int16_t)(_l3gd20XFilter.Current - __l3gd20Offset.x)) * __l3gd20LSBSensitivity;
		__l3gdGyroExtendedData->RateYDPS += ((float)(int16_t)(_l3gd20YFilter.Current - __l3gd20Offset.y)) * __l3gd20LSBSensitivity;
		__l3gdGyroExtendedData->RateZDPS += ((float)(int16_t)(_l3gd20ZFilter.Current - __l3gd20Offset.z)) * __l3gd20LSBSensitivity;
	}
	else if(L3GD20FiFoState == FiFoState_ReadQueueSize)
		__twb_imu_l3gd20_QueueFiFoRead();
	else if(L3GD20FiFoState == FiFoState_ReadQueue){
		uint8_t sampleCount = L3GD20GryoI2CJob->InBufferSize / L3GD20_FIFO_SLOT_SIZE;
		for(uint8_t slotIndex = 0; slotIndex < sampleCount; ++slotIndex)
			__twb_imu_l3gd20_ProcessSlot(slotIndex);

		L3GD20FiFoState = FiFoState_Idle;
	}
	else{
		/*
		 * Only Other Option is a simple register
		 * read, so just read in first set of X, Y, Z
		 */


		L3GD20FiFoState = FiFoState_Idle;
	}

	return OK;
}

/*
 * Primary starting point for
 * doing a read from the gyro.
 */
iOpResult_e __twb_imu_l3gd20_Read(void){
	if(SensorConfig->L3GD20_FIFO_Enable == Enabled){
		L3GD20GryoI2CJob->Address = L3G20ADDR;
		L3GD20GryoI2CJob->OutBufferSize = 1;
		L3GD20GryoI2CJob->OutBuffer[0] = L3GD20_FIFO_SRC_REG;
		L3GD20GryoI2CJob->InBufferSize = 1; /* Only read one character, the size of the queue */
		L3GD20GryoI2CJob->Callback = &__twb_imu_l3gd20_QueueSizeRead;

		L3GD20FiFoState = FiFoState_ReadingQueueSize;
	}
	else{
		L3GD20GryoI2CJob->Address = L3G20ADDR;
		L3GD20GryoI2CJob->OutBufferSize = 1;
		L3GD20GryoI2CJob->OutBuffer[0] = L3GD20_OUT_X_L | 0x80;
		L3GD20GryoI2CJob->InBufferSize = 6; /* Read X, Y & Z */
		L3GD20GryoI2CJob->Callback = &__twb_imu_l3gd20_RegisterRead;

		L3GD20FiFoState = FiFoState_Idle;
	}

	return TWB_I2C_QueueAsyncJob(L3GD20GryoI2CJob);
}

/* Gets called when IRQ is asserted (Usually WTM, but could be DRDY */
iOpResult_e TWB_IMU_L3GD20_IRQ(){
	return __twb_imu_l3gd20_Read();
}

/* Gets called at the sample rate for the register */
iOpResult_e TWB_IMU_L3GD20_Read(float deltaT){
	return __twb_imu_l3gd20_Read();
}

void TWB_IMU_L3GD20_SendDiag(void){
	_commonDiagBuffer->SensorId = GyroL3GD20Sensor->SensorId;

	/*
	_commonDiagBuffer->RawY = (int16_t)(_l3gd20XFilter.Current - __l3gd20Offset.x);
	_commonDiagBuffer->RawX = (int16_t)(_l3gd20YFilter.Current - __l3gd20Offset.y);
	_commonDiagBuffer->RawZ = (int16_t)(_l3gd20ZFilter.Current - __l3gd20Offset.z);
*/
	_commonDiagBuffer->RawX = (int16_t)(__l3gdGyroExtendedData->RateXDPS);
	_commonDiagBuffer->RawY = (int16_t)(__l3gdGyroExtendedData->RateYDPS);
	_commonDiagBuffer->RawZ = (int16_t)(__l3gdGyroExtendedData->RateZDPS);

	_commonDiagBuffer->X = (int16_t)(GyroL3GD20Sensor->X * 10.0f);
	_commonDiagBuffer->Y = (int16_t)(GyroL3GD20Sensor->Y * 10.0f);
	_commonDiagBuffer->Z = (int16_t)(GyroL3GD20Sensor->Z * 10.0f);

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
