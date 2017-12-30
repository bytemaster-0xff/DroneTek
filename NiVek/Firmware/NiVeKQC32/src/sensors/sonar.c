#include "sensors/sonar.h"
#include "sensors/snsr_main.h"

#include "main.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "twb_eeprom.h"
#include "filters/Filter.h"

#include "twb_config.h"

//Sonar module:

//Trigger Pin - PC4
//Echo    Pin - PC5

#define SONAR_TRIGGER_PORT GPIOC
#define SONAR_TRIGGER_PIN GPIO_Pin_4

#define SONAR_ECHO_PORT GPIOB
#define SONAR_ECHO_PIN GPIO_Pin_2

#define SONAR_TIM_PRESCALER 1680
#define SONAR_uSEC_SCALER 10.0f

FilterFloat_t __sonarFilter;

uint32_t __sonar_LastCount;
EdgeType_t __sonar_LastEdge = EdgeTypeSame;

uint32_t __sonar_ThisCount;
uint32_t __sonar_CountDelta;

float __sonar_PWM_uSeconds;
float __sonar_Meters;

#define SONAR_TMR_COUNT_MAX 0xFFFF

TIM_TypeDef *__sonarTimer;


#define uSEC_PER_M 5700.0f;

iOpResult_e __sonar_initTimeBase(void){
	RCC_APB2PeriphClockCmd(TMR_SONAR_PERIPH , ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = SONAR_TIM_PRESCALER; //Scale to 1/100 of a milliseconds.
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = SONAR_TMR_COUNT_MAX;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TMR_SONAR, &TIM_TimeBaseStructure);
	TIM_Cmd(TMR_SONAR, ENABLE);

	__sonarTimer = TMR_SONAR;

	return OK;
}

iOpResult_e TWB_IMU_Sonar_Init(void){
	assert_succeed(TWB_SE_ReadU8(SONAR_EEPROM_OFFSET + 0, &SensorConfig->Sonar_Enabled));
	assert_succeed(TWB_SE_ReadU8(SONAR_EEPROM_OFFSET + 1, &SensorConfig->Sonar_Type));
	assert_succeed(TWB_SE_ReadU8(SONAR_EEPROM_OFFSET + 2, &SensorConfig->Sonar_FilterOption));
	assert_succeed(TWB_SE_ReadU8(SONAR_EEPROM_OFFSET + 3, &SensorConfig->Sonar_SampleRate));

	SonarSensor->IsEnabled = SensorConfig->Sonar_Enabled;
	SonarSensor->SampleRate = SensorConfig->Sonar_SampleRate;

	SonarSensor->ProcessData = &TWB_IMU_SONAR_ProcessData;

	SonarSensor->Read = &TWB_IMU_Sonar_Read;

	SonarSensor->StartZero = &TWB_IMU_Sonar_Zero;

	SonarSensor->Start = &TWB_IMU_Sonar_Start;
	SonarSensor->Stop = &TWB_IMU_Sonar_Stop;
	SonarSensor->Pause = &TWB_IMU_Sonar_Pause;
	SonarSensor->Resume = &TWB_IMU_Sonar_Resume;

	__sonarFilter.FilterType = SensorConfig->Sonar_FilterOption;

	GPIO_InitTypeDef sonarPinConfig;
	//Setup the trigger pin
	sonarPinConfig.GPIO_Pin = SONAR_TRIGGER_PIN;
	sonarPinConfig.GPIO_Mode = GPIO_Mode_OUT;
	sonarPinConfig.GPIO_Speed = GPIO_Speed_50MHz;
	sonarPinConfig.GPIO_OType = GPIO_OType_PP;
	sonarPinConfig.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(SONAR_TRIGGER_PORT, &sonarPinConfig);

	__sonar_initTimeBase();

	if(SonarSensor->IsEnabled == Enabled){
		TWB_Debug_Print("SONAR:");
		switch(SensorConfig->Sonar_Type){
			case NO_SONAR: TWB_Debug_Print("NONE_"); break;
			case HC_SR04: TWB_Debug_Print("HC_SR04_"); break;
			case MB1030_EZ3: TWB_Debug_Print("MB1030_EX3_"); break;
			default : TWB_Debug_Print("UNDEFINED_"); break;
		}
	}

	__sonar_Meters = 0.0f;

	return OK;
}

bool TWB_IMU_Sonar_Zero(uint8_t sampleSize, float pauseMS){
    //TODO: At some point zero this when on the ground.
	return true;
}

