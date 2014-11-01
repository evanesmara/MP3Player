/******************************************************************************
 *
 * Copyright:
 *    (C) 2005 Embedded Artists AB
 *
 * Description:
 *    Small test program for LPC2106 QuickStart board
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <LPC2xxx.H>
#include "general.h"
#include "eeprom.h"
#include <printf_P.h>
#include <ea_init.h>
#include <string.h>

#include "usb/lpc_usb.h"
#include "usb/lpc_hid.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define MAX_LENGTH 14
#define DELAY_STEP 1000


/*****************************************************************************
 *
 * Description:
 *    Delay function that uses timer #1
 *
 ****************************************************************************/
void delay_us(unsigned int delay)
{
  //set up timer
  T1TCR = 0x02;    //reset timer
  T1TCR = 0x00;
  T1PR  = 0x00;       //count every clock cycle
  T1MR0 = delay*15;   //since each clock cycle = 1/15MHz = 67ns
  T1IR  = 0xff;       //reset all flags before enable IRQs
  T1MCR = 0x04;       //stop timer on match
  T1TCR = 0x01;       //start timer
  while(T1TCR & 0x01) //wait until bit #0 is reset
    ;
}

/*****************************************************************************
 *
 * Description:
 *    Test EEPROM
 *
 ****************************************************************************/
tU8
testEEPROM(void)
{
  tU8 eepromTestResultOK;
  tU8 testString1[] = "String #1";
  tU8 testString2[] = "sTrInG #2";
  tU8 testBuf[MAX_LENGTH];
  tS8 errorCode;
  
  eepromTestResultOK = TRUE;
  printf("\nTest #1 - write string '%s' to address 0x0000", testString1);
  errorCode = eepromWrite(0x0000, testString1, sizeof(testString1));
  if (errorCode == I2C_CODE_OK)
    printf("\n        - done (status code OK)");
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  if (eepromPoll() == I2C_CODE_OK)
    printf("\n        - program cycle completed");
  else
  {
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

  printf("\nTest #2 - write string '%s' to address 0x00a0", testString2);
  errorCode = eepromWrite(0x00a0, testString2, sizeof(testString2));
  if (errorCode == I2C_CODE_OK)
    printf("\n        - done (status code OK)");
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
   
  if (eepromPoll() == I2C_CODE_OK)
    printf("\n        - program cycle completed");
  else
  {
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

  /*
   * Read from eeprom
   */
  printf("\nTest #3 - read string from address 0x0000");
  errorCode = eepromPageRead(0x0000, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString1)-1)
      printf("\n        - string is '%s'", testBuf);
    else
    {
      printf("\n        - wrong length (read string is %d characters long)!", strlen(testBuf));
      eepromTestResultOK = FALSE;
    }
  }
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }

  printf("\nTest #4 - read string from address 0x00a0");
  errorCode = eepromPageRead(0x00a0, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString2)-1)
      printf("\n        - string is '%s'", testBuf);
    else
    {
      printf("\n        - wrong length (read string is %d characters long)!", strlen(testBuf));
      eepromTestResultOK = FALSE;
    }
  }
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }

  /*
   * Write/Read from eeprom
   */
  printf("\nTest #5 - write string '%s' to address 0x0004", testString2);
  errorCode = eepromWrite(0x0004, testString2, sizeof(testString2));
  if (errorCode == I2C_CODE_OK)
    printf("\n        - done (status code OK)");
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  if (eepromPoll() == I2C_CODE_OK)
    printf("\n        - program cycle completed");
  else
  {
    printf("\n        - program cycle failed!");
    eepromTestResultOK = FALSE;
  }

  printf("\nTest #6 - read string from address 0x0000");
  errorCode = eepromPageRead(0x0000, testBuf, MAX_LENGTH);
  if (errorCode == I2C_CODE_OK)
  {
    if (strlen(testBuf) == sizeof(testString2)-1+4)
      printf("\n        - string is '%s'", testBuf);
    else
    {
      printf("\n        - wrong length (read string is %d characters long)!", strlen(testBuf));
      eepromTestResultOK = FALSE;
    }
  }
  else
  {
    printf("\n        - failed (error code = %d)!", errorCode);
    eepromTestResultOK = FALSE;
  }
  
  return eepromTestResultOK;
}

/*****************************************************************************
 *
 * Description:
 *    Print an alternativ symbol 
 *
 ****************************************************************************/
