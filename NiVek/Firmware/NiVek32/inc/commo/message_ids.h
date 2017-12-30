/*
 * message_ids.h
 *
 *  Created on: Mar 20, 2013
 *      Author: Kevin
 */

#ifndef MESSAGE_IDS_H_
#define MESSAGE_IDS_H_

/* Module ids */
typedef enum {
	MOD_Sensor = 70,
    MOD_FlightControls = 80,
    MOD_WiFi = 90,
    MOD_System = 100
} TWB_Modules_t;
/* End Module Ids */

#define CMD_Kill 60

#define CMD_ArmNiVek 70
#define CMD_SafeNiVek 71

#define CMD_ArmESCs 72

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
#define CMD_Set_Altitude 107
#define CMD_Set_Heading 108

#define CMD_ThrottleYawRollThrottle 110

#define CMD_Set_ESC_Power 120

#define CMD_Start_RC_Calibration 130
#define CMD_CalibrateRCChannel 131
#define CMD_Cancel_RC_Calibration 132
#define CMD_End_RC_Calibration 133

#define CMD_PWMInData 135
#define CMD_RCChannel 136
#define CMD_CalibrationCompleted 137

#define CMD_StartMotorConfig 140
#define CMD_EndMotorConfig 141

#define CMD_StartPIDConfig 150
#define CMD_EndPIDConfig 151

#define CMD_SensorZero 0x81
#define CMD_SensorStart 0x90
#define CMD_SensorStop 0x91
#define CMD_SensorRestart 0x92
#define CMD_SensorRestoreDefaults 0x93

#define CMD_BeginSensorDiagnostics 0x94
#define CMD_EndSensorDiagnostics 0x95

#define CMD_BeginSensorFusionDiag 0xB0
#define CMD_EndSensorFusionDiag 0xB1
#define CMD_ReadSensorFusionConfig 0xB2

#define CMD_BeginMagCalibration 0xA0
#define CMD_CancelMagCalibration 0xA2
#define CMD_EndMagCalibration 0xA3

#define CMD_SetCfg_Value 0xC0
#define CMD_GetCfg_Values 0xC1

#define MSG_SnsrsConfigData			0x30
#define MSG_SetConfigData			0x31
#define MSG_SetDiagnostics			0x32
#define MSG_SnsrFusion	    		0x33
#define MSG_SnsrFusionConfig 		0x34
#define MSG_InvalidConfigOpreation	0x35

#define MSG_SnsrStarted				0x70
#define MSG_SnsrStopped				0x71

#define MSG_GpsData 				0x11
#define MSG_BATTERY 				0x12
#define MSG_SYSTEM_STATUS_ID 		0x13
#define MSG_SYSTEM_TARGETS_ID 		0x14
#define MSG_SYSTEM_MOTOR_STATUS_ID  0x16
#define MSG_ALTITUDE_DATA_ID  		0x17

#define MSG_SnsrData				0x80
#define MSG_PidSnsrData				0x81
#define MSG_PidSnsrDataDetails		0x82
#define MSG_SnsrTimeout				0xA0
#define MSG_SnsrFailure				0xA1

#define RC_IN_ThrottleMin 1
#define RC_IN_ThrottleMax 2

#define RC_IN_Pitch_Up 10
#define RC_IN_Pitch_Idle 11
#define RC_IN_Pitch_Down 12

#define RC_IN_Roll_Left 20
#define RC_IN_Roll_Idle 21
#define RC_IN_Roll_Right 22

#define RC_IN_Yaw_Port 30
#define RC_IN_Yaw_Idle 31
#define RC_IN_Yaw_Starboard 32

#endif /* MESSAGE_IDS_H_ */
