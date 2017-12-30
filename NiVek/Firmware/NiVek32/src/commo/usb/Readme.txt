NiVek USB Implementation


Five Interfaces as defined in USB

Command
Telemetry
Debug Output
Console
DFU

File Definitions
1) twb_usb.c 		- initialization and high level interface to the USB devices
2) usb_bsp.c 		- Board support package, will initialize USB specific H/W for NiVek.
3) usb_core.c 		- Provides low level drivers for communicating with the USB periperal
4) usb_dcd_int.c 	- Interrupt handlers for a communication device USB device
5) usdd_cdc_core.c 	- Provide core implementation of a communciations device
6) usbd_core.c		- Manage setup/configuration for a USBD device
7) usbd_desc.c		- Provide one place for populating the configuration with strings and such during enumeration
8) usbd_ioreq.c		- Looks like flow control for a communciation device (Not 100% sure as of 8/24/2013)
9) usbd_req.c		- Handles enumeration requests
10)usbd_usr.c		- Provide hooks to allow app to take action upon connect/disconnect of USB devices. 

