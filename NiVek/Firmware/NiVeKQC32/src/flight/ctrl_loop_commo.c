/*
 * ctrl_loop_commo.c
 *
 *  Created on: Mar 18, 2013
 *      Author: Kevin
 */

#include "sensors/snsr_main.h"
#include "flight/ctrl_loop.h"
#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "flight/pwm_source.h"
#include "flight/pwm_capture.h"

void TWB_GPIO_SendPIDConfig(void){
	PIDSensorData->Pitch = SensorData->Pitch;
	PIDSensorData->Roll = SensorData->Roll;
	PIDSensorData->Heading = SensorData->Heading;
	PIDSensorData->AltitudeM = SensorData->AltitudeM;

	PIDSensorData->TargetAltitude = Targets->TargetAltitude;
	PIDSensorData->TargetHeading = Targets->TargetHeading;
	PIDSensorData->TargetPitch = Targets->TargetPitch;
	PIDSensorData->TargetRoll = Targets->TargetRoll;

	PIDSensorData->ESC_Front = MotorStatus->ESC_Front;
	PIDSensorData->ESC_Left = MotorStatus->ESC_Left;
	PIDSensorData->ESC_Right = MotorStatus->ESC_Right;
	PIDSensorData->ESC_Rear = MotorStatus->ESC_Rear;

	//MSG_PidSnsrDataDetails

	//TWB_Commo_SendMessage(MOD_Sensor, MSG_PidSnsrData, (uint8_t *)PIDSensorData, sizeof(PIDSensorData_t));

	TWB_Commo_SendMessage(MOD_Sensor, MSG_PidSnsrDataDetails, (uint8_t *)PitchPIDDetails, sizeof(PIDTuning_t));
	TWB_Commo_SendMessage(MOD_Sensor, MSG_PidSnsrDataDetails, (uint8_t *)RollPIDDetails, sizeof(PIDTuning_t));
}

float __ctrlLoop_resolve_DFilter(uint8_t setting){
	switch(setting){
		case 0: return 0.0; /* N/A */
		case 1: return 0.5f; /* 5Hz */
		case 2: return 0.3f; /* 10Hz */
		case 3: return 0.25f;  /* 15Hz */
		case 4: return 0.1f;  /* 20Hz */
		case 5: return 0.075;  /* 25Hz */
		case 6: return 0.05;  /* 30Hz */
		case 7: return 0.025;  /* 35Hz */
		case 8: return 0.01;  /* 40Hz */
	}

	return FP_NAN;
}

