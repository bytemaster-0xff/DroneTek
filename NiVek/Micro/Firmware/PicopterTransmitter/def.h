
#ifndef DEF_H
#define DEF_H

enum LEDTypes{
	LEDComms = 0,
	LEDOnline = 1,
	LEDFailure = 2,
	LEDArmed = 3,
};

#define NUMBERLEDS LEDArmed + 1

enum LEDStates{
	LEDOn,
	LEDOff,
	LEDFastBlick,
	LEDSlowBlink,
};


#define LEDPort PORTF

#define LEDPINComms 2
#define LEDPINArmed 3
#define LEDPINFail 4
#define LEDPINOnline 5


#endif