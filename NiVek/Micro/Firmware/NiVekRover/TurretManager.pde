
#include "Turret.h"


Servo panServo = Servo();
Servo tiltServo = Servo();

const int periodAsMeasured = 24;
const int minMicroseconds = 1200;
const int maxMicrsseconds = 2400;

#define FRONT_SONAR_PIN 12

int _currentPan;
int _currentTilt;

void InitTurret(){

	_currentTilt = 1400;
	_currentPan = 1400;
}

void SetTurretPanTilt(int pan, int tilt){

}

void UpdateTurret()
{

}

bool isPinging = false;
byte _sendBuffer[4];
unsigned long _lastMicros = 0;

void ReadSonar()
{
	//Serial.write("HI");
	/*
	Wire.requestFrom(0x40, 6);

	while (Wire.available())
	{
		char c = Wire.read();
	//	Serial.print(c);
	}
	*/
	
	/*
	pinMode(FRONT_SONAR_PIN, OUTPUT);
	digitalWrite(FRONT_SONAR_PIN, LOW);                          // Make sure pin is low before sending a short high to trigger ranging
	delayMicroseconds(2);
	digitalWrite(FRONT_SONAR_PIN, HIGH);                         // Send a short 10 microsecond high burst on pin to start ranging
	delayMicroseconds(100);
	digitalWrite(FRONT_SONAR_PIN, LOW);                                  // Send pin low again before waiting for pulse back in
	//pinMode(FRONT_SONAR_PIN, INPUT);
//	ulong duration = pulseIn(FRONT_SONAR_PIN, HIGH);                        // Reads echo pulse in from SRF05 in micro seconds
	//int distance = duration / 58;                                      // Dividing this by 58 gives us a distance in cm
	/*Logger.print("DIST: "); */
	//Logger.print(distance);*/
	//Logger.println("cm");

	//pinMode(FRONT_SONAR_PIN, OUTPUT);
//	digitalWrite(FRONT_SONAR_PIN, LOW);                                  // Send pin low again before waiting for pulse back in

	/*if (!isPinging){
		writeGPIO4(CMD, GET_S4A);
	}
	else{
		short result = readShortGPIO4(RESULT);
	}*/

//	isPinging = !isPinging;

}

void HandleTurretMessge(NiVekMessage *msg)
{
	switch (msg->TypeId)
	{
		case 100:
			/*int pan =  msg->MsgBuffer[0];
			int tilt = msg->MsgBuffer[1];


			Logger.print("New Value");
			Logger.print(pan);
			Logger.print(" " );
			Logger.println(tilt);

			panServo.write(pan);
			tiltServo.write(tilt);*/
			break;

		case 110: _currentPan = 1400; _currentTilt = 1400; break;
		case 111: _currentTilt -= 20; break;
		case 112: _currentTilt += 20; break;
		case 113: _currentPan -= 20; break;
		case 114: _currentPan += 20; break;
	}

	_sendBuffer[0] = _currentPan >> 8;
	_sendBuffer[1] = _currentPan & 0xFF;
	_sendBuffer[2] = _currentTilt >> 8;
	_sendBuffer[3] = _currentTilt & 0xFF;

	Wire.beginTransmission(0x40);
	Wire.write(_sendBuffer, 4);
	Wire.endTransmission();

	Logger.println(_currentTilt);
	Logger.println(_currentPan);
	

	sendAck(msg);
}