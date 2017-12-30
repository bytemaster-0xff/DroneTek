

#include "sensors/snsr_main.h"
#include "string.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "twb_eeprom.h"
#include "sensors/lsm303_mag.h"
#include "flight/ctrl_loop.h"

extern Sensor_t *__sensorInFocus;
extern Sensor_t *SensorsList;

iOpResult_e __sensorsUpdateSensorSetting(uint16_t addr, uint8_t value){
	/* Normally we need to wait 5MS between, between serial eeprom write */
	assert_succeed(TWB_SE_WriteU8(addr, value));

	TWB_Debug_Print2Int("Update setting: ", addr, value);

	Sensor_t *snsr = SensorsList;
	while(snsr != NULL){
		if(addr >= snsr->EEPROMBaseAddr && addr < snsr->EEPROMBaseAddr + SENSOR_BLOCK_EEPROM_SIZE)
			return snsr->UpdateSetting(addr - snsr->EEPROMBaseAddr, value);

		snsr = snsr->Next;
	}

	//If we made it hear, we are updating the common values on the
	//sensor configuration.
	switch(addr){
		case (SENSOR_CFG_EEPROM_OFFSET + 0):
			SensorConfig->ControlLoopUpdateRateHz = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 0:
			SensorConfig->SensorSendDataRate = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 1:
			SensorConfig->GPSSendDataRate = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 2:
			SensorConfig->BatterySendDataRate = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 3:
			SensorConfig->StatusSendDataRate = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 4:
			SensorConfig->TargetsSendDataRate = value;
			break;
		case DATA_TX_INTVL_EEPROM_OFFSET + 5:
			SensorConfig->MotorsSendDataRate = value;
			break;
	}

	return OK;
}

