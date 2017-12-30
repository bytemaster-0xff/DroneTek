

#define I2C_CMD 0x55


#define NO_OP 0x00            // does nothing
#define PULLB_ON 0x01         // PORTB pull-ups
#define PULLB_OFF 0x02
#define GET_AD0 0x03          // Read A/D channels
#define GET_AD1 0x04
#define GET_AD2 0x05
#define GET_AD3 0x06
#define GET_AD4 0x07
#define GET_S4A 0x08          // SRF04 - Trig on RB0, Echo on RB2
#define GET_S4B 0x09          // SRF04 - Trig on RB6, Echo on RB7
#define GET_S5A 0x0A          // SRF05 - Trig + Echo on RB6
#define GET_S5B 0x0B          // SRF05 - Trig + Echo on RB7
#define SET_US 0x0C           // sonar ranging in uS
#define SET_CM 0x0D           // sonar ranging in cm
#define SET_IN 0x0E           // sonar ranging in inches

#define GPIO_ADDR 0x40        // Factory supplied default I2C address
#define CMD 0                 // write only - command register
#define REVISION 0            // read only - firmware revision
#define MASK_A 1              // write only - TRISA reg
#define RESULT 1              // read only - result of A/D
#define MASK_B 2              // write only - TRISB reg
#define AD_CONTROL 3          // read/write - ADCON1
#define PORT_A 4              // read/write
#define PORT_B 5              // read/write
#define PWM 6                 // read/write
#define ADDR_CHANGE 7         // write only

void initGPIO4() {

}

#define GPIO14 0x48

void writeGPIO4(byte reg, byte val){
	Wire.beginTransmission(GPIO14);
	Wire.write(reg); 
	Wire.write(val);
	Wire.endTransmission();	
}

byte readByteGPIO4(byte reg)
{
	Wire.beginTransmission(GPIO14);
	Wire.requestFrom(GPIO14, 1);
	Wire.endTransmission();

	return 0;
}

byte readShortGPIO4(byte reg)
{
	char buffer[2];
	Wire.beginTransmission(0x40);
	Wire.requestFrom(GPIO14, 2);
	Wire.endTransmission();

	return (buffer[0] << 8) | buffer[1];
}


