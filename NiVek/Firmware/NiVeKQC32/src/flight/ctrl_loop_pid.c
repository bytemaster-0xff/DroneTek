/*
 * ctrl_loop_pid.c
 *
 *  Created on: Mar 18, 2013
 *      Author: Kevin
 */

#include "flight/ctrl_loop.h"
#include "flight/pid.h"
#include "flight/pwm_source.h"
#include "flight/autopilot.h"
#include "math.h"
#include "common/twb_math.h"


#define TIMX_PRESCALER_32 300
#define TIMX_COUNT 5000

uint8_t  _lastCountInitilized = false;
uint16_t _lastCountTimer3 = 0;

uint16_t currentTimer = 0;
uint16_t deltaCount = 0;

uint16_t countsPer = 0;

#define MIN_THROTTLE_VALUE 0.05f

PID_t *PIDPitch;
PID_t *PIDRoll;
PID_t *PIDYaw;
PID_t *PIDAltitude;

/*Power inputs from 0 - 1.00 */
void __flightCtrlUpdateMotor(Motors_t motor, float power){
	power = twb_clampf(power, 0.0, 1.0f);

	TWB_PWM_SetESCPower(motor, (uint8_t)(power * 255.0f));
}

//We need to integrate the Yaw input over time to be able to control the heading of the quad.
//values coming from the control inputs are +/- 0-50.  Not really enough to integrate with respect to time
//So this value will add smaller values that are rounded and finally applied to the target heading as a
// 1/10 of a degree value.
float _targetHeadingPrecise;

/*
 * Method will take the inputs from the PID controller and apply those to motor power, the motor power
 * is calculated via the auto pilot.
 */
iOpResult_e __twb_ctrl_loop_xfer_function(void){

	/*
	 * TODO: only supports a quad configuration, this is where we would do some fancy trig and distribute corrections across more than 4 motors
	 *
	 * OK, OK, OK, I know what you are thinking...Kevin stay focused on Quad, not multi
	 *
	 * Motor should probably be it's on structure that can be passed to
	 * a set of methods that act upon it to provide an OO approach...Kevin stay focus, not important for V1.0
	 *
	 *
	 * After calculating the PID correction value, this sign on the output
	 * value will follow the sign on the error.
	 *
	 * That is to say if the pitch has a negative value for the error, more power
	 * should be applied to the rear since the front end is higher than it should be.
	 *
	 * Also if the roll error is negative, the output value will also be negative and
	 * more power will be applied to the starboard side to bring that up.
	 *
	 */

	switch(PIDConstantsConfig->FrameConfig)
	{
		/* PIDYaw gets calculated from the autopilot and applied here */

		case FrameConfig_X:{
			float pwrPortFront = InternalState->Throttle + PIDPitch->NormalizedOutput + PIDRoll->NormalizedOutput + PIDYaw->NormalizedOutput;
			float pwrPortRear = InternalState->Throttle - PIDPitch->NormalizedOutput + PIDRoll->NormalizedOutput - PIDYaw->NormalizedOutput ;

			float pwrStarboardFront = InternalState->Throttle + PIDPitch->NormalizedOutput - PIDRoll->NormalizedOutput - PIDYaw->NormalizedOutput;
			float pwrStarboardRear = InternalState->Throttle - PIDPitch->NormalizedOutput - PIDRoll->NormalizedOutput + PIDYaw->NormalizedOutput;

			pwrPortFront = twb_clampf(PIDConstants->k_esc_front * pwrPortFront, 0.0, 1.0f);
			pwrStarboardFront = twb_clampf(PIDConstants->k_esc_left * pwrStarboardFront, 0.0, 1.0f);
			pwrStarboardRear = twb_clampf(PIDConstants->k_esc_right * pwrStarboardRear, 0.0, 1.0f);
			pwrPortRear = twb_clampf(PIDConstants->k_esc_rear * pwrPortRear, 0.0, 1.0f);

			__flightCtrlUpdateMotor(Motor_PortFront, pwrPortFront);
			__flightCtrlUpdateMotor(Motor_StarboardRear, pwrStarboardRear);
			__flightCtrlUpdateMotor(Motor_StarboardFront, pwrStarboardFront);
			__flightCtrlUpdateMotor(Motor_PortRear, pwrPortRear);

			RollPIDDetails->Power1 = (uint8_t)(pwrPortFront * 255.0f);
			RollPIDDetails->Power2 = (uint8_t)(pwrStarboardFront * 255.0f);

			PitchPIDDetails->Power1 = (uint8_t)(pwrStarboardRear * 255.0f);
			PitchPIDDetails->Power2 = (uint8_t)(pwrPortRear * 255.0f);
		}
		break;
		case FrameConfig_Cross: {
			float pwrFront = InternalState->Throttle + PIDPitch->NormalizedOutput + PIDYaw->NormalizedOutput;
			float pwrRear = InternalState->Throttle - PIDPitch->NormalizedOutput + PIDYaw->NormalizedOutput ;

			float pwrStarboard = InternalState->Throttle - PIDRoll->NormalizedOutput - PIDYaw->NormalizedOutput;
			float pwrPort = InternalState->Throttle + PIDRoll->NormalizedOutput  - PIDYaw->NormalizedOutput;

			pwrFront = twb_clampf(PIDConstants->k_esc_front * pwrFront, 0.0, 1.0f);
			pwrStarboard = twb_clampf(PIDConstants->k_esc_right * pwrStarboard, 0.0, 1.0f);
			pwrPort = twb_clampf(PIDConstants->k_esc_left * pwrPort, 0.0, 1.0f);
			pwrRear= twb_clampf(PIDConstants->k_esc_rear * pwrRear, 0.0, 1.0f);

			__flightCtrlUpdateMotor(Motor_Front, pwrFront);
			__flightCtrlUpdateMotor(Motor_Rear, pwrRear);
			__flightCtrlUpdateMotor(Motor_Starboard, pwrStarboard);
			__flightCtrlUpdateMotor(Motor_Port, pwrPort);

			RollPIDDetails->Power1 = (uint8_t)(pwrStarboard * 255.0f);
			RollPIDDetails->Power2 = (uint8_t)(pwrPort * 255.0f);

			PitchPIDDetails->Power1 = (uint8_t)(pwrFront * 255.0f);
			PitchPIDDetails->Power2 = (uint8_t)(pwrRear * 255.0f);
		}
		break;
	}

	return OK;
}


