/*
 * commo.h
 *
 *  Created on: Dec 3, 2012
 *      Author: kevinw
 */

#ifndef COMMO_H_
#define COMMO_H_

#include "commo/message_ids.h"
#include "common/twb_timer.h"
#include "common/twb_common.h"

#define MSG_BUFFER_PAYLOAD_LEN_OUT 128
#define MSG_BUFFER_PAYLOAD_LEN_IN 64
//#define MSG_BUFFER_PAYLOAD_LEN 96
#define MSG_QUEUE_SIZE_OUT 64
#define MSG_QUEUE_SIZE_IN 20

#define ACK 0x06
#define NAK 0x15

#define MSG_FAILED_COMMAND 105


typedef struct {
  uint8_t ExpectACK;
  uint8_t SystemId;
  uint8_t TypeId;
  uint16_t MsgSize;
  uint16_t SerialNumber;
  uint8_t CheckSum;
  uint8_t *MsgBuffer;
} TWB_Commo_Message_t; //Size per structure 96 Buffer + 8 for framing = 104

extern Timer_t *WiFiInWatchDog;
extern Timer_t *SendTimer;

extern TWB_Commo_Message_t TWB_Incoming_Messages[MSG_QUEUE_SIZE_IN];  /* Total of 636 bytes used for commo messages */
extern TWB_Commo_Message_t TWB_Outgoing_Messages[MSG_QUEUE_SIZE_OUT]; /* Total of 636 bytes used for commo messages */

/*Total Size Used for buffers is 1248 */

//Index into the message Queue where the app should pickup the next message (if not equal to RxMsgTail)
extern uint8_t TWB_Commo_RxMsgHead;
//Index into the Message Queue where next message should be created
extern uint8_t TWB_Commo_RxMsgTail;

//Index into the Message Queue to be picked up and sent out via UART
extern uint8_t TWB_Commo_TxMsgHead;
//Index into the Message Queue where next message should be created to be sent out
extern uint8_t TWB_Commo_TxMsgTail;

extern uint16_t TWB_Commo_RxChIdx;
extern uint16_t TWB_Commo_TXChIdx;

iOpResult_e TWB_Commo_Init(void);

void TWB_Commo_ProcessOutgoingMessages(void);
void TWB_Commo_ProcessIncomingMessages(void);

void TWB_Commo_SendKeepAlivePing(void);
void TWB_Commo_SendMessage(uint8_t system, uint8_t msgType, uint8_t *buffer, uint16_t size);

void TWB_Main_WiFiSendExpired(void);
void TWB_Main_WIFiReceiveExpired(void);

iOpResult_e TWB_Commo_SendNak(uint16_t msgSerialNumber);

#endif /* COMMO_H_ */
