#include "Arduino.h"
#include "Turret.h"


Turret::Turret(int pinPan, int pinTilt){
	m_pinPan = pinPan;
	m_pinTilt = pinTilt;

	pinMode(m_pinPan, OUTPUT);
	pinMode(m_pinTilt, OUTPUT);

	analogWrite(m_pinPan, 100);
	analogWrite(m_pinTilt, 100);
}

void Turret::SetPan(int angle) {
	Serial.write("PAN ");
	Serial.println(angle);
	analogWrite(m_pinPan, angle);
}

void Turret::SetTilt(int angle) {
	Serial.write("TILT ");
	Serial.println(angle);	analogWrite(m_pinTilt, angle);
}

void Turret::Update() {

}

Turret::~Turret()
{
}
