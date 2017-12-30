/*
 * pid.c
 *
 *  Created on: May 16, 2013
 *      Author: Kevin
 *
 *      Adapted from ArduDrone on GitHub
 *
 *      https://github.com/diydrones/ardupilot/blob/master/libraries/AC_PID/AC_PID.cpp
 */

#include "flight/pid.h"
#include "common/twb_common.h"
#include "common/twb_math.h"
#include <math.h>

iOpResult_e __twb_PID_get_steady_p(PID_t *pid){
	if(pid->k_RateP == 0.0f)
		return OK;

	pid->pSteadyComponent = pid->ThisError * pid->k_SteadyP;

	return OK;
}

iOpResult_e __twb_PID_get_steady_i(PID_t *pid){
	if(pid->k_SteadyI == 0.0f)
		return 0.0f;

	//Integrate
	pid->SteadySigmaError += (float)pid->ThisError * pid->DeltaT;

	//Clamp on max sigma error
	pid->SteadySigmaError = twb_clampf(pid->SteadySigmaError, -pid->Steady_MaxSigmaError, pid->Steady_MaxSigmaError);
	pid->iSteadyComponent = pid->SteadySigmaError * pid->k_SteadyI;

	return OK;
}

iOpResult_e __twb_PID_get_rate_p(PID_t *pid){
	if(pid->k_RateP == 0.0f)
		return OK;

	pid->pRateComponent = pid->ThisRateError * pid->k_RateP;

	return OK;
}


iOpResult_e __twb_PID_get_rate_i(PID_t *pid){
	if(pid->k_RateI == 0.0f)
		return 0.0f;

	/*
	 * If this is set, take off a proportion of this sigma each time, this will
	 * let the error total naturally decay back to zero once the number is small.
	 */
	if(pid->IDecayFactor != 0.0f)
		pid->RateSigmaError -= (float)pid->RateSigmaError * pid->IDecayFactor * pid->DeltaT;

	//Integrate
	pid->RateSigmaError += pid->ThisRateError * pid->DeltaT;

	//Clamp on max sigma error
	pid->RateSigmaError = twb_clampf(pid->RateSigmaError, -pid->Rate_MaxSigmaError, pid->Rate_MaxSigmaError);
	pid->iRateComponent = pid->RateSigmaError * pid->k_RateI;

	return OK;
}

float __twb_PID_get_rate_d(PID_t *pid){
    if (pid->k_RateD == 0.0f)
    	return 0.0f;

    float derivative;
	if (pid->LastRateError != 0.0f) {
		/* Old */
		// calculate instantaneous derivative
		derivative = pid->ThisRateError - pid->LastRateError;

		// discrete low pass filter, cuts out the
		// high frequency noise that can drive the controller crazy
		if(pid->DerivativeFilter != 0.0 && pid->LastDerivative != 0.0f)
			derivative = pid->LastDerivative + (pid->DeltaT / (pid->DerivativeFilter + pid->DeltaT)) * (derivative - pid->LastDerivative);

		pid->LastDerivative = derivative;

		pid->dRateComponent = pid->k_RateD * derivative;
	}
	else
		pid->dRateComponent = 0.0f;

	return OK;
}

iOpResult_e TWB_PID_Update(PID_t *pid){
	assert_succeed(__twb_PID_get_steady_p(pid));
	assert_succeed(__twb_PID_get_steady_i(pid));

	pid->TargetRate = pid->pSteadyComponent + pid->iSteadyComponent;
	pid->ThisRateError = pid->TargetRate - pid->ThisRate;

	assert_succeed(__twb_PID_get_rate_p(pid));
	assert_succeed(__twb_PID_get_rate_i(pid));
	assert_succeed(__twb_PID_get_rate_d(pid));

	pid->Output = pid->pRateComponent + pid->iRateComponent + pid->dRateComponent;
	pid->NormalizedOutput = pid->Output;
	//pid->NormalizedOutput = (InternalState->Throttle * 1.0f) * pid->Output;

	pid->LastRateError = pid->ThisRateError;
	pid->LastError = pid->ThisError;

	return OK;
}

iOpResult_e TWB_PID_Reset(PID_t *pid){
    pid->RateSigmaError = 0.0f;
	// mark derivative as invalid
    pid->LastError = 0.0;
    pid->LastDerivative = 0.0;
    pid->Output = 0.0f;
    pid->NormalizedOutput = 0.0f;

    return OK;
}
