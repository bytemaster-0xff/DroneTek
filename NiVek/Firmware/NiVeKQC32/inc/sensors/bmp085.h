#ifndef __BMP085_H__
#define __BMP085_H__


#include "common/twb_common.h"

//TODO: Really need to spend some time to understand pulling BMP085 values, a little bit of a mistery right now

#define E_BMP_NULL_PTR		    (char)-127
#define E_BMP_COMM_RES		    (char)-1
#define E_BMP_OUT_OF_RANGE		(char)-2
#define E_SENSOR_NOT_DETECTED   (char) 0


#define BMP085_CHIP_ID_REG			0xD0
#define BMP085_VERSION_REG			0xD1

#define BMP085_CTRL_MEAS_REG		0xF4
#define BMP085_ADC_OUT_MSB_REG		0xF6
#define BMP085_ADC_OUT_LSB_REG		0xF7

#define BMP085_SOFT_RESET_REG		0xE0

#define BMP085_T_MEASURE        0x2E				// temperature measurent 
#define BMP085_P_MEASURE        0x34				// pressure measurement

#define BMP085_TEMP_CONVERSION_TIME  5				// TO be spec'd by GL or SB


/* SMD500 specific constants */


#define SMD500_PROM_START__ADDR		0xf8
#define SMD500_PROM_DATA__LEN		8

#define SMD500_PARAM_M1     -2218        //calibration parameter
#define SMD500_PARAM_M2      -457        //calibration parameter
#define SMD500_PARAM_M3     -1984        //calibration parameter
#define SMD500_PARAM_M4      8808        //calibration parameter
#define SMD500_PARAM_M5       496        //calibration parameter
#define SMD500_PARAM_M6      1415        //calibration parameter

#define SMD500_PARAM_MB     -4955        //calibration parameter
#define SMD500_PARAM_MC     11611        //calibration parameter
#define SMD500_PARAM_MD    -12166        //calibration parameter
#define SMD500_PARAM_ME    -17268        //calibration parameter
#define SMD500_PARAM_MF     -8970        //calibration parameter

#define SMD500_PARAM_MG      3038        //calibration parameter
#define SMD500_PARAM_MH     -7357        //calibration parameter
#define SMD500_PARAM_MI      3791        //calibration parameter
#define SMD500_PARAM_MJ     64385        //calibration parameter

#define SMD500_STANDBY          0   				// set the device in stand-by modus to reduce power consumption
#define SMD500_MASTERCLOCK_32768HZ    0x04		   		// external Master clock 32.768kHz
#define SMD500_MASTERCLOCK_1MHZ       0 				// external Master clock 1MHz
#define SMD500_T_RESOLUTION_13BIT     0				// 13 Bit resolution temperature
#define SMD500_T_RESOLUTION_16BIT     0x80				// 16 Bit resolution temperature
#define SMD500_T_MEASURE              0x6A				// temperature measurent 
#define SMD500_P_MEASURE              0xF0				// pressure measurement

#define SMD500_TEMP_CONVERSION_TIME_13    9
#define SMD500_TEMP_CONVERSION_TIME_16	  34
 

/* register write and read delays */

#define BMP085_MDELAY_DATA_TYPE	unsigned int
#define BMP085_MDELAY_RETURN_TYPE  void

/** this structure holds all device specific calibration parameters 
*/
typedef struct {
   short ac1;
   short ac2;
   short ac3;
   unsigned short ac4;
   unsigned short ac5;
   unsigned short ac6;
   short b1;
   short b2;
   short mb;
   short mc;
   short md;      		   
} BMP085Calib_t;



#define BMP085_CHIP_ID__POS		0
#define BMP085_CHIP_ID__MSK		0xFF
#define BMP085_CHIP_ID__LEN		8
#define BMP085_CHIP_ID__REG		BMP085_CHIP_ID_REG


#define BMP085_ML_VERSION__POS		0
#define BMP085_ML_VERSION__LEN		4
#define BMP085_ML_VERSION__MSK		0x0F
#define BMP085_ML_VERSION__REG		BMP085_VERSION_REG



#define BMP085_AL_VERSION__POS  	4
#define BMP085_AL_VERSION__LEN  	4
#define BMP085_AL_VERSION__MSK		0xF0
#define BMP085_AL_VERSION__REG		BMP085_VERSION_REG

#define BMP085_GET_BITSLICE(regvar, bitname)\
			(regvar & bitname##__MSK) >> bitname##__POS


#define BMP085_SET_BITSLICE(regvar, bitname, val)\
		  (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  


short bmp085_get_temperature(unsigned long ut);

iOpResult_e bmp085_get_cal_param(void);
iOpResult_e smd500_get_cal_param(void);

iOpResult_e TWB_IMU_BMP085_Init(void);
iOpResult_e TWB_IMU_BMP085_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_IMU_BMP085_ResetDefaults(void);
iOpResult_e TWB_IMU_BMP085_Zero(uint8_t sampleSize, float pauseMS);
iOpResult_e TWB_IMU_BMP085_IRQ(void);
iOpResult_e TWB_IMU_BMP085_ProcessData(void);
iOpResult_e TWB_IMU_BBMP085_GetTemperature(int16_t *result);
iOpResult_e TWB_IMU_BMP085_Read(float deltaT);

iOpResult_e TWB_IMU_BMP085_Start(void);
iOpResult_e TWB_IMU_BMP085_Stop(void);
iOpResult_e TWB_IMU_BMP085_Pause(void);
iOpResult_e TWB_IMU_BMP085_Resume(void);


void TWB_IMU_BMP085_SendDiag(void);

#endif   // __BMP085_H__





