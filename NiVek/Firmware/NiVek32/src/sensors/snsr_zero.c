/*
 * sensor_zero.c
 *
 *  Created on: Jan 20, 2013
 *      Author: kevinw
 */

#include "common/twb_leds.h"
#include "sensors/snsr_main.h"
#include "common/twb_i2c.h"
#include "string.h"
#include "main.h"
#include "twb_eeprom.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "twb_config.h"

#define ZERO_PAUSE_BETWEEN_SENSORS 250.0f

Sensor_t *__sensorBeingZeroed;
I2CJob_t *I2CZeroJob;

/* Address where the calibration factors should be stored in the EEPROM */
int16_t __zero_EEPROM_Address;

/*
 * In a nut shell, here is how the zeroing process works
 *
 * It's designed to work in the background via IRQ's and the main loop so this should
 * never block the CPU
 *
 * 1) The commo loop for the sensors receives a command to zero
 *    - TWB_Sensors_Start_Zero get's called from the main loop
 *    - Puts the sensor state in Zero
 *    - Puts the Zero module state in zero
 *    - Calls TWB_Sensors_Zero with zero for the deltaT
 *    - Initially TWB_Sensors_Zero will be in the idle state so
 *      - Start flashing the sensor LED fast
 *      - Put the zero state into TWB_Sensors_Zeroing since the process has started.
 *      - Call BeginZero passing in the head for the sensors
 *         - BeginZero finds the first sensor that is enabled that supports "StartZero"
 *           and calls it.
 *         - If Start Zero returns true it means that calibration for that sensor has been
 *           completed (sync), so go ahead and do the next sensor, if not put it into a
 *           state where it waits for the I2C job to complete.
 *      - If BeginZero returns false it's done and we can resume with normal operation
 *        otherwise it just returns and lets the operation continue.
 *
 */

/*
 * Some crappy global variables, since
 * we really don't have C++ classes, just
 * picture these variables 'er encapsulated
 * in this file.
 */
int32_t __zero_TempCounts[3] = {0,0,0};
uint8_t __zero_TotalSampleCount;
uint8_t __zero_SamplesIndex;

float __zeroMsAccum;
float __zero_PauseMS;

bool __zero_BigEndian;

typedef enum {
	ZeroState_Idle,
	ZeroState_WaitingForI2CJob,
	ZeroState_PauseBetweenReading,
	ZeroState_I2C_ReadCompleted,
	ZeroState_I2C_StoringToE2PROM,
	ZeroState_PauseBetweenSensor
} ZeroStates_e;

ZeroStates_e __zero_CurrentState = Idle;

void __twb_sensorWriteCalComplete(void *asyncJob){
	__zero_CurrentState = ZeroState_PauseBetweenSensor;
	__zeroMsAccum = 0;
}


/* Callback from the I2C Job */
void __twb_sensorCalComplete(void *asyncJob){
	I2CJob_t *job = asyncJob;

	/*
	 * Since we throw the first 5 samples away,
	 * we make sure we are above 1 for the first sample index.
	 * prior to that, just let the bit's spill out over the
	 * PCB.
	 */
	if(++__zero_SamplesIndex > 0){
		if(__zero_BigEndian){
			__zero_TempCounts[0] += (int32_t)((job->InBuffer[0] << 8 | job->InBuffer[1]) | (job->InBuffer[0] & 0b10000000 ? 0xFFFF0000 : 0)) ;
			__zero_TempCounts[1] += (int32_t)((job->InBuffer[2] << 8 | job->InBuffer[3]) | (job->InBuffer[2] & 0b10000000 ? 0xFFFF0000 : 0)) ;
			__zero_TempCounts[2] += (int32_t)((job->InBuffer[4] << 8 | job->InBuffer[5]) | (job->InBuffer[4] & 0b10000000 ? 0xFFFF0000 : 0)) ;
		}
		else{
			__zero_TempCounts[0] += (int32_t)((job->InBuffer[1] << 8 | job->InBuffer[0]) | (job->InBuffer[1] & 0b10000000 ? 0xFFFF0000 : 0)) ;
			__zero_TempCounts[1] += (int32_t)((job->InBuffer[3] << 8 | job->InBuffer[2]) | (job->InBuffer[3] & 0b10000000 ? 0xFFFF0000 : 0)) ;
			__zero_TempCounts[2] += (int32_t)((job->InBuffer[5] << 8 | job->InBuffer[4]) | (job->InBuffer[5] & 0b10000000 ? 0xFFFF0000 : 0)) ;
		}
	}

	/*
	 * Since this needs to be really fast, don't
	 * do any processing here, let it all happen
	 * on the main "thread".
	 */
	__zero_CurrentState = ZeroState_I2C_ReadCompleted;
}

