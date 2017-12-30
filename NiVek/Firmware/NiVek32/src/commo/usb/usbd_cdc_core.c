/**
  ******************************************************************************
  * @file    usbd_cdc_core.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                CDC Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class

  *           @note
  *             For the Abstract Control Model, this core allows only transmitting the requests to
  *             lower layer dispatcher (ie. usbd_cdc_vcp.c/.h) which should manage each request and
  *             perform relative actions.
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "twb_debug.h"
#include "memory_mgmt.h"
#include "twb_usb.h"


/*********************************************
   CDC Device library callbacks
 *********************************************/
uint8_t  usbd_cdc_Init        (void  *pdev, uint8_t cfgidx);
uint8_t  usbd_cdc_DeInit      (void  *pdev, uint8_t cfgidx);
uint8_t  usbd_cdc_Setup       (void  *pdev, USB_SETUP_REQ *req);
uint8_t  usbd_cdc_EP0_RxReady  (void *pdev);
uint8_t  usbd_cdc_DataIn      (void *pdev, uint8_t epnum);
uint8_t  usbd_cdc_DataOut     (void *pdev, uint8_t epnum);
uint8_t  usbd_cdc_SOF         (void *pdev);
uint8_t  usbd_cdc_iso_in_incomplete  (void *pdev);
uint8_t  usbd_cdc_iso_out_incomplete  (void *pdev);

/*********************************************
   CDC specific management functions
 *********************************************/
static uint8_t Handle_USBAsynchXfer  (void *pdev, uint8_t epnum);
static uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length);

extern uint8_t USBD_DeviceDesc   [USB_SIZ_DEVICE_DESC];

__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc  [USB_CDC_CONFIG_DESC_SIZE] __ALIGN_END ;
__ALIGN_BEGIN uint8_t usbd_cdc_OtherCfgDesc  [USB_CDC_CONFIG_DESC_SIZE] __ALIGN_END ;
__ALIGN_BEGIN static __IO uint32_t  usbd_cdc_AltSet  __ALIGN_END = 0;

Ring_Buffer_t *USB_Console_Buffer;
Ring_Buffer_t *USB_Telemetry_Buffer;
Ring_Buffer_t *USB_Debug_Buffer;

typedef enum {
	USB_Tx_State_Idle,
	USB_Tx_State_Busy
} USB_Tx_State_e;

USB_Tx_State_e __usb_txState = USB_Tx_State_Idle;

__ALIGN_BEGIN uint8_t CmdBuff[CDC_CMD_PACKET_SZE] __ALIGN_END ;

static uint32_t cdcCmd = 0xFF;
static uint32_t cdcLen = 0;

