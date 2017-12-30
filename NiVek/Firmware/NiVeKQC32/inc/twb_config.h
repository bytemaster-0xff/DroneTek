/*
 * twb_config.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef TWB_CONFIG_H_
#define TWB_CONFIG_H_

#include "common/twb_common.h"
#include <math.h>

#include "stm32f4xx.h"

#define __ALIGN_BEGIN
#define __ALIGN_END


typedef struct {
	__ALIGN_BEGIN uint8_t *Buffer; __ALIGN_END
	uint16_t PtrIn;
	uint16_t PtrOut;
	uint16_t Len;
	uint16_t Size;
} Ring_Buffer_t;


typedef struct vector{
	float x, y, z;
} vector_t;

typedef struct u16_vector{
	uint16_t x, y, z;
} u16_vector_t;

typedef struct s16_vector{
	int16_t x, y, z;
} s16_vector_t;

typedef enum {
	Calibrated = 0xFF,
	NotCalibrated = 0x00
}IsCalibrated_t;

typedef enum {
	SystemStartingUp = 1,
	SystemSensorRestartRequired = 10,
	SystemRestarting = 11,
	SystemZeroing = 20,
	SystemResettingDefaults = 30,
	SystemReady = 100,
	SystemFailure = 0
} SystemStatus_e;


typedef enum {
	SensorAppState_Startup = 0,
	SensorAppState_Initializing = 1,

	SensorAppState_Ready = 2,

	SensorAppState_ResettingDefaults = 11,

	SensorAppState_Starting = 100,
	SensorAppState_Restarting = 101,
	SensorAppState_Resuming = 102,
	SensorAppState_Online = 103,

	SensorAppState_Stopping = 104,
	SensorAppState_Offline = 105,

	SensorAppState_Pausing = 106,
	SensorAppState_Paused = 107,

	SensorAppState_Diagnostics = 108,

	SensorAppState_Failure = 1,

	SensorAppState_Zero = 110,
	SensorAppState_Zeroing = 111,
	SensorAppState_ZeroCompleted = 112,

	SensorAppState_SnsrFusionPitch = 120,
	SensorAppState_SnsrFusionRoll = 121,
	SensorAppState_SnsrFusionHeading = 122,
	SensorAppState_SnsrFusionAltitude = 123,

	SensorAppState_MagCalibration = 140,
	SensorAppState_FinishingCalibration = 142,

	SensorAppState_PIDConfig = 150

} SensorStates_e;

typedef enum {
	FrameConfig_Cross = 10,
	FrameConfig_X = 20
} FrameConfig_e;

typedef enum {
	PitchDoesFollowRollPID = 0xFF,
	PitchDoesNotFollowRollPID = 0x00
} PitchFollowRollPID_e;

typedef enum {
	GPIOAppState_Startup = 0,
	GPIOAppState_Ready = 50,
	GPIOAppState_Launching = 51,
	GPIOAppState_Flying = 52,
	GPIOAppState_Landing = 53,
	GPIOAppState_Crashed = 60,
	GPIOAppState_Idle = 100,
	GPIOAppState_MotorConfig = 110,
	GPIOAppState_PIDConfig = 120,
	GPIOAppState_RCStartCalibration = 160,
	GPIOAppState_RCCalibration = 161,
	GPIOAppState_RCEndCalibration = 162
} FlightCtrlStates_e;

typedef enum {
	Measure_None = 0,
	Measure_Pitch = 10,
	Measure_Roll = 11,
	Measure_Heading = 12,
	Measure_Altitude = 13
} Measure_e;

typedef enum {
	DataReady,
	DataNotReady,
} SensorDataReady_e;

typedef enum {
	IRQAsserted,
	IRQNotAsserted,
} IRQAssertionStatus_e;


typedef enum {
	Armed = 0xFF,
	Safe = 0x00
} GPIOArmedState_e;


typedef enum {
	None = 0,
	WiFiControlled = 20,
	RCControlled = 40
} ControlMethodState_e;

typedef enum {
	WiFiConnected = 255,
	WiFiDisconnected = 0
} WiFiConnected_t;

typedef enum {
	ESC_Front = 1,
	ESC_Left = 2,
	ESC_Right = 3,
	ESC_Rear = 4
} Motors_t;

typedef enum {
	AutoPilot_Idle,
	AutoPilot_Paused,
	AutoPilot_Disabled,
	AutoPilot_Active,
} AutoPilotState_e;


typedef enum {
	DGPS_None_0,
	DGPS_RTCM_1,
	DGPS_WAAS_2
} DGPSModes_e;


#define Motor_PortFront ESC_Front
#define Motor_PortRear ESC_Left
#define Motor_StarboardFront ESC_Right
#define Motor_StarboardRear ESC_Rear

#define Motor_Front ESC_Front
#define Motor_Port ESC_Left
#define Motor_Starboard ESC_Right
#define Motor_Rear ESC_Rear


typedef enum  {
	AltitudeHold_On = 255,
	AltitudeHold_Off = 0,
} AltitudeHold_e;

typedef enum {
	SampleRate_1KHz = 0,
	SampleRate_500Hz = 1,
	SampleRate_200Hz = 2,
	SampleRate_150Hz = 3,
	SampleRate_100Hz = 4,
	SampleRate_50Hz = 5,
	SampleRate_25Hz = 6,
	SampleRate_20Hz = 7,
	SampleRate_10Hz = 8,
	SampleRate_5Hz = 9,
	SampleRate_1Hz = 10,
	SampleRate_IRQ = 0xFF
} SampleRate_e;


typedef enum {
	FiFoState_Idle,
	FiFoState_ReadingQueueSize,
	FiFoState_ReadQueueSize,
	FiFoState_ReadingQueue,
	FiFoState_ReadQueue
} FiFoState_e;

typedef enum {
	GPS_NoFix,
	GPS_2DFix,
	GPS_3DFix
} FixType_e;

typedef enum {
	FlightMode_Stable,
	FlightMode_Acro
} FlightMode_e;

typedef struct {
	uint16_t FirmwareVersion;
    uint8_t Platform;
	SystemStatus_e SystemState;
	SensorStates_e SnsrState;
	FlightCtrlStates_e GPIOState;
	GPIOArmedState_e ArmedStatus;
	ControlMethodState_e ControlMethod;
	AltitudeHold_e AltitudeHold;
	FrameConfig_e FrameConfig;
	FlightMode_e FlightMode;

} SystemStatus_t;

typedef struct {
	//These values are the current power of the motors
	uint8_t ESC_Front; // (0-255) Not Scaled
	uint8_t ESC_Left; // (0-255) Not Scaled
	uint8_t ESC_Right; // (0-255) Not Scaled
	uint8_t ESC_Rear; // (0-255) Not Scaled
} MotorStatus_t;

typedef struct {
	int16_t TargetHeading; // (0-360) Not Scaled
	int16_t TargetAltitude; // (0-360) Not Scaled

	//These values are the ones used in the control loop.
	uint8_t Throttle; // (0-100) Not Scaled
	int8_t TargetPitch; // (-90 - +90) Not Scaled
	int8_t TargetRoll; // (-90 - +90) Not Scaled
	int8_t Yaw; // (-50 - +50) Not Scaled
} TargetsData_t;

/* Package takes slightly under 10ms at 57600 baud */
typedef struct {
	int16_t Pitch; // Scaled to 1 place
	int16_t Roll; // Scaled to 1 place
	uint16_t Heading;   // Int Value (0-360) // Not Scaled
	int16_t AltitudeM;   // Int Value (0-65535), with respect to take off, zero decimal places

} SensorData_t;