/*
 * This is call back into from the sensor when the sensor
 * needs to grab readings from the peripheral via I2C,
 * rather than each sensor implementing there own logic
 * to grab/average a bunch of readings, methods in this
 * file will make that happen.
 */
iOpResult_e TWB_Sensor_Calibrate(uint8_t i2cAddr, uint8_t regAddr, uint16_t eepromAddr, uint8_t sampleSize, float pauseMS, s16_vector_t *cals, bool _bigEndian){
	/* Throw the first 5 samples away */
	__zero_SamplesIndex = -5;
	__zero_TotalSampleCount = sampleSize;
	__zero_PauseMS = pauseMS;
	__zero_EEPROM_Address = eepromAddr;
	__zero_BigEndian = _bigEndian;

	__zero_TempCounts[0] = 0;
	__zero_TempCounts[1] = 0;
	__zero_TempCounts[2] = 0;

	I2CZeroJob->Address = i2cAddr;
	I2CZeroJob->OutBufferSize = 1;
	I2CZeroJob->OutBuffer[0] = regAddr;
	I2CZeroJob->InBufferSize = 6;
	I2CZeroJob->Callback = &__twb_sensorCalComplete;

	TWB_I2C_QueueAsyncJob(I2CZeroJob);

	return OK;
}

uint8_t _tmpBuffer[4];

/*
 * This method will be called to zero the next active sensor in the
 * sensor linked list.
 *
 * It will return false, if not done and will return true if it reached
 * the end of the linked list w/o starting a zero process.
 */
bool __twb_Sensors_BeginZero(Sensor_t *snsr){
	while(snsr != NULL){
		if(snsr->IsEnabled == Enabled && snsr->StartZero != NULL){
			TWB_Debug_Print("Zero ");
			TWB_Debug_Print(snsr->Name);
			TWB_Debug_Print("\r\n");

			__sensorBeingZeroed = snsr;
			/*
			 * When Start Zero is called it will return true
			 * if it calibration is sync and we can immediately
			 * sync the next sensor, and false if the sync
			 * method is async.  Either way we are guaranteed
			 * to have a minimum of 250ms between calibration
			 * of different sensors.
			 */
			if(snsr->StartZero(CALIBRATION_POINT_COUNT, CALIBRATION_WAIT_MS)){
				__zeroMsAccum = 0;
				__zero_CurrentState = ZeroState_PauseBetweenSensor;
			}
			else
				__zero_CurrentState = ZeroState_WaitingForI2CJob;

			/*
			 * Return true to let the caller know we something
			 * happened with the prior sensor
			 */
			return false;
		}

		/*
		 * If we made it here, the current sensor is not
		 * Enabled or the Zero method is not implemented.
		 *
		 * Grab the next one.
		 *
		 */
		snsr = snsr->Next;
	}

	TWB_Debug_Print("Finished All Sensors \r\n");

	/*
	 * If we made it here, we are DONE!
	 *
	 * Don't do anything, but let the caller know we are done.
	 *
	 */
	return true;
}

iOpResult_e TWB_Sensors_Start_Zero(void){
	__zero_CurrentState = ZeroState_Idle;
	SystemStatus->SnsrState = SensorAppState_Zero;

	TWB_Sensors_Zero(0.0f);
	return OK;
}


/*
 * This method is a handler that will get called upon
 * each update loop.
 *
 * It will act differently depending upon the state
 * the zero engine is in.
 *
 */