iOpResult_e TWB_Sensors_HandleMessage(TWB_Commo_Message_t *msg) {
	/*
	 * If NiVek is armed...(power going to motors)
	 * Don't even think about changing the sensor actions
	 * We don't want to take the chance of messing with the control loop and
	 * motors
	 */
	if(SystemStatus->ArmedStatus == Armed){
		TWB_Debug_Print("Armed: Ignoring\r\n");

		uint8_t buffer[2];

		buffer[1] =  msg->SerialNumber & 8;
		buffer[0] =  msg->SerialNumber >> 8;

		TWB_Commo_SendMessage(MOD_Sensor,MSG_InvalidConfigOpreation, buffer, 2);
		return Ignored;
	}

	/* If we made it here, pause the sensors */
	TWB_Sensors_Pause();

	switch(msg->TypeId){
		case CMD_SensorZero:
			assert_succeed(TWB_Sensors_Start_Zero());
			break;

		case CMD_SensorStart:
			TWB_Debug_Print("Starting Sensors\r\n");

			/*Only restart if offline */
			if(SystemStatus->SnsrState == SensorAppState_Offline)
				TWB_Sensors_Start();
			break;

		case CMD_SensorStop:
			TWB_Debug_Print("Stopping Sensors\r\n");

			TWB_Sensors_Stop();

			break;

		case CMD_SensorRestart:
			TWB_Debug_Print("Restarting Sensors\r\n");
			SystemStatus->SnsrState = SensorAppState_Restarting;

			assert_succeed(TWB_Sensors_Init());
			break;

		case CMD_SensorRestoreDefaults:
			assert_succeed(TWB_IMU_RestoreDefaults());
			break;

		case CMD_BeginMagCalibration:
			SystemStatus->SnsrState = SensorAppState_MagCalibration;

			TWB_Debug_Print("Begin Compass Calibration\r\n");
			TWB_IMU_LSM303_Mag_BeginCalibration();
			break;


		case CMD_CancelMagCalibration:
			TWB_Debug_Print("Cancel Compass Calibration\r\n");
			TWB_IMU_LSM303_Mag_CancelCalibration();

			SystemStatus->SnsrState = SensorAppState_Online;
			break;


		case CMD_EndMagCalibration:
			SystemStatus->SnsrState = SensorAppState_FinishingCalibration;

			TWB_Debug_Print("End Compass Calibration\r\n");
			TWB_IMU_LSM303_Mag_EndCalibration();

			SystemStatus->SnsrState = SensorAppState_Online;

			break;

		case CMD_BeginSensorDiagnostics:
			__sensorInFocus = NULL;
			Sensor_t *snsr = SensorsList;
			while(snsr != NULL){
				if(snsr->SensorId == msg->MsgBuffer[0]){
					__sensorInFocus = snsr;
					snsr = NULL;
				}
				else
					snsr = snsr->Next;
			}

			if(__sensorInFocus != NULL){
				TWB_Debug_PrintInt("Sensor Diagnostics: ", msg->MsgBuffer[0]);
				SystemStatus->SnsrState = SensorAppState_Diagnostics;
			}
			else{
				TWB_Debug_PrintInt("Couldn't find sensor: ", msg->MsgBuffer[0]);
				SystemStatus->SnsrState = SensorAppState_Online;
			}


			break;

		case CMD_BeginSensorFusionDiag:
			TWB_Debug_Print("Begin Sensor Fusion Diagnostics\r\n");
			switch(msg->MsgBuffer[0]){
				case Measure_Pitch: SystemStatus->SnsrState = SensorAppState_SnsrFusionPitch; break;
				case Measure_Roll: SystemStatus->SnsrState = SensorAppState_SnsrFusionRoll; break;
				case Measure_Heading: SystemStatus->SnsrState = SensorAppState_SnsrFusionHeading; break;
				case Measure_Altitude: SystemStatus->SnsrState = SensorAppState_SnsrFusionAltitude; break;
			}

			break;

		case CMD_EndSensorFusionDiag:
			TWB_Debug_Print("End Sensor Fusion Diagnostics\r\n");
			SystemStatus->SnsrState = SensorAppState_Online;
			break;


		case CMD_EndSensorDiagnostics:
			TWB_Debug_Print("End Sensor Diagnostics\r\n");
			SystemStatus->SnsrState = SensorAppState_Online;
			break;


		case CMD_SetCfg_Value:
			__sensorsUpdateSensorSetting((uint16_t)(msg->MsgBuffer[0] << 8 | msg->MsgBuffer[1]), msg->MsgBuffer[2]);
			TWB_Commo_SendMessage(MOD_Sensor, MSG_SetConfigData, msg->MsgBuffer, 3);;
			break;
		case CMD_GetCfg_Values:
			TWB_Commo_SendMessage(MOD_Sensor, MSG_SnsrsConfigData, (uint8_t*)SensorConfig, sizeof(SensorConfig_t));
			break;
		case CMD_ReadSensorFusionConfig:
			TWB_Commo_SendMessage(MOD_Sensor, MSG_SnsrFusionConfig, (uint8_t*)ComplementaryFilterConfig, sizeof(SensorFusion_t));
			break;
	}

	/* And then resume the normal sensor operation, if the sensors were not taken offline*/
	if(SystemStatus->SnsrState != SensorAppState_Offline
			&& SystemStatus->SnsrState != SensorAppState_Zeroing)
		TWB_Sensors_Resume();

	return OK;
}

void TWB_Sensors_QueueMsg(void){
	switch(SystemStatus->SnsrState){
		case SensorAppState_SnsrFusionPitch:
		case SensorAppState_SnsrFusionRoll:
		case SensorAppState_SnsrFusionHeading:
		case SensorAppState_SnsrFusionAltitude:
			ComplementarySnsrFusion->SendDiag();
			break;
		case SensorAppState_Diagnostics:
			__sensorInFocus->SendDiag();
			break;
		case SensorAppState_MagCalibration:
			MagLSM303Sensor->SendDiag();
			break;
		case SensorAppState_PIDConfig:
			TWB_GPIO_SendPIDConfig();
			break;
		default:
			TWB_Commo_SendMessage(MOD_Sensor,MSG_SnsrData, (uint8_t *)SensorData, sizeof(SensorData_t));
			break;
	}
}



