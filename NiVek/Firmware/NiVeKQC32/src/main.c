
#include <stdio.h>
#include <stdlib.h>

#include "../stm32f4periph/inc/misc.h"
#include "../stm32f4periph/inc/stm32f4xx_exti.h"
#include "main.h"
#include "twb_config.h"

#include "commo/wifi.h"
#include "commo/msg_parser.h"
#include "commo/message_ids.h"
#include "commo/usb/twb_usb.h"

#include "common/twb_common.h"
#include "common/twb_i2c.h"
#include "common/twb_debug.h"
#include "common/twb_leds.h"
#include "common/twb_serial_eeprom.h"
#include "common/memory_mgmt.h"
#include "common/twb_strings.h"

#include "system/cli.h"

#include "flight/pwm_capture.h"
#include "flight/pwm_source.h"
#include "flight/ctrl_loop.h"

#include "sensors/gps.h"
#include "sensors/lipo_adc.h"
#include "sensors/snsr_main.h"
#include "sensors/bmp085.h"
#include "sensors/sonar.h"
#include "Filters/Filter.h"
#include "nivek_system.h"


#define NIVEK_SETTINGS_VERSION_ADDR 0x01
#define NIVEK_SETTINGS_VERSION 11


/* For Rev II boards, here are the associated IRQ's we care about
 *
 * Pin 1 - AVAILABLE
 * Pin 2, Port B, Sonar Echo
 * Pin 3, Port C, BMP085
 * Pin 4, Port B Pitch/RC1
 * Pin 5, Port B Roll/RC2
 * Pin 6, Port B Yaw/RC3
 * Pin 7, Port B Throttle/RC4
 * Pin 8 - AVAILABLE
 * Pin 9 Port B ITG3200
 * Pin 10, Port C L3GD20 IRQ
 * Pin 11, Port C Aux1
 * Pin 12, Port C Aux2
 * Pin 13, Available
 */

void __configureEXTI(void){
	//Individual IRQ pins configured in SNSR_MAIN and PWM_CAPTURE
	EXTI_InitTypeDef extInit;
	extInit.EXTI_Line = EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line9 | EXTI_Line10 | EXTI_Line11 | EXTI_Line12 | EXTI_Line13 | EXTI_Line14;
    extInit.EXTI_Mode = EXTI_Mode_Interrupt;
    extInit.EXTI_LineCmd = ENABLE;
    extInit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_Init(&extInit);

    NVIC_InitTypeDef nvicInit;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x00; /* 0 = Highest, 3= Lowest */
    nvicInit.NVIC_IRQChannelCmd = ENABLE;

    nvicInit.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_Init(&nvicInit);

    nvicInit.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_Init(&nvicInit);

	nvicInit.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&nvicInit);

	nvicInit.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&nvicInit);

    nvicInit.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_Init(&nvicInit);
}

iOpResult_e __setDefaults(void){
	uint8_t version;
	assert_succeed(TWB_SE_ReadU8(NIVEK_SETTINGS_VERSION_ADDR, &version));
	TWB_Debug_PrintInt("NiVek Config Version: ", version);
    /* TODO: At some point we want to monitor a pad on the PCB, if that pad is grounded we would also reset the configuration */
	if(version != NIVEK_SETTINGS_VERSION){
		TWB_Debug_Print("CONFIGURATION: Old or not configured.\r\n");
		StartupPhase = 240;
		assert_succeed(TWB_IMU_RestoreDefaults());
		StartupPhase = 220;
		assert_succeed(TWB_Sys_RestoreDefaults(version));
		StartupPhase = 210;
		assert_succeed(TWB_SE_WriteU8Pause(NIVEK_SETTINGS_VERSION_ADDR, NIVEK_SETTINGS_VERSION));
	}
	else{
		TWB_Debug_Print("CONFIGURATION: OK.\r\n");
	}

	StartupPhase = 200;

	return OK;
}

#define TIM_MAIN_LOOP_PSC 1
//#define TIM_MAIN_LOOP_PERIOD 8400
#define TIM_MAIN_LOOP_PERIOD 10500

iOpResult_e __initMainLoopTimer(void){
	TIM_TimeBaseInitTypeDef  pwmTimeBase;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	pwmTimeBase.TIM_Prescaler = TIM_MAIN_LOOP_PSC;
	pwmTimeBase.TIM_CounterMode = TIM_CounterMode_Up;
	pwmTimeBase.TIM_ClockDivision = 0;
	pwmTimeBase.TIM_Period = TIM_MAIN_LOOP_PERIOD;
	pwmTimeBase.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TMR_MAIN_LOOP, &pwmTimeBase);

	TMR_MAIN_LOOP->EGR |= 0b01;  /* Enable auto reload by setting bit 1 */
	TMR_MAIN_LOOP->DIER |= 0b01; /* Enabled IRQ by setting bit 1 */

	/* Note: We don't enable the main loop here, we wait until
	 * every thing is spun up (sensors, commo, etc), then in
	 * __startMainLoop we enable the timer and interrupt to start
	 * servicing requests.
	 */

	/* 2) Enable the IRQ channel */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15; /* 0 = Highest, 15= Lowest */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	return OK;
}


