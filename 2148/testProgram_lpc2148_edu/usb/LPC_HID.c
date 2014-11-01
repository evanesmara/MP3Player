/*************************************************************************
 *
 **************************************************************************/
#include <lpc2xxx.h>
#include <printf_P.h>
#include "lpc_hid.h"

typedef union
{
	unsigned int Data[2];
	unsigned short SetupRecuest;
	struct
	{
		unsigned char RequestType;
		unsigned char Request;
		unsigned short Value;
		unsigned short Index;
		unsigned short Length;
	}__attribute__ ((packed)) DataFields;
}__attribute__ ((packed)) USB_SETUP_PACKET_DEF;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

unsigned char currentConfiguration __attribute__ ((aligned (8)));
unsigned char currentAddress __attribute__ ((aligned (8)));

short mouseDescriptor[] __attribute__ ((aligned (8))) =
{ 0x0105, // Usage Page (Generic Desktop)
		0x0209, // Usage (Mouse)
		0x01A1, // Collection (Application)
		0x0109, //  Usage (Pointer)
		0x00A1, //  Collection (Physical)
		0x0905, //    Usage Page (Buttons)
		0x0119, //    Usage Minimumù (01)
		0x0329, //    Usage Maximum (03)
		0x0015, //    Logical Minimum (0)
		0x0125, //    Logical Maximum (1)
		0x0395, //    Report Count (3)
		0x0175, //    Report Size (1)
		0x0281, //    3 Button bits
		0x0195, //    Report Count (1)
		0x0575, //    Report Size (6)
		0x0181, //    6 bit padding
		0x0105, //    Generic desktop
		0x3009, //    Usage(X)
		0x3109, //    Usage(Y)
		0x8115, //    Logical Minimum (-127)
		0x7F25, //    Logical Maximum (127)
		0x0875, //    Report Size (8)
		0x0295, //    Report Count (2)
		0x0681, //    2 position bytes
		0xC0C0 // End collection x2
		};

// Check http://www.usb.org/developers/hidpage/#Class_Definition
char devDescriptor[] __attribute__ ((aligned (8))) =
{
/* Device descriptor */
0x12, // bLength
		0x01, // bDescriptorType
		0x10, // bcdUSBL
		0x01, //
		0x00, // bDeviceClass:
		0x00, // bDeviceSubclass:
		0x00, // bDeviceProtocol:
		MAX_CTRL_EP_PK_SIZE, // bMaxPacketSize0
		0xFF, // idVendorL
		0xFF, //
		0x01, // idProductL
		0x00, //
		0x00, // bcdDeviceL
		0x00, //
		0x01, // iManufacturer
		0x02, // iProduct
		0x00, // SerialNumber
		0x01 // bNumConfigs
		};

char cfgDescriptor[] __attribute__ ((aligned (8))) =
{
/* ============== CONFIGURATION 1 =========== */
/* Configuration 1 descriptor */
0x09, // CbLength
		0x02, // CbDescriptorType
		0x22, // CwTotalLength 2 EP + Control
		0x00, 0x01, // CbNumInterfaces
		0x01, // CbConfigurationValue
		0x00, // CiConfiguration
		0xA0, // CbmAttributes Bus powered + Remote Wakeup
		0x32, // CMaxPower: 100mA

		/* Mouse Interface Descriptor Requirement */
		0x09, // bLength
		0x04, // bDescriptorType
		0x00, // bInterfaceNumber
		0x00, // bAlternateSetting
		0x01, // bNumEndpoints
		0x03, // bInterfaceClass: HID code
		0x01, // bInterfaceSubclass
		0x02, // bInterfaceProtocol: Mouse
		0x00, // iInterface

		/* HID Descriptor */
		0x09, // bLength
		0x21, // bDescriptor type: HID Descriptor Type
		0x00, // bcdHID
		0x01, 0x00, // bCountry Code
		0x01, // bNumDescriptors
		0x22, // bDescriptorType
		sizeof(mouseDescriptor), // wItemLength
		0x00,

		/* Endpoint 1 descriptor */
		0x07, // bLength
		0x05, // bDescriptorType
		((EP_REP & 1) << 7) + (EP_REP >> 1),// bEndpointAddress and direction, Endpoint Logic address!!
		0x03, // bmAttributes      INT
		0x04, // wMaxPacketSize: 3 bytes (button, x, y)
		0x00, 0x0A // bInterval
		};

