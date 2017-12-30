/**
  ******************************************************************************
  * @file    usb_conf.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    22-July-2011
  * @brief   general low level driver configuration
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CONF__H__
#define __USB_CONF__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/** @addtogroup USB_OTG_DRIVER
  * @{
  */
  
/** @defgroup USB_CONF
  * @brief USB low level driver configuration file
  * @{
  */ 

/** @defgroup USB_CONF_Exported_Defines
  * @{
  */ 

/* USB Core and PHY interface configuration.
   Tip: To avoid modifying these defines each time you need to change the USB
        configuration, you can declare the needed define in your toolchain
        compiler preprocessor.
   */
#ifndef USE_USB_OTG_FS
#define USE_USB_OTG_FS
#endif /* USE_USB_OTG_FS */

#ifndef USE_USB_OTG_HS
 //#define USE_USB_OTG_HS
#endif /* USE_USB_OTG_HS */

#ifndef USE_ULPI_PHY
#define USE_ULPI_PHY
#endif /* USE_ULPI_PHY */

#ifndef USE_EMBEDDED_PHY
 //#define USE_EMBEDDED_PHY
#endif /* USE_EMBEDDED_PHY */

#ifndef USE_I2C_PHY
 //#define USE_I2C_PHY
#endif /* USE_I2C_PHY */


#ifdef USE_USB_OTG_FS 
 #define USB_OTG_FS_CORE
#endif

#ifdef USE_USB_OTG_HS
 #define USB_OTG_HS_CORE
#endif

/*******************************************************************************
*                      FIFO Size Configuration in Device mode
*  
*  (i) Receive data FIFO size = RAM for setup packets + 
*                   OUT endpoint control information +
*                   data OUT packets + miscellaneous
*      Space = ONE 32-bits words
*     --> RAM for setup packets = 10 spaces
*        (n is the nbr of CTRL EPs the device core supports) 
*     --> OUT EP CTRL info      = 1 space
*        (one space for status information written to the FIFO along with each 
*        received packet)
*     --> data OUT packets      = (Largest Packet Size / 4) + 1 spaces 
*        (MINIMUM to receive packets)
*     --> OR data OUT packets  = at least 2*(Largest Packet Size / 4) + 1 spaces 
*        (if high-bandwidth EP is enabled or multiple isochronous EPs)
*     --> miscellaneous = 1 space per OUT EP
*        (one space for transfer complete status information also pushed to the 
*        FIFO with each endpoint's last packet)
*
*  (ii)MINIMUM RAM space required for each IN EP Tx FIFO = MAX packet size for 
*       that particular IN EP. More space allocated in the IN EP Tx FIFO results
*       in a better performance on the USB and can hide latencies on the AHB.
*
*  (iii) TXn min size = 16 words. (n  : Transmit FIFO index)
*   (iv) When a TxFIFO is not used, the Configuration should be as follows: 
*       case 1 :  n > m    and Txn is not used    (n,m  : Transmit FIFO indexes)
*       --> Txm can use the space allocated for Txn.
*       case2  :  n < m    and Txn is not used    (n,m  : Transmit FIFO indexes)
*       --> Txn should be configured with the minimum space of 16 words
*  (v) The FIFO is used optimally when used TxFIFOs are allocated in the top 
*       of the FIFO.Ex: use EP1 and EP2 as IN instead of EP1 and EP3 as IN ones.
*******************************************************************************/

/*******************************************************************************
*                     FIFO Size Configuration in Host mode
*
*  (i) Receive data FIFO size = (Largest Packet Size / 4) + 1 or
*                             2x (Largest Packet Size / 4) + 1,  If a
*                             high-bandwidth channel or multiple isochronous
*                             channels are enabled
*
*  (ii) For the host nonperiodic Transmit FIFO is the largest maximum packet size
*      for all supported nonperiodic OUT channels. Typically, a space
*      corresponding to two Largest Packet Size is recommended.
*
*  (iii) The minimum amount of RAM required for Host periodic Transmit FIFO is
*        the largest maximum packet size for all supported periodic OUT channels.
*        If there is at least one High Bandwidth Isochronous OUT endpoint,
*        then the space must be at least two times the maximum packet size for
*        that channel.
*******************************************************************************/
 

/****************** USB OTG FS CONFIGURATION **********************************/
#ifdef USB_OTG_FS_CORE
 #define RX_FIFO_FS_SIZE                          128
 #define TX0_FIFO_FS_SIZE                          64
 #define TX1_FIFO_FS_SIZE                          64
 #define TX2_FIFO_FS_SIZE                          64
 #define TX3_FIFO_FS_SIZE                          64
 #define TXH_NP_FS_FIFOSIZ                         96
 #define TXH_P_FS_FIFOSIZ                          96

 //#define USB_OTG_FS_LOW_PWR_MGMT_SUPPORT
 //#define USB_OTG_FS_SOF_OUTPUT_ENABLED
#endif

/****************** USB OTG MODE CONFIGURATION ********************************/
//#define USE_HOST_MODE
#define USE_DEVICE_MODE
//#define USE_OTG_MODE



#define __ALIGN_BEGIN
#define __ALIGN_END

#define __packed    __attribute__ ((__packed__))

#endif
