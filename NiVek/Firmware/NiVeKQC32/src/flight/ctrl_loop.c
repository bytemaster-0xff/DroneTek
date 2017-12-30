/*
 * ctrl_loop.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "common/twb_common.h"
#include "twb_config.h"
#include <stdlib.h>
#include <string.h>
#include "flight/autopilot.h"
#include "flight/pwm_source.h"
#include "flight/pwm_capture.h"
#include "flight/ctrl_loop.h"
#include "commo/message_ids.h"

#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "common/memory_mgmt.h"
#include "sensors/snsr_main.h"
#include "math.h"


/* Break this out since logic also needs to be ran from the commo loop */
iOpResult_e TWB_GPIO_CtrlLoop_SetPitchRollPID(void){
	PIDRoll->k_RateP = PIDConstantsConfig->k_roll_rate_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_roll_rate_p) / P_RATE_SCALER;
	PIDRoll->k_RateI = PIDConstantsConfig->k_roll_rate_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_roll_rate_i) / I_RATE_SCALER;
	PIDRoll->k_RateD = PIDConstantsConfig->k_roll_rate_d == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_roll_rate_d) / D_RATE_SCALER;
	PIDRoll->k_SteadyP = PIDConstantsConfig->k_roll_steady_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_roll_steady_p) / P_STEADY_SCALER;
	PIDRoll->k_SteadyI = PIDConstantsConfig->k_roll_steady_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_roll_steady_i) / I_STEADY_SCALER;

	PIDRoll->Rate_MaxSigmaError = ((float)PIDConstantsConfig->roll_rate_max_i) / MAX_I_SCALER;
	PIDRoll->Steady_MaxSigmaError = ((float)PIDConstantsConfig->roll_steady_max_i) / MAX_I_SCALER;

	PIDRoll->ISteadyDecayFactor = PIDConstantsConfig->roll_steady_i_decay_factor == 0 ? FP_NAN : ((float) PIDConstantsConfig->roll_steady_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDRoll->IRateDecayFactor = PIDConstantsConfig->roll_rate_i_decay_factor == 0 ? FP_NAN:  ((float)PIDConstantsConfig->roll_rate_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDRoll->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->roll_filter_d);

	if(PIDConstantsConfig->PitchFollowsRollPID == PitchDoesFollowRollPID){
		PIDPitch->k_RateP = PIDRoll->k_RateP;
		PIDPitch->k_RateI = PIDRoll->k_RateI;
		PIDPitch->k_RateD = PIDRoll->k_RateD;
		PIDPitch->k_SteadyP = PIDRoll->k_SteadyP;
		PIDPitch->k_SteadyI = PIDRoll->k_SteadyI;

		PIDPitch->Rate_MaxSigmaError = PIDRoll->Rate_MaxSigmaError;
		PIDPitch->Steady_MaxSigmaError = PIDRoll->Steady_MaxSigmaError;
		PIDPitch->IRateDecayFactor = PIDRoll->IRateDecayFactor;
		PIDPitch->ISteadyDecayFactor = PIDRoll->ISteadyDecayFactor;
		PIDPitch->DerivativeFilter = PIDRoll->DerivativeFilter;
		TWB_Debug_Print("Copied Roll Settings into Pitch.\r\n");
	}
	else{
		PIDPitch->k_RateP = PIDConstantsConfig->k_pitch_rate_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_pitch_rate_p) / P_RATE_SCALER;
		PIDPitch->k_RateI = PIDConstantsConfig->k_pitch_rate_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_pitch_rate_i) / I_RATE_SCALER;
		PIDPitch->k_RateD = PIDConstantsConfig->k_pitch_rate_d == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_pitch_rate_d) / D_RATE_SCALER;
		PIDPitch->k_SteadyP = PIDConstantsConfig->k_pitch_steady_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_pitch_steady_p) / P_STEADY_SCALER;
		PIDPitch->k_SteadyI = PIDConstantsConfig->k_pitch_steady_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_pitch_steady_i) / I_STEADY_SCALER;

		PIDPitch->Rate_MaxSigmaError = ((float)PIDConstantsConfig->pitch_rate_max_i) / MAX_I_SCALER;
		PIDPitch->Steady_MaxSigmaError = ((float)PIDConstantsConfig->pitch_steady_max_i) / MAX_I_SCALER;
		PIDPitch->IRateDecayFactor = PIDConstantsConfig->pitch_rate_i_decay_factor == 0 ? FP_NAN : ((float)PIDConstantsConfig->pitch_rate_i_decay_factor) / DECAY_FACTOR_SCALER;
		PIDPitch->ISteadyDecayFactor = PIDConstantsConfig->pitch_steady_i_decay_factor == 0 ? FP_NAN : ((float) PIDConstantsConfig->pitch_steady_i_decay_factor) / DECAY_FACTOR_SCALER;
		PIDPitch->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->pitch_filter_d);
		TWB_Debug_Print("Setting Pitch settings from EEPROM Configuration.\r\n");
	}

	return OK;
}

