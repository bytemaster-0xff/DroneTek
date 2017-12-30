/**
  * @brief   USB-OTG Core Layer
  */

/* Includes ------------------------------------------------------------------*/
#include "common/twb_common.h"
#include "commo/usb/usb_core.h"
#include "commo/usb/usb_bsp.h"
#include "common/twb_debug.h"


uint32_t USB_OTG_GetMode(USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_READ_REG32(&pdev->regs.GREGS->GINTSTS ) & 0x1); }
uint8_t USB_OTG_IsDeviceMode(USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_GetMode(pdev) != HOST_MODE); }
uint8_t USB_OTG_IsHostMode(USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_GetMode(pdev) == HOST_MODE); }
uint32_t USB_OTG_ReadOtgItr (USB_OTG_CORE_HANDLE *pdev) { return (USB_OTG_READ_REG32 (&pdev->regs.GREGS->GOTGINT)); }

uint32_t USB_OTG_ReadCoreItr(USB_OTG_CORE_HANDLE *pdev){
  uint32_t v = 0;
  v = USB_OTG_READ_REG32(&pdev->regs.GREGS->GINTSTS);
  v &= USB_OTG_READ_REG32(&pdev->regs.GREGS->GINTMSK);
  return v;
}

void USB_OTG_InitDevSpeed(USB_OTG_CORE_HANDLE *pdev , uint8_t speed){
  USB_OTG_DCFG_TypeDef   dcfg;
  
  dcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DCFG);
  dcfg.b.devspd = speed;
  USB_OTG_WRITE_REG32(&pdev->regs.DREGS->DCFG, dcfg.d32);
}

USB_OTG_STS USB_OTG_CoreInitDev (USB_OTG_CORE_HANDLE *pdev){
	USB_OTG_STS             status       = USB_OTG_OK;
	USB_OTG_DEPCTL_TypeDef  depctl;
	uint32_t i;
	USB_OTG_DCFG_TypeDef    dcfg;
	USB_OTG_FSIZ_TypeDef    nptxfifosize;
	USB_OTG_FSIZ_TypeDef    txfifosize;
	USB_OTG_DIEPMSK_TypeDef msk;
	USB_OTG_DTHRCTL_TypeDef dthrctl;
  
	depctl.d32 = 0;
	dcfg.d32 = 0;
	nptxfifosize.d32 = 0;
	txfifosize.d32 = 0;
	msk.d32 = 0;
  
	/* Restart the Phy Clock */
	USB_OTG_WRITE_REG32(pdev->regs.PCGCCTL, 0);
	/* Device configuration register */
	dcfg.d32 = USB_OTG_READ_REG32( &pdev->regs.DREGS->DCFG);
	dcfg.b.perfrint = DCFG_FRAME_INTERVAL_80;
	USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DCFG, dcfg.d32 );
  
    USB_OTG_InitDevSpeed (pdev , USB_OTG_SPEED_PARAM_FULL);
    
    /* set Rx FIFO size */
    USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GRXFSIZ, RX_FIFO_FS_SIZE);
    
    /* EP0 TX*/
    nptxfifosize.b.depth     = TX0_FIFO_FS_SIZE;
    nptxfifosize.b.startaddr = RX_FIFO_FS_SIZE;
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->DIEPTXF0_HNPTXFSIZ, nptxfifosize.d32 );
    
    /* EP1 TX*/
    txfifosize.b.startaddr = nptxfifosize.b.startaddr + nptxfifosize.b.depth;
    txfifosize.b.depth = TX1_FIFO_FS_SIZE;
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->DIEPTXF[0], txfifosize.d32 );
    
    /* EP2 TX*/
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX2_FIFO_FS_SIZE;
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->DIEPTXF[1], txfifosize.d32 );
    
    /* EP3 TX*/  
    txfifosize.b.startaddr += txfifosize.b.depth;
    txfifosize.b.depth = TX3_FIFO_FS_SIZE;
    USB_OTG_WRITE_REG32( &pdev->regs.GREGS->DIEPTXF[2], txfifosize.d32 );

	/* Flush the FIFOs */
	USB_OTG_FlushTxFifo(pdev , 0x10); /* all Tx FIFOs */
	USB_OTG_FlushRxFifo(pdev);

	/* Clear all pending Device Interrupts */
	USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DIEPMSK, 0 );
	USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DOEPMSK, 0 );
	USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINT, 0xFFFFFFFF );
	USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINTMSK, 0 );
  
	for (i = 0; i < pdev->cfg.dev_endpoints; i++){
		depctl.d32 = USB_OTG_READ_REG32(&pdev->regs.INEP_REGS[i]->DIEPCTL);
		if (depctl.b.epena){
			depctl.d32 = 0;
			depctl.b.epdis = 1;
			depctl.b.snak = 1;
		}
		else
			depctl.d32 = 0;

		USB_OTG_WRITE_REG32( &pdev->regs.INEP_REGS[i]->DIEPCTL, depctl.d32);
		USB_OTG_WRITE_REG32( &pdev->regs.INEP_REGS[i]->DIEPTSIZ, 0);
		USB_OTG_WRITE_REG32( &pdev->regs.INEP_REGS[i]->DIEPINT, 0xFF);
	}

	for (i = 0; i <  pdev->cfg.dev_endpoints; i++){
		USB_OTG_DEPCTL_TypeDef  depctl;
		depctl.d32 = USB_OTG_READ_REG32(&pdev->regs.OUTEP_REGS[i]->DOEPCTL);
		if (depctl.b.epena){
			depctl.d32 = 0;
			depctl.b.epdis = 1;
			depctl.b.snak = 1;
		}
		else
			depctl.d32 = 0;

		USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[i]->DOEPCTL, depctl.d32);
		USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[i]->DOEPTSIZ, 0);
		USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
	}

	msk.d32 = 0;
	msk.b.txfifoundrn = 1;
	USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DIEPMSK, msk.d32, msk.d32);
  
	USB_OTG_EnableDevInt(pdev);
	return status;
}


