/*
 * usb_core_cfg.c
 *
 *  Created on: Aug 24, 2013
 *      Author: Kevin
 */

#include "common/twb_common.h"
#include "commo/usb/usb_core.h"
#include "commo/usb/usb_bsp.h"
#include "common/twb_debug.h"


USB_OTG_STS USB_OTG_CoreReset(USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_STS status = USB_OTG_OK;
  __IO USB_OTG_GRSTCTL_TypeDef  greset;
  uint32_t count = 0;

  greset.d32 = 0;
  /* Wait for AHB master IDLE state. */
  do{
    USB_OTG_BSP_uDelay(3);
    greset.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GRSTCTL);
    if (++count > 200000)
      return USB_OTG_OK;
  }

  while (greset.b.ahbidle == 0);
  /* Core Soft Reset */
  count = 0;
  greset.b.csftrst = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GRSTCTL, greset.d32 );
  do{
    greset.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GRSTCTL);
    if (++count > 200000)
      break;

  }
  while (greset.b.csftrst == 1);
  /* Wait for 3 PHY Clocks*/

  USB_OTG_BSP_uDelay(3);
  return status;
}

USB_OTG_STS USB_OTG_CoreInit(USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_GUSBCFG_TypeDef  usbcfg;
  USB_OTG_GCCFG_TypeDef    gccfg;
  USB_OTG_GI2CCTL_TypeDef  i2cctl;
  USB_OTG_GAHBCFG_TypeDef  ahbcfg;

  usbcfg.d32 = 0;
  gccfg.d32 = 0;
  ahbcfg.d32 = 0;

  if (pdev->cfg.phy_itface == USB_OTG_ULPI_PHY){
    gccfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GCCFG);
    gccfg.b.pwdn = 0;

    if (pdev->cfg.Sof_output)
      gccfg.b.sofouten = 1;

    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GCCFG, gccfg.d32);

    /* Init The ULPI Interface */
    usbcfg.d32 = 0;
    usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);

    usbcfg.b.physel            = 0; /* HS Interface */

    usbcfg.b.term_sel_dl_pulse = 0; /* Data line pulsing using utmi_txvalid */
    usbcfg.b.ulpi_utmi_sel     = 1; /* ULPI seleInterfacect */

    usbcfg.b.phyif             = 0; /* 8 bits */
    usbcfg.b.ddrsel            = 0; /* single data rate */

    usbcfg.b.ulpi_fsls = 0;
    usbcfg.b.ulpi_clk_sus_m = 0;
    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);

    /* Reset after a PHY select  */
    USB_OTG_CoreReset(pdev);

    if(pdev->cfg.dma_enable == 1){
      ahbcfg.b.hburstlen = 5; /* 64 x 32-bits*/
      ahbcfg.b.dmaenable = 1;
      USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32);
    }
  }
  else{ /* FS interface (embedded Phy or I2C Phy) */
    usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);;
    usbcfg.b.physel  = 1; /* FS Interface */
    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);
    /* Reset after a PHY select and set Host mode */
    USB_OTG_CoreReset(pdev);
    /* Enable the I2C interface and deactivate the power down*/
    gccfg.d32 = 0;
    gccfg.b.pwdn = 1;

    if(pdev->cfg.phy_itface == USB_OTG_I2C_PHY)
    {
      gccfg.b.i2cifen = 1;
    }
    gccfg.b.vbussensingA = 1 ;
    gccfg.b.vbussensingB = 1 ;
#ifndef VBUS_SENSING_ENABLED
    gccfg.b.disablevbussensing = 1;
#endif

    if(pdev->cfg.Sof_output)
      gccfg.b.sofouten = 1;

    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GCCFG, gccfg.d32);
    USB_OTG_BSP_mDelay(20);
    /* Program GUSBCFG.OtgUtmifsSel to I2C*/
    usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);

    if(pdev->cfg.phy_itface == USB_OTG_I2C_PHY)
      usbcfg.b.otgutmifssel = 1;

    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);

    if(pdev->cfg.phy_itface == USB_OTG_I2C_PHY){
      /*Program GI2CCTL.I2CEn*/
      i2cctl.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GI2CCTL);
      i2cctl.b.i2cdevaddr = 1;
      i2cctl.b.i2cen = 0;
      i2cctl.b.dat_se0 = 1;
      i2cctl.b.addr = 0x2D;
      USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GI2CCTL, i2cctl.d32);

      USB_OTG_BSP_mDelay(200);

      i2cctl.b.i2cen = 1;
      USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GI2CCTL, i2cctl.d32);
      USB_OTG_BSP_mDelay(200);
    }
  }

  /* case the HS core is working in FS mode */
  if(pdev->cfg.dma_enable == 1){
    ahbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GAHBCFG);
    ahbcfg.b.hburstlen = 5; /* 64 x 32-bits*/
    ahbcfg.b.dmaenable = 1;
    USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32);
  }

  /* initialize OTG features */
