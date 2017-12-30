/*
 * complementary.c
 *
 *  Created on: Jan 4, 2013
 *      Author: kevinw
 */

#include "sensors/fusion/complementary.h"
#include "common/twb_serial_eeprom.h"
#include "twb_eeprom.h"
#include "common/twb_debug.h"
#include "common/memory_mgmt.h"
#include "sensors/GPS.h"
#include "flight/autopilot.h"

SensorFusion_t *ComplementaryFilterConfig;

typedef struct {
	int16_t Combined;
	int16_t Sensor1;
	int16_t Sensor2;
	int16_t Sensor3;

	SensorDevice_e DeviceType;

} SensorFusionMsg_t;

SensorFusionMsg_t *SensorFusionMsg;

CompFilter_t *CompFilterPitch;
CompFilter_t *CompFilterRoll;
CompFilter_t *CompFilterHeading;
CompFilter_t *CompFilterAltitude;

void __snsr_ResolveCompFilter(CompFilterValues_t value, CompFilter_t *filterSettings){
	float timeConstant = 0.0f;

	switch(value){
		case CompFilter_1sec: timeConstant = 1.000; break;
		case CompFilter_900Ms: timeConstant = 0.90; break;
		case CompFilter_800Ms: timeConstant = 0.80; break;
		case CompFilter_700Ms: timeConstant = 0.70; break;
		case CompFilter_600Ms: timeConstant = 0.60; break;
		case CompFilter_500Ms: timeConstant = 0.50; break;
		case CompFilter_400Ms: timeConstant = 0.400; break;
		case CompFilter_300Ms: timeConstant = 0.300; break;
		case CompFilter_200Ms: timeConstant = 0.200; break;
		case CompFilter_150Ms: timeConstant = 0.150; break;
		case CompFilter_100Ms: timeConstant = 0.100; break;
		case CompFilter_50Ms: timeConstant = 0.050; break;
		case CompFilter_25Ms: timeConstant = 0.025; break;
		case CompFilter_20Ms: timeConstant = 0.020; break;
		case CompFilter_15Ms: timeConstant = 0.015; break;
		case CompFilter_10Ms: timeConstant = 0.010; break;
		case CompFilter_05Ms: timeConstant = 0.005; break;
		case CompFilter_02Ms: timeConstant = 0.002; break;
		case CompFilter_01Ms: timeConstant = 0.001; break;
	}

	float ms = TWB_Timer_GetMSFromSampleRate(ComplementaryFilterConfig->SampleRate) / 1000.0f;

    float alpha = timeConstant / (timeConstant + ms);

	filterSettings->Period1 = alpha;;
	filterSettings->Period2 = 1.0f - alpha;
}

void __snsr_compFilter_ResolveCompFilters(void){
	__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantPitch, CompFilterPitch);
	__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantRoll, CompFilterRoll);
	__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantHeading, CompFilterHeading);
	__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantAltitude, CompFilterAltitude);
}

Sensor_t *__snsr_compFilter_ResolveSensor(SensorDevice_e sensorType){
	switch(sensorType){
		case GyroITG3200: return GyroITG3200Sensor;
		case GyroL3GD20: return GyroL3GD20Sensor;
		case GyroMPU60x0: return GyroMPU60x0Sensor;

		case AccLSM303: return AccLSM303Sensor;
		case AccADXL345: return AccADXL345Sensor;
		case AccMPU60x0: return AccMPU60x0Sensor;

		case MagLSM303: return MagLSM303Sensor;
		case MagHMS5883: return MagHMC5883Sensor;

		case AltBMP085: return PressureBMP085Sensor;
		case AltMS5611: return PressureMS5611Sensor;
		case AltSonar: return SonarSensor;
		case AltGPS: return GeoGPSSensor;

		default: return SensorNone;
	}
}

void __snsr_compFilter_ResolveSensors(void){
	CompFilterPitch->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Pitch);
	CompFilterPitch->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Pitch);
	CompFilterPitch->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Pitch);

	CompFilterRoll->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Roll);
	CompFilterRoll->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Roll);
	CompFilterRoll->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Roll);

	CompFilterHeading->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Heading);
	CompFilterHeading->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Heading);
	CompFilterHeading->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Heading);

	CompFilterAltitude->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Altitude);
	CompFilterAltitude->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Altitude);
	CompFilterAltitude->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Altitude);
}

