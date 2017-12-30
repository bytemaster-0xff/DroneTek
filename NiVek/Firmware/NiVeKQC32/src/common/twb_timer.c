
#include <string.h>
#include <stdlib.h>

#include "twb_eeprom.h"

#include "commo/wifi.h"
#include "commo/commo.h"
#include "sensors/snsr_main.h"
#include "sensors/gps.h"
#include "sensors/lipo_adc.h"

#include "flight/autopilot.h"
#include "flight/pwm_capture.h"
#include "flight/pwm_source.h"
#include "flight/ctrl_loop.h"
#include "flight/autopilot.h"

#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "common/twb_timer.h"
#include "common/twb_leds.h"
#include "common/memory_mgmt.h"
#include "common/twb_i2c.h"



#include "main.h"
#include "stdio.h"

#define I2C_MS_TIMEOUT 2
#define LED_FAST_BLINK_MS 100
#define LED_SLOW_BLINK_MS 500

#define RC_TIMEOUT_MS 2000 //If nothing is coming from the radio for more than 2 seconds, we probably have a problem

#define RCV_TIMEOUT_MS 5000 //If we don't get a ping from WiFi in 3 seconds, timeout and reset the connection
#define SND_TIMEOUT_MS 1000 //If we don't send something every one second, send a ping.
#define I2C_TIMEOUT_MS 2 //Very fast 2MS Watchdog.

Timer_t *_watchDogHead = NULL;

/* ============================ */
/* Create Timers and Watch Dogs */
Timer_t *WiFiInWatchDog;
Timer_t *I2CWatchDog;
Timer_t *RCWatchDog;
Timer_t *RCWatchDog;
Timer_t *WiFiHeartBeatTimer;

Timer_t *SendSensorData;
Timer_t *SendMotorData;
Timer_t *SendTargetData;
Timer_t *SendStatusData;
Timer_t *SendBatteryData;
Timer_t *SendAltitudeData;
Timer_t *SendGPSData;
Timer_t *SendPWMInData;

Timer_t *WiFiActionTimer;
Timer_t *WiFiCmds;

#define AutoResets true
#define SingleShot false

#define AutoEnable true
#define ManualEnable false

/* Done Timers and Watch Dogs   */
/* ============================ */

Timer_t *__createTimer(char *name, uint16_t initialTimeoutMS, void (*handler)(void), bool autoReset, bool isEnabled ){
	Timer_t *watchDog = (Timer_t *)pm_malloc(sizeof(Timer_t));

	watchDog->Name = (char *)pm_malloc((size_t)strlen(name) + 1);
	strcpy(watchDog->Name, name);
	watchDog->IsEnabled = isEnabled;
	watchDog->TimeOutMS = initialTimeoutMS;
	watchDog->ExpiredHandler = handler;
	watchDog->AutoReset = autoReset;

	watchDog->Next = NULL;

	if(_watchDogHead == NULL)
		_watchDogHead = watchDog;
	else{
		Timer_t *ptr = _watchDogHead;
		while(ptr->Next != NULL) ptr = ptr->Next;
		ptr->Next = watchDog;
	}

	return watchDog;
}

uint16_t TWB_Timer_GetMSFromSampleRate(SampleRate_e sampleRate){
	switch(sampleRate){
		case SampleRate_1KHz: return 1;
		case SampleRate_500Hz: return 2;
		case SampleRate_200Hz: return 5;
		case SampleRate_150Hz: return 6;
		case SampleRate_100Hz: return 10;
		case SampleRate_50Hz: return 20;
		case SampleRate_25Hz: return 40;
		case SampleRate_20Hz: return 50;
		case SampleRate_10Hz: return 100;
		case SampleRate_5Hz: return 200;
		case SampleRate_1Hz: return 1000;
		case SampleRate_IRQ: return 0;
	}

	return 0;
}