char LanguagesStr[] __attribute__ ((aligned (8))) =
{
/* String descriptor 0*/
0x04, // bLength
		0x03, // bDescriptorType
		0x09, 0x04 // Language English
		};

char ManufacturerStr[] __attribute__ ((aligned (8))) =
{
		/* String descriptor 1*/
		60, // bLength
		0x03, // bDescriptorType
		'P', 0, 'h', 0, 'i', 0, 'l', 0, 'i', 0, 'p', 0, ' ', 0, 'S', 0, 'e', 0, 'm', 0, 'i', 0, 'c', 0, 'o', 0, 'n', 0, 'd', 0, 'u', 0, 'c', 0, 't',
		0, 'o', 0, 'r', 0, 's', 0, ' ', 0, 'L', 0, 'P', 0, 'C', 0, '2', 0, '1', 0, '4', 0, '8', 0 };

char ProductStr[] __attribute__ ((aligned (8))) =
{
		/* String descriptor 2*/
		98, // bLength
		0x03, // bDescriptorType
		'E', 0, 'm', 0, 'b', 0, 'e', 0, 'd', 0, 'd', 0, 'e', 0, 'd', 0, ' ', 0, 'A', 0, 'r', 0, 't', 0, 'i', 0, 's', 0, 't', 0, 's', 0, ' ', 0, 'Q',
		0, 'u', 0, 'i', 0, 'c', 0, 'k', 0, 'S', 0, 't', 0, 'a', 0, 'r', 0, 't', 0, ' ', 0, 'B', 0, 'o', 0, 'a', 0, 'r', 0, 'd', 0, ' ', 0, '-', 0,
		' ', 0, 't', 0, 'e', 0, 's', 0, 't', 0, ' ', 0, 'p', 0, 'r', 0, 'o', 0, 'g', 0, 'r', 0, 'a', 0, 'm', 0 };

const char * const StrPoin[] __attribute__ ((aligned (8))) =
{ LanguagesStr, ManufacturerStr, ProductStr };

/*************************************************************************
 * Function Name: HID_Intr
 * Parameters: none
 * Return: none
 * Description: HID Init
 *
 *************************************************************************/
void HID_Init (void)
{
	currentConfiguration = 0;
	currentAddress = 0;
	// Realize I/O Ctrl Endpoint
	USB_RealizeEndPoint (CTRL_ENP_OUT, MAX_CTRL_EP_PK_SIZE, USB_LowPrior, 1);
	USB_RealizeEndPoint (CTRL_ENP_IN, MAX_CTRL_EP_PK_SIZE, USB_LowPrior, 0);
	// Set dafault Address
	USB_SetAdd (currentAddress);
	/* Set the device status, making the soft connect active */
	USB_Cmd (CMD_USB_SET_DEV_STAT, 1);
}

/*************************************************************************
 * Function Name: HID_GetConfiguration
 * Parameters: none
 * Return: unsigned char
 * Description: Return current device configuration
 *
 *************************************************************************/
unsigned char HID_GetConfiguration (void)
{
	return (currentConfiguration);
}

/*************************************************************************
 * Function Name: HID_CallBack
 * Parameters: none
 * Return: none
 * Description: HID callback subroutine
 *
 *************************************************************************/
