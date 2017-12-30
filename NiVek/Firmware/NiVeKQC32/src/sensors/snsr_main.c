
#include "sensors/snsr_main.h"
#include "twb_config.h"
#include "twb_eeprom.h"

#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "common/twb_i2c.h"
#include "common/twb_common.h"
#include "common/twb_timer.h"
#include "common/memory_mgmt.h"
#include "flight/ctrl_loop.h"

#include "main.h"
#include "common/twb_leds.h"

#include "stdlib.h"
#include "string.h"

//The main sensors will always return 6 bytes from
//their registers.  Allocate it here for use in all sensors
uint8_t __commonSensorBuffer[30]; // Declare it for 5x6 to allow importing FIFO buffer

uint32_t ticks;
float deltaT;
uint32_t ms;
uint32_t thisCountHere;
uint32_t lastSensorCount;
uint8_t sensorIndex;

#define TMR_SNSR_DIVISOR 1680
#define TMR_SNSR_ROLLOVER 0xFFFF

Sensor_t *__sensorInFocus = NULL;

extern I2CJob_t *I2CZeroJob;

TIM_TimeBaseInitTypeDef  tbid;

TIM_TypeDef *tickTimer;

/*
 * This timer is used to get a highly accurate count that
 * will be passed to the sensor that will be the deltaT
 * between samples
 */
iOpResult_e __snsr_Configure_Timer(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9 , ENABLE);

	tbid.TIM_Prescaler = TMR_SNSR_DIVISOR;
	tbid.TIM_CounterMode = TIM_CounterMode_Up;
	tbid.TIM_Period = TMR_SNSR_ROLLOVER;
	tbid.TIM_ClockDivision = 0;
	tbid.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TMR_SENSOR, &tbid);

	tickTimer = TMR_SENSOR;

	TIM_Cmd(TMR_SENSOR, ENABLE);

	return OK;
}


/*
 * A number of our sensors are configured to signal
 * an interrupt when their data is ready.  The following
 * chunk of code is used ot configure those pins and IRQ
 * handlers.
 */
iOpResult_e __snsr_Configure_Incoming_IRQs(void){
	GPIO_InitTypeDef irqConfig;
	irqConfig.GPIO_Mode = GPIO_Mode_IN;
	irqConfig.GPIO_PuPd = GPIO_PuPd_NOPULL;

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/*
	 * You may disagree about these "magic-numbers" but hey
	 * it's my code :).  Besides, I had constants defined
	 * and it's just as easy to define them here, keeps
	 * the file size smaller and you can immediately
	 * see what's hooked up to what
	 */
	/* PC10- L3GD20 Gyro */
	irqConfig.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOC, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource10);

	/* PA8- ITG3200/MPU6050 Gyro/Acc */
	irqConfig.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);

	/* PC3- BMP085-EOC */
	irqConfig.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource3);

	/* PB2- Sonar Echo/HC-SR04 */
	irqConfig.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOB, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);

	/* PB14- ADXL 345 */
	irqConfig.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);

	/* PB13- LSM303 Acc/Mag */
	irqConfig.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &irqConfig);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource13);

	return OK;
}

void __initSensorData(void){
	SensorData->Pitch = 0;
	SensorData->Roll = 0;
	SensorData->Heading = 0;

	MotorStatus->ESC_Front = 0;
	MotorStatus->ESC_Right = 0;
	MotorStatus->ESC_Left= 0;
	MotorStatus->ESC_Rear = 0;

	Targets->TargetHeading = 0;
	Targets->TargetPitch = 0;
	Targets->TargetRoll = 0;
	Targets->Throttle = 0;
	Targets->Yaw = 0;

	InternalState->Pitch = 0.0f;
	InternalState->PitchRateDPS = 0.0f;
	InternalState->Roll = 0.0f;
	InternalState->RollRateDPS = 0.0f;
	InternalState->Heading = 0.0f;
	InternalState->YawRateDPS = 0.0f;
	InternalState->AltitudeM = 0.0f;
	InternalState->AltDelta = 0.0f;

	InternalState->TargetPitch = 0.0f;
	InternalState->TargetRoll = 0.0f;
	InternalState->TargetHeading = 0.0f;
	InternalState->TargetAltitude = 0.0f;

	InternalState->ESC_Front = 0.0f;
	InternalState->ESC_Port = 0.0f;
	InternalState->ESC_Starboard = 0.0f;
	InternalState->ESC_Rear = 0.0f;
}

uint16_t _msThreshold;

/*
 * Primary routine used to startup/initialize the sensors in the system.
 */
