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

#ifndef PicopterRadio_h

#include "ZigduinoRadioCfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "radio.h"
#include "board.h"
#include "transceiver.h"

#ifdef __cplusplus
} // extern "C"
#endif

// device address definitions
#define PCR_FLIER_DEVADDR 0x12
#define PCR_CTRLER_DEVADDR 0x34
#define PCR_RCCHAN_CNT 6 // number of RC channels
#define PCR_RFCHAN_MAX 3 // should be 16 in theory
#define PCR_RFCHAN_START 11

// these button maps correspond with the bits of the Wii Classic Controller button bytes
//  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
// 4| BDR | BDD | BLT | B-  | BH  | B+  | BRT |  1  |
// 5| BZL | BY  | BY  | BA  | BX  | BZR | BDL | BDU |
#define PCC_ARM 10
#define PCC_DISARM 12
#define PCC_ZEROSENSORS 3
#define PCC_ACROMODE 9
#define PCC_STABLEMODE 2
#define PCC_SYNCCHAN 4
#define PCC_WIICALI 5
#define PCC_ACCTRIM 7
#define PCC_ACCTRIMUP 0
#define PCC_ACCTRIMDOWN 14
#define PCC_ACCTRIMLEFT 1
#define PCC_ACCTRIMRIGHT 15

#ifdef __cplusplus
#include <Stream.h>
#include <NiVekMessage.h>
#include <Print.h>

#define OUTGOING_MESSAGE_QUEUE_SIZE 10

#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04

#define ACK 0x06
#define NAK 0x15

// just a class definition, for usage and comments, see the c/cpp file
class cPicopterRadio /*: public Stream*/
{
    private:
		uint8_t m_802_15_4_Channel;
		uint16_t m_dlnId; //Drone Local Network Id
		uint8_t m_deviceAddress;
		bool m_isCoordinator;

		NiVekMessage *m_messageQueue[OUTGOING_MESSAGE_QUEUE_SIZE];

		uint8_t m_messageQueueHead;
		uint8_t m_messageQueueTail;

		uint8_t m_hasRepliedToDiscovery;

    public:
        cPicopterRadio();
		void init(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress);
		void beginAsDrone(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress);
		void beginAsCoordinator(uint8_t channel, uint16_t dlnId, uint8_t deviceAddress);
        void requestDiscovery();
        uint8_t hasDiscovered();
        void setChannel(channel_t chan);
        void sendFlightCommands(uint8_t droneAddress);
        void setRxMode();

		NiVekMessage *getNextOutgoingMessage();
		bool isOutgoingMessageReady();
		bool getIsCoordinator();

        uint8_t rand();
        uint8_t isPresent();

		uint8_t getDeviceAddress();
		void setDeviceAddress(uint8_t addr);

        virtual int available(void);
        virtual int read(void);


		void setHasRepliedToDiscovery(uint8_t);

		void commit(NiVekMessage *msg);
};

extern cPicopterRadio PicopterRadio; // make single instance accessible

#else

// function prototypes if compiled as C
void pcr_begin(uint8_t);
void pcr_requestDiscovery();
uint8_t pcr_hasDiscovered();
int pcr_available(void);
int pcr_peek(void);
int pcr_read(void);
void pcr_flush(void);
uint8_t pcr_rand();
uint8_t pcr_isPresent();
void pcr_setFlushOnNext();
void write(NiVekMessage *msg);

#endif

// function prototypes for both C and C++
void pcr_flushTx(void);
void pcr_write(uint8_t);
void pcr_setRxMode(void);

extern volatile uint16_t pcr_rcChan[PCR_RCCHAN_CNT]; // load or read RC channels from here

#define PicopterRadio_h
#endif