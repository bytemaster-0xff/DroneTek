/*
 * usb_core_io.c
 *
 *  Created on: Aug 24, 2013
 *      Author: Kevin
 */

#include "twb_common.h"
#include "usb_core.h"
#include "usb_bsp.h"
#include "twb_debug.h"

USB_OTG_STS USB_OTG_WritePacket(USB_OTG_CORE_HANDLE *pdev, uint8_t *src, uint8_t epnum, uint16_t len) {
	USB_OTG_STS status = USB_OTG_OK;

	uint32_t count32b= 0 , i= 0;
	__IO uint32_t *fifo;

	count32b =  (len + 3) / 4;

	//TWB_Debug_PrintInt("TxPk: ", epnum);
	//TWB_Debug_Print2Int("Size/Packets: ", len, count32b);

	fifo = pdev->regs.DFIFO[epnum];

	for (i = 0; i < count32b; i++, src+=4){
		/*TWB_Debug_Print("Addr: ");
		TWB_Debug_PrintHexNumb(fifo);
		TWB_Debug_Print(" - ");
		TWB_Debug_PrintHexNumb(src);
		TWB_Debug_Print(" - ");
		TWB_Debug_PrintHexNumb(*((__packed uint32_t *)src));
		TWB_Debug_Print("\r\n");*/

		USB_OTG_WRITE_REG32( fifo, *((__packed uint32_t *)src) );
	}

	return status;
}


void *USB_OTG_ReadPacket(USB_OTG_CORE_HANDLE *pdev, uint8_t *dest, uint16_t len){
	uint32_t i=0;
	uint32_t count32b = (len + 3) / 4;

	__IO uint32_t *fifo = pdev->regs.DFIFO[0];

	for ( i = 0; i < count32b; i++, dest += 4 )
		*(__packed uint32_t *)dest = USB_OTG_READ_REG32(fifo);

	return ((void *)dest);
}


USB_OTG_STS USB_OTG_FlushTxFifo (USB_OTG_CORE_HANDLE *pdev , uint32_t num ){
	USB_OTG_STS status = USB_OTG_OK;
	__IO USB_OTG_GRSTCTL_TypeDef  greset;

	uint32_t count = 0;
	greset.d32 = 0;
	greset.b.txfflsh = 1;
	greset.b.txfnum  = num;
	USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GRSTCTL, greset.d32 );
	do{
		greset.d32 = USB_OTG_READ_REG32( &pdev->regs.GREGS->GRSTCTL);
		if (++count > 200000)
			break;
	}
	while (greset.b.txfflsh == 1);

	/* Wait for 3 PHY Clocks*/
	USB_OTG_BSP_uDelay(3);
	return status;
}


USB_OTG_STS USB_OTG_FlushRxFifo( USB_OTG_CORE_HANDLE *pdev ){
	USB_OTG_STS status = USB_OTG_OK;
	__IO USB_OTG_GRSTCTL_TypeDef  greset;
	uint32_t count = 0;

	greset.d32 = 0;
	greset.b.rxfflsh = 1;

	USB_OTG_WRITE_REG32( &pdev->regs.GREGS->GRSTCTL, greset.d32 );

	do{
		greset.d32 = USB_OTG_READ_REG32( &pdev->regs.GREGS->GRSTCTL);
		if (++count > 200000)
			break;
	}
	while (greset.b.rxfflsh == 1);

	/* Wait for 3 PHY Clocks*/
	USB_OTG_BSP_uDelay(3);
	return status;
}

/* Will start transfer TO device as OUT from Host */
USB_OTG_STS __usb_otg_epStartOutXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	USB_OTG_STS status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef     depctl;
	USB_OTG_DEPXFRSIZ_TypeDef  deptsiz;

    depctl.d32  = USB_OTG_READ_REG32(&(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL));
    deptsiz.d32 = USB_OTG_READ_REG32(&(pdev->regs.OUTEP_REGS[ep->num]->DOEPTSIZ));

    /* Program the transfer size and packet count as follows:
    * pktcnt = N
    * xfersize = N * maxpacket
    */
    if (ep->xfer_len == 0){
      deptsiz.b.xfersize = ep->maxpacket;
      deptsiz.b.pktcnt = 1;
    }
    else{
      deptsiz.b.pktcnt = (ep->xfer_len + (ep->maxpacket - 1)) / ep->maxpacket;
      deptsiz.b.xfersize = deptsiz.b.pktcnt * ep->maxpacket;
    }

    USB_OTG_WRITE_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPTSIZ, deptsiz.d32);

    if (pdev->cfg.dma_enable == 1)
      USB_OTG_WRITE_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPDMA, ep->dma_addr);

    if (ep->type == EP_TYPE_ISOC){
      if (ep->even_odd_frame)
        depctl.b.setd1pid = 1;
      else
        depctl.b.setd0pid = 1;
    }

    /* EP enable */
    depctl.b.cnak = 1;
    depctl.b.epena = 1;
    USB_OTG_WRITE_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL, depctl.d32);

	return status;
}

