#include <string.h>
#include <stdio.h>

#include "commo/msg_parser.h"
#include "commo/commo.h"
#include "commo/wifi.h"
#include "system/cli.h"

#include "common/twb_common.h"
#include "common/twb_debug.h"
#include "common/twb_timer.h"

typedef enum TWB_Commo_ParserStates{
	TWB_SOH,
	TBW_ExpectACK,
	TWB_SourceAddress,
	TWB_DestAddress,
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

iParseState_t _commoTXState = TWB_SOH;
iParseState_t _commoParserState = TWB_SOH;

#define NAK 0x15

TWB_Commo_Message_t *TWB_OutgoingMsg;

iOpResult_e TWB_Commo_SendNak(uint16_t msgSerialNumber) {
	uint8_t buffer[2];
	buffer[0] = msgSerialNumber >> 8;
	buffer[1] = msgSerialNumber & 0xFF;

	TWB_Commo_SendMessage(MOD_System, NAK, buffer, 2);

	return OK;
}

void TWB_MsgParser_HandleRX_Overrun(void){
	TWB_Debug_Print("^");

	//Just clear the IRQ bit and reset the parse state back
	//to idle so we can pick up the next message.
	USART_ClearFlag(USART2, USART_FLAG_ORE);
	_commoParserState = TWB_SOH;
}

uint8_t __srcAddress; /* Source of Original Message */
uint8_t __dstAddress; /* Dest of Original Message (this device) */

void TWB_MsgParser_HandleTXE(void){
	uint8_t ch = 0x00;

	switch(_commoTXState){
		case TWB_SOH:
			ch = 0x01;
			_commoTXState = TBW_ExpectACK;
			break;
		case TBW_ExpectACK:
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum = 0x00;
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].ExpectACK;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_SourceAddress;
			break;
		case TWB_SourceAddress:
			ch = __dstAddress;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_DestAddress;
			break;
		case TWB_DestAddress:
			ch = __srcAddress;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_SystemId;
			break;
		case TWB_SystemId:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].SystemId;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_MessageType;

			break;
		case TWB_MessageType:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].TypeId;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_SerialNumberMSB;
			break;
		case TWB_SerialNumberMSB:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].SerialNumber >> 8;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_SerialNumberLSB;

			break;
		case TWB_SerialNumberLSB:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].SerialNumber & 0xFF;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;
			_commoTXState = TWB_SizeMSB;
			break;
		case TWB_SizeMSB:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].MsgSize >> 8;
			_commoTXState = TWB_SizeLSB;
			break;
		case TWB_SizeLSB:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].MsgSize & 0xFF;
			_commoTXState = TWB_STX;
			break;
		case TWB_STX:
			ch = 0x02;
			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum = 0x00;
			_commoTXState = TWB_Payload;
			TWB_Commo_TXChIdx = 0x00;

			if(TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].MsgSize == 0)
				_commoTXState = TWB_ETX;

			break;
		case TWB_Payload:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].MsgBuffer[TWB_Commo_TXChIdx++];

			TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum ^= ch;

			if(TWB_Commo_TXChIdx == TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].MsgSize){
				_commoTXState = TWB_ETX;
			}
			break;
		case TWB_ETX:
			ch = 0x03;
			_commoTXState = TWB_CheckSum;
			break;
		case TWB_CheckSum:
			ch = TWB_Outgoing_Messages[TWB_Commo_TxMsgHead].CheckSum;
			_commoTXState = TWB_EOT;
			break;
		case TWB_EOT:
			TWB_Timer_Reset(WiFiHeartBeatTimer);

			ch = 0x04;
			TWB_Commo_TxMsgHead++;
			if(TWB_Commo_TxMsgHead == MSG_QUEUE_SIZE_OUT)
				TWB_Commo_TxMsgHead = 0;

			//If we have sent the last character and have caught up in the queue, disable the IRQ
			if(TWB_Commo_TxMsgHead == TWB_Commo_TxMsgTail)
				USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

			_commoTXState = TWB_SOH;

			break;

	}
