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

static void ProcMain(void* arg);
#define STACK_SIZE_MAIN  400
static tU8 stack_Main[STACK_SIZE_MAIN];

static void ProcLCD2x16(void* arg);
#define STACK_SIZE_LCD2X16 2048
static tU8 stack_LCD2x16[STACK_SIZE_LCD2X16];
static tU8 pid_lcd2x16;

static void ProcRest(void* arg);
#define STACK_SIZE_REST 2048
static tU8 stack_rest[STACK_SIZE_REST];
static tU8 pid_rest;

static void WyswietlNaLCD128x128(int nrTekstu);
static void WyswietlTekstNaLCD128x128(char *s, BOOL czyZgasic);
static void OdrwarzajDzwiek();

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

/*
 * Funkcja wejœciowa programu.
 */
int main(void) {
	tU8 error;
	tU8 pid_main;

	// immediately turn off buzzer (if connected)
	IODIR0 |= 0x00000080;
	IOSET0 = 0x00000080;

	osInit();

	osCreateProcess(ProcMain, stack_Main, STACK_SIZE_MAIN, &pid_main, 1, NULL,
			&error);
	osStartProcess(pid_main, &error);

	osStart();

	return 0;
}

/*
 * Procedura g³ówna programu odpalaj¹ca pozosta³e procedury.
 */
static void ProcMain(void* arg) {
	tU8 error;

	//Inicjalizacja ekranu LCD i ustawienie kolorów wyœwietlacza i tekstu.
	WyswietlTekstNaLCD128x128("Inicjalizacja", TRUE);

	// Wyœwietlacz LCD 2x16 znaków
	osCreateProcess(ProcLCD2x16, stack_LCD2x16, STACK_SIZE_LCD2X16,
			&pid_lcd2x16, 3, NULL, &error);
	osStartProcess(pid_lcd2x16, &error);

	// inicjalizacja joystick'a
	initKeyProc();

	// Wyœwietlacz 128x128, obs³uga joystick'a, dioda RGB, karta SD, d¿wiêk
	//	osCreateProcess(ProcRest, stack_rest, STACK_SIZE_REST, &pid_rest, 3, NULL,
	//			&error);
	//	osStartProcess(pid_rest, &error);
	ProcRest(0);

	// Zakoñczenie procesów.
	osDeleteProcess();
}

static void ProcLCD2x16(void* arg) {
	WyswietlTekstNaLcd();
}

