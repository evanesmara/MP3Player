#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"

#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include <string.h>

#include "testLcd.c"
#include "testRGB.c"
#include "EAvoice.c"

#include "i2c.h"
#include "key.h"

#include "usb/lpc_usb.h"
#include "usb/lpc_hid.h"

#include "pff2/src/diskio.h"
#include "pff2/src/pff.h"

#include "functions.h"

static void ProcMain (void* arg);
#define STACK_SIZE_MAIN  400
static tU8 stack_Main[STACK_SIZE_MAIN];

static void ProcLCD2x16 (void* arg);
#define STACK_SIZE_LCD2X16 2048
static tU8 stack_LCD2x16[STACK_SIZE_LCD2X16];
static tU8 pid_lcd2x16;

static void ProcRest (void* arg);
#define STACK_SIZE_REST 2048

static void WyswietlMenuGlowne (int nrTekstu);
static void WyswietlTekstNaLCD128x128 (char *s, BOOL czyZgasic);
static void OdtwarzajDzwiek ();

/*
 * 0 - ok
 */
FRESULT wynikInicjalizacjiSd = 2;
FATFS fatfs;
/*
 * Struktura plików z SD
 */
DWORD rc;
volatile tU32 msClock = 0;

/*
 * 0 - niebieska
 * 1 - czerwona
 * 2 - zielona
 */
int kolorDiody = 0;

char listaPlikow[256 * 12];

/*
 * Funkcja wejœciowa programu.
 */
int main (void)
{
	tU8 error;
	tU8 pid_main;

	// immediately turn off buzzer (if connected)
	IODIR0 |= 0x00000080;
	IOSET0 = 0x00000080;

	osInit ();

	osCreateProcess (ProcMain, stack_Main, STACK_SIZE_MAIN, &pid_main, 1, NULL, &error);
	osStartProcess (pid_main, &error);

	osStart ();

	return 0;
}

/*
 * Procedura g³ówna programu odpalaj¹ca pozosta³e procedury.
 */
static void ProcMain (void* arg)
{
	tU8 error;

	// Inicjalizacja ekranu LCD i ustawienie kolorów wyœwietlacza i tekstu.
	WyswietlTekstNaLCD128x128 ("Inicjalizacja", TRUE);

	// Wyœwietlacz LCD 2x16 znaków
	osCreateProcess (ProcLCD2x16, stack_LCD2x16, STACK_SIZE_LCD2X16, &pid_lcd2x16, 3, NULL, &error);
	osStartProcess (pid_lcd2x16, &error);

	// Inicjalizacja joystick'a.
	initKeyProc ();

	// Reszta procesu g³ównego - obs³uga mp3.
	ProcRest (0);

	// Zakoñczenie procesów.
	osDeleteProcess ();
}

static void ProcLCD2x16 (void* arg)
{
	WyswietlTekstNaLcd ();
}

