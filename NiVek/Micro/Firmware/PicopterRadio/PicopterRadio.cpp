/* Copyright (c) 2011 Frank Zhao
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/*
 * Please note, this file is designed to support both compilation with a C compiler and C++ compiler
*/

#include "PicopterRadio.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define BROADCAST_DEVICE_ADDRESS 0xFF
#define null 0x00

#define PCR_FRAME_START_IDX 7

/* Differnt packet types */
#define PCR_DATATYPE_FLIGHTCOMMAND 0
#define PCR_DATATYPE_SERIAL 1
#define PCR_DATATYPE_DISCOVER 2
#define PCR_DATATYPE_DISCOVER_REPLY 3

#define PCR_RX_FIFO_SIZE 128 // size for the RX FIFO ring buffer
#define PCR_TX_FIFO_SIZE (MAX_FRAME_SIZE - PCR_FRAME_START_IDX - 3 - 2 - 2) // size for the TX FIFO ring buffer
#define PCR_SERTX_FIFO_SIZE 128 // size for the serial TX FIFO ring buffer

#define PCR_CHAN_EEPROM_ADDR ((uint8_t *)(E2END - 5))

// these macros and pointers are designed to eliminate the overhead of function calls
#define RADIO_RX_FRAME_PTR    ((uint8_t*)&TRXFBST)
#define RADIO_RX_LENGTH       (TST_RX_LENGTH)
#define RADIO_RX_GETLENGTH()  (TST_RX_LENGTH)
#define RADIO_TX_FRAME_PTR    ((uint8_t*)&TRXFBST + 1)
#define RADIO_TX_SETLENGTH(x) TRXFBST = (x)
#define RADIO_STATUS_MASK     (_BV(TRX_CMD0) | _BV(TRX_CMD1) | _BV(TRX_CMD2) | _BV(TRX_CMD3) | _BV(TRX_CMD4))
#define PCR_PREPTX()          {\
                              TCNT4 = 0; tmr4_ovf_flag_tx = 0; tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;\
                              while ((tmpStatusReg == BUSY_RX || tmpStatusReg == BUSY_TX || tmpStatusReg == STATE_TRANSITION_IN_PROGRESS) && tmr4_ovf_flag_tx < 1) tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;\
                              TRX_STATE = CMD_PLL_ON;\
                              while (tmpStatusReg != PLL_ON) tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;\
                              RADIO_TX_FRAME_PTR[0] = 0x01;\
                              RADIO_TX_FRAME_PTR[1] = 0x80;\
                              RADIO_TX_FRAME_PTR[2] = 0x00;\
                              RADIO_TX_FRAME_PTR[3] = 0x11;\
                              RADIO_TX_FRAME_PTR[4] = 0x22;\
                              RADIO_TX_FRAME_PTR[5] = 0x33;\
                              RADIO_TX_FRAME_PTR[6] = 0x44;\
                              }
#define PCR_FINALIZETX(x)     {\
                              RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + (x)] = 0;\
                              RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + (x) + 1] = 0;\
                              RADIO_TX_SETLENGTH(PCR_FRAME_START_IDX + (x) + 2);\
                              ZR_RFTX_LED_ON();\
                              TRX_STATE = CMD_TX_START;\
                              }
#define PCR_WAITTXDONE()      {TCNT4 = 0; tmr4_ovf_flag_tx = 0; tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK; while (tmr4_ovf_flag_tx < 1 && (tmpStatusReg != RX_ON && tmpStatusReg != PLL_ON)) tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;}
#define PCR_ALLOWEDTX()       (tmr4_ovf_flag_rx1 >= pcr_waitThresh)
#define PCR_WAITFREEAIR()     {while (tmr4_ovf_flag_rx1 < pcr_waitThresh);}
#define PCR_SETRXMODE()       {TCNT4 = 0; tmr4_ovf_flag_tx = 0; tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;\
                              while ((tmpStatusReg == BUSY_RX || tmpStatusReg == BUSY_TX || tmpStatusReg == STATE_TRANSITION_IN_PROGRESS) && tmr4_ovf_flag_tx < 1) tmpStatusReg = TRX_STATUS & RADIO_STATUS_MASK;\
                              TRX_STATE = CMD_RX_ON;\
                              }

volatile uint8_t tmpStatusReg;

