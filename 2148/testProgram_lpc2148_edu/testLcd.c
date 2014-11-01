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

#define LCD_DATA      0x00ff0000  //P1.16-P1.23
#define LCD_E         0x02000000  //P1.25
#define LCD_RW        0x00400000  //P0.22
#define LCD_RS        0x01000000  //P1.24
#define LCD_BACKLIGHT 0x40000000  //P0.30

static void initLCD (void)
{
	IODIR1 |= (LCD_DATA | LCD_E | LCD_RS);
	IOCLR1 = (LCD_DATA | LCD_E | LCD_RS);

	IODIR0 |= LCD_RW;
	IOCLR0 = LCD_RW;

	IODIR0 |= LCD_BACKLIGHT;
	IOCLR0 = LCD_BACKLIGHT;
}

/*
 * Wywo³anie przerwania opóŸniaj¹cego na oko³o 37 mikrosekund.
 * nop - No Operation
 */
static void delay37us (void)
{
	volatile tU32 i;

	for (i = 0; i < 15 * 2500; ++i)
	{
		asm volatile (" nop");
		//delay 15 ns x 2500 = about 37,5 us delay
	}
}

/*
 * W³¹cza (TRUE) lub wy³¹cza (FALSE) podœwielenie ekranu.
 */
static void PodswietlEkran (tU8 onOff)
{
	if (onOff == TRUE)
	{
		IOSET0 = LCD_BACKLIGHT;
	} else
	{
		IOCLR0 = LCD_BACKLIGHT;
	}
}

/*
 * Zainicjownie poprzez rejestry kerunek pinów jako wyjœcia (dla wypisania tekstu)
 */
static void WypiszZnakNaEkran (tU8 reg, tU8 data)
{
	volatile tU8 i;

	if (reg == 0)
	{
		IOCLR1 = LCD_RS;
	} else
	{
		IOSET1 = LCD_RS;
	}

	IOCLR0 = LCD_RW;
	IOCLR1 = LCD_DATA;
	IOSET1 = ((tU32) data << 16) & LCD_DATA;
	IOSET1 = LCD_E;

	for (i = 0; i < 16; ++i)
	{
		asm volatile (" nop");
		//delay 15 ns x 16 = about 240 ns delay
	}
	IOCLR1 = LCD_E;

	for (i = 0; i < 16; ++i)
	{
		asm volatile (" nop");
		//delay 15 ns x 16 = about 240 ns delay
	}
}

/*
 * Wypisuje ci¹g znaków w linii na ekranie.
 */
static void WypiszCiag (const char* tekst)
{
	while (*tekst != '\0')
	{
		WypiszZnakNaEkran (1, *tekst);
		delay37us ();

		tekst++;
	}
}

/*
 * Funkcja czyszcz¹ca ekran wyœwietlacza.
 */
static void WyczyscEkran ()
{
	WypiszZnakNaEkran (0, 0x01);
	osSleep (50);
}

static void PoczekajIWyczyscEkran (int ileCzekac)
{
	//OpóŸnienie przed znikniêciem.
	osSleep (ileCzekac);
	//Wy³¹czenie podœwietlenia monitora przed wyczyszczeniem.
	PodswietlEkran (FALSE);

	WyczyscEkran();

	PodswietlEkran (TRUE);
}

/*
 * Przenosi kursor wyœwietlacza do nastêpnej linii.
 */
static void PrzejdzDoNastepnejLinii ()
{
	WypiszZnakNaEkran (0, 0xC0);
	delay37us ();
}

/*
 * Wyœwietlenie na ekranie LCD napisów.
 */
static void WyswietlTekstNaLcd (void)
{
	initLCD ();
	PoczekajIWyczyscEkran (1);

	while (1)
	{
		WypiszZnakNaEkran (0, 0x30);
		osSleep (1);
		WypiszZnakNaEkran (0, 0x30);
		delay37us ();
		WypiszZnakNaEkran (0, 0x30);
		delay37us ();
		WypiszZnakNaEkran (0, 0x38);
		delay37us ();

		//display off
		WypiszZnakNaEkran (0, 0x08);
		delay37us ();

		WyczyscEkran ();

		//display control set
		WypiszZnakNaEkran (0, 0x06);
		osSleep (1);

		//display control set
		WypiszZnakNaEkran (0, 0x0c);
		delay37us ();

		//cursor home
		WypiszZnakNaEkran (0, 0x02);
		osSleep (1);

		WypiszCiag ("   Odtwarzacz");

		PrzejdzDoNastepnejLinii ();

		WypiszCiag ("       MP3");

		PoczekajIWyczyscEkran (500);
	}
}
