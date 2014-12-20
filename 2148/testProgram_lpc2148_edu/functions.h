#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "pff2/src/pff.h"
#include "inttypes.h"

typedef enum { // wybrany stan odtwarzania
	Player_Stoped = 0,
	Player_Playing,
	Player_Paused,
	Player_Prev,
	Player_Next,
	Player_Random
} PlayerStatus;

typedef enum { //okre�la metode prezentacji czasu odtwarzania
	Timer_UP = 0, //odliczanie ile mine�o
	Timer_DOWN
//odliczanie ile pozosta�o
} TimerStatus;

typedef enum {
	M_Lista = 0, M_Odtwarzanie
};

typedef enum {
	O_INICJALIZUJ_KARTE, O_PODEJRZYJ_LISTE_PLIKOW
} OPCJE_MENU;

////wypisuje na terminalu, pe�n� zawarto�� karty
//FRESULT listDir(CHAR* _path, tBool first);
//
////tworzy list� plik�w dla otwarzacza
//uint32 filesList(CHAR* _path, CHAR* list);

#endif
