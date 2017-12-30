#include "def.h"

extern uint8_t rawADC[];
uint16_t accCalibrationSampleCount = 0;
extern uint16_t gyroCalibrationSampleCount;

void clearAccTrim() {
	accTrim[ROLL] = 0;
	accTrim[PITCH] = 0;
}

void clearAccZero() {
	accZero[ROLL] = 0;
	accZero[PITCH] = 0;
	accZero[YAW] = 0;
}

void calibrateACC() {
	static int32_t a[3];

	if (SensorState == SnsrZero)
		transitionToSensorState(SnsrZeroing);

	for (uint8_t axis = 0; axis < 3; axis++) {
		if (accCalibrationSampleCount == ACC_CALIBRATION_SAMPLES)
			a[axis] = 0;

		a[axis] += imu.accADC[axis];

		imu.accADC[axis] = 0;
	}

	// Calculate average, shift Z down by acc_1G and store values in EEPROM at end of calibration
	if (accCalibrationSampleCount == 1) {
		accZero[ROLL] = a[ROLL] / ACC_CALIBRATION_SAMPLES;
		accZero[PITCH] = a[PITCH] / ACC_CALIBRATION_SAMPLES;
		accZero[YAW] = a[YAW] / ACC_CALIBRATION_SAMPLES - acc_1G; // for nunchuk 200=1G

		clearAccTrim();

		writeParams(); // write accZero in EEPROM
	}

	if (accCalibrationSampleCount == 1 && gyroCalibrationSampleCount == 0)
		calibrationFinished();

	accCalibrationSampleCount--;
}

void beginAccZero() {
	clearAccZero();

	accCalibrationSampleCount = ACC_CALIBRATION_SAMPLES;
}

void ACC_Common() {
	if (accCalibrationSampleCount > 0)
		calibrateACC();

	imu.accADC[ROLL] -= accZero[ROLL];
	imu.accADC[PITCH] -= accZero[PITCH];
	imu.accADC[YAW] -= accZero[YAW];
}

void ACC_Read() {
	MPU60x0_ReadAcc();

	//usefull info is on the 14 bits  [2-15] bits  /4 => [0-13] bits  /8 => 11 bit resolution
	ACC_ORIENTATION(-((rawADC[0] << 8) | rawADC[1]) / 16,
		-((rawADC[2] << 8) | rawADC[3]) / 16,
		((rawADC[4] << 8) | rawADC[5]) / 16);

	ACC_Common();
}

