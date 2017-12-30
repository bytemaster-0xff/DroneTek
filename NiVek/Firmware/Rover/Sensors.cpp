#include "Sensors.h"
#include "Compass.h"
#include "Comms.h"

#define CALIBRATE_XY  0xA0
#define END_CALIBRATE_XY  0xA3

void HandleSensorMessage(NiVekMessage *msg)
{
	switch (msg->TypeId)
	{
	case CALIBRATE_XY:
		BeginCompassCalibration();
		break;
	case END_CALIBRATE_XY:
		EndCompassCalibration();
		break;
	}

	if (msg->ExpectACK)
		sendAck(msg);
}