/*
 * twb_eeprom.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef TWB_EEPROM_H_
#define TWB_EEPROM_H_

#define EEPROM_I2C_ADDR 0x50

#define SENSOR_BLOCK_EEPROM_SIZE 	0x0040

#define ITG_EEPROM_OFFSET 			0x0020
#define LSM303_ACC_EEPROM_OFFSET 	0x0060
#define LSM303_MAG_EEPROM_OFFSET 	0x00A0
#define L3GD_EEPROM_OFFSET 			0x00E0
#define BMP085_EEPROM_OFFSET 		0x0120
#define ADXL345_EEPROM_OFFSET 		0x0160
#define HMC5883_EEPROM_OFFSET 		0x01A0
#define SONAR_EEPROM_OFFSET 		0x01E0
#define LIPO_ADC_EEPROM_OFFSET 		0x0220
#define MPU60x0_GYR_EEPROM_OFFSET 	0x0260
#define MPU60x0_ACC_EEPROM_OFFSET 	0x02A0
#define GPS_HEADING_EEPROM_OFFSET 	0x02E0
#define GPS_ALT_EEPROM_OFFSET 		0x0320
#define GPS_GEO_EEPROM_OFFSET 		0x0360
#define MS5611_EEPROM_OFFSET 		0x0380
#define KALMAN_EEPROM_OFFSET 		0x03A0
#define COMPLEMENTARY_EEPROM_OFFSET 0x03E0

#define SENSOR_CFG_EEPROM_OFFSET 	0x0600
#define PID_EEPROM_OFFSET 			0x0700
#define DATA_TX_INTVL_EEPROM_OFFSET 0x0800
#define COMMON_EEPROM_OFFSET 		0x0900
#define PWM_IN_CALIBRATION 			0x0A00
#define AUTO_PILOT_OFFSET 			0x0B00

#define SYSTEM_AREA_OFFSET 			0x1000 /* Allocate 1K for System Values */

#define END_SYSTEM_AREA_OFFSET      0x13FF




#endif /* TWB_EEPROM_H_ */