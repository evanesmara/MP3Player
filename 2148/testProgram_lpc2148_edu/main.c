#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"

#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>

#include "testLcd.c"
#include "testRGB.c"

#include "i2c.h"
#include "key.h"

#include "usb/lpc_usb.h"
#include "usb/lpc_hid.h"

#include "pff2/src/diskio.h"
#include "pff2/src/pff.h"

#include "functions.h"

static void ProceduraGlowna(void* arg);
#define PAMIEC_PROCEDURA_GLOWNA  400
static tU8 ProceduraGlownaPamiec[PAMIEC_PROCEDURA_GLOWNA];

static void ObslugaEkranuLcd1(void* arg);
#define ObslugaEkranuLcd1_STACK_SIZE 2048
static tU8 ObslugaEkranuLcd1Stack[ObslugaEkranuLcd1_STACK_SIZE];
static tU8 pid1;

static void ObslugaWyswietlacza128(void* arg);
#define ObslugaWyswietlacza128_STACK_SIZE 2048
static tU8 ObslugaWyswietlacza128Stack[ObslugaWyswietlacza128_STACK_SIZE];
static tU8 pid3;

static void ZapalajDiodeProces(void* arg);
#define ZAPALAJDIODE_STACK_SIZE 256
static tU8 zapalajDiode[ZAPALAJDIODE_STACK_SIZE];
static tU8 pidZapalajDiode;
FRESULT wynikInicjalizacjiSd = 2;

void PoinformujZeWczytanoSd();
void PoinformujZeNieWczytanoSd();
void OdtwarzajDzwiek();
static void WyswietlTekst(int _ktory);
FATFS fatfs;
DWORD rc;

volatile tU32 msClock = 0;

//0 - niebieska
//1 - czerwona
//2 - zielona
int kolorDiody = 0;

/*
 * Funkcja wejœciowa programu.
 */
int main(void) {
	tU8 error;
	tU8 pid;

	//immediately turn off buzzer (if connected)
	IODIR0 |= 0x00000080;
	IOSET0 = 0x00000080;

	osInit();

	//	InicjalizacjaKonsoli();

	osCreateProcess(ProceduraGlowna, ProceduraGlownaPamiec,
			PAMIEC_PROCEDURA_GLOWNA, &pid, 1, NULL, &error);
	osStartProcess(pid, &error);

	osStart();
	return 0;
}

/*
 * Procedura g³ówna programu odpalaj¹ca pozosta³e procedury.
 */
static void ProceduraGlowna(void* arg) {
	tU8 error;

	//InicjalizacjaKonsoli();

	lcdInit();
	lcdColor(0xff, 0x00);
	lcdClrscr();
	lcdGotoxy(0, 30);
	lcdPuts("Inicjalizacja");
	osSleep(200);
	lcdClrscr();

	// Wyœwietlacz 1.
	osCreateProcess(ObslugaEkranuLcd1, ObslugaEkranuLcd1Stack,
			ObslugaEkranuLcd1_STACK_SIZE, &pid1, 3, NULL, &error);
	osStartProcess(pid1, &error);

	//	if (kolorDiody == 0) {
	//		wynikInicjalizacjiSd = pf_mount(&fatfs);
	//
	//		if (wynikInicjalizacjiSd == FR_OK)
	//			kolorDiody = 2;
	//		else
	//			kolorDiody = 1;
	//		ZapalajDiode(kolorDiody, 1);
	//	}
	// Stworzenie procesu obs³uguj¹cego migaj¹c¹ diodê (na zielono, czerwono lub niebiesko).
	//	osCreateProcess(ZapalajDiodeProces, zapalajDiode, ZAPALAJDIODE_STACK_SIZE,
	//			&pidZapalajDiode, 3, NULL, &error);
	//	osStartProcess(pidZapalajDiode, &error);

	initKeyProc();

	// Wyœwietlacz 128x128
	osCreateProcess(ObslugaWyswietlacza128, ObslugaWyswietlacza128Stack,
			ObslugaWyswietlacza128_STACK_SIZE, &pid3, 3, NULL, &error);
	osStartProcess(pid3, &error);

	// Zakoñczenie procesów.
	osDeleteProcess();
}

static void ObslugaEkranuLcd1(void* arg) {
	WyswietlTekstNaLcd();
}

static void ObslugaWyswietlacza128(void *arg) {
	//Inicjalizacja ekranu LCD i ustawienie kolorów wyœwietlacza i tekstu.

	WyswietlTekst(0);
	int tekst = 0;
	//ZapalajDiode(kolorDiody, 0);

	while (1) {
		tU8 ruchJoysticka = checkKey();

		if (ruchJoysticka != KEY_NOTHING) {
			switch (ruchJoysticka) {
			case KEY_UP:

				//				kolorDiody = 2; // dzia³a
				tekst = (++tekst % 3);
				WyswietlTekst(tekst);
				break;
			case KEY_DOWN:
				//				kolorDiody = 1;// dzia³a
				tekst = (++tekst % 3);
				WyswietlTekst(tekst);
				break;
			case KEY_RIGHT:
				if (tekst == 2) {
					tekst = 4;
				}
				break;
			}
		} else {
			if (tekst == 4 && wynikInicjalizacjiSd != FR_OK) {
				wynikInicjalizacjiSd = pf_mount(&fatfs);

				if (wynikInicjalizacjiSd == FR_OK) {
					kolorDiody = 2;

				} else {
					kolorDiody = 1;
				}

				rc = listDir("/", TRUE); //poka¿ dok³adn¹ zawartoœæ karty
				if (rc) {
					tekst = 3;

					lcdInit();
					osSleep(100);
					lcdInit();
					//						WyswietlTekst(3);
					lcdColor(0x00, 0xf0);
					lcdClrscr();
					lcdPuts(rc);
					//printf("listDir, rc=%x\n", rc);
				} else {
					lcdInit();
					WyswietlTekst(3);
				}

			} else if (tekst == 1) {
				// TODO: dŸwiêk
			}
			ZapalajDiode(kolorDiody, 0);
		}
	}
}

static void WyswietlTekst(int _ktory) {
	lcdInit();
	lcdColor(0x00, 0xf0);
	lcdClrscr();
	//ZapalajDiode(1, 0);
	switch (_ktory) {
	case 0:
		lcdGotoxy(0, 30);
		lcdPuts("Graj");
		break;
	case 1:
		lcdGotoxy(0, 30);
		lcdPuts("Zatrzymaj");
		//		lcdGotoxy(0, 50);
		//		lcdPuts("Zatrzymaj");
		break;
	case 2:
		lcdGotoxy(0, 30);
		lcdPuts("Brak plików");
		//		lcdGotoxy(0, 50);
		//		lcdPuts("Zatrzymaj");

		break;
	case 3:
		lcdGotoxy(0, 30);
		lcdPuts("Nie wczytano"/*fatfs.buf*/);
		// TODO: poka¿ zawartoœæ
		break;
	default:
		break;
	}
}

static void ZapalajDiodeProces(void* arg) {

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
