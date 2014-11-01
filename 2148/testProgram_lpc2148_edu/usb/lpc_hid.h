#ifndef __LPC_HID_H
#define __LPC_HID_H

/*************************************************************************
 *
**************************************************************************/
#include "lpc_usb.h"

#define USB_CLEAR_FEATURE_DEV       0x0100
#define USB_CLEAR_FEATURE_INTERFACE 0x0101
#define USB_CLEAR_FEATURE_ENDPOINT  0x0102
#define USB_GET_CONFIGURATION       0x0880
#define USB_GET_DESCRIPTOR          0x0680
#define USB_GET_INTERFACE           0x1081
#define USB_GET_STATUS_DEV          0x0080
#define USB_GET_STATUS_INTERFACE    0x0081
#define USB_GET_STATUS_ENDPOINT     0x0082
#define USB_SET_ADD                 0x0500
#define USB_SET_CONFIGURATION       0x0900
#define USB_SET_DESCRIPTOR          0x0700
#define USB_SET_FEATURE_DEV         0x0300
#define USB_SET_FEATURE_INTERFACE   0x0301
#define USB_SET_FEATURE_ENDPOINT    0x0302
#define USB_SET_INTERFACE           0x1101
#define USB_SYNC_FRAME              0x1282

#define USB_GET_HID_DESCRIPTOR      0x0681
#define USB_HID_SET_IDLE            0x0A21
#define USB_HID_GET_IDLE            0x02A1

#define EP_REP                      ENP1_IN // Phisyc report endpoint
#define MAX_CTRL_EP_PK_SIZE         8       // Size of maximum packet for the control endpoin

void HID_Init (void);
unsigned char HID_GetConfiguration (void);
void HID_CallBack (void);
void HID_CtrlStall (void);
void HID_SendDataByCtrlEP (unsigned char * pData, unsigned int DataSize);
void HID_ImplSetupPacket (void);
void HID_SendReport(char button, char x, char y);

#endif //__LPC_HID_H
