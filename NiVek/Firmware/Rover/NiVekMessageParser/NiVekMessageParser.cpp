#include "NiVekMessageParser.h"
//#include "NiVekRadio.h"

#define MODULE_SYSTEM	100
#define ACK 0x06
#define NAK 0x15


NiVekMessageParser::NiVekMessageParser(NiVekLogger *print, NiVekMessageScribe *scribe){
	_printer = print;
	_messageScribe = scribe;

	_commoParserState = TWB_SOH;
}

bool NiVekMessageParser::available(){
	return (_rxMsgHead != _rxMsgTail);
}

NiVekMessage* NiVekMessageParser::currentMessage(){
	NiVekMessage *msg = &_incomingMessages[_rxMsgHead];
	if (++_rxMsgHead == MSG_QUEUE_SIZE_IN)
		_rxMsgHead = 0;

	return msg;
}

void NiVekMessageParser::sendNak(uint8_t localAddress, uint8_t destAddress, uint16_t serialNumber, const char *errMsg){
	NiVekMessage *msg = _messageScribe->beginMessage(0x00, localAddress, destAddress, MODULE_SYSTEM, NAK);
	msg->serialize8((serialNumber >> 8) & 0xFF);
	msg->serialize8(serialNumber & 0xFF);
	_messageScribe->endMessage();
}

void NiVekMessageParser::handleCh(uint8_t ch) {
		switch (_commoParserState){
		case TWB_SOH:
			if (ch == 0x01){ //SOH
				_commoParserState = TWB_ExpectACK;
				_rxChIdx = 0;
			}

			break;
		case TWB_ExpectACK:
			if (ch != 0x00 && ch != 0xFF)
				_commoParserState = TWB_SOH;
			else{
				_incomingMessages[_rxMsgTail].ExpectACK = ch;
				_incomingMessages[_rxMsgTail].CheckSum ^= ch;
				_commoParserState = TWB_ExpectSourceAddress;
			}
			break;
		case TWB_ExpectSourceAddress:
			_incomingMessages[_rxMsgTail].SourceAddress = ch;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_ExpectDestAddress;
			break;
		case TWB_ExpectDestAddress:
			_incomingMessages[_rxMsgTail].DestAddress = ch; 
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_SystemId;
			break;
		case TWB_SystemId:
			_incomingMessages[_rxMsgTail].SystemId = ch;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_MessageType;
			break;
		case TWB_MessageType:
			_incomingMessages[_rxMsgTail].TypeId = ch;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_SerialNumberMSB;
			break;
		case TWB_SerialNumberMSB:
			_incomingMessages[_rxMsgTail].SerialNumber = 0;
			_incomingMessages[_rxMsgTail].SerialNumber = ch << 8;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_SerialNumberLSB;
			
			break;
		case TWB_SerialNumberLSB:
			_incomingMessages[_rxMsgTail].SerialNumber |= ch;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			_commoParserState = TWB_SizeMSB;
			break;
		case TWB_SizeMSB:
			_incomingMessages[_rxMsgTail].MsgSize = 0;
			_incomingMessages[_rxMsgTail].MsgSize = ch << 8;
			
			_commoParserState = TWB_SizeLSB;
			break;
		case TWB_SizeLSB:
			_incomingMessages[_rxMsgTail].MsgSize |= ch;

			if (_incomingMessages[_rxMsgTail].MsgSize > 128){
				_commoParserState = TWB_SOH;
				//sendNak(_incomingMessages[_rxMsgTail].SourceAddress, _incomingMessages[_rxMsgTail].DestAddress, _incomingMessages[_rxMsgTail].SerialNumber, "TOOBIG");
			}
			else
				_commoParserState = TWB_STX;
			break;
		case TWB_STX:
			if (ch == 0x02){ //STX				
				if (_incomingMessages[_rxMsgTail].MsgSize == 0)
					_commoParserState = TWB_ETX;
				else
					_commoParserState = TWB_Payload;

			}
			else{ //If we don't see STX, something got wacked :(
				_commoParserState = TWB_SOH;
				//sendNak(_incomingMessages[_rxMsgTail].SourceAddress, _incomingMessages[_rxMsgTail].DestAddress, _incomingMessages[_rxMsgTail].SerialNumber, "!STX");
			}
			break;
		case TWB_Payload:
			_incomingMessages[_rxMsgTail].MsgBuffer[_rxChIdx++] = ch;
			_incomingMessages[_rxMsgTail].CheckSum ^= ch;
			if (_rxChIdx == _incomingMessages[_rxMsgTail].MsgSize)
				_commoParserState = TWB_ETX;


			break;
		case TWB_ETX:
			if (ch == 0x03) //ETX
				_commoParserState = TWB_CheckSum;
			else{ //If we don't see STX, something got wacked :(
				_commoParserState = TWB_SOH;
				//sendNak(_incomingMessages[_rxMsgTail].SourceAddress, _incomingMessages[_rxMsgTail].DestAddress, _incomingMessages[_rxMsgTail].SerialNumber, "!ETX");
			}
			break;
		case TWB_CheckSum:
			_commoParserState = TWB_EOT;

			if (_incomingMessages[_rxMsgTail].CheckSum == ch || true)
				_commoParserState = TWB_EOT;
			else{
				_commoParserState = TWB_SOH;
				sendNak(_incomingMessages[_rxMsgTail].SourceAddress, _incomingMessages[_rxMsgTail].DestAddress, _incomingMessages[_rxMsgTail].SerialNumber, "!CS");
			}
			break;
		case TWB_EOT:
			if (ch == 0x04){ //EOT
				_rxMsgTail++; //
				if (_rxMsgTail == MSG_QUEUE_SIZE_IN)
					_rxMsgTail = 0;
			}
			else
				sendNak(_incomingMessages[_rxMsgTail].SourceAddress, _incomingMessages[_rxMsgTail].DestAddress, _incomingMessages[_rxMsgTail].SerialNumber, "!EOT");

			_commoParserState = TWB_SOH;

			break;
		}
}