// Used for heart beat detection
volatile uint8_t tmr4_ovf_cnt = 0;
volatile uint8_t tmr4_ovf_flag_tx = 0xFF;
volatile uint8_t tmr4_ovf_flag_rx1 = 0xFF;
volatile uint8_t tmr4_ovf_flag_rx8 = 0xFF;
volatile uint8_t tmr4_ovf_cnt_rcrx = 0xFF;
uint8_t pcr_waitThresh;

// RX BUffer
volatile uint8_t pcr_rxRingBuffer[PCR_RX_FIFO_SIZE];
volatile uint8_t pcr_rxRingBufferHead = 0;
volatile uint8_t pcr_rxRingBufferTail = 0;

// stores incoming and outgoing RC channel data
volatile uint16_t pcr_rcChan[PCR_RCCHAN_CNT];

// single global instance, made available for use
cPicopterRadio PicopterRadio = cPicopterRadio();


cPicopterRadio::cPicopterRadio(){
	m_hasRepliedToDiscovery = 0;
	m_messageQueueHead = 0;
	m_messageQueueTail = 0;
}

void cPicopterRadio::init(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress){
	m_deviceAddress = deviceAddress;

	// note that radio_rfa.c has been heavily modified to suit our needs here
	radio_init(0, MAX_FRAME_SIZE);

	// set the channel
	uint8_t chan = eeprom_read_byte(PCR_CHAN_EEPROM_ADDR);
	chan = 0;
	radio_set_param(RP_CHANNEL((uint8_t) ((chan % PCR_RFCHAN_MAX) + PCR_RFCHAN_START)));

	radio_set_param(RP_DATARATE(TRX_OQPSK250));

	// default to receiver
	radio_set_state(STATE_RX);

	// enable the appropriate LEDs, including PA_EXT
#ifdef ENABLE_DIG3_DIG4
	trx_bit_write(SR_PA_EXT_EN, 1);
#endif

	// setup the UART
	//UBRR0 = 25; // 38400
	UBRR0 = 8; // 115200
	UCSR0A = 0;
	UCSR0B |= _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);

	// start timer 4
	TCCR4A = 0x00; // turn everything off
	TCCR4B = 0x01; // prescaler 1
	TIMSK4 = _BV(TOIE4); // enable overflow interrupt
	TCNT4 = 0;

	if (m_isCoordinator)
	{
		DDRB &= ~_BV(2);
		DDRB |= _BV(1);
		PORTB |= _BV(2);
		PORTB &= ~_BV(1);

		pcr_waitThresh = 4; // controller has to wait longer before allow transmission
	}
	else
	{
		DDRB &= ~_BV(1);
		DDRB |= _BV(2);
		PORTB |= _BV(1);
		PORTB &= ~_BV(2);

		pcr_waitThresh = 1;
	}

	//pcr_waitThresh = (m_isCoordinator) ? 4 : 1;
}

void cPicopterRadio::beginAsDrone(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress){
	m_isCoordinator = false;
	init(channel, dlnId, deviceAddress);
}

void cPicopterRadio::beginAsCoordinator(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress){
	m_isCoordinator = true;
	init(channel, dlnId, deviceAddress);
}

inline uint8_t __nvr_sendMessage(NiVekMessage *msg, uint8_t startIdx){
	uint8_t bufferIdx, idx;

	bufferIdx = startIdx;

	/* Add NiVek message header */
	RADIO_TX_FRAME_PTR[bufferIdx++] = SOH;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->ExpectACK;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->SourceAddress;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->DestAddress;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->SystemId;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->TypeId;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->SerialNumber >> 8;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->SerialNumber & 0xFF;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->MsgSize >> 8;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->MsgSize & 0xFF;

	RADIO_TX_FRAME_PTR[bufferIdx++] = STX;

	for (idx = 0; idx < msg->MsgSize; ++idx)
		RADIO_TX_FRAME_PTR[bufferIdx++] = msg->MsgBuffer[idx];

	RADIO_TX_FRAME_PTR[bufferIdx++] = ETX;
	RADIO_TX_FRAME_PTR[bufferIdx++] = msg->CheckSum;
	RADIO_TX_FRAME_PTR[bufferIdx++] = EOT;

	return bufferIdx;
}

/**
 * @brief Timer4 Overflow Interrupt
 *
 * sets some flags to indicate whether or not the overflow has occured
 */
