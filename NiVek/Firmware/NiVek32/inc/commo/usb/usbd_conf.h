#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#include "stm32f4xx.h"

#define USBD_CFG_MAX_NUM                1
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ            50

#define USB_INT_CMD 				0x00
#define USB_INT_CONSOLE				0x01
#define USB_INT_TELEMETRY			0x02
#define USB_INT_DEBUG				0x03

#define EP_TELEMETRY_IRQ_IN         0x81  /* EP1 for data IN */
#define EP_CONSOLE_IRQ_IN           0x82  /* EP1 for data IN */
#define EP_DEBUG_IRQ_IN            	0x83  /* EP1 for data IN */
#define EP_CMD 		                0x84  /* EP2 for CDC commands */

#define EP_TELEMETRY_OUT            0x01  /* EP1 for data OUT */
#define EP_CONSOLE_OUT              0x02  /* EP1 for data OUT */

#define CDC_DATA_MAX_PACKET_SIZE    64   /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SZE          8    /* Control Endpoint Packet size */

#define CDC_IN_FRAME_INTERVAL       5
#define USB_BUFFER_SIZE 				2048

#endif