void HID_CallBack (void)
{
	if (DEV_INT_STAT & USB_ErrorInterrupt)
	{
		// Error interrupt
		//printf("\nUSB error irq\n");
		USB_Cmd (CMD_USB_RD_ERROR_STAT, 0);
		DEV_INT_CLR = USB_ErrorInterrupt;
	}

	if (DEV_INT_STAT & USB_DevStatusInterrupt)
	{
		// Device status interrupt
		int USB_DevIntrStaus = USB_Cmd (CMD_USB_GET_DEV_STAT, 0);
		// Bus reset
		if (USB_DevIntrStaus & USB_BusReset)
		{
			//printf("\nUSB bus reset\n");
			HID_Init ();
		}
		DEV_INT_CLR = USB_DevStatusInterrupt;
	}

	if (DEV_INT_STAT & USB_FastInterrupt)
	{
		DEV_INT_CLR = USB_FastInterrupt;
	}

	if (DEV_INT_STAT & USB_SlowInterrupt)
	{
		// Slow interrupt
		if (EP_INT_STAT & (1 << CTRL_ENP_OUT))
		{
			// Select And Clear interrupt
			HID_ImplSetupPacket ();
			EP_INT_CLR = 1 << CTRL_ENP_OUT;
		}
		DEV_INT_CLR = USB_SlowInterrupt;
	}
}

/*************************************************************************
 * Function Name: HID_CtrlStall
 * Parameters: none
 * Return: none
 * Description: Stalls Cotrol Endpoint
 *
 *************************************************************************/
void HID_CtrlStall (void)
{
	USB_Cmd (CMD_USB_SET_EP_STAT + CTRL_ENP_OUT, 0x80);
}

/*************************************************************************
 * Function Name: HID_SendDataByCtrlEP
 * Parameters: none
 * Return: none
 * Description: Send zero length size packet
 *
 *************************************************************************/
void HID_SendZeroLengthByCtrlEP (void)
{
	//EA  USBCTRL_bit.LOG_ENDPOINT = 0;
	//EA  USBCTRL_bit.WR_EN = 1;
	USB_CTRL = (USB_CTRL & 0x01) | 0x02;
	TX_PLENGTH = 0;
	do
	{
		TX_DATA = 0;
	}
	//EA  while (USBCTRL_bit.WR_EN);
	while (USB_CTRL & 0x02);

	// Validate buffer
	USB_ValidateBuffer (CTRL_ENP_IN);

	// Wait for interrupt by EP0 IN
	while ((EP_INT_STAT & (1 << CTRL_ENP_IN)) == 0)
	{
		if ((USB_Cmd (CMD_USB_SET_EP_STAT + CTRL_ENP_IN, 0) & 1) == 0)
		{
			if ((EP_INT_STAT & (1 << CTRL_ENP_IN)) == 0)
			{
				break;
			}
		}
	}
	// Clear Endpoint inerrupt (in USB clock domain)
	USB_Cmd (CMD_USB_SEL_CLR_INT_EP + CTRL_ENP_IN, 0);
	// Clear interrupt
	EP_INT_CLR = (1 << CTRL_ENP_IN);
}

/*************************************************************************
 * Function Name: HID_SendDataByCtrlEP
 * Parameters: none
 * Return: none
 * Description: Send data by Cotrol Endpoint
 *
 *************************************************************************/
void HID_SendDataByCtrlEP (unsigned char * Data, unsigned int DataSize)
{
	unsigned int * pData = (unsigned int *) Data;
	unsigned int i;
	int ZerroLength = 0;

	do
	{
		if (DataSize == MAX_CTRL_EP_PK_SIZE)
		{
			++ZerroLength;
		}
		i = MIN(DataSize,MAX_CTRL_EP_PK_SIZE);
		DataSize -= i;
		// Select Endpoint
		//EA    USBCTRL_bit.LOG_ENDPOINT = 0;
		//EA    USBCTRL_bit.WR_EN = 1;
		USB_CTRL = (USB_CTRL & 0x01) | 0x02;

		TX_PLENGTH = i;

		do
		{
			TX_DATA = *pData++;
		} while (USB_CTRL & 0x02);
		//EA    while (USBCTRL_bit.WR_EN);

		// Validate buffer
		USB_ValidateBuffer (CTRL_ENP_IN);
		// Wait until data is send or overwrite by setup packet
		while ((EP_INT_STAT & (1 << CTRL_ENP_IN)) == 0)
		{
			if ((USB_Cmd (CMD_USB_SET_EP_STAT + CTRL_ENP_IN, 0) & 1) == 0)
			{
				if ((EP_INT_STAT & (1 << CTRL_ENP_IN)) == 0)
				{
					USB_Cmd (CMD_USB_SEL_CLR_INT_EP + CTRL_ENP_IN, 0);
					return;
				}
			}
		}

		USB_Cmd (CMD_USB_SEL_CLR_INT_EP + CTRL_ENP_IN, 0);
		EP_INT_CLR = (1 << CTRL_ENP_IN);
	} while (DataSize);
	if (ZerroLength)
	{
		HID_SendZeroLengthByCtrlEP ();
	}
}