iOpResult_e TWB_GPIO_CtrlLoop_SetAltitudePID(void){
	PIDAltitude->k_RateP = PIDConstantsConfig->k_altitude_rate_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_altitude_rate_p) / P_RATE_SCALER;
	PIDAltitude->k_RateI = PIDConstantsConfig->k_altitude_rate_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_altitude_rate_i) / I_RATE_SCALER;
	PIDAltitude->k_RateD = PIDConstantsConfig->k_altitude_rate_d == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_altitude_rate_d) / D_RATE_SCALER;
	PIDAltitude->k_SteadyP = PIDConstantsConfig->k_altitude_steady_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_altitude_steady_p) / P_STEADY_SCALER;
	PIDAltitude->k_SteadyI = PIDConstantsConfig->k_altitude_steady_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_altitude_steady_i) / I_STEADY_SCALER;

	PIDAltitude->Rate_MaxSigmaError = ((float)PIDConstantsConfig->altitude_rate_max_i) / MAX_I_SCALER;
	PIDAltitude->IRateDecayFactor = PIDConstantsConfig->altitude_rate_i_decay_factor == 0 ? FP_NAN : ((float)PIDConstantsConfig->altitude_rate_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDAltitude->ISteadyDecayFactor = PIDConstantsConfig->altitude_steady_i_decay_factor == 0 ? FP_NAN : ((float) PIDConstantsConfig->altitude_steady_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDAltitude->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->altitude_filter_d);

	return OK;
}

iOpResult_e TWB_GPIO_CtrlLoop_SetYawPID(void){
	PIDYaw->k_RateP = PIDConstantsConfig->k_heading_rate_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_heading_rate_p) / P_RATE_SCALER;
	PIDYaw->k_RateI = PIDConstantsConfig->k_heading_rate_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_heading_rate_i) / I_RATE_SCALER;
	PIDYaw->k_RateD = PIDConstantsConfig->k_heading_rate_d == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_heading_rate_d) / D_RATE_SCALER;
	PIDYaw->k_SteadyP = PIDConstantsConfig->k_heading_steady_p == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_heading_steady_p) / P_STEADY_SCALER;
	PIDYaw->k_SteadyI = PIDConstantsConfig->k_heading_steady_i == 0 ? FP_NAN : ((float)PIDConstantsConfig->k_heading_steady_i) / I_STEADY_SCALER;

	PIDYaw->Rate_MaxSigmaError = ((float)PIDConstantsConfig->heading_rate_max_i) / MAX_I_SCALER;
	PIDYaw->Steady_MaxSigmaError = ((float)PIDConstantsConfig->heading_steady_max_i) / MAX_I_SCALER;
	PIDYaw->IRateDecayFactor = PIDConstantsConfig->heading_rate_i_decay_factor == 0 ? FP_NAN : ((float) PIDConstantsConfig->heading_rate_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDYaw->ISteadyDecayFactor = PIDConstantsConfig->heading_steady_i_decay_factor == 0 ? FP_NAN : ((float) PIDConstantsConfig->heading_steady_i_decay_factor) / DECAY_FACTOR_SCALER;
	PIDYaw->DerivativeFilter = __ctrlLoop_resolve_DFilter(PIDConstantsConfig->heading_filter_d);

	return OK;
}