iOpResult_e __ctrlLoop_setPIDConstant(uint8_t *payload){
	uint16_t constId = payload[0] + PID_EEPROM_OFFSET;

	if((constId >= K_PITCH_RATE_P_ADDR) && (constId <= K_ROLL_STEADY_I_MAX_ADDR)){
		int16_t value = (int16_t)(payload[1] << 8 | payload[2]);

		assert_succeed(TWB_SE_WriteS16((uint16_t)constId, value));

		switch(constId){
			case K_PITCH_RATE_P_ADDR: PIDConstantsConfig->k_pitch_rate_p = value; break;
			case K_PITCH_RATE_I_ADDR: PIDConstantsConfig->k_pitch_rate_i = value; break;
			case K_PITCH_RATE_D_ADDR: PIDConstantsConfig->k_pitch_rate_d = value; break;
			case K_PITCH_STEADY_P_ADDR: PIDConstantsConfig->k_pitch_steady_p = value; break;
			case K_PITCH_STEADY_I_ADDR: PIDConstantsConfig->k_pitch_steady_i = value; break;
			case K_PITCH_STEADY_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->pitch_steady_i_decay_factor = value; break;

			case K_PITCH_RATE_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->pitch_rate_i_decay_factor = value; break;
			case K_PITCH_RATE_I_MAX_ADDR: PIDConstantsConfig->pitch_rate_max_i = value; break;
			case K_PITCH_STEADY_I_MAX_ADDR: PIDConstantsConfig->pitch_steady_max_i = value; break;

			case K_ROLL_RATE_P_ADDR: PIDConstantsConfig->k_roll_rate_p = value; break;
			case K_ROLL_RATE_I_ADDR: PIDConstantsConfig->k_roll_rate_i = value; break;
			case K_ROLL_RATE_D_ADDR: PIDConstantsConfig->k_roll_rate_d = value;  break;
			case K_ROLL_STEADY_P_ADDR: PIDConstantsConfig->k_roll_steady_p = value; break;
			case K_ROLL_STEADY_I_ADDR: PIDConstantsConfig->k_roll_steady_i = value; break;
			case K_ROLL_STEADY_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->roll_steady_i_decay_factor = value; break;

			case K_ROLL_RATE_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->roll_rate_i_decay_factor = value;   break;
			case K_ROLL_RATE_I_MAX_ADDR: PIDConstantsConfig->roll_rate_max_i = value; break;
			case K_ROLL_STEADY_I_MAX_ADDR: PIDConstantsConfig->roll_steady_max_i = value; break;
		}

		TWB_Debug_Print2Int("Set PID Constant", payload[0], value );

		TWB_GPIO_CtrlLoop_SetPitchRollPID();
	}
	else if(constId >= K_YAW_RATE_P_ADDR && constId <= K_ALTITUDE_STEADY_I_MAX_ADDR){
		int16_t value = (int16_t)(payload[1] << 8 | payload[2]);

		assert_succeed(TWB_SE_WriteS16((uint16_t)constId, value));

		switch(constId){
		  /* Start Altitude Settings */

			case K_ALTITUDE_RATE_P_ADDR: PIDConstantsConfig->k_altitude_rate_p = value; PIDAltitude->k_RateP = value == 0 ? FP_NAN :(float)value / P_RATE_SCALER;  break;
			case K_ALTITUDE_RATE_I_ADDR: PIDConstantsConfig->k_altitude_rate_i = value; PIDAltitude->k_RateI = value == 0 ? FP_NAN :(float)value / I_RATE_SCALER;  break;
			case K_ALTITUDE_RATE_D_ADDR: PIDConstantsConfig->k_altitude_rate_d = value; PIDAltitude->k_RateD = value == 0 ? FP_NAN :(float)value / D_RATE_SCALER;  break;
			case K_ALTITUDE_STEADY_P_ADDR: PIDConstantsConfig->k_altitude_steady_p = value; PIDAltitude->k_SteadyP = value == 0 ? FP_NAN :(float)value / P_STEADY_SCALER;  break;
			case K_ALTITUDE_STEADY_I_ADDR: PIDConstantsConfig->k_altitude_steady_i = value; PIDAltitude->k_SteadyI = value == 0 ? FP_NAN :(float)value / I_STEADY_SCALER;  break;
			case K_ALTITUDE_STEADY_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->altitude_steady_i_decay_factor = value; PIDAltitude->ISteadyDecayFactor = value == 0 ? FP_NAN : (float) value / DECAY_FACTOR_SCALER;  break;

			case K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->altitude_rate_i_decay_factor = value; PIDAltitude->IRateDecayFactor =  value == 0 ? FP_NAN : (float)value / DECAY_FACTOR_SCALER;  break;
			case K_ALTITUDE_RATE_I_MAX_ADDR: PIDConstantsConfig->altitude_rate_max_i = value; PIDAltitude->Rate_MaxSigmaError = (float)value / MAX_I_SCALER;  break;
			case K_ALTITUDE_STEADY_I_MAX_ADDR: PIDConstantsConfig->altitude_steady_max_i = value; PIDAltitude->Steady_MaxSigmaError = (float)value / MAX_I_SCALER;  break;

			/* Start Yaw Settings */

			case K_YAW_RATE_P_ADDR: PIDConstantsConfig->k_heading_rate_p = value; PIDYaw->k_RateP =  value == 0 ? FP_NAN : (float)value / P_RATE_SCALER;  break;
			case K_YAW_RATE_I_ADDR: PIDConstantsConfig->k_heading_rate_i = value; PIDYaw->k_RateI =  value == 0 ? FP_NAN : (float)value / I_RATE_SCALER; break;
			case K_YAW_RATE_D_ADDR: PIDConstantsConfig->k_heading_rate_d = value; PIDYaw->k_RateD =  value == 0 ? FP_NAN : (float)value / D_RATE_SCALER;  break;
			case K_YAW_STEADY_P_ADDR: PIDConstantsConfig->k_heading_steady_p = value; PIDYaw->k_SteadyP =  value == 0 ? FP_NAN : (float)value / P_STEADY_SCALER;  break;
			case K_YAW_STEADY_I_ADDR: PIDConstantsConfig->k_heading_steady_i = value; PIDYaw->k_SteadyI =  value == 0 ? FP_NAN : (float)value / I_STEADY_SCALER; break;
			case K_YAW_STEADY_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->heading_steady_i_decay_factor = value; PIDYaw->ISteadyDecayFactor = value == 0 ? FP_NAN : (float) value / DECAY_FACTOR_SCALER;  break;

			case K_YAW_RATE_I_DECAY_FACTOR_ADDR: PIDConstantsConfig->heading_rate_i_decay_factor = value; PIDYaw->IRateDecayFactor =  value == 0 ? FP_NAN : (float)value / DECAY_FACTOR_SCALER;  break;
			case K_YAW_RATE_I_MAX_ADDR: PIDConstantsConfig->heading_rate_max_i = value; PIDYaw->Rate_MaxSigmaError = (float)value / MAX_I_SCALER;  break;
			case K_YAW_STEADY_I_MAX_ADDR: PIDConstantsConfig->heading_steady_max_i = value; PIDYaw->Steady_MaxSigmaError = (float)value / MAX_I_SCALER;  break;
		}

		TWB_Debug_Print2Int("Configured Pitch/Yaw PID", payload[0], value );
	}
	else if(constId >= K_ESC_FRONT_ADDR && constId <= K_ESC_REAR_ADDR){
		int16_t value = (int16_t)(payload[1] << 8 | payload[2]);

		assert_succeed(TWB_SE_WriteS16((uint16_t)constId, value));

		switch(constId){
			case K_ESC_FRONT_ADDR: PIDConstantsConfig->k_esc_front = value; PIDConstants->k_esc_front = (float)value / ESC_SCALER;  break;
			case K_ESC_LEFT_ADDR: PIDConstantsConfig->k_esc_left = value; PIDConstants->k_esc_left = (float)value / ESC_SCALER;  break;
			case K_ESC_RIGHT_ADDR: PIDConstantsConfig->k_esc_right = value; PIDConstants->k_esc_right = (float)value / ESC_SCALER;  break;
			case K_ESC_REAR_ADDR: PIDConstantsConfig->k_esc_rear = value; PIDConstants->k_esc_rear = (float)value / ESC_SCALER;  break;
		}

		TWB_Debug_Print2Int("Configured ESC Gain", payload[0], value );
	}
	else if(constId >= K_PITCH_STABLE_BAND && constId <= K_ALTITUDE_STABLE_BAND){
		uint8_t value = payload[2];
		assert_succeed(TWB_SE_WriteU8((uint16_t)constId, value));

		switch(constId){
			case K_PITCH_STABLE_BAND: PIDConstantsConfig->PitchStableBand = value; PIDConstants->PitchStableBand = (float)value / STABLE_BAND_SCALER; PIDPitch->StableBand = (float)value / STABLE_BAND_SCALER; break;
			case K_ROLL_STABLE_BAND: PIDConstantsConfig->RollStableBand = value; PIDConstants->RollStableBand = (float)value / STABLE_BAND_SCALER; PIDRoll->StableBand = (float)value / STABLE_BAND_SCALER; break;
			case K_HEADING_STABLE_BAND: PIDConstantsConfig->HeadingStableBand = value; PIDConstants->HeadingStableBand = (float)value / STABLE_BAND_SCALER; PIDYaw->StableBand = (float)value / STABLE_BAND_SCALER; break;
			case K_ALTITUDE_STABLE_BAND: PIDConstantsConfig->AltitudeStableBand = value; PIDConstants->AltitudeStableBand = (float)value / STABLE_BAND_SCALER; PIDAltitude->StableBand = (float)value / STABLE_BAND_SCALER; break;
		}

		TWB_Debug_Print2Int("Configured Stable Band", payload[0], value );
	}
	else if(constId == PID_SAMPLE_RATE){
		uint8_t value = payload[2];
		assert_succeed(TWB_SE_WriteU8((uint16_t)constId, value));

		PIDConstantsConfig->PIDSampleRate = value;
		PIDControllerAction->SampleRate = PIDConstantsConfig->PIDSampleRate;

		TWB_Debug_Print2Int("Configured PID Sample Rate", payload[0], value );
	}
	else if(constId >= K_THROTTLE_SENSITIVITY && constId <= K_YAW_SENSITIVITY){
		int8_t value = payload[2];
		assert_succeed(TWB_SE_WriteS8((uint16_t)constId, value));

		switch(constId){
			case K_THROTTLE_SENSITIVITY: PIDConstantsConfig->ThrottleSensitivity = value; PIDConstants->ThrottleSensitivity = 25.0f * ((float) value / 100.0f); break;
			case K_PITCH_SENSITIVITY: PIDConstantsConfig->PitchSensitivity = value; PIDConstants->PitchSensitivity = (float)value / SENSTIVITY_SCALER; break;
			case K_ROLL_SENSITIVITY: PIDConstantsConfig->RollSensitivity= value; PIDConstants->RollSensitivity = (float)value / SENSTIVITY_SCALER; break;
			case K_YAW_SENSITIVITY: PIDConstantsConfig->YawSensitivity = value; PIDConstants->YawSensitivity = (float)value / SENSTIVITY_SCALER; break;
		}

		TWB_Debug_Print2Int("Set Control Sensitivity ", payload[0], value );
	}
	else if(constId >= K_PITCH_FILTER_D && constId <= K_ALTITUDE_FILTER_D){
		int8_t value = payload[2];
		assert_succeed(TWB_SE_WriteS8((uint16_t)constId, value));

		switch(constId){
			case K_PITCH_FILTER_D: PIDConstantsConfig->pitch_filter_d = value; break;
			case K_ROLL_FILTER_D: PIDConstantsConfig->roll_filter_d = value; break;
			case K_YAW_FILTER_D: PIDConstantsConfig->heading_filter_d = value; PIDYaw->DerivativeFilter = __ctrlLoop_resolve_DFilter(value); break;
			case K_ALTITUDE_FILTER_D: PIDConstantsConfig->altitude_filter_d = value; PIDAltitude->DerivativeFilter = __ctrlLoop_resolve_DFilter(value); break;
		}

		TWB_Debug_Print2Int("Set Derivative Filter Selection ", payload[0], value );

		PIDRoll->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->roll_filter_d);

		if(PIDConstantsConfig->PitchFollowsRollPID == PitchDoesFollowRollPID){
			PIDPitch->DerivativeFilter = PIDRoll->DerivativeFilter;
			TWB_Debug_Print("Updated pitch derivative filter from roll filter.\r\n");
		}
		else{
			TWB_Debug_Print("Updated pitch derivative filter from configuration\r\n");
			PIDPitch->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->pitch_filter_d);
		}
	}else if(constId == K_ESC_UPDATE_RATE){
		//Even though the update rate is just a byte,
		//it's sent as a uint16 value.  Just use the LSB.
		//this allows a consistent value for the sending
		//config values.
		assert_succeed(TWB_SE_WriteU8(K_ESC_UPDATE_RATE, payload[2]));
		assert_succeed(TWB_PWM_SetNewESCUpdateRate(payload[2]));

		TWB_Debug_Print2Int("Set ESC Update Rate ", payload[0], payload[2] );
	}
	else if(constId == FRAME_CONFIG){
		if(payload[2] == FrameConfig_Cross){
			TWB_Debug_Print("Set Frame Config: Cross\r\n");
			PIDConstantsConfig->FrameConfig = payload[2];
			assert_succeed(TWB_SE_WriteU8(FRAME_CONFIG, payload[2]));
		}
		else if(payload[2] == FrameConfig_X){
			TWB_Debug_Print("Set Frame Config: X\r\n");
			PIDConstantsConfig->FrameConfig = payload[2];
			assert_succeed(TWB_SE_WriteU8(FRAME_CONFIG, payload[2]));
		}
		else{
			TWB_Debug_Print("!!!UKNOWN FRAME TYPE!!!\r\n");
		}
	}
	else if(constId == K_INITIAL_THROTTLE){
		PIDConstantsConfig->InitialThrottle = payload[2];
		assert_succeed(TWB_SE_WriteU8(K_INITIAL_THROTTLE, payload[2]));

		TWB_Debug_PrintInt("Set initial throttle (0-255): ", payload[2]);
	}

	else if(constId == K_INITIAL_ALTITUDE){
		PIDConstantsConfig->InitialAltitude = payload[2];
		assert_succeed(TWB_SE_WriteU8(K_INITIAL_ALTITUDE, PIDConstantsConfig->InitialAltitude));

		TWB_Debug_PrintInt("Set initial altitude (cm): ", PIDConstantsConfig->InitialAltitude);
	}
	else if(constId == K_PITCH_FOLLOW_ROLL_PID){
		PIDConstantsConfig->PitchFollowsRollPID = (PitchFollowRollPID_e)payload[2];
		assert_succeed(TWB_SE_WriteU8(K_PITCH_FOLLOW_ROLL_PID, PIDConstantsConfig->PitchFollowsRollPID));

		TWB_GPIO_CtrlLoop_SetPitchRollPID();
	}
	else if (constId = K_THROTTLE_DEAD_SPACE){
		PIDConstantsConfig->ThrottleDeadSpace = payload[2];
		assert_succeed(TWB_SE_WriteU8(K_THROTTLE_DEAD_SPACE, PIDConstantsConfig->PitchFollowsRollPID));
		TWB_Debug_PrintInt("Set throttle Deadspace (0-100): ", payload[2]);
	}

	return OK;
}