/* CDC interface class callbacks structure */
USBD_Class_cb_TypeDef  USBD_CDC_cb = 
{
  usbd_cdc_Init,
  usbd_cdc_DeInit,
  usbd_cdc_Setup,
  0x00,                 /* EP0_TxSent, */
  usbd_cdc_EP0_RxReady,
  usbd_cdc_DataIn,
  usbd_cdc_DataOut,
  usbd_cdc_SOF,
  usbd_cdc_iso_in_incomplete,
  usbd_cdc_iso_out_incomplete,
  USBD_cdc_GetCfgDesc,
};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[USB_CDC_CONFIG_DESC_SIZE]  __ALIGN_END =
{
  /*Configuration Descriptor*/
  USB_CONFIG_STRUCT_SIZE,   /* bLength: Configuration Descriptor size */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZE,                /* wTotalLength:no of returned bytes */
  0x00,
  0x04,   /* bNumInterfaces: 3 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
	  /*Command Interface Descriptor */
      USB_INTERFACE_STRUCT_SIZE,   /* bLength: Interface Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
	  /* Interface descriptor type */
	  USB_INT_CMD,   /* bInterfaceNumber: Number of Interface */
	  0x00,   /* bAlternateSetting: Alternate setting */
	  0x01,   /* bNumEndpoints: One endpoints used */
	  0x02,   /* bInterfaceClass: Communication Interface Class */
	  0x02,   /* bInterfaceSubClass: Abstract Control Model */
	  0x01,   /* bInterfaceProtocol: Common AT commands */
	  USBD_IDX_INTERFACE0_STR,   /* iInterface: */

	  /*Header Functional Descriptor*/
	  0x05,   /* bLength: Endpoint Descriptor size */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x00,   /* bDescriptorSubtype: Header Func Desc */
	  0x10,   /* bcdCDC: spec release number */
	  0x01,

	  /*Call Management Functional Descriptor*/
	  0x05,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
	  0x00,   /* bmCapabilities: D0+D1 */
	  0x01,   /* bDataInterface: 1 */

	  /*ACM Functional Descriptor*/
	  0x04,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
	  0x02,   /* bmCapabilities */

	  /*Union Functional Descriptor*/
	  0x05,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x06,   /* bDescriptorSubtype: Union func desc */
	  0x00,   /* bMasterInterface: Communication class interface */
	  0x01,   /* bSlaveInterface0: Data Class Interface */

		  /*Endpoint 2 Descriptor*/
	      USB_END_POINT_STRUCT_SIZE,                           /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
		  EP_CMD,                     /* bEndpointAddress */
		  0x03,                           /* bmAttributes: Interrupt */
		  LOBYTE(CDC_CMD_PACKET_SZE),     /* wMaxPacketSize: */
		  HIBYTE(CDC_CMD_PACKET_SZE),
		  0xFF,                           /* bInterval: */
  
	  /*---------------------------------------------------------------------------*/
  
      /*Telemetry Output Interface*/
      USB_INTERFACE_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
	  USB_INT_TELEMETRY,   /* bInterfaceNumber: Number of Interface */
	  0x00,   /* bAlternateSetting: Alternate setting */
	  0x02,   /* bNumEndpoints: Two endpoints used */
	  0xFF,   /* bInterfaceClass: CDC */
	  0xFF,   /* bInterfaceSubClass: */
	  0xFF,   /* bInterfaceProtocol: */
	  USBD_IDX_INTERFACE1_STR,   /* iInterface: */

		  /*Telemetry Output End Point*/
		  USB_END_POINT_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
		  EP_TELEMETRY_OUT,                  /* bEndpointAddress */
		  USB_OTG_EP_BULK,                              /* bmAttributes: Bulk */
		  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
		  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
		  0x00,                              /* bInterval: ignore for Bulk transfer */

		  /*Telemetry Input End Point*/
		  USB_END_POINT_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
		  EP_TELEMETRY_IRQ_IN,                         /* bEndpointAddress */
		  USB_OTG_EP_INT,                              /* bmAttributes: Bulk */
		  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
		  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
		  0x01,                               /* bInterval: ignore for Bulk transfer */


	  /*Telemetry Output Interface*/
	  USB_INTERFACE_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
	  USB_INT_CONSOLE,   /* bInterfaceNumber: Number of Interface */
	  0x00,   /* bAlternateSetting: Alternate setting */
	  0x02,   /* bNumEndpoints: Three endpoints used */
	  0xFF,   /* bInterfaceClass: CDC */
	  0xFF,   /* bInterfaceSubClass: */
	  0xFF,   /* bInterfaceProtocol: */
	  USBD_IDX_INTERFACE2_STR,   /* iInterface: */

		  /* IRQ Used to ask interface if it has anything to send. */
		  USB_END_POINT_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
		  EP_CONSOLE_IRQ_IN,                  /* bEndpointAddress */
		  USB_OTG_EP_INT,                              /* bmAttributes: IRQ */
		  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
		  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
		  0x10,                              /* bInterval: ignore for Bulk transfer */
		  /* Send Console Commands to Device */
		  USB_END_POINT_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
		  EP_CONSOLE_OUT,                         /* bEndpointAddress */
		  USB_OTG_EP_BULK,                              /* bmAttributes: Bulk */
		  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
		  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
		  0x00,                               /* bInterval: ignore for Bulk transfer */

	  /*Telemetry Output Interface*/
	  USB_INTERFACE_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
	  USB_INT_DEBUG,   /* bInterfaceNumber: Number of Interface */
		  0x00,   /* bAlternateSetting: Alternate setting */
		  0x01,   /* bNumEndpoints: Three endpoints used */
		  0xFF,   /* bInterfaceClass: CDC */
		  0xFF,   /* bInterfaceSubClass: */
		  0xFF,   /* bInterfaceProtocol: */
		  USBD_IDX_INTERFACE3_STR,   /* iInterface: */

		  /* IRQ Used to ask interface if it has anything to send. */
		  USB_END_POINT_STRUCT_SIZE,   /* bLength: Endpoint Descriptor size */
		  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType: Endpoint */
		  EP_DEBUG_IRQ_IN,                  /* bEndpointAddress */
		  USB_OTG_EP_INT,                              /* bmAttributes: IRQ */
		  LOBYTE(CDC_DATA_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
		  HIBYTE(CDC_DATA_MAX_PACKET_SIZE),
		  0x05,
} ;


uint8_t  usbd_cdc_Init (void  *pdev, uint8_t cfgidx){
	DCD_EP_Open(pdev, EP_CMD, CDC_CMD_PACKET_SZE, USB_OTG_EP_INT);

	DCD_EP_Open(pdev, EP_TELEMETRY_IRQ_IN, CDC_DATA_IN_PACKET_SIZE, USB_OTG_EP_INT);
	DCD_EP_Open(pdev, EP_TELEMETRY_OUT, CDC_DATA_OUT_PACKET_SIZE, USB_OTG_EP_BULK);

	DCD_EP_Open(pdev, EP_CONSOLE_IRQ_IN, CDC_DATA_IN_PACKET_SIZE, USB_OTG_EP_INT);
	DCD_EP_Open(pdev, EP_DEBUG_IRQ_IN, CDC_DATA_OUT_PACKET_SIZE, USB_OTG_EP_INT);
	DCD_EP_Open(pdev, EP_CONSOLE_OUT, CDC_DATA_OUT_PACKET_SIZE, USB_OTG_EP_BULK);

	uint8_t *pbuf;
	pbuf = (uint8_t *)USBD_DeviceDesc;
	pbuf[4] = DEVICE_CLASS_CDC;
	pbuf[5] = DEVICE_SUBCLASS_CDC;

	DCD_EP_PrepareRx(pdev, EP_CONSOLE_OUT, (uint8_t*)(USB_Console_Buffer->Buffer), CDC_DATA_OUT_PACKET_SIZE);
	DCD_EP_PrepareRx(pdev, EP_TELEMETRY_OUT, (uint8_t*)(USB_Telemetry_Buffer->Buffer), CDC_DATA_OUT_PACKET_SIZE);

	return USBD_OK;
}

uint8_t  usbd_cdc_DeInit (void  *pdev, uint8_t cfgidx){
	DCD_EP_Close(pdev, EP_CMD);

	DCD_EP_Close(pdev, EP_TELEMETRY_IRQ_IN);
	DCD_EP_Close(pdev, EP_TELEMETRY_OUT);

	DCD_EP_Close(pdev, EP_CONSOLE_IRQ_IN);
	DCD_EP_Close(pdev, EP_DEBUG_IRQ_IN);
	DCD_EP_Close(pdev, EP_CONSOLE_OUT);


	return USBD_OK;
}

uint8_t  usbd_cdc_Setup (void  *pdev, USB_SETUP_REQ *req){
	uint16_t len=USB_CDC_DESC_SIZE;
	uint8_t  *pbuf=usbd_cdc_CfgDesc + 9;

	switch (req->bmRequest & USB_REQ_TYPE_MASK){
		case USB_REQ_TYPE_CLASS :
			if (req->wLength) {
				if (req->bmRequest & 0x80)
					USBD_CtlSendData (pdev, CmdBuff, req->wLength);
				else {
					cdcCmd = req->bRequest;
					cdcLen = req->wLength;
          
				  USBD_CtlPrepareRx (pdev, CmdBuff, req->wLength);
  			  }
  		  }
  		  else{
  			  /* Add code here to handle any control "stuff* */
  		  }
			break;
    
		/* Standard Requests -------------------------------*/
		case USB_REQ_TYPE_STANDARD:
			switch (req->bRequest) {
				case USB_REQ_GET_DESCRIPTOR:{
					if( (req->wValue >> 8) == CDC_DESCRIPTOR_TYPE) {
						pbuf = usbd_cdc_CfgDesc + 9 + (9 * USBD_ITF_MAX_NUM);
						len = MIN(USB_CDC_DESC_SIZE , req->wLength);
					}

					USBD_CtlSendData (pdev, pbuf, len);
				}
				break;

				case USB_REQ_CLEAR_FEATURE: TWB_Debug_Print2Int("CLRFTR", req->wIndex, req->wValue); break;
				case USB_REQ_GET_INTERFACE : USBD_CtlSendData (pdev, (uint8_t *)&usbd_cdc_AltSet, 1); break;
				case USB_REQ_SET_INTERFACE :
					if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
						usbd_cdc_AltSet = (uint8_t)(req->wValue);
					else
						USBD_CtlError (pdev, req);
					break;
				default: USBD_CtlError (pdev, req); break;
			}
			break;

		default: USBD_CtlError (pdev, req); return USBD_FAIL;
 	  }

  	  return USBD_OK;
}

uint8_t  usbd_cdc_EP0_RxReady (void  *pdev) {
  if (cdcCmd != NO_CMD)
    cdcCmd = NO_CMD;
  
  return USBD_OK;
}

uint8_t  usbd_cdc_DataIn (void *pdev, uint8_t epnum) {
  uint16_t txPtr;
  uint16_t txLen;

  Ring_Buffer_t *rxBuffer = null;

  switch(epnum){
  	  case EP_TELEMETRY_IRQ_IN & 0x7F: rxBuffer = USB_Telemetry_Buffer; break;
  	  case EP_CONSOLE_IRQ_IN & 0x7F: rxBuffer = USB_Console_Buffer; break;
  	  case EP_DEBUG_IRQ_IN & 0x7F: rxBuffer = USB_Debug_Buffer; break;
  }

  if(rxBuffer == null){
	  TWB_Debug_Print("NOP");
	  return USBD_OK;
  }

  if (__usb_txState == USB_Tx_State_Busy){
    if (rxBuffer->Len == 0){
//    	TWB_Debug_Print("DONE TX!\r\n");
    	//TODO: Could be sleeping bug for concurrency and End Point transmission
    	__usb_txState = USB_Tx_State_Idle;
    }
    else {
    	TWB_Debug_Print("More ?\r\n");
      if (rxBuffer->Len > CDC_DATA_IN_PACKET_SIZE){
        txPtr =  rxBuffer->PtrOut;
        txLen = CDC_DATA_IN_PACKET_SIZE;
        
        rxBuffer->PtrOut += CDC_DATA_IN_PACKET_SIZE;
        rxBuffer->Len -= CDC_DATA_IN_PACKET_SIZE;
      }
      else {
        txPtr =  rxBuffer->PtrOut;
        txLen = rxBuffer->Len;
        
        rxBuffer->PtrOut += rxBuffer->Len;
        rxBuffer->Len = 0;
      }

      DCD_EP_Tx (pdev, epnum, (uint8_t*)&rxBuffer->Buffer[txPtr], txLen);
    }
  }  
  
  return USBD_OK;
}


uint8_t  usbd_cdc_DataOut (void *pdev, uint8_t epnum){
  uint16_t USB_Rx_Cnt;
  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = ((USB_OTG_CORE_HANDLE*)pdev)->dev.out_ep[epnum].xfer_count;
  
  switch(epnum){
  	  case EP_TELEMETRY_OUT:
  		TWB_USB_Handle_TelemetryIn(USB_Telemetry_Buffer->Buffer, USB_Rx_Cnt);
  		DCD_EP_PrepareRx(pdev, epnum, (uint8_t*)(USB_Telemetry_Buffer->Buffer), CDC_DATA_OUT_PACKET_SIZE);
  		  break;
  	  case EP_CONSOLE_OUT:
  		TWB_USB_Handle_ConsoleIn(USB_Console_Buffer->Buffer, USB_Rx_Cnt);
  		DCD_EP_PrepareRx(pdev, epnum, (uint8_t*)(USB_Console_Buffer->Buffer), CDC_DATA_OUT_PACKET_SIZE);
  		  break;
  }

  return USBD_OK;
}

uint8_t  usbd_cdc_SOF (void *pdev) {
  static uint32_t FrameCount = 0;
  
  if (FrameCount++ == CDC_IN_FRAME_INTERVAL) {
    FrameCount = 0;
    
    if(Handle_USBAsynchXfer(pdev, EP_TELEMETRY_IRQ_IN) == USBD_OK)
    	if(Handle_USBAsynchXfer(pdev, EP_CONSOLE_IRQ_IN) == USBD_OK)
    		Handle_USBAsynchXfer(pdev, EP_DEBUG_IRQ_IN);
  }
  
  return USBD_OK;
}

uint8_t Handle_USBAsynchXfer (void *pdev, uint8_t epaddr) {
	uint16_t txPtr;
	uint16_t txLen;
  
	Ring_Buffer_t *rxBuffer = null;

	switch(epaddr){
	  case EP_TELEMETRY_IRQ_IN: rxBuffer = USB_Telemetry_Buffer; break;
	  case EP_CONSOLE_IRQ_IN: rxBuffer = USB_Console_Buffer; break;
	  case EP_DEBUG_IRQ_IN: rxBuffer = USB_Debug_Buffer; break;
	}

	if(rxBuffer == null)
		return USBD_OK;

	if(__usb_txState == USB_Tx_State_Idle){
		if (rxBuffer->PtrOut == USB_BUFFER_SIZE)
			rxBuffer->PtrOut = 0;

		if(rxBuffer->PtrOut == rxBuffer->PtrIn) {
			__usb_txState = USB_Tx_State_Idle;
			return USBD_OK;
		}
    
		if(rxBuffer->PtrOut > rxBuffer->PtrIn) /* rollback */
			rxBuffer->Len = USB_BUFFER_SIZE - rxBuffer->PtrOut;
		else
			rxBuffer->Len = rxBuffer->PtrIn - rxBuffer->PtrOut;

		if (rxBuffer->Len > CDC_DATA_IN_PACKET_SIZE){
		  txPtr = rxBuffer->PtrOut;
		  txLen = CDC_DATA_IN_PACKET_SIZE;

		  rxBuffer->PtrOut += CDC_DATA_IN_PACKET_SIZE;
		  rxBuffer->Len -= CDC_DATA_IN_PACKET_SIZE;
		}
		else{
		  txPtr = rxBuffer->PtrOut;
		  txLen = rxBuffer->Len;

		  rxBuffer->PtrOut += rxBuffer->Len;
		  rxBuffer->Len = 0;
		}

		__usb_txState = USB_Tx_State_Busy;

		//Call a method to initialize transfer.
		DCD_EP_Tx (pdev, epaddr, (uint8_t*)&rxBuffer->Buffer[txPtr], txLen);

		return USBD_BUSY;
	}

	return USBD_OK;
}

uint8_t  *USBD_cdc_GetCfgDesc (uint8_t speed, uint16_t *length){
  *length = sizeof (usbd_cdc_CfgDesc);
  return usbd_cdc_CfgDesc;
}

uint8_t  usbd_cdc_iso_in_incomplete  (void *pdev) {return USBD_OK;}
uint8_t  usbd_cdc_iso_out_incomplete  (void *pdev){return USBD_OK;}
