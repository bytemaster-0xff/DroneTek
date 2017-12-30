
// ************************************************************************************************************

// I2C Barometer MS561101BA

// ************************************************************************************************************

//

// specs are here: http://www.meas-spec.com/downloads/MS5611-01BA03.pdf

// useful info on pages 7 -> 12



// registers of the device

#define MS561101BA_ADDRESS 0x77 //CBR=0 0xEE I2C address when pin CSB is connected to LOW (GND)


#define MS561101BA_PRESSURE    0x40
#define MS561101BA_TEMPERATURE 0x50
#define MS561101BA_RESET       0x1E



// OSR (Over Sampling Ratio) constants

#define MS561101BA_OSR_256  0x00
#define MS561101BA_OSR_512  0x02
#define MS561101BA_OSR_1024 0x04
#define MS561101BA_OSR_2048 0x06

#define MS561101BA_OSR_4096 0x08

#define OSR MS561101BA_OSR_4096

int32_t baroPressure;
int32_t baroPressureSum;
int32_t baroTemperature;


static struct {
	uint16_t c[7];

	union { uint32_t val; uint8_t raw[4]; } ut; //uncompensated T
	union { uint32_t val; uint8_t raw[4]; } up; //uncompensated P

	uint8_t  state;
	uint32_t deadline;

} ms561101ba_ctx;



void i2c_MS561101BA_reset(){
	i2c_writeReg(MS561101BA_ADDRESS, MS561101BA_RESET, 0);
}



void i2c_MS561101BA_readCalibration(){
	i2c_errors_count = 0;

	union { uint16_t val; uint8_t raw[2]; } data;

	for (uint8_t i = 0; i<6; i++) {
		i2c_rep_start(MS561101BA_ADDRESS << 1);
		i2c_write(0xA2 + 2 * i);
		delay(10);
		i2c_rep_start((MS561101BA_ADDRESS << 1) | 1);//I2C read direction => 1
		delay(10);
		data.raw[1] = i2c_readAck();  // read a 16 bit register
		data.raw[0] = i2c_readNak();
		ms561101ba_ctx.c[i + 1] = data.val;
	}
}



bool  Baro_init() {
	i2c_errors_count == 0;

	delay(10);

	i2c_MS561101BA_reset();

	delay(100);

	i2c_MS561101BA_readCalibration();

	delay(10);

	i2c_MS561101BA_UT_Start();

	ms561101ba_ctx.deadline = getCurrentTime() + 10000;

	return (i2c_errors_count == 0);
}

// read uncompensated temperature value: send command first

void i2c_MS561101BA_UT_Start() {
	i2c_rep_start(MS561101BA_ADDRESS << 1);      // I2C write direction
	i2c_write(MS561101BA_TEMPERATURE + OSR);  // register selection
	i2c_stop();
}



// read uncompensated pressure value: send command first

void i2c_MS561101BA_UP_Start() {
	i2c_errors_count = 0;

	i2c_rep_start(MS561101BA_ADDRESS << 1);      // I2C write direction
	i2c_write(MS561101BA_PRESSURE + OSR);     // register selection
	i2c_stop();
}



// read uncompensated pressure value: read result bytes

void i2c_MS561101BA_UP_Read() {

	i2c_rep_start(MS561101BA_ADDRESS << 1);

	i2c_write(0);

	i2c_rep_start((MS561101BA_ADDRESS << 1) | 1);

	ms561101ba_ctx.up.raw[2] = i2c_readAck();

	ms561101ba_ctx.up.raw[1] = i2c_readAck();

	ms561101ba_ctx.up.raw[0] = i2c_readNak();
}



// read uncompensated temperature value: read result bytes

void i2c_MS561101BA_UT_Read() {
	i2c_errors_count = 0;

	i2c_rep_start(MS561101BA_ADDRESS << 1);

	i2c_write(0);

	i2c_rep_start((MS561101BA_ADDRESS << 1) | 1);

	ms561101ba_ctx.ut.raw[2] = i2c_readAck();

	ms561101ba_ctx.ut.raw[1] = i2c_readAck();

	ms561101ba_ctx.ut.raw[0] = i2c_readNak();

	if (i2c_errors_count > 0)
		Serial.print("Error talking to MS5611.");
}



void i2c_MS561101BA_Calculate() {

	int32_t off2, sens2, delt;



	int64_t dT = (int32_t) ms561101ba_ctx.ut.val - ((int32_t) ms561101ba_ctx.c[5] << 8);

	baroTemperature = 2000 + ((dT * ms561101ba_ctx.c[6]) >> 23);

	int64_t off = ((uint32_t) ms561101ba_ctx.c[2] << 16) + ((dT * ms561101ba_ctx.c[4]) >> 7);

	int64_t sens = ((uint32_t) ms561101ba_ctx.c[1] << 15) + ((dT * ms561101ba_ctx.c[3]) >> 8);



	if (baroTemperature < 2000) { // temperature lower than 20st.C 

		delt = baroTemperature - 2000;

		delt = 5 * delt*delt;

		off2 = delt >> 1;

		sens2 = delt >> 2;

		if (baroTemperature < -1500) { // temperature lower than -15st.C

			delt = baroTemperature + 1500;

			delt = delt*delt;

			off2 += 7 * delt;

			sens2 += (11 * delt) >> 1;

		}

		off -= off2;

		sens -= sens2;

	}



	baroPressure = (((ms561101ba_ctx.up.val * sens) >> 21) - off) >> 15;	
}



//return 0: no data available, no computation ;  1: new value available  ; 2: no new value, but computation time

uint8_t Baro_Update() {                            // first UT conversion is started in init procedure

	if (getCurrentTime() < ms561101ba_ctx.deadline) return 0;

	ms561101ba_ctx.deadline = getCurrentTime() + 10000;  // UT and UP conversion take 8.5ms so we do next reading after 10ms 

	TWBR = ((F_CPU / 400000L) - 16) / 2;          // change the I2C clock rate to 400kHz, MS5611 is ok with this speed

	if (ms561101ba_ctx.state == 0) {
		i2c_MS561101BA_UT_Read();
		i2c_MS561101BA_UP_Start();

		Baro_Common();                              // moved here for less timecycle spike

		ms561101ba_ctx.state = 1;

		return 1;

	}
	else {
		i2c_MS561101BA_UP_Read();
		i2c_MS561101BA_UT_Start();
		i2c_MS561101BA_Calculate();

		ms561101ba_ctx.state = 0;

		return 2;
	}

}


#define BARO_TAB_SIZE   21

void Baro_Common() {
	static int32_t baroHistTab[BARO_TAB_SIZE];
	static uint8_t baroHistIdx;

	uint8_t indexplus1 = (baroHistIdx + 1);

	if (indexplus1 == BARO_TAB_SIZE) indexplus1 = 0;

	baroHistTab[baroHistIdx] = baroPressure;



	baroPressureSum += baroHistTab[baroHistIdx];
	baroPressureSum -= baroHistTab[indexplus1];
	baroHistIdx = indexplus1;
}


