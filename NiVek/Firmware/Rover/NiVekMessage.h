#ifndef NIVEKMESSAGE_H
#define NIVEKMESSAGE_H

#include <stdint.h>
//#include "Arduino.h"

class NiVekMessage
{
public:
	NiVekMessage();

	void clear();
	void add_xor(uint8_t ch);
	void serialize16(int16_t a);
	void serialize(uint8_t *ptr, uint8_t len);
	void serialize8(uint8_t a);

	uint8_t SourceAddress;
	uint8_t DestAddress;
	uint8_t ExpectACK;
	uint8_t SystemId;
	uint8_t TypeId;
	uint16_t MsgSize;
	uint16_t SerialNumber;
	uint8_t CheckSum;
	uint8_t MsgBuffer[64];
};


typedef enum TWB_Commo_ParserStates{
	TWB_SOH,
	TWB_ExpectACK,
	TWB_ExpectSourceAddress,
	TWB_ExpectDestAddress,
	TWB_SystemId,
	TWB_MessageType,
	TWB_SerialNumberMSB,
	TWB_SerialNumberLSB,
	TWB_SizeMSB,
	TWB_SizeLSB,
	TWB_STX,
	TWB_Payload,
	TWB_ETX,
	TWB_CheckSum,
	TWB_EOT
} iParseState_t;

#endif