/* Will start transfer FROM device as IN to Host */


USB_OTG_STS __usb_otg_epStartInXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	USB_OTG_STS status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef     depctl;
	USB_OTG_DEPXFRSIZ_TypeDef  deptsiz;
	uint32_t fifoemptymsk = 0;

	depctl.d32 = 0;
	deptsiz.d32 = 0;

	depctl.d32  = USB_OTG_READ_REG32(&(pdev->regs.INEP_REGS[ep->num]->DIEPCTL));
	deptsiz.d32 = USB_OTG_READ_REG32(&(pdev->regs.INEP_REGS[ep->num]->DIEPTSIZ));

	if (ep->xfer_len == 0){
	  deptsiz.b.xfersize = 0;
	  deptsiz.b.pktcnt = 1;
	}
	else{
	  deptsiz.b.xfersize = ep->xfer_len;
	  deptsiz.b.pktcnt = (ep->xfer_len - 1 + ep->maxpacket) / ep->maxpacket;
	}

	USB_OTG_WRITE_REG32(&pdev->regs.INEP_REGS[ep->num]->DIEPTSIZ, deptsiz.d32);

	TWB_Debug_Print2Int("START XFER: ", ep->num, ep->xfer_len);

	depctl.b.cnak = 1;
	depctl.b.epena = 1;
	USB_OTG_WRITE_REG32(&pdev->regs.INEP_REGS[ep->num]->DIEPCTL, depctl.d32);

	/* Enable the Tx FIFO Empty Interrupt for this EP */
	if (ep->xfer_len > 0){
	  fifoemptymsk |= 1 << ep->num;
	  USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DIEPEMPMSK, 0, fifoemptymsk);
	}

	return status;
}

USB_OTG_STS USB_OTG_EPStartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
  if (ep->is_in == 1)
	  return __usb_otg_epStartInXfer(pdev, ep);
  else
	  return __usb_otg_epStartOutXfer(pdev, ep);
}


USB_OTG_STS __usb_otg_ep0StartOutXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	USB_OTG_STS                 status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef      depctl;
	USB_OTG_DEP0XFRSIZ_TypeDef  deptsiz;

	depctl.d32   = 0;
	deptsiz.d32  = 0;

	depctl.d32  = USB_OTG_READ_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL);
	deptsiz.d32 = USB_OTG_READ_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPTSIZ);

	if (ep->xfer_len != 0)
		ep->xfer_len = ep->maxpacket;

	deptsiz.b.xfersize = ep->maxpacket;
	deptsiz.b.pktcnt = 1;

	USB_OTG_WRITE_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPTSIZ, deptsiz.d32);

	if (pdev->cfg.dma_enable == 1)
	  USB_OTG_WRITE_REG32(&pdev->regs.OUTEP_REGS[ep->num]->DOEPDMA, ep->dma_addr);

	/* EP enable */
	depctl.b.cnak = 1;
	depctl.b.epena = 1;
	USB_OTG_WRITE_REG32 (&(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL), depctl.d32);

	return status;
}

USB_OTG_STS __usb_otg_ep0StartInXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	USB_OTG_STS                 status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef      depctl;
	USB_OTG_DEP0XFRSIZ_TypeDef  deptsiz;
	USB_OTG_INEPREGS          *in_regs;
	uint32_t fifoemptymsk = 0;

	depctl.d32   = 0;
	deptsiz.d32  = 0;

	in_regs = pdev->regs.INEP_REGS[0];
	depctl.d32  = USB_OTG_READ_REG32(&in_regs->DIEPCTL);
	deptsiz.d32 = USB_OTG_READ_REG32(&in_regs->DIEPTSIZ);

	if (ep->xfer_len == 0){
		deptsiz.b.xfersize = 0;
		deptsiz.b.pktcnt = 1;
	}
	else{
	  if (ep->xfer_len > ep->maxpacket){
		ep->xfer_len = ep->maxpacket;
		deptsiz.b.xfersize = ep->maxpacket;
	  }
	  else
		deptsiz.b.xfersize = ep->xfer_len;

	  deptsiz.b.pktcnt = 1;
	}

	USB_OTG_WRITE_REG32(&in_regs->DIEPTSIZ, deptsiz.d32);

	/* EP enable, IN data in FIFO */
	depctl.b.cnak = 1;
	depctl.b.epena = 1;
	USB_OTG_WRITE_REG32(&in_regs->DIEPCTL, depctl.d32);

	/* Enable the Tx FIFO Empty Interrupt for this EP */
	if (ep->xfer_len > 0){
	  fifoemptymsk |= 1 << ep->num;
	  USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DIEPEMPMSK, 0, fifoemptymsk);
	}

	return status;
}

USB_OTG_STS USB_OTG_EP0StartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
  if (ep->is_in == 1)
	 return  __usb_otg_ep0StartInXfer(pdev, ep);
  else
	  return __usb_otg_ep0StartOutXfer(pdev, ep);
}