static void ProcRest(void *arg) {

	int tekst = 0;
	int niepowodzenie = 0;
	StatusOdtwarzania *status;
	status = ZATRZYMAJ;

	WyswietlNaLCD128x128(0);
	//ZapalajDiode(kolorDiody, 0);

	while (niepowodzenie < 5) {
		tU8 ruchJoysticka = checkKey();

		if (ruchJoysticka != KEY_NOTHING) {
			switch (ruchJoysticka) {
			case KEY_UP:
				//				kolorDiody = 2; // dzia³a
				tekst = (++tekst % 3);
				WyswietlNaLCD128x128(tekst);
				break;
			case KEY_DOWN:
				//				kolorDiody = 1;// dzia³a
				tekst = (++tekst % 3);
				WyswietlNaLCD128x128(tekst);
				break;
			case KEY_RIGHT:
				if (tekst == 2) {
					tekst = 4; // mo¿na wczytaæ kartê
				} else if (tekst == 0) {
					tekst = 5; // mo¿na odtworzyæ d¿wiêk
				} else if (tekst == 1) {
					tekst = 6;
				}
				break;
			}
		} else {
			if (tekst == 4 && wynikInicjalizacjiSd != FR_OK) {
				tekst = 3;
				wynikInicjalizacjiSd = pf_mount(&fatfs);
				//				osSleep(500);

				if (wynikInicjalizacjiSd == FR_OK) {
					kolorDiody = 2;
					niepowodzenie = 0;
				} else {
					kolorDiody = 1;
					++niepowodzenie;
				}
				if (wynikInicjalizacjiSd == FR_OK) {
					rc = listDir("/", TRUE); // poka¿ dok³adn¹ zawartoœæ karty
				}
				//				osSleep(500);
				osSleep(100);
				char pliki[256 * 12];
				char nazwa[12];
				uint32 filesCount = filesList("/", pliki); // stwórz liste plików

				if (filesCount > 0) {
//					strncpy(nazwa, &pliki[i * 12], 12);
					tekst = 0;
					WyswietlTekstNaLCD128x128(/*nazwa*/"OK", FALSE);
					kolorDiody = 2;

				} else {
					tekst = 1;
					lcdInit();
					WyswietlNaLCD128x128(3);
					kolorDiody = 1;
				}
			} else if (tekst == 4) {
				tekst = 3;
				if (rc) {
					WyswietlTekstNaLCD128x128(rc, FALSE);
				} else {
					WyswietlNaLCD128x128(3);
				}
			} else if (tekst == 5) {
				tekst = 0;
				if (status == ZATRZYMAJ) {
					status = GRAJ;
					kolorDiody = 3;
					OdrwarzajDzwiek(status);
				}
			} else if (tekst == 6) {
				tekst = 1;
				if (status == GRAJ) {
					status = ZATRZYMAJ;
				}
			}
			ZapalajDiode(kolorDiody, 0);
		}
	}
	lcdInit();
	if (wynikInicjalizacjiSd != FR_OK) {
		WyswietlTekstNaLCD128x128("Blad karty.", FALSE);
	}
	ZapalajDiode(kolorDiody, 0);
}

static void OdrwarzajDzwiek(StatusOdtwarzania *status) {
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
	while (cnt++ < 0xF500/*0xF890*/&& status == GRAJ) {
		tS32 val;

		val = EAvoice[cnt] - 128;
		val = val * 2;
		if (val > 127) {
			val = 127;
		} else if (val < -127) {
			val = -127;
		}

		DACR = ((val + 128) << 8) | // actual value to output
				(1 << 16); // BIAS = 1, 2.5uS settling time

		// delay 125 us = 850 for 8kHz, 600 for 11 kHz
		i = 0;
		for (i = 0; i < 850; i++) {
			asm volatile (" nop");
		}
	}
	status = ZATRZYMAJ;

}

static void WyswietlTekstNaLCD128x128(char *s, BOOL czyZgasic) {
	lcdInit();
	lcdColor(0xff, 0x00);
	lcdClrscr();
	lcdGotoxy(0, 30);
	lcdPuts(s);
	if (czyZgasic) {
		osSleep(200);
		lcdClrscr();
	}
}

/*
 * 0 - graj
 * 1 - zatrzymaj
 * 2 - wczytaj kartê
 * 3 - brak plków
 */
static void WyswietlNaLCD128x128(int nrTekstu) {
	lcdInit();
	lcdColor(0x00, 0xff);
	lcdClrscr();

	switch (nrTekstu) {
	case 0:
		lcdGotoxy(0, 30);
		lcdPuts("Testuj dzwiek");
		break;
	case 1:
		lcdGotoxy(0, 30);
		lcdPuts("Zatrzymaj dzwiek");
		break;
	case 2:
		lcdGotoxy(0, 10);
		lcdPuts("Pokaz pliki..");
		lcdGotoxy(0, 40);
		lcdPuts("zielona lampka:");
		lcdGotoxy(0, 60);
		lcdPuts("wczytano");
		lcdGotoxy(0, 80);
		lcdPuts("czerwona lampka:");
		lcdGotoxy(0, 100);
		lcdPuts("blad karty");

		break;
	case 3:
		lcdGotoxy(0, 30);
		lcdPuts("Brak plikow.");
		break;
	default:
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
void appTick(tU32 elapsedTime) {
	msClock += elapsedTime;
}