ISR(TIMER4_OVF_vect)
{
    if (tmr4_ovf_cnt % 8 == 0)
    {
        tmr4_ovf_flag_tx = (tmr4_ovf_flag_tx < 128) ? (tmr4_ovf_flag_tx + 1) : tmr4_ovf_flag_tx;
        tmr4_ovf_flag_rx8 = (tmr4_ovf_flag_rx8 < 128) ? (tmr4_ovf_flag_rx8 + 1) : tmr4_ovf_flag_rx8;
        tmr4_ovf_cnt = 0;
    }
    
    tmr4_ovf_cnt++;
    tmr4_ovf_flag_rx1 = (tmr4_ovf_flag_rx1 < 128) ? (tmr4_ovf_flag_rx1 + 1) : tmr4_ovf_flag_rx1;
    tmr4_ovf_cnt_rcrx = (tmr4_ovf_cnt_rcrx < 128) ? (tmr4_ovf_cnt_rcrx + 1) : tmr4_ovf_cnt_rcrx;
}

void __nvr_readAdditionalSerialData() {
	uint8_t i;

	uint8_t payLoadLen = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 3 + (PCR_RCCHAN_CNT * 2)];
	for (i = 0; i < payLoadLen; i++){
		uint8_t j = (pcr_rxRingBufferHead + 1) % PCR_RX_FIFO_SIZE;
		if (j != pcr_rxRingBufferTail){
			pcr_rxRingBuffer[pcr_rxRingBufferHead] = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 4 + (PCR_RCCHAN_CNT * 2) + i];
			pcr_rxRingBufferHead = j;
		}
		else
			break;
	}
}

void __nvr_handleFlightDataMessage(){
	uint8_t checksum = 0;
	uint8_t b;
	uint8_t i;
	uint16_t tempRead[PCR_RCCHAN_CNT];
	for (i = 0; i < PCR_RCCHAN_CNT; i++){
		b = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 2 + (i * 2)];
		tempRead[i] = b;
		checksum += b;
		b = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 3 + (i * 2)];
		tempRead[i] |= b << 8;
		checksum += b;
	}

	if (RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 2 + (PCR_RCCHAN_CNT * 2)] == checksum){

		for (i = 0; i < PCR_RCCHAN_CNT; i++)
			pcr_rcChan[i] = tempRead[i];

		tmr4_ovf_cnt_rcrx = 0;

		__nvr_readAdditionalSerialData();
	}
}

NiVekMessage *cPicopterRadio::getNextOutgoingMessage() {
	if (m_messageQueueHead != m_messageQueueTail){
		NiVekMessage *msg = m_messageQueue[m_messageQueueTail++];
		if (m_messageQueueTail == OUTGOING_MESSAGE_QUEUE_SIZE)
			m_messageQueueTail = 0;

		return msg;
	}
	else
		return null;
}

void __nvr_handleSerialData() {
	uint8_t i;
	uint8_t len = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 2];
	for (i = 0; i < len; i++){
		uint8_t j = (pcr_rxRingBufferHead + 1) % PCR_RX_FIFO_SIZE;
		if (j != pcr_rxRingBufferTail){
			pcr_rxRingBuffer[pcr_rxRingBufferHead] = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX + 3 + i];
			pcr_rxRingBufferHead = j;
		}
		else
			break;
	}
}

void __nvr_resetTimers() {
	TCNT4 = 0; 
	tmr4_ovf_flag_rx8 = 0; 
	tmr4_ovf_flag_rx1 = 0;
}

/* Full NiVek Frame 
[0] = 0x01
[1] = 0x80
[2] = 0x00
[3] = 0x11
[4] = 0x22
[5] = Drone Local Network ID (MSB)
[6] = Drone Local Network ID (LSB) <-- End of PCR_FRAME_START

[7] = Destination Address (0xFF for broadcast)
[8] = Message Type (RC Data, NiVek Message, Discovery Request, Discovery Reply)
[9] = Full message size (not including the first 7 bytes included in PCR_FRAME_START

[10]......[X] NiVek Messages (follows <SOH>...<EOT> format

[X+1] = 0x00
[X+2] = 0x00

*/


bool __nvr_validateMessageHeader() {
	return RADIO_RX_FRAME_PTR[0] == 0x01 &&
		RADIO_RX_FRAME_PTR[1] == 0x80 &&
		RADIO_RX_FRAME_PTR[2] == 0x00 &&
		RADIO_RX_FRAME_PTR[3] == 0x11 &&
		RADIO_RX_FRAME_PTR[4] == 0x22 &&
		RADIO_RX_FRAME_PTR[5] == 0x33 &&
	    RADIO_RX_FRAME_PTR[6] == 0x44;
}