#ifdef  USE_OTG_MODE
  usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);
  usbcfg.b.hnpcap = 1;
  usbcfg.b.srpcap = 1;
  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);
  USB_OTG_EnableCommonInt(pdev);
#endif

  return status;
}

USB_OTG_STS USB_OTG_SetCurrentMode(USB_OTG_CORE_HANDLE *pdev , uint8_t mode){
  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_GUSBCFG_TypeDef  usbcfg;

  usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);

  usbcfg.b.force_host = 0;
  usbcfg.b.force_dev = 0;

  if ( mode == HOST_MODE)
    usbcfg.b.force_host = 1;
  else if ( mode == DEVICE_MODE)
    usbcfg.b.force_dev = 1;

  USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);
  USB_OTG_BSP_mDelay(50);
  return status;
}

USB_OTG_STS USB_OTG_SelectCore(USB_OTG_CORE_HANDLE *pdev, USB_OTG_CORE_ID_TypeDef coreID){
	uint32_t i , baseAddress = 0;
	USB_OTG_STS status = USB_OTG_OK;

	pdev->cfg.dma_enable       = 0;

	/* at startup the core is in FS mode */
	pdev->cfg.speed            = USB_OTG_SPEED_FULL;
	pdev->cfg.mps              = USB_OTG_FS_MAX_PACKET_SIZE ;

    baseAddress                = USB_OTG_FS_BASE_ADDR;
    pdev->cfg.coreID           = USB_OTG_FS_CORE_ID;
    pdev->cfg.host_channels    = 8 ;
    pdev->cfg.dev_endpoints    = 4 ;
    pdev->cfg.TotalFifoSize    = 320; /* in 32-bits */
    pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;


    pdev->regs.GREGS = (USB_OTG_GREGS *)(baseAddress + USB_OTG_CORE_GLOBAL_REGS_OFFSET);
    pdev->regs.DREGS =  (USB_OTG_DREGS  *)  (baseAddress + USB_OTG_DEV_GLOBAL_REG_OFFSET);

    for (i = 0; i < pdev->cfg.dev_endpoints; i++){
    	pdev->regs.INEP_REGS[i]  = (USB_OTG_INEPREGS *) (baseAddress + USB_OTG_DEV_IN_EP_REG_OFFSET  + (i * USB_OTG_EP_REG_OFFSET));
    	pdev->regs.OUTEP_REGS[i] = (USB_OTG_OUTEPREGS *)(baseAddress + USB_OTG_DEV_OUT_EP_REG_OFFSET + (i * USB_OTG_EP_REG_OFFSET));
    }

    pdev->regs.HREGS = (USB_OTG_HREGS *)(baseAddress + USB_OTG_HOST_GLOBAL_REG_OFFSET);
    pdev->regs.HPRT0 = (uint32_t *)(baseAddress + USB_OTG_HOST_PORT_REGS_OFFSET);

    for (i = 0; i < pdev->cfg.host_channels; i++)
    	pdev->regs.HC_REGS[i] = (USB_OTG_HC_REGS *)(baseAddress + USB_OTG_HOST_CHAN_REGS_OFFSET + 	(i * USB_OTG_CHAN_REGS_OFFSET));

    for (i = 0; i < pdev->cfg.host_channels; i++)
    	pdev->regs.DFIFO[i] = (uint32_t *)(baseAddress + USB_OTG_DATA_FIFO_OFFSET +	(i * USB_OTG_DATA_FIFO_SIZE));

    pdev->regs.PCGCCTL = (uint32_t *)(baseAddress + USB_OTG_PCGCCTL_OFFSET);

    return status;
}
