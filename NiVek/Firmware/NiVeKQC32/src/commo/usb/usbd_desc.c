/**
  ******************************************************************************
  * @file    usbd_desc.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   This file provides the USBD descriptors and string formating method.
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
#include "commo/usb/usbd_core.h"
#include "commo/usb/usbd_desc.h"
#include "commo/usb/usbd_req.h"
#include "commo/usb/usbd_conf.h"
#include "commo/usb/usb_regs.h"
#include "common/twb_debug.h"

/*Valid ones
#define USBD_VID                        0x0483
#define USBD_PID                        0x5740
*/

#define USBD_VID                        0x1483
#define USBD_PID                        0x681a

//
//#define USBD_PID                        0x5756
//
//#define USBD_PID                        0x5724



/** @defgroup USB_String_Descriptors
  * @{
  */ 
#define USBD_LANGID_STRING              0x409
#define USBD_MANUFACTURER_STRING        "Software Logistics, LLC"

#define USBD_PRODUCT_FS_STRING          "NiVek"
#define USBD_SERIALNUMBER_FS_STRING     "00000000050C"

#define USBD_CONFIGURATION_FS_STRING    "VCP Config"

#define USBD_INTERFACE0_Name        "NiVek Control"
#define USBD_INTERFACE1_Name        "NiVek Telemetry"
#define USBD_INTERFACE2_Name        "NiVek Console"
#define USBD_INTERFACE3_Name        "NiVek Debug"
#define USBD_INTERFACE4_Name        "NiVek DFU"


#define OS_STRING 'M',0,'S',0,'F',0,'T',0,'1',0,'0',0,'0',0, 4, 0

USBD_DEVICE USR_desc =
{
  USBD_USR_DeviceDescriptor,
  USBD_USR_LangIDStrDescriptor, 
  USBD_USR_ManufacturerStrDescriptor,
  USBD_USR_ProductStrDescriptor,
  USBD_USR_SerialStrDescriptor,
  USBD_USR_ConfigStrDescriptor,
  USBD_USR_InterfaceStrDescriptor,
  USBD_USR_OSStrDescriptor,
  USBD_USR_ExtCompatIDFeatureDescriptor,
  USBD_USR_ExtPropertiesFeatureDescriptor,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END =
  {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,0x02,                  /*bcdUSB */
    0x00,                       /*bDeviceClass Populated in cdc core*/
    0x00,                       /*bDeviceSubClass  Populated in cdc core*/
    0x00,                       /*bDeviceProtocol*/
    USB_OTG_MAX_EP0_SIZE,      /*bMaxPacketSize*/
    LOBYTE(USBD_VID),           /*idVendor*/
    HIBYTE(USBD_VID),           /*idVendor*/
    LOBYTE(USBD_PID),           /*idVendor*/
    HIBYTE(USBD_PID),           /*idVendor*/
    0x00,0x02,                  /*bcdDevice rel. 2.00*/
    USBD_IDX_MFC_STR,           /*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR,       /*Index of product string*/
    USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
    USBD_CFG_MAX_NUM            /*bNumConfigurations*/
  } ; /* USB_DeviceDescriptor */


/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END =
{
     USB_SIZ_STRING_LANGID,         
     USB_DESC_TYPE_STRING,       
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING), 
};
// Extended Compat ID OS Feature Descriptor
__ALIGN_BEGIN uint8_t USBD_OSStrDesc[USB_LEN_OS_DESC] __ALIGN_END =
{
USB_LEN_OS_DESC,
USB_DESC_TYPE_STRING,
OS_STRING,
};

typedef struct
{
// Header
uint32_t dwLength;
uint16_t  bcdVersion;
uint16_t  wIndex;
uint8_t  bCount;
uint8_t  bReserved0[7];
// Function Section 1
uint8_t  bInterfaceNumber1;
uint8_t  bReserved1;
uint8_t  bCompatibleID1[8];
uint8_t  bSubCompatibleID1[8];
uint8_t  bReserved31[6];

// Function Section 2
uint8_t  bInterfaceNumber2;
uint8_t  bReserved2;
uint8_t  bCompatibleID2[8];
uint8_t  bSubCompatibleID2[8];
uint8_t  bReserved32[6];

// Function Section 3
uint8_t  bSecondInterfaceNumber3;
uint8_t  bReserved3;
uint8_t  bCompatibleID3[8];
uint8_t  bSubCompatibleID3[8];
uint8_t  bReserved33[6];

// Function Section 4
uint8_t  bSecondInterfaceNumber4;
uint8_t  bReserved4;
uint8_t  bCompatibleID4[8];
uint8_t  bSubCompatibleID4[8];
uint8_t  bReserved34[6];


} USBD_CompatIDDescStruct;


__ALIGN_BEGIN USBD_CompatIDDescStruct USBD_CompatIDDesc __ALIGN_END = {
		sizeof(USBD_CompatIDDesc),
		0x0100,
		0x0004, /* Reserved */
		0x04, /* Number of Interfaces */
		{0},
		/* Start Interface 2 */
		0x00, 0x01, "WINUSB", {0}, {0},
		0x01, 0x01, "WINUSB", {0}, {0},
		0x02, 0x01, "WINUSB", {0}, {0},
		0x03, 0x01, "WINUSB", {0}, {0}
};