typedef struct {
	int16_t Pitch; // Scaled to 1 place
	int16_t Roll; // Scaled to 1 place
	uint16_t Heading;   // Int Value (0-360) // Not Scaled
	uint16_t AltitudeM;   // Int Value (0-65535), with respect to take off, zero decimal places

	int8_t TargetPitch; // (-90 - +90) Not Scaled
	int8_t TargetRoll; // (-90 - +90) Not Scaled
	int16_t TargetHeading; // (0-360) Not Scaled
	int16_t TargetAltitude; // (0-360) Not Scaled

	uint8_t ESC_Front; // (0-255) Not Scaled
	uint8_t ESC_Left; // (0-255) Not Scaled
	uint8_t ESC_Right; // (0-255) Not Scaled
	uint8_t ESC_Rear; // (0-255) Not Scaled
} PIDSensorData_t;

typedef struct {
	int16_t Altitude; /* Meters scaled to CM or 2 decimal places */
	int16_t Sonar;
	int16_t BMP085;
	int16_t MS5611;
	int16_t GPS;
} AltitudeData_t;

typedef struct {
	float Pitch;

	float Roll;

	float Heading;

	float AltitudeM;

	float AltDelta;

	float PitchRateDPS;
	float RollRateDPS;
	float YawRateDPS;
	float AltitudeRate;

	float Pressure_hPa;
	float Temperature;

	float Throttle;

	float TargetPitch;
	float TargetRoll;
	float TargetHeading;
	float TargetAltitude;

	float TargetPitchRate;
	float TargetRollRate;
	float TargetHeadingRate;
	float TargetAltitudeRate;

	float ESC_Front;
	float ESC_Port;
	float ESC_Starboard;
	float ESC_Rear;
} InternalState_t;