/* Make sure to keep the order the same */
iOpResult_e __initModules(void){
	assert_succeed(TWB_Sys_Init());

	assert_succeed(TWB_Commo_Init());

	assert_succeed(TWB_PWM_Capture_Init());
	assert_succeed(TWB_PWM_Source_Init());

	assert_succeed(TWB_WiFi_Init());

	assert_succeed(__initMainLoopTimer());

	return OK;
}

iOpResult_e __main_sendVersionInfo(void){
	uint8_t tempStrBuffer[4];

	uint16_t version = (uint16_t)FIRMWARE_VERSION / 100.0f;
	uint16_t revision = (uint16_t)((FIRMWARE_VERSION - (float)version) * 100.0f);

	TWB_Debug_Print("Firmware Version: ");

	twb_itoa(version, tempStrBuffer, 10);
	TWB_Debug_Print((const char *)tempStrBuffer);

	twb_itoa(revision, tempStrBuffer, 10);

	TWB_Debug_Print((revision < 10) ? ".0" : ".");
	TWB_Debug_Print((const char *)tempStrBuffer);
	TWB_Debug_Print("\r\r");

	TWB_Timer_SleepMS(30);

	return OK;
}


iOpResult_e __initMain(void){
	//Enable the FPU - Woo-Fricken-Hoo
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));

#ifndef PLATFORM_ARM_NiVek
	RCC_HSEConfig( RCC_HSE_ON);

	ErrorStatus  HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus != SUCCESS)     {
		TWB_Debug_Print("Could not start HSE\r\n");
		while(1) {}
	}
#endif

	StartupPhase = 253;

	/*
	 * Before we go too far into the startup routine
	 * sleep for about 1/2 a second to make sure
	 * all the peripherals have enough time
	 * to go through their startup routine.
	 *
	 * This is especially true for the GPS
	 */
	TWB_Timer_SleepMS(1000);

	assert_succeed(TWB_Debug_Init());

	StartupPhase = 251;

	TWB_Timer_SleepMS(1000);

	TWB_Debug_Print("SYSTEM STARTING\r\n");

	/*
	 * Create our timers, should do early on so we can use them
	 * in the start sequence.
	 */
	assert_succeed(TWB_Timer_CreateTimers());

	StartupPhase = 249;


	assert_succeed(TWB_CLI_Init());

	StartupPhase = 247;
	/*
	 * Make sure our I2C bus is online
	 * BEFORE we do any additional configuration
	 * that might use the I2C bus
	 */
	assert_succeed(TWB_I2C_Init(I2C2));

	StartupPhase = 245;

	/*
	 * Setting up debug is a two step process
	 * We want it alive as soon as possible for diagnostics
	 * but we also need to pull some values form EEPROM so we need
	 * to do final initialization after we init the I2C controller to
	 * read settings.
	 */
	assert_succeed(TWB_Debug_ReadDefaults());

	StartupPhase = 243;

	TWB_Debug_Print("\r\n-------\r\n");
	TWB_Debug_Print("Welcome to NiVek QC5\r\n");
	__main_sendVersionInfo();
	TWB_Debug_Print("\r\n-------\r\n\r\n");

	/*
	 * Need to init the sensor array here
	 * sensors are really the equivalent
	 * of timed tasks in the system.
	 *
	 * Really just an attempt to bring an OO
	 * concept into low level C w/o too much
	 * overhead.
	 *
	 * Sensors might not be the exact right
	 * term but will work for now.
	 */
	TWB_InitSensorArray();

	StartupPhase = 241;

	/*
	 * If necessary go ahead and set default
	 * configuration values for each sensor
	 */
	assert_succeed(__setDefaults());

	StartupPhase = 150;



	/*
	 * Just initialize anything that isn't a sensor/module
	 * Perhaps at some point everything should be?!?!?
	 */
	assert_succeed(__initModules());
	/*
	 * Initialize the sensors/modules
	 */
	if(TWB_Sensors_Init() != OK){
		/*
		 * If we fail, it sucks but don't want
		 * to lockup the device, we want the telemetry
		 * configuration still online to see if we can
		 * recover.
		 */
		TWB_LEDS_SetState(LED_Snsr_Fault, LED_FastBlink);
		TWB_Debug_Print("Sensor initialization failed.\r\n");
		SystemStatus->SnsrState = SensorAppState_Failure;
		SystemStatus->SystemState = SystemFailure;
	}

	TWB_Timer_SleepMS(500);
	/*
	 * Just reboot the wifi or what ever is connected
	 * as the telemetry device.
	 */
	TWB_WiFi_Reboot();

	TWB_Timer_SleepMS(500);

	if(SystemStatus->SystemState != SystemFailure)
		SystemStatus->SystemState = SystemReady;

	WiFiIsConnected = WiFiDisconnected;

	__configureEXTI();

	//Enable our communication update tasks.
	TWB_Timer_Enable(SendSensorData);
	TWB_Timer_Enable(SendBatteryData);
	TWB_Timer_Enable(SendGPSData);
	TWB_Timer_Enable(SendStatusData);
	TWB_Timer_Enable(SendTargetData);
	TWB_Timer_Enable(SendMotorData);
	TWB_Timer_Enable(SendAltitudeData);

