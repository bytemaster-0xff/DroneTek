#include "Drive.h"
#include "Comms.h"
#include "Wire.h"

Motor FrontRight(10);
Motor FrontLeft(12);
Motor RearRight(11);
Motor RearLeft(13);

#define GET_ENCODINGS				20
#define RESET_ENCODINGS				21

void ReadWheelEncoders(){
	uint8_t buffer[1];
	buffer[0] = GET_ENCODINGS;

	Wire.beginTransmission(0x42);
	Wire.write(buffer, 1);
	Wire.endTransmission();

	int available = Wire.requestFrom(0x42, 16);
	if (available == 16)
	{
		FrontRight.Encoder = Wire.read() << 24 | Wire.read() << 16 | Wire.read() << 8 | Wire.read();
		RearRight.Encoder = Wire.read() << 24 | Wire.read() << 16 | Wire.read() << 8 | Wire.read();
		FrontLeft.Encoder = Wire.read() << 24 | Wire.read() << 16 | Wire.read() << 8 | Wire.read();
		RearLeft.Encoder = Wire.read() << 24 | Wire.read() << 16 | Wire.read() << 8 | Wire.read();
	}
}

/*		0
 *270		90
 *	   180
 */

void SetDirection(uint8_t power, uint16_t angle)
{
	FrontRight.SetSpeedDirection(power, DirectionReverse);
	RearRight.SetSpeedDirection(power, DirectionForward);

	FrontLeft.SetSpeedDirection(power, DirectionForward);
	RearLeft.SetSpeedDirection(power, DirectionReverse);
}

void HandleMotorMessage(NiVekMessage *msg) {
	uint8_t m_motorOutBuffer[6];

	if (msg->ExpectACK)
		sendAck(msg);

	switch (msg->TypeId)
	{
	case 150: /* Specific Motor */
	{
		byte direction = msg->MsgBuffer[2] >> 8 | msg->MsgBuffer[3];
		switch (msg->MsgBuffer[0])
		{
		case 255:
			FrontRight.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse);
			FrontLeft.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse);
			RearRight.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse);
			RearLeft.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse);

			Wire.beginTransmission(0x42);
			m_motorOutBuffer[0] = msg->TypeId;
			m_motorOutBuffer[1] = msg->MsgBuffer[0];
			Wire.write(m_motorOutBuffer, 2);
			Wire.endTransmission();

			break;
		case 1: FrontRight.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse); break;
		case 2: FrontLeft.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse); break;
		case 3: RearRight.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse); break;
		case 4: RearLeft.SetSpeedDirection(msg->MsgBuffer[1], direction == 1 ? DirectionForward : DirectionReverse); break;
			break;

		}
	}
		break;
	case 160:

		Wire.beginTransmission(0x42);

		m_motorOutBuffer[0] = RESET_ENCODINGS;
		Wire.write(m_motorOutBuffer, 1);
		Wire.endTransmission();

		break;

	case 120:
	{
		uint16_t direction = msg->MsgBuffer[1] << 8 | msg->MsgBuffer[2];
		SetDirection(msg->MsgBuffer[0], direction);
	}
		break;
	case 100: /* Stop */
/*		FrontRight.SetSpeed(0);
		FrontLeft.SetSpeed(0);
		RearRight.SetSpeed(0);
		RearLeft.SetSpeed(0);*/

		Wire.beginTransmission(0x42);
		m_motorOutBuffer[0] = msg->TypeId;

		Wire.write(m_motorOutBuffer, 1);
		Wire.endTransmission();
		break;

	case 110: /* Turn Left */
	case 111: /* Turn Right */
	case 121: /* Forward */
	case 122: /* Up Right */
	case 123: /* Right */
	case 124: /* DownRight */
	case 125: /* Down */
	case 126: /* Down Left */
	case 127: /* Left */
	case 128: /* Up Left */
		Wire.beginTransmission(0x42);
		m_motorOutBuffer[0] = msg->TypeId;
		m_motorOutBuffer[1] = msg->MsgBuffer[0];
		m_motorOutBuffer[2] = 0;

		Wire.write(m_motorOutBuffer, 3);
		Wire.endTransmission();

	}
}

void MotorsUpdate(){

}
