
#include "Motor.h"


Motor FrontRight(10);
Motor FrontLeft(12);
Motor RearRight(11);
Motor RearLeft(13);

void HandleMotorMessage(NiVekMessage *msg) {

	Logger.println(msg->TypeId);
	switch (msg->TypeId)
	{
	case 100:
		switch (msg->MsgBuffer[0])
		{
			case 255:
				FrontRight.SetSpeed(msg->MsgBuffer[1]);
				FrontLeft.SetSpeed(msg->MsgBuffer[1]);
				RearRight.SetSpeed(msg->MsgBuffer[1]);
				RearLeft.SetSpeed(msg->MsgBuffer[1]);
				break;
			case 1: FrontRight.SetSpeed(msg->MsgBuffer[1]); break;
			case 2: FrontLeft.SetSpeed(msg->MsgBuffer[1]); break;
			case 3: RearRight.SetSpeed(msg->MsgBuffer[1]); break;
			case 4: RearLeft.SetSpeed(msg->MsgBuffer[1]); break;
				break;

		}
		break;
	case 110:
		FrontRight.SetSpeed(0);
		FrontLeft.SetSpeed(0);
		RearRight.SetSpeed(0);
		RearLeft.SetSpeed(0);
		break;
	}

	if (msg->ExpectACK)
		sendAck(msg);
}

void MotorsUpdate(){
	
}
