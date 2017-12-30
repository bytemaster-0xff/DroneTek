
#include "common\twb_i2c.h"
#include "common\memory_mgmt.h"
#include "common\twb_timer.h"
#include "common\twb_debug.h"

#define I2C_400K 400000

I2CJob_t *__twbI2CCurrentJob;
I2CJob_t *__twbI2CSyncJob;

uint8_t I2CQueueJobHead;
uint8_t I2CQueueJobTail;

I2C_LastError_e __i2cLastError = I2C_Error_None;

I2C_TypeDef *_myI2C = PRIMARY_I2C;

void __i2c_EnableInterrupts(FunctionalState state){
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannelCmd = state;
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; /* 0 = Highest Priority, 3 = Lowest Priority */
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	//Start out in a disabled state, will enable upon entry into IRQ driven I2C commo
	I2C_ITConfig(PRIMARY_I2C, I2C_IT_ERR, ENABLE);
	I2C_ITConfig(PRIMARY_I2C, I2C_IT_EVT, ENABLE);
	I2C_ITConfig(PRIMARY_I2C, I2C_IT_BUF, ENABLE);
}

void __twb_i2c_pauseBetweenTxAndRx(void){
	/* Need to spin just a little bit to allow I2C devices between start and stop bits
	 * sorta sucks we are blocking but it should only be less than a few microseconds */
	volatile uint16_t wait = 150; /* A count of 150 yields 10.5uS */
	while(wait-- > 0);
}

iOpResult_e __twb_i2c_startNextJob(void){
	/* Make this job current */
	__twbI2CCurrentJob = I2CQueue[I2CQueueJobTail++];
	if(I2CQueueJobTail == I2C_JOB_QUEUE_SIZE)
		I2CQueueJobTail = 0;

	__twbI2CCurrentJob->BytesReceived = (uint8_t)0;
	__twbI2CCurrentJob->BytesSent = (uint8_t)0;

	__i2cLastError = I2C_Error_None;

	TWB_Timer_Enable(I2CWatchDog);

    //Make sure we are not busy, we are going to block here, need to do some sort of time out on this.
	while(I2C_GetFlagStatus(PRIMARY_I2C, I2C_FLAG_BUSY) && __i2cLastError != I2C_WatchdogExpire);

	if(__i2cLastError != I2C_Error_None){
		__i2cLastError = I2C_BusBusy;
		__twbI2CCurrentJob->Result = I2C_BusBusy;
		__twbI2CCurrentJob->State = I2CTimeOut;
		return I2C_BusError;
	}


	/* Set the starting mode for the I2C transfer */
	__twbI2CCurrentJob->Mode = (__twbI2CCurrentJob->OutBufferSize > 0) ? I2CSend : I2CReceive;

	/* Now set the current state */
	__twbI2CCurrentJob->State = PendingMasterMode;

	/* And finally set the bit on the CR to send a start bit. */
	PRIMARY_I2C->CR1 |= I2C_CR1_START;

	return OK;
}

bool __twb_i2c_startNextJobIfNecessary(void){
	if(I2CQueueJobHead != I2CQueueJobTail){
		__twb_i2c_startNextJob();
		return true;
	}
	else{
		__twbI2CCurrentJob = null;
		return false;
	}
}

void __twb_i2c_completeJob(iOpResult_e result){
	/* We finished, so trigger a stop bit */
    PRIMARY_I2C->CR1 |= I2C_CR1_STOP;

    /* Wait for any communications to complete */
	while(I2C_GetFlagStatus(PRIMARY_I2C, I2C_FLAG_BUSY));

	__twbI2CCurrentJob->Result = result;

	/* If we have any call backs, do them now. */
	if(__twbI2CCurrentJob->Callback != 0x00)
		__twbI2CCurrentJob->Callback(__twbI2CCurrentJob);

	__twbI2CCurrentJob->State = I2CCompleted;

	TWB_Timer_Disable(I2CWatchDog);

	__twb_i2c_startNextJobIfNecessary();
}

void __twb_i2c_switchToReceiveMode(void)
{
	/* Close out the write process, with a stop bit */
	PRIMARY_I2C->CR1 |= I2C_CR1_STOP;

	/* Wait for communications to finish up */
	while(I2C_GetFlagStatus(PRIMARY_I2C, I2C_FLAG_BUSY));

	/* TODO: This next step is probably not necessary if I knew what the hell I2C Clock Stretching is */
	__twb_i2c_pauseBetweenTxAndRx();

	/* At this point the transmit phase is 100% complete but since we have
	 * bytes to be read in switch to the receive mode */
	__twbI2CCurrentJob->Mode = I2CReceive;

	/* Now set the current state */
	__twbI2CCurrentJob->State = PendingMasterMode;

	/* And finally set the bit on the CR to send a start bit. */
	PRIMARY_I2C->CR1 |= I2C_CR1_START;
}

