/*
 * twb_serial_eeprom.c
 *
 *  Created on: Oct 4, 2012
 *      Author: kevinw
 */

#include "common/twb_serial_eeprom.h"
#include "common/twb_debug.h"
#include "common/twb_i2c.h"
#include "common/twb_i2c.h"
#include "common/twb_timer.h"

//#define EEPROM_ADDR 0xA0
#define EEPROM_ADDR 0x50

#define SERIAL_EEPROM_WRITE_WAIT 5

#define EEPROM_BUFFER_SIZE 36

uint8_t __eepromWriteBuffer[EEPROM_BUFFER_SIZE];
uint8_t __eepromReadBuffer[EEPROM_BUFFER_SIZE];

iOpResult_e TWB_SE_Init() {
	return OK;
}

iOpResult_e TWB_SE_ReadBuffer(uint16_t address, uint8_t *out, uint8_t len) {
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;

	assert_succeed(TWB_I2C_WriteReadBuffer(EEPROM_ADDR, __eepromWriteBuffer, out, 2, len));

	return OK;
}

iOpResult_e TWB_SE_ReadU8(uint16_t address, uint8_t *out) {
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;

	assert_succeed(TWB_I2C_WriteReadBuffer(EEPROM_ADDR, __eepromWriteBuffer, __eepromReadBuffer, 2, 1));
	*out = (uint8_t) __eepromReadBuffer[0];

	return OK;
}

iOpResult_e TWB_SE_ReadU16(uint16_t address, uint16_t *out){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;

	assert_succeed(TWB_I2C_WriteReadBuffer(EEPROM_ADDR, __eepromWriteBuffer, __eepromReadBuffer, 2, 2));
	*out = (uint16_t)__eepromReadBuffer[0] << 8 | __eepromReadBuffer[1];

	return OK;
}

iOpResult_e TWB_SE_ReadU32(uint16_t address, uint32_t *out){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	assert_succeed(TWB_I2C_WriteReadBuffer(EEPROM_ADDR, __eepromWriteBuffer, __eepromReadBuffer, 2, 4));
	*out =(uint32_t)(__eepromReadBuffer[0] << 24 | __eepromReadBuffer[1] << 16 | __eepromReadBuffer[2] << 8 | __eepromReadBuffer[3]);

	return OK;
}

iOpResult_e TWB_SE_WriteU8(uint16_t address, uint8_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value;

	return TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 3);
}

iOpResult_e TWB_SE_WriteU8Pause(uint16_t address, uint8_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value;

	assert_succeed(TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 3));
	TWB_Timer_SleepMS(SERIAL_EEPROM_WRITE_WAIT);
	return OK;
}

iOpResult_e TWB_SE_WriteS8Pause(uint16_t address, int8_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = (int8_t)value;

	assert_succeed(TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 3));
	TWB_Timer_SleepMS(SERIAL_EEPROM_WRITE_WAIT);
	return OK;
}

iOpResult_e TWB_SE_WriteU16(uint16_t address, uint16_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value >> 8;
	__eepromWriteBuffer[3] = value & 0xFF;

	return TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 4);
}

iOpResult_e TWB_SE_WriteU16Pause(uint16_t address, uint16_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value >> 8;
	__eepromWriteBuffer[3] = value & 0xFF;

	assert_succeed(TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 4));
	TWB_Timer_SleepMS(SERIAL_EEPROM_WRITE_WAIT);
	return OK;
}

iOpResult_e TWB_SE_WriteU32(uint16_t address, uint32_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value >> 24;
	__eepromWriteBuffer[3] = value >> 16;
	__eepromWriteBuffer[4] = value >> 8;
	__eepromWriteBuffer[5] = value & 0xFF;

	return TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 6);
}

iOpResult_e TWB_SE_WriteU32Pause(uint16_t address, uint32_t value){
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;
	__eepromWriteBuffer[2] = value >> 24;
	__eepromWriteBuffer[3] = value >> 16;
	__eepromWriteBuffer[4] = value >> 8;
	__eepromWriteBuffer[5] = value & 0xFF;

	assert_succeed(TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 6));

	TWB_Timer_SleepMS(SERIAL_EEPROM_WRITE_WAIT);

	return OK;
}

iOpResult_e TWB_SE_WriteBuffer(uint16_t address, uint8_t *buffer, uint8_t count) {
	__eepromWriteBuffer[0] = address >> 8;
	__eepromWriteBuffer[1] = address & 0xFF;

	if(count > EEPROM_BUFFER_SIZE - 2) /* Knock off two for address */
		TWB_Debug_PrintInt("ERROR MAX BUFFER SIZE", EEPROM_BUFFER_SIZE);

	for(uint8_t idx = 0; idx < count; ++idx)
		__eepromWriteBuffer[2 + idx] = buffer[idx];

	return TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 2 + count);
}

iOpResult_e TWB_SE_WriteBufferPause(uint16_t address, uint8_t *buffer, uint8_t count) {
	assert_succeed(TWB_I2C_WriteBuffer(EEPROM_ADDR, __eepromWriteBuffer, 3));
	TWB_Timer_SleepMS(SERIAL_EEPROM_WRITE_WAIT);

	return OK;
}


iOpResult_e TWB_SE_ReadS8(uint16_t address, int8_t *out) {
	return TWB_SE_ReadU8(address, (uint8_t*)out);
}

iOpResult_e TWB_SE_ReadS16(uint16_t address, int16_t *out) {
	return TWB_SE_ReadU16(address, (uint16_t*)out);
}

iOpResult_e TWB_SE_ReadS32(uint16_t address, int32_t *out) {
	return TWB_SE_ReadU32(address, (uint32_t*)out);
}

iOpResult_e TWB_SE_WriteS8(uint16_t address, int8_t data) {
	return TWB_SE_WriteU8(address, (uint8_t)data);
}

iOpResult_e TWB_SE_WriteS16(uint16_t address, int16_t data) {
	return TWB_SE_WriteU16(address, (uint16_t)data);
}

iOpResult_e TWB_SE_WriteS16Pause(uint16_t address, int16_t data) {
	return TWB_SE_WriteU16Pause(address, (uint16_t)data);
}

iOpResult_e TWB_SE_WriteS32(uint16_t address, int32_t data) {
	return TWB_SE_WriteU32(address, (uint32_t)data);
}

iOpResult_e TWB_SE_WriteS32Pause(uint16_t address, int32_t data) {
	return TWB_SE_WriteU32Pause(address, (uint32_t)data);
}


iOpResult_e TWB_SE_QueueAsyncWrite(I2CJob_t* job){
	job->Address = EEPROM_ADDR;
	return TWB_I2C_QueueAsyncJob(job);
}
