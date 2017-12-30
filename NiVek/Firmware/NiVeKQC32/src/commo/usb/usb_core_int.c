/*
 * usb_core_int.c
 *
 *  Created on: Aug 24, 2013
 *      Author: Kevin
 */

#include "common/twb_common.h"
#include "commo/usb/usb_core.h"
#include "commo/usb/usb_bsp.h"
#include "common/twb_debug.h"


void USB_OTG_EnableCommonInt(USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_GINTMSK_TypeDef  int_mask;

  int_mask.d32 = 0;
  /* Clear any pending USB_OTG Interrupts */
  USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GINTSTS, 0xFFFFFFFF);
  /* Enable the interrupts in the INTMSK */
  int_mask.b.wkupintr = 1;
  int_mask.b.usbsuspend = 1;

  int_mask.b.otgintr = 1;
  int_mask.b.sessreqintr = 1;
  int_mask.b.conidstschng = 1;
  USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GINTMSK, int_mask.d32);
}


USB_OTG_STS USB_OTG_EnableGlobalInt(USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_GAHBCFG_TypeDef  ahbcfg;

  ahbcfg.d32 = 0;
  ahbcfg.b.glblintrmsk = 1; /* Enable interrupts */
  USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GAHBCFG, 0, ahbcfg.d32);
  return status;
}

USB_OTG_STS USB_OTG_DisableGlobalInt(USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_GAHBCFG_TypeDef  ahbcfg;
  ahbcfg.d32 = 0;
  ahbcfg.b.glblintrmsk = 1; /* Enable interrupts */
  USB_OTG_MODIFY_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32, 0);
  return status;
}

USB_OTG_STS USB_OTG_EnableDevInt(USB_OTG_CORE_HANDLE *pdev){
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_GINTMSK_TypeDef  intmsk;

    intmsk.d32 = 0;

    /* Disable all interrupts. */
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GINTMSK, 0);
    /* Clear any pending interrupts */
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GINTSTS, 0xFFFFFFFF);
    /* Enable the common interrupts */
    USB_OTG_EnableCommonInt(pdev);

    if (pdev->cfg.dma_enable == 0)
      intmsk.b.rxstsqlvl = 1;

    /* Enable interrupts matching to the Device mode ONLY */
    intmsk.b.usbsuspend = 1;
    intmsk.b.usbreset   = 1;
    intmsk.b.enumdone   = 1;
    intmsk.b.inepintr   = 1;
    intmsk.b.outepintr  = 1;
    intmsk.b.sofintr    = 1;

    intmsk.b.incomplisoin    = 1;
    intmsk.b.incomplisoout    = 1;
    USB_OTG_MODIFY_REG32( &pdev->regs.GREGS->GINTMSK, intmsk.d32, intmsk.d32);
    return status;
}