// Extended Properties OS Feature Descriptor
__ALIGN_BEGIN
typedef struct
{
// Header
uint32_t dwLength;
uint16_t  bcdVersion;
uint16_t  wIndex;
uint16_t  wCount;

// Custom Property Section 1
uint32_t dwSize;
uint32_t dwPropertyDataType;
uint16_t  wPropertyNameLength;
uint16_t  bPropertyName[20];
uint32_t dwPropertyDataLength;
uint16_t  bPropertyData[39]; /* Kind of cool, send with uint16_t will send as two bytes so it looks like unicode! :) */

uint32_t dwLabelSize;                  /* 4   4*/
uint32_t dwLabelPropertyDataType;      /* 4   8 */
uint16_t dwLabelPropertyNameLength;    /* 2  10*/
uint16_t dwLabelPropertyName[6];       /* 12 22 */
uint32_t  bLabelPropertyDateLength;    /* 4  26*/
uint16_t  bLabelPropertyData[6];       /* 12 38*/
} __attribute__ ((packed)) USBD_ExtPropertiesDescStruct;
__ALIGN_END


__ALIGN_BEGIN USBD_ExtPropertiesDescStruct USBD_ExtPropertiesDesc0 __ALIGN_END = {
		sizeof(USBD_ExtPropertiesDesc0),
		0x0100, /* Version 1.0 of OS descriptors */
		0x0005, /* Index for extended descriptors */
		0x0002, /* Number of extended descriptors */

		/* Start Extended Descriptor for Device Guid */
		0x00000084, /* Size */
		0x00000001, /* Data Type 1 = Unicode String */
		0x0028,     /* Property Name Length */
		{'D','e','v','i','c','e','I','n','t','e','r','f','a','c','e','G','U','I','D',0},
		0x0000004E,  /* Property Contents Length */
		{'{','F','7','0','2','4','2','C','7','-','F','B','2','5','-','4','4','3','B', '-','9','E','7','E','-','A','4','2','6','0','F','3','7','3','9','8','0','}',0},

		/* Start Extended Descriptor for Label */
		38, /* Size */
		1, /* Data Type 1 = Unicode String */
		0x000C,
		{'L','a','b','e','l',0},
		0x0000000C,
		{'N','i','V','e','k',0},
};


__ALIGN_BEGIN USBD_ExtPropertiesDescStruct USBD_ExtPropertiesDesc1  __ALIGN_END = {
		sizeof(USBD_ExtPropertiesDesc0),
		0x0100, /* Version 1.0 of OS descriptors */
		0x0005, /* Index for extended descriptors */
		0x0002, /* Number of extended descriptors */

		/* Start Extended Descriptor Detail */
		0x00000084, /* Size */
		0x00000001, /* Data Type 1 = Unicode String */
		0x0028,     /* Property Name Length */
		{'D','e','v','i','c','e','I','n','t','e','r','f','a','c','e','G','U','I','D',0},
		0x0000004E,  /* Property Contents Length */
		{'{','F','7','0','2','4','2','C','7','-','F','B','2','5','-','4','4','3','B', '-','9','E','7','E','-','A','4','2','6','0','F','3','7','3','9','8','1','}',0},

		/* Start Extended Descriptor for Label */
		38, /* Size */
		1, /* Data Type 1 = Unicode String */
		0x000C,
		{'L','a','b','e','l',0},
		0x0000000C,
		{'N','i','V','e','k',0},
};

__ALIGN_BEGIN USBD_ExtPropertiesDescStruct USBD_ExtPropertiesDesc2  __ALIGN_END = {
		sizeof(USBD_ExtPropertiesDesc0),
		0x0100, /* Version 1.0 of OS descriptors */
		0x0005, /* Index for extended descriptors */
		0x0002, /* Number of extended descriptors */

		/* Start Extended Descriptor Detail */
		0x00000084, /* Size */
		0x00000001, /* Data Type 1 = Unicode String */
		0x0028,     /* Property Name Length */
		{'D','e','v','i','c','e','I','n','t','e','r','f','a','c','e','G','U','I','D',0},
		0x0000004E,  /* Property Contents Length */
		{'{','F','7','0','2','4','2','C','7','-','F','B','2','5','-','4','4','3','B', '-','9','E','7','E','-','A','4','2','6','0','F','3','7','3','9','8','2','}',0},

		/* Start Extended Descriptor for Label */
		38, /* Size */
		1, /* Data Type 1 = Unicode String */
		0x000C,
		{'L','a','b','e','l',0},
		0x0000000C,
		{'N','i','V','e','k',0},
};