iOpResult_e TWB_Sensors_Init(void) {
	assert_succeed(TWB_SE_ReadU8(SENSOR_CFG_EEPROM_OFFSET, &SensorConfig->ControlLoopUpdateRateHz));

	/* Initialize all the state variables for the sensors */
	__initSensorData();

	/* Iterate through each sensor and let it perform initialization */
	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		snsr->SensorFault = snsr->Init();
		snsr->DataReady = DataNotReady;

		if(snsr->SensorFault == OK) /* if w didn't fail, mark the sensor as ready, don't bail, other sensors could still be initialized */
			snsr->Status = SensorStatus_Ready;
		else{
			snsr->Status = SensorStatus_Failure;
			TWB_Debug_Print("\r\n!!!!Failed to initialized: ");
			TWB_Debug_Print(snsr->Name);
			TWB_Debug_Print("!!!!\r\n");

			return FAIL;
		}

		snsr = snsr->Next;
	}

	/* Configure any IRQ lines coming in from the sensors for data complete */
	assert_succeed(__snsr_Configure_Incoming_IRQs());

	/*
	 * Configure our timing source to be used to provide accurate deltaT between
	 * readings for our sensors.
	 */
	assert_succeed(__snsr_Configure_Timer());

	/* Fast flash to state sensors are ready, but not online/sampling */
	TWB_LEDS_SetState(LED_Snsr_Online, LED_FastBlink);

	/* Set the sensor state to be ready */
	SystemStatus->SnsrState = SensorAppState_Ready;

	I2CZeroJob = pm_malloc(sizeof(I2CJob_t));
	/*
	 * Not 100% happy without "loose" we are defining these buffer
	 * but should never be more than 12 bytes.
	 */
	I2CZeroJob->OutBuffer = pm_malloc(12); /*Need a min. eight bytes to use this structure to write to the EEPROM */
	I2CZeroJob->InBuffer = pm_malloc(12);  /*Need a min. six bytes to read the value from the sensor */

	TWB_Debug_Print("\r\nSensors:     OK\r\n");

	return OK;
}

/*
 * This method will be called every 1/10mS or 100uSeconds
 *
 * This method will also pass in a running total of ticks that roll over at
 * one second or at 10000.
 *
 * This method only does one job since it's executing in an interrupt
 * handler.
 *
 */
iOpResult_e TWB_IMU_ProcessSensors(uint16_t ticks){
//	TWB_Debug_TimerStart();
	if(SystemStatus->SnsrState == SensorAppState_Zeroing){
		return TWB_Sensors_Zero(0.1f);
	}

	Sensor_t *snsr = SensorsList;
	/*
	 * First look for any sensors that need their data processed, this is generally
	 * triggered by an I2C job completing (which could kick off another job).
	 */
	while(snsr != NULL){
		if(snsr->IsEnabled ){

			/* This can be set by an IRQ, is so we want to process, in this loop */
			if(snsr->DataReady == DataReady){
				/*
				 * Since this is relatively expensive, only do the FP divide when we need to process the data.
				 * Ticks will have a resolution of 1/10 a ms, so dividing by 10000 will give us deltaT in seconds.
				 * This should be relatively accurate however for integration we will use the internal sample
				 * rate of the sensor and just make sure we don't miss any values.  At 200Hz w/o a FiFo we have a
				 * range of 5ms to pull the data, that's should be the equivelant of 50 ticks.  Hopefully
				 * when it really matters, this loop will always be considerably smaller than that.
				 *
				 */
				snsr->SensorFault = snsr->ProcessData();
				snsr->DataReady = DataNotReady;

				/*
				 * If we are processing any data, just don't do any work this cycle, let it
				 * finish what it's doing, the next time it will pickup any work on any additional
				 * sensors.  This will give a 250 uSecond pause before kicking off any jobs.
				 */
				return OK;
			}
		}

		snsr = snsr->Next;
	}


	/*
	 * If we made it here, we know we don't have any pending data processing jobs so
	 * see if any sensors need additional processing.
	 */
	snsr = SensorsList;
	while(snsr != NULL){
		if(snsr->IsEnabled){
			if(snsr->IRQAsserted == IRQAsserted){
				snsr->SensorFault = snsr->HandleIRQ();
				snsr->IRQAsserted = IRQNotAsserted;
			}

			/* Now grab latest timings and process the data */
			thisCountHere = TMR_SENSOR->CNT; //Don't wanna optimize this out since TIM16->CNT could increment while in the loop.

			lastSensorCount = snsr->LastUpdate;

			/* The prescaler on the timer used for the sensor has ticks as 10 uSeconds in 1/100 of a millisecond
			 * Max number of milliseconds where this is valid is: TIM_16_ROLLOVER / 100 or 60000 / 100 or 600ms,
			 *
			 * With 1Hz update rate it won't be accurate but with that much time between updates, the exact
			 * timing probably isn't as critical
			 *
			 * This raw value from the counter is stored in ticks
			 *
			 * MS is used to decide when to call the process/read method on the sensor based upon
			 * configured in that sensors sample rate.
			 *
			 * Finally we convert ticks to actual actual MS.  Since we are using floating point numbers...well whoo-hoo!
			 *
			 */
			ticks = snsr->LastUpdate > thisCountHere ? thisCountHere + TMR_SNSR_ROLLOVER - snsr->LastUpdate : thisCountHere - snsr->LastUpdate;
			ms = ticks / 100;

			/*
			 * When the sensors first come on line, get a reading, pass a negative value
			 * into the read method to ensure the read method know's it's the initial read
			 * so it can act accordingly.
			 *
			 * These should be very fast in that no processing should occur, so we can always attempt to do these
			 *
			 *
			 */
			if(snsr->Status == SensorStatus_Ready){
				snsr->SensorFault = snsr->Read(-1.0f);
				snsr->LastUpdate = thisCountHere;
				snsr->Status = SensorStatus_Online;
			}
			else{
				_msThreshold = TWB_Timer_GetMSFromSampleRate(snsr->SampleRate);

				if(snsr->SampleRate != SampleRate_IRQ && _msThreshold <= ms){
					deltaT = (float)ticks / 100.0f;  /* See note above for timings */
					snsr->LastUpdate = thisCountHere;

					//snsr->SensorFault = snsr->Read(_msThreshold * 0.001f);
					snsr->SensorFault = snsr->Read(deltaT / 1000.0f);
					return OK;
				}
			}
		}

		snsr = snsr->Next;
	}

//	TWB_Debug_TimerEnd();

	return OK;
}




