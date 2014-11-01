/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "../startup/config.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD

#define AIN0 0
#define AIN1 1
#define AIN2 2
#define AIN3 3
#define AIN4 4
#define AIN5 5
#define AIN6 6
#define AIN7 7

#define VREF_IN_MV 2475

/*****************************************************************************
 *
 * Description:
 *    xxx
 *
 ****************************************************************************/
static void delay37us (void)
{
	volatile tU32 i;

	for (i = 0; i < 2500; i++)
		asm volatile (" nop");
	//delay 15 ns x 2500 = about 37 us delay
}

/*****************************************************************************
 *
 * Description:
 *    Start a conversion of one selected analogue input and return
 *    10-bit result.
 *
 * Params:
 *    [in] channel - analogue input channel to convert.
 *
 * Return:
 *    10-bit conversion result
 *
 ****************************************************************************/
static tU16 getAnalogueInput (tU8 channel)
{
	//start conversion now (for selected channel)
	ADCR = (ADCR & 0xFFFFFF00) | (1 << channel) | (1 << 24);

	//wait til done
	while ((ADDR & 0x80000000) == 0)
		;

	//get result and adjust to 10-bit integer
	return (ADDR >> 6) & 0x3FF;
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 ****************************************************************************/
void testAdc (void)
{
	tU32 integerResult;
	tU32 mvResult;
	tU16 dacValue = 0;

	printf("\n\n*******************************************************\n");
	printf("*                                                     *\n");
	printf("* The ADC will read the analogue inputs...            *\n");
	printf("*                                                     *\n");
	printf("*******************************************************\n\n");

	//set AIN1 = P0.28, AIN2 = P0.29, and AIN3 = P0.30
	//  PINSEL1 &= ~0x3f0c0000;
	//  PINSEL1 |=  0x15080000;
	PINSEL1 &= ~0x0f0c0000;
	PINSEL1 |= 0x05080000;

	//initialize ADC
	ADCR = (1 << 0) | //SEL = 1, dummy channel #1
			((CRYSTAL_FREQUENCY * PLL_FACTOR / VPBDIV_FACTOR) / 4500000 - 1) << 8 | //set clock division factor, so ADC clock is 4.5MHz
			(0 << 16) | //BURST = 0, conversions are SW controlled
			(0 << 17) | //CLKS  = 0, 11 clocks = 10-bit result
			(1 << 21) | //PDN   = 1, ADC is active
			(1 << 24) | //START = 1, start a conversion now
			(0 << 27); //EDGE  = 0, not relevant when start=1

	//short delay and dummy read
	osSleep (1);
	integerResult = ADDR;

	for (;;)
	{
		//get AIN1 (P0.28)
		integerResult = getAnalogueInput (AIN1);

		//calculate result in mV
		mvResult = integerResult * VREF_IN_MV;
		mvResult /= 1024;

		//    printf("\rAIN1: %d = %d mV", integerResult, mvResult);

		printf("\rAIN1: ");
		consolSendNumber (10, 4, FALSE, ' ', integerResult);
		printf("=");
		consolSendNumber (10, 4, FALSE, ' ', mvResult);
		printf(" mV");

		//get AIN2 (P0.29)
		integerResult = getAnalogueInput (AIN2);

		//calculate result in mV
		mvResult = integerResult * VREF_IN_MV;
		mvResult /= 1024;

		//    printf(", AIN2: %d = %d mV", integerResult, mvResult);
		printf(", AIN2: ");
		consolSendNumber (10, 4, FALSE, ' ', integerResult);
		printf("=");
		consolSendNumber (10, 4, FALSE, ' ', mvResult);
		printf(" mV");

		//set analogue output
		DACR = (dacValue << 6) | //actual value to output
				(1 << 16); //BIAS = 1, 2.5uS settling time
		dacValue++;
		dacValue &= 0x03ff;
		//    printf(", DAC = %d", dacValue);
		printf(", DAC=");
		consolSendNumber (10, 4, FALSE, ' ', dacValue);

		//wait for DAC value to settle
		osSleep (1);

		//get AIN3 (P0.30)
		PINSEL1 &= ~0x30000000;
		PINSEL1 |= 0x10000000; //set P0.30 = AIN3
		delay37us ();
		integerResult = getAnalogueInput (AIN3);
		PINSEL1 &= ~0x30000000; //set P0.30 back to GPIO

		//calculate result in mV
		mvResult = integerResult * VREF_IN_MV;
		mvResult /= 1024;

		//    printf(", AIN3: %d = %d mV", integerResult, mvResult);
		printf(", AIN3:");
		consolSendNumber (10, 4, FALSE, ' ', integerResult);
#if 0
		printf("=");
		consolSendNumber(10,4,FALSE,' ', mvResult);
		printf(" mV");
#endif

		{
			tU8 data[3];

			//test LM75
			if (1 == lm75Read (0x90, &data[0], 3))
			{
				tU16 temp;
				temp = (((tU16) data[0] << 8) + (tU16) data[1]) >> 7;
				printf(", t=");
				consolSendNumber (10, 2, FALSE, ' ', temp / 2);
				printf(".");
				consolSendNumber (10, 1, FALSE, ' ', (temp & 1) * 5);
			} else
				printf(", t=??? ");
		}

		printf(", [");
		consolSendNumber (10, 2, FALSE, ' ', HOUR);
		printf(":");
		consolSendNumber (10, 2, FALSE, ' ', MINx);
		printf(":");
		consolSendNumber (10, 2, FALSE, ' ', SEC);
		printf("]");

		//wait 250 ms
		osSleep (100);
	}
}