typedef struct {
	uint16_t Cell1Battery; //44
	uint16_t Cell2Battery; //46
	uint16_t Cell3Battery; //48
} BatteryCondition_t;

// P I are scaled by a factor of 10000
// D is a scaled by a factor of 1000
// Motor Constants are scaled integers with 2 places behind the decimal point.
typedef struct {
	int16_t k_pitch_rate_p; // 0
	int16_t k_pitch_rate_i; // 2
	int16_t k_pitch_rate_d; // 4
	uint16_t pitch_steady_i_decay_factor; // 6
	uint16_t pitch_rate_max_i; // 8
	int16_t k_pitch_steady_p; // 10
	int16_t k_pitch_steady_i; // 12
	int16_t k_pitch_steady_d; // 14
	uint16_t pitch_rate_i_decay_factor;  // 16
	uint16_t pitch_steady_max_i; // 18


	int16_t k_roll_rate_p;  	// 20
	int16_t k_roll_rate_i;  	// 22
	int16_t k_roll_rate_d;  	// 24
	uint16_t roll_rate_i_decay_factor;  // 26
	uint16_t roll_rate_max_i; 	// 28
	int16_t k_roll_steady_p;   	// 30
	int16_t k_roll_steady_i;   	// 32
	int16_t k_roll_steady_d;   	// 34
	uint16_t roll_steady_i_decay_factor;  // 36
	uint16_t roll_steady_max_i; 	// 38


	int16_t k_heading_rate_p; // 40
	int16_t k_heading_rate_i; // 42
	int16_t k_heading_rate_d; // 44
	uint16_t heading_rate_i_decay_factor; //46
	uint16_t heading_rate_max_i; 	// 48
	int16_t k_heading_steady_p; 		// 50
	int16_t k_heading_steady_i; 		// 52
	int16_t k_heading_steady_d;   	// 54
	uint16_t heading_steady_i_decay_factor; //56
	uint16_t heading_steady_max_i; 	// 58


	int16_t k_altitude_rate_p; 		// 60
	int16_t k_altitude_rate_i; 		// 62
	int16_t k_altitude_rate_d; 		// 64
	uint16_t altitude_rate_i_decay_factor; // 66
	uint16_t altitude_rate_max_i; 	// 68
	int16_t k_altitude_steady_p; 		// 70
	int16_t k_altitude_steady_i; 		// 72
	int16_t k_altitude_steady_d; 		// 74
	uint16_t altitude_steady_i_decay_factor; //76
	uint16_t altitude_steady_max_i; 	// 78

	int16_t k_esc_front; 			// 80
	int16_t k_esc_left;  			// 82
	int16_t k_esc_right; 			// 84
	int16_t k_esc_rear;  			// 86

	uint8_t ESCUpdateRate; 			// 88

	uint8_t PitchStableBand; 		// 89 - Degrees where we do not apply and stability, in 1/10 of a degree
	uint8_t RollStableBand; 		// 90 - Degrees where we do not apply and stability, in 1/10 of a degree
	uint8_t HeadingStableBand; 		// 91 - Degrees where we do not apply and stability, in 1/10 of a degree
	uint8_t AltitudeStableBand; 	// 92 - CMs where don't apply any corrections, in 1/10 of a degree

	uint8_t PIDSampleRate; 			// 93

	uint8_t ThrottleSensitivity;      // 94  0 - 25
	int8_t PitchSensitivity; 		// 95  -10.0 - 10.0 as a scaled decimal
	int8_t RollSensitivity; 		// 96 -10.0 - 10.0 as a scaled decimal
	int8_t YawSensitivity; 			// 97 -10.0 - 10.0 as a scaled decimal

	uint8_t pitch_filter_d; 		// 98
	uint8_t roll_filter_d;  		// 99
	uint8_t heading_filter_d;  		// 100
	uint8_t altitude_filter_d;  	// 101
	FrameConfig_e FrameConfig; 		// 102

	uint8_t InitialThrottle; 		// 103 - 0 - 255 Throttle Percent
	uint8_t InitialAltitude; 		// 104 - 0 - 255 CM

	SampleRate_e AutoPilotSampleRate; 	// 105

	PitchFollowRollPID_e PitchFollowsRollPID; // 106 0x00/0xFF

	uint8_t ThrottleDeadSpace;  //0x107

} PIDConstantsConfig_t;