#ifndef PLATFORM_ARM_NiVek
	assert_succeed(TWB_USB_Init());
#endif
	TWB_Debug_Print("System Ready\r\n");

	return OK;
}

uint16_t tickCounter = 0;
#define ONE_SECOND_TICKS 10000


/*
 * This would be considered to be ran on the "Main-Thread" er, I guess only
 * thread since our CPU's only have one core.  Really the main loop.  With
 * very few exceptions, when the IRQ comes in it either processes one-or-two
 * bytes if it's doing something with communications, or it sets a flag
 * that is picked up in the main loop.  Very little processing happens in an
 * IRQ.  It should all be happening in this loop.
 *
 * The primary control loop items 1-4 should happen very quickly, however
 * processing the incoming message queues can take a little while since
 * they might cause some additional tasks to occur such as:
 *
 *   1)  Zero the sensors
 *   2)  Update a single sensor value (have to wait approximately 5ms after writing to EEPROM)
 *   3)  Reset defaults in EEPROM for the system
 *   4)  Restart the sensors
 *
 *   While any of these are happening, the rest of methods are not
 *   getting serviced, however IRQ's may continue to come in
 *
 *   Processing the outgoing message queue is very fast, it just queues
 *   up messages to be sent out via IRQ TXE situation.
 *
 *   None of these should be happening when the system is armed, if they
 *   do, they will be ignored since they interfere with the flight control
 *   loop.
 */
void TIM7_IRQHandler(void){
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerStart();
#endif

	/*Acknowledge the IRQ */
	TMR_MAIN_LOOP->SR = 0;
	tickCounter++;

	/*
	 * Start out by processing any messages that either need to be sent
	 * or need to be parsed.
	 */
	TWB_Commo_ProcessIncomingMessages();
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerEnd();
#endif

	TWB_Commo_ProcessOutgoingMessages();
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerStart();
#endif

	/*
	 * Scan the timers to see if anything expired or needs to be done
	 * This can be both recurring tasks AND watch dog timeouts or other
	 * general purpose tasks.
	 *
	 * Although this process works at 100 uSeconds or 0.1ms
	 * timers are limited to a resolution of 1ms.  Could potentially
	 * tighten this up a bit, but would rather not rely on that level
	 * of accuracy with our simple timers.
	 */
	TWB_Timer_Scan();
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerEnd();
#endif

	/* Each of the following performs it's tasks and then updates state
	 * The following task pickup that state and optionally do something
	 * with it.  Each task manages it's own timing and decides if it
	 * needs to do something or just return back here.  */

	// 1st - Go through the sensor array and apply updates where appropriate
	TWB_IMU_ProcessSensors(tickCounter);
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerStart();
#endif

	if(tickCounter == ONE_SECOND_TICKS)
		tickCounter = 0;

	// 2nd - Process PWM from RC and update state
	TWB_PWM_ProcessRCIn();
#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerEnd();
#endif

	// 3rd - Update the power to be sent to the motors.
	TWB_PWM_ManagePower();

#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerStart();
#endif

	TWB_CLI_ProcessCommands();

#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerEnd();
#endif


	/*
	 * This is important enough to always ensure we have the red LED lit if the motors could spin up *
	 *
	 * TODO: Need to make 100% certain that if the SensorData->ArmedStatus is safe, there is no way the motors can start...period
	 */
	TWB_LEDS_SetState(LED_Armed, SystemStatus->ArmedStatus == Safe ? LED_Off : LED_On);

#ifdef MAIN_LOOP_TIMER
	TWB_Debug_TimerStart();
#endif
}