iOpResult_e TWB_CtrlLoop_HandleMessage(TWB_Commo_Message_t *msg) {
	switch(msg->TypeId){
		case CMD_ArmNiVek:
			TWB_GPIO_Arm();
			break;

		case CMD_LaunchNiVek:
			TWB_PWM_SetNewESCUpdateRate(PIDConstantsConfig->ESCUpdateRate);
			TWB_Debug_Print("Launching NiVek!\r\n");
			SystemStatus->GPIOState = GPIOAppState_Launching;
			SystemStatus->AltitudeHold = AltitudeHold_On;

			break;

		case CMD_LandNiVek:
			SystemStatus->GPIOState = GPIOAppState_Landing;
			SystemStatus->AltitudeHold = AltitudeHold_On;
			Targets->TargetAltitude = 0;
			InternalState->TargetAltitude = 0.0f;
			TWB_Debug_Print("Landing NiVek!\r\n");

			break;

		case CMD_Kill:
			TWB_GPIO_Safe();

			SystemStatus->ArmedStatus = Safe;
			SystemStatus->GPIOState = GPIOAppState_Idle;
			TWB_Debug_Print("KILL!!\r\n");

			break;
		case CMD_SafeNiVek:
			TWB_GPIO_Safe();
			break;

		case CMD_ArmESCs: /* TODO - add timed logic to put motors to 100%, then 0 */ break;
		case CMD_ReadAllPIDConstants:
			TWB_Commo_SendMessage(MOD_FlightControls, MSG_Out_PIDConstants, (uint8_t *)PIDConstantsConfig, sizeof(PIDConstantsConfig_t));
		break;

		case CMD_SetPIDConstant:
			if(__ctrlLoop_setPIDConstant(msg->MsgBuffer) == OK)
				TWB_Commo_SendMessage(MOD_FlightControls, MSG_Set_PIDConstant, &msg->MsgBuffer[0], 1);
			else{
				TWB_Debug_Print("Y");
				TWB_Commo_SendNak(msg->SerialNumber);
				return FAIL;
			}

			break;
		case CMD_ResetPIDValues:
			if(TWB_GPIO_CtrlLoop_ResetDefaults())
				TWB_Commo_SendMessage(MOD_FlightControls, MSG_Out_PIDConstants, (uint8_t *)PIDConstantsConfig, sizeof(PIDConstantsConfig_t));
			break;

		case CMD_UseRC:
			TWB_Debug_Print("Use RC\r\n");

			SystemStatus->ControlMethod = RCControlled;
			assert_succeed(TWB_SE_WriteU8(COMMON_EEPROM_OFFSET + 0, SystemStatus->ControlMethod));
			break;
		case CMD_UseWiFi:
			TWB_Debug_Print("Use WiFi\r\n");

			SystemStatus->ControlMethod = WiFiControlled;
			assert_succeed(TWB_SE_WriteU8(COMMON_EEPROM_OFFSET + 0, SystemStatus->ControlMethod));
			break;

		case CMD_Set_AltitudeHold:
			TWB_Debug_Print("Altitude Hold Set\r\n");

			SystemStatus->AltitudeHold = AltitudeHold_On;
			InternalState->TargetAltitude = InternalState->AltitudeM;
			Targets->TargetAltitude = (float)InternalState->AltitudeM;

			assert_succeed(TWB_SE_WriteU8(COMMON_EEPROM_OFFSET + 1, SystemStatus->AltitudeHold));

			break;
		case CMD_Clear_AltitudeHold:
			TWB_Debug_Print("Altitude Hold Clear\r\n");
			SystemStatus->AltitudeHold = AltitudeHold_Off;

			InternalState->TargetAltitude = 0.0f;
			Targets->TargetAltitude = 0;

			assert_succeed(TWB_SE_WriteU8(COMMON_EEPROM_OFFSET + 1, SystemStatus->AltitudeHold));
			break;

		case CMD_Set_Heading:
			Targets->TargetHeading = (msg->MsgBuffer[0] << 8 | msg->MsgBuffer[1]);
			InternalState->TargetHeading = (float)Targets->TargetHeading;
			TWB_Debug_PrintInt("Set New Heating: ", InternalState->TargetHeading);

			break;

		case CMD_Set_Altitude:
			SystemStatus->AltitudeHold = AltitudeHold_On;
			Targets->TargetAltitude = (msg->MsgBuffer[0] << 8 | msg->MsgBuffer[1]);
			InternalState->TargetAltitude = (float) Targets->TargetAltitude;
			TWB_Debug_PrintInt("Set New Altitude: ", InternalState->TargetAltitude);

			break;

		case CMD_ThrottleYawRollThrottle:
			if(msg->MsgBuffer[0] != WiFiThrottle){
				WiFiThrottle = msg->MsgBuffer[0];
				TWB_Debug_PrintInt("Throttle  : ", WiFiThrottle);
			}

			if(msg->MsgBuffer[1] != WiFiPitch){
				WiFiPitch = msg->MsgBuffer[1];
				TWB_Debug_PrintInt("Pitch  : ", WiFiPitch);
			}

			if(msg->MsgBuffer[2] != WiFiRoll){
				WiFiRoll = msg->MsgBuffer[2];
				TWB_Debug_PrintInt("Roll  : ", WiFiRoll);
			}

			if(msg->MsgBuffer[3] != WiFiYaw){
				WiFiYaw = msg->MsgBuffer[3];
				TWB_Debug_PrintInt("Yaw  : ", WiFiYaw);
			}

			break;

		case CMD_Set_ESC_Power:
			//First byte is the motor index (0xFF == All)
			//Second byte is the power (0-255);
			if(msg->MsgBuffer[0] == 0xFF){
				TWB_Debug_Print("Setting ESC Power - All\r\n");

				TWB_PWM_SetESCPower(Motor_PortFront, msg->MsgBuffer[1]);
				TWB_PWM_SetESCPower(Motor_StarboardRear, msg->MsgBuffer[1]);
				TWB_PWM_SetESCPower(Motor_StarboardFront, msg->MsgBuffer[1]);
				TWB_PWM_SetESCPower(Motor_PortRear, msg->MsgBuffer[1]);
				WiFiThrottle = msg->MsgBuffer[1];
			}
			else{
				TWB_Debug_Print("Setting ESC Power - Single\r\n");
				TWB_PWM_SetESCPower(msg->MsgBuffer[0], msg->MsgBuffer[1]);
			}

			break;

		case CMD_Start_RC_Calibration: assert_succeed(TWB_RCIN_BeginCalibration()); break;
		case CMD_CalibrateRCChannel: TWB_RCIN_CalibrateStick(msg->MsgBuffer[0]); break;
		case CMD_Cancel_RC_Calibration: assert_succeed(TWB_RCIN_CancelCalibration()); break;
		case CMD_End_RC_Calibration: assert_succeed(TWB_RCIN_FinalizeCalibration()); break;

		case CMD_StartMotorConfig:
			TWB_PWM_SetNewESCUpdateRate(PIDConstantsConfig->ESCUpdateRate);

			SystemStatus->ArmedStatus = Armed;
			SystemStatus->GPIOState = GPIOAppState_MotorConfig;
			WiFiThrottle = 0;
			break;
		case CMD_EndMotorConfig:
			SystemStatus->ArmedStatus = Safe;
			SystemStatus->GPIOState = GPIOAppState_Idle;
			break;
		case CMD_StartPIDConfig:
			TWB_PWM_SetNewESCUpdateRate(PIDConstantsConfig->ESCUpdateRate);

			TWB_Debug_Print("Start PID Config.");

			SystemStatus->SnsrState = SensorAppState_PIDConfig;
			SystemStatus->ArmedStatus = Armed;
			SystemStatus->GPIOState = GPIOAppState_PIDConfig;
			break;
		case CMD_EndPIDConfig:
			TWB_Debug_Print("End PID Config.");

			SystemStatus->SnsrState = SensorAppState_Online;
			SystemStatus->ArmedStatus = Safe;
			SystemStatus->GPIOState = GPIOAppState_Idle;
			break;
	}

	return OK;
}