iOpResult_e TWB_SNSR_Complementary_Init(void){
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 0, &ComplementaryFilterConfig->Enabled));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 1, &ComplementaryFilterConfig->SampleRate));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 2, &ComplementaryFilterConfig->FilterOptionPitch));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 3, &ComplementaryFilterConfig->FilterOptionRoll));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 4, &ComplementaryFilterConfig->FilterOptionHeading));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 5, &ComplementaryFilterConfig->FilterOptionAltitude));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 6, &ComplementaryFilterConfig->TimeConstantPitch));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 7, &ComplementaryFilterConfig->TimeConstantRoll));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 8, &ComplementaryFilterConfig->TimeConstantHeading));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 9, &ComplementaryFilterConfig->TimeConstantAltitude));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 10, &ComplementaryFilterConfig->Sensor1Pitch));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 11, &ComplementaryFilterConfig->Sensor2Pitch));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 12, &ComplementaryFilterConfig->Sensor3Pitch));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 13, &ComplementaryFilterConfig->Sensor1Roll));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 14, &ComplementaryFilterConfig->Sensor2Roll));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 15, &ComplementaryFilterConfig->Sensor3Roll));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 16, &ComplementaryFilterConfig->Sensor1Heading));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 17, &ComplementaryFilterConfig->Sensor2Heading));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 18, &ComplementaryFilterConfig->Sensor3Heading));

	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 19, &ComplementaryFilterConfig->Sensor1Altitude));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 20, &ComplementaryFilterConfig->Sensor2Altitude));
	assert_succeed(TWB_SE_ReadU8(COMPLEMENTARY_EEPROM_OFFSET + 21, &ComplementaryFilterConfig->Sensor3Altitude));

	SensorFusionMsg = pm_malloc(sizeof(SensorFusionMsg_t));

	CompFilterPitch = pm_malloc(sizeof(CompFilter_t));
	CompFilterPitch->Axis = PitchAxis;
	CompFilterPitch->Filter = pm_malloc(sizeof(FilterFloat_t));
	CompFilterPitch->Filter->FilterType = ComplementaryFilterConfig->FilterOptionPitch;
	CompFilterPitch->Filter->IsInitialized = false;
	CompFilterPitch->Filter->Current = 0.0f;
	CompFilterPitch->Filter->CurrentSlot = 0;
	CompFilterPitch->Filter->Previous = 0.0f;

	CompFilterRoll = pm_malloc(sizeof(CompFilter_t));
	CompFilterRoll->Axis = RollAxis;
	CompFilterRoll->Filter = pm_malloc(sizeof(FilterFloat_t));
	CompFilterRoll->Filter->FilterType = ComplementaryFilterConfig->FilterOptionRoll;
	CompFilterRoll->Filter->IsInitialized = false;
	CompFilterRoll->Filter->Current = 0.0f;
	CompFilterRoll->Filter->CurrentSlot = 0;
	CompFilterRoll->Filter->Previous = 0.0f;

	CompFilterHeading = pm_malloc(sizeof(CompFilter_t));
	CompFilterHeading->Axis = HeadingAxis;
	CompFilterHeading->Filter = pm_malloc(sizeof(FilterFloat_t));
	CompFilterHeading->Filter->FilterType = ComplementaryFilterConfig->FilterOptionHeading;
	CompFilterHeading->Filter->IsInitialized = false;
	CompFilterHeading->Filter->Current = 0.0f;
	CompFilterHeading->Filter->CurrentSlot = 0;
	CompFilterHeading->Filter->Previous = 0.0f;

	CompFilterAltitude = pm_malloc(sizeof(CompFilter_t));
	CompFilterAltitude->Axis = AltitudeAxis;
	CompFilterAltitude->Filter = pm_malloc(sizeof(FilterFloat_t));
	CompFilterAltitude->Filter->FilterType = ComplementaryFilterConfig->FilterOptionAltitude;
	CompFilterAltitude->Filter->IsInitialized = false;
	CompFilterAltitude->Filter->Current = 0.0f;
	CompFilterAltitude->Filter->CurrentSlot = 0;
	CompFilterAltitude->Filter->Previous = 0.0f;

	ComplementarySnsrFusion->IsEnabled = ComplementaryFilterConfig->Enabled;
	ComplementarySnsrFusion->SampleRate = ComplementaryFilterConfig->SampleRate;

	ComplementarySnsrFusion->Read = &TWB_SNSR_Complementary_Process;

	__snsr_compFilter_ResolveSensors();
	__snsr_compFilter_ResolveCompFilters();

	return OK;
}

