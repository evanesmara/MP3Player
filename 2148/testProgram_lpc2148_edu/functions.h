#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "pff2/src/pff.h"
#include "inttypes.h"

typedef enum { // wybrany stan odtwarzania
	ZATRZYMAJ = 0,
	GRAJ,
	PAUZA
} StatusOdtwarzania;

////wypisuje na terminalu, pe³n¹ zawartoœæ karty
//FRESULT listDir(CHAR* _path, tBool first);
//
////tworzy listê plików dla otwarzacza
//uint32 filesList(CHAR* _path, CHAR* list);

#endif
