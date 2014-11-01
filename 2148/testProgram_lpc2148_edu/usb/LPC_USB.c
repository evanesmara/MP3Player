/*************************************************************************
 *
**************************************************************************/
#include <lpc2xxx.h>
#include <printf_P.h>
#include "lpc_usb.h"

#define USB_INTS         (*((volatile unsigned long *) 0xE01FC1C0))
#define PLLCON_PLLE         (1<<0)      /* PLL Enable */
#define PLLCON_PLLC         (1<<1)      /* PLL Connect */
#define PLLSTAT_PLOCK       (1<<10)     /* PLL Lock Status */

#define tSR unsigned int    /* define the type for status words */
#define m_os_dis_int()                                    {localSR = halDisableInterrupts(); asm volatile (" nop");}
#define m_os_ena_int()                                    {halRestoreInterrupts(localSR); asm volatile (" nop");}


//void USB_ISR (void)__attribute__ ((interrupt));

Fnpr IntrCallback;
/*****************************************************************************
 *
 * Description:
 *    Disable interrupts 
 *
 * Returns:
 *    The current status register, before disabling interrupts. 
 *
 ****************************************************************************/
unsigned int
halDisableInterrupts(void)
{
  tSR returnReg;

  asm volatile ("0: mrs %0, cpsr  \n\t"
                "orr r1, %0, #0xC0                           \n\t"
                "msr cpsr_c, r1                              \n\t"
                "mrs r1, cpsr                                \n\t"
                "and r1, r1, #0xC0                           \n\t"
                "cmp r1, #0xC0                               \n\t"
                "bne 0            \n\t"
                : "=r"(returnReg)
                :
                : "r1"
               );
  return returnReg;
}

/*****************************************************************************
 *
 * Description:
 *    Enable interrupts. 
 *
 ****************************************************************************/
void
halEnableInterrupts(void)
{
  asm volatile ("mrs r3, cpsr       \n\t"                          
                "bic r3, r3, #0xC0  \n\t"                      
                "msr cpsr, r3       \n\t"                           
                :                                       
                :                                       
                : "r3"                                  
               );
}

/*****************************************************************************
 *
 * Description:
 *    Restore interrupt state. 
 *
 * Params:
 *    [in] restoreValue - The value of the new status register. 
 *
 ****************************************************************************/
void
halRestoreInterrupts(unsigned int restoreValue)
{
  asm volatile ("msr cpsr_c, %0  \n\t"
                :
                : "r" (restoreValue)
                : "r1"
               );
}

/*************************************************************************
 * Function Name: USB_Cmd
 * Parameters:  unsigned char Command
 *              unsigned char Data
 * Return: unsigned int - command result
 * Description: Implement a command transmit to USB Engine
 *
 *************************************************************************/
unsigned int USB_Cmd (unsigned short Command, unsigned char Data)
{
//EA unsigned int cpu_sr, tmp = 0;
unsigned int localSR, tmp = 0;
  // Disable interrurp and save current state of the interrupt flags
//EA  cpu_sr = disable_interrupts();
m_os_dis_int();

  // Clear Data reg full and Command reg. empry interrupts
  DEV_INT_CLR = USB_CommRegEmptyInterrupt + USB_CommDataFullInterrupt;

  // Load commonad in USB engine
  CMD_CODE = (((unsigned int)Command&0xFF) << 16) + USB_CMD_WR;

  // Wait until command is accepted
//EA  while ((DEVINTS & USB_CommRegEmptyInterrupt) == 0);
  while ((DEV_INT_STAT & USB_CommRegEmptyInterrupt) == 0);

  // clear Command reg. empry interrupt
  DEV_INT_CLR = USB_CommRegEmptyInterrupt;

  // determinate next phase of the command
  switch (Command)
  {
  case CMD_USB_SET_ADDRESS:
  case CMD_USB_CFG_DEV:
  case CMD_USB_SET_MODE:
  case CMD_USB_SET_DEV_STAT:
    CMD_CODE = ((unsigned int)Data << 16) + USB_DATA_WR;
    while ((DEV_INT_STAT & USB_CommRegEmptyInterrupt) == 0);
    break;
  case CMD_USB_RD_FRAME_NUMB:
  case CMD_USB_RD_TEST_REG:
    CMD_CODE = ((unsigned int)Command << 16) + USB_DATA_RD;
    while ((DEV_INT_STAT & USB_CommDataFullInterrupt) == 0);
    DEV_INT_CLR = USB_CommDataFullInterrupt;
    tmp = CMD_DATA;
    CMD_CODE = ((unsigned int)Command << 16) + USB_DATA_RD;
    while ((DEV_INT_STAT & USB_CommDataFullInterrupt) == 0);
    DEV_INT_CLR = USB_CommDataFullInterrupt;
    tmp |= CMD_DATA << 8;
    break;
  case CMD_USB_GET_DEV_STAT:
  case CMD_USB_GET_ERROR:
  case CMD_USB_RD_ERROR_STAT:
  case CMD_USB_CLR_BUF:
    CMD_CODE = ((unsigned int)Command << 16) + USB_DATA_RD;
    while ((DEV_INT_STAT & USB_CommDataFullInterrupt) == 0);
    tmp = CMD_DATA;
    break;
  default:
    switch (Command & 0xE0)
    {
    case CMD_USB_SEL_EP:
    case CMD_USB_SEL_CLR_INT_EP:
      CMD_CODE = ((unsigned int)Command << 16) + USB_DATA_RD;
      while ((DEV_INT_STAT & USB_CommDataFullInterrupt) == 0);
      tmp = CMD_DATA;
      break;
    case CMD_USB_SET_EP_STAT:
      CMD_CODE = ((unsigned int)Data << 16) + USB_DATA_WR;
      while ((DEV_INT_STAT & USB_CommRegEmptyInterrupt) == 0);
      break;
    }
    break;
  }
  DEV_INT_CLR = USB_CommRegEmptyInterrupt + USB_CommDataFullInterrupt;

  // restore the interrupt flags
//EA  restore_interrupts(cpu_sr);
m_os_ena_int();

  return(tmp);
}