iOpResult_e TWB_SNSR_Complementary_ResetDefaults(void) {
	ComplementaryFilterConfig->Enabled = Enabled;
	ComplementaryFilterConfig->SampleRate = SampleRate_200Hz;

	ComplementaryFilterConfig->TimeConstantPitch = CompFilter_200Ms;
	ComplementaryFilterConfig->TimeConstantRoll = CompFilter_200Ms;
	ComplementaryFilterConfig->TimeConstantHeading = CompFilter_100Ms;
	ComplementaryFilterConfig->TimeConstantAltitude = CompFilter_500Ms;

	ComplementaryFilterConfig->FilterOptionPitch = FilterOption_None;
	ComplementaryFilterConfig->FilterOptionRoll = FilterOption_None;
	ComplementaryFilterConfig->FilterOptionHeading = FilterOption_None;
	ComplementaryFilterConfig->FilterOptionAltitude = FilterOption_None;

	ComplementaryFilterConfig->Sensor1Pitch = GyroL3GD20;
	ComplementaryFilterConfig->Sensor2Pitch = AccLSM303;
	ComplementaryFilterConfig->Sensor3Pitch = SensorNone;

	ComplementaryFilterConfig->Sensor1Roll = GyroL3GD20;
	ComplementaryFilterConfig->Sensor2Roll = AccLSM303;
	ComplementaryFilterConfig->Sensor3Roll = SensorNone;

	ComplementaryFilterConfig->Sensor1Heading = GyroL3GD20;
	ComplementaryFilterConfig->Sensor2Heading = MagLSM303;
	ComplementaryFilterConfig->Sensor3Heading = SensorNone;

	ComplementaryFilterConfig->Sensor1Altitude = AltSonar;
	ComplementaryFilterConfig->Sensor2Altitude = AltBMP085;
	ComplementaryFilterConfig->Sensor3Altitude = SensorNone;

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 0, ComplementaryFilterConfig->Enabled));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 1, ComplementaryFilterConfig->SampleRate));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 2, ComplementaryFilterConfig->FilterOptionPitch));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 3, ComplementaryFilterConfig->FilterOptionRoll));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 4, ComplementaryFilterConfig->FilterOptionHeading));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 5, ComplementaryFilterConfig->FilterOptionAltitude));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 6, ComplementaryFilterConfig->TimeConstantPitch));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 7, ComplementaryFilterConfig->TimeConstantRoll));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 8, ComplementaryFilterConfig->TimeConstantHeading));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 9, ComplementaryFilterConfig->TimeConstantAltitude));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 10, ComplementaryFilterConfig->Sensor1Pitch));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 11, ComplementaryFilterConfig->Sensor2Pitch));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 12, ComplementaryFilterConfig->Sensor3Pitch));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 13, ComplementaryFilterConfig->Sensor1Roll));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 14, ComplementaryFilterConfig->Sensor2Roll));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 15, ComplementaryFilterConfig->Sensor3Roll));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 16, ComplementaryFilterConfig->Sensor1Heading));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 17, ComplementaryFilterConfig->Sensor2Heading));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 18, ComplementaryFilterConfig->Sensor3Heading));

	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 19, ComplementaryFilterConfig->Sensor1Altitude));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 20, ComplementaryFilterConfig->Sensor2Altitude));
	assert_succeed(TWB_SE_WriteU8Pause(COMPLEMENTARY_EEPROM_OFFSET + 21, ComplementaryFilterConfig->Sensor3Altitude));

	ComplementarySnsrFusion->IsEnabled = ComplementaryFilterConfig->Enabled;
	ComplementarySnsrFusion->SampleRate = ComplementaryFilterConfig->SampleRate;

	return OK;
}

