

void connectionMonitor(){
	uint8_t idx;

	// Picopter's radio library has a built in function to determine whether or not the signal is present
	if (PicopterRadio.isPresent()) {
		if (ConnectionState == Disconnected){
			Serial.write("CONNECTION ESTABLISHED\r\n");
			ConnectionState = Connected;
		}
	}
	else {
		for (idx = 0; idx < 3; idx++) 
			rcData[idx] = MIDRC;

		rcData[THROTTLE] = FAILSAVE_THROTTLE;

		safeNiVek(null);
		if (ConnectionState == Connected){
			Serial.write("LOST CONNECTION\r\n");

			ConnectionState = Disconnected;
		}
	}
}

void isFlipped() {
	if (FlightMode == FlightModeStable){
		if (accSmooth[YAW] < -400 && IsArmed == Armed){
			IsArmed = Safe;
			Serial.write("Thinking quad is on back, disarm");
		}
	}
}

void watchdog(){
	onTheGround();
	isFlipped();
	connectionMonitor();
}
