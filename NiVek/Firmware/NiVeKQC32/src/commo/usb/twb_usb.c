/*
 * twb_usb.c
 *
 *  Created on: May 25, 2013
 *      Author: Kevin
 */

#include "common/twb_debug.h"
#include "commo/usb/twb_usb.h"
#include "commo/usb/usbd_core.h"
#include "commo/usb/usbd_cdc_core.h"
#include "commo/usb/usbd_usr.h"
#include "commo/usb/usbd_desc.h"
#include "commo/usb/usb_dcd_int.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

Ring_Buffer_t * __usbd_cdc_createBuffer(uint16_t bufferSize){
	Ring_Buffer_t *buffer = (Ring_Buffer_t *)pm_malloc(sizeof(Ring_Buffer_t));
	buffer->Buffer = (uint8_t *)pm_malloc(bufferSize);
	buffer->PtrIn = 0;
	buffer->PtrOut = 0;
	buffer->Len = 0;
	buffer->Size = bufferSize;

	return buffer;
}


iOpResult_e TWB_USB_Init(void){
	USB_Console_Buffer = __usbd_cdc_createBuffer(USB_BUFFER_SIZE);
	USB_Telemetry_Buffer = __usbd_cdc_createBuffer(USB_BUFFER_SIZE);
	USB_Debug_Buffer = __usbd_cdc_createBuffer(USB_BUFFER_SIZE);


	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);

	return OK;
}

void OTG_FS_IRQHandler(void){
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
}


iOpResult_e TWB_USB_Handle_ConsoleIn(uint8_t *in, uint16_t len){
	TWB_Debug_Print("CONSOLEIN->");

	for(uint16_t idx = 0; idx < len; ++idx){
		TWB_Debug_AddCh(in[idx]);
		TWB_CLI_ParseCH(in[idx]);
	}

	TWB_Debug_Print("<-\r\n");
	return OK;
}

iOpResult_e TWB_USB_Handle_TelemetryIn(uint8_t *in, uint16_t len){
	TWB_Debug_PrintInt("TELEM IN-> ", len);

	for(uint16_t idx = 0; idx < len; ++idx){
		TWB_Debug_AddCh(in[idx]);
	}

	TWB_Debug_Print("\r\n");
	return OK;
}

iOpResult_e TWB_USB_Write_Console_Out(uint8_t *out, uint16_t len){

	return OK;
}

iOpResult_e TWB_USB_Write_Debug_Out(uint8_t *out, uint16_t len){

	return OK;
}

iOpResult_e TWB_USB_Write_Telemetry_Out(uint8_t *out, uint16_t len){

	return OK;
}