/*
 * This is where the app truly starts, at this point
 * configuration is done and we just start executing
 * the code make good things happen, like spin up the
 * motor, etc...
 */
iOpResult_e __startMainLoop(void){
	/* 1) Start the sensors */
	if(SystemStatus->SnsrState != SensorAppState_Failure){
		/*
		 * If we fail don't freeze the MCU, we might be able
		 * to reset the sensors or some other value.
		 */
		if(TWB_Sensors_Start() != OK){
			SystemStatus->SnsrState = SensorAppState_Failure;
			SystemStatus->SystemState = SystemFailure;
		}
		else
			SystemStatus->SystemState = SystemReady;
	}
	else
		SystemStatus->SystemState = SystemFailure;
	/* 3) Start the timer */
	TIM_Cmd(TMR_MAIN_LOOP, ENABLE);

	/* 4) Return OK to idle caller */
	return OK;
}

void __twb_main_AllocateMemory(void){
	SensorConfig = pm_malloc(sizeof(SensorConfig_t));
	PIDConstantsConfig = pm_malloc(sizeof(PIDConstantsConfig_t));
	ComplementaryFilterConfig = pm_malloc(sizeof(SensorFusion_t));

	SystemStatus = pm_malloc(sizeof(SystemStatus_t));
	SensorData = pm_malloc(sizeof(SensorData_t));
	InternalState = pm_malloc(sizeof(InternalState_t));
	GPSData = pm_malloc(sizeof(GPSData_t));
	HomeGPSReading = pm_malloc(sizeof(GPSReading_t));
	CurrentGPSReading = pm_malloc(sizeof(GPSReading_t));

	PIDSensorData = pm_malloc(sizeof(PIDSensorData_t));

	MotorStatus = pm_malloc(sizeof(MotorStatus_t));
	Targets = pm_malloc(sizeof(TargetsData_t));

	PIDConstants = pm_malloc(sizeof(PIDConstants_t));
	BatteryCondition = pm_malloc(sizeof(BatteryCondition_t));
	PWMIn = pm_malloc(sizeof(PWMIn_t));

	AltitudeData = pm_malloc(sizeof(AltitudeData_t));

	_commonDiagBuffer = pm_malloc(sizeof(SensorDiagData_t));

	NiVekName = (uint8_t *)pm_malloc(NIVEK_NAME_SIZE);
}

void TWB_Status_QueueMsg(void) {
	SystemStatus->FirmwareVersion = (uint16_t)(FIRMWARE_VERSION * 100.0f);

	TWB_Commo_SendMessage(MOD_System, MSG_SYSTEM_STATUS_ID, (uint8_t *)SystemStatus, sizeof(SystemStatus_t));
}

int main(void){

	/* Enable Clock Security System(CSS): this will generate an NMI exception if HSE clock fails */
	RCC_ClockSecuritySystemCmd(ENABLE);

	//Generate an interrupt when HSE is ready.
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	TWB_LEDS_Init();
	TWB_LEDS_SetState(LED_Snsr_Fault, LED_On);
	TWB_LEDS_SetState(LED_Snsr_Fault, LED_FastBlink);

	if (SysTick_Config(SystemCoreClock / 1000)){
		while(true){
			uint32_t idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  SET);
			while (idx-- > 0);

			idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  RESET);
			while (idx-- > 0);
		}
	}

	__twb_main_AllocateMemory();

	SystemStatus->SystemState = SystemStartingUp;
	SystemStatus->Platform = 101;
	SystemStatus->FlightMode = FlightMode_Stable;
	/*
	 * Without setting this all IRQ pre-emption is done via
	 * sub priorities.  Setting this to four allows for exclusive
	 * use of PreemptionPiority for the IRQ's.
	 */


	if(__initMain() != OK){
		TWB_Debug_Print("Error initializing\r\n");
		TWB_Debug_PrintInt("Err: SYS", StartupPhase);

		while(true){
			volatile uint32_t idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  SET);
			while (idx-- > 0);

			idx = 168000 * 10;
			GPIO_WriteBit(GPIOC, GPIO_Pin_14,  RESET);
			while (idx-- > 0);
		}
	}

	TWB_LEDS_SetState(LED_Snsr_Fault, LED_Off);

	pm_printHeapSize();

	/* Everything is ready to go at this point
	 * So start the main loop
	 */
	__startMainLoop();

	/*
	 * It will return immediately
	 * For now, sit and spin
	 *
	 * Eventually this is where the NETMF stuff goes
	 *
	 * */

	TWB_Debug_Print("Entering endless main loop...");

	while(1);
}
