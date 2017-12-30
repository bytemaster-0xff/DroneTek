#include "NiVekMessageParser.h"
#include "Motor.h"

extern Motor FrontRight;
extern Motor FrontLeft;
extern Motor RearRight;
extern Motor RearLeft;


void HandleMotorMessage(NiVekMessage *message);

void ReadWheelEncoders();