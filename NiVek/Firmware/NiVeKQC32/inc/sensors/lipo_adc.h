/*
 * lipo_adc.h
 *
 *  Created on: Oct 24, 2012
 *      Author: kevinw
 */

#ifndef LIPO_ADC_H_
#define LIPO_ADC_H_

#include "common/twb_common.h"

iOpResult_e TWB_ADC_Init(void);
iOpResult_e TWB_ADC_ResetDefaults(void);
iOpResult_e TWB_ADC_UpdateSetting(uint16_t addr, uint8_t value);
iOpResult_e TWB_ADC_Read(float deltaT);
iOpResult_e TWB_ADC_IRQ(float deltaT);

void TWB_ADC_QueueMsg(void);
void TWB_ADC_SendDiag(void);

#endif /* LIPO_ADC_H_ */
