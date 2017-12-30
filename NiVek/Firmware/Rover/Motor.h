
#include <stdint.h>

typedef enum Directions
{
	DirectionOff = 0,
	DirectionForward = 10,
	DirectionReverse = 11
} Directions;

class Motor
{
private:
	uint8_t m_address;
	long m_currentSpeed;

	void UpdateMotor();

public:
	

	Motor(uint8_t  address);

	long Encoder;

	void SetDirection(Directions direction);
	void SetSpeed(int speed);
	void SetSpeedDirection(int speed, Directions direction);
	void Stop();

	~Motor();

private:
	Directions m_currentDirection;
};

