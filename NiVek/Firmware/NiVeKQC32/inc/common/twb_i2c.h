/*
 * twb_i2c.h
 *
 *  Created on: Oct 3, 2012
 *      Author: kevinw
 */

#ifndef TWB_I2C_H_
#define TWB_I2C_H_

#include "twb_common.h"

iOpResult_e TWB_I2C_Init(I2C_TypeDef *port);

void TWB_I2C_Timeout(void);

iOpResult_e TWB_I2C_Write(uint8_t address, uint8_t size);
iOpResult_e TWB_I2C_Read(uint8_t address, uint8_t size);

iOpResult_e TWB_I2C_RawRead(uint16_t address, uint8_t* buffer, uint8_t count);
iOpResult_e TWB_I2C_RawWrite(uint16_t address, uint8_t* buffer, uint8_t count);

iOpResult_e TWB_I2C_ReadRegisterValue(uint8_t addr, uint8_t regaddr, uint8_t* out);
iOpResult_e TWB_I2C_ReadRegisterValues(uint8_t addr, uint8_t regaddr, uint8_t* out, uint8_t count);
iOpResult_e TWB_I2C_WriteRegister(uint8_t address, uint8_t regaddr, uint8_t value);
iOpResult_e TWB_I2C_ReadU16RegisterValue(uint8_t addr, uint8_t regaddr, uint16_t* out);

iOpResult_e TWB_I2C_SendU8(uint8_t addr, uint8_t data);

iOpResult_e TWB_I2C_WriteReadBuffer(uint8_t addr, uint8_t *writeBuffer, uint8_t *readBuffer, uint8_t writeLen, uint8_t readLen);
iOpResult_e TWB_I2C_WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t len);


void TWB_I2C_WatchdogExpired(void);


//Start state could either be Pending or Idle.  If the queue is empty,
//idle will be the initial state, otherwise, it will be dropped into
//the queue as pending.

typedef enum {
	Pending = 0x01,  //Item dropped in queue ready for processing by I2C IRQ Hander.
	Idle = 0x02,     //Item has been dequeued or (first one) and processing will begin shortly.
	PendingMasterMode  = 0x03,
	PendingAddressSent = 0x04,
	PendingSending  = 0x05,
	PendingReceive  = 0x06,

	PendingMasterModeRx  = 0x07,

	PendingReady = 0x08, //After the start condition is sent we wait until the SDL/SCL line are released.

	I2CCompleted  = 0x09,
	I2CAckFailure = 0x10,
	I2CTimeOut = 0x11
} I2CStates_t;


typedef enum {
	I2CSend,
	I2CReceive,
} I2CModes;


typedef struct I2CJob {
	I2CModes Mode;
	uint8_t Address;       //This is the core address of the device.
	uint8_t InBufferSize;
	uint8_t BytesSent;
	uint8_t OutBufferSize;
	uint8_t BytesReceived;
	iOpResult_e Result;

	uint8_t *InBuffer;
	uint8_t *OutBuffer;
	I2CStates_t State;
	void (*Callback)(void *result);
} I2CJob_t;

iOpResult_e TWB_I2C_QueueAsyncJob(I2CJob_t* job);
iOpResult_e TWB_I2C_ProcessJobSynchronously(I2CJob_t* job);

extern iOpResult_e __lastI2CEvent;

#define I2C_JOB_QUEUE_SIZE 12

extern uint8_t I2CQueueJobHead;
extern uint8_t I2CQueueJobTail;

extern I2CJob_t *I2CQueue[I2C_JOB_QUEUE_SIZE];

typedef enum {
	I2C_Error_None,
	I2C_BusBusy,
	I2C_WatchdogExpire

} I2C_LastError_e;

I2C_LastError_e TWB_I2C_GetLastError(void);



#endif /* TWB_I2C_H_ */