static char* KomunikatInicjalizacji (FRESULT komunikat)
{
	switch (komunikat)
	{
	case 0:
		return "FR_OK";
	case 1:
		return "FR_DISK_ERR";
	case 2:
		return "FR_NOT_READY";
	case 3:
		return "FR_NO_FILE";
	case 4:
		return "FR_NO_PATH";
	case 5:
		return "FR_NOT_OPENED";
	case 6:
		return "FR_NOT_ENABLED";
	case 7:
		return "FR_NO_FILESYSTEM";
	default:
		return "";
	}
}
static void ProcRest (void *arg)
{
	int menuGlowne = 0;
	int niepowodzenie = 0;

	StatusOdtwarzania *status;
	status = ZATRZYMAJ;

	// Inicjalizacja Karty.
	do
	{
		wynikInicjalizacjiSd = pf_mount (&fatfs);
		osSleep (300);

		if (wynikInicjalizacjiSd != FR_OK)
		{
			niepowodzenie += 1;
		}

		kolorDiody = niepowodzenie % 3;
		ZapalajDiode (kolorDiody, 0);

	} while (niepowodzenie < 15 && wynikInicjalizacjiSd != FR_OK);

	if (wynikInicjalizacjiSd == FR_OK)
	{
		kolorDiody = 2;
		WyswietlTekstNaLCD128x128 ("Powodzenie.", 0);

		filesList ("/", listaPlikow); // stwórz liste plików

	} else
	{
		kolorDiody = 1;
		WyswietlTekstNaLCD128x128 (KomunikatInicjalizacji (wynikInicjalizacjiSd), 0);
	}

	ZapalajDiode (kolorDiody, 0);
	osSleep (500);

	while (wynikInicjalizacjiSd == FR_OK)
	{
		tU8 ruchJoysticka = checkKey ();

		if (ruchJoysticka != KEY_NOTHING)
		{
			switch (ruchJoysticka)
			{
			case KEY_UP:
				menuGlowne = (++menuGlowne % 3);
				break;

			case KEY_DOWN:
				menuGlowne = (--menuGlowne % 3);
				break;

			case KEY_RIGHT:

				if (menuGlowne == 1)
				{
					if (status == ZATRZYMAJ)
					{
						status = GRAJ;
					} else
					{
						status = ZATRZYMAJ;
					}
					OdtwarzajDzwiek (status);
				}
				break;
			}
			WyswietlMenuGlowne (menuGlowne);
		}

		osSleep (100);
	}
}

static void OdtwarzajDzwiek (StatusOdtwarzania *status)
{
	tU32 cnt = 0;
	tU32 i;

	IODIR |= 0x00000380;
	IOCLR = 0x00000380;

	//
	//Initialize DAC: AOUT = P0.25
	//
	PINSEL1 &= ~0x000C0000;
	PINSEL1 |= 0x00080000;

	cnt = 0;
	while (cnt++ < 0xF500/*0xF890*/&& status == GRAJ)
	{
		tS32 val;

		val = EAvoice[cnt] - 128;
		val = val * 2;
		if (val > 127)
		{
			val = 127;
		} else if (val < -127)
		{
			val = -127;
		}

		DACR = ((val + 128) << 8) | // actual value to output
				(1 << 16); // BIAS = 1, 2.5uS settling time

		// delay 125 us = 850 for 8kHz, 600 for 11 kHz
		i = 0;
		for (i = 0; i < 850; i++)
		{
			asm volatile (" nop");
		}
	}
	status = ZATRZYMAJ;

}

static void WyswietlTekstNaLCD128x128 (char *s, BOOL czyZgasic)
{
	lcdInit ();
	lcdColor (0xff, 0x00);
	lcdClrscr ();
	lcdGotoxy (0, 30);
	lcdPuts (s);

	if (czyZgasic)
	{
		osSleep (200);
		lcdClrscr ();
	}
}

static void WyswietlMenuGlowne (int nrTekstu)
{
	int i;

	lcdInit ();
	lcdColor (0x00, 0xff);
	lcdClrscr ();

	switch (nrTekstu)
	{
	case 0:

		for (i = 0; i < 3; ++i)
		{
			lcdGotoxy (0, 30);
			lcdPuts (listaPlikow[i * 12]);
		}

		break;

	case 1:
		lcdGotoxy (0, 30);
		lcdPuts ("Odtwarzanie - P");
		lcdGotoxy (0, 60);
		lcdPuts ("Zatrzymanie - L");

		break;

	}
}

static void WyswietlOdtwarzanie (int nrTekstu)
{
	lcdInit ();
	lcdColor (0x00, 0xff);
	lcdClrscr ();

	switch (nrTekstu)
	{
	case 0:
		lcdGotoxy (0, 30);
		lcdPuts ("Pause");
		break;

	case 1:
		lcdGotoxy (0, 30);
		lcdPuts ("Playing");
		break;

	}
}

/*****************************************************************************
 *
 * Description:
 *    The timer tick entry function that is called once every timer tick
 *    interrupt in the RTOS. Observe that any processing in this
 *    function must be kept as short as possible since this function
 *    execute in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - The number of elapsed milliseconds since last call.
 *
 ****************************************************************************/
void appTick (tU32 elapsedTime)
{
	msClock += elapsedTime;
}
