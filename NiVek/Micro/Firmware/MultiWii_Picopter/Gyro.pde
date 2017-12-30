#include "def.h"


extern uint8_t rawADC[]; 
extern uint16_t accCalibrationSampleCount;
uint16_t gyroCalibrationSampleCount = 0;

void calibrateGyros() {
	uint8_t axis;
	static int32_t g[3];

	for (axis = 0; axis < 3; axis++) {
		if (gyroCalibrationSampleCount == GYRO_CALIBRATION_SAMPLES)
			g[axis] = 0;

		g[axis] += imu.gyroADC[axis];

		imu.gyroADC[axis] = 0;

		if (gyroCalibrationSampleCount == 1)
			gyroZero[axis] = g[axis] / GYRO_CALIBRATION_SAMPLES;
	}

	if (accCalibrationSampleCount == 0 && gyroCalibrationSampleCount == 1)
		calibrationFinished();

	gyroCalibrationSampleCount--;
}

void clearGyroZero() {
	gyroZero[ROLL] = 0;
	gyroZero[PITCH] = 0;
	gyroZero[YAW] = 0;
}

void beginGyroZero() {
	clearGyroZero();

	gyroCalibrationSampleCount = GYRO_CALIBRATION_SAMPLES;
}

void GYRO_Common() {
	static int16_t previousGyroADC[3] = { 0, 0, 0 };
	uint8_t axis;

	if (gyroCalibrationSampleCount > 0)
		calibrateGyros();

	for (axis = 0; axis < 3; axis++) {
		imu.gyroADC[axis] -= gyroZero[axis];
		//anti gyro glitch, limit the variation between two consecutive readings
		imu.gyroADC[axis] = constrain(imu.gyroADC[axis], previousGyroADC[axis] - 800, previousGyroADC[axis] + 800);
		previousGyroADC[axis] = imu.gyroADC[axis];

		
	}
}

bool GYRO_Read() {
	if (MPU60x0_ReadGyro() == false){
		Serial.write("BAD GYRO");
		return false;
	}

	Serial.println(((rawADC[0] << 8) | rawADC[1]) / 4);

	GYRO_ORIENTATION(+(((rawADC[2] << 8) | rawADC[3]) / 4), // range: +/- 8192; +/- 2000 deg/sec
		-(((rawADC[0] << 8) | rawADC[1]) / 4),
		-(((rawADC[4] << 8) | rawADC[5]) / 4));

	GYRO_Common();

	return true;
}