iOpResult_e TWB_Sensors_Zero(float ms) {
	__zeroMsAccum += ms;

	switch(__zero_CurrentState){
		case ZeroState_Idle:
			TWB_LEDS_SetState(LED_Snsr_Online, LED_FastBlink);

			TWB_Debug_Print("Begin Zero Sensors\r\n");
			SystemStatus->SnsrState = SensorAppState_Zeroing;

			/*
			 * When we first come in, all we do is call the zero
			 * method on the first sensor that is enabled
			 * and has a zero method.
			 *
			 * Upon completion of the calibration method
			 * we will either end up waiting for an I2C
			 * job to complete or the sensor will have
			 * already finished it's calibration and we
			 * put in place or 250ms pause between sensors.
			 *
			 * Of course if no sensors are enabled, then
			 * we just immediately end the calibration
			 *
			 * Begin_Zero returns true if zeroing sensors doesn't need to be done or is true.
			 *
			 */
			if(__twb_Sensors_BeginZero(SensorsList) == true){
				__zero_CurrentState = ZeroState_Idle;
				TWB_Debug_Print("End Zero Sensors\r\n");
				TWB_Sensors_Resume();
			}
			break;

		case ZeroState_WaitingForI2CJob: /* No Op, will change state in the I2C Job Callback */ break;
		case ZeroState_I2C_ReadCompleted:
			/*
			 * In the I2C Callback, we will increment the sample count so the time
			 * it comes here we should have the correct number of samples read.
			 */
			if(__zero_TotalSampleCount == __zero_SamplesIndex){
				/*
				 * If we have finished doing the call, grab an average of all the sensor
				 * values, and store them into a temp variable.
				 */
				u16_vector_t cals;
				cals.x = (int16_t)(__zero_TempCounts[0] / __zero_TotalSampleCount);
				cals.y = (int16_t)(__zero_TempCounts[1] / __zero_TotalSampleCount);
				cals.z = (int16_t)(__zero_TempCounts[2] / __zero_TotalSampleCount);

				/*
				 * If applicable, call a method on the sensor to notify the sensor
				 * that the calibration factors have been stored.
				 *
				 * This must complete relatively fast, it should simply update the data
				 * structure for the calibration in the sensor.
				 * calibration factors.
				 */
				if(__sensorBeingZeroed->CompleteZero != null)
					__sensorBeingZeroed->CompleteZero(&cals);

				/*
				 * Bonus feature of zeroing :) - If the EEPROM address where
				 * the calibration factors should be stored is set
				 * an async job will be queued off to write these values.
				 *
				 * Each sensor should NOT implement their own routine to do so.
				 */
				if(__zero_EEPROM_Address != null){
					I2CZeroJob->InBufferSize = 0;
					I2CZeroJob->OutBufferSize = 8;
					I2CZeroJob->Address = EEPROM_I2C_ADDR;

					I2CZeroJob->OutBuffer[0] = (uint8_t)(__zero_EEPROM_Address >> 8);
					I2CZeroJob->OutBuffer[1] = (uint8_t)(__zero_EEPROM_Address & 0xFF);

					I2CZeroJob->OutBuffer[2] = (uint8_t)(cals.x >> 8);
					I2CZeroJob->OutBuffer[3] = (uint8_t)(cals.x & 0xFF);
					I2CZeroJob->OutBuffer[4] = (uint8_t)(cals.y >> 8);
					I2CZeroJob->OutBuffer[5] = (uint8_t)(cals.y & 0xFF);
					I2CZeroJob->OutBuffer[6] = (uint8_t)(cals.z >> 8);
					I2CZeroJob->OutBuffer[7] = (uint8_t)(cals.z & 0xFF);
					I2CZeroJob->Callback = __twb_sensorWriteCalComplete;
					TWB_SE_QueueAsyncWrite(I2CZeroJob);
					__zero_CurrentState = ZeroState_I2C_StoringToE2PROM;
				}
				else{
					__zeroMsAccum = 0;
					__zero_CurrentState = ZeroState_PauseBetweenSensor;
				}
			}
			else{
				//Contains the starting MS when the reading has been read.
				__zeroMsAccum = 0;
				__zero_CurrentState = ZeroState_PauseBetweenReading;
			}

			break;
		case ZeroState_I2C_StoringToE2PROM: /* Will just sit in this state until the calibration write has finished */


			break;

		case ZeroState_PauseBetweenReading:
			/*
			 * We just let the sensor "rest" for a few milliseconds
			 * to make sure we don't grab same wacked out data for
			 * all the readings when doing a zero.
			 *
			 * A default value for a pause is passed to the sensor
			 * but ultimately it's the sensors responsibility to
			 * figure out how long of a pause should take place.
			 */
			if(__zeroMsAccum > __zero_PauseMS){
				/*
				 * If that pause has expired, queue up the next reading job
				 */
				__zero_CurrentState = ZeroState_WaitingForI2CJob;

				/* Now queue up getting the next reading */
				TWB_I2C_QueueAsyncJob(I2CZeroJob);
			}

			break;

		case ZeroState_PauseBetweenSensor:
			/*
			 * This gets called after it's determined that we have
			 * completed calibration for a sensor, but want to wait
			 * between sensors before calibrating the next one.
			 *
			 * This is primarily important when writing the calibration
			 * factors to the EEPOM since we need about 5ms between
			 * the writes to the EEPROM.
			 *
			 * If the call to begin zeroing returns a true, that means
			 * that we are all done, so reset the state to idle
			 * and resume normal sensor readings.
			 *
			 */
			if(__zeroMsAccum > ZERO_PAUSE_BETWEEN_SENSORS){
				if(__twb_Sensors_BeginZero(__sensorBeingZeroed->Next)){
					__zero_CurrentState = ZeroState_Idle;
					TWB_Sensors_Resume();
				}
			}
			break;
	}

	return OK;
}
