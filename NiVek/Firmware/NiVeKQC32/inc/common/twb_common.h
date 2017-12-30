/*
 * twb_common.h
 *
 *  Created on: Oct 3, 2012
 *      Author: kevinw
 */

#ifndef TWB_COMMON_H_
#define TWB_COMMON_H_

#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_i2c.h"

#include "twb_config.h"

#define bool unsigned char

#define true 0xFF
#define false 0x00

typedef enum {
  OK,
  FAIL,
  Ignored,
  Nack,
  I2C_BusError,
  ArbitrationLost,
  OverOrUnderRun,
  OutOfMemory,
  I2C_Timeout,
  I2C_GeneralFailure,
  InvalidBusSpeed,
  InvalidByteCount,
  UnknownMessageType
} iOpResult_e;



#define assert_succeed(result) if(result != OK) return result;
#define assert_succeed_msg(result, errMsg) if(result != OK) { TWB_Debug_Print(errMsg);  return result};

#define TWB_Buffer_PutCh(buff, ch) buff->Buffer[buff->PtrIn++] = ch; if (buff->PtrIn == buff->Size)  buff->PtrIn = 0;

typedef enum {
	EdgeTypeRise,
	EdgeTypeFall,
	EdgeTypeSame
} EdgeType_t;

//EdgeType_t TWB_GetEdgeType(uint16_t last, uint16_t this, uint16_t pin);

#endif /* TWB_COMMON_H_ */
