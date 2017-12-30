

#include "common\twb_i2c.h"


void __i2c_EnableInterrupts(FunctionalState state){
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannelCmd = state;

  NVIC_InitStructure.NVIC_IRQChannel = I2C2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 2;

  NVIC_Init(&NVIC_InitStructure);
  I2C_ITConfig(i2c, I2C_IT_ERR, state);

}


void I2C2_IRQHandler(void){
  /*if((i2c->ISR & I2C_ERROR_MASK)){ // Error?
    i2cError = i2c->ISR & I2C_ERROR_MASK;
    i2c->ICR |= I2C_ERROR_MASK;
  } else { // NAK?
    i2cEvent = i2c->ISR & I2C_EVENT_MASK;
    i2c->ICR |= I2C_IT_NACKF;
  }


  __i2cAttention = I2C_ATTENTION_EVENT;*/
}


iOpResult_e TWB_I2C_Init(I2C_TypeDef *port){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	GPIO_InitTypeDef gpioI2C;
	gpioI2C.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	gpioI2C.GPIO_Mode = GPIO_Mode_AF;
	gpioI2C.GPIO_OType = GPIO_OType_OD;
	gpioI2C.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioI2C.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &gpioI2C);

	I2C_DeInit(i2c);

	i2c = port;

	I2C_InitTypeDef i2cCfg;
	i2cCfg.I2C_Mode = I2C_Mode_I2C;
	i2cCfg.I2C_OwnAddress1 = 0x00;
	i2cCfg.I2C_Ack = I2C_Ack_Enable;
	i2cCfg.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	i2cCfg.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	i2cCfg.I2C_DigitalFilter = 0;

	i2cCfg.I2C_Timing = 0x50330309; //Trust me on this one...gives us an I2C Clock of about 450KHz.  Review table 73 in STM32F0 docs, would still like a little better understanding.

	I2C_Init(i2c, &i2cCfg);

	I2C_Cmd(i2c, ENABLE);

	__i2c_EnableInterrupts(ENABLE);

	TWB_Debug_Print("I2C Online\r\n");

	return OK;
}

iOpResult_e TWB_I2C_RawWrite(uint16_t address, uint8_t* buffer, uint8_t count){
	__i2cAttention = I2C_ATTENTION_NO_EVENT;
	uint8_t bytesWritten = 0;
	address <<= 1;

	I2C_MasterRequestConfig(i2c, I2C_Direction_Transmitter);
	I2C_TransferHandling(i2c, address, count, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

	//TWB_Timer_Enable(I2CWatchDog);

	while(!__i2cAttention && count){
	  while((i2c->ISR & I2C_FLAG_TXE) == RESET);

	  I2C_SendData(i2c, *buffer++);
	  bytesWritten++;
	  count--;
	  while(!__i2cAttention){
		if((i2c->ISR & I2C_FLAG_TXIS) != RESET)
		  break;

		if((i2c->ISR & I2C_FLAG_STOPF) != RESET){
			i2c->ICR = I2C_FLAG_STOPF;
		  break;
		}
	  }
	}
	//TWB_Timer_Disable(I2CWatchDog);

	i2cCheckResult(__func__);

	return OK;
}

iOpResult_e TWB_I2C_RawRead(uint16_t address, uint8_t* buffer, uint8_t count){
    address <<= 1;

    I2C_MasterRequestConfig(i2c, I2C_Direction_Receiver);
    I2C_TransferHandling(i2c, address, count, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
    __i2cAttention = I2C_ATTENTION_NO_EVENT;

	//TWB_Timer_Enable(I2CWatchDog);

    while(!__i2cAttention && count){
      while(!__i2cAttention){
        if((i2c->ISR & I2C_FLAG_RXNE) != RESET){
          *buffer++ = I2C_ReceiveData(i2c);
          count--;
          break;
        }
        if((i2c->ISR & I2C_FLAG_STOPF) != RESET){
        	i2c->ICR = I2C_FLAG_STOPF;
          break;
        }
      }
    }
    //TWB_Timer_Disable(I2CWatchDog);

    i2cCheckResult(__func__);

    return OK;
}