iOpResult_e TWB_GPIO_CtrlLoop_Init(void){
	assert_succeed(TWB_SE_ReadS16(K_PITCH_RATE_P_ADDR, &PIDConstantsConfig->k_pitch_rate_p));
	assert_succeed(TWB_SE_ReadS16(K_PITCH_RATE_I_ADDR, &PIDConstantsConfig->k_pitch_rate_i));
	assert_succeed(TWB_SE_ReadS16(K_PITCH_RATE_D_ADDR, &PIDConstantsConfig->k_pitch_rate_d));
	assert_succeed(TWB_SE_ReadU16(K_PITCH_RATE_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->pitch_rate_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_PITCH_RATE_I_MAX_ADDR, &PIDConstantsConfig->pitch_rate_max_i));
	assert_succeed(TWB_SE_ReadS16(K_PITCH_STEADY_P_ADDR, &PIDConstantsConfig->k_pitch_steady_p));
	assert_succeed(TWB_SE_ReadS16(K_PITCH_STEADY_I_ADDR, &PIDConstantsConfig->k_pitch_steady_i));
	assert_succeed(TWB_SE_ReadU16(K_PITCH_STEADY_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->pitch_steady_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_PITCH_STEADY_I_MAX_ADDR, &PIDConstantsConfig->pitch_steady_max_i));

	assert_succeed(TWB_SE_ReadS16(K_ROLL_RATE_P_ADDR, &PIDConstantsConfig->k_roll_rate_p));
	assert_succeed(TWB_SE_ReadS16(K_ROLL_RATE_I_ADDR, &PIDConstantsConfig->k_roll_rate_i));
	assert_succeed(TWB_SE_ReadS16(K_ROLL_RATE_D_ADDR, &PIDConstantsConfig->k_roll_rate_d));
	assert_succeed(TWB_SE_ReadU16(K_ROLL_RATE_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->roll_rate_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_ROLL_RATE_I_MAX_ADDR, &PIDConstantsConfig->roll_rate_max_i));
	assert_succeed(TWB_SE_ReadS16(K_ROLL_STEADY_P_ADDR, &PIDConstantsConfig->k_roll_steady_p));
	assert_succeed(TWB_SE_ReadS16(K_ROLL_STEADY_I_ADDR, &PIDConstantsConfig->k_roll_steady_i));
	assert_succeed(TWB_SE_ReadU16(K_ROLL_STEADY_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->roll_steady_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_ROLL_STEADY_I_MAX_ADDR, &PIDConstantsConfig->roll_steady_max_i));

	assert_succeed(TWB_SE_ReadS16(K_YAW_RATE_P_ADDR, &PIDConstantsConfig->k_heading_rate_p));
	assert_succeed(TWB_SE_ReadS16(K_YAW_RATE_I_ADDR, &PIDConstantsConfig->k_heading_rate_i));
	assert_succeed(TWB_SE_ReadS16(K_YAW_RATE_D_ADDR, &PIDConstantsConfig->k_heading_rate_d));
	assert_succeed(TWB_SE_ReadU16(K_YAW_RATE_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->heading_rate_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_YAW_RATE_I_MAX_ADDR, &PIDConstantsConfig->heading_rate_max_i));
	assert_succeed(TWB_SE_ReadS16(K_YAW_STEADY_P_ADDR, &PIDConstantsConfig->k_heading_steady_p));
	assert_succeed(TWB_SE_ReadS16(K_YAW_STEADY_I_ADDR, &PIDConstantsConfig->k_heading_steady_i));
	assert_succeed(TWB_SE_ReadU16(K_YAW_STEADY_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->heading_steady_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_YAW_STEADY_I_MAX_ADDR, &PIDConstantsConfig->heading_steady_max_i));

	assert_succeed(TWB_SE_ReadS16(K_ALTITUDE_RATE_P_ADDR, &PIDConstantsConfig->k_altitude_rate_p));
	assert_succeed(TWB_SE_ReadS16(K_ALTITUDE_RATE_I_ADDR, &PIDConstantsConfig->k_altitude_rate_i));
	assert_succeed(TWB_SE_ReadS16(K_ALTITUDE_RATE_D_ADDR, &PIDConstantsConfig->k_altitude_rate_d));
	assert_succeed(TWB_SE_ReadU16(K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->altitude_rate_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_ALTITUDE_RATE_I_MAX_ADDR, &PIDConstantsConfig->altitude_rate_max_i));
	assert_succeed(TWB_SE_ReadS16(K_ALTITUDE_STEADY_P_ADDR, &PIDConstantsConfig->k_altitude_steady_p));
	assert_succeed(TWB_SE_ReadS16(K_ALTITUDE_STEADY_I_ADDR, &PIDConstantsConfig->k_altitude_steady_i));
	assert_succeed(TWB_SE_ReadU16(K_ALTITUDE_STEADY_I_DECAY_FACTOR_ADDR, &PIDConstantsConfig->altitude_steady_i_decay_factor));
	assert_succeed(TWB_SE_ReadU16(K_ALTITUDE_STEADY_I_MAX_ADDR, &PIDConstantsConfig->altitude_steady_max_i));

	assert_succeed(TWB_SE_ReadS16(K_ESC_FRONT_ADDR, &PIDConstantsConfig->k_esc_front));
	assert_succeed(TWB_SE_ReadS16(K_ESC_LEFT_ADDR, &PIDConstantsConfig->k_esc_right));
	assert_succeed(TWB_SE_ReadS16(K_ESC_RIGHT_ADDR, &PIDConstantsConfig->k_esc_left));
	assert_succeed(TWB_SE_ReadS16(K_ESC_REAR_ADDR, &PIDConstantsConfig->k_esc_rear));

	assert_succeed(TWB_SE_ReadU8(K_PITCH_STABLE_BAND, &PIDConstantsConfig->PitchStableBand));
	assert_succeed(TWB_SE_ReadU8(K_ROLL_STABLE_BAND, &PIDConstantsConfig->RollStableBand));
	assert_succeed(TWB_SE_ReadU8(K_HEADING_STABLE_BAND, &PIDConstantsConfig->HeadingStableBand));
	assert_succeed(TWB_SE_ReadU8(K_ALTITUDE_STABLE_BAND, &PIDConstantsConfig->AltitudeStableBand));

	assert_succeed(TWB_SE_ReadU8(PID_SAMPLE_RATE, &PIDConstantsConfig->PIDSampleRate));

	assert_succeed(TWB_SE_ReadS8(K_THROTTLE_SENSITIVITY, &PIDConstantsConfig->ThrottleSensitivity));
	assert_succeed(TWB_SE_ReadS8(K_PITCH_SENSITIVITY, &PIDConstantsConfig->PitchSensitivity));
	assert_succeed(TWB_SE_ReadS8(K_ROLL_SENSITIVITY, &PIDConstantsConfig->RollSensitivity));
	assert_succeed(TWB_SE_ReadS8(K_YAW_SENSITIVITY, &PIDConstantsConfig->YawSensitivity));

	assert_succeed(TWB_SE_ReadU8(K_PITCH_FILTER_D, &PIDConstantsConfig->pitch_filter_d));
	assert_succeed(TWB_SE_ReadU8(K_ROLL_FILTER_D, &PIDConstantsConfig->roll_filter_d));
	assert_succeed(TWB_SE_ReadU8(K_YAW_FILTER_D, &PIDConstantsConfig->heading_filter_d));
	assert_succeed(TWB_SE_ReadU8(K_ALTITUDE_FILTER_D, &PIDConstantsConfig->altitude_filter_d));

	assert_succeed(TWB_SE_ReadU8(FRAME_CONFIG, &PIDConstantsConfig->FrameConfig));

	assert_succeed(TWB_SE_ReadU8(K_INITIAL_THROTTLE, &PIDConstantsConfig->InitialThrottle));
	assert_succeed(TWB_SE_ReadU8(K_INITIAL_ALTITUDE, &PIDConstantsConfig->InitialAltitude));

	assert_succeed(TWB_SE_ReadU8(K_AUTOPILOT_SAMPLE_RATE, &PIDConstantsConfig->AutoPilotSampleRate));
	assert_succeed(TWB_SE_ReadU8(K_PITCH_FOLLOW_ROLL_PID, &PIDConstantsConfig->PitchFollowsRollPID));

	assert_succeed(TWB_SE_ReadU8(COMMON_EEPROM_OFFSET + 0, &SystemStatus->ControlMethod));
	assert_succeed(TWB_SE_ReadU8(COMMON_EEPROM_OFFSET + 1, &SystemStatus->AltitudeHold));

	SystemStatus->GPIOState = GPIOAppState_Idle;

	PIDControllerAction->IsEnabled = true;
	PIDControllerAction->SampleRate = PIDConstantsConfig->PIDSampleRate;

	PIDControllerAction->Read = &TWB_GPIO_CtrlLoop_Exec;

	PIDPitch = pm_malloc(sizeof(PID_t));
	PIDRoll = pm_malloc(sizeof(PID_t));
	PIDYaw = pm_malloc(sizeof(PID_t));
	PIDAltitude = pm_malloc(sizeof(PID_t));

	TWB_GPIO_CtrlLoop_SetPitchRollPID();
	TWB_GPIO_CtrlLoop_SetAltitudePID();
	TWB_GPIO_CtrlLoop_SetYawPID();

	PIDConstants->k_esc_front = ((float)PIDConstantsConfig->k_esc_front) / ESC_SCALER;
	PIDConstants->k_esc_left = ((float)PIDConstantsConfig->k_esc_left) / ESC_SCALER;
	PIDConstants->k_esc_right = ((float)PIDConstantsConfig->k_esc_right) / ESC_SCALER;
	PIDConstants->k_esc_rear = ((float)PIDConstantsConfig->k_esc_rear) / ESC_SCALER;

	PIDConstants->PitchStableBand = ((float)PIDConstantsConfig->PitchStableBand) / STABLE_BAND_SCALER;
	PIDConstants->RollStableBand = ((float)PIDConstantsConfig->RollStableBand) / STABLE_BAND_SCALER;
	PIDConstants->AltitudeStableBand = ((float)PIDConstantsConfig->AltitudeStableBand) / STABLE_BAND_SCALER;
	PIDConstants->HeadingStableBand = ((float)PIDConstantsConfig->HeadingStableBand) / STABLE_BAND_SCALER;

	PIDAltitude->StableBand = ((float)PIDConstantsConfig->AltitudeStableBand) / STABLE_BAND_SCALER;
	PIDYaw->StableBand = ((float)PIDConstantsConfig->HeadingStableBand) / STABLE_BAND_SCALER;
	PIDPitch->StableBand = ((float)PIDConstantsConfig->PitchStableBand) / STABLE_BAND_SCALER;
	PIDRoll->StableBand = ((float)PIDConstantsConfig->RollStableBand) / STABLE_BAND_SCALER;

	PIDConstants->ThrottleSensitivity = 25.0f * ((float) PIDConstantsConfig->ThrottleSensitivity) / 100.0f;
	PIDConstants->PitchSensitivity = ((float)PIDConstantsConfig->PitchSensitivity) / SENSTIVITY_SCALER;
	PIDConstants->RollSensitivity = ((float)PIDConstantsConfig->RollSensitivity) / SENSTIVITY_SCALER;
	PIDConstants->YawSensitivity = ((float)PIDConstantsConfig->YawSensitivity) / SENSTIVITY_SCALER;

	RollPIDDetails = pm_malloc(sizeof(PIDTuning_t));
	PitchPIDDetails = pm_malloc(sizeof(PIDTuning_t));

	RollPIDDetails->ForRoll = 0xFF;
	PitchPIDDetails->ForRoll = 0x00;

	TWB_Debug_Print("Flight Ctrl: OK\r\n");

	return OK;
}