iOpResult_e TWB_SNSR_Complementary_UpdateSettings(uint16_t addr, uint8_t value){
	switch(addr){
	case 0:
		ComplementarySnsrFusion->IsEnabled = value;
		ComplementaryFilterConfig->Enabled = value;

		break;
	case 1:
		ComplementarySnsrFusion->SampleRate = value;
		ComplementaryFilterConfig->SampleRate = value;
		break;

	case 2:
		ComplementaryFilterConfig->FilterOptionPitch = value;
		CompFilterPitch->Filter->FilterType = value;
		CompFilterPitch->Filter->IsInitialized = false;
		CompFilterPitch->Filter->CurrentSlot = 0;
		CompFilterPitch->Filter->Previous = 0.0f;
		break;

	case 3:
		ComplementaryFilterConfig->FilterOptionRoll = value;
		CompFilterRoll->Filter->FilterType = value;
		CompFilterRoll->Filter->IsInitialized = false;
		CompFilterRoll->Filter->CurrentSlot = 0;
		CompFilterRoll->Filter->Previous = 0.0f;

		break;
	case 4:
		ComplementaryFilterConfig->FilterOptionHeading = value;
		CompFilterHeading->Filter->FilterType = value;
		CompFilterHeading->Filter->IsInitialized = false;
		CompFilterHeading->Filter->CurrentSlot = 0;
		CompFilterHeading->Filter->Previous = 0.0f;
		break;
	case 5:
		ComplementaryFilterConfig->FilterOptionAltitude = value;
		CompFilterAltitude->Filter->FilterType = value;
		CompFilterAltitude->Filter->IsInitialized = false;
		CompFilterAltitude->Filter->CurrentSlot = 0;
		CompFilterAltitude->Filter->Previous = 0.0f;
		break;

    /* Update the time constants for the comp filters */
	case 6:
		ComplementaryFilterConfig->TimeConstantPitch = value;
		__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantPitch, CompFilterPitch);
		break;
	case 7:
		ComplementaryFilterConfig->TimeConstantRoll = value;
		__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantRoll, CompFilterRoll);
		break;
	case 8:
		ComplementaryFilterConfig->TimeConstantHeading = value;
		__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantHeading, CompFilterHeading);
		break;
	case 9:
		ComplementaryFilterConfig->TimeConstantAltitude = value;
		__snsr_ResolveCompFilter(ComplementaryFilterConfig->TimeConstantAltitude, CompFilterAltitude);
		break;


	/* Update the filters for each axis */

	case 10:
		ComplementaryFilterConfig->Sensor1Pitch = value;
		CompFilterPitch->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Pitch);
		break;
	case 11:
		ComplementaryFilterConfig->Sensor2Pitch = value;
		CompFilterPitch->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Pitch);
		break;
	case 12:
		ComplementaryFilterConfig->Sensor3Pitch = value;
		CompFilterPitch->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Pitch);
		break;
	case 13:
		ComplementaryFilterConfig->Sensor1Roll = value;
		CompFilterRoll->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Roll);
	break;
	case 14:
		ComplementaryFilterConfig->Sensor2Roll = value;
		CompFilterRoll->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Roll);
		break;
	case 15:
		ComplementaryFilterConfig->Sensor3Roll = value;
		CompFilterRoll->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Roll);
		break;
	case 16:
		ComplementaryFilterConfig->Sensor1Heading = value;
		CompFilterHeading->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Heading);
		break;
	case 17:
		ComplementaryFilterConfig->Sensor2Heading = value;
		CompFilterHeading->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Heading);
		break;
	case 18:
		ComplementaryFilterConfig->Sensor3Heading = value;
		CompFilterHeading->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Heading);
		break;
	case 19:
		ComplementaryFilterConfig->Sensor1Altitude = value;
		CompFilterAltitude->Sensor1 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor1Altitude);
		break;
	case 20:
		ComplementaryFilterConfig->Sensor2Altitude = value;
		CompFilterAltitude->Sensor2 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor2Altitude);
		break;
	case 21:
		ComplementaryFilterConfig->Sensor3Altitude = value;
		CompFilterAltitude->Sensor3 = __snsr_compFilter_ResolveSensor(ComplementaryFilterConfig->Sensor3Altitude);
		break;
	}
	return OK;
}

