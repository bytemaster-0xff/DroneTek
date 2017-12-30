#ifndef DEF_H
#define DEF_H

#define DRONE_NAME_LENGTH 32

#define I2C_PULLUPS_ENABLE         PORTD |= 1<<0; PORTD |= 1<<1;   // PIN A4&A5 (SDA&SCL)
#define I2C_PULLUPS_DISABLE        PORTD &= ~(1<<0); PORTD &= ~(1<<1);
#define ISR_UART                   ISR(USART0_UDRE_vect)
#define ISR_TWI					   ISR(TWI_vect_num);

#define ACC_ORIENTATION(X, Y, Z)  {imu.accADC[ROLL]  =  -Y; imu.accADC[PITCH]  = X; imu.accADC[YAW]  = Z;}
#define GYRO_ORIENTATION(X, Y, Z) {imu.gyroADC[ROLL] =  -Y; imu.gyroADC[PITCH] = X; imu.gyroADC[YAW] = Z;}

#define GYRO_CALIBRATION_SAMPLES 400
#define ACC_CALIBRATION_SAMPLES 400

#define null 0

#define ACC 1
#define MAG 1
#define GYRO 1
#define BARO 1
#define GPSPRESENT 0

#define ROLL       0
#define PITCH      1
#define YAW        2
#define THROTTLE   3
#define AUX1       4
#define AUX2       5
#define CAMPITCH   6
#define CAMROLL    7

#define PIDALT     3
#define PIDLEVEL   4

#define MULTITYPE 3

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


typedef struct  {
	int32_t X, Y, Z;
} t_int32_t_vector_def;

typedef struct  {
	uint16_t XL; int16_t X;
	uint16_t YL; int16_t Y;
	uint16_t ZL; int16_t Z;
} t_int16_t_vector_def;

typedef union {
	int32_t A32[3];
	t_int32_t_vector_def V32;
	int16_t A16[6];
	t_int16_t_vector_def V16;
} t_int32_t_vector;

#endif