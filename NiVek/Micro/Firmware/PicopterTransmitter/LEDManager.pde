

//Used for flashing (fast/slow LEDS)
LEDTypes		__ledTimings[NUMBERLEDS];
LEDStates		__ledState[NUMBERLEDS];
uint8_t  __ledPins[NUMBERLEDS];

void setLEDState(LEDTypes led, LEDStates newState){
	__ledState[led] = newState;

	switch (newState){
		case LEDOn: LEDPort |= _BV(__ledPins[led]); break;
		case LEDOff: LEDPort &= ~_BV(__ledPins[led]); break;
		case LEDFastBlick:

			break;
		case LEDSlowBlink:

			break;

	}
}

void updateLEDS(){

}


void initLEDS(){
	__ledPins[LEDComms] = LEDPINComms;
	__ledPins[LEDArmed] = LEDPINArmed;
	__ledPins[LEDFailure] = LEDPINFail;
	__ledPins[LEDOnline] = LEDPINOnline;

	DDRF |= 1 << LEDComms;
	DDRF |= 1 << LEDOnline;
	DDRF |= 1 << LEDFailure;
	DDRF |= 1 << LEDArmed;

	setLEDState(LEDArmed, LEDOff);
	setLEDState(LEDFailure, LEDOff);
	setLEDState(LEDOnline, LEDOff);
	setLEDState(LEDComms, LEDOff);
}
