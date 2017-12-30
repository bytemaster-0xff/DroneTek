#ifndef ENUMS_H
#define ENUMS_H

enum GPIOStates
{
	GPIOStartup = 0,
	GPIOReady = 50,
	GPIOLaunching = 51,
	GPIOFlying = 52,
	GPIOLanding = 53,
	GPIOCrashed = 60,
	GPIOIdle = 100,
	GPIOMotorConfig = 110,
	GPIOPIDConfig = 120,
	GPIOStartRCCalibration = 160,
	GPIORCCalibration = 161,
	GPIORCEndCalibration = 162
};

enum ConnectionStates 
{
	Connected = 0xFF,
	Disconnected = 0x00
};

enum PlatformEnum
{
	PlatformFull = 100,
	PlatformMini = 101,
	PlatformMicro = 102
};

enum SystemStates
{
	SysStartingUp = 1,
	SysRestartRequired = 10,
	SysRestarting = 11,
	SysZeroing = 20,
	SysResettingDefaults = 30,
	SysReady = 100,
	SysFailure = 0
};

enum IsArmedStates
{
	Armed = 0xFF,
	Safe = 0x00
};

enum ControlMethods
{
	Control_None = 0,
	Control_RC = 40,
	Control_Digital = 20
};

enum AltitudeHoldMethod
{
	AltitudeHoldOn = 255,
	AltitudeHoldOff = 0
};

enum FrameConfigEnum
{
	FrameConfig_Cross = 0,
	FrameConfig_X = 1
};

enum FlightModeEnum
{
	FlightModeStable = 1,
	FlightModeAcrobat = 0
};

enum SensorStates
{
	SnsrStartup = 0,

	SnsrInitializing = 1,
	SnsrReady = 2,
	SnsrRestart = 10,
	SnsrResettingDefaults = 11,

	SnsrStarting = 100,
	SnsrRestarting = 101,
	SnsrResuming = 102,
	SnsrOnline = 103,
	SnsrStopping = 104,

	SnsrOffline = 105,
	SnsrPausing = 106,
	SnsrPaused = 107,

	SnsrDiagnostics = 108,

	SnsrZero = 110,
	SnsrZeroing = 111,
	SnsrZeroCompleted = 112,

	SnsrFusionPitch = 120,
	SnsrFusionRoll = 121,
	SnsrFusionHeading = 122,
	SnsrFusionAltitude = 123,

	SnsrXYCalibratingCompass = 140,
	SnsrZCalibratingCompass = 141,
	SnsrFinishingMagCal = 142,

	SnsrPIDConfig = 150
};

typedef enum SensorIds {
	SensorIds_MPU6050_Acc = 10,
	SensorIds_MPU6050_Gyro = 11,

	SensorIds_HMC5883_Mag = 20,

	SensorIds_MSU5611_Baro = 30,

	SensorIds_IMU_Attitude = 40,
	SensorIds_IMU_Heading = 41,
	SensorIds_IMU_Altitude = 42,
} SensorIds_t;

typedef enum AxisTypes{
	AxisType_None = 0,
	AxisType_Pitch = 10,
	AxisType_Roll = 11,
	AxisType_Heading = 12,
	AxisType_Altitude = 13,
} AxisTypes_t;


typedef struct fp_vector {
	float X;
	float Y;
	float Z;
} t_fp_vector_def;

typedef union {
	float   A[3];
	int32_t A16[3];
	int32_t A32[3];
	t_fp_vector_def V;
} t_fp_vector;


#endif