void
printSymbol(void)
{
  static tU8 symbol = 0;
  
  switch(symbol)
  {
    case 0:  printf("\r.         "); symbol++; break;
    case 1:  printf("\r..        "); symbol++; break;
    case 2:  printf("\r...       "); symbol++; break;
    case 3:  printf("\r ...      "); symbol++; break;
    case 4:  printf("\r  ...     "); symbol++; break;
    case 5:  printf("\r   ...    "); symbol++; break;
    case 6:  printf("\r    ...   "); symbol++; break;
    case 7:  printf("\r     ...  "); symbol++; break;
    case 8:  printf("\r      ... "); symbol++; break;
    case 9:  printf("\r       ..."); symbol++; break;
    case 10: printf("\r        .."); symbol++; break;
    case 11: printf("\r         ."); symbol++; break;
    default: printf("\r          "); symbol = 0; break;
  }
}

/*****************************************************************************
 *
 * Description:
 *    
 *
 ****************************************************************************/
void waitAndCheck(void)
{
  tU32 delayTime;
  tU8  rxChar;
  static tU8 escape = FALSE;

  for(delayTime=0; delayTime<150000; delayTime+= DELAY_STEP)
  {
    delay_us(DELAY_STEP);

    /* check if character received */
    if (consolGetChar(&rxChar) == TRUE)
    {
      printf("\rReceived char: %c (%d decimal)\n", rxChar, rxChar);
      if (rxChar == 224)
        escape = TRUE;
      else if (escape == TRUE)
      {
        if (rxChar != 224)
      	  escape = FALSE;
      	switch(rxChar)
      	{
      		case 72:
            if(HID_GetConfiguration())
            {
            	printf(" ");
              HID_SendReport(0,0,-10);
            	printf(" ");
            }
            break;
      		case 77:
            if(HID_GetConfiguration())
            {
            	printf(" ");
              HID_SendReport(0,10,0);
            	printf(" ");
            }
            break;
      		case 80:
            if(HID_GetConfiguration())
            {
            	printf(" ");
              HID_SendReport(0,0,10);
            	printf(" ");
            }
            break;
      		case 75:
            if(HID_GetConfiguration())
            {
            	printf(" ");
              HID_SendReport(0,-10,0);
            	printf(" ");
            }
            break;
          default:
            break;
        }
      }
    }
  }
}

/*****************************************************************************
 *
 * Description:
 *    Main method 
 *
 ****************************************************************************/
