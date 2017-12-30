/* File manages requests coming from USB Host for information */


#include "commo/usb/usbd_req.h"
#include "commo/usb/usbd_ioreq.h"
#include "commo/usb/usbd_desc.h"
#include "commo/usb/usbd_cdc_core.h"
#include "common/twb_debug.h"

__ALIGN_BEGIN uint32_t USBD_ep_status __ALIGN_END  = 0; 
__ALIGN_BEGIN uint32_t USBD_default_cfg __ALIGN_END  = 0;
__ALIGN_BEGIN uint32_t USBD_cfg_status __ALIGN_END  = 0;
__ALIGN_BEGIN uint8_t  USBD_StrDesc[USB_MAX_STR_DESC_SIZ] __ALIGN_END ;

static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetAddress(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_GetStatus(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static void USBD_SetFeature(USB_OTG_CORE_HANDLE  *pdev,  USB_SETUP_REQ *req);
static void USBD_ClrFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req);
static uint8_t USBD_GetLen(uint8_t *buf);


USBD_Status  USBD_StdDevReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req){
  switch (req->bRequest) {
  	  case USB_REQ_GET_DESCRIPTOR: USBD_GetDescriptor (pdev, req); break;
  	  case USB_REQ_SET_ADDRESS: USBD_SetAddress(pdev, req); break;
  	  case USB_REQ_SET_CONFIGURATION: USBD_SetConfig (pdev , req); break;
  	  case USB_REQ_GET_CONFIGURATION: USBD_GetConfig (pdev , req); break;
  	  case USB_REQ_GET_STATUS: USBD_GetStatus (pdev , req); break;
  	  case USB_REQ_SET_FEATURE: USBD_SetFeature (pdev , req); break;
  	  case USB_REQ_CLEAR_FEATURE: USBD_ClrFeature (pdev , req); break;
  	  default: USBD_CtlError(pdev , req); break;
  }
  
  return USBD_OK;
}

USBD_Status  USBD_VendDevReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req){
	uint16_t len = 0;
	uint8_t *pbuf = 0;

	switch (req->wIndex){
   		case USB_DESC_TYPE_OS_FEATURE_EXT_PROPERTIES: pbuf = pdev->dev.usr_device->GetExtPropertiesFeatureDescriptor(pdev->cfg.speed, &len, req->wValue); break;
   		case USB_DESC_TYPE_OS_FEATURE_EXT_COMPAT_ID: pbuf = pdev->dev.usr_device->GetExtCompatIDFeatureDescriptor(pdev->cfg.speed, &len); break;
	}

	if((len != 0)&& (req->wLength != 0)) {
		len = MIN(len , req->wLength);
		USBD_CtlSendData (pdev, pbuf, len);
	}

	return USBD_OK;
}


/* Handle request for interface information */
USBD_Status  USBD_StdItfReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req){
	switch (pdev->dev.device_status) {
  	  case USB_OTG_CONFIGURED:
  		  if (LOBYTE(req->wIndex) <= USBD_ITF_MAX_NUM){
  			  pdev->dev.class_cb->Setup (pdev, req);
      
  			  if((req->wLength == 0))
  				  USBD_CtlSendStatus(pdev);
  		  }
  		  else
  			  USBD_CtlError(pdev , req);
  		  break;
  	  default: USBD_CtlError(pdev , req); break;
	}

	return USBD_OK;
}


void __usb_ep_setFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req, uint8_t ep_addr){
	switch (pdev->dev.device_status) {
		case USB_OTG_ADDRESSED:
			if ((ep_addr != 0x00) && (ep_addr != 0x80))
				DCD_EP_Stall(pdev , ep_addr);
			break;

		case USB_OTG_CONFIGURED:
			if (req->wValue == USB_FEATURE_EP_HALT) {
				if ((ep_addr != 0x00) && (ep_addr != 0x80))
					DCD_EP_Stall(pdev , ep_addr);
			}

			pdev->dev.class_cb->Setup (pdev, req);
			USBD_CtlSendStatus(pdev);
			break;

		default: USBD_CtlError(pdev , req); break;
	}

}

