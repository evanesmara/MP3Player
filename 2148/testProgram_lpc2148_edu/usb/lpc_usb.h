#ifndef __LPC_USB_H
#define __LPC_USB_H

/*************************************************************************
 *
**************************************************************************/


typedef enum
{
  USB_OK = 0,USB_PLL_ERROR,USB_INTR_ERROR,
} USB_ERROR_CODES_DEF;

typedef enum
{
  CTRL_ENP_OUT=0, CTRL_ENP_IN,
  ENP1_OUT      , ENP1_IN    ,
  ENP2_OUT      , ENP2_IN    ,
  ENP3_OUT      , ENP3_IN    ,
  ENP4_OUT      , ENP4_IN    ,
  ENP5_OUT      , ENP5_IN    ,
  ENP6_OUT      , ENP6_IN    ,
  ENP7_OUT      , ENP7_IN    ,
  ENP8_OUT      , ENP8_IN    ,
  ENP9_OUT      , ENP9_IN    ,
  ENP10_OUT     , ENP10_IN   ,
  ENP11_OUT     , ENP11_IN   ,
  ENP12_OUT     , ENP12_IN   ,
  ENP13_OUT     , ENP13_IN   ,
  ENP14_OUT     , ENP14_IN   ,
  ENP15_OUT     , ENP15_IN   ,
} USB_ENDOPINT_DEF;

typedef enum
{
  USB_LowPrior = 0,
  USB_HighPrior,
} USB_INT_PRIORITY_DEF;

typedef enum
{
  USB_NotFast = 0,
  USB_EP_Fast,
  USB_FrameFast,
} USB_DEV_INT_PRIORITY_DEF;

#define CMD_USB_SEL_EP              0x00
#define CMD_USB_SEL_CLR_INT_EP      0x40
#define CMD_USB_SET_EP_STAT         0x140
#define CMD_USB_SET_ADDRESS         0xD0
#define CMD_USB_CFG_DEV             0xD8
#define CMD_USB_CLR_BUF             0xF2
#define CMD_USB_SET_MODE            0xF3
#define CMD_USB_RD_FRAME_NUMB       0xF5
#define CMD_USB_VAL_BUF             0xFA
#define CMD_USB_RD_ERROR_STAT       0xFB
#define CMD_USB_RD_TEST_REG         0xFD
#define CMD_USB_SET_DEV_STAT        0x1FE
#define CMD_USB_GET_DEV_STAT        0xFE
#define CMD_USB_GET_ERROR           0xFF

#define USB_CMD_WR                  0x00000500
#define USB_DATA_WR                 0x00000100
#define USB_DATA_RD                 0x00000200

#define USB_FrameInterrupt          0x00000001
#define USB_FastInterrupt           0x00000002
#define USB_SlowInterrupt           0x00000004
#define USB_DevStatusInterrupt      0x00000008
#define USB_CommRegEmptyInterrupt   0x00000010
#define USB_CommDataFullInterrupt   0x00000020
#define USB_RxPacketInterrupt       0x00000040
#define USB_TxPacketInterrupt       0x00000080
#define USB_EPRealizeInterrupt      0x00000100
#define USB_ErrorInterrupt          0x00000200

#define USB_Connect                 0x00000001
#define USB_ConnectChange           0x00000002
#define USB_Suspend                 0x00000004
#define USB_SuspendChange           0x00000008
#define USB_BusReset                0x00000010


typedef void(* Fnpr)(void);

unsigned int USB_Cmd (unsigned short Command, unsigned char Data);

USB_ERROR_CODES_DEF USB_Init(int IntrSlot,Fnpr UserIntrCallback,USB_DEV_INT_PRIORITY_DEF DevIntrPrio);

void USB_RealizeEndPoint(USB_ENDOPINT_DEF EndPoint,int MaxPacketSize,
                         USB_INT_PRIORITY_DEF IntrPriority, int IntrEna);

unsigned int USB_ClearBuffer(USB_ENDOPINT_DEF EndPoint);
void USB_ValidateBuffer(USB_ENDOPINT_DEF EndPoint);
void USB_SetAdd(unsigned int DevAdd);
void USB_ISR (void);

#endif //__LPC_USB_H
