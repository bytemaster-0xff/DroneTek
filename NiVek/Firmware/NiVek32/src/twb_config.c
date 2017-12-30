/*
 * twb_config.h
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "twb_config.h"

//TODO: This sucks/hacks, when including math lib, linker expects __errorno to be defined.  So do it here!
uint16_t __errno;

SensorDiagData_t *_commonDiagBuffer;

BatteryCondition_t *BatteryCondition;
GPSData_t *GPSData;
PIDConstantsConfig_t *PIDConstantsConfig;
SensorData_t *SensorData;
SensorConfig_t *SensorConfig;
MotorStatus_t *MotorStatus;
SystemStatus_t *SystemStatus;
TargetsData_t *Targets;
PIDSensorData_t *PIDSensorData;

PIDTuning_t *RollPIDDetails;
PIDTuning_t *PitchPIDDetails;

AltitudeData_t *AltitudeData;

InternalState_t *InternalState;
PIDConstants_t *PIDConstants;

uint8_t WiFiThrottle;
int8_t WiFiYaw;
int8_t WiFiPitch;
int8_t WiFiRoll;

int8_t WiFiAux1;
int8_t WiFiAux2;

GPSReading_t *HomeGPSReading;
GPSReading_t *CurrentGPSReading;


/* Internal State Variables */
WiFiConnected_t WiFiIsConnected;
