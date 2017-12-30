/*
 * lipo_adc.c
 *
 *  Created on: Oct 24, 2012
 *      Author: kevinw
 */


#include "sensors/lipo_adc.h"
#include "twb_eeprom.h"

#include "twb_config.h"

#include "main.h"

#include "common/twb_debug.h"
#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "commo/commo.h"
#include "commo/message_ids.h"

#include "filters/filter.h"
#include "sensors/snsr_main.h"

#include <stdlib.h>
#include <string.h>

//ADC Inputs
// Pin  8 - Cell 1
// Pin  9 - Cell 2
// Pin 10 - Cell 3



#define ADC_DEFAULT_SAMPLING_RATE       ADC_SampleTime_144Cycles


Filter_t __lipoAdcFilter1;
Filter_t __lipoAdcFilter2;
Filter_t __lipoAdcFilter3;

DMA_InitTypeDef       DMA_InitStructure;

ADC_TypeDef *__lipoADC;

iOpResult_e TWB_ADC_Init(void) {
	NVIC_InitTypeDef nvicInit;

	assert_succeed(TWB_SE_ReadU8(LIPO_ADC_EEPROM_OFFSET + 0, &SensorConfig->LIPO_ADC_Enabled));
	assert_succeed(TWB_SE_ReadU8(LIPO_ADC_EEPROM_OFFSET + 1, &SensorConfig->LIPO_ADC_FilterOption));
	assert_succeed(TWB_SE_ReadU8(LIPO_ADC_EEPROM_OFFSET + 2, &SensorConfig->LIPO_ADC_SampleRate));

	ADCLipoSensor->SampleRate = SensorConfig->LIPO_ADC_SampleRate;
	ADCLipoSensor->IsEnabled = SensorConfig->LIPO_ADC_Enabled;

	ADCLipoSensor->IsEnabled = Enabled;
	ADCLipoSensor->Read = &TWB_ADC_Read;

	__lipoAdcFilter1.FilterType = SensorConfig->LIPO_ADC_FilterOption;
	__lipoAdcFilter2.FilterType = SensorConfig->LIPO_ADC_FilterOption;
	__lipoAdcFilter3.FilterType = SensorConfig->LIPO_ADC_FilterOption;

	ADC_DeInit();

	/* ADC1 Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	BatteryCondition->Cell1Battery = 0;
	BatteryCondition->Cell2Battery = 0;
	BatteryCondition->Cell3Battery = 0;

	ADC_InitTypeDef adcInit;
	adcInit.ADC_Resolution = ADC_Resolution_12b;
	adcInit.ADC_ScanConvMode = DISABLE;
	adcInit.ADC_ContinuousConvMode = DISABLE;
	adcInit.ADC_DataAlign = ADC_DataAlign_Right;
	adcInit.ADC_NbrOfConversion = 4;
	adcInit.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

	ADC_Init(ADC1, &adcInit);

	/* Dunno what these are for, but these are from the sample code */
	ADC_CommonInitTypeDef adCommon;
	adCommon.ADC_Mode = ADC_Mode_Independent;
	adCommon.ADC_Prescaler = ADC_Prescaler_Div2;
	adCommon.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	adCommon.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&adCommon);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_DEFAULT_SAMPLING_RATE); /* PC0 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_DEFAULT_SAMPLING_RATE); /* PC1 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_DEFAULT_SAMPLING_RATE); /* PC2 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_DEFAULT_SAMPLING_RATE); /* PC3 */

	/* Now Configure the IO for the ADC Pins */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef adcPinInit;
	adcPinInit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 ;//PF9
	adcPinInit.GPIO_Mode = GPIO_Mode_AN ;
	adcPinInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &adcPinInit);

	/* Turn on AD Converter */
	ADC_ITConfig(ADC1, ADC_IT_OVR, DISABLE);
	ADC_ITConfig(ADC1, ADC_IT_AWD, DISABLE);

	nvicInit.NVIC_IRQChannel = ADC_IRQn;
	/* Timing is less than critical to load the timings. */
	nvicInit.NVIC_IRQChannelPreemptionPriority = 8;
	nvicInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicInit);

	ADC_Cmd(ADC1, ENABLE);

	TWB_Debug_Print("ADC-");

	__lipoADC = ADC1;

	return OK;
}

uint8_t __adcChannelSeq = 0;

/*
 * TODO: Should be able to do a scan on the channels rather than just read them individually.
 */
iOpResult_e TWB_ADC_Read(float deltaT){
	__adcChannelSeq = 0;

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_DEFAULT_SAMPLING_RATE); /* PC1 */

	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

	ADC_SoftwareStartConv(ADC1);
	return OK;
}

