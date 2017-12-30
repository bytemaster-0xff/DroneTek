#ifndef NIVEKMESSAGESCRIBE_H
#define NIVEKMESSAGESCRIBE_H

#include <stdint.h>
#include "NiVekMessage.h"

#define MESSAGE_MAX_COUNT 10

class NiVekMessageScribe
{
private:
	NiVekMessage m_messageSlots[MESSAGE_MAX_COUNT]; /*Not liking name, but want to pre-allocate memory */
	uint16_t m_serialNumber;
	uint8_t m_messageSlotHead;
	uint8_t m_messageSlotTail;

public:
	NiVekMessageScribe();
	NiVekMessage *beginMessage(uint8_t expectAck, uint8_t sourceAddress, uint8_t destAddress, uint8_t moduleId, uint8_t messageId);
	void endMessage();
	NiVekMessage *getNextToSend();
};

#endif