// P and D values are scaled integers with 2 places behind the decimal
// D is a scaled integer with 3 places behind the decimal
// Motor Constants are scaled integers with 2 places behind the decimal point.
typedef struct {
	float k_esc_left;
	float k_esc_right;
	float k_esc_front;
	float k_esc_rear;

	float PitchStableBand; // 27 - Degrees where we do not apply and stability, in 1/10 of a degree
	float RollStableBand; // 28 - Degrees where we do not apply and stability, in 1/10 of a degree
	float HeadingStableBand; // 29 - Degrees where we do not apply and stability, in 1/10 of a degree
	float AltitudeStableBand; // 30 - CM where we don't attempt to correct for altitude errors

	float ThrottleSensitivity; // 0 - 25.0 as scaled decimal
	float PitchSensitivity; // -10.0 - 10.0 as a scaled decimal
	float RollSensitivity; // -10.0 - 10.0 as a scaled decimal
	float YawSensitivity; // -10.0 - 10.0 as a scaled decimal

} PIDConstants_t;

typedef struct {
	char Latitude[10];  //0 - 9
	uint8_t NorthSouth; //10
	char Longitude[10]; //11
	uint8_t EastWest; //21
	char SatellitesUsed[2]; //22
	uint8_t ValidFix; //24
	uint16_t GpsHeading; //25
	int16_t Altitude; //27

} GPSData_t;

typedef struct {
	float Latitude;
	float Longitude;
	uint8_t SatellitesUsed;
	float Error;
	FixType_e FixType;
	float Heading;
	float AltitudeM;
	float GroundSpeedMS;
} GPSReading_t;

typedef enum {
  SensorNone = 0x00,

  GyroITG3200 = 0x20,
  GyroMPU60x0 = 0x21,
  GyroL3GD20 = 0x22,

  AccLSM303 = 0x30,
  AccADXL345 = 0x31,
  AccMPU60x0 = 0x32,

  MagLSM303 = 0x40,
  MagHMS5883 = 0x41,
  MagGPS = 0x42,

  AltBMP085 = 0x50,
  AltSonar = 0x51,
  AltGPS = 0x52,
  AltMS5611 = 0x53,

  ADCLipo = 0x60,

  GeoGPS = 0x70,

  SnsrFusionComplementary = 0x80,
  SnsrFusionKalman = 0x81,

  PIDController = 0x90,
  AutoPilot = 0xA0
} SensorDevice_e;

typedef enum {
	NO_SONAR = 0x00,
	HC_SR04 = 0x01,
	MB1030_EZ3 = 0x02
} SonarType_e;

typedef enum {
	UpdateRate_400Hz = 0,
	UpdateRate_300Hz = 1,
	UpdateRate_250Hz = 2,
	UpdateRate_200Hz = 3,
	UdpateRate_150Hz = 4,
	UpdateRate_125Hz = 5,
	UpdateRate_100Hz = 6,
	UpdateRate_75Hz = 7,
	UpdateRate_50Hz = 8,
	UpdateRate_40Hz = 9,
	UpdateRate_30Hz = 10,
	UpdateRate_25Hz = 11,
	UpdateRate_20Hz = 12,
	UpdateRate_15Hz = 13,
	UpdateRate_10Hz = 14,
	UpdateRate_5Hz = 15,
	UpdateRate_1Hz = 16
} UpdateRate_e;