void __usp_ep_clearFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req, uint8_t ep_addr){
	switch (pdev->dev.device_status) {
		case USB_OTG_ADDRESSED:
			if ((ep_addr != 0x00) && (ep_addr != 0x80))
				DCD_EP_Stall(pdev , ep_addr);
			break;

		case USB_OTG_CONFIGURED:
			if (req->wValue == USB_FEATURE_EP_HALT){
				if ((ep_addr != 0x00) && (ep_addr != 0x80)){
					DCD_EP_ClrStall(pdev , ep_addr);
					pdev->dev.class_cb->Setup (pdev, req);
				}

				USBD_CtlSendStatus(pdev);
			}
			break;

		default: USBD_CtlError(pdev , req); break;
	}
}

void __usp_ep_getStatus(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req, uint8_t ep_addr){
	switch (pdev->dev.device_status){
		case USB_OTG_ADDRESSED:
			if ((ep_addr != 0x00) && (ep_addr != 0x80))
				DCD_EP_Stall(pdev , ep_addr);
			break;

		case USB_OTG_CONFIGURED:
			if ((ep_addr & 0x80)== 0x80){
				if(pdev->dev.in_ep[ep_addr & 0x7F].is_stall)
					  USBD_ep_status = 0x0001;
				else
					USBD_ep_status = 0x0000;
			}
			else if ((ep_addr & 0x80)== 0x00){
				if(pdev->dev.out_ep[ep_addr].is_stall)
					USBD_ep_status = 0x0001;
				else
					USBD_ep_status = 0x0000;
			}

			USBD_CtlSendData (pdev, (uint8_t *)&USBD_ep_status, 2);
			break;

		default: USBD_CtlError(pdev , req); break;
	}
}

/* Handle request for end point information */
USBD_Status  USBD_StdEPReq (USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req){
	uint8_t ep_addr  = LOBYTE(req->wIndex);
  
	switch (req->bRequest){
		case USB_REQ_SET_FEATURE : __usb_ep_setFeature(pdev, req, ep_addr); break;
		case USB_REQ_CLEAR_FEATURE : __usp_ep_clearFeature(pdev, req, ep_addr); break;
		case USB_REQ_GET_STATUS: __usp_ep_getStatus(pdev, req, ep_addr); break;
		default: /* NOP */  break;
	}

	return USBD_OK;
}


static void USBD_GetDescriptor(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	uint16_t len;
	uint8_t *pbuf;
	switch (req->wValue >> 8) {
		case USB_DESC_TYPE_DEVICE:
			pbuf = pdev->dev.usr_device->GetDeviceDescriptor(pdev->cfg.speed, &len);
			if ((req->wLength == 64) ||( pdev->dev.device_status == USB_OTG_DEFAULT))
				len = 8;

			break;
    
		case USB_DESC_TYPE_CONFIGURATION:

			pbuf   = (uint8_t *)pdev->dev.class_cb->GetConfigDescriptor(pdev->cfg.speed, &len);

			pbuf[1] = USB_DESC_TYPE_CONFIGURATION;
			pdev->dev.pConfig_descriptor = pbuf;
			break;
    
		case USB_DESC_TYPE_STRING:
			switch ((uint8_t)(req->wValue)){
				case USBD_IDX_LANGID_STR: pbuf = pdev->dev.usr_device->GetLangIDStrDescriptor(pdev->cfg.speed, &len); break;
				case USBD_IDX_MFC_STR: pbuf = pdev->dev.usr_device->GetManufacturerStrDescriptor(pdev->cfg.speed, &len); break;
				case USBD_IDX_PRODUCT_STR: pbuf = pdev->dev.usr_device->GetProductStrDescriptor(pdev->cfg.speed, &len); break;
				case USBD_IDX_SERIAL_STR: pbuf = pdev->dev.usr_device->GetSerialStrDescriptor(pdev->cfg.speed, &len); break;
				case USBD_IDX_CONFIG_STR: pbuf = pdev->dev.usr_device->GetConfigurationStrDescriptor(pdev->cfg.speed, &len); break;
				case USBD_IDX_INTERFACE0_STR:
				case USBD_IDX_INTERFACE1_STR:
				case USBD_IDX_INTERFACE2_STR:
				case USBD_IDX_INTERFACE3_STR:
				case USBD_IDX_INTERFACE4_STR:
					pbuf = pdev->dev.usr_device->GetInterfaceStrDescriptor(pdev->cfg.speed, &len, (uint8_t)(req->wValue));
					break;
				case USBD_IDX_OS_STR:pbuf = pdev->dev.usr_device->GetOSStrDescriptor(pdev->cfg.speed, &len); break;
				default: USBD_CtlError(pdev , req); return;
			}
			break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER:
			pbuf   = (uint8_t *)pdev->dev.class_cb->GetConfigDescriptor(pdev->cfg.speed, &len);
			USBD_DeviceQualifierDesc[4]= pbuf[14];
			USBD_DeviceQualifierDesc[5]= pbuf[15];
			USBD_DeviceQualifierDesc[6]= pbuf[16];

			pbuf = USBD_DeviceQualifierDesc;
			len  = USB_LEN_DEV_QUALIFIER_DESC;
			break;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			USBD_CtlError(pdev , req);
			return;
    
		default: USBD_CtlError(pdev , req); return;
	}
  
	if((len != 0)&& (req->wLength != 0)){
		len = MIN(len , req->wLength);
    
		USBD_CtlSendData (pdev, pbuf, len);
	}
}