void __twb_i2c_SetAckFlag(void){
	//If only pulling one byte, send a NAK back to the device.
	if(__twbI2CCurrentJob->InBufferSize == 1)
		PRIMARY_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
	else
		PRIMARY_I2C->CR1 |= I2C_CR1_ACK;
}

void __twb_i2c_SendAddress(void){
	volatile uint8_t address = __twbI2CCurrentJob->Address;

	if(__twbI2CCurrentJob->Mode == I2CReceive){
		address <<= 1; /* LSH Address one bit */
		address |= I2C_OAR1_ADD0; //or in a 1 in the final position
	}
	else{
		address <<= 1; /* LSH Address one bit */
		address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD0); /* Ensure write bit is cleared */
	}

	PRIMARY_I2C->DR = address;
}

uint32_t __lastFlag = 0;

void I2C1_EV_IRQHandler(void){
	volatile uint32_t flag1 = 0, flag2 = 0;
	flag1 = PRIMARY_I2C->SR1;
	flag2 = PRIMARY_I2C->SR2;
	flag2 = flag2 << 16;

	TWB_Timer_Reset(I2CWatchDog);

	__lastFlag = (flag1 | flag2);

	switch(__twbI2CCurrentJob->State){
		case Idle:	break;
		case Pending:
			if((__lastFlag & I2C_EVENT_MASTER_MODE_SELECT) == I2C_EVENT_MASTER_MODE_SELECT)
				TWB_Debug_Print("2");
			else
				TWB_Debug_Print("3");

			__twb_i2c_completeJob(I2C_GeneralFailure);

			break;

		case PendingMasterMode:
			if((__lastFlag & I2C_EVENT_MASTER_MODE_SELECT) == I2C_EVENT_MASTER_MODE_SELECT){
				/* Send the address in with the LSB either cleared or set depending on TX or RX */
				__twb_i2c_SendAddress();

				/* Transition to the Pending Address Sent state */
				__twbI2CCurrentJob->State = PendingAddressSent;

				/* If we are in the receive mode, make sure we set/clear the ACK flag */
				if(__twbI2CCurrentJob->Mode == I2CReceive)
					__twb_i2c_SetAckFlag();
			}
			break;
		case PendingAddressSent:
			if((__lastFlag & I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED && __twbI2CCurrentJob->Mode == I2CSend ){
				__twbI2CCurrentJob->State = PendingSending;
				PRIMARY_I2C->DR = __twbI2CCurrentJob->OutBuffer[__twbI2CCurrentJob->BytesSent++];
			}

			if((__lastFlag & I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED && __twbI2CCurrentJob->Mode == I2CReceive)
				__twbI2CCurrentJob->State = PendingReceive;

			break;

		case  PendingReceive:{
			if((__lastFlag & I2C_EVENT_MASTER_BYTE_RECEIVED) == I2C_EVENT_MASTER_BYTE_RECEIVED){
				/* Add the currently read in value to the incoming data buffer */
				__twbI2CCurrentJob->InBuffer[__twbI2CCurrentJob->BytesReceived++] = PRIMARY_I2C->DR;

				/* If we have read in the current number of bytes, we are done */
				if(__twbI2CCurrentJob->BytesReceived == __twbI2CCurrentJob->InBufferSize)
					__twb_i2c_completeJob(OK);
				else if(__twbI2CCurrentJob->BytesReceived == __twbI2CCurrentJob->InBufferSize - 1){
					/* If we have just read the second to last byte, then disable sending the NAK
					 * to let the peripheral know we are done */
					PRIMARY_I2C->CR1 &= (uint16_t)~((uint16_t)I2C_CR1_ACK);
					/* Then just continue on read the final byte in the next IRQ */
				}
			}
		}

		break;

		case  PendingSending:{
			/* If we have sent all the bytes in the out buffer, trigger a stop bit */
			if((__lastFlag & I2C_EVENT_MASTER_BYTE_TRANSMITTED) == I2C_EVENT_MASTER_BYTE_TRANSMITTED){
				if(__twbI2CCurrentJob->BytesSent == __twbI2CCurrentJob->OutBufferSize){
					if(__twbI2CCurrentJob->InBufferSize == 0) /* Write Only */
						__twb_i2c_completeJob(OK);
					else
						__twb_i2c_switchToReceiveMode();
				}
				else /* Otherwise send another event */
					PRIMARY_I2C->DR =  __twbI2CCurrentJob->OutBuffer[__twbI2CCurrentJob->BytesSent++];
			}
		}
		break;
		default: break;
	}
}

void TWB_I2C_WatchdogExpired(void){
	/*
	 * Generate a stop bit just to be safe, but if this happened,
	 * chances are other bad things happened as well.
	 *
	 * But just in case generate a stop bit.
	 *  */
	PRIMARY_I2C->CR1 |= I2C_CR1_STOP;

	__i2cLastError = I2C_WatchdogExpire;

	/* Chances are very good that if this failed */
	if(__twbI2CCurrentJob != null)
		__twbI2CCurrentJob->State = I2CTimeOut;

	/* Disable the watch dog timer */
	TWB_Timer_Disable(I2CWatchDog);
}


void I2C1_ER_IRQHandler(void){
	volatile uint32_t flag1 = 0, flag2 = 0;
	flag1 = PRIMARY_I2C->SR1;
	flag2 = PRIMARY_I2C->SR2;
	flag2 = flag2 << 16;

	volatile uint32_t lastevent = (flag1 | flag2);

	for(uint8_t idx = 0; idx < __twbI2CCurrentJob->OutBufferSize; idx++)
		TWB_Debug_PrintInt("I2C Value-> ", __twbI2CCurrentJob->OutBuffer[idx]);

	if((lastevent & I2C_EVENT_SLAVE_ACK_FAILURE) == I2C_EVENT_SLAVE_ACK_FAILURE){
		TWB_Debug_Print("Y");
		//Clear the slave failure event
		PRIMARY_I2C->SR1 &= ~I2C_EVENT_SLAVE_ACK_FAILURE;

		__twb_i2c_completeJob(I2CAckFailure);
	}
}

iOpResult_e TWB_I2C_Init(I2C_TypeDef *port){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef gpioI2C;
	gpioI2C.GPIO_Pin = GPIO_Pin_8;
	gpioI2C.GPIO_Mode = GPIO_Mode_AF;
	gpioI2C.GPIO_OType = GPIO_OType_OD;
	gpioI2C.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpioI2C.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioI2C);

	gpioI2C.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &gpioI2C);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

	I2C_DeInit(PRIMARY_I2C);

	I2C_InitTypeDef i2cCfg;
	i2cCfg.I2C_Mode = I2C_Mode_I2C;
	i2cCfg.I2C_DutyCycle = I2C_DutyCycle_2;
	i2cCfg.I2C_OwnAddress1 = 0x00;
	i2cCfg.I2C_Ack = I2C_Ack_Enable;
	i2cCfg.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	i2cCfg.I2C_ClockSpeed = I2C_400K;

	I2C_Init(PRIMARY_I2C, &i2cCfg);

	I2C_Cmd(PRIMARY_I2C, ENABLE);

	__i2c_EnableInterrupts(ENABLE);

	I2CQueueJobHead = 0;
	I2CQueueJobTail = 0;

	__twbI2CSyncJob = (I2CJob_t *)pm_malloc(sizeof(I2CJob_t));

	return OK;
}

