



void calibrationFinished() {
	blinkLED(10, 15, 1 + 3 * 0);

	transitionToSensorState(SnsrZeroCompleted);
}

void beginSensorZero(NiVekMessage *msg) {
	Serial.write("Begin Zero.");
	transitionToSensorState(SnsrZero);

	beginAccZero();
	beginGyroZero();

	sendAck(msg);
}