enum USB_OTG_SPEED USB_OTG_GetDeviceSpeed (USB_OTG_CORE_HANDLE *pdev){
  USB_OTG_DSTS_TypeDef  dsts;
  enum USB_OTG_SPEED speed = USB_SPEED_UNKNOWN;
  
  dsts.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DSTS);
  
  switch (dsts.b.enumspd)
  {
  case DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ:
    speed = USB_SPEED_HIGH;
    break;
  case DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ:
  case DSTS_ENUMSPD_FS_PHY_48MHZ:
    speed = USB_SPEED_FULL;
    break;
    
  case DSTS_ENUMSPD_LS_PHY_6MHZ:
    speed = USB_SPEED_LOW;
    break;
  }
  
  return speed;
}

uint32_t USB_OTG_ReadDevAllOutEp_itr(USB_OTG_CORE_HANDLE *pdev){
  uint32_t v;
  v  = USB_OTG_READ_REG32(&pdev->regs.DREGS->DAINT);
  v &= USB_OTG_READ_REG32(&pdev->regs.DREGS->DAINTMSK);
  return ((v & 0xffff0000) >> 16);
}

uint32_t USB_OTG_ReadDevOutEP_itr(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum){
  uint32_t v;
  v  = USB_OTG_READ_REG32(&pdev->regs.OUTEP_REGS[epnum]->DOEPINT);
  v &= USB_OTG_READ_REG32(&pdev->regs.DREGS->DOEPMSK);
  return v;
}


uint32_t USB_OTG_ReadDevAllInEPItr(USB_OTG_CORE_HANDLE *pdev){
  uint32_t v;
  v = USB_OTG_READ_REG32(&pdev->regs.DREGS->DAINT);
  v &= USB_OTG_READ_REG32(&pdev->regs.DREGS->DAINTMSK);
  return (v & 0xffff);
}

void USB_OTG_ActiveRemoteWakeup(USB_OTG_CORE_HANDLE *pdev){
  
  USB_OTG_DCTL_TypeDef     dctl;
  USB_OTG_DSTS_TypeDef     dsts;
  USB_OTG_PCGCCTL_TypeDef  power;  
  
  if (pdev->dev.DevRemoteWakeup) {
    dsts.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DSTS);
    if(dsts.b.suspsts == 1)
    {
      if(pdev->cfg.low_power)
      {
        /* un-gate USB Core clock */
        power.d32 = USB_OTG_READ_REG32(&pdev->regs.PCGCCTL);
        power.b.gatehclk = 0;
        power.b.stoppclk = 0;
        USB_OTG_WRITE_REG32(pdev->regs.PCGCCTL, power.d32);
      }   
      /* active Remote wakeup signaling */
      dctl.d32 = 0;
      dctl.b.rmtwkupsig = 1;
      USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DCTL, 0, dctl.d32);
      USB_OTG_BSP_mDelay(5);
      USB_OTG_MODIFY_REG32(&pdev->regs.DREGS->DCTL, dctl.d32, 0 );
    }
  }
}

void USB_OTG_UngateClock(USB_OTG_CORE_HANDLE *pdev){
  if(pdev->cfg.low_power){
    USB_OTG_DSTS_TypeDef     dsts;
    USB_OTG_PCGCCTL_TypeDef  power; 
    
    dsts.d32 = USB_OTG_READ_REG32(&pdev->regs.DREGS->DSTS);
    
    if(dsts.b.suspsts == 1){
      /* un-gate USB Core clock */
      power.d32 = USB_OTG_READ_REG32(&pdev->regs.PCGCCTL);
      power.b.gatehclk = 0;
      power.b.stoppclk = 0;
      USB_OTG_WRITE_REG32(pdev->regs.PCGCCTL, power.d32);
    }
  }
}

void USB_OTG_StopDevice(USB_OTG_CORE_HANDLE *pdev){
  uint32_t i;
  
  pdev->dev.device_status = 1;
    
  for (i = 0; i < pdev->cfg.dev_endpoints ; i++)
  {
    USB_OTG_WRITE_REG32( &pdev->regs.INEP_REGS[i]->DIEPINT, 0xFF);
    USB_OTG_WRITE_REG32( &pdev->regs.OUTEP_REGS[i]->DOEPINT, 0xFF);
  }

  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DIEPMSK, 0 );
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DOEPMSK, 0 );
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINTMSK, 0 );
  USB_OTG_WRITE_REG32( &pdev->regs.DREGS->DAINT, 0xFFFFFFFF );  
  
  /* Flush the FIFO */
  USB_OTG_FlushRxFifo(pdev);
  USB_OTG_FlushTxFifo(pdev ,  0x10 );  
}



