#ifndef APPDATA_H
#define APPDATA_H

#include "Enums.h"
#include "def.h"

ConnectionStates ConnectionState = Disconnected;
GPIOStates GPIOState = GPIOStartup;
PlatformEnum PlatformType = PlatformMicro;
SystemStates SystemState = SysStartingUp;
IsArmedStates IsArmed = Safe;
ControlMethods ControlMethod = Control_RC;
AltitudeHoldMethod AltitudeHold = AltitudeHoldOff;
FrameConfigEnum FrameConfig = FrameConfig_X;
FlightModeEnum FlightMode = FlightModeStable;
SensorStates SensorState = SnsrStartup;

uint8_t LocalAddress = 20;

char DroneName[DRONE_NAME_LENGTH];

uint16_t acc_1G;             // this is the 1G measured acceleration
int16_t  acc_25deg;

int16_t  accSmooth[3];       // projection of smoothed and normalized gravitation force vector on x/y/z axis, as measured by accelerometer
int16_t  accTrim[2] = { 0, 0 };
//static int16_t  heading, magHold;

int32_t  pressure;
int32_t  BaroAlt;
int32_t  EstVelocity;
int32_t  EstAlt;             // in cm

//for log
uint16_t cycleTimeMax = 0;       // highest ever cycle timen
uint16_t cycleTimeMin = 65535;   // lowest ever cycle timen
int16_t  i2c_errors_count = 0;


// ******************
// rc functions
// ******************
#define MINCHECK 1100
#define MAXCHECK 1900

int16_t rcData[8];    // interval [1000;2000]
int16_t rcCommand[4]; // interval [1000;2000] for THROTTLE and [-500;+500] for ROLL/PITCH/YAW 
uint8_t rcRate8;
uint8_t rcExpo8;
int16_t lookupRX[7]; //  lookup table for expo & RC rate

// **************
// gyro+acc IMU
// **************
//int16_t  gyroADC[3], accADC[3], magADC[3]; // Raw Values

//int16_t gyroData[3] = { 0, 0, 0 };
int16_t gyroZero[3] = { 0, 0, 0 };
int16_t accZero[3] = { 0, 0, 0 };
int16_t magZero[3] = { 0, 0, 0 };

static float   magGain[3] = { 1.0, 1.0, 1.0 };  // gain for each axis, populated at sensor init
static uint8_t magInit = 0;

int8_t  smallAngle25 = 1;

// *************************
// motor and servo functions
// *************************
int16_t axisPID[3];
int16_t motor[8];

// **********************
// EEPROM & LCD functions
// **********************
uint8_t P8[5], I8[5], D8[5]; //8 bits is much faster and the code is much shorter
uint8_t dynP8[3], dynI8[3], dynD8[3];
uint8_t rollPitchRate;
uint8_t yawRate;
uint8_t dynThrPID;


// ***********************
// IMU Filter Values
// ***********************
uint16_t GyroAccCompFilter;
uint16_t GyroMagCompFilter;
uint16_t AccBaroCompFilter;

uint16_t lastModeChan = 0;

typedef struct {
	int16_t  accSmooth[3];
	int16_t  gyroData[3];
	int16_t  magADC[3];
	int16_t  gyroADC[3];
	int16_t  accADC[3];
} imu_t;

typedef struct {
	uint8_t  vbat;               // battery voltage in 0.1V steps
	uint16_t intPowerMeterSum;
	uint16_t rssi;              // range: [0;1023]
	uint16_t amperage;
} analog_t;

typedef struct {
	int32_t  EstAlt;             // in cm
	int16_t  vario;              // variometer in cm/s
} alt_t;

typedef struct {
	int16_t angle[2];            // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
	int16_t heading;             // variometer in cm/s
} att_t;

extern imu_t imu;
extern analog_t analog;
extern alt_t alt;
extern att_t att;

#endif