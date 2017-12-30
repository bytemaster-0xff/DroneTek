/*
 * snsr_mgmt.c
 *
 *  Created on: Jan 9, 2013
 *      Author: kevinw
 */

#include "sensors/snsr_main.h"

#include "flight/autopilot.h"
#include "flight/ctrl_loop.h"
#include "sensors/itg3200.h"
#include "sensors/mpu60x0.h"
#include "sensors/l3g20.h"
#include "sensors/lsm303_mag.h"
#include "sensors/lsm303_acc.h"
#include "sensors/hmc5883.h"
#include "sensors/lipo_adc.h"
#include "sensors/adxl345.h"
#include "sensors/gps.h"
#include "sensors/sonar.h"
#include "sensors/bmp085.h"
#include "sensors/ms5611.h"
#include "sensors/fusion/complementary.h"
#include "sensors/fusion/kalman.h"

#include "common/twb_serial_eeprom.h"
#include "common/memory_mgmt.h"
#include "twb_eeprom.h"
#include "string.h"
#include "twb_config.h"

Sensor_t *SensorsList;

Sensor_t *SensorsList;

Sensor_t *GyroITG3200Sensor;
Sensor_t *GyroL3GD20Sensor;

Sensor_t *GyroMPU60x0Sensor;
Sensor_t *AccMPU60x0Sensor;

Sensor_t *AccADXL345Sensor;
Sensor_t *AccLSM303Sensor;

Sensor_t *MagHMC5883Sensor;
Sensor_t *MagLSM303Sensor;

Sensor_t *SonarSensor;
Sensor_t *PressureBMP085Sensor;
Sensor_t *PressureMS5611Sensor;

Sensor_t *AltGPSSensor;
Sensor_t *HeadingGPSSensor;
Sensor_t *GeoGPSSensor;

Sensor_t *ADCLipoSensor;

Sensor_t *ComplementarySnsrFusion;
Sensor_t *KalmanSnsrFusion;

Sensor_t *PIDControllerAction;

Sensor_t *AutoPilotAction;

Sensor_t *__addSensor(const char *name,
		uint8_t sensorId,
		uint16_t eepromBase,
		iOpResult_e(*init)(void),
		iOpResult_e(*resetDefaults)(void),
		iOpResult_e (*updateSettings)(uint16_t addr, uint8_t value),
		void (*sendDiag)(void)) {
	Sensor_t *sensor = pm_malloc(sizeof(Sensor_t));
	sensor->Name = (char *)pm_malloc((size_t)strlen(name) + 1);
	strcpy(sensor->Name, name);
	sensor->SensorId = sensorId;
	sensor->EEPROMBaseAddr = eepromBase;
	sensor->Init = init;
	sensor->UpdateSetting = updateSettings;
	sensor->ResetDefaults = resetDefaults;
	sensor->SensorFault = OK;
	sensor->DataReady = DataNotReady;
	sensor->IRQAsserted = IRQNotAsserted;
	sensor->LastUpdate = 0;
	sensor->Status = SensorStatus_Idle;
	sensor->SendDiag = sendDiag;

	sensor->Start = NULL;
	sensor->Pause = NULL;
	sensor->Resume = NULL;
	sensor->Stop = NULL;

	sensor->Next = NULL;

	if(SensorsList == NULL)
		SensorsList = sensor;
	else{
		Sensor_t *tmp = SensorsList;
		while(tmp->Next != NULL)
			tmp = tmp->Next;

		tmp->Next = sensor;
	}

	return sensor;
}


