/*
 * usb_core_ep.c
 *
 *  Created on: Aug 24, 2013
 *      Author: Kevin
 */

#include "common/twb_common.h"
#include "commo/usb/usb_core.h"
#include "commo/usb/usb_bsp.h"
#include "common/twb_debug.h"


uint32_t USB_OTG_GetEPStatus(USB_OTG_CORE_HANDLE *pdev ,USB_OTG_EP *ep){
  USB_OTG_DEPCTL_TypeDef  depctl;
  __IO uint32_t *depctl_addr;
  uint32_t Status = 0;

  depctl.d32 = 0;
  if (ep->is_in == 1){
    depctl_addr = &(pdev->regs.INEP_REGS[ep->num]->DIEPCTL);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

    if (depctl.b.stall == 1)
      Status = USB_OTG_EP_TX_STALL;
    else if (depctl.b.naksts == 1)
      Status = USB_OTG_EP_TX_NAK;
    else
      Status = USB_OTG_EP_TX_VALID;
  }
  else{
    depctl_addr = &(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);
    if (depctl.b.stall == 1)
      Status = USB_OTG_EP_RX_STALL;
    else if (depctl.b.naksts == 1)
      Status = USB_OTG_EP_RX_NAK;
    else
      Status = USB_OTG_EP_RX_VALID;
  }

  /* Return the current status */
  return Status;
}

void __usb_otg_SetInEpStatus(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep , uint32_t status){
	TWB_Debug_Print2Int("EP STAT-IN->", ep->num, status);

	USB_OTG_DEPCTL_TypeDef  depctl;
	__IO uint32_t *depctl_addr;

	depctl.d32 = 0;

	depctl_addr = &(pdev->regs.INEP_REGS[ep->num]->DIEPCTL);
	depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

	if (status == USB_OTG_EP_TX_STALL)  {
	  USB_OTG_EPSetStall(pdev, ep);
	  return;
	}
	else if (status == USB_OTG_EP_TX_NAK)
	  depctl.b.snak = 1;
	else if (status == USB_OTG_EP_TX_VALID){
	  if (depctl.b.stall == 1){
		ep->even_odd_frame = 0;
		USB_OTG_EPClearStall(pdev, ep);
		return;
	  }

	  depctl.b.cnak = 1;
	  depctl.b.usbactep = 1;
	  depctl.b.epena = 1;
	}
	else if (status == USB_OTG_EP_TX_DIS)
	  depctl.b.usbactep = 0;

	USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
}

void __usb_otg_SetOutEpStatus(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep , uint32_t status){
	USB_OTG_DEPCTL_TypeDef  depctl;
	__IO uint32_t *depctl_addr;

	depctl.d32 = 0;

    depctl_addr = &(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

    if (status == USB_OTG_EP_RX_STALL)
      depctl.b.stall = 1;
    else if (status == USB_OTG_EP_RX_NAK)
      depctl.b.snak = 1;
    else if (status == USB_OTG_EP_RX_VALID){
      if (depctl.b.stall == 1){
        ep->even_odd_frame = 0;
        USB_OTG_EPClearStall(pdev, ep);
        return;
      }
      depctl.b.cnak = 1;
      depctl.b.usbactep = 1;
      depctl.b.epena = 1;
    }
    else if (status == USB_OTG_EP_RX_DIS)
      depctl.b.usbactep = 0;

    USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
}

void USB_OTG_SetEPStatus (USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep , uint32_t status){
	if (ep->is_in == 1)
		__usb_otg_SetOutEpStatus(pdev, ep, status);
	else
		__usb_otg_SetOutEpStatus(pdev, ep, status);
}

USB_OTG_STS  USB_OTG_EP0Activate(USB_OTG_CORE_HANDLE *pdev){
	USB_OTG_STS             status = USB_OTG_OK;
	USB_OTG_DSTS_TypeDef    dsts;
	USB_OTG_DEPCTL_TypeDef  diepctl;
	USB_OTG_DCTL_TypeDef    dctl;

	dctl.d32 = 0;

	/* Read the Device Status and Endpoint 0 Control registers */
	dsts.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DSTS);
	diepctl.d32 = USB_OTG_READ_REG32(&pdev->regs.INEP_REGS[0]->DIEPCTL);

	/* Set the MPS of the IN EP based on the enumeration speed */
	switch (dsts.b.enumspd){
		case DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
		case DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
		case DSTS_ENUMSPD_FS_PHY_48MHZ:
			diepctl.b.mps = DEP0CTL_MPS_64;
		break;
		case DSTS_ENUMSPD_LS_PHY_6MHZ:
			diepctl.b.mps = DEP0CTL_MPS_8;
		break;
	}

	USB_OTG_WRITE_REG32(&pdev->regs.INEP_REGS[0]->DIEPCTL, diepctl.d32);
	dctl.b.cgnpinnak = 1;
	USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DCTL, dctl.d32, dctl.d32);
	return status;
}

