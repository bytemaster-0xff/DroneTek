void initHardware() {
	Serial.println("Initialize Hardware");

	BATMON = 0x13; // enable battery monitor, high threshold at 2.775V
}