#ifdef STM32F0
	USART2->TDR = (ch & (uint16_t)0x01FF);
#endif

	USART2->DR = (ch & (uint16_t)0x01FF);
}

void TWB_MsgParser_HandleRX(uint8_t ch){
	switch(_commoParserState){
	case TWB_SOH:
		if(ch == 0x01){ //SOH
			_commoParserState = TBW_ExpectACK;
			TWB_Commo_RxChIdx = 0;
		}
		else{
			if(WiFiState == WiFi_CLIMode)
				TWB_Debug_AddCh(ch);
		}

		break;
	case TBW_ExpectACK:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum = 0x00;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].ExpectACK = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;

		_commoParserState = TWB_SourceAddress;
		break;
	case TWB_SourceAddress:
		__srcAddress = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		_commoParserState = TWB_DestAddress;
		break;
	case TWB_DestAddress:
		__dstAddress = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		_commoParserState = TWB_SystemId;
		break;
	case TWB_SystemId:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SystemId = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		_commoParserState = TWB_MessageType;
		break;
	case TWB_MessageType:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].TypeId = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		_commoParserState = TWB_SerialNumberMSB;
		break;
	case TWB_SerialNumberMSB:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber = 0;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber = ch << 8;
		_commoParserState = TWB_SerialNumberLSB;
		break;
	case TWB_SerialNumberLSB:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber |= ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		_commoParserState = TWB_SizeMSB;
		break;
	case TWB_SizeMSB:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize = 0;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize = ch << 8;
		_commoParserState = TWB_SizeLSB;
		break;
	case TWB_SizeLSB:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize |= ch;
		if(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize > 1024){
			_commoParserState = TWB_SOH;
			TWB_Debug_PrintInt("Message size too big", ch);
			TWB_Commo_SendNak(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber);
		}
		else
			_commoParserState = TWB_STX;
		break;
	case TWB_STX:
		if(ch == 0x02){ //STX
			if(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize == 0)
				_commoParserState = TWB_ETX;
			else
				_commoParserState = TWB_Payload;

		}
		else{ //If we don't see STX, something got wacked :(
			TWB_Debug_PrintInt("Exp STX", ch);
			_commoParserState = TWB_SOH;
			TWB_Commo_SendNak(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber);
		}
	break;
	case TWB_Payload:
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgBuffer[TWB_Commo_RxChIdx++] = ch;
		TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum ^= ch;
		if(TWB_Commo_RxChIdx == TWB_Incoming_Messages[TWB_Commo_RxMsgTail].MsgSize)
			_commoParserState = TWB_ETX;

		break;
	case TWB_ETX:
		if(ch == 0x03) //ETX
			_commoParserState = TWB_CheckSum;
		else{ //If we don't see STX, something got wacked :(
			_commoParserState = TWB_SOH;
			TWB_Debug_PrintInt("Exp: ETX, Rcv:", ch);
			TWB_Commo_SendNak(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber);
		}
		break;
	case TWB_CheckSum:
		_commoParserState = TWB_EOT;

		if(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum == ch)
			_commoParserState = TWB_EOT;
		else{
			_commoParserState = TWB_SOH;
			TWB_Debug_Print2Int("Checksum Mismatch ", ch, TWB_Incoming_Messages[TWB_Commo_RxMsgTail].CheckSum);
			TWB_Commo_SendNak(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber);
		}
		break;
	case TWB_EOT:
		if(ch == 0x04){ //EOT
			TWB_Commo_RxMsgTail++; //
			if(TWB_Commo_RxMsgTail == MSG_QUEUE_SIZE_IN)
				TWB_Commo_RxMsgTail = 0;
		}
		else
			TWB_Commo_SendNak(TWB_Incoming_Messages[TWB_Commo_RxMsgTail].SerialNumber);

		_commoParserState = TWB_SOH;

		break;
	}
}