void TWB_Targets_QueueMsg(void) {
	TWB_Commo_SendMessage(MOD_System, MSG_SYSTEM_TARGETS_ID, (uint8_t *)Targets, sizeof(TargetsData_t));
}

iOpResult_e TWB_GPIO_CtrlLoop_ResetDefaults(void) {
	PIDConstantsConfig->k_roll_steady_p = 0;
	PIDConstantsConfig->k_pitch_steady_p = 0;
	PIDConstantsConfig->k_heading_steady_p = 0;
	PIDConstantsConfig->k_altitude_steady_p = 0;

	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_STEADY_P_ADDR, PIDConstantsConfig->k_roll_steady_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_STEADY_P_ADDR, PIDConstantsConfig->k_pitch_steady_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_STEADY_P_ADDR, PIDConstantsConfig->k_heading_steady_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_STEADY_P_ADDR, PIDConstantsConfig->k_altitude_steady_p));

	PIDConstantsConfig->k_roll_steady_i = 0;
	PIDConstantsConfig->k_pitch_steady_i = 0;
	PIDConstantsConfig->k_heading_steady_i = 0;
	PIDConstantsConfig->k_altitude_steady_i = 0;

	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_STEADY_I_ADDR, PIDConstantsConfig->k_roll_steady_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_STEADY_I_ADDR, PIDConstantsConfig->k_pitch_steady_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_STEADY_I_ADDR, PIDConstantsConfig->k_heading_steady_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_STEADY_I_ADDR, PIDConstantsConfig->k_altitude_steady_i));

	PIDConstantsConfig->k_roll_rate_p = 0;
	PIDConstantsConfig->k_pitch_rate_p = 0;
	PIDConstantsConfig->k_heading_rate_p = 0;
	PIDConstantsConfig->k_altitude_rate_p = 0;

	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_RATE_P_ADDR, PIDConstantsConfig->k_roll_rate_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_RATE_P_ADDR, PIDConstantsConfig->k_pitch_rate_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_RATE_P_ADDR, PIDConstantsConfig->k_heading_rate_p));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_RATE_P_ADDR, PIDConstantsConfig->k_altitude_rate_p));

	PIDConstantsConfig->k_roll_rate_i = 0;
	PIDConstantsConfig->k_pitch_rate_i = 0;
	PIDConstantsConfig->k_heading_rate_i = 0;
	PIDConstantsConfig->k_altitude_rate_i = 0;

	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_RATE_I_ADDR, PIDConstantsConfig->k_roll_rate_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_RATE_I_ADDR, PIDConstantsConfig->k_pitch_rate_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_RATE_I_ADDR, PIDConstantsConfig->k_heading_rate_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_RATE_I_ADDR, PIDConstantsConfig->k_altitude_rate_i));

	PIDConstantsConfig->k_roll_rate_d = 0;
	PIDConstantsConfig->k_pitch_rate_d = 0;
	PIDConstantsConfig->k_heading_rate_d = 0;
	PIDConstantsConfig->k_altitude_rate_d = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_RATE_D_ADDR, PIDConstantsConfig->k_roll_rate_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_RATE_D_ADDR, PIDConstantsConfig->k_pitch_rate_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_RATE_D_ADDR, PIDConstantsConfig->k_heading_rate_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_RATE_D_ADDR, PIDConstantsConfig->k_altitude_rate_d));

	PIDConstantsConfig->roll_steady_i_decay_factor = 0;
	PIDConstantsConfig->pitch_steady_i_decay_factor = 0;
	PIDConstantsConfig->heading_steady_i_decay_factor = 0;
	PIDConstantsConfig->altitude_steady_i_decay_factor = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_STEADY_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->roll_steady_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_STEADY_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->pitch_steady_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_STEADY_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->heading_steady_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_STEADY_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->altitude_steady_i_decay_factor));


	PIDConstantsConfig->roll_rate_i_decay_factor = 0;
	PIDConstantsConfig->pitch_rate_i_decay_factor = 0;
	PIDConstantsConfig->heading_rate_i_decay_factor = 0;
	PIDConstantsConfig->altitude_rate_i_decay_factor = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_RATE_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->roll_rate_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_RATE_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->pitch_rate_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_RATE_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->heading_rate_i_decay_factor));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR, PIDConstantsConfig->altitude_rate_i_decay_factor));

	PIDConstantsConfig->roll_rate_max_i = 0;
	PIDConstantsConfig->pitch_rate_max_i = 0;
	PIDConstantsConfig->heading_rate_max_i = 0;
	PIDConstantsConfig->altitude_rate_max_i = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_RATE_I_MAX_ADDR, PIDConstantsConfig->roll_rate_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_RATE_I_MAX_ADDR, PIDConstantsConfig->pitch_rate_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_RATE_I_MAX_ADDR, PIDConstantsConfig->heading_rate_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_RATE_I_MAX_ADDR, PIDConstantsConfig->altitude_rate_max_i));

	PIDConstantsConfig->roll_steady_max_i = 0;
	PIDConstantsConfig->pitch_steady_max_i = 0;
	PIDConstantsConfig->heading_steady_max_i = 0;
	PIDConstantsConfig->altitude_steady_max_i = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_STEADY_I_MAX_ADDR, PIDConstantsConfig->roll_steady_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_STEADY_I_MAX_ADDR, PIDConstantsConfig->pitch_steady_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_STEADY_I_MAX_ADDR, PIDConstantsConfig->heading_steady_max_i));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_STEADY_I_MAX_ADDR, PIDConstantsConfig->altitude_steady_max_i));

	PIDConstantsConfig->pitch_filter_d = 0;
	PIDConstantsConfig->roll_filter_d = 0;
	PIDConstantsConfig->heading_filter_d = 0;
	PIDConstantsConfig->altitude_filter_d = 0;
	assert_succeed(TWB_SE_WriteU16Pause(K_PITCH_FILTER_D, PIDConstantsConfig->pitch_filter_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_ROLL_FILTER_D, PIDConstantsConfig->roll_filter_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_YAW_FILTER_D, PIDConstantsConfig->heading_filter_d));
	assert_succeed(TWB_SE_WriteU16Pause(K_ALTITUDE_FILTER_D, PIDConstantsConfig->altitude_filter_d));

	PIDConstantsConfig->k_esc_front = 1 * ESC_SCALER;
	PIDConstantsConfig->k_esc_left = 1 * ESC_SCALER;
	PIDConstantsConfig->k_esc_right = 1 * ESC_SCALER;
	PIDConstantsConfig->k_esc_rear = 1 * ESC_SCALER;

	assert_succeed(TWB_SE_WriteU16Pause(K_ESC_FRONT_ADDR, PIDConstantsConfig->k_esc_front));
	assert_succeed(TWB_SE_WriteU16Pause(K_ESC_LEFT_ADDR, PIDConstantsConfig->k_esc_left));
	assert_succeed(TWB_SE_WriteU16Pause(K_ESC_RIGHT_ADDR, PIDConstantsConfig->k_esc_right));
	assert_succeed(TWB_SE_WriteU16Pause(K_ESC_REAR_ADDR, PIDConstantsConfig->k_esc_rear));

	PIDConstantsConfig->ESCUpdateRate = 1; //Default of 50Hz for standard ESC
	assert_succeed(TWB_SE_WriteU8Pause(K_ESC_UPDATE_RATE, PIDConstantsConfig->ESCUpdateRate));

	PIDConstantsConfig->PitchStableBand = 2 * SENSTIVITY_SCALER; // +/- 2.0 degrees
	PIDConstantsConfig->RollStableBand = 2 * SENSTIVITY_SCALER; // +/- 2.0 degrees
	PIDConstantsConfig->HeadingStableBand = 2 * SENSTIVITY_SCALER; // +/- 2.0 degrees
	PIDConstantsConfig->AltitudeStableBand = 5 * SENSTIVITY_SCALER; // +/- 5 CM

	assert_succeed(TWB_SE_WriteU8Pause(K_PITCH_STABLE_BAND, PIDConstantsConfig->PitchStableBand));
	assert_succeed(TWB_SE_WriteU8Pause(K_ROLL_STABLE_BAND, PIDConstantsConfig->RollStableBand));
	assert_succeed(TWB_SE_WriteU8Pause(K_HEADING_STABLE_BAND, PIDConstantsConfig->HeadingStableBand));
	assert_succeed(TWB_SE_WriteU8Pause(K_ALTITUDE_STABLE_BAND, PIDConstantsConfig->AltitudeStableBand));

	PIDConstantsConfig->ThrottleSensitivity = 100; //0 Default sensitivity Negative numbers, less sensitive, Positive numbers, more sensitive.
	PIDConstantsConfig->PitchSensitivity = 0; //0 Default sensitivity Negative numbers, less sensitive, Positive numbers, more sensitive.
	PIDConstantsConfig->RollSensitivity = 0; //0 Default sensitivity Negative numbers, less sensitive, Positive numbers, more sensitive.
	PIDConstantsConfig->YawSensitivity = 0; //0 Default sensitivity Negative numbers, less sensitive, Positive numbers, more sensitive.

	assert_succeed(TWB_SE_WriteS8Pause(K_THROTTLE_SENSITIVITY, PIDConstantsConfig->ThrottleSensitivity));
	assert_succeed(TWB_SE_WriteS8Pause(K_PITCH_SENSITIVITY, PIDConstantsConfig->PitchSensitivity));
	assert_succeed(TWB_SE_WriteS8Pause(K_ROLL_SENSITIVITY, PIDConstantsConfig->RollSensitivity));
	assert_succeed(TWB_SE_WriteS8Pause(K_YAW_SENSITIVITY, PIDConstantsConfig->YawSensitivity));

	PIDConstantsConfig->InitialAltitude = 200;
	PIDConstantsConfig->InitialThrottle = 190;

	assert_succeed(TWB_SE_WriteS8Pause(K_INITIAL_ALTITUDE, PIDConstantsConfig->InitialAltitude));
	assert_succeed(TWB_SE_WriteS8Pause(K_INITIAL_THROTTLE, PIDConstantsConfig->InitialThrottle));

	PIDConstantsConfig->PIDSampleRate = SampleRate_200Hz;
	assert_succeed(TWB_SE_WriteS8Pause(PID_SAMPLE_RATE, PIDConstantsConfig->PIDSampleRate));

	PIDConstantsConfig->FrameConfig = FrameConfig_Cross;
	assert_succeed(TWB_SE_WriteU8Pause(FRAME_CONFIG, PIDConstantsConfig->FrameConfig));

	PIDConstantsConfig->AutoPilotSampleRate = SampleRate_50Hz;
	assert_succeed(TWB_SE_WriteU8Pause(K_AUTOPILOT_SAMPLE_RATE, PIDConstantsConfig->AutoPilotSampleRate));

	PIDConstantsConfig->PitchFollowsRollPID = PitchDoesFollowRollPID;
	assert_succeed(TWB_SE_WriteU8Pause(K_PITCH_FOLLOW_ROLL_PID, PIDConstantsConfig->PitchFollowsRollPID));

	PIDConstantsConfig->ThrottleDeadSpace = 0;
	assert_succeed(TWB_SE_WriteU8Pause(K_THROTTLE_DEAD_SPACE, PIDConstantsConfig->ThrottleDeadSpace));

	SystemStatus->ControlMethod = WiFiControlled;
	SystemStatus->AltitudeHold = AltitudeHold_Off;

	assert_succeed(TWB_SE_WriteU8Pause(COMMON_EEPROM_OFFSET + 0, SystemStatus->ControlMethod));
	assert_succeed(TWB_SE_WriteU8Pause(COMMON_EEPROM_OFFSET + 1, SystemStatus->AltitudeHold));

	return OK;
}

iOpResult_e TWB_GPIO_Arm(void) {
	SystemStatus->ArmedStatus = Armed;
	SystemStatus->GPIOState = GPIOAppState_Ready;

	TWB_PWM_SetNewESCUpdateRate(PIDConstantsConfig->ESCUpdateRate);
	TWB_AutoPilot_Arm();

	TWB_Debug_Print("Armed NiVek!\r\n");

	return OK;
}

iOpResult_e TWB_GPIO_Safe(void) {
	SystemStatus->ArmedStatus = Safe;
	SystemStatus->GPIOState = GPIOAppState_Idle;
	TWB_Debug_Print("Safe NiVek!\r\n");
	TWB_AutoPilot_Safe();

	return OK;
}