void __nvr_replyToDiscovery(){
	PCR_PREPTX();
	RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 0] = PicopterRadio.getDeviceAddress(); // set payload
	RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 1] = PCR_DATATYPE_DISCOVER_REPLY; // set payload
	PCR_FINALIZETX(2);
	PCR_WAITTXDONE();
	PCR_SETRXMODE();
}

void __nvr_sendOutgoingMsgs() {
	/* Need to reconsider this...*/
	tmr4_ovf_flag_rx1 = pcr_waitThresh + 1;

	uint8_t newBufferPtr = PCR_FRAME_START_IDX;
	NiVekMessage *msg = PicopterRadio.getNextOutgoingMessage();
	if (msg != null){
		PCR_WAITFREEAIR();
		PCR_PREPTX();

		/* Add lower level protocol header */
		//RADIO_TX_FRAME_PTR[bufferIdx++] = msg->DestAddress; // set payload
		RADIO_TX_FRAME_PTR[newBufferPtr++] = msg->DestAddress; // set payload
		RADIO_TX_FRAME_PTR[newBufferPtr++] = PCR_DATATYPE_SERIAL; // set payload
		RADIO_TX_FRAME_PTR[newBufferPtr++] = 0x00; /* Place Holder for Length */

		newBufferPtr = __nvr_sendMessage(msg, newBufferPtr);

		RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 2] = newBufferPtr - (PCR_FRAME_START_IDX + 2);

		RADIO_TX_FRAME_PTR[newBufferPtr++] = 0;
		RADIO_TX_FRAME_PTR[newBufferPtr++] = 0;
		RADIO_TX_SETLENGTH(newBufferPtr);
		
		TRX_STATE = CMD_TX_START;
	}
}

ISR(TRX24_RX_END_vect){
	__nvr_resetTimers();

	/* Check to see if we have the expected header and we have the right address for our Drone Local Network ID. */
	if (!__nvr_validateMessageHeader()){
		return;		
	}

	uint8_t destAddress = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX];

    // Is the address for me, or is a broadcast message?
	if (PicopterRadio.getIsCoordinator() ||  /* I'm Cooridinator */
		destAddress == 0xFF || /* Broadcast */
		destAddress == PicopterRadio.getDeviceAddress()) /* For Me */
	{		
        uint8_t dataType = RADIO_RX_FRAME_PTR[PCR_FRAME_START_IDX+1];

		if (dataType == PCR_DATATYPE_FLIGHTCOMMAND){
			uint8_t idx;			
			__nvr_handleFlightDataMessage();
			
			__nvr_sendOutgoingMsgs(); 
			
		}
		else if (dataType == PCR_DATATYPE_SERIAL)
			__nvr_handleSerialData();
        else if (dataType == PCR_DATATYPE_DISCOVER)
			__nvr_replyToDiscovery();
        else if (dataType == PCR_DATATYPE_DISCOVER_REPLY)
			PicopterRadio.setHasRepliedToDiscovery(0xFF);
    }
	else

	__nvr_resetTimers();
}

void cPicopterRadio::requestDiscovery(){
	PicopterRadio.setHasRepliedToDiscovery(0xFF);

    PCR_PREPTX();
	RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 0] = m_deviceAddress;
    RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX+1] = PCR_DATATYPE_DISCOVER; // set payload
    PCR_FINALIZETX(2);
    PCR_WAITTXDONE();
    PCR_SETRXMODE();
}

void cPicopterRadio::setHasRepliedToDiscovery(uint8_t value){
	m_hasRepliedToDiscovery = value;
}

uint8_t cPicopterRadio::hasDiscovered(){
    return m_hasRepliedToDiscovery;
}

int cPicopterRadio::read(){
    if (pcr_rxRingBufferHead == pcr_rxRingBufferTail)
        return -1;
    else{
        uint8_t c = pcr_rxRingBuffer[pcr_rxRingBufferTail];
        pcr_rxRingBufferTail = (pcr_rxRingBufferTail + 1) % PCR_RX_FIFO_SIZE; // pop
        return c;
    }
}

int cPicopterRadio::available(){
    int res = PCR_RX_FIFO_SIZE;
    res += pcr_rxRingBufferHead;
    res -= pcr_rxRingBufferTail;
    res %= PCR_RX_FIFO_SIZE;
    return res;
}

