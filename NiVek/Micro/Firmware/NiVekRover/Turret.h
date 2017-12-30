#pragma once
class Turret
{
private:
	int m_pinPan;
	int m_pinTilt;

public:
	Turret(int pinPan, int panTilt);
	void SetPan(int angle);
	void SetTilt(int angle);

	void Update();

	~Turret();
};