/*************************************************************************
 * Function Name: USB_Init
 * Parameters: int IntrSlot
 *             Fnpr UserIntrCallback
 *             USB_DEV_INT_PRIORITY_DEF DevIntrPrio
 * Return: USB_ERROR_CODE_DEF
 *         USB_OK  0: Pass
 *     USB_ERROR   1: Error
 * Description: Init USB
 *
 *************************************************************************/
USB_ERROR_CODES_DEF USB_Init(int IntrSlot, Fnpr UserIntrCallback,USB_DEV_INT_PRIORITY_DEF DevIntrPrio)
{
//EAint p = 1, m = 48000000 / FOSC, Fpll, i;
//EAint i;

//EA  if (IntrSlot > VIC_Slot15)
//EA  {
//EA    return(USB_INTR_ERROR);
//EA  }

  /* Turn on USB */
//EA  PM_OpenPeripheral(0x80000000);
PCONP |= 0x80000000;                      /* Turn On USB PCLK */

  // Enable Vbus sense
//EA  PINSEL1_bit.P0_23 = 1;
  // 1 - GoodLinc
  // 2 - ~Connect
//EA  PINSEL1_bit.P0_31 = 2;
  PINSEL1  |= 0x80004000;


  // Init Pll for USB engine freq - 48MHz
#if 0 //EA
  PLL48CFG_bit.MSEL = m-1;      //Fosc is define in LPC2148_sys_cnfg.h into [Hz]
  PLL48CFG_bit.PSEL = i;

  PLL48CON_bit.PLLE = true;     // Enable PLL
  PLL48FEED = PLLFEED_DATA1;
  PLL48FEED = PLLFEED_DATA2;

  while (!PLL48STAT_bit.PLOCK); // Wait PLL lock

  PLL48CON_bit.PLLC = true;     // Connect PLL
  PLL48FEED = PLLFEED_DATA1;
  PLL48FEED = PLLFEED_DATA2;
#endif //EA
  PLL48CFG  = 0x23;                         /* M = 4, P = 2 */
  PLL48CON  = PLLCON_PLLE;                  /* PLL Enable */
  PLL48FEED = 0xAA;                         /* Feed Sequence 1 */
  PLL48FEED = 0x55;                         /* Feed Sequence 2 */
    
  while ((PLL48STAT & PLLSTAT_PLOCK) == 0); /* Wait for PLL Lock */

  PLL48CON  = PLLCON_PLLE | PLLCON_PLLC;    /* PLL Enable & Connect */
  PLL48FEED = 0xAA;                         /* Feed Sequence 1 */
  PLL48FEED = 0x55;                         /* Feed Sequence 2 */

  IntrCallback = UserIntrCallback;

  // USB interrupt connect to VIC
//EA  VIC_SetVectoredIRQ(USB_ISR,(LPC_VicIrqSlots_t)IntrSlot,VIC_USB);
//EA  VIC_EnableInt(1<<VIC_USB);
  VICVectAddr0 = (unsigned long)USB_ISR;    /* USB Interrupt -> Vector 0 */
  VICVectCntl0 = 0x20 | 22;                 /* USB Interrupt -> IRQ Slot 0 */
  VICIntEnable = 1 << 22;                   /* Enable USB Interrupt */

  // init USB engine
  DEV_INT_PRIO   = DevIntrPrio;

  // EndPoint Init
  REALIZE_EP = EP_INT_EN = 0x3;
  EP_INT_CLR  = 0xFFFFFFFF;

  // Clear all interrupt flags
  DEV_INT_CLR = 0x3FF;

  // Eanble all interrupts
  // without EP Realize, Command Reg Empty, Command Data Reg Full
  DEV_INT_EN = 0x20E;

  // Enable USB inerrupt
//EA  USBINTS_bit.EN_USB_INTS = 1;
  USB_INTS = 0x80000000;
//EA

  return (USB_OK);
}

