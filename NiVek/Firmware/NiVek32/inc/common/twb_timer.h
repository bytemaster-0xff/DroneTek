#ifndef TWB_TIMEOUTS_H_
#define TWB_TIMEOUTS_H_

#include "common/twb_common.h"

typedef struct {
	char *Name;
	bool AutoReset;
	bool IsEnabled;
	bool IsExpired;
	uint16_t TimeOutMS;
	uint16_t CurrentMS;
	void (*ExpiredHandler)(void);

	void *Next;
} Timer_t;

iOpResult_e TWB_Timer_CreateTimers(void);
uint16_t TWB_Timer_GetMSFromSampleRate(SampleRate_t sampleRate);

void TWB_Timer_Reset(Timer_t *watchDog);
void TWB_Timer_Enable(Timer_t *watchDog);
void TWB_Timer_Disable(Timer_t *watchDog);

void TWB_Timer_Scan(void);

void TWB_Watchdog_Init(void);
void TWB_Watchdog_DecrementMS(void);
void TWB_Timer_SleepMS(uint32_t millisecs);
uint16_t TWB_Timeouts_GetMS(void);

extern Timer_t *WiFiInWatchDog;
extern Timer_t *WiFiActionTimer;
extern Timer_t *WiFiHeartBeatTimer;
extern Timer_t *RCWatchDog;

extern Timer_t *SendSensorData;
extern Timer_t *SendBatteryData;
extern Timer_t *SendGPSData;
extern Timer_t *SendPWMInData;
extern Timer_t *SendMotorData;
extern Timer_t *SendTargetData;
extern Timer_t *SendAltitudeData;
extern Timer_t *SendStatusData;
extern Timer_t *I2CWatchDog;
extern Timer_t *WiFiCmds;



#endif /* TWB_TIMOUTS_H_ */
