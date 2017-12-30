#include "Turret.h"
#include "Wire.h"
#include "Comms.h"

uint16_t _currentPan;
uint16_t _currentTilt;

uint16_t _frontSonar;
uint16_t _rearSonar;
uint16_t _currentLeft;
uint16_t _currentRight;

bool _scan = true;

int _scanRate = 10;

#define TURRET_MSG_BUFFER_SIZE 4

byte _sendBuffer[4];

byte _turretMsgBufferLength[TURRET_MSG_BUFFER_SIZE];
byte _turretMsgBuffer[TURRET_MSG_BUFFER_SIZE][16];

byte _turretMsgBufferHead = 0;
byte _turretMsgBufferTail = 0;

void InitTurret(){

	Wire.begin();

	_currentTilt = 1400;
	_currentPan = 1400;
}

void ReadTurretSensors()
{
	//Wire.beginTransmission(0x40);
	int available = Wire.requestFrom(0x40, 10);
	if (available == 10)
	{		
		_currentPan = Wire.read() << 8 | Wire.read();
		_currentTilt = Wire.read() << 8 | Wire.read();
		_frontSonar = Wire.read();
		_rearSonar = Wire.read();
		_currentLeft = Wire.read() << 8 | Wire.read();
		_currentRight = Wire.read() << 8 | Wire.read();
	}

	while (_turretMsgBufferHead != _turretMsgBufferTail)
	{
		Wire.beginTransmission(0x40);
		Wire.write(_turretMsgBuffer[_turretMsgBufferTail], _turretMsgBufferLength[_turretMsgBufferTail]);
		_turretMsgBufferTail++;
		Wire.endTransmission();

		if (_turretMsgBufferTail >= TURRET_MSG_BUFFER_SIZE)
			_turretMsgBufferTail = 0;
	}
}


void HandleTurretMessge(NiVekMessage *msg){
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
	case 120: 
		_turretMsgBufferLength[_turretMsgBufferHead] = 1;
		_turretMsgBuffer[_turretMsgBufferHead][0] = 10;
		_scan = true; 
		break;
	case 121: 
		_turretMsgBufferLength[_turretMsgBufferHead] = 1;
		_turretMsgBuffer[_turretMsgBufferHead][0] = 11;
		_scan = false; 
		break;
	}

	if (msg->TypeId >= 110 && msg->TypeId <= 114)
	{
		_turretMsgBufferLength[_turretMsgBufferHead] = 5;
		_turretMsgBuffer[_turretMsgBufferHead][0] = 20;
		_turretMsgBuffer[_turretMsgBufferHead][1] = _currentPan >> 8;
		_turretMsgBuffer[_turretMsgBufferHead][2] = _currentPan & 0xFF;
		_turretMsgBuffer[_turretMsgBufferHead][3] = _currentTilt >> 8;
		_turretMsgBuffer[_turretMsgBufferHead][4] = _currentTilt & 0xFF;
	}

	_turretMsgBufferHead++;
	if (_turretMsgBufferHead == TURRET_MSG_BUFFER_SIZE)
		_turretMsgBufferHead = 0;

	if (msg->ExpectACK)
		sendAck(msg);
}