typedef enum {
	CompFilter_1sec = 0,
	CompFilter_900Ms = 1,
	CompFilter_800Ms = 2,
	CompFilter_700Ms = 3,
	CompFilter_600Ms = 4,
	CompFilter_500Ms = 5,
	CompFilter_400Ms = 6,
	CompFilter_300Ms = 7,
	CompFilter_200Ms = 8,
	CompFilter_150Ms = 9,
	CompFilter_100Ms = 10,
	CompFilter_50Ms = 11,
	CompFilter_25Ms = 12,
	CompFilter_20Ms = 13,
	CompFilter_15Ms = 14,
	CompFilter_10Ms = 15,
	CompFilter_05Ms = 16,
	CompFilter_02Ms = 17,
	CompFilter_01Ms = 18,
} CompFilterValues_t;


typedef enum {
	FilterOption_None = 0,
	FilterOption_Median_3_Sample = 0x01,
	FilterOption_Median_5_Sample = 0x02,
	FilterOption_Median_7_Sample = 0x03,
	FilterOption_Median_9_Sample = 0x04,
	FilterOption_Moving_Average_0_01 = 0x10,
	FilterOption_Moving_Average_0_1 = 0x20,
	FilterOption_Moving_Average_1 = 0x30,
	FilterOption_Moving_Average_2 = 0x40,
	FilterOption_Moving_Average_5 = 0x50,
	FilterOption_Moving_Average_10 = 0x60,
	FilterOption_Moving_Average_15 = 0x70,
	FilterOption_Moving_Average_20 = 0x80,
	FilterOption_Moving_Average_30 = 0x90,
	FilterOption_Moving_Average_40 = 0xA0,
	FilterOption_Moving_Average_50 = 0xB0,
	FilterOption_Moving_Average_60 = 0xC0,
	FilterOption_Moving_Average_70 = 0xD0,
	FilterOption_Moving_Average_80 = 0xE0,
	FilterOption_Moving_Average_90 = 0xF0,
} FilterOptions_t;

typedef enum {
	Enabled = 0xFF,
	Disabled = 0
} Enabled_t;

typedef struct {
	UpdateRate_e ControlLoopUpdateRateHz;

	SampleRate_e StatusSendDataRate; /* 7 Values */
	SampleRate_e GPSSendDataRate;
	SampleRate_e BatterySendDataRate;
	SampleRate_e SensorSendDataRate;
	SampleRate_e TargetsSendDataRate;
	SampleRate_e MotorsSendDataRate;
	SampleRate_e AltitudeSendDataRate;

	Enabled_t ITG3200_Enabled; /* 5 Values */
	uint8_t ITG3200_DLPF_FS;
	uint8_t ITG3200_SMPLRT_DIV;
	FilterOptions_t ITG3200_FilterOption;
	SampleRate_e ITG3200_SampleRate;

	Enabled_t MPU60x0_Gyro_Enabled; /* 2 Values */
	Enabled_t MPU60x0_Acc_Enabled;

	uint8_t MPU60x0_ACC_GYR_DLPF; /* 5 Values */
	uint8_t MPU60x0_GYR_SMPLRT_DIV;
	uint8_t MPU60x0_GYRO_FS;
	uint8_t MPU60x0_ACC_FS;
	Enabled_t MPU60x0_FIFO_Enable;

	FilterOptions_t MPU60x0_GyroFilterOption; /* 4 Values */
	FilterOptions_t MPU60x0_AccFilterOption;
	SampleRate_e MPU60x0_GyroSampleRate;
	SampleRate_e MPU60x0_AccSampleRate;

	Enabled_t LSM303_ACC_Enabled; /* 8 Values */
	uint8_t LSM303_ACC_REG1;
	uint8_t LSM303_ACC_REG2;
	uint8_t LSM303_ACC_REG3;
	uint8_t LSM303_ACC_REG4;
	Enabled_t LSM303_ACC_FIFO_Enable;
	FilterOptions_t LSM303_ACC_FilterOption;
	SampleRate_e LSM303_ACC_SampleRate;

	Enabled_t LSM303_MAG_Enabled; /* 7 Values */
	uint8_t LSM303_MAG_CRA;
	uint8_t LSM303_MAG_CRB;
	uint8_t LSM303_MAG_MR;
	FilterOptions_t LSM303_MAG_FilterOption;
	SampleRate_e LSM303_MAG_SampleRate;
	IsCalibrated_t LSM303_MAG_Calibrated;

	Enabled_t L3GD20_Enabled;
	uint8_t L3GD20_REG1;
	uint8_t L3GD20_REG2;
	uint8_t L3GD20_REG3;
	uint8_t L3GD20_REG4;
	uint8_t L3GD20_REG5;
	Enabled_t L3GD20_FIFO_Enable;
	FilterOptions_t L3GD20_FilterOption;
	SampleRate_e L3GD20_SampleRate;

	Enabled_t ADXL345_Enabled;
	uint8_t ADXL345_BW_RATE;
	uint8_t ADXL345_DATA_FORMAT;
	Enabled_t ADXL345_FIFO_Enable;
	FilterOptions_t ADXL345_FilterOption;
	SampleRate_e ADXL345_SampleRate;

	Enabled_t HMC5883_Enabled;
	uint8_t HMC5883_CRA;
	uint8_t HMC5883_CRB;
	uint8_t HMC5883_MODE;
	FilterOptions_t HMC5883_FilterOption;
	SampleRate_e HMC5883_SampleRate;
	IsCalibrated_t HMC5883_Calibrated;

	Enabled_t Sonar_Enabled;
	SonarType_e Sonar_Type;
	FilterOptions_t Sonar_FilterOption;
	SampleRate_e Sonar_SampleRate;

	Enabled_t BMP085_Enabled;
	uint8_t BMP085_NumberSamples;
	uint8_t BMP085_OverSamplingRate;
	FilterOptions_t BMP085_FilterOption;
	SampleRate_e BMP085_SampleRate;

	Enabled_t MS5611_Enabled;
	uint8_t MS5611_OverSamplingRate;
	FilterOptions_t MS5611_FilterOption;
	SampleRate_e MS5611_SampleRate;

	DGPSModes_e GPS_DGPSMode;

	Enabled_t LIPO_ADC_Enabled;
	FilterOptions_t LIPO_ADC_FilterOption;
	SampleRate_e LIPO_ADC_SampleRate;

	Enabled_t Kalman_Enabled;
	SampleRate_e Kalman_SampleRate;

} SensorConfig_t;