iOpResult_e TWB_IMU_Sonar_ResetDefaults(void){
	SensorConfig->Sonar_FilterOption = FilterOption_None;
	SensorConfig->Sonar_SampleRate = SampleRate_5Hz;
	SensorConfig->Sonar_Enabled = Disabled;
	SensorConfig->Sonar_Type = NO_SONAR;

	SonarSensor->IsEnabled = SensorConfig->Sonar_Enabled;
	SonarSensor->SampleRate = SensorConfig->Sonar_SampleRate;

	assert_succeed(TWB_SE_WriteU8Pause(SONAR_EEPROM_OFFSET + 0, SensorConfig->Sonar_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(SONAR_EEPROM_OFFSET + 1, SensorConfig->Sonar_Type));
	assert_succeed(TWB_SE_WriteU8Pause(SONAR_EEPROM_OFFSET + 2, SensorConfig->Sonar_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(SONAR_EEPROM_OFFSET + 3, SensorConfig->Sonar_SampleRate));

	return OK;
}

iOpResult_e TWB_IMU_Sonar_UpdateSetting(uint16_t addr, uint8_t value) {
	switch(addr){
		case 0:
			SensorConfig->Sonar_Enabled = value;
			SonarSensor->IsEnabled = value;

			if(SensorConfig->Sonar_Enabled == Enabled){
				TWB_Debug_Print("SONAR:");
				switch(SensorConfig->Sonar_Type){
					case NO_SONAR: TWB_Debug_Print("NONE_"); break;
					case HC_SR04: TWB_Debug_Print("HC_SR04_"); break;
					case MB1030_EZ3: TWB_Debug_Print("MB1030_EX3_"); break;
					default : TWB_Debug_Print("UNDEFINED_"); break;
				}
			}


			break;
		case 1:
			SensorConfig->Sonar_Type = value;
			break;
		case 2:
			SensorConfig->Sonar_FilterOption = value;
			__sonarFilter.FilterType = value;

			break;
		case 3:
			SensorConfig->Sonar_SampleRate = value;
			SonarSensor->SampleRate = SensorConfig->Sonar_SampleRate;
			break;
	}

	return OK;
}

iOpResult_e TWB_IMU_Sonar_Start(void){
	__sonarFilter.IsInitialized = false;
	__sonarFilter.CurrentSlot = 0;
	return OK;
}

iOpResult_e TWB_IMU_Sonar_Stop(void){

	return OK;
}

iOpResult_e TWB_IMU_Sonar_Pause(void){
	return OK;
}

iOpResult_e TWB_IMU_Sonar_Resume(void){
	__sonarFilter.IsInitialized = false;
	__sonarFilter.CurrentSlot = 0;
	return OK;
}

/*
 * Step 1 - toggle the trigger pin for 10 uSeconds (if on HC_SR04, MB1030 is free running)
 */
iOpResult_e TWB_IMU_Sonar_Read(float deltaT) {
	switch(SensorConfig->Sonar_Type){
		case HC_SR04: /* HC_SR04 requires 10 uSec trigger */
			GPIO_WriteBit(SONAR_TRIGGER_PORT, SONAR_TRIGGER_PIN, SET);

			//TODO: Spinning in here...wasting 10 valuable uSeconds :(
			//TODO: Need to measure clock
			volatile uint16_t idx = 500 * 7 / 2; //Gives us approximately 10 uSec pulse
			while(idx-- > 0);

			GPIO_WriteBit(SONAR_TRIGGER_PORT, SONAR_TRIGGER_PIN, RESET);
			break;
		default: /* NOP */; break;
	}

	return OK;
}

/*
 * Step 2 - Detect the Rise and Fall edges to determine the pulse width.
 */
void EXTI2_IRQHandler(void){
	EXTI_ClearITPendingBit(EXTI_Line2);

	__sonar_ThisCount = TMR_SONAR->CNT;

	//On rising edge...get start count
	if((GPIOB->IDR & GPIO_Pin_2) == GPIO_Pin_2){
		__sonar_LastCount = __sonar_ThisCount;
	}
	else{
		//On Falling Edge, get delta.
		if(__sonar_ThisCount < __sonar_LastCount)
			__sonar_ThisCount += SONAR_TMR_COUNT_MAX;

		//Calculate delta in counts or 1/10 of a MS;
		__sonar_CountDelta =  (__sonar_ThisCount - __sonar_LastCount);

		SonarSensor->DataReady = DataReady;
	}
}


/*
 * Step 3 - Turn the pulse width into Meters
 */
iOpResult_e TWB_IMU_SONAR_ProcessData(void){
	__sonar_PWM_uSeconds = __sonar_CountDelta * SONAR_uSEC_SCALER;

	__sonar_Meters = __sonar_PWM_uSeconds / uSEC_PER_M;

	AltitudeData->Sonar = (int16_t) (__sonar_Meters * 100.0f);

	SonarSensor->Z = __sonar_Meters;

	return OK;
}

/* When appropriate send out the data via telemetry */
void TWB_IMU_SONAR_SendDiag(void){
	_commonDiagBuffer->SensorId = SonarSensor->SensorId;

	_commonDiagBuffer->RawX = 0;
	_commonDiagBuffer->RawY = 0;
	_commonDiagBuffer->RawZ = 0;

	_commonDiagBuffer->X = SonarSensor->X;
	_commonDiagBuffer->Y = 0;
	_commonDiagBuffer->Z = 0;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
