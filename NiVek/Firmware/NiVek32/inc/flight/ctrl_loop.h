/*
 * ctrl_loop.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#ifndef CTRL_LOOP_H_
#define CTRL_LOOP_H_

#include "common/twb_common.h"
#include "commo/msg_parser.h"
#include "commo/commo.h"
#include "twb_eeprom.h"


/* Outgoing Message Types */

#define MSG_Out_PIDConstants 0x20
#define MSG_Set_PIDConstant 0x21
#define MSG_Out_StickCalibration 0x30

/* End Outgoing Message Types */


/* Address of constants used for PID control */
#define K_PITCH_RATE_P_ADDR 			  PID_EEPROM_OFFSET + 0
#define K_PITCH_RATE_I_ADDR 			  PID_EEPROM_OFFSET + 2
#define K_PITCH_RATE_D_ADDR 			  PID_EEPROM_OFFSET + 4
#define K_PITCH_RATE_I_DECAY_FACTOR_ADDR  PID_EEPROM_OFFSET + 6
#define K_PITCH_RATE_I_MAX_ADDR  	  PID_EEPROM_OFFSET + 8
#define K_PITCH_STEADY_P_ADDR  	  PID_EEPROM_OFFSET + 10
#define K_PITCH_STEADY_I_ADDR  	  PID_EEPROM_OFFSET + 12
#define K_PITCH_STEADY_I_MAX_ADDR  	  PID_EEPROM_OFFSET + 14

#define K_ROLL_RATE_P_ADDR 			  PID_EEPROM_OFFSET + 16
#define K_ROLL_RATE_I_ADDR 			  PID_EEPROM_OFFSET + 18
#define K_ROLL_RATE_D_ADDR 			  PID_EEPROM_OFFSET + 20
#define K_ROLL_RATE_I_DECAY_FACTOR_ADDR   PID_EEPROM_OFFSET + 22
#define K_ROLL_RATE_I_MAX_ADDR		  PID_EEPROM_OFFSET + 24
#define K_ROLL_STEADY_P_ADDR  	  		  PID_EEPROM_OFFSET + 26
#define K_ROLL_STEADY_I_ADDR  	      PID_EEPROM_OFFSET + 28
#define K_ROLL_STEADY_I_MAX_ADDR  	      PID_EEPROM_OFFSET + 30

#define K_YAW_RATE_P_ADDR 			  PID_EEPROM_OFFSET + 32
#define K_YAW_RATE_I_ADDR 			  PID_EEPROM_OFFSET + 34
#define K_YAW_RATE_D_ADDR 			  PID_EEPROM_OFFSET + 36
#define K_YAW_RATE_I_DECAY_FACTOR_ADDR    PID_EEPROM_OFFSET + 38
#define K_YAW_RATE_I_MAX_ADDR 		  PID_EEPROM_OFFSET + 40
#define K_YAW_STEADY_P_ADDR  	  		  PID_EEPROM_OFFSET + 42
#define K_YAW_STEADY_I_ADDR  	      PID_EEPROM_OFFSET + 44
#define K_YAW_STEADY_I_MAX_ADDR  	      PID_EEPROM_OFFSET + 46


#define K_ALTITUDE_RATE_P_ADDR 			PID_EEPROM_OFFSET + 48
#define K_ALTITUDE_RATE_I_ADDR 			PID_EEPROM_OFFSET + 50
#define K_ALTITUDE_RATE_D_ADDR 			PID_EEPROM_OFFSET + 52
#define K_ALTITUDE_RATE_I_DECAY_FACTOR_ADDR PID_EEPROM_OFFSET + 54
#define K_ALTITUDE_RATE_I_MAX_ADDR 		PID_EEPROM_OFFSET + 56
#define K_ALTITUDE_STEADY_P_ADDR  	  		  PID_EEPROM_OFFSET + 58
#define K_ALTITUDE_STEADY_I_ADDR  	      PID_EEPROM_OFFSET + 60
#define K_ALTITUDE_STEADY_I_MAX_ADDR  	      PID_EEPROM_OFFSET + 62

#define K_ESC_FRONT_ADDR 			PID_EEPROM_OFFSET + 64
#define K_ESC_LEFT_ADDR 			PID_EEPROM_OFFSET + 66
#define K_ESC_RIGHT_ADDR 				PID_EEPROM_OFFSET + 68
#define K_ESC_REAR_ADDR 				PID_EEPROM_OFFSET + 70

#define K_ESC_UPDATE_RATE 			PID_EEPROM_OFFSET + 72

#define K_PITCH_STABLE_BAND 		PID_EEPROM_OFFSET + 73
#define K_ROLL_STABLE_BAND 			PID_EEPROM_OFFSET + 74
#define K_HEADING_STABLE_BAND 		PID_EEPROM_OFFSET + 75
#define K_ALTITUDE_STABLE_BAND 		PID_EEPROM_OFFSET + 76

#define PID_SAMPLE_RATE 			PID_EEPROM_OFFSET + 77

#define K_PITCH_SENSITIVITY 		PID_EEPROM_OFFSET + 78
#define K_ROLL_SENSITIVITY 			PID_EEPROM_OFFSET + 79
#define K_YAW_SENSITIVITY 			PID_EEPROM_OFFSET + 80

#define K_PITCH_FILTER_D 			PID_EEPROM_OFFSET + 81
#define K_ROLL_FILTER_D 			PID_EEPROM_OFFSET + 82
#define K_YAW_FILTER_D 				PID_EEPROM_OFFSET + 83
#define K_ALTITUDE_FILTER_D 		PID_EEPROM_OFFSET + 84

#define FRAME_CONFIG           		PID_EEPROM_OFFSET + 85

#define K_INITIAL_THROTTLE    		PID_EEPROM_OFFSET + 86
#define K_INITIAL_ALTITUDE    		PID_EEPROM_OFFSET + 87

#define K_AUTOPILOT_SAMPLE_RATE 	PID_EEPROM_OFFSET + 88

#define K_PITCH_FOLLOW_ROLL_PID 	PID_EEPROM_OFFSET + 89

/* End Constants used for PID control */

extern int16_t PitchError;
extern int16_t RollError;


iOpResult_e TWB_GPIO_CtrlLoop_SetPitchRollPID(void);
iOpResult_e TWB_GPIO_CtrlLoop_Init(void);
iOpResult_e TWB_GPIO_CtrlLoop_ResetDefaults(void);
iOpResult_e TWB_GPIO_CtrlLoop_Exec(float deltaT);

iOpResult_e TWB_CtrlLoop_HandleMessage(TWB_Commo_Message_t *msg);

float __ctrlLoop_resolve_DFilter(uint8_t setting);

void TWB_GPIO_SendPIDConfig(void);

iOpResult_e TWB_GPIO_Arm(void);
iOpResult_e TWB_GPIO_Safe(void);

void TWB_Targets_QueueMsg(void);

#define P_STEADY_SCALER 100.0f
#define I_STEADY_SCALER 100.0f

#define P_RATE_SCALER 10000.0f
#define I_RATE_SCALER 10000.0f
#define D_RATE_SCALER 10000.0f

#define MAX_I_SCALER 10.0f

#define SENSTIVITY_SCALER 10.0f

#define ESC_SCALER 100.0f
#define STABLE_BAND_SCALER 10.0f

#define DECAY_FACTOR_SCALER 100.0f

#endif /* CTRL_LOOP_H_ */
