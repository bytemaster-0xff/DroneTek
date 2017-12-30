#define MINCOMMAND 0

int16_t motor[4];

void writeMotors() { // [1000;2000] => [125;250]
	#define _pqSetPwm(_x, _s) (((((_x) > (MINCOMMAND + (0xFF * _s))) ? (MINCOMMAND + (0xFF * _s)) : (((_x) < (MINCOMMAND)) ? (MINCOMMAND) : (_x))) - MINCOMMAND) / _s)

	OCR1A = _pqSetPwm(motor[0], 4); // REAR_R
	OCR1B = _pqSetPwm(motor[1], 4); // FRONT_R
	OCR3A = _pqSetPwm(motor[2], 4); // REAR_L
	OCR1C = _pqSetPwm(motor[3], 4); // FRONT_L
}



void writeAllMotors(uint16_t power){
	motor[0] = power;
	motor[1] = power;
	motor[2] = power;
	motor[3] = power;
}

void vestPower(uint8_t idx, uint16_t value){
	DebugWriter.print("Setting Power:");
	DebugWriter.print(" ");
	DebugWriter.print((long)idx);
	DebugWriter.print(" ");
	DebugWriter.print(value);
	DebugWriter.println("");

	motor[idx] = value;
}

void vestAllOff() {
	DebugWriter.println("VEST ALL OFF.");

	writeAllMotors(0);
}

void vestAllOn(){
	DebugWriter.println("VEST ALL ON.");

	writeAllMotors(2000);
}


void updateMotors(){
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