typedef struct {
	Enabled_t Enabled;
	SampleRate_e SampleRate;

	FilterOptions_t FilterOptionPitch;
	FilterOptions_t FilterOptionRoll;
	FilterOptions_t FilterOptionHeading;
	FilterOptions_t FilterOptionAltitude;

	CompFilterValues_t TimeConstantPitch;
	CompFilterValues_t TimeConstantRoll;
	CompFilterValues_t TimeConstantHeading;
	CompFilterValues_t TimeConstantAltitude;

	SensorDevice_e Sensor1Pitch;
	SensorDevice_e Sensor2Pitch;
	SensorDevice_e Sensor3Pitch;

	SensorDevice_e Sensor1Roll;
	SensorDevice_e Sensor2Roll;
	SensorDevice_e Sensor3Roll;

	SensorDevice_e Sensor1Heading;
	SensorDevice_e Sensor2Heading;
	SensorDevice_e Sensor3Heading;

	SensorDevice_e Sensor1Altitude;
	SensorDevice_e Sensor2Altitude;
	SensorDevice_e Sensor3Altitude;

} SensorFusion_t;

extern SensorFusion_t *ComplementaryFilterConfig;


typedef struct {
	uint16_t Radio1Raw;
	uint16_t Radio2Raw;
	uint16_t Radio3Raw;
	uint16_t Radio4Raw;
	uint16_t Radio5Raw;
	uint16_t Radio6Raw;

	uint16_t CalMinPitch;
	uint16_t CalIdlePitch;
	uint16_t CalMaxPitch;

	uint16_t CalMinRoll;
	uint16_t CalIdleRoll;
	uint16_t CalMaxRoll;

	uint16_t CalMinYaw;
	uint16_t CalIdleYaw;
	uint16_t CalMaxYaw;

	uint16_t CalMinThrottle;
	uint16_t CalMaxThrottle;

	uint8_t Throttle;
	int8_t Pitch;
	int8_t Roll;
	int8_t Yaw;

	uint8_t Aux1;
	uint8_t Aux2;

	IsCalibrated_t IsCalibrated;
} PWMIn_t;

