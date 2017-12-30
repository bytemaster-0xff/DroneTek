/*
 * twb_i2c.c
 *
 *  Created on: Oct 3, 2012
 *      Author: kevinw
 */

#include "common/twb_debug.h"
#include "common/twb_i2c.h"
#include "common/twb_timer.h"

I2C_TypeDef *i2c = I2C2;

I2CJob_t *I2CQueue[];

uint8_t __i2cTempInBuffer[6];
uint8_t __i2cTempOutBuffer[6];
extern I2CJob_t* __twbI2CSyncJob;

iOpResult_e TWB_I2C_WriteBuffer(uint8_t addr, uint8_t *writeBuffer, uint8_t writeLen){
	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = writeBuffer;
	__twbI2CSyncJob->OutBufferSize = writeLen;
	__twbI2CSyncJob->InBufferSize = 0;
	__twbI2CSyncJob->Callback = 0x00;

	return TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);
}

iOpResult_e TWB_I2C_WriteReadBuffer(uint8_t addr, uint8_t *writeBuffer, uint8_t *readBuffer, uint8_t writeLen, uint8_t readLen){
	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = writeBuffer;
	__twbI2CSyncJob->OutBufferSize = writeLen;
	__twbI2CSyncJob->InBuffer = readBuffer;
	__twbI2CSyncJob->InBufferSize = readLen;
	__twbI2CSyncJob->Callback = 0x00;

	return TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);
}

iOpResult_e TWB_I2C_ReadRegisterValue(uint8_t addr, uint8_t regaddr, uint8_t* out){
	__i2cTempOutBuffer[0] = regaddr;

	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->OutBufferSize = 1;
	__twbI2CSyncJob->InBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->InBufferSize = 1;
	__twbI2CSyncJob->Callback = 0x00;

	iOpResult_e result = TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);

	if(result == OK)
		*out = __i2cTempOutBuffer[0];

	return result;
}

iOpResult_e TWB_I2C_ReadU16RegisterValue(uint8_t addr, uint8_t regaddr, uint16_t* out){
	__i2cTempOutBuffer[0] = regaddr;

	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->OutBufferSize = 1;
	__twbI2CSyncJob->InBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->InBufferSize = 2;
	__twbI2CSyncJob->Callback = 0x00;

	iOpResult_e result = TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);

	if(result == OK)
		*out = __i2cTempOutBuffer[0] << 8 | __i2cTempOutBuffer[1];

	return result;
}


iOpResult_e TWB_I2C_WriteRegister(uint8_t addr, uint8_t regaddr, uint8_t value){
	__i2cTempOutBuffer[0] = regaddr;
	__i2cTempOutBuffer[1] = value;

	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->OutBufferSize = 2;
	__twbI2CSyncJob->InBufferSize = 0;
	__twbI2CSyncJob->Callback = 0x00;

	return TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);
}

iOpResult_e TWB_I2C_ReadRegisterValues(uint8_t addr, uint8_t regaddr, uint8_t* out, uint8_t count){

	__i2cTempOutBuffer[0] = regaddr;

	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->OutBufferSize = 1;
	__twbI2CSyncJob->InBufferSize = count;
	__twbI2CSyncJob->InBuffer = out;
	__twbI2CSyncJob->Callback = 0x00;

	return TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);
}

iOpResult_e TWB_I2C_SendU8(uint8_t addr, uint8_t data){

	__i2cTempOutBuffer[0] = data;

	__twbI2CSyncJob->Address = addr;
	__twbI2CSyncJob->OutBuffer = __i2cTempOutBuffer;
	__twbI2CSyncJob->OutBufferSize = 1;
	__twbI2CSyncJob->InBufferSize = 0;
	__twbI2CSyncJob->InBuffer = null;
	__twbI2CSyncJob->Callback = 0x00;

	return TWB_I2C_ProcessJobSynchronously(__twbI2CSyncJob);
}
