static uint32_t neutralizeTime = 0;


/*ISR_TWI {

}

*/
void i2c_init(void) {
#if defined(INTERNAL_I2C_PULLUPS)
	I2C_PULLUPS_ENABLE
#else
	I2C_PULLUPS_DISABLE
#endif
		TWSR = 0;                                    // no prescaler => prescaler = 1
	TWBR = ((16000000L / I2C_SPEED) - 16) / 2;   // change the I2C clock rate
	TWCR = 1 << TWEN;                              // enable twi module, no interrupt
}

void i2c_rep_start(uint8_t address) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // send REPEAT START condition
	waitTransmissionI2C();                       // wait until transmission completed
	TWDR = address;                              // send device address
	TWCR = (1 << TWINT) | (1 << TWEN);
	waitTransmissionI2C();                       // wail until transmission completed
}

void i2c_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	//  while(TWCR & (1<<TWSTO));                // <- can produce a blocking state with some WMP clones
}

void i2c_write(uint8_t data) {
	TWDR = data;                                 // send data to the previously addressed device
	TWCR = (1 << TWINT) | (1 << TWEN);
	waitTransmissionI2C();
}

uint8_t i2c_readAck() {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	waitTransmissionI2C();
	return TWDR;
}

uint8_t i2c_readNak(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	waitTransmissionI2C();
	uint8_t r = TWDR;
	i2c_stop();
	return r;
}

void waitTransmissionI2C() {
	uint16_t count = 255;
	while (!(TWCR & (1 << TWINT))) {
		count--;
		if (count == 0) {              //we are in a blocking state => we don't insist
			TWCR = 0;                  //and we force a reset on TWINT register
			neutralizeTime = micros(); //we take a timestamp here to neutralize the value during a short delay
			i2c_errors_count++;
			break;
		}
	}
}

void i2c_getSixRawADC(uint8_t add, uint8_t reg) {
	i2c_rep_start(add);
	i2c_write(reg);         // Start multiple read at the reg register
	i2c_rep_start(add + 1);  // I2C read direction => I2C address + 1
	for (uint8_t i = 0; i < 5; i++)
		rawADC[i] = i2c_readAck();
	rawADC[5] = i2c_readNak();
}

void i2c_writeReg(uint8_t add, uint8_t reg, uint8_t val) {
	i2c_rep_start(add + 0);  // I2C write direction
	i2c_write(reg);        // register selection
	i2c_write(val);        // value to write in register
	i2c_stop();
}

uint8_t i2c_readReg(uint8_t add, uint8_t reg) {
	i2c_rep_start(add + 0);  // I2C write direction
	i2c_write(reg);        // register selection
	i2c_rep_start(add + 1);  // I2C read direction
	return i2c_readNak();  // Read single register and return value
}
