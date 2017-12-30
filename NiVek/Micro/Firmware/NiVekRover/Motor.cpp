#include "Arduino.h"
#include "Motor.h"
#include <Wire.h>

Motor::Motor(byte address)
{
	m_address = address;
	m_currentSpeed = 0;
	m_currentDirection = DirectionOff;
}

byte m_motorOutBuffer[6];

void Motor::UpdateMotor() {
	Wire.beginTransmission(0x42);

	Serial.print("MOTOR: ");
	Serial.print(m_address);
	Serial.print(" ");
	Serial.print(m_currentSpeed);
	Serial.print(" ");
	Serial.println(m_currentDirection);
	
	m_motorOutBuffer[0] = m_address;
	m_motorOutBuffer[1] = m_currentSpeed >> 8;
	m_motorOutBuffer[2] = m_currentSpeed & 0xFF;
	m_motorOutBuffer[3] = m_currentDirection;

	Wire.write(m_motorOutBuffer, 4);
	Wire.endTransmission();
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
