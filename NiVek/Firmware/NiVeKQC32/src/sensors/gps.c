/*
 * gps.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include <stdlib.h>
#include <string.h>
#include "twb_eeprom.h"
#include "twb_config.h"

#include "common/twb_serial_eeprom.h"

#include "sensors/gps.h"
#include "sensors/snsr_main.h"

#include "commo/commo.h"
#include "commo/message_ids.h"

#include "common/twb_common.h"
#include "common/twb_debug.h"

//Maximum amount of data being stored
//for a single value coming from the NEMA messages
#define RX_BUFFER_SIZE 16

uint8_t _uartRXBuffer[RX_BUFFER_SIZE];

uint8_t _state;
uint8_t _valueIndex;

const char *msgTypes[] = {"GPGLL", "GPGSA", "GPGSV", "GPRMC", "GPVTG","GPGGA"};

typedef enum {
	GPS_MSG_Type_GLL,
	GPS_MSG_Type_GSA,
	GPS_MSG_Type_GSV,
	GPS_MSG_Type_RMC,
	GPS_MSG_Type_VTG,
	GPS_MSG_Type_GGA
} GPS_MSG_Types;

typedef enum {
		GPS_IDLE,
		GPS_MSG_TYPE,
		GPS_READING_VALUE

} GPS_STATES_TypeDef;

//Which typeof GPS message is coming in.
uint8_t _msgType;

//Position into the GPS buffer used to hold values.
uint8_t _uartRXBufferIndex;




uint8_t checkSum;

void __gps_sendCommand(char *command){
	checkSum = 0x00;
	char *ch = command;

	while(*ch){
		if(checkSum == 0)
			checkSum = *ch++;
		else
			checkSum = checkSum ^ (uint8_t)*ch++;
	}

	/* Debug port is on the same one as GPS port...for now, so send it to debug and GPS will pick i tup */
	TWB_Debug_Print("$");
	TWB_Debug_Print(command);
	TWB_Debug_Print("*");
	TWB_Debug_PrintHexString(checkSum);

	TWB_Debug_Print("\r\n");
}

