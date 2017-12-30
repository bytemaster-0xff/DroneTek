



void getPIDValue(NiVekMessage *msg){

	
}

void setPIDValue(NiVekMessage *msg){
	Serial.print("SHOULD SET NEW PID VALUE!");

	uint8_t axis = msg->MsgBuffer[0];
	P8[axis] = msg->MsgBuffer[1];
	I8[axis] = msg->MsgBuffer[2];
	D8[axis] = msg->MsgBuffer[3];

	writeParams();

	sendAck(msg);
}

void pid(){
	uint8_t axis;
	int16_t PTerm, ITerm, DTerm;
	int16_t error, errorAngle;
	int16_t delta, deltaSum;
	static int16_t lastGyro[3] = { 0, 0, 0 };
	static int16_t delta1[3], delta2[3];

	for (axis = 0; axis < 3; axis++) {
		if (FlightMode == FlightModeStable && axis < 2) {
			errorAngle = constrain(2 * rcCommand[axis], -500, +500) - att.angle[axis] + accTrim[axis];	//16 bits is ok here
			PTerm = (int32_t) errorAngle * P8[PIDLEVEL] / 100;										//32 bits is needed for calculation: errorAngle*P8[PIDLEVEL] could exceed 32768   16 bits is ok for result

			errorAngleI[axis] = constrain(errorAngleI[axis] + errorAngle, -10000, +10000);			//WindUp     //16 bits is ok here
			ITerm = ((int32_t) errorAngleI[axis] * I8[PIDLEVEL]) >> 12;								//32 bits is needed for calculation:10000*I8 could exceed 32768   16 bits is ok for result
		}
		else {
			if (abs(rcCommand[axis])<350)
				error = rcCommand[axis] * 10 * 8 / P8[axis];										//16 bits is needed for calculation: 350*10*8 = 28000      16 bits is ok for result if P8>2 (P>0.2)
			else
				error = (int32_t) rcCommand[axis] * 10 * 8 / P8[axis]; //32 bits is needed for calculation: 500*5*10*8 = 200000   16 bits is ok for result if P8>2 (P>0.2)

			error -= imu.gyroData[axis];

			PTerm = rcCommand[axis];

			errorGyroI[axis] = constrain(errorGyroI[axis] + error, -16000, +16000);          //WindUp //16 bits is ok here

			if (abs(imu.gyroData[axis])>640)
				errorGyroI[axis] = 0;

			ITerm = (errorGyroI[axis] / 125 * I8[axis]) >> 6;                                   //16 bits is ok here 16000/125 = 128 ; 128*250 = 32000
		}

		if (abs(imu.gyroData[axis]) < 160)
			PTerm -= imu.gyroData[axis] * dynP8[axis] / 10 / 8; //16 bits is needed for calculation   160*200 = 32000         16 bits is ok for result
		else
			PTerm -= (int32_t) imu.gyroData[axis] * dynP8[axis] / 10 / 8;

		delta = imu.gyroData[axis] - lastGyro[axis];
		lastGyro[axis] = imu.gyroData[axis];
		deltaSum = delta1[axis] + delta2[axis] + delta;
		delta2[axis] = delta1[axis];
		delta1[axis] = delta;

		if (abs(deltaSum) < 640)
			DTerm = (deltaSum*dynD8[axis]) >> 5;    //16 bits is needed for calculation 640*50 = 32000           16 bits is ok for result 
		else
			DTerm = ((int32_t) deltaSum*dynD8[axis]) >> 5;              //32 bits is needed for calculation

		axisPID[axis] = PTerm + ITerm - DTerm;
	}
}

void dynamicPID() {
	uint8_t axis;
	uint8_t prop1, prop2;

	uint16_t pMeterRaw, powerValue;     //used for current reading

	//PITCH & ROLL only dynamic PID adjustemnt,  depending on throttle value
	if (rcData[THROTTLE] < 1500)
		prop2 = 100;
	else if (rcData[THROTTLE] < 2000)
		prop2 = 100 - (uint16_t) dynThrPID*(rcData[THROTTLE] - 1500) / 500;
	else
		prop2 = 100 - dynThrPID;

	for (axis = 0; axis < 3; axis++) {
		uint16_t tmp = min(abs(rcData[axis] - MIDRC), 500);

		if (axis != 2) { //ROLL & PITCH
			uint16_t tmp2 = tmp / 100;
			rcCommand[axis] = lookupRX[tmp2] + (tmp - tmp2 * 100) * (lookupRX[tmp2 + 1] - lookupRX[tmp2]) / 100;
			prop1 = 100 - (uint16_t) rollPitchRate*tmp / 500;
			prop1 = (uint16_t) prop1*prop2 / 100;
		}
		else { //YAW
			rcCommand[axis] = tmp;
			prop1 = 100 - (uint16_t) yawRate*tmp / 500;
		}

		dynP8[axis] = (uint16_t) P8[axis] * prop1 / 100;
		dynD8[axis] = (uint16_t) D8[axis] * prop1 / 100;

		if (rcData[axis] < MIDRC)
			rcCommand[axis] = -rcCommand[axis];
	}

	rcCommand[THROTTLE] = MINTHROTTLE + (int32_t) (MAXTHROTTLE - MINTHROTTLE)* (rcData[THROTTLE] - MINCHECK) / (2000 - MINCHECK);
}

void pidHandler() {
	if (GPIOState != GPIOMotorConfig){
		pid();
		dynamicPID();
	}
}
