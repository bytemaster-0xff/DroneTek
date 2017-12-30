#include "Motor.h"
#include "Wire.h"
#include "Comms.h"


Motor::Motor(uint8_t address)
{
	m_address = address;
	m_currentSpeed = 0;
	m_currentDirection = DirectionForward;
}



void Motor::UpdateMotor() {
	uint8_t m_motorOutBuffer[6];
	Wire.beginTransmission(0x42);
	
	m_motorOutBuffer[0] = m_address;
	m_motorOutBuffer[1] = m_currentSpeed >> 8;
	m_motorOutBuffer[2] = m_currentSpeed & 0xFF;
	m_motorOutBuffer[3] = m_currentDirection;

	Wire.write(m_motorOutBuffer, 4);
	Wire.endTransmission();
}

void Motor::SetSpeedDirection(int speed, Directions direction){
	m_currentSpeed = speed;
	m_currentDirection = direction;

	UpdateMotor();
}

void Motor::SetDirection(Directions direction) {
	m_currentDirection = direction;
	if (DirectionOff){
		m_currentSpeed = 0;
		SetSpeed(0);
	}

	UpdateMotor();
}

void Motor::Stop(){
	m_currentDirection = DirectionOff;
	m_currentSpeed = 0;
	UpdateMotor();
}

void Motor::SetSpeed(int speed) {
	if (m_currentDirection == DirectionOff)
		m_currentDirection = DirectionForward;

	m_currentSpeed = speed;
	UpdateMotor();
}

Motor::~Motor()
{
}