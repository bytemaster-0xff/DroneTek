#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "nivek_system.h"

#include "common\twb_debug.h"
#include "common\twb_leds.h"
#include "commo\wifi.h"
#include "commo\commo.h"

#include "sensors\snsr_main.h"
#include "sensors\gps.h"
#include "sensors\lipo_adc.h"
#include "flight\ctrl_loop.h"

#include "common\memory_mgmt.h"
#include "commo\msg_parser.h"

#define OUTGOING_MSG_QUEUE_SIZE 16
#define QUEUED_MAIN_LOOP_METHODS_SIZE 12;

uint8_t OutgoingMsgQueueHead = 0;
uint8_t OutgoingMsgQueueTail = 0;
OutgoingMsg_t OutgoingMsgQueue[OUTGOING_MSG_QUEUE_SIZE];

#define PING_TIMEOUT 5000  //ms

//Index into the message Queue where the app should pickup the next message (if not equal to RxMsgTail)
uint8_t TWB_Commo_RxMsgHead = 0;
//Index into the Message Queue where next message should be created
uint8_t TWB_Commo_RxMsgTail = 0;


//Index into the Message Queue to be picked up and sent out via UART
uint8_t TWB_Commo_TxMsgHead = 0;
//Index into the Message Queue where next message should be created to be sent out
uint8_t TWB_Commo_TxMsgTail = 0;

uint16_t TWB_Commo_RxChIdx = 0;
uint16_t TWB_Commo_TXChIdx = 0;

/*Use approximately 1648 bytes as a buffer for serial communications*/
TWB_Commo_Message_t TWB_Incoming_Messages[MSG_QUEUE_SIZE_IN]; /* Total of 828 bytes used for commo messages */
TWB_Commo_Message_t TWB_Outgoing_Messages[MSG_QUEUE_SIZE_OUT]; /* Total of 828 bytes used for commo messages */


uint16_t __serialNumber = 0;

uint8_t __messageInProcessFlag = RESET;

void __commo_MsgBegin(void){
	while(__messageInProcessFlag == SET);
	__messageInProcessFlag = SET;
}

//This message should be called once a message has been
//prepared to be sent out.  It will queue it for transmission
//and set the IRQ to fire when a character has been sent.
void __commo_MsgCommit(void) {
	TWB_Commo_TxMsgTail++;

	if(TWB_Commo_TxMsgTail == MSG_QUEUE_SIZE_OUT)
		TWB_Commo_TxMsgTail = 0;

	if(TWB_Commo_TxMsgTail == TWB_Commo_TxMsgHead)
		TWB_Debug_Print("Tail caught head :(");

	//First off, after we increment the outgoing message buffer,
	//Point to our new outgoing buffer where data can be sent.
	TWB_Outgoing_Messages[TWB_Commo_TxMsgTail].SerialNumber = __serialNumber++;

	//Enable Transmitter Empty IRQ, so once the ch has been sent we either
		//load up a new character or determine the transmission has been completed.
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

	__messageInProcessFlag = RESET;
}

iOpResult_e TWB_Commo_Init(void){
	pm_printHeapSize();

	for(uint8_t idx = 0; idx < MSG_QUEUE_SIZE_OUT; ++idx){
		TWB_Outgoing_Messages[idx].MsgBuffer = pm_malloc(MSG_BUFFER_PAYLOAD_LEN_OUT);
	}

	for(uint8_t idx = 0; idx < MSG_QUEUE_SIZE_IN; ++idx){
		TWB_Incoming_Messages[idx].MsgBuffer = pm_malloc(MSG_BUFFER_PAYLOAD_LEN_IN);
	}

	pm_printHeapSize();

	return OK;
}

void TWB_Commo_ProcessOutgoingMessages(void){
	if(OutgoingMsgQueueHead == OutgoingMsgQueueTail)
		return;

	switch(OutgoingMsgQueue[OutgoingMsgQueueHead++]){
		case Outgoing_Msg_Sensor: TWB_Sensors_QueueMsg(); break;
		case Outgoing_Msg_GPS: TWB_GPS_QueueMsg(); break;
		case Outgoing_Msg_Battery: TWB_ADC_QueueMsg(); break;
	}

	if(OutgoingMsgQueueHead == OUTGOING_MSG_QUEUE_SIZE)
		OutgoingMsgQueueHead = 0;
}