/*
 * Quad Rotation Axis
 *
 * Pitch, Front/Rear angle, pitch down is negative
 *
 * Roll, Left/Right Angle, when looking from behind right side going down is a positive angle
 *
 *
 * View from Behind
 *
 * Port (left)	Starboard (right)
 * 		\
 *  	 \
 *   	  \
 *    	   O
 *    		\
 *      	 \
 *       	  \
 *
 *   Approximately +60 degree bank/roll angle
 *
 *  Front          Rear
 *
 *           /
 *          /
 *         /
 *        O
 *       /
 *      /
 *     /
 *
 * Pitched down or approximately a -60 degree angle
 *
 */

bool _transitionedToFlightMode = false;

/*
 * Well, really doesn't update telemetry, but rather than structures that
 * are transmitted via telemetry.
 */
void __twb_gpio_update_telemetry(void){
	Targets->TargetAltitude = (int16_t)InternalState->TargetAltitude;
	Targets->TargetHeading = (int16_t)InternalState->TargetHeading;
	Targets->TargetPitch = (int8_t)InternalState->TargetPitch;
	Targets->TargetRoll = (int8_t)InternalState->TargetRoll;
	Targets->Throttle =  (uint8_t)(InternalState->Throttle * 255.0f);

	PitchPIDDetails->Target = Targets->TargetPitch;
	RollPIDDetails->Target = Targets->TargetRoll;

	PitchPIDDetails->Angle = SensorData->Pitch;
	PitchPIDDetails->TargetRate = (int16_t) (PIDPitch->TargetRate * 100.0f);
	PitchPIDDetails->Rate = (int16_t) (PIDPitch->ThisRate * 100.0f);
	PitchPIDDetails->Derivative = (int16_t)(PIDPitch->LastDerivative * 100.0f);
	PitchPIDDetails->ErrorSigma = (int16_t)(PIDPitch->RateSigmaError * 100.0f);

	PitchPIDDetails->P_SteadyComponent = (int16_t) (PIDPitch->pSteadyComponent * 10.0f);
	PitchPIDDetails->I_SteadyComponent = (int16_t) (PIDPitch->iSteadyComponent * 10.0f);

	PitchPIDDetails->P_RateComponent = (int16_t) (PIDPitch->pRateComponent * 1000.0f);
	PitchPIDDetails->I_RateComponent = (int16_t) (PIDPitch->iRateComponent * 1000.0f);
	PitchPIDDetails->D_RateComponent = (int16_t) (PIDPitch->dRateComponent * 1000.0f);

	RollPIDDetails->Angle = SensorData->Roll;
	RollPIDDetails->TargetRate = (int16_t) (PIDRoll->TargetRate * 100.0f);
	RollPIDDetails->Rate = (int16_t) (PIDRoll->ThisRate * 100.0f);
	RollPIDDetails->Derivative = (int16_t) (PIDRoll->LastDerivative * 100.0f);
	RollPIDDetails->ErrorSigma = (int16_t) (PIDRoll->RateSigmaError * 100.0f);

	RollPIDDetails->P_SteadyComponent = (int16_t)(PIDRoll->pSteadyComponent * 10.0f);
	RollPIDDetails->I_SteadyComponent = (int16_t)(PIDRoll->iSteadyComponent * 10.0f);

	RollPIDDetails->P_RateComponent = (int16_t)(PIDRoll->pRateComponent * 1000.0f);
	RollPIDDetails->I_RateComponent = (int16_t)(PIDRoll->iRateComponent * 1000.0f);
	RollPIDDetails->D_RateComponent = (int16_t)(PIDRoll->dRateComponent * 1000.0f);
}