static void USBD_SetAddress(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req) {
	uint8_t  dev_addr;

	if ((req->wIndex == 0) && (req->wLength == 0)) {
		dev_addr = (uint8_t)(req->wValue) & 0x7F;
    
		if (pdev->dev.device_status == USB_OTG_CONFIGURED)
			USBD_CtlError(pdev , req);
		else {
			pdev->dev.device_address = dev_addr;
			DCD_EP_SetAddress(pdev, dev_addr);
			USBD_CtlSendStatus(pdev);

			if (dev_addr != 0)
				pdev->dev.device_status  = USB_OTG_ADDRESSED;
			else
				pdev->dev.device_status  = USB_OTG_DEFAULT;
		}
  } 
  else 
     USBD_CtlError(pdev , req);                        
}

/**
* @brief  USBD_SetConfig
*         Handle Set device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_SetConfig(USB_OTG_CORE_HANDLE  *pdev, 
                           USB_SETUP_REQ *req)
{
  
  static uint8_t  cfgidx;
  
  cfgidx = (uint8_t)(req->wValue);                 
  
  if (cfgidx > USBD_CFG_MAX_NUM ) 
  {            
     USBD_CtlError(pdev , req);                              
  } 
  else 
  {
    switch (pdev->dev.device_status) 
    {
    case USB_OTG_ADDRESSED:
      if (cfgidx) 
      {                                			   							   							   				
        pdev->dev.device_config = cfgidx;
        pdev->dev.device_status = USB_OTG_CONFIGURED;
        USBD_SetCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else 
      {
         USBD_CtlSendStatus(pdev);
      }
      break;
      
    case USB_OTG_CONFIGURED:
      if (cfgidx == 0) 
      {                           
        pdev->dev.device_status = USB_OTG_ADDRESSED;
        pdev->dev.device_config = cfgidx;          
        USBD_ClrCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);
        
      } 
      else  if (cfgidx != pdev->dev.device_config) 
      {
        /* Clear old configuration */
        USBD_ClrCfg(pdev , pdev->dev.device_config);
        
        /* set new configuration */
        pdev->dev.device_config = cfgidx;
        USBD_SetCfg(pdev , cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else
      {
        USBD_CtlSendStatus(pdev);
      }
      break;
      
    default:					
       USBD_CtlError(pdev , req);                     
      break;
    }
  }
}

/**
* @brief  USBD_GetConfig
*         Handle Get device configuration request
* @param  pdev: device instance
* @param  req: usb request
* @retval status
*/
static void USBD_GetConfig(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	TWB_Debug_Print(".getcfg");
  if (req->wLength != 1)
     USBD_CtlError(pdev , req);
  else{
    switch (pdev->dev.device_status )  {
    	case USB_OTG_ADDRESSED: TWB_Debug_Print(".cfg_add"); USBD_CtlSendData (pdev,  (uint8_t *)&USBD_default_cfg, 1); break;
    	case USB_OTG_CONFIGURED: TWB_Debug_Print(".cfg_cfg"); USBD_CtlSendData (pdev, &pdev->dev.device_config, 1); break;
    	default: USBD_CtlError(pdev , req); break;
    }
  }
}