USB_OTG_STS USB_OTG_EPActivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	USB_OTG_STS status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef  depctl;
	USB_OTG_DAINT_TypeDef  daintmsk;
	__IO uint32_t *addr;

	depctl.d32 = 0;
	daintmsk.d32 = 0;
	/* Read DEPCTLn register */

	if (ep->is_in == 1){
		addr = &pdev->regs.INEP_REGS[ep->num]->DIEPCTL;
		daintmsk.ep.in = 1 << ep->num;
	}
	else{
		addr = &pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL;
		daintmsk.ep.out = 1 << ep->num;
	}

	/* If the EP is already active don't change the EP Control register. */
	depctl.d32 = USB_OTG_READ_REG32(addr);

	if (!depctl.b.usbactep){
		depctl.b.mps    = ep->maxpacket;
		depctl.b.eptype = ep->type;
		depctl.b.txfnum = ep->tx_fifo_num;
		depctl.b.setd0pid = 1;
		depctl.b.usbactep = 1;
		USB_OTG_WRITE_REG32(addr, depctl.d32);
	}

	/* Enable the Interrupt for this EP */
	USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DAINTMSK, 0, daintmsk.d32);
	return status;
}

USB_OTG_STS USB_OTG_EPDeactivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	TWB_Debug_PrintInt("EP DEACT->", ep->num);

	USB_OTG_STS status = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef  depctl;
	USB_OTG_DAINT_TypeDef  daintmsk;
	__IO uint32_t *addr;

	  depctl.d32 = 0;
	  daintmsk.d32 = 0;

	if (ep->is_in == 1){
		addr = &pdev->regs.INEP_REGS[ep->num]->DIEPCTL;
		daintmsk.ep.in = 1 << ep->num;
	}
	else{
		addr = &pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL;
		daintmsk.ep.out = 1 << ep->num;
	}

	depctl.b.usbactep = 0;
	USB_OTG_WRITE_REG32(addr, depctl.d32);

	USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DAINTMSK, daintmsk.d32, 0);
	return status;
}

USB_OTG_STS USB_OTG_EPSetStall(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	TWB_Debug_PrintInt("EP STALL->", ep->num);
  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_DEPCTL_TypeDef  depctl;
  __IO uint32_t *depctl_addr;

  depctl.d32 = 0;
  if (ep->is_in == 1){
    depctl_addr = &(pdev->regs.INEP_REGS[ep->num]->DIEPCTL);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);
    /* set the disable and stall bits */
    if (depctl.b.epena)
      depctl.b.epdis = 1;

    depctl.b.stall = 1;
    USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  }
  else{
    depctl_addr = &(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL);
    depctl.d32 = USB_OTG_READ_REG32(depctl_addr);
    /* set the stall bit */
    depctl.b.stall = 1;
    USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  }
  return status;
}

USB_OTG_STS USB_OTG_EPClearStall(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep){
	TWB_Debug_PrintInt("EP CLEAR STALL->", ep->num);

  USB_OTG_STS status = USB_OTG_OK;
  USB_OTG_DEPCTL_TypeDef  depctl;
  __IO uint32_t *depctl_addr;

  depctl.d32 = 0;

  if (ep->is_in == 1)
    depctl_addr = &(pdev->regs.INEP_REGS[ep->num]->DIEPCTL);
  else
    depctl_addr = &(pdev->regs.OUTEP_REGS[ep->num]->DOEPCTL);

  depctl.d32 = USB_OTG_READ_REG32(depctl_addr);

  depctl.b.stall = 0;
  if (ep->type == EP_TYPE_INTR || ep->type == EP_TYPE_BULK)
    depctl.b.setd0pid = 1; /* DATA0 */

  USB_OTG_WRITE_REG32(depctl_addr, depctl.d32);
  return status;
}

void USB_OTG_EP0_OutStart(USB_OTG_CORE_HANDLE *pdev){
	USB_OTG_DEP0XFRSIZ_TypeDef doeptsize0;
	doeptsize0.d32 = 0;
	doeptsize0.b.supcnt = 3;
	doeptsize0.b.pktcnt = 1;
	doeptsize0.b.xfersize = 8 * 3;
	USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[0]->DOEPTSIZ, doeptsize0.d32 );
}
