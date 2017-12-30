#pragma once

class Motor
{
private: 
	byte m_address;
	long m_currentSpeed;

	void UpdateMotor();

public:
	typedef enum Directions
	{
		DirectionOff = 0,
		DirectionForward = 10,
		DirectionReverse = 11
	} Directions;

	Motor(byte address);

	void SetDirection(Directions direction);
	void SetSpeed(int speed);
	void Stop();

	~Motor();

private:
	Directions m_currentDirection;
};

