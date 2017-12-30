// Main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "arduino.h"
#include "Comms.h"
#include "Drive.h"
#include "Timer.h"
#include "Turret.h"
#include <process.h>
#include "Compass.h"

#define USE_NETWORKSERIAL

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

void StartCommoListener(void *nop)
{
	while (true)
	{
		commsHandler();
		delay(1);
	}
}

// Helper function for logging to debug output and the console
void CustomLogging(char* str)
{
	OutputDebugStringA(str); // for VS Output
	printf(str); // for commandline output
}

int _tmain(int argc, _TCHAR* argv[])
{
	return RunArduinoSketch();
}

void setup()
{
	// TODO: Add your code here
	CustomLogging("NiVek Rover 1 - Online !\n");
	// initialize the digital pin as an output.
	pinMode(led, OUTPUT);

	initComms();
	InitTurret();

	initTimings();

	InitCompass();

	_beginthread(StartCommoListener, 1024, NULL);
}

void fourHundredHzLoop(){

}

/* Primary Flight Control Happens Here*/
void twoHundredHzLoop(){
	
}

void oneHundredHzLoop(){

}


void fiftyHzLoop(){
	
}

void twentyHzLoop() {	
	ReadTurretSensors();
	ReadWheelEncoders();
}

void tenHzLoop() {
	ReadCompass();
	
}

void fiveHzLoop() {
///	sprintf_s(OutputBuffer, OUTPUT_BUFFER_SIZE, "Heading: %d\tPan: %d\tTilt %d\tSonar: %d\tRear Sonar:%d\tIR: Left: %d\tRight: %d\t %d\t%d\t%d\t%d\r\n", (int) CurrentHeading, _currentPan, _currentTilt, _sonar, _rearSonar, _currentLeft, _currentRight, FrontLeft.Encoder, FrontRight.Encoder, RearLeft.Encoder, RearRight.Encoder);
//	WriteOutput();


}

void oneHzLoop(){
	SendTelemetry();
}

void loop()
{
	timerHandler();
	delay(1);	
}


