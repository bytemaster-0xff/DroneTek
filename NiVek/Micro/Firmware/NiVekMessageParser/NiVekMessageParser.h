#ifndef NIVEKMESSAGEPARSER_H
#define NIVEKMESSAGEPARSER_H

#include <stdint.h>
#include "NiVekMessage.h"
#include "NiVekMessageScribe.h"
#include "Print.h"

#define MSG_QUEUE_SIZE_IN 4

class NiVekMessageParser
{
public:
	NiVekMessageParser(Print *printer, NiVekMessageScribe *scribe);

	void handleCh(uint8_t ch);

	NiVekMessage *currentMessage();

	bool available();

private:
	Print *_printer;
	NiVekMessageScribe *_messageScribe;

	void sendNak(uint8_t localAddress, uint8_t destAddress, uint16_t serialNumber, const char *);

	uint8_t _commoParserState;
	
	uint8_t _rxChIdx;
	uint8_t _rxMsgHead;
	uint8_t _rxMsgTail;
	NiVekMessage _incomingMessages[MSG_QUEUE_SIZE_IN];
};


#endif