/*************************************************************************
 * Function Name: USB_RealizeEndPoint
 * Parameters: USB_ENDOPINT_DEF EndPoint - endpoiunt index
 *             int MaxPacketSize - When is 0 disable relevant endpoint
 *             USB_INT_PRIORITY_DEF IntrPriority
 *             int IntrEna
 * Return: none
 * Description: Enable or disable endpoint
 *
 *************************************************************************/
void USB_RealizeEndPoint( USB_ENDOPINT_DEF EndPoint,int MaxPacketSize,
                          USB_INT_PRIORITY_DEF IntrPriority, int IntrEna)
{
unsigned int Mask = (1 << EndPoint);
  if (MaxPacketSize)
  {
    // Realize endpoit
    REALIZE_EP  |= Mask;
    // Set endpoit maximum packet size
    EP_INDEX       = EndPoint;
    MAXPACKET_SIZE   = MaxPacketSize;
    // Wait for Realize complete
    while ((DEV_INT_STAT & USB_EPRealizeInterrupt) == 0);
    // Clear Endpoint Realize interrupt flag
    DEV_INT_CLR     = USB_EPRealizeInterrupt;
    // Set endpoint interrupt priority
    if (IntrPriority)
    {
      EP_INT_PRIO |= Mask;
    }
    // Endpoint interrup flag clear and CDFULL flag!!!
    EP_INT_CLR    = Mask;
    // Enable endpoint interrup
    if (IntrEna) EP_INT_EN |= Mask;
  }
  else
  {
    Mask =~ Mask;
    // Disable relevant endpoint and interrupt
    REALIZE_EP &= Mask;
    EP_INT_EN &= Mask;
  }
}

/*************************************************************************
 * Function Name: USB_ClearBuffer
 * Parameters: USB_ENDOPINT_DEF EndPoint - endpoiunt index
 * Return: Packet overwrite statrus
 * Description: Clear buffer of the corresponding endpoint
 *
 *************************************************************************/
unsigned int USB_ClearBuffer(USB_ENDOPINT_DEF EndPoint)
{
  // Select endpoint
  USB_Cmd(CMD_USB_SEL_EP | EndPoint,0);
  // Clear selected end point
  return(USB_Cmd(CMD_USB_CLR_BUF,0));
}

/*************************************************************************
 * Function Name: USB_ValidateBuffer
 * Parameters: USB_ENDOPINT_DEF EndPoint - endpoiunt index
 * Return: none
 * Description: Validate buffer only of IN Endpoint
 *
 *************************************************************************/
void USB_ValidateBuffer(USB_ENDOPINT_DEF EndPoint)
{
  if (EndPoint & 1)
  {
    USB_Cmd(CMD_USB_SEL_EP | EndPoint,0);
    USB_Cmd(CMD_USB_VAL_BUF,0);
  }
}

/*************************************************************************
 * Function Name: USB_SetAdd
 * Parameters: unsigned int DevAdd - defice address between 0 - 127
 * Return: none
 * Description: Set device address
 *
 *************************************************************************/
void USB_SetAdd(unsigned int DevAdd)
{
	int i;
  for(i=2;i;--i)
  {
    USB_Cmd(CMD_USB_SET_ADDRESS,DevAdd | 0x80);
  }
}

/*************************************************************************
 * Function Name: USB_ISR
 * Parameters: none
 * Return: none
 * Description: USB interrupt subroutine
 *
 *************************************************************************/
void USB_ISR (void)
{
  IntrCallback();
  VICVectAddr = 0;    // Clear interrupt in VIC.
}
