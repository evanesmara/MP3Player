#include "../startup/lpc2xxx.h"
#include "lcd_hw.h"

/*****************************************************************************
 *
 * Description:
 *    Send 9-bit data to LCD controller
 *
 ****************************************************************************/
void sendToLCD (uint8_t firstBit, uint8_t data)
{
	//disable SPI
	IOCLR0 = LCD_CLK;
	PINSEL0 &= 0xffffc0ff;

	if (1 == firstBit)
	{
		IOSET0 = LCD_MOSI; //set MOSI
	}
	else
	{
		IOCLR0 = LCD_MOSI; //reset MOSI
	}

	//Set clock high
	IOSET0 = LCD_CLK;

	//Set clock low
	IOCLR0 = LCD_CLK;

	/*
	 * Enable SPI again
	 */
	//initialize SPI interface
	S0SPCCR = 0x08;
	S0SPCR = 0x20;

	//connect SPI bus to IO-pins
	PINSEL0 |= 0x00001500;

	//send byte
	S0SPDR = data;
	while ((S0SPSR & 0x80) == 0)
	{
		// Nic nie rób.
	}
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the SPI interface for the LCD controller
 *
 ****************************************************************************/
void initSpiForLcd (void)
{
	//make SPI slave chip select an output and set signal high
	IODIR0 |= (LCD_CS | LCD_CLK | LCD_MOSI);

	//deselect controller
	selectLCD (FALSE);

	//connect SPI bus to IO-pins
	PINSEL0 |= 0x00001500;

	//initialize SPI interface
	S0SPCCR = 0x08;
	S0SPCR = 0x20;
}

/*****************************************************************************
 *
 * Description:
 *    Select/deselect LCD controller (by controlling chip select signal)
 *
 ****************************************************************************/
void selectLCD (tBool select)
{
	if (TRUE == select)
	{
		IOCLR0 = LCD_CS;
	} else
	{
		IOSET0 = LCD_CS;
	}
}