iOpResult_e TWB_SNSR_Complementary_Process(float deltaT){
	//TODO: After complementary filter, we need to pump the angle back into the gyro, or pickup the deltaX/deltaY and use those as part of this.
	if(ComplementaryFilterConfig->Sensor1Pitch != SensorNone){
		if(ComplementaryFilterConfig->Sensor2Pitch != SensorNone)
			InternalState->Pitch = (CompFilterPitch->Sensor1->X * CompFilterPitch->Period1) + (CompFilterPitch->Sensor2->X * CompFilterPitch->Period2);
		else
			InternalState->Pitch = CompFilterPitch->Sensor1->X;

		TWB_Filter_Median_Apply_Float(CompFilterPitch->Filter, InternalState->Pitch);

		InternalState->Pitch = CompFilterPitch->Filter->Current;
		InternalState->PitchRateDPS = ((GryoExtendedData_t *)CompFilterRoll->Sensor1->Extended)->RateXDPS;
	}

	if(ComplementaryFilterConfig->Sensor1Roll != SensorNone){
		if(ComplementaryFilterConfig->Sensor2Roll != SensorNone)
			InternalState->Roll = (CompFilterRoll->Sensor1->Y * CompFilterRoll->Period1) + (CompFilterRoll->Sensor2->Y * CompFilterRoll->Period2);
		else
			InternalState->Roll = CompFilterRoll->Sensor1->Y;

		TWB_Filter_Median_Apply_Float(CompFilterRoll->Filter, InternalState->Roll);

		InternalState->Roll = CompFilterRoll->Filter->Current;
		InternalState->RollRateDPS = ((GryoExtendedData_t *)CompFilterRoll->Sensor1->Extended)->RateYDPS;
	}

	if(ComplementaryFilterConfig->Sensor1Heading != SensorNone){
		if(ComplementaryFilterConfig->Sensor2Heading != SensorNone){
			float sensor1 = CompFilterHeading->Sensor1->Z;
			float sensor2 = CompFilterHeading->Sensor2->Z;

			/* Need to be careful around 0 degrees */
			if(sensor1 > 350.0f && sensor2 < 10.0f){
				sensor1 = sensor2;
				InternalState->Heading = sensor1;
			}
			else if(sensor1 < 10.0f && sensor2 > 350.f){
				sensor1 = sensor2;
				InternalState->Heading = sensor1;
			}
			else
				InternalState->Heading = (sensor1 * CompFilterHeading->Period1) + (sensor2 * CompFilterHeading->Period2);
		}
		else
			InternalState->Heading = CompFilterHeading->Sensor1->Z;

		TWB_Filter_Median_Apply_Float(CompFilterHeading->Filter, InternalState->Heading);

		InternalState->Heading = CompFilterHeading->Filter->Current;
		InternalState->YawRateDPS = ((GryoExtendedData_t *)CompFilterRoll->Sensor1->Extended)->RateZDPS;
	}

	TWB_Autopilot_DetermineAltitude(CompFilterAltitude);

	/*Now update the gyro with the output of the complemenatary filter */
	if(ComplementaryFilterConfig->Sensor1Pitch >= GyroITG3200 && ComplementaryFilterConfig->Sensor1Pitch <= GyroL3GD20)
		CompFilterPitch->Sensor1->X = InternalState->Pitch;

	if(ComplementaryFilterConfig->Sensor2Pitch >= GyroITG3200 && ComplementaryFilterConfig->Sensor2Pitch <= GyroL3GD20)
		CompFilterPitch->Sensor2->X = InternalState->Pitch;

	if(ComplementaryFilterConfig->Sensor1Roll >= GyroITG3200 && ComplementaryFilterConfig->Sensor1Roll <= GyroL3GD20)
		CompFilterPitch->Sensor1->Y = InternalState->Roll;

	if(ComplementaryFilterConfig->Sensor2Roll >= GyroITG3200 && ComplementaryFilterConfig->Sensor2Roll <= GyroL3GD20)
		CompFilterPitch->Sensor2->Y = InternalState->Roll;

	if(ComplementaryFilterConfig->Sensor1Heading >= GyroITG3200 && ComplementaryFilterConfig->Sensor1Heading <= GyroL3GD20)
		CompFilterPitch->Sensor1->Z = InternalState->Heading;

	if(ComplementaryFilterConfig->Sensor2Heading >= GyroITG3200 && ComplementaryFilterConfig->Sensor2Heading <= GyroL3GD20)
		CompFilterPitch->Sensor2->Z = InternalState->Heading;

	SensorData->Pitch = (int16_t)(InternalState->Pitch * 10.0f);
	SensorData->Roll = (int16_t)(InternalState->Roll * 10.0f);
	SensorData->Heading = (uint16_t)(InternalState->Heading * 10.0f);
	SensorData->AltitudeM = (int16_t)(InternalState->AltitudeM * 10.0f);

	return OK;
}


