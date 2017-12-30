#include <avr/eeprom.h>

static uint8_t checkNewConf = 12;

typedef struct eep_entry_t{
  void *  var;
  uint8_t size;
};

// ************************************************************************************************************
// EEPROM Layout definition
// ************************************************************************************************************
static eep_entry_t eep_entry[] = {
	&checkNewConf, sizeof(checkNewConf),
	&DroneName, sizeof(DroneName),
	&LocalAddress, sizeof(LocalAddress),
	&P8, sizeof(P8),
	&I8, sizeof(I8),
	&D8, sizeof(D8),
	&rcRate8, sizeof(rcRate8),
	&rcExpo8, sizeof(rcExpo8),
	&rollPitchRate, sizeof(rollPitchRate),
	&yawRate, sizeof(yawRate),
	&dynThrPID, sizeof(dynThrPID),
	&accZero, sizeof(accZero),
	&magZero, sizeof(magZero),
	&accTrim, sizeof(accTrim),
	&GyroAccCompFilter, sizeof(GyroAccCompFilter),
	&GyroMagCompFilter, sizeof(GyroMagCompFilter),
	&AccBaroCompFilter, sizeof(AccBaroCompFilter),
};  

#define EEBLOCK_SIZE sizeof(eep_entry)/sizeof(eep_entry_t)
// ************************************************************************************************************

void readEEPROM() {
	uint8_t i, _address = eep_entry[0].size;
	
	for (i = 1; i < EEBLOCK_SIZE; i++) {
		eeprom_read_block(eep_entry[i].var, (void*) (_address), eep_entry[i].size); 
		_address += eep_entry[i].size;
	}

	for(i=0;i<7;i++) 
		lookupRX[i] = (2500+rcExpo8*(i*i-25))*i*(int32_t)rcRate8/1250;
}

void writeParams() {
	uint8_t i, _address = 0;

	for (i = 0; i < EEBLOCK_SIZE; i++) {
		eeprom_write_block(eep_entry[i].var, (void*) (_address), eep_entry[i].size); 
		_address += eep_entry[i].size;
	}
}

void setDefaults() {
	Serial.println("EPROM EMPTY - Setting Defaults");

	memset(DroneName, 0, sizeof(DroneName));
	strcpy(DroneName, "RightGlove");

	LocalAddress = 21;

	P8[ROLL] = 70; I8[ROLL] = 20; D8[ROLL] = 20;
	P8[PITCH] = 70; I8[PITCH] = 20; D8[PITCH] = 20;
	P8[YAW] = 85; I8[YAW] = 0;  D8[YAW] = 0;
	P8[PIDALT] = 45; I8[PIDALT] = 0;  D8[PIDALT] = 0;
	P8[PIDLEVEL] = 70; I8[PIDLEVEL] = 0; D8[PIDALT] = 0;

	GyroAccCompFilter = 310;
	GyroMagCompFilter = 200;
	AccBaroCompFilter = 200;
	
	rcRate8 = 30; // = 0.9 in GUI
	rcExpo8 = 65;

	rollPitchRate = 0;
	yawRate = 0;
	dynThrPID = 0;

	accTrim[ROLL] = 0; accTrim[PITCH] = 0;
}

void checkFirstTime() {
	uint8_t test_val; 
	eeprom_read_block((void*)&test_val, (void*)(0), sizeof(test_val));

	if (test_val == checkNewConf) {
		PicopterRadio.setDeviceAddress(LocalAddress);
		Serial.println("EEPROM Initialized, Continue startup.");
	}
	else{
		setDefaults();
		writeParams();

		blinkLED(15, 20, 1);

		readEEPROM();

		Serial.println("Initialized EEPROM");

		PicopterRadio.setDeviceAddress(LocalAddress);
		return;
	}
}