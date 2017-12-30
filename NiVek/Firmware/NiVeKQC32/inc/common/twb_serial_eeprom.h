
#ifndef TWB_SERIAL_EEPROM_H_
#define TWB_SERIAL_EEPROM_H_

#include "twb_common.h"
#include "twb_i2c.h"

iOpResult_e TWB_SE_Init(void);

iOpResult_e TWB_SE_ReadBuffer(uint16_t address, uint8_t *out, uint8_t len);

iOpResult_e TWB_SE_ReadU8(uint16_t address, uint8_t *out);
iOpResult_e TWB_SE_ReadU16(uint16_t address, uint16_t *out);
iOpResult_e TWB_SE_ReadU32(uint16_t address, uint32_t *out);

iOpResult_e TWB_SE_WriteU8(uint16_t address, uint8_t data);
iOpResult_e TWB_SE_WriteU8Pause(uint16_t address, uint8_t value);

iOpResult_e TWB_SE_WriteU16(uint16_t address, uint16_t data);
iOpResult_e TWB_SE_WriteU16Pause(uint16_t address, uint16_t data);
iOpResult_e TWB_SE_WriteU32(uint16_t address, uint32_t data);
iOpResult_e TWB_SE_WriteU32Pause(uint16_t address, uint32_t data);

iOpResult_e TWB_SE_WriteBuffer(uint16_t address, uint8_t *buffer, uint8_t count);
iOpResult_e TWB_SE_WriteBufferPause(uint16_t address, uint8_t *buffer, uint8_t count);

iOpResult_e TWB_SE_ReadS8(uint16_t address, int8_t *out);
iOpResult_e TWB_SE_ReadS16(uint16_t address, int16_t *out);
iOpResult_e TWB_SE_ReadS32(uint16_t address, int32_t *out);

iOpResult_e TWB_SE_WriteS8(uint16_t address, int8_t data);
iOpResult_e TWB_SE_WriteS8Pause(uint16_t address, int8_t value);
iOpResult_e TWB_SE_WriteS16(uint16_t address, int16_t data);
iOpResult_e TWB_SE_WriteS16Pause(uint16_t address, int16_t data);
iOpResult_e TWB_SE_WriteS32(uint16_t address, int32_t data);
iOpResult_e TWB_SE_WriteS32Pause(uint16_t address, int32_t data);

iOpResult_e TWB_SE_QueueAsyncWrite(I2CJob_t* job);

#endif /* TWB_SERIAL_EEPROM_H_ */