/*Something is really screwy w/ compiler and optimization */
bool isDone(I2CJob_t* job){
	volatile bool isDone = job->State == I2CCompleted ||
			job->State == I2CAckFailure ||
			job->State == I2CTimeOut;
	return isDone;
}

void __twb_i2c_waitUntilDone(I2CJob_t* job) {
	while(!isDone(job));

	/* Just ensure the watch dog is put back on the leash*/
	TWB_Timer_Disable(I2CWatchDog);
}


iOpResult_e TWB_I2C_QueueAsyncJob(I2CJob_t* job){
	/* Add the job to the I2C Queue, then bump the head of the queue */
	I2CQueue[I2CQueueJobHead++] = job;

	if(I2CQueueJobHead == I2CQueueJobTail)
	{
		TWB_Debug_Print("1");
	}

	/* If we caught the size of the queue, then restart it */
	if(I2CQueueJobHead == I2C_JOB_QUEUE_SIZE)
		I2CQueueJobHead = 0;

	/* If we aren't currently processing a job start it now */
	if(__twbI2CCurrentJob == null || __twbI2CCurrentJob->State == I2CCompleted){
		assert_succeed(__twb_i2c_startNextJob());
	}
	else
		job->State = Pending;

	return OK;
}

/*
 * Method to queue an I2C Transfer and then block until *
 * transfer has completed.
 */
iOpResult_e TWB_I2C_ProcessJobSynchronously(I2CJob_t* job){
	/* 1) Start the job, this may/may not start right away */
	assert_succeed(TWB_I2C_QueueAsyncJob(job));

	/* 2) Wait until we are done. */
	__twb_i2c_waitUntilDone(job);

	return job->Result == OK ? OK : FAIL;
}