iOpResult_e TWB_Timer_CreateTimers(void){
	/*
	 * Initially we just put defaults in for the timeout for the timers, this let's us
	 * bring the timeout sequence online very early on in the process to help
	 * with startup.  Then we read in the values from EEPROM and configure the
	 * timers with custom timeouts
	 *
	 * (not implemented as of 9/1/2013, no mechanism to read in timers))
	 */
	SensorConfig->SensorSendDataRate = SampleRate_50Hz;
	SensorConfig->GPSSendDataRate = SampleRate_1Hz;
	SensorConfig->BatterySendDataRate = SampleRate_1Hz;
	SensorConfig->StatusSendDataRate = SampleRate_5Hz;
	SensorConfig->MotorsSendDataRate = SampleRate_10Hz;
	SensorConfig->TargetsSendDataRate = SampleRate_5Hz;
	SensorConfig->AltitudeSendDataRate = SampleRate_10Hz;

	__createTimer("LEDFAST", LED_FAST_BLINK_MS, &TWB_LEDS_UpdateFastBlink, AutoResets, AutoEnable);
	__createTimer("LEDSLOW", LED_SLOW_BLINK_MS, &TWB_LEDS_UpdateSlowBlink, AutoResets, AutoEnable);

	RCWatchDog =  __createTimer("RCWDTMR", RC_TIMEOUT_MS, &TWB_PWM_RCWatchDogExpire, SingleShot, ManualEnable);
	WiFiInWatchDog = __createTimer("RCVTMR", RCV_TIMEOUT_MS, &TWB_Main_WIFiReceiveExpired, SingleShot, ManualEnable);
	WiFiHeartBeatTimer = __createTimer("SNDTMR", SND_TIMEOUT_MS, &TWB_Main_WiFiSendExpired, AutoResets, ManualEnable);
	I2CWatchDog = __createTimer("I2CWDG", I2C_TIMEOUT_MS, &TWB_I2C_WatchdogExpired, SingleShot, ManualEnable);

	SendSensorData = __createTimer("SNDSNSR", TWB_Timer_GetMSFromSampleRate(SensorConfig->SensorSendDataRate), &TWB_Sensors_QueueMsg, AutoResets, ManualEnable);
	SendBatteryData = __createTimer("SNDBATR", TWB_Timer_GetMSFromSampleRate(SensorConfig->BatterySendDataRate), &TWB_ADC_QueueMsg, AutoResets, ManualEnable);
	SendStatusData = __createTimer("SNDSTAT", TWB_Timer_GetMSFromSampleRate(SensorConfig->StatusSendDataRate), &TWB_Status_QueueMsg, AutoResets, ManualEnable);
	SendGPSData = __createTimer("SNDGPS", TWB_Timer_GetMSFromSampleRate(SensorConfig->GPSSendDataRate), &TWB_GPS_QueueMsg, AutoResets, ManualEnable);
	SendPWMInData = __createTimer("SNDPWMIn", SEND_PWM_UPDATE_RATE_MS, &TWB_RCIN_QueueMsg, AutoResets, ManualEnable);
	SendAltitudeData = __createTimer("SNDALT", SensorConfig->AltitudeSendDataRate, &TWB_AutoPilot_QueueAltitudeMsg, AutoResets, ManualEnable);
	WiFiActionTimer = __createTimer("WIFACT", 2, &TWB_WiFi_Timer_Tick, SingleShot, ManualEnable);

	SendTargetData = __createTimer("SNDTGTDATE", TWB_Timer_GetMSFromSampleRate(SensorConfig->TargetsSendDataRate), &TWB_Targets_QueueMsg, AutoResets, ManualEnable);
	SendMotorData = __createTimer("SNDMGRDAT", TWB_Timer_GetMSFromSampleRate(SensorConfig->MotorsSendDataRate), &TWB_Motors_QueueMsg, AutoResets, ManualEnable);
	WiFiCmds =  __createTimer("WIFICMDS", TWB_Timer_GetMSFromSampleRate(SensorConfig->MotorsSendDataRate), &TWB_Motors_QueueMsg, SingleShot, ManualEnable);

	return OK;
}

/*
 * Initially we just put defaults in for the timeout for the timers, this let's us
 * bring the timeout sequence online very early on in the process to help
 * with startup.  Then we read in the values from EEPROM and configure the
 * timers with custom timeouts
 *
 * (not implemented as of 9/1/2013, no mechanism to read in timers))
 */
iOpResult_e TWB_Timer_ReadTimeouts(){
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 0, &SensorConfig->SensorSendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 1, &SensorConfig->GPSSendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 2, &SensorConfig->BatterySendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 3, &SensorConfig->StatusSendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 4, &SensorConfig->TargetsSendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 5, &SensorConfig->MotorsSendDataRate));
	assert_succeed(TWB_SE_ReadU8(DATA_TX_INTVL_EEPROM_OFFSET + 6, &SensorConfig->AltitudeSendDataRate));

	return OK;
}

void TWB_Timer_Enable(Timer_t *watchDog) {
	watchDog->IsEnabled = true;
	watchDog->IsExpired = false;

	if(!watchDog->IsEnabled)
		watchDog->CurrentMS = watchDog->TimeOutMS;
}

void TWB_Timer_Disable(Timer_t *watchDog) {
	watchDog->IsEnabled = false;
}

void TWB_Timer_Reset(Timer_t *watchDog){
	watchDog->IsEnabled = true;
	watchDog->CurrentMS = watchDog->TimeOutMS;
	watchDog->IsExpired = false;
}

void TWB_Timer_Scan(void){
	Timer_t *ptr = _watchDogHead;
	while(ptr != NULL){
		if(ptr->IsEnabled &&        /* If we are enabled */
		  !ptr->IsExpired &&        /* And haven't already expired */
		   ptr->CurrentMS == 0){    /* And current MS just went to zero */
			ptr->ExpiredHandler();  /* Execute the handler */

			if(ptr->AutoReset)
				TWB_Timer_Reset(ptr);  /* If this is a timer, reset so it will fire again. */
			else
				ptr->IsExpired = true;    /* Otherwise mark as expired */
		}

		ptr = ptr->Next;
	}
}

volatile uint16_t __sleepTimer;

/*Will occur every one millisecond */
void SysTick_Handler(void){
	Timer_t *ptr = _watchDogHead;
	while(ptr != NULL){
		if(ptr->IsEnabled && ptr->CurrentMS > 0)
			ptr->CurrentMS--;

		ptr = ptr->Next;
	}

	if(__sleepTimer > 0)
		__sleepTimer--;
}

void TWB_Timer_SleepMS(uint32_t sleepMS){
	__sleepTimer = sleepMS;
	while(__sleepTimer > 0);
}

uint16_t TWB_Timer_GetMS(void){
	return __sleepTimer;
}