iOpResult_e TWB_ADC_ResetDefaults(void){
	SensorConfig->LIPO_ADC_Enabled = Disabled;
	SensorConfig->LIPO_ADC_FilterOption = FilterOption_None;
	SensorConfig->LIPO_ADC_SampleRate = SampleRate_10Hz;

	ADCLipoSensor->SampleRate = SensorConfig->LIPO_ADC_SampleRate;
	ADCLipoSensor->IsEnabled = SensorConfig->LIPO_ADC_Enabled;

	assert_succeed(TWB_SE_WriteU8Pause(LIPO_ADC_EEPROM_OFFSET + 0, SensorConfig->LIPO_ADC_Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(LIPO_ADC_EEPROM_OFFSET + 1, SensorConfig->LIPO_ADC_FilterOption));
	assert_succeed(TWB_SE_WriteU8Pause(LIPO_ADC_EEPROM_OFFSET + 2, SensorConfig->LIPO_ADC_SampleRate));

	return OK;
}

iOpResult_e TWB_ADC_UpdateSetting(uint16_t addr, uint8_t value){
	switch(addr){
	case 0:
		SensorConfig->LIPO_ADC_Enabled = value;
		if(SensorConfig->LIPO_ADC_Enabled == Enabled){
			assert_succeed(TWB_ADC_Init());
		}
		else
			ADCLipoSensor->IsEnabled = Disabled;

		break;
	case 1:
		SensorConfig->LIPO_ADC_FilterOption = value;
		__lipoAdcFilter1.FilterType = value;
		__lipoAdcFilter2.FilterType = value;
		__lipoAdcFilter3.FilterType = value;

		__lipoAdcFilter1.IsInitialized = false;
		__lipoAdcFilter2.IsInitialized = false;
		__lipoAdcFilter3.IsInitialized = false;

		__lipoAdcFilter1.CurrentSlot = 0;
		__lipoAdcFilter2.CurrentSlot = 0;
		__lipoAdcFilter3.CurrentSlot = 0;
		break;
	case 2:
		SensorConfig->LIPO_ADC_SampleRate = value;
		ADCLipoSensor->SampleRate = SensorConfig->LIPO_ADC_SampleRate;
		break;
	}

	return OK;
}

/* Temp holding for batteries, holds raw
 * voltage on pins, used to allow setting structure
 * atomically.
 */
uint16_t __adcCell1;
uint16_t __adcCell2;
uint16_t __adcCell3;

void ADC_IRQHandler(void){
	if(ADC_GetITStatus(ADC1, ADC_IT_JEOC) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);

		ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
		ADC_ITConfig(ADC1, ADC_IT_JEOC, DISABLE);

		BatteryCondition->Cell1Battery = __adcCell1 * 2;
		BatteryCondition->Cell2Battery = (__adcCell2 * 4) - __adcCell1 * 2;
		BatteryCondition->Cell3Battery = (__adcCell3 * 6) - __adcCell2 * 4;
	}
	else if(ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_OVR);
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);

		//uint16_t adcData = ADC1->DR >> 4;
		uint16_t adcData = ADC1->DR;

		switch(__adcChannelSeq++){ /* VALUE * 3300 // 0xFF will give millivolts w/ integer math, scaling happens when assign to BatteryCondition structure */
			case 0:
				TWB_Filter_Median_Apply(&__lipoAdcFilter1, adcData);
				__adcCell1 = __lipoAdcFilter1.Current  * 3300 / 0xFFF;
				ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_DEFAULT_SAMPLING_RATE); /* PC1 */
				ADC_SoftwareStartConv(ADC1);
				break;
			case 1:
				TWB_Filter_Median_Apply(&__lipoAdcFilter2, adcData);
				__adcCell2 = __lipoAdcFilter2.Current * 3300 / 0xFFF;
				ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_DEFAULT_SAMPLING_RATE); /* PC1 */
				ADC_SoftwareStartConv(ADC1);
				break;
			case 2:
				ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);

				TWB_Filter_Median_Apply(&__lipoAdcFilter3, adcData);
				__adcCell3 = __lipoAdcFilter3.Current * 3300 / 0xFFF;

				BatteryCondition->Cell1Battery = __adcCell1 * 2;
				BatteryCondition->Cell2Battery = (__adcCell2 * 4) - __adcCell1 * 2;
				BatteryCondition->Cell3Battery = (__adcCell3 * 6) - __adcCell2 * 4;
				break;
		}
	}
	/*else if(ADC_GetITStatus(ADC1, ADC_IT_EOSMP) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOSMP);
		TWB_Debug_Print("EOSMP!\r\n");
	}
	else if(ADC_GetITStatus(ADC1, ADC_IT_ADRDY) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_ADRDY);
		TWB_Debug_Print("ADRDY!\r\n");
	}*/
	else if(ADC_GetITStatus(ADC1, ADC_IT_OVR) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_OVR);
		TWB_Debug_Print("OVR!\r\n");
	}
	else if(ADC_GetITStatus(ADC1, ADC_IT_AWD) == SET){
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
		TWB_Debug_Print("AWD!\r\n");
	}
	else {
		TWB_Debug_Print("ADC?");
	}
}

void TWB_ADC_QueueMsg(void){
	TWB_Commo_SendMessage(MOD_Sensor, MSG_BATTERY, (uint8_t *)BatteryCondition, sizeof(BatteryCondition_t));
}

void TWB_ADC_SendDiag(){
	_commonDiagBuffer->SensorId = ADCLipoSensor->SensorId;

	_commonDiagBuffer->RawX = (int16_t)__lipoAdcFilter1.Current;
	_commonDiagBuffer->RawY = (int16_t)__lipoAdcFilter2.Current;
	_commonDiagBuffer->RawZ = (int16_t)__lipoAdcFilter3.Current;

	_commonDiagBuffer->X = (int16_t)BatteryCondition->Cell1Battery;
	_commonDiagBuffer->Y = (int16_t)BatteryCondition->Cell2Battery;
	_commonDiagBuffer->Z = (int16_t)BatteryCondition->Cell3Battery;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SetDiagnostics, (uint8_t *)_commonDiagBuffer, sizeof(SensorDiagData_t));
}