int
main(void)
{
  /* initialize console */
  InicjalizacjaKonsoli();

  /* initialize I2C */
  i2cInit();
  
  printf("\n***************************************************");
  printf("\n*                                                 *");
  printf("\n* Test program for LPC2148 QuickStart Board       *");
  printf("\n* Version: 2.1                                    *");
  printf("\n* Date:    2007-04-24                             *");
  printf("\n* (C) Embedded Artists 2005-2007                  *");
  printf("\n*                                                 *");
  printf("\n*                                                 *");
  printf("\n* The test program emulates a mouse HID, so       *");
  printf("\n* you can move the cursor on the PC (after the    *");
  printf("\n* USB has connected) by pressen the arrow keys    *");
  printf("\n* on the keyboard.                                *");
  printf("\n* (beware, the testprogram may hang if the send   *");
  printf("\n*  function is stressed - this will be fixed      *");
  printf("\n*  in a comming update of the test program)       *");
  printf("\n*                                                 *");
  printf("\n***************************************************\n");

  /*
   * Write to eeprom
   */
  printf("\n***************************************************");
  printf("\n* EEPROM and I2C test                             *");
  printf("\n***************************************************");

  if (testEEPROM() == TRUE)
    printf("\n\nSummary of tests: Passed all tests!");
  else
    printf("\n\nSummary of tests: Failed at least one test!");

  //
  //Test the RTC (Real-Time Clock)
  //
  printf("\n***************************************************");
  printf("\n* RTC test                                        *");
  printf("\n***************************************************\n");
  
  CCR  = 0x00000012;
  CCR  = 0x00000010;
  ILR  = 0x00000000;
  CIIR = 0x00000000;
  AMR  = 0x00000000;
  //PREINT  = 449;
  //PREFRAC = 0;
  delay_us(500000);
  SEC  = 0;
  MINx  = 0;
  HOUR = 0;
  delay_us(500000);
  CCR  = 0x00000011;
  
  delay_us(500000);
  printf(".");
  delay_us(500000);
  printf(".");
  delay_us(500000);
  printf(".");
  delay_us(500000);
  printf(".");
  delay_us(500000);
  printf(".");
  delay_us(500000);
  printf(".");
  delay_us(200000);
  
  if ((SEC == 3) && (MINx == 0) && (HOUR == 0))
    printf(" test OK!\n");
  else
  {
    printf(" test FAILED! [%d:%d:%d]\n", SEC, MINx, HOUR);
  }

  /*
   * I/O and UART test
   */
  printf("\n");
  printf("\n***************************************************");
  printf("\n* I/O and UART test                               *");
  printf("\n* Loop through all I/O pins (running '0')         *");
  printf("\n* - P0.4 to P0.31, P1.16 to P1.31                 *");
  printf("\n*   (not in above order, and except USB pins)     *");
  printf("\n* - P0.2 to P0.3 tested via EEPROM and I2C test   *");
  printf("\n* - P0.0 to P0.1 tested via UART test             *");
  printf("\n*                                                 *");
  printf("\n* Press any key on terminal and verify echo back  *");
  printf("\n***************************************************");
  printf("\n");

  IODIR0 = 0xfffffff0;
  IOSET0 = 0xfffffff0;
  IODIR1 = 0xffff0000;
  IOSET1 = 0xffff0000;

  // Initialize USB
  if (USB_Init(0,HID_CallBack,USB_NotFast))
    printf("\nError initializing USB\n");

  /* HID Init */
  HID_Init();

  while(1)
  {
    tU32 pattern;
    tU32 state = 0;
    

IOCLR0 = 0xfffffff0;
IOCLR1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOSET0 = 0xfffffff0;
IOSET1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOCLR0 = 0xfffffff0;
IOCLR1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOSET0 = 0xfffffff0;
IOSET1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOCLR0 = 0xfffffff0;
IOCLR1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOSET0 = 0xfffffff0;
IOSET1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOCLR0 = 0xfffffff0;
IOCLR1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
IOSET0 = 0xfffffff0;
IOSET1 = 0xffff0000;
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();
waitAndCheck();

state = 0;
while(1)
{
    switch(state)
    {
      case 0:
        IOCLR0 = 0x00000020;
        waitAndCheck();
        IOSET0 = 0x00000020;
        printSymbol();
        break;
      case 1:
        IOCLR0 = 0x00000080;
        waitAndCheck();
        IOSET0 = 0x00000080;
        printSymbol();
        break;
      case 2:
        IOCLR0 = 0x00000200;
        waitAndCheck();
        IOSET0 = 0x00000200;
        printSymbol();
        break;
      case 3:
        IOCLR0 = 0x00000800;
        waitAndCheck();
        IOSET0 = 0x00000800;
        printSymbol();
        break;
      case 4:
        IOCLR0 = 0x00002000;
        waitAndCheck();
        IOSET0 = 0x00002000;
        printSymbol();
        break;
      case 5:
        IOCLR0 = 0x00008000;
        waitAndCheck();
        IOSET0 = 0x00008000;
        printSymbol();
        break;
      case 6:
        IOCLR0 = 0x00020000;
        waitAndCheck();
        IOSET0 = 0x00020000;
        printSymbol();
        break;
      case 7:
        IOCLR0 = 0x00080000;
        waitAndCheck();
        IOSET0 = 0x00080000;
        printSymbol();
        break;
      case 8:
        IOCLR0 = 0x00200000;
        waitAndCheck();
        IOSET0 = 0x00200000;
        printSymbol();
        break;
      case 9:
        IOCLR0 = 0x00800000;
        waitAndCheck();
        IOSET0 = 0x00800000;
        printSymbol();
        break;
      case 10:
        IOCLR0 = 0x08000000;
        waitAndCheck();
        IOSET0 = 0x08000000;
        printSymbol();
        break;
      case 11:
        IOCLR1 = 0x00200000;
        waitAndCheck();
        IOSET1 = 0x00200000;
        printSymbol();
        break;
      case 12:
        IOCLR1 = 0x00800000;
        waitAndCheck();
        IOSET1 = 0x00800000;
        printSymbol();
        break;

      case 13:
        IOCLR0 = 0x00000010;
        waitAndCheck();
        IOSET0 = 0x00000010;
        printSymbol();
        break;
      case 14:
        IOCLR0 = 0x00000040;
        waitAndCheck();
        IOSET0 = 0x00000040;
        printSymbol();
        break;
      case 15:
        IOCLR0 = 0x00000100;
        waitAndCheck();
        IOSET0 = 0x00000100;
        printSymbol();
        break;
      case 16:
        IOCLR0 = 0x00000400;
        waitAndCheck();
        IOSET0 = 0x00000400;
        printSymbol();
        break;
      case 17:
        IOCLR0 = 0x00001000;
        waitAndCheck();
        IOSET0 = 0x00001000;
        printSymbol();
        break;
      case 18:
        IOCLR0 = 0x00004000;
        waitAndCheck();
        IOSET0 = 0x00004000;
        printSymbol();
        break;
      case 19:
        IOCLR0 = 0x00010000;
        waitAndCheck();
        IOSET0 = 0x00010000;
        printSymbol();
        break;
      case 20:
        IOCLR0 = 0x00040000;
        waitAndCheck();
        IOSET0 = 0x00040000;
        printSymbol();
        break;
      case 21:
        IOCLR0 = 0x00100000;
        waitAndCheck();
        IOSET0 = 0x00100000;
        printSymbol();
        break;
      case 22:
        IOCLR0 = 0x00400000;
        waitAndCheck();
        IOSET0 = 0x00400000;
        printSymbol();
        break;
      case 23:
        IOCLR0 = 0x02000000;
        waitAndCheck();
        IOSET0 = 0x02000000;
        printSymbol();
        break;
      case 24:
        IOCLR0 = 0x04000000;
        waitAndCheck();
        IOSET0 = 0x04000000;
        printSymbol();
        break;
      case 25:
        IOCLR1 = 0x00100000;
        waitAndCheck();
        IOSET1 = 0x00100000;
        printSymbol();
        break;
      case 26:
        IOCLR1 = 0x00400000;
        waitAndCheck();
        IOSET1 = 0x00400000;
        printSymbol();
        break;

      case 27:
        IOCLR1 = 0x40000000;
        waitAndCheck();
        IOSET1 = 0x40000000;
        printSymbol();
        break;
      case 28:
        IOCLR1 = 0x10000000;
        waitAndCheck();
        IOSET1 = 0x10000000;
        printSymbol();
        break;
      case 29:
        IOCLR1 = 0x04000000;
        waitAndCheck();
        IOSET1 = 0x04000000;
        printSymbol();
        break;
      case 30:
        IOCLR1 = 0x01000000;
        waitAndCheck();
        IOSET1 = 0x01000000;
        printSymbol();
        break;
      case 31:
        IOCLR0 = 0x40000000;
        waitAndCheck();
        IOSET0 = 0x40000000;
        printSymbol();
        break;
      case 32:
        IOCLR0 = 0x10000000;
        waitAndCheck();
        IOSET0 = 0x10000000;
        printSymbol();
        break;
      case 33:
        IOCLR1 = 0x00040000;
        waitAndCheck();
        IOSET1 = 0x00040000;
        printSymbol();
        break;
      case 34:
        IOCLR1 = 0x00010000;
        waitAndCheck();
        IOSET1 = 0x00010000;
        printSymbol();
        break;

      case 35:
        IOCLR1 = 0x80000000;  //
        waitAndCheck();
        IOSET1 = 0x80000000;  //
        printSymbol();
        break;
      case 36:
        IOCLR1 = 0x20000000;
        waitAndCheck();
        IOSET1 = 0x20000000;
        printSymbol();
        break;
      case 37:
        IOCLR1 = 0x08000000;
        waitAndCheck();
        IOSET1 = 0x08000000;
        printSymbol();
        break;
      case 38:
        IOCLR1 = 0x02000000;
        waitAndCheck();
        IOSET1 = 0x02000000;
        printSymbol();
        break;
      case 39:
        IOCLR0 = 0x80000000;
        waitAndCheck();
        IOSET0 = 0x80000000;
        printSymbol();
        break;
      case 40:
        IOCLR0 = 0x20000000;
        waitAndCheck();
        IOSET0 = 0x20000000;
        printSymbol();
        break;
      case 41:
        IOCLR1 = 0x00080000;
        waitAndCheck();
        IOSET1 = 0x00080000;
        printSymbol();
        break;
      case 42:
        IOCLR1 = 0x00020000;
        waitAndCheck();
        IOSET1 = 0x00020000;
        printSymbol();
        break;

      default:
        state = 0;
        break;
    }
    state++;
    if (state > 42)
      break;
}
  }
  
  return 0;
}

