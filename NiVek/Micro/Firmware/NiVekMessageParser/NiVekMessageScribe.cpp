

#include "NiVekMessage.h"
#include "NiVekMessageScribe.h"

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define ACK 0x06
#define NAK 0x15


NiVekMessageScribe::NiVekMessageScribe(){
	m_messageSlotHead = 0;
	m_messageSlotTail = 0;
	m_serialNumber = 0;
}

NiVekMessage *NiVekMessageScribe::beginMessage(uint8_t expectAck, uint8_t sourceAddress, uint8_t destAddress, uint8_t systemId, uint8_t messageId){
	NiVekMessage *message = &m_messageSlots[m_messageSlotHead];
	message->clear();

	message->ExpectACK = expectAck;
	message->SourceAddress = sourceAddress;
	message->DestAddress = destAddress;
	message->SystemId = systemId;
	message->TypeId = messageId;
	message->MsgSize = 0;
	message->SerialNumber = m_serialNumber++;

	message->add_xor(expectAck);
	message->add_xor(sourceAddress);
	message->add_xor(destAddress);
	message->add_xor(systemId);
	message->add_xor(messageId);

	message->add_xor(message->SerialNumber >> 8);
	message->add_xor(message->SerialNumber & 0xFF);

	m_messageSlotHead++;

	if (m_messageSlotHead == MESSAGE_MAX_COUNT)
		m_messageSlotHead = 0;

	return message;
}