typedef struct {
	float ThisError;
	float LastError;

	float ThisRate;

	float ThisRateError;
	float LastRateError;

	float DeltaT;

	float k_RateP;
	float k_RateI;
	float k_RateD;

	float k_SteadyP;
	float k_SteadyI;

	float DerivativeFilter;
	float Rate_MaxSigmaError;
	float Steady_MaxSigmaError;

	float ISteadyDecayFactor;
	float IRateDecayFactor;

	float SteadySigmaError;
	float RateSigmaError;

	float LastDerivative;

	float MaxOutputPct;

	float Output;

	float TargetRate;

	float NormalizedOutput;

	float pSteadyComponent;
	float iSteadyComponent;

	float pRateComponent;
	float iRateComponent;
	float dRateComponent;

	float StableBand;
} PID_t;


typedef struct {
	int16_t RawX;
	int16_t RawY;
	int16_t RawZ;

	int16_t X;
	int16_t Y;
	int16_t Z;

	uint8_t SensorId;

} SensorDiagData_t;

typedef struct {
	int16_t ErrorSigma;
	int16_t Derivative;

	int16_t TargetRate;
	int16_t Rate;

	int16_t P_SteadyComponent;
	int16_t I_SteadyComponent;

	int16_t P_RateComponent;
	int16_t I_RateComponent;
	int16_t D_RateComponent;
	int16_t Angle;

	int8_t Target;
	uint8_t Throttle;

	uint8_t Power1;
	uint8_t Power2;

	uint8_t ForRoll;

} PIDTuning_t;


#define null 0x00

extern SensorDiagData_t *_commonDiagBuffer;

extern BatteryCondition_t *BatteryCondition;
extern GPSData_t *GPSData;
extern PIDConstantsConfig_t *PIDConstantsConfig;
extern SensorData_t *SensorData;
extern MotorStatus_t *MotorStatus;
extern SystemStatus_t *SystemStatus;
extern SensorConfig_t *SensorConfig;
extern TargetsData_t *Targets;
extern PWMIn_t *PWMIn;
extern InternalState_t *InternalState;
extern PIDConstants_t *PIDConstants;
extern PIDSensorData_t *PIDSensorData;

extern GPSReading_t *HomeGPSReading;
extern GPSReading_t *CurrentGPSReading;

extern AltitudeData_t *AltitudeData;

extern PIDTuning_t *RollPIDDetails;
extern PIDTuning_t *PitchPIDDetails;

extern PID_t *PIDPitch;
extern PID_t *PIDRoll;
extern PID_t *PIDYaw;
extern PID_t *PIDAltitude;

extern uint8_t WiFiThrottle;
extern int8_t WiFiYaw;
extern int8_t WiFiPitch;
extern int8_t WiFiRoll;

extern AutoPilotState_e AutoPilotState;

extern uint8_t *NiVekName;
extern uint8_t NiVekAddress;
extern uint32_t NivekSerialNumber;

extern Ring_Buffer_t *USB_Console_Buffer;
extern Ring_Buffer_t *USB_Telemetry_Buffer;
extern Ring_Buffer_t *USB_Debug_Buffer;


//Internal state variables
extern WiFiConnected_t WiFiIsConnected;
extern SensorDataReady_e SensorDataReady;

#define PRIMARY_I2C I2C1

#define TMR_PWM_CAPTURE     TIM14
#define TMR_PWM_CAPTURE_PERIPH RCC_APB1Periph_TIM14

#define TMR_MAIN_LOOP TIM7
#define TMR_MAIN_LOOP_PERIPH RCC_APB1Periph_TIM7

#define TMR_PWM_OUT_PRIMARY TIM3
#define TMR_PWM_OUT_PRIMARY_PERIPH RCC_APB1Periph_TIM3
#define TMR_PWM_OUT_PRIMARY_GPIO_AF GPIO_AF_TIM3

#define TMR_PWM_OUT_AUX_GPIO_AF GPIO_AF_TIM2
#define TMR_PWM_OUT_AUX     TIM2
#define TMR_PWM_OUT_AUX_PERIPH RCC_APB1Periph_TIM2

#define TMR_SONAR TIM10
#define TMR_SONAR_PERIPH RCC_APB2Periph_TIM10

#define TMR_SENSOR TIM9


#endif /* TWB_CONFIG_H_ */


