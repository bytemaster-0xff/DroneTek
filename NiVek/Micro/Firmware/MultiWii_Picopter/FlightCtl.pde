#define CMD_StableMode	50
#define CMD_AcroMode	51

#define CMD_Kill 60

#define CMD_ArmNiVek 70
#define CMD_SafeNiVek 71

#define CMD_LaunchNiVek 80
#define CMD_LandNiVek 81

#define CMD_ReadAllPIDConstants 90
#define CMD_GetPIDConstant 91
#define CMD_SetPIDConstant 92
#define CMD_ResetPIDValues 93
#define CMD_GetConfigValue 94
#define CMD_SetConfigValue 95

#define CMD_UseRC 100
#define CMD_UseWiFi 101

#define CMD_Set_AltitudeHold 105
#define CMD_Clear_AltitudeHold 106

#define CMD_SetAltitude 107
#define CMD_SetHeading 108

#define CMD_ThrottleYawRollThrottle 110

#define CMD_Set_ESC_Power 120

#define CMD_Start_RC_CalibrationMode 130
#define CMD_CalibrateRCChanncel 131
#define CMD_Cancel_RC_CalibrationMode 132
#define CMD_End_RC_CalibrationMode 133

#define CMD_StartMotorConfig 140
#define CMD_EndMotorConfig 141

#define CMD_StartPIDConfig 150
#define CMD_EndPIDConfig 151


static int16_t errorGyroI[3] = { 0, 0, 0 };
static int16_t errorAngleI[2] = { 0, 0 };

void armNiVek(NiVekMessage *msg){
	Serial.write("Putting NiVek in Armed Mode\r\n");
	
	IsArmed = Armed;
	if (msg != null)
		sendAck(msg);
}

void safeNiVek(NiVekMessage *msg){
	if (IsArmed == Armed){
		IsArmed = Safe;
		if (msg != null)
			sendAck(msg);

		Serial.write("Putting NiVek in Safe Mode\r\n");
	}
}

void setConfigValue(NiVekMessage *msg){

}

void getConfigValue(NiVekMessage *msg){

}

void handleFlightMessage(NiVekMessage *msg){	
	Serial.println(msg->TypeId);
	switch (msg->TypeId){
		case CMD_Set_ESC_Power: setMotorPower(msg); break;
		case CMD_AcroMode: turnOffStableMode(msg); break;
		case CMD_StableMode: turnOnStableMode(msg); break;
		case CMD_StartMotorConfig: beginMotorConfig(msg); break;
		case CMD_EndMotorConfig: endMotorConfig(msg); break;
		case CMD_ReadAllPIDConstants: sendConfiguration(msg->SourceAddress);  break;
		case CMD_SetPIDConstant: setPIDValue(msg); break;
		case CMD_GetPIDConstant: getPIDValue(msg);  break;
		case CMD_SetConfigValue: setConfigValue(msg); break;
		case CMD_GetConfigValue: getConfigValue(msg); break;
		case CMD_ArmNiVek:
			if (getOkToArm())
				armNiVek(msg);
			else
				sendNak(msg);
			break;

		case CMD_SafeNiVek:
			safeNiVek(msg);
		break;
	}
}

void clearIntegral(){
	errorGyroI[ROLL] = 0; 
	errorGyroI[PITCH] = 0; 
	errorGyroI[YAW] = 0;

	errorAngleI[ROLL] = 0; 
	errorAngleI[PITCH] = 0;
}

void onTheGround(){
	return;

	if (rcData[THROTTLE] < MINCHECK){
		static uint8_t rcDelayCommand;

		uint16_t modeChan;
		modeChan = pcr_rcChan[4];

		if (rcData[THROTTLE] < MINCHECK)
			clearIntegral();

		if (modeChan & (1 << PCC_ZEROSENSORS))
			clearIntegral();

		rcDelayCommand++;

		if (rcData[YAW] < MINCHECK && rcData[PITCH] < MINCHECK && IsArmed == Safe) {
			if (rcDelayCommand == 20)
				beginSensorZero(null);
		}		
		else
			rcDelayCommand = 0;
	}
}


void turnOnStableMode(NiVekMessage *msg){
	Serial.write("Turn on stable mode.");
	FlightMode = FlightModeStable;
	sendAck(msg);	
}

void turnOffStableMode(NiVekMessage *msg){
	Serial.write("Enter acrobat mode.");
	FlightMode = FlightModeAcrobat;

	clearIntegral();

	sendAck(msg);
}