void TWB_SNSR_Complementary_SendDiag(void){
	switch(SystemStatus->SnsrState){
		case SensorAppState_SnsrFusionPitch:
			SensorFusionMsg->Combined = (int16_t)(InternalState->Pitch * 10.0f);

			SensorFusionMsg->Sensor1 = ComplementaryFilterConfig->Sensor1Pitch == SensorNone ? 0 :(int16_t)(CompFilterPitch->Sensor1->X * 10.0f);
			SensorFusionMsg->Sensor2 = ComplementaryFilterConfig->Sensor2Pitch == SensorNone ? 0 :(int16_t)(CompFilterPitch->Sensor2->X * 10.0f);
			SensorFusionMsg->Sensor3 = ComplementaryFilterConfig->Sensor3Pitch == SensorNone ? 0 :(int16_t)(CompFilterPitch->Sensor3->X * 10.0f);

			break;
		case SensorAppState_SnsrFusionRoll:
			SensorFusionMsg->Combined = (int16_t)(InternalState->Roll * 10.0f);

			SensorFusionMsg->Sensor1 = ComplementaryFilterConfig->Sensor1Roll == SensorNone ? 0 : (int16_t)(CompFilterRoll->Sensor1->Y * 10.0f);
			SensorFusionMsg->Sensor2 = ComplementaryFilterConfig->Sensor2Roll == SensorNone ? 0 : (int16_t)(CompFilterRoll->Sensor2->Y * 10.0f);
			SensorFusionMsg->Sensor3 = ComplementaryFilterConfig->Sensor3Roll == SensorNone ? 0 : (int16_t)(CompFilterRoll->Sensor3->Y * 10.0f);

			break;
		case SensorAppState_SnsrFusionHeading:
			SensorFusionMsg->Combined = (int16_t)(InternalState->Heading * 10.0f);

			SensorFusionMsg->Sensor1 = ComplementaryFilterConfig->Sensor1Heading == SensorNone ? 0 : (int16_t)(CompFilterHeading->Sensor1->Z * 10.0f);
			SensorFusionMsg->Sensor2 = ComplementaryFilterConfig->Sensor2Heading == SensorNone ? 0 : (int16_t)(CompFilterHeading->Sensor2->Z * 10.0f);
			SensorFusionMsg->Sensor3 = ComplementaryFilterConfig->Sensor3Heading == SensorNone ? 0 : (int16_t)(CompFilterHeading->Sensor3->Z * 10.0f);

			break;
		case SensorAppState_SnsrFusionAltitude:
			SensorFusionMsg->Combined = (int16_t)(InternalState->AltitudeM * 10.0f);

			SensorFusionMsg->Sensor1 = ComplementaryFilterConfig->Sensor1Altitude == SensorNone ? 0 : (int16_t)(CompFilterAltitude->Sensor1->Z * 10.0f);
			SensorFusionMsg->Sensor2 = ComplementaryFilterConfig->Sensor2Altitude == SensorNone ? 0 : (int16_t)(CompFilterAltitude->Sensor2->Z * 10.0f);
			SensorFusionMsg->Sensor3 = ComplementaryFilterConfig->Sensor3Altitude == SensorNone ? 0 : (int16_t)(CompFilterAltitude->Sensor3->Z * 10.0f);
			break;
		default: break;
	}


	SensorFusionMsg->DeviceType = SnsrFusionComplementary;

	TWB_Commo_SendMessage(MOD_Sensor, MSG_SnsrFusion, (uint8_t *)SensorFusionMsg, sizeof(SensorFusionMsg_t));
}
