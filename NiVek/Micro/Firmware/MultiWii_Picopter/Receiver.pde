
// Configure each rc pin for PCINT
void initReceiver() {
	PicopterRadio.beginAsDrone(0, 0, PCR_FLIER_DEVADDR);
}

/* rcData is populated int "pcr_rcChan" via ISR in the radio code,
   take those raw values and make available to app, via rcDataArray  */
void computeRC() {
	static int16_t rcData4Values[8][4], rcDataMean[8];
	static uint8_t rc4ValuesIndex = 0;
	uint8_t chan, a, ind;

	rc4ValuesIndex++;

	for (chan = 0; chan < 8; chan++) {
		rcData4Values[chan][rc4ValuesIndex % 4] = pcr_rcChan[chan];
		rcDataMean[chan] = 0;

		if (chan != AUX1 && chan != AUX2) {
			for (a=0;a<4;a++) 
				rcDataMean[chan] += rcData4Values[chan][a];

			rcDataMean[chan] = (rcDataMean[chan] + 2) / 4;

			if (rcDataMean[chan] < rcData[chan] - 3)  
				rcData[chan] = rcDataMean[chan] + 2;

			if (rcDataMean[chan] > rcData[chan] + 3)  
				rcData[chan] = rcDataMean[chan] - 2;
		}
		else 
			rcData[chan] = pcr_rcChan[chan];
	}
}

void handleModeSwitch(){
	return;

	uint16_t modeChan;
	modeChan = pcr_rcChan[4];

	if (pcr_rcChan[AUX2] == (pcr_rcChan[AUX1] ^ 0xAA55)){
		if (modeChan & (1 << PCC_ZEROSENSORS)) {
			beginSensorZero(null);
		}

		if ((modeChan & (1 << PCC_ACCTRIM)) && (modeChan & (1 << PCC_ACCTRIMUP)) && (lastModeChan & (1 << PCC_ACCTRIMUP)) == 0) {
			accTrim[PITCH] += 2;
			writeParams();
		}

		if ((modeChan & (1 << PCC_ACCTRIM)) && (modeChan & (1 << PCC_ACCTRIMDOWN)) && (lastModeChan & (1 << PCC_ACCTRIMDOWN)) == 0) {
			accTrim[PITCH] -= 2;
			writeParams();
		}

		if ((modeChan & (1 << PCC_ACCTRIM)) && (modeChan & (1 << PCC_ACCTRIMLEFT)) && (lastModeChan & (1 << PCC_ACCTRIMLEFT)) == 0) {
			accTrim[ROLL] -= 2;
			writeParams();
		}

		if ((modeChan & (1 << PCC_ACCTRIM)) && (modeChan & (1 << PCC_ACCTRIMRIGHT)) && (lastModeChan & (1 << PCC_ACCTRIMRIGHT)) == 0) {
			accTrim[ROLL] += 2;
			writeParams();
		}

		if (modeChan & (1 << PCC_STABLEMODE) && FlightMode == FlightModeAcrobat)
			turnOnStableMode(null);

		if (modeChan & (1 << PCC_ACROMODE) && FlightMode == FlightModeStable)
			turnOffStableMode(null);
	}

	lastModeChan = modeChan;
}

void radioHandler() {
	computeRC();
	handleModeSwitch();
}