/*************************************************************************
 * Function Name: HID_ImplSetupPacket
 * Parameters: none
 * Return: none
 * Description: Setup packet implementation
 *
 *************************************************************************/
void HID_ImplSetupPacket (void)
{
	static USB_SETUP_PACKET_DEF Setup __attribute__ ((aligned (8)));
	static unsigned short Status, Length __attribute__ ((aligned (8)));
	int i = 0;
	do
	{
		// Read from CTRL Endpoint (0 - out)
		//EA    USBCTRL_bit.LOG_ENDPOINT = 0;
		//EA    USBCTRL_bit.RD_EN = 1;
		USB_CTRL = (USB_CTRL & 0x02) | 0x01;

		// Wait Packet ready flag
		//EA    while (RCVEPKTLEN_bit.PKT_RDY == 0);
		while ((RX_PLENGTH & 0x800) == 0)
			;
		// Read data from USB FIFO
		// Get size of packet
		//EA    Length = RCVEPKTLEN_bit.PKT_LNGTH;
		Length = RX_PLENGTH & 0x3ff;
		//EA    while (RCVEPKTLEN_bit.DV)
		while (RX_PLENGTH & 0x400)
		{
			//EA      Setup.Data[i++] = RCVEDATA;
			Setup.Data[i++] = RX_DATA;
		}
		// Clear Buffer Full flag
		i = USB_ClearBuffer (CTRL_ENP_OUT);
		USB_Cmd (CMD_USB_SEL_CLR_INT_EP + CTRL_ENP_OUT, 0);
	} while (i);

	if (Length == 0)
		return;

	switch (Setup.SetupRecuest)
	{
	case USB_GET_DESCRIPTOR:
		//printf("\nUSB get descriptor\n");

		if (Setup.DataFields.Value == 0x100) // Return Device Descriptor
		{
			HID_SendDataByCtrlEP ((unsigned char *) devDescriptor, MIN(sizeof(devDescriptor), Setup.DataFields.Length));
		} else if (Setup.DataFields.Value == 0x200) // Return Configuration Descriptor
		{
			HID_SendDataByCtrlEP ((unsigned char *) cfgDescriptor, MIN(sizeof(cfgDescriptor), Setup.DataFields.Length));
		} else if ((Setup.DataFields.Value & 0xFF00) == 0x300) // Return String
		{
			if ((Setup.DataFields.Value & 0xF) < 3)
			{
				const char * Data = StrPoin[Setup.DataFields.Value & 0xF];
				HID_SendDataByCtrlEP ((unsigned char *) Data, MIN(*Data, Setup.DataFields.Length));
			} else
			{
				HID_CtrlStall ();
			}
		} else
		{
			HID_CtrlStall ();
		}
		break;
	case USB_SET_ADD:
		//printf("\nUSB set address\n");
		HID_SendZeroLengthByCtrlEP ();
		currentAddress = Setup.DataFields.Value;
		USB_SetAdd (currentAddress);
		break;
	case USB_SET_CONFIGURATION:
		//printf("\nUSB set configuration\n");
		if (Setup.DataFields.Value == 1)
		{
			currentConfiguration = Setup.DataFields.Value;
			USB_RealizeEndPoint (EP_REP, 4, USB_LowPrior, 0);
			USB_Cmd (CMD_USB_CFG_DEV, 1);
			HID_SendZeroLengthByCtrlEP ();
		} else
		{
			// Enable only Endpoint 0
			USB_Cmd (CMD_USB_CFG_DEV, 0);
			HID_SendZeroLengthByCtrlEP ();
		}
		break;
	case USB_GET_CONFIGURATION:
		//printf("\nUSB get configuration\n");
		HID_SendDataByCtrlEP (&currentConfiguration, sizeof(currentConfiguration));
		break;
	case USB_GET_STATUS_DEV:
	case USB_GET_STATUS_INTERFACE:
		//printf("\nUSB get status\n");
		Status = 0;
		HID_SendDataByCtrlEP ((unsigned char *) &Status, sizeof(Status));
		break;
	case USB_GET_STATUS_ENDPOINT:
		//printf("\nUSB get status endpoint\n");
		Status = 0;
		if (currentConfiguration && (Setup.DataFields.Index <= 1))
		{
			HID_SendDataByCtrlEP ((unsigned char *) &Status, sizeof(Status));
		} else if (currentAddress && (Setup.DataFields.Index == 0))
		{
			HID_SendDataByCtrlEP ((unsigned char *) &Status, sizeof(Status));
		} else
		{
			HID_CtrlStall ();
		}
		break;
	case USB_SET_FEATURE_ENDPOINT:
		//printf("\nUSB set feature endpoint\n");
		if (Setup.DataFields.Index <= 1)
		{
			HID_SendZeroLengthByCtrlEP ();
		} else
		{
			HID_CtrlStall ();
		}
		break;
	case USB_CLEAR_FEATURE_ENDPOINT:
		//printf("\nUSB clear feature endpoint\n");
		if ((Setup.DataFields.Value == 0) && (Setup.DataFields.Index == 1))
		{
			HID_SendZeroLengthByCtrlEP ();
		} else
		{
			HID_CtrlStall ();
		}
		break;
	case USB_SET_FEATURE_INTERFACE:
	case USB_CLEAR_FEATURE_INTERFACE:
	case USB_HID_SET_IDLE:
		HID_SendZeroLengthByCtrlEP ();
		break;
	case USB_GET_HID_DESCRIPTOR:
		//printf("\nUSB get hid descriptor\n");
		HID_SendDataByCtrlEP ((unsigned char*) mouseDescriptor, MIN(sizeof(mouseDescriptor), Setup.DataFields.Length));
		break;
	case USB_SET_FEATURE_DEV:
	case USB_CLEAR_FEATURE_DEV:
	case USB_SET_DESCRIPTOR:
	case USB_GET_INTERFACE:
	case USB_SET_INTERFACE:
	default:
		//printf("\nUSB yyy\n");
		HID_CtrlStall ();
		break;
	}
}

