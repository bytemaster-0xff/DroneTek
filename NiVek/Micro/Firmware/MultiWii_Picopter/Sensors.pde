
// ****************
// GYRO common part
// ****************

#define CMD_SensorZero 0x81
#define CMD_SensorStart 0x90
#define CMD_SensorStop 0x91
#define CMD_SensorRestart 0x92
#define CMD_SensorResetDefaults 0x93
#define CMD_BeginSensorDiagnostics 0x94
#define CMD_EndSensorDiagnostics 0x95

#define CMD_BeginMagCalibrationXY 0xA0
#define CMD_BeginMagCalibrationZ 0xA1
#define CMD_CancelMagCalibration 0xA2
#define CMD_EndMagCalibration 0xA3

#define CMD_BeginSensorFusionDiag 0xB0
#define CMD_EndSensorFusionDiag 0xB1
#define CMD_ReadSensorFusionConfig 0xB2

#define CMD_SetCfg_Value 0xC0
#define CMD_ReadCfgValues 0xC1
#define CMD_ResetCfg_Value 0xC2

#define SENSOR_CFG_EEPROM_OFFSET 0x80
#define SENSOR_CFG_TIME_CONSTANT SENSOR_CFG_EEPROM_OFFSET + 1;
#define SENSOR_CFG_ACC_LPF SENSOR_CFG_EEPROM_OFFSET + 2;


void transitionToSensorState(SensorStates newState) {
	SensorState = newState;
}


bool initSensors() {
	Serial.print("Starting Sensors: ");

	delay(300);
	i2c_init();
	delay(100);

	if (MPU60x0_init())
		Serial.write("MPU60x0 ");
	else{
		Serial.println("MPU60x0 Not Found!");
		SensorState = SnsrOffline;
		return false;
	}

	/*if (Baro_init())
		Serial.write("MS5611 ");
	else{
		Serial.println("\r\nMS5611 Not Found!");
		return false;
	}*/

	/*if (Mag_init())
		Serial.write("HMC5881 ");
	else{
		Serial.println("\r\nHMC5881 Not Found!");
		return false;
	}
	*/

	beginSensorZero(null);

	acc_1G = 512;
	acc_25deg = acc_1G * 0.423;  

	SensorState = SnsrOnline;

	return true;
}


void sensorHandler() {
	ACC_Read();
	GYRO_Read();
	//Baro_Update();

	if (SensorState == SnsrZeroCompleted)
		SensorState = SnsrOnline;
}

void updateSensorValue(NiVekMessage *msg){
	SensorIds_t sensor = (SensorIds_t)msg->MsgBuffer[0];
	Serial.write("Update Sensor ->");
	Serial.println((int) msg->MsgBuffer[0]);
	switch (sensor){
		case SensorIds_IMU_Attitude:
		case SensorIds_IMU_Heading:
		case SensorIds_IMU_Altitude:
			updateCompFilterSetting(msg);
		break;
	}
}


void handleSensorMessage(NiVekMessage *msg){
	switch (msg->TypeId){
	case CMD_SensorZero: beginSensorZero(msg); break;
	case CMD_SetCfg_Value: updateSensorValue(msg); break;

	case CMD_BeginSensorFusionDiag:
		switch ((AxisTypes_t)msg->MsgBuffer[0]){
			case AxisType_Pitch: transitionToSensorState(SnsrFusionPitch); break;
			case AxisType_Roll: transitionToSensorState(SnsrFusionRoll); break;
			case AxisType_Heading: transitionToSensorState(SnsrFusionHeading); break;
			case AxisType_Altitude: transitionToSensorState(SnsrFusionAltitude); break;
		}
		sendAck(msg);
		break;
	case CMD_EndSensorFusionDiag:
		transitionToSensorState(SnsrOnline);
		sendAck(msg);
		break;
	case CMD_ReadSensorFusionConfig: sendCompFilterConfig(msg); break;
	}
}
