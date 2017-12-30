/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   This file includes the user application layer
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

#include "commo/usb/usbd_usr.h"
#include "commo/usb/usbd_ioreq.h"

USBD_Usr_cb_TypeDef USR_cb = {
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,
};


void USBD_USR_Init(void){}

void USBD_USR_DeviceReset(uint8_t speed ){
	switch (speed){
		case USB_OTG_SPEED_HIGH: break;
		case USB_OTG_SPEED_FULL: break;
		default: break;
	}
}

void USBD_USR_DeviceConfigured(void){
	TWB_Debug_Print("USB Device Configured\r\n");
}

void USBD_USR_DeviceConnected(void) {
	TWB_Debug_Print("USB Device Connected\r\n");
}

void USBD_USR_DeviceDisconnected(void) {
	TWB_Debug_Print("USB Device Disconnected\r\n");
}

void USBD_USR_DeviceSuspended(void) {
	TWB_Debug_Print("USB Device Suspended\r\n");
}

void USBD_USR_DeviceResumed(void) {
	TWB_Debug_Print("USB Device Resumed\r\n");
}