void TWB_Commo_ProcessIncomingMessages(void){
	while(TWB_Commo_RxMsgTail != TWB_Commo_RxMsgHead){
		TWB_Timer_Reset(WiFiInWatchDog);

		iOpResult_e result;

		switch(TWB_Incoming_Messages[TWB_Commo_RxMsgHead].SystemId){
			case MOD_Sensor:
				result = TWB_Sensors_HandleMessage(&TWB_Incoming_Messages[TWB_Commo_RxMsgHead]);
				break;
			case MOD_FlightControls:
				result = TWB_CtrlLoop_HandleMessage(&TWB_Incoming_Messages[TWB_Commo_RxMsgHead]);
				break;
			case MOD_System:
				result = TWB_Sys_HandleMessage(&TWB_Incoming_Messages[TWB_Commo_RxMsgHead]);
				break;
				default:
					TWB_Debug_Print("Other Message\r\n");
					result = UnknownMessageType;
					break;
		}

		if(TWB_Incoming_Messages[TWB_Commo_RxMsgHead].ExpectACK)
		{
			uint8_t buffer[2];
			buffer[0] = TWB_Incoming_Messages[TWB_Commo_RxMsgHead].SerialNumber >> 8;
			buffer[1] = TWB_Incoming_Messages[TWB_Commo_RxMsgHead].SerialNumber & 0xFF;

			TWB_Commo_SendMessage(MOD_System, result == OK ? ACK : NAK, buffer, 2);
		}

		TWB_Commo_RxMsgHead++;

		if(TWB_Commo_RxMsgHead == MSG_QUEUE_SIZE_IN)
			TWB_Commo_RxMsgHead = 0;
	}
}


void TWB_Commo_SendMessage(uint8_t systemId, uint8_t msgId, uint8_t *buffer, uint16_t size){
	if(WiFiIsConnected == WiFiConnected){ /* CLI Echo to WiFly, special case where we configure communication channel */
		__commo_MsgBegin();
		TWB_Outgoing_Messages[TWB_Commo_TxMsgTail].SystemId = systemId;
		TWB_Outgoing_Messages[TWB_Commo_TxMsgTail].TypeId = msgId;
		if(size > 0)
			memcpy(TWB_Outgoing_Messages[TWB_Commo_TxMsgTail].MsgBuffer, buffer, size);

		TWB_Outgoing_Messages[TWB_Commo_TxMsgTail].MsgSize = size;

		__commo_MsgCommit();
	}
}

void TWB_Commo_SendKeepAlivePing(void){
	uint8_t buffer[2];

	buffer[0] = SystemStatus->SnsrState;
	buffer[1] = SystemStatus->GPIOState;

	TWB_Commo_SendMessage(MOD_System, MSG_PING, buffer, 2);
	TWB_Commo_ProcessOutgoingMessages();
	TWB_Debug_Print("Sending Keep Alive Ping\r\n");
}


//The tasks, OneHz, TwoHz, etc are called via an IRQ
//don't really want to do much work here.  If we need
//to send out a message queue it to be picked up in the
//main loop.
//
//Also to avoid multitasking issues, all messages
//are queued for UART delivery in the main loop since
//they might take a few clock cycles to build up
//and we don't want an IRQ coming in and adding a
//message in the middle of a message.
//
void TWB_Commo_QueueOutgoingMsg(OutgoingMsg_t msgType){
	if(WiFiIsConnected == WiFiConnected){
		OutgoingMsgQueue[OutgoingMsgQueueTail++] = msgType;
		if(OutgoingMsgQueueTail == OUTGOING_MSG_QUEUE_SIZE)
			OutgoingMsgQueueTail = 0;
	}
}

void TWB_Main_WiFiSendExpired(void){
	TWB_Commo_SendMessage(MOD_System, MSG_PING, null, 0);
	TWB_Debug_Print("Sending Keep Alive Ping\r\n");
}

void TWB_Main_WIFiReceiveExpired(void){
	WiFiIsConnected = WiFiDisconnected;
	TWB_Timer_Disable(WiFiHeartBeatTimer);

	TWB_Debug_Print("\r\nTelemetry heart beat lost\r\n\r\n");

	TWB_WiFi_Reboot();

	TWB_LEDS_SetState(LED_WiFi_Connected, LED_Off);

	if(WiFiControlled == SystemStatus->ControlMethod){
		SystemStatus->SystemState = SystemReady;
		SystemStatus->GPIOState = GPIOAppState_Idle;
		SystemStatus->SnsrState = SensorAppState_Online;
		SystemStatus->ArmedStatus = Safe;
	}
}