ISR(TRX24_TX_END_vect){ 
	PCR_SETRXMODE();
}

void cPicopterRadio::sendFlightCommands(uint8_t droneAddress){
	
	
    uint8_t checksum = 0;
    uint8_t b;

    PCR_WAITFREEAIR();
    PCR_PREPTX();

    uint8_t dataLen = PCR_FRAME_START_IDX;

	RADIO_TX_FRAME_PTR[dataLen++] = droneAddress;
    RADIO_TX_FRAME_PTR[dataLen++] = PCR_DATATYPE_FLIGHTCOMMAND; // set payload

    for (uint8_t i = 0; i < PCR_RCCHAN_CNT; i++, dataLen += 2) {
        b = pcr_rcChan[i] & 0xFF;
        checksum += b;
        RADIO_TX_FRAME_PTR[dataLen] = b;
        b = (pcr_rcChan[i] & 0xFF00) >> 8;
        checksum += b;
        RADIO_TX_FRAME_PTR[dataLen+1] = b;
    }

    RADIO_TX_FRAME_PTR[dataLen++] = checksum;
	RADIO_TX_FRAME_PTR[dataLen++] = 0; /* This will contain the potential extra serial payload.*/

	uint8_t serialPayloadLen = 0;

	NiVekMessage *msg = PicopterRadio.getNextOutgoingMessage();
	if (msg != null){
		uint8_t primaryDataLen = dataLen;
		dataLen = __nvr_sendMessage(msg, dataLen);
		serialPayloadLen = dataLen - primaryDataLen;
	}

	RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 3 + (PCR_RCCHAN_CNT * 2)] = serialPayloadLen;

    //dataLen += 2;
    /*
	RADIO_TX_FRAME_PTR[PCR_FRAME_START_IDX + 2] = dataLen - (PCR_FRAME_START_IDX + 2);
	
	for (pcr_txRingBufferTail = 0; pcr_txRingBufferTail < pcr_txRingBufferHead; pcr_txRingBufferTail++, dataLen++)
        RADIO_TX_FRAME_PTR[dataLen] = pcr_txRingBuffer[pcr_txRingBufferTail];*/

/*	RADIO_TX_FRAME_PTR[dataLen++] = 0;
	RADIO_TX_FRAME_PTR[dataLen++] = 0;

	

	RADIO_TX_SETLENGTH(dataLen);

	TRX_STATE = CMD_TX_START;*/

	PCR_FINALIZETX(PCR_RCCHAN_CNT * 2 + // two bytes per RC Channel
		3 + 
		1 + //
		serialPayloadLen);
}

void cPicopterRadio::setChannel(channel_t chan){
	m_802_15_4_Channel = chan;
    radio_set_param(RP_CHANNEL(chan));
}

ISR(TRX24_RX_START_vect){
    TCNT4 = 0; 
	tmr4_ovf_flag_rx8 = 0; 
	tmr4_ovf_flag_rx1 = 0;
}

extern "C" { void radio_error(radio_error_t err) { PCR_SETRXMODE(); } } 

void cPicopterRadio::setRxMode(){
    PCR_SETRXMODE();
}

uint8_t cPicopterRadio::rand(){
    PCR_SETRXMODE();
    uint8_t r = 0;
    r |= (PHY_RSSI & 0x60) >> 5;
    _delay_us(2);
    r |= (PHY_RSSI & 0x60) >> 3;
    _delay_us(2);
    r |= (PHY_RSSI & 0x60) >> 1;
    _delay_us(2);
    r |= (PHY_RSSI & 0x60) << 1;
    return r;
}

uint8_t cPicopterRadio::isPresent(){
    return (tmr4_ovf_cnt_rcrx < 32) ? 1 : 0;
}


void cPicopterRadio::commit(NiVekMessage *msg){
	m_messageQueue[m_messageQueueHead++] = msg;

	if (m_messageQueueHead == OUTGOING_MESSAGE_QUEUE_SIZE)
		m_messageQueueHead = 0;
}

bool cPicopterRadio::getIsCoordinator() {
	return m_isCoordinator;
}

uint8_t cPicopterRadio::getDeviceAddress() {
	return m_deviceAddress;
}

void cPicopterRadio::setDeviceAddress(uint8_t addr) {
	m_deviceAddress = addr;
}

