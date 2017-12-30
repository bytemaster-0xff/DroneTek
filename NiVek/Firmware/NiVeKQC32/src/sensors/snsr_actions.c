/*
 * snsr_actions.c
 *
 *  Created on: Jan 9, 2013
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
#include "nivek_system.h"

iOpResult_e TWB_IMU_RestoreDefaults(void){
	SystemStatus->SystemState = SystemResettingDefaults;

	TWB_Debug_Print("Resetting Defaults\r\n");
	StartupPhase = 238;

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		iOpResult_e result = snsr->ResetDefaults();
		if(result != OK){
			TWB_Debug_Print("Error initializing Sensor: ");
			TWB_Debug_Print(snsr->Name);
			TWB_Debug_Print("\r\n");
			TWB_Debug_PrintInt("Err Code: ", result);
			TWB_Debug_Print("\r\n");

			return result;
		}

		snsr = snsr->Next;
	}

	StartupPhase = 233;

	SensorConfig->SensorSendDataRate = SampleRate_50Hz;
	SensorConfig->GPSSendDataRate = SampleRate_1Hz;
	SensorConfig->BatterySendDataRate = SampleRate_1Hz;
	SensorConfig->StatusSendDataRate = SampleRate_5Hz;
	SensorConfig->MotorsSendDataRate = SampleRate_10Hz;
	SensorConfig->TargetsSendDataRate = SampleRate_5Hz;

	assert_succeed(TWB_SE_WriteU8Pause(SENSOR_CFG_EEPROM_OFFSET + 0, SensorConfig->ControlLoopUpdateRateHz ));

	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 0, SensorConfig->SensorSendDataRate));
	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 1, SensorConfig->GPSSendDataRate));
	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 2, SensorConfig->BatterySendDataRate));
	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 3, SensorConfig->StatusSendDataRate));
	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 4, SensorConfig->TargetsSendDataRate));
	assert_succeed(TWB_SE_WriteU8Pause(DATA_TX_INTVL_EEPROM_OFFSET + 5, SensorConfig->MotorsSendDataRate));

	TWB_Debug_Print("Done resetting Defaults\r\n");

	SystemStatus->SystemState = SystemReady;
	return OK;
}

SensorStates_e _sensorStateBeforePause;

/*
 * Start will do what ever is necessary to start
 * the sensor such as turn on IRQ/FiFo filling
 * it will also initialize any state variables.
 *
 * This is different than initialization where
 * all the stored config is read and common
 * settings on the sensor are updated.
 *
 */
iOpResult_e TWB_Sensors_Start(void) {
	TWB_LEDS_SetState(LED_Snsr_Online, LED_FastBlink);

	SystemStatus->SnsrState = SensorAppState_Starting;

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		if(snsr->Start != NULL)
			assert_succeed(snsr->Start());

		snsr = snsr->Next;
	}

	SystemStatus->SnsrState = SensorAppState_Online;
	TWB_LEDS_SetState(LED_Snsr_Online, LED_On);

	TWB_Debug_Print("Sensors starting\r\n");

	return OK;
}

/*
 * Pause will turn off any IRQ's and/or FIFO filling
 * in the sensor.
 */
iOpResult_e TWB_Sensors_Pause(void) {
	_sensorStateBeforePause = SystemStatus->SnsrState;

	SystemStatus->SnsrState = SensorAppState_Pausing;

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		if(snsr->Pause != NULL)
			assert_succeed(snsr->Pause());

		snsr = snsr->Next;
	}

	SystemStatus->SnsrState = SensorAppState_Paused;
	TWB_LEDS_SetState(LED_Snsr_Online, LED_FastBlink);

	TWB_Debug_Print("Sensors paused\r\n");

	return OK;
}

/* resume will turn back on any IRQ's and/or FIFO filling
 * in the sensor, could also reset state variables
 * and filters.
 */
iOpResult_e TWB_Sensors_Resume(void) {
	TWB_LEDS_SetState(LED_Snsr_Online, LED_FastBlink);

	/*
	 * If sensor state is paused coming, when attempting
	 * to resume, then we know the state didn't changed
	 * while paused and we can move to a resuming state.
	 */
	if(SystemStatus->SnsrState == SensorAppState_Paused || SystemStatus->SnsrState == SensorAppState_Zeroing)
		SystemStatus->SnsrState = SensorAppState_Resuming;

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		if(snsr->Pause != NULL)
			assert_succeed(snsr->Resume());

		snsr = snsr->Next;
	}

	TWB_LEDS_SetState(LED_Snsr_Online, LED_On);

	/*
	 * If the state is resuming, once we come back out
	 * of the resuming state, set it to what it was prior
	 * to being paused.
	 */
	if(SystemStatus->SnsrState == SensorAppState_Resuming)
		SystemStatus->SnsrState = _sensorStateBeforePause;

	TWB_Debug_Print("Sensors resumed\r\n");

	return OK;
}

/* Shuts down the sensor array.
 * Not really sure wehre this will be used, but
 * want to provide at least the capability to do so.
 */
iOpResult_e TWB_Sensors_Stop(void) {
	SystemStatus->SnsrState = SensorAppState_Stopping;

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		if(snsr->Pause != NULL)
			assert_succeed(snsr->Stop());

		snsr = snsr->Next;
	}

	TWB_LEDS_SetState(LED_Snsr_Online, LED_Off);

	SystemStatus->SnsrState = SensorAppState_Offline;

	TWB_Debug_Print("Sensors Stopped\r\n");

	return OK;
}


