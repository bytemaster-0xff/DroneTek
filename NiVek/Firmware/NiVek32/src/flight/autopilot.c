/*
 * autopilot.c
 *
 *  Created on: Jul 4, 2013
 *      Author: Kevin
 */

#include "flight/autopilot.h"
#include "sensors/snsr_main.h"
#include "sensors/bmp085.h"
#include "sensors/ms5611.h"
#include "flight/pid.h"
#include "common/twb_math.h"
#include "sensors/fusion/complementary.h"

#define MAX_YAW_DEGREE_PER_SECONDS 10.0f //25 degrees per second with YAW fully slammed with normal sensitivity.

float _gpsLaunchAltitude = 0.0f;
float _bmp085_pressureLaunchAltitude = 0.0f;
float _ms5611_pressureLaunchAltitude = 0.0f;

AutoPilotState_e AutoPilotState;

iOpResult_e TWB_AutoPilot_Start(void);
iOpResult_e TWB_AutoPilot_Pause(void);
iOpResult_e TWB_AutoPilot_Resume(void);
iOpResult_e TWB_AutoPilot_Stop(void);

iOpResult_e TWB_AutoPilot_Init(void){
	AutoPilotAction->Read = &TWB_AutoPilot_Exec;

	AutoPilotAction->Start = &TWB_AutoPilot_Start;
	AutoPilotAction->Pause = &TWB_AutoPilot_Pause;
	AutoPilotAction->Resume = &TWB_AutoPilot_Resume;
	AutoPilotAction->Stop = &TWB_AutoPilot_Stop;

	//Always service the auto pilot it will hand the case where it has nothing to do.
	AutoPilotAction->IsEnabled = true;
	AutoPilotAction->SampleRate = SampleRate_20Hz;

	AutoPilotState = AutoPilot_Idle;

	return OK;
}

void TWB_AutoPilot_QueueAltitudeMsg(void) {
	TWB_Commo_SendMessage(MOD_Sensor, MSG_ALTITUDE_DATA_ID, (uint8_t *)AltitudeData, sizeof(AltitudeData_t));
}

iOpResult_e TWB_AutoPilot_Arm(void){

	return OK;
}

iOpResult_e TWB_AutoPilot_Safe(void){

	return OK;
}

iOpResult_e TWB_AutoPilot_ResetDefaults(void) {

	return OK;
}

iOpResult_e __twb_gpio_manage_yaw(float deltaT){
	/* To turn the unit we read values from the Yaw or rudder stick
	 * These values are +/- 90, this really isn't an absolute heading but the longer you push it in one
	 * direction, the faster it should turn in that direction + is to the right, - is to the left.
	 *
	 * So if we say that the full scale rotation will be 10 degrees a second (pulled that number out of my ass will probably revisit)
	 *
	 * Probably some major opportunities for optimization, but will get working first.
	 *
	 * TODO: Optimization opportunity;
	 */

	/* So with a control loop hz of 200, and a MAX_YAW_DEGREE_PER_SECONDS of 25 (check constant)
	 * a percentage yaw of 0.25 (25% of stick), would yield 25 * 0.25 / 200 or for every control loop
	 * we add in 0.03 degrees, or with 25% stick we should be turning at a rate of 6.25 degrees a second
	 *
	 * The MAX_YAW_DEGREE_PER_SECONDS really is the course measure that dictates the control inputs mapped to
	 * the rate of change of the heading, on top of that we also have sensitivity for fine tuning.
	 */

	float percentageYaw = ((float)Targets->Yaw) / 90.0f; //values can be between +/- 90, so this will give us the percentage of the stick in a direction
	InternalState->TargetHeading += percentageYaw * MAX_YAW_DEGREE_PER_SECONDS * deltaT;

	/* Ensure our target heading is between 0 and 360 */
	if(InternalState->TargetHeading < 0.0f)
		InternalState->TargetHeading = 360.0f + InternalState->TargetHeading; //Add the negative number, always keep heading positive and between 360.
	else if(InternalState->TargetHeading > 360.0f)
		InternalState->TargetHeading -= 360.f;

	float delta = InternalState->Heading- InternalState->TargetHeading;

	if(delta > 180.0f)
		PIDYaw->ThisError = -(360.0f - delta);
	else if(delta < -180.0f)
		PIDYaw->ThisError = 360.0f + delta;
	else
		PIDYaw->ThisError = delta;

	/* Heading gets calculated here, but then is applied in the transfer function in the primary control loop */
	TWB_PID_Update(PIDYaw);

	return OK;
}


