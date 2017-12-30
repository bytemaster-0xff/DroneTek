

#include <MemoryFree.h>
#include <ZigduinoRadioCfg.h>
#include <transceiver.h>
#include <radio.h>
#include <const.h>
#include <board_cfg.h>
#include <board.h>
#include <atmega_rfa1.h>
#include <NiVekMessage.h>
#include <NiVekMessageParser.h>
#include <NiVekMessageScribe.h>

#include "AppData.h"
#include "config.h"
#include "def.h"
#include "Enums.h"

//#include <NiVekRadio.h>
#include <PicopterRadio.h>

#define APP_VERSION 0.14
#define VERSION  (int)(APP_VERSION * 100.0f)

void setup() {
	Serial.println("");

	IsArmed = Safe;

	/* First - Startup comms to serial port and radio.*/
	initComms();

	writeWelcomeMessage();
	
	/* Configure any hardware registers etc...*/
	initHardware();

	/* Read/Init EEPROM Sys Config */
	initSys();

	/* Initialized IMU (Flight Control Algorithm */
	initIMU();
	
	/* Initialize state and hardware for motors*/
	initMotors();	

	initLEDS();

	/* Initialize/start up any related sensors */
	if (!initSensors())
		fastBlink();

	Serial.println();

	/* Start our Timer */
	initTimings();


	Serial.print("freeMemory()=");
	Serial.println(freeMemory());

	Serial.println("Startup Completed");
	Serial.println("===================");
}

void fourHundredHzLoop(){

}

/* Primary Flight Control Happens Here*/
void twoHundredHzLoop(){	
	/* 1 -> Get Sensor Feedback */
	sensorHandler();

	/* 2 -> Turn raw sensor readings into attitude */
	imuHandler();

	/* 3 -> Apply sensor feedback */
	pidHandler();

	/* 4 -> Update Motors */
	motorsHandler();
}

void oneHundredHzLoop(){

}


void fiftyHzLoop(){
	/* Handle inputs from radio */
	radioHandler();
}

void twentyHzLoop() {
	/* Send Attitude data */
	sendBasicTelemetry();
}

void tenHzLoop() {
	

	//sendMotorStatus();
	//sendRCChannel();
}

void fiveHzLoop() {		
	
	batteryCheck();
}

void oneHzLoop(){
	sendStatus();
}

void loop() {
	/* Check for comms, both from serial and radio */
	commsHandler();

	/* Check for "bad-stuff" */
	watchdog();

	/* Break out arm/safe check to ensure motors won't startup accidentally*/
	checkArmedStateChange();

	/* Timer Handler will call the specific loop handlers */
	timerHandler();
}