iOpResult_e TWB_GPS_Geo_Init(void){
	_uartRXBufferIndex = 0x00;

	strcpy(GPSData->Longitude,"?");
	strcpy(GPSData->Latitude,"?");
	strcpy(GPSData->SatellitesUsed,"-");

	GPSData->EastWest = ' ';
	GPSData->NorthSouth = ' ';
	GPSData->ValidFix = 'N';

	_valueIndex = 0;

	assert_succeed(TWB_SE_ReadU8(MS5611_EEPROM_OFFSET + 1, &SensorConfig->GPS_DGPSMode));

	/* Previously we have set the baud to 115200, now set the output rate from the GPS to 115200 baud */
	TWB_Debug_Print("\r\nMSG FOR GPS: \r\n");

	__gps_sendCommand("PMTK300,100,0,0,0,0"); /* Should be a 2C Check Sum */
	TWB_Timer_SleepMS(30);
	__gps_sendCommand("PMTK314,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
	TWB_Timer_SleepMS(30);
	if(SensorConfig->GPS_DGPSMode > DGPS_WAAS_2)
		SensorConfig->GPS_DGPSMode = DGPS_None_0;

	switch(SensorConfig->GPS_DGPSMode){
		case DGPS_None_0 : __gps_sendCommand("PMTK301,0"); break;
		case DGPS_RTCM_1 : __gps_sendCommand("PMTK301,1"); break;
		case DGPS_WAAS_2 : __gps_sendCommand("PMTK301,2"); break;
	}

	TWB_Timer_SleepMS(30);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	TWB_Debug_Print("GPS:         OK\r\n");

	return OK;
}

iOpResult_e TWB_GPS_Heading_Init(void){

	return OK;
}

iOpResult_e TWB_GPS_Alt_Init(void) {

	return OK;
}

void clearBuffer(void){
	_uartRXBufferIndex = 0;
	memset(_uartRXBuffer, 0x00, RX_BUFFER_SIZE);
}

void determineMessageType(void){
	uint8_t idx;

	_msgType = -1;
	for(idx = 0; idx < 6; ++idx){
		if(strncmp(msgTypes[idx],(const char *)_uartRXBuffer,5) == 0)
			_msgType = idx;
	}

	_uartRXBuffer[5] = 0;
}


iOpResult_e TWB_GPS_Geo_Update_Setting(uint16_t addr, uint8_t value){
	switch(addr){
		case 0:
			SensorConfig->GPS_DGPSMode = value;

			if(SensorConfig->GPS_DGPSMode > DGPS_WAAS_2)
				SensorConfig->GPS_DGPSMode = DGPS_None_0;

			switch(SensorConfig->GPS_DGPSMode){
				case DGPS_None_0 : __gps_sendCommand("PMTK301,0"); break;
				case DGPS_RTCM_1 : __gps_sendCommand("PMTK301,1"); break;
				case DGPS_WAAS_2 : __gps_sendCommand("PMTK301,2"); break;
			}

			break;
	}

	return OK;
}

uint8_t isnumb(char ch){
	return (ch >= '0' && ch <= '9');
}

/* Incredibly basic way of doing this but will only happen a couple times a second */
float twb_atof(const char * p){
	float value = 0.0f;
    bool negate = false;
    if(p[0] == '-'){
    	negate = true;
    	++p;
    }

    while (isnumb(*p)){
    	value *= 10.0f;
    	uint8_t wholePart = *p - '0';
    	value += wholePart;

        ++p;
    }

    if (*p == '.') {
    	++p;
    	uint8_t decidx = 1;
        while (isnumb(*p)){
        	uint8_t decPart = *p - '0';
        	value += (float)decPart / (10.0f * decidx++);
            ++p;
        }
    }

    return value * (negate ? -1.0f : 1.0f);
}

/* Message structures can be found at:
	http://www.gpsinformation.org/dale/nmea.htm
*/

void parseValue(void){
	if(_msgType == GPS_MSG_Type_GGA){
		switch(_valueIndex){
				case 1:
					if(_uartRXBuffer[0] != 0x00)
						strncpy(GPSData->Latitude, (const char *)_uartRXBuffer,10);
					else
						strcpy(GPSData->Latitude, "?");
					break;
				case 2:
					if(_uartRXBuffer[0] != 0x00)
						GPSData->NorthSouth = _uartRXBuffer[0];
					else
						GPSData->NorthSouth = ' ';
					break;
				break;
				case 3:
					if(_uartRXBuffer[0] != 0x00)
						strncpy(GPSData->Longitude, (const char *)_uartRXBuffer,10);
					else
						strcpy(GPSData->Longitude, "?");
					break;
				case 4:
					if(_uartRXBuffer[0] != 0x00)
						GPSData->EastWest = _uartRXBuffer[0];
					else
						GPSData->EastWest = ' ';

					break;
				case 5: {
						GPSData->ValidFix = _uartRXBuffer[0];
						GeoGPSSensor->X  = GPSData->ValidFix != '0' ? 1.0f : 0.0f;
					}
						break;
				case 6:
					if(_uartRXBuffer[0] != 0x00)
						strncpy(GPSData->SatellitesUsed,(const char *)_uartRXBuffer,2);
					else
						strcpy(GPSData->SatellitesUsed,"-");

					break;
				case 7: /* Horizontal Diluation of position */ break;
				case 8:{
					CurrentGPSReading->AltitudeM = twb_atof((const char *)_uartRXBuffer);
					GeoGPSSensor->Z = CurrentGPSReading->AltitudeM;
					AltitudeData->GPS = (int16_t)(CurrentGPSReading->AltitudeM * 100.0f);
					GPSData->Altitude = AltitudeData->GPS;
				}
				break;
		}

		_valueIndex += 1;
	}
	else if(_msgType == GPS_MSG_Type_VTG){
		switch(_valueIndex){
			case 1: /* True track made good (degrees) */
				break;
			case 2: /*  T */ break;
			case 3:
				    /* Magnetic Track Made Good (degrees) */
				break;
			case 4: /*  M */ break;
			case 5: /* Ground Speed (knots) */

				break;
			case 6: /* N */
				break;
			case 7: /* Ground Speed (KMPH) */
				break;
			case 8: /* K */
				break;

		}
	}
}

iOpResult_e TWB_GPS_SetHomeLocation(void){

	return OK;
}


iOpResult_e TWB_GPS_Alt_ResetDefaults(void){

	return OK;
}

iOpResult_e TWB_GPS_Heading_ResetDefaults(void){

	return OK;
}

iOpResult_e TWB_GPS_Geo_ResetDefaults(void){

	return OK;
}


iOpResult_e TWB_GPS_Alt_Update_Setting(uint16_t addr, uint8_t value){

	return OK;
}

iOpResult_e TWB_GPS_Heading_Update_Setting(uint16_t addr, uint8_t value){

	return OK;
}

void TWB_GPS_QueueMsg(void){
	TWB_Commo_SendMessage(MOD_Sensor, MSG_GpsData, (uint8_t *)GPSData, sizeof(GPSData_t));
}

/* Overrun on GPS handler not critical
 * but is strange that it could occur?!!?!?
 */
void TWB_GPS_HandleRX_Overrun(void){
	//Just clear the IRQ bit and reset the parse state back
	//to idle so we can pick up the next message.
	USART_ClearFlag(USART1, USART_FLAG_ORE);

	_state = GPS_IDLE;
}

void TWB_GPS_HandleRX_IRQ(uint8_t ch){
	switch(ch){
		case '$':
			clearBuffer();
			_state = GPS_MSG_TYPE;
		break;
		case ',':
			switch(_state){
				case GPS_MSG_TYPE:
					determineMessageType();
					_state = GPS_READING_VALUE;
					clearBuffer();
					_valueIndex = 0;
				break;
				case GPS_READING_VALUE:
					parseValue();

					clearBuffer();
				break;
			}
		break;
		case '\r': break;
		case '\n':
			_state = GPS_IDLE;
		break;
		default:
			 switch(_state){
				 case GPS_READING_VALUE:
				 case GPS_MSG_TYPE:
					_uartRXBuffer[_uartRXBufferIndex++] = ch;
					break;
			 }
		 break;
	}
}