/*************************************************************************
 * Function Name: HID_SendReport
 * Parameters: char button, char x, char y
 * Return: none
 * Description: Send data by Cotrol Endpoint
 *
 *************************************************************************/
void HID_SendReport (char button, char x, char y)
{
	unsigned int Data = button + (x << 8) + (y << 16);
	unsigned int failsafe;

	//EA  USBCTRL_bit.LOG_ENDPOINT = EP_REP >> 1;
	//EA  USBCTRL_bit.WR_EN = 1;
	USB_CTRL = (USB_CTRL & 0x01) | 0x02 | (EP_REP << 1);

	//EA  TPKTLEN = 3;
	TX_PLENGTH = 3;
	//EA  TDATA = Data;
	TX_DATA = Data;
	//EA  if (USBCTRL_bit.WR_EN)
	if (USB_CTRL & 0x02)
	{
#ifndef NDEBUG
		printf("Error Send Data");
#endif
		return;
	}

	// Validate buffer
	USB_ValidateBuffer (EP_REP);

	// Wait to complete of transmition
	failsafe = 0;
	while ((EP_INT_STAT & (1 << EP_REP)) == 0)
	{
		// Get enmdpoint status
		if ((USB_Cmd (CMD_USB_SET_EP_STAT + EP_REP, 0) & 1) == 0)
		{
			if ((EP_INT_STAT & (1 << EP_REP)) == 0)
			{
				break;
			}
		}
		//failsafe++;
		//if (failsafe > 10000)
		//  break;

		//printf("\r"); //effective!!!

	}
	USB_Cmd (CMD_USB_SEL_CLR_INT_EP + EP_REP, 0);
	EP_INT_CLR = (1 << EP_REP);
}
