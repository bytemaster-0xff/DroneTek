uint8_t rawADC[6];

#if !defined(ACC_ORIENTATION) 
#define ACC_ORIENTATION(X, Y, Z)  {accADC[ROLL]  = X; accADC[PITCH]  = Y; accADC[YAW]  = Z;}
#endif
#if !defined(GYRO_ORIENTATION) 
#define GYRO_ORIENTATION(X, Y, Z) {gyroADC[ROLL] = X; gyroADC[PITCH] = Y; gyroADC[YAW] = Z;}
#endif

#define MPU60x0_I2CADDR			0xD0
#define MPU60x0_ACC_REG_ADDR	0x3B
#define MPU60x0_GYRO_REG_ADDR	0x43


bool MPU60x0_ReadAcc() {
	i2c_getSixRawADC(MPU60x0_I2CADDR, MPU60x0_ACC_REG_ADDR);
	return (i2c_errors_count == 0);
}

bool MPU60x0_ReadGyro() {
	i2c_getSixRawADC(MPU60x0_I2CADDR, MPU60x0_GYRO_REG_ADDR);
	return (i2c_errors_count == 0);
}


bool MPU60x0_init() {
	i2c_writeReg(MPU60x0_I2CADDR, 0x6B, 0x80); // reset
	delay(100);
	i2c_writeReg(MPU60x0_I2CADDR, 0x6B, 0x03); // clk sel PLL gyro Z
	i2c_writeReg(MPU60x0_I2CADDR, 0x19, 0x04); // sample rate 200Hz
	i2c_writeReg(MPU60x0_I2CADDR, 0x1A, 0x03); // DLPF 42Hz
	i2c_writeReg(MPU60x0_I2CADDR, 0x1B, 0x18); // gyro scale 2000 deg/s
	i2c_writeReg(MPU60x0_I2CADDR, 0x1C, 0x00); // accel scale 2g

	return (i2c_errors_count == 0);
}