iOpResult_e __twb_gpio_set_safe(void){
	TWB_PWM_SetESCPower(Motor_PortFront, 0);
	TWB_PWM_SetESCPower(Motor_StarboardRear, 0);
	TWB_PWM_SetESCPower(Motor_PortRear, 0);
	TWB_PWM_SetESCPower(Motor_StarboardFront, 0);
	TWB_PID_Reset(PIDPitch);
	TWB_PID_Reset(PIDRoll);
	TWB_PID_Reset(PIDYaw);
	TWB_PID_Reset(PIDAltitude);

	PitchPIDDetails->Target = Targets->TargetPitch;
	RollPIDDetails->Target = Targets->TargetRoll;

	PitchPIDDetails->Angle = SensorData->Pitch;
	RollPIDDetails->Angle = SensorData->Roll;

	PitchPIDDetails->Power1 = 0;
	PitchPIDDetails->Power2 = 0;

	RollPIDDetails->Power1 = 0;
	RollPIDDetails->Power2 = 0;

	if(_transitionedToFlightMode != false){
		TWB_AutoPilot_TransitionToFlightMode();
		_transitionedToFlightMode = false;
	}

	if(SystemStatus->ArmedStatus == Armed)
		SystemStatus->GPIOState = GPIOAppState_Ready;
	else
		SystemStatus->GPIOState = GPIOAppState_Idle;

	InternalState->TargetAltitude = 0.0f;
	InternalState->Throttle = 0.0f;

	__twb_gpio_update_telemetry();

	return OK;
}

float __ctrlLoopDeltaT;


iOpResult_e TWB_GPIO_CtrlLoop_Exec(float deltaT){
	__ctrlLoopDeltaT = deltaT;

	/* TODO: Ugghhh...hard coded at 200Hz, need to confirm that this is passing 0.005 before uncommenting */
	deltaT = 0.005;

	InternalState->TargetPitch = (float)SystemStatus->ControlMethod == WiFiControlled ? WiFiPitch : PWMIn->Pitch; /*Pitch will be between 0 - +/-90 */
	InternalState->TargetRoll = (float)SystemStatus->ControlMethod == WiFiControlled ? WiFiRoll : PWMIn->Roll;    /*Roll  will be between 0 - +/-90 */

	//If we are in motor config state, the motor is manually controlled
	//don't interfere with it here.
	if(SystemStatus->GPIOState == GPIOAppState_MotorConfig)
		return OK;

	/* In safe mode, ALWAYS set everything to zero */
	if(SystemStatus->ArmedStatus == Safe)
		return __twb_gpio_set_safe();

	//First off get the values from either the RC controller or incoming telemetry channel and make those to the target inputs.
	//These are in full degrees, not scaled by any decimal points.
	if(SystemStatus->AltitudeHold == AltitudeHold_Off) /* If not launching set the throttle from the input of the primary control source */
		InternalState->Throttle = (float)(SystemStatus->ControlMethod == WiFiControlled ? WiFiThrottle : PWMIn->Throttle) / 255.0f; /* Initial source will always be between 0 and 255 */
	/* else --> If we are still in the launching state, the throttle is set in the auto pilot. */

	if((InternalState->Throttle < MIN_THROTTLE_VALUE) && SystemStatus->GPIOState != GPIOAppState_Launching) /* If throttle is < 5% and we aren't in the launch state, don't spin up the motors */
		return __twb_gpio_set_safe();

	/* If we make it here, it means we are attempting to fly the quad, it means we have already "taken-off",
	 * or we have moved the throttle past 5%, either way the first time in capture the initial heading */
	if(_transitionedToFlightMode == false){
		TWB_AutoPilot_TransitionToFlightMode();
		_transitionedToFlightMode = true;
	}

	/*
	 * Negative error on pitch means that the front end is pointing up or 0 - (+30)
	 * Positive error on pitch means that the front end is pointing down or 0 - (-30)
	 */
	PIDPitch->ThisError = InternalState->TargetPitch - InternalState->Pitch;
	PIDPitch->ThisRate = InternalState->PitchRateDPS;
	PIDPitch->DeltaT = deltaT;
	TWB_PID_Update(PIDPitch);

	/*
	 * Negative error means that the starboard side is tipped down or 0 - (+30)
	 * Positive error means that the starboard side is tipped up or 0 - (-30)
	 */
	PIDRoll->ThisError = InternalState->TargetRoll - InternalState->Roll;
	PIDRoll->ThisRate = InternalState->RollRateDPS;
	PIDRoll->DeltaT = deltaT;
	TWB_PID_Update(PIDRoll);

	__twb_ctrl_loop_xfer_function();

	__twb_gpio_update_telemetry();

	return OK;
}