iOpResult_e TWB_InitSensorArray(void){
	GyroITG3200Sensor = __addSensor("ITG3200", GyroITG3200, ITG_EEPROM_OFFSET, &TWB_IMU_ITG3200_Init, &TWB_IMU_ITG3200_ResetDefaults, &TWB_IMU_ITG3200_UpdateSetting, &TWB_IMU_ITG3200_SendDiag);
	GyroMPU60x0Sensor = __addSensor("MPU60x0GYR", GyroMPU60x0, MPU60x0_GYR_EEPROM_OFFSET, &TWB_IMU_MPU60x0_Gyro_Init, &TWB_IMU_MPU60x0_Gyro_ResetDefaults, &TWB_IMU_MPU60x0_Gyro_UpdateSetting, &TWB_IMU_MPU60x0_Gyro_SendDiag);
	GyroL3GD20Sensor = __addSensor("L3GD20", GyroL3GD20, L3GD_EEPROM_OFFSET, &TWB_IMU_L3GD20_Init, &TWB_IMU_L3GD20_ResetDefaults, &TWB_IMU_L3GD20_UpdateSetting, &TWB_IMU_L3GD20_SendDiag);

	AccADXL345Sensor = __addSensor("ADXL345", AccADXL345, ADXL345_EEPROM_OFFSET, &TWB_IMU_ADXL345_Init, &TWB_IMU_ADXL345_ResetDefaults, &TWB_IMU_ADXL345_UpdateSetting, &TWB_IMU_ADXL345_SendDiag);
	AccLSM303Sensor = __addSensor("LSM303_ACC", AccLSM303, LSM303_ACC_EEPROM_OFFSET, &TWB_IMU_LSM303_Acc_Init, &TWB_IMU_LSM303_Acc_ResetDefaults, &TWB_IMU_LSM303_Acc_UpdateSetting, &TWB_IMU_LSM303_Acc_SendDiag);
	AccMPU60x0Sensor = __addSensor("MPU60x0ACC", AccMPU60x0, MPU60x0_ACC_EEPROM_OFFSET, &TWB_IMU_MPU60x0_Acc_Init, &TWB_IMU_MPU60x0_Acc_ResetDefaults, &TWB_IMU_MPU60x0_Acc_UpdateSetting, &TWB_IMU_MPU60x0_Acc_SendDiag);

	MagHMC5883Sensor = __addSensor("HMC5883", MagHMS5883,  HMC5883_EEPROM_OFFSET, &TWB_IMU_HMC5883_Init, &TWB_IMU_HMC5883_ResetDefaults, &TWB_IMU_HMC5883_UpdateSetting, &TWB_IMU_HMC5883_SendDiag);
	MagLSM303Sensor = __addSensor("LSM303_MAG", MagLSM303, LSM303_MAG_EEPROM_OFFSET, &TWB_IMU_LSM303_Mag_Init, &TWB_IMU_LSM303_Mag_ResetDefaults, &TWB_IMU_LSM303_Mag_UpdateSetting, &TWB_IMU_LSM303_Mag_SendDiag);

	ADCLipoSensor = __addSensor("LIPO_ADC", ADCLipo, LIPO_ADC_EEPROM_OFFSET, &TWB_ADC_Init, &TWB_ADC_ResetDefaults, &TWB_ADC_UpdateSetting, &TWB_ADC_SendDiag);
	SonarSensor = __addSensor("SONAR", AltSonar, SONAR_EEPROM_OFFSET, &TWB_IMU_Sonar_Init, &TWB_IMU_Sonar_ResetDefaults, &TWB_IMU_Sonar_UpdateSetting, &TWB_IMU_SONAR_SendDiag);
	PressureBMP085Sensor = __addSensor("BMP085", AltBMP085, BMP085_EEPROM_OFFSET, &TWB_IMU_BMP085_Init, &TWB_IMU_BMP085_ResetDefaults, &TWB_IMU_BMP085_UpdateSetting, &TWB_IMU_BMP085_SendDiag);
	PressureMS5611Sensor = __addSensor("MS5611", AltMS5611, MS5611_EEPROM_OFFSET, &TWB_IMU_MS5611_Init, &TWB_IMU_MS5611_ResetDefaults, &TWB_IMU_MS5611_UpdateSetting, &TWB_IMU_MS5611_SendDiag);

	AltGPSSensor = __addSensor("GPS_ALT", AltGPS, GPS_ALT_EEPROM_OFFSET, &TWB_GPS_Alt_Init, &TWB_GPS_Alt_ResetDefaults, &TWB_GPS_Alt_Update_Setting, NULL);
	HeadingGPSSensor = __addSensor("GPS_HDNG", MagGPS, GPS_HEADING_EEPROM_OFFSET, &TWB_GPS_Heading_Init, &TWB_GPS_Heading_ResetDefaults, &TWB_GPS_Heading_Update_Setting, NULL);
	GeoGPSSensor = __addSensor("GPS_GEO", GeoGPS, GPS_GEO_EEPROM_OFFSET, &TWB_GPS_Geo_Init, &TWB_GPS_Geo_ResetDefaults, &TWB_GPS_Geo_Update_Setting, NULL);

	ComplementarySnsrFusion = __addSensor("FUS_COMPL", SnsrFusionComplementary, COMPLEMENTARY_EEPROM_OFFSET, &TWB_SNSR_Complementary_Init, &TWB_SNSR_Complementary_ResetDefaults, &TWB_SNSR_Complementary_UpdateSettings, &TWB_SNSR_Complementary_SendDiag);
	KalmanSnsrFusion = __addSensor("FUS_KAL", SnsrFusionKalman, KALMAN_EEPROM_OFFSET, &TWB_SNSR_Kalman_Init, &TWB_SNSR_Kalman_ResetDefaults, &TWB_SNSR_Kalman_UpdateSetting, &TWB_SNSR_Kalman_SendDiag);

	PIDControllerAction = __addSensor("PID_CTRLR", PIDController, PID_EEPROM_OFFSET, &TWB_GPIO_CtrlLoop_Init, &TWB_GPIO_CtrlLoop_ResetDefaults, NULL, NULL);

	AutoPilotAction = __addSensor("AUTOPILOT", AutoPilot, AUTO_PILOT_OFFSET, &TWB_AutoPilot_Init, &TWB_AutoPilot_ResetDefaults, TWB_AutoPilot_UpdateSettings, NULL);

	return OK;
}