iOpResult_e TWB_Autopilot_DetermineAltitude(CompFilter_t *filter){
	if(ComplementaryFilterConfig->Sensor1Altitude == SensorNone)
		return OK;

	float sensor1 = 0.0f;
	float sensor2 = 0.0f;

	switch(ComplementaryFilterConfig->Sensor1Altitude){
		case AltGPS: sensor1 = AltGPSSensor->Z - _gpsLaunchAltitude; break;
		case AltMS5611: sensor1 = PressureMS5611Sensor->Z - _ms5611_pressureLaunchAltitude; break;
		case AltBMP085: sensor1 = PressureBMP085Sensor->Z -_bmp085_pressureLaunchAltitude; break;
		case AltSonar: sensor1 = SonarSensor->Z; break;
		default: sensor1 = 0; break;
	}

	switch(ComplementaryFilterConfig->Sensor2Altitude){
			case AltGPS: sensor2 = AltGPSSensor->Z - _gpsLaunchAltitude; break;
			case AltMS5611: sensor2 = PressureMS5611Sensor->Z - _ms5611_pressureLaunchAltitude; break;
			case AltBMP085: sensor2 = PressureBMP085Sensor->Z -_bmp085_pressureLaunchAltitude; break;
			case AltSonar: sensor2 = SonarSensor->Z; break;
			default: sensor2 = 0; break;
		}

	if(ComplementaryFilterConfig->Sensor2Altitude == SensorNone)
		InternalState->AltitudeM = sensor1;
	else if( ComplementaryFilterConfig->Sensor1Altitude == AltGPS && filter->Sensor1->X == 0.0)
		InternalState->AltitudeM = sensor2;
	else if( ComplementaryFilterConfig->Sensor2Altitude == AltGPS && filter->Sensor2->X == 0.0)
		InternalState->AltitudeM = sensor1;
	else
		InternalState->AltitudeM = (sensor1 * filter->Period1) + (sensor2 * filter->Period2);

	TWB_Filter_Median_Apply_Float(filter->Filter, InternalState->AltitudeM);
	InternalState->AltitudeM = filter->Filter->Current;

	return OK;
}


iOpResult_e __twb_autoPilot_manage_throttle(float deltaT){
	if(SystemStatus->AltitudeHold != AltitudeHold_On) /* Nothing to do, throttle is controlled manually */
		return OK; /* No change */

	if(SystemStatus->GPIOState == GPIOAppState_Launching){
	   if(SensorData->AltitudeM < PIDConstantsConfig->InitialAltitude)
		   InternalState->Throttle = (float)(PIDConstantsConfig->InitialThrottle / 255.0f); /* Give initial throttle until we reach initial altitude */
	   else
		   SystemStatus->GPIOState = GPIOAppState_Flying; /* Otherwise change the mode to flying */

	   return OK;
	}

	//Altitude will be in actual CM
	PIDAltitude->ThisError = InternalState->TargetAltitude - InternalState->AltitudeM;

	TWB_PID_Update(PIDAltitude); /* If we are outside or stable band calculate the correction */

	InternalState->Throttle += (PIDAltitude->Output);

	InternalState->Throttle = twb_clampf(InternalState->Throttle, 0.0, 1.0f);

	//TODO: not happy w/ this algorithm :(
	if(SystemStatus->GPIOState == GPIOAppState_Landing){
		if(InternalState->Throttle < 0.20 || InternalState->AltitudeM < 0.5f){
			SystemStatus->GPIOState = GPIOAppState_Ready;
			InternalState->Throttle = 0;
		}

		return OK;
	}

	return OK;
}

iOpResult_e TWB_AutoPilot_TransitionToFlightMode() {
	InternalState->TargetHeading = InternalState->Heading;

	Targets->TargetHeading = (int16_t)InternalState->Heading;

	_gpsLaunchAltitude = CurrentGPSReading->AltitudeM;
	_bmp085_pressureLaunchAltitude = PressureBMP085Sensor->Z;
	_ms5611_pressureLaunchAltitude = PressureMS5611Sensor->Z;

	/* First time through, if state == launching then set the target altitude */
	if(SystemStatus->GPIOState == GPIOAppState_Launching){
		Targets->TargetAltitude = PIDConstantsConfig->InitialAltitude;
		InternalState->TargetAltitude = (float)PIDConstantsConfig->InitialAltitude;
	}
	else
		SystemStatus->GPIOState = GPIOAppState_Flying; /* We have throttle, are armed and not launching - manual flight */


	//Will already be filtered.
	TWB_IMU_BMP085_Zero(0,0);
	return OK;
}

iOpResult_e TWB_AutoPilot_TransitionFromFlightMode() {

	return OK;
}

iOpResult_e TWB_AutoPilot_Start() {

	return OK;
}

iOpResult_e TWB_AutoPilot_Stop() {
	AutoPilotState = AutoPilot_Idle;
	return OK;
}

iOpResult_e TWB_AutoPilot_Pause() {
	AutoPilotState = AutoPilot_Paused;
	return OK;
}

iOpResult_e TWB_AutoPilot_Resume() {

	return OK;
}

float __autopilotDeltaT;

iOpResult_e TWB_AutoPilot_Exec(float deltaT) {
	__autopilotDeltaT = deltaT;

	/* In safe mode, ALWAYS set everything to zero */
	if(SystemStatus->ArmedStatus == Safe){
		InternalState->Throttle = 0.0f;
		return OK;
	}

 	/*
 	 * Yaw input  will be between 0 - +/-90 and will control the current rate of rotation, more yaw more rotation, yaw simply updates the target heading, then the PID will adjust accordingly.
 	 *
 	 * Yaw will simply apply a factor to speed/slow counter rotating props to allow the copter to be rotated.
 	 *
 	 * Eventually we would like to say, turn to heading 35 degrees, but that's in a future version.
 	 */
	Targets->Yaw = SystemStatus->ControlMethod == WiFiControlled ? WiFiYaw : PWMIn->Yaw;

	assert_succeed(__twb_gpio_manage_yaw(deltaT));
	assert_succeed(__twb_autoPilot_manage_throttle(deltaT));

	return OK;
}


iOpResult_e TWB_AutoPilot_UpdateSettings(uint16_t addr, uint8_t value){


	return OK;
}
