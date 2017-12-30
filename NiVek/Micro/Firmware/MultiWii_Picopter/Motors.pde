#define NUMBER_MOTOR 4

void writeMotors() { // [1000;2000] => [125;250]
	#define _pqSetPwm(_x, _s) (((((_x) > (MINCOMMAND + (0xFF * _s))) ? (MINCOMMAND + (0xFF * _s)) : (((_x) < (MINCOMMAND)) ? (MINCOMMAND) : (_x))) - MINCOMMAND) / _s)

	OCR1A = _pqSetPwm(motor[0], 4); // REAR_R
	OCR1B = _pqSetPwm(motor[1], 4); // FRONT_R
	OCR3A = _pqSetPwm(motor[2], 4); // REAR_L
	OCR1C = _pqSetPwm(motor[3], 4); // FRONT_L
}

void writeAllMotors(int16_t mc) {   // Sends commands to all motors
	for (uint8_t i = 0; i < NUMBER_MOTOR; i++)
		motor[i] = mc;

	writeMotors();
}

void initOutput() {
	// initialize timer compare match outputs
	// some of this initialization is done in the "core" wiring.c
	
	//Initialize the I/O pins for the motors
	DDRB |= _BV(5) | _BV(6) | _BV(7);
	PORTB &= ~(_BV(5) | _BV(6) | _BV(7));

	DDRE |= _BV(3);
	PORTE &= ~_BV(3);

	//Initialize the 
	TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(COM1C1);
	TCCR3A |= _BV(COM1A1);
	writeAllMotors(0);
}

#define PIDMIX(X,Y,Z) rcCommand[THROTTLE] + (axisPID[ROLL] * X) + (axisPID[PITCH] * Y) + (YAW_DIRECTION * axisPID[YAW] * Z)

void mixTable() {
	int16_t maxMotor;
	uint8_t i, axis;

	axisPID[YAW] = constrain(axisPID[YAW], -100 - abs(rcCommand[YAW]), +100 + abs(rcCommand[YAW]));

	motor[0] = PIDMIX(-1, +1, -1); //REAR_R  OCR1A
	motor[1] = PIDMIX(-1, -1, +1); //FRONT_R OCR1B
	motor[2] = PIDMIX(+1, +1, +1); //REAR_L  OCR3A
	motor[3] = PIDMIX(+1, -1, -1); //FRONT_L OCR1C

	maxMotor = motor[0];

	for (i = 1; i< NUMBER_MOTOR; i++){
		if (motor[i]>maxMotor) 
			maxMotor = motor[i];
	}

	for (i = 0; i < NUMBER_MOTOR; i++) {
		if (maxMotor > MAXTHROTTLE) // this is a way to still have good gyro corrections if at least one motor reaches its max.
			motor[i] -= maxMotor - MAXTHROTTLE;

		motor[i] = constrain(motor[i], MINTHROTTLE, MAXTHROTTLE);

		if ((rcData[THROTTLE]) < MINCHECK)
			motor[i] = MINTHROTTLE;

		motor[i] -= 500;
	}
}

void updateMotors(){
	if (IsArmed == Safe) {
		OCR1A = 0;
		OCR1B = 0;
		OCR1C = 0;
		OCR3A = 0;
	}
	else {
		writeMotors();
	}
}

void beginMotorConfig(NiVekMessage *msg){
	Serial.println("Begin Motor Configuration");

	uint8_t idx;
	for (idx = 0; idx < NUMBER_MOTOR; idx++) {
		motor[idx] = 0;
	}

	writeMotors();

	GPIOState = GPIOMotorConfig;

	sendAck(msg);
}

void endMotorConfig(NiVekMessage *msg){
	if (GPIOState == GPIOMotorConfig){
		uint8_t idx;
		for (idx = 0; idx < NUMBER_MOTOR; idx++) 
			motor[idx] = 0;

		writeMotors();

		sendAck(msg);

		GPIOState = GPIOReady;

		Serial.println("End Motor Configuration");
	}
	else
		sendNak(msg);
}

void setMotorPower(NiVekMessage *msg){
	if (GPIOState != GPIOMotorConfig){
		sendNak(msg);
		return;
	}

	uint8_t idx;
	if (msg->MsgBuffer[0] == 0xFF){
		for (idx = 0; idx < NUMBER_MOTOR; idx++) {
			motor[idx] = msg->MsgBuffer[1] * 8;
		}

		Serial.print("Setting all motors to: ");
		Serial.print(motor[0]);
		Serial.println();
	}
	else{
		motor[msg->MsgBuffer[0] - 1] = msg->MsgBuffer[1] * 8;

		Serial.print("Setting motor ");
		Serial.print((uint16_t)msg->MsgBuffer[0]);
		Serial.print("Setting motor ");
		Serial.print(motor[msg->MsgBuffer[0] - 1]);
		Serial.println();
	}

	sendAck(msg);
}


void printMotors() {
	uint8_t idx;

	Serial.print("RAW THROTTLE IN ");
	Serial.print(rcData[THROTTLE]);;
	Serial.print(", ");
	Serial.print(rcCommand[THROTTLE]);;
	Serial.print(", ");

	for (idx = 0; idx < 4; ++idx){
		Serial.print(motor[idx]);;
		Serial.print(", ");
	}
	Serial.println();
}

void initMotors() {
	Serial.println("Initialize Motor");

	initOutput();
}

void motorsHandler(){
	if (GPIOState == GPIOMotorConfig){
		writeMotors();
	}
	else{
		mixTable();	
		updateMotors();
	}
}