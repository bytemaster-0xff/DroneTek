
void writeWelcomeMessage() {
	Serial.println("NiVek Micro - Starting Up ");
	Serial.print("V");
	Serial.print(APP_VERSION);
	Serial.println();
}

void blinkLED(uint8_t num, uint8_t wait, uint8_t repeat) {
	uint8_t i, r;
	for (r = 0; r < repeat; r++) {
		for (i = 0; i < (num / 2); i++) {
			delay(wait);
			delay(wait);
		}
		delay(60);
	}
}

void fastBlink() {
	uint8_t i, r;
	while (true){
		delay(50);
	}
}

static IsArmedStates LastIsArmed;

void checkArmedStateChange(){
	uint16_t modeChan;
	modeChan = pcr_rcChan[4];
	// also there are dedicated buttons on the controller to arming and disarming
	if (IsArmed == Armed && modeChan & (1 << PCC_DISARM))
		safeNiVek(null);

	if (IsArmed == Safe && (modeChan & (1 << PCC_ARM)) && pcr_rcChan[AUX2] == (pcr_rcChan[AUX1] ^ 0xAA55))
		armNiVek(null);

	if (!getOkToArm() && IsArmed == Armed)
		safeNiVek(null);

	if (IsArmed != LastIsArmed){
		if (IsArmed == Armed)
			Serial.println("State changed to Armed.\r\n");
		else
			Serial.println("State changed to Safe.\r\n");

		LastIsArmed = IsArmed;
	}
}

void batteryCheck(){
	if ((BATMON & (1 << BATMON_OK)) == 0) {
		// if low battery
		IsArmed = Safe;

		Serial.write("Detect low battery conditions\r\n.");

		// freeze code forever and blink LEDs
		while (1) {
			OCR1A = 0;
			OCR1B = 0;
			OCR1C = 0;
			OCR3A = 0;
			delay(50);
			delay(50);
		}
	}
}
