/*
 * kalman.c
 *
 *  Created on: Jan 4, 2013
 *      Author: kevinw
 */

#include "sensors/fusion/kalman.h"
#include "sensors/snsr_main.h"

iOpResult_e TWB_SNSR_Kalman_Init(void){
	KalmanSnsrFusion->SampleRate = SensorConfig->Kalman_Enabled;
	KalmanSnsrFusion->IsEnabled = SensorConfig->Kalman_SampleRate;

	return OK;
}

iOpResult_e TWB_SNSR_Kalman_ResetDefaults(void) {

	return OK;
}

iOpResult_e TWB_SNSR_Kalman_UpdateSetting(uint16_t addr, uint8_t value){

	return OK;
}

void TWB_SNSR_Kalman_SendDiag(void){

}


iOpResult_e TWB_SNSR_Kalman_Process(float deltaT){

	return OK;
}