static void USBD_GetStatus(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	switch (pdev->dev.device_status){
		case USB_OTG_ADDRESSED:
		case USB_OTG_CONFIGURED:
			if (pdev->dev.DevRemoteWakeup)
				USBD_cfg_status = USB_CONFIG_SELF_POWERED | USB_CONFIG_REMOTE_WAKEUP;
			else
				USBD_cfg_status = USB_CONFIG_SELF_POWERED;
    
			USBD_CtlSendData (pdev, (uint8_t *)&USBD_cfg_status, 1);
			break;
    
		default : USBD_CtlError(pdev , req); break;
	}
}

static void USBD_SetFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	USB_OTG_DCTL_TypeDef     dctl;
	uint8_t test_mode = 0;
 
	if (req->wValue == USB_FEATURE_REMOTE_WAKEUP){
		pdev->dev.DevRemoteWakeup = 1;
		pdev->dev.class_cb->Setup (pdev, req);
		USBD_CtlSendStatus(pdev);
	}
	else if ((req->wValue == USB_FEATURE_TEST_MODE) && ((req->wIndex & 0xFF) == 0)){
		dctl.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DCTL);
    
		test_mode = req->wIndex >> 8;
		switch (test_mode) {
			case 1:  dctl.b.tstctl = 1; break; // TEST_J
			case 2:  dctl.b.tstctl = 2; break; // TEST_K
			case 3:  dctl.b.tstctl = 3; break; // TEST_SE0_NAK
			case 4:  dctl.b.tstctl = 4; break; // TEST_PACKET
			case 5:  dctl.b.tstctl = 5; break; // TEST_FORCE_ENABLE
		}

		USB_OTG_WRITE_REG32(&pdev->regs.DREGS->DCTL, dctl.d32);
		USBD_CtlSendStatus(pdev);
	}
}

static void USBD_ClrFeature(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	switch (pdev->dev.device_status){
		case USB_OTG_ADDRESSED:
		case USB_OTG_CONFIGURED:
			if (req->wValue == USB_FEATURE_REMOTE_WAKEUP){
				pdev->dev.DevRemoteWakeup = 0;
				pdev->dev.class_cb->Setup (pdev, req);
				USBD_CtlSendStatus(pdev);
			}
			break;
		default : USBD_CtlError(pdev , req); break;
  }
}

void USBD_ParseSetupRequest( USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	req->bmRequest     = *(uint8_t *)  (pdev->dev.setup_packet);
	req->bRequest      = *(uint8_t *)  (pdev->dev.setup_packet +  1);
	req->wValue        = SWAPBYTE      (pdev->dev.setup_packet +  2);
	req->wIndex        = SWAPBYTE      (pdev->dev.setup_packet +  4);
	req->wLength       = SWAPBYTE      (pdev->dev.setup_packet +  6);

	pdev->dev.in_ep[0].ctl_data_len = req->wLength  ;
	pdev->dev.device_state = USB_OTG_EP0_SETUP;
}

void USBD_CtlError( USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ *req){
	if((req->bmRequest & 0x80) == 0x80)
		DCD_EP_Stall(pdev , 0x80);
	else{
		if(req->wLength == 0)
			DCD_EP_Stall(pdev , 0x80);
		else
			DCD_EP_Stall(pdev , 0);
	}
  
	USB_OTG_EP0_OutStart(pdev);
}


void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len){
	uint8_t idx = 0;
  
	if (desc != (void *)0){
		*len =  USBD_GetLen(desc) * 2 + 2;
		unicode[idx++] = *len;
		unicode[idx++] =  USB_DESC_TYPE_STRING;
    
		while (*desc != 0x00){
			unicode[idx++] = *desc++;
			unicode[idx++] =  0x00;
		}
	}
}

static uint8_t USBD_GetLen(uint8_t *buf){
    uint8_t  len = 0;

    while (*buf != 0x00){
        len++;
        buf++;
    }

    return len;
}
