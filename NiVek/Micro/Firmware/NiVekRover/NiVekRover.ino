//#include <NiVekMessageParser.h>

#include <Wire.h>
#include <Servo.h>
#include "def.h"
#include <NiVekMessageScribe.h>
#include <NiVekMessageParser.h>
#include <NiVekMessage.h>
int led = 13;

#define APP_VERSION 0.16
#define VERSION  (int)(APP_VERSION * 100.0f)

#define Logger Serial
#define BlueTooth Serial1


void writeWelcomeMessage() {
	Logger.println("NiVek Rover - Starting Up ");
	Logger.print("V");
	Logger.print(APP_VERSION);
	Logger.println();
}

// the setup routine runs once when you press reset:
void setup() {
	
	// initialize the digital pin as an output.
	initComms();
	InitTurret();
	initTimings();
	
	Wire.begin();

	//initGPIO4();

	writeWelcomeMessage();
}

// the loop routine runs over and over again forever:
void loop() {
	timerHandler();
	commsHandler();

	unsigned long microSeconds = micros();
	

	//MotorsUpdate();
}

void oneHzLoop() {
	//Serial.print("X");
}

void fiveHzLoop() {
//	
	ReadSonar();
}

void tenHzLoop() {

}

void twentyHzLoop() {

}

void fiftyHzLoop() {

}

void oneHundredHzLoop(){

}

void twoHundredHzLoop(){

}

void fourHundredHzLoop(){

}