__ALIGN_BEGIN USBD_ExtPropertiesDescStruct USBD_ExtPropertiesDesc3  __ALIGN_END = {
		sizeof(USBD_ExtPropertiesDesc0),
		0x0100, /* Version 1.0 of OS descriptors */
		0x0005, /* Index for extended descriptors */
		0x0002, /* Number of extended descriptors */

		/* Start Extended Descriptor Detail */
		0x00000084, /* Size */
		0x00000001, /* Data Type 1 = Unicode String */
		0x0028,     /* Property Name Length */
		{'D','e','v','i','c','e','I','n','t','e','r','f','a','c','e','G','U','I','D',0},
		0x0000004E,  /* Property Contents Length */
		{'{','F','7','0','2','4','2','C','7','-','F','B','2','5','-','4','4','3','B', '-','9','E','7','E','-','A','4','2','6','0','F','3','7','3','9','8','3','}',0},

		/* Start Extended Descriptor for Label */
		38, /* Size */
		1, /* Data Type 1 = Unicode String */
		0x000C,
		{'L','a','b','e','l',0},
		0x0000000C,
		{'N','i','V','e','k',0},
};


/**
* @brief  USBD_USR_DeviceDescriptor 
*         return the device descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *  USBD_USR_DeviceDescriptor( uint8_t speed , uint16_t *length)
{
  *length = sizeof(USBD_DeviceDesc);
  return USBD_DeviceDesc;
}

/**
* @brief  USBD_USR_LangIDStrDescriptor 
*         return the LangID string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *  USBD_USR_LangIDStrDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_LangIDDesc);  
  return USBD_LangIDDesc;
}

uint8_t *  USBD_USR_ProductStrDescriptor( uint8_t speed , uint16_t *length){
	USBD_GetString ((uint8_t*)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);

	return USBD_StrDesc;
}

uint8_t *  USBD_USR_ManufacturerStrDescriptor( uint8_t speed , uint16_t *length){
  USBD_GetString ((uint8_t*)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

uint8_t *  USBD_USR_SerialStrDescriptor( uint8_t speed , uint16_t *length){
  USBD_GetString ((uint8_t*)USBD_SERIALNUMBER_FS_STRING, USBD_StrDesc, length);

  return USBD_StrDesc;
}

uint8_t *  USBD_USR_ConfigStrDescriptor( uint8_t speed , uint16_t *length){
	USBD_GetString ((uint8_t*)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, length);

	return USBD_StrDesc;
}


uint8_t *  USBD_USR_InterfaceStrDescriptor( uint8_t speed , uint16_t *length, uint8_t intIdx){
	switch(intIdx){
		case USBD_IDX_INTERFACE0_STR: USBD_GetString ((uint8_t*)USBD_INTERFACE0_Name, USBD_StrDesc, length); break;
		case USBD_IDX_INTERFACE1_STR: USBD_GetString ((uint8_t*)USBD_INTERFACE1_Name, USBD_StrDesc, length); break;
		case USBD_IDX_INTERFACE2_STR: USBD_GetString ((uint8_t*)USBD_INTERFACE2_Name, USBD_StrDesc, length); break;
		case USBD_IDX_INTERFACE3_STR: USBD_GetString ((uint8_t*)USBD_INTERFACE3_Name, USBD_StrDesc, length); break;
		case USBD_IDX_INTERFACE4_STR: USBD_GetString ((uint8_t*)USBD_INTERFACE4_Name, USBD_StrDesc, length); break;
	}

  return USBD_StrDesc;  
}

uint8_t *  USBD_USR_OSStrDescriptor( uint8_t speed , uint16_t *length){
	TWB_Debug_Print(".-OSD");
  *length =  sizeof(USBD_OSStrDesc);
  return USBD_OSStrDesc;
}

uint8_t *  USBD_USR_ExtPropertiesFeatureDescriptor( uint8_t speed , uint16_t *length, uint8_t intIdx)
{
	switch(intIdx){
		case 0:
			TWB_Debug_Print(".-EXT0");
			*length =  sizeof(USBD_ExtPropertiesDesc0);
			return (uint8_t*)&USBD_ExtPropertiesDesc0;
		break;
		case 1:
			TWB_Debug_Print(".-EXT1");
			*length =  sizeof(USBD_ExtPropertiesDesc1);
			return (uint8_t*)&USBD_ExtPropertiesDesc1;
			break;
		case 2:
			TWB_Debug_Print(".-EXT2");
			*length =  sizeof(USBD_ExtPropertiesDesc2);
			return (uint8_t*)&USBD_ExtPropertiesDesc2;
			break;
		case 3:
			TWB_Debug_Print(".-EXT3");
			*length =  sizeof(USBD_ExtPropertiesDesc3);
			return (uint8_t*)&USBD_ExtPropertiesDesc3;
			break;

		default:
			TWB_Debug_Print(".-EXT?");
			break;
	}

	return 0;
}

uint8_t *  USBD_USR_ExtCompatIDFeatureDescriptor( uint8_t speed , uint16_t *length){
	TWB_Debug_Print(".-CMP");
  *length =  sizeof(USBD_CompatIDDesc);
  return (uint8_t*)&USBD_CompatIDDesc;
}

