#ifndef LED_H
#define LED_H

#include <inttypes.h>

//posiadamy diody od 0 do 4
//diody 0..3, P1.16 .. P1.19
//dioda 4, P0.14

//inicjujemy wszystkie diody
void initLeds();

//inicjuje odpowiednie wyj�cie
void initLed(uint8_t led);

//zapala diode na okre�lonej pozycji
void turnOnLed(uint8_t led);

//gasi diode na okre�lonej pozycji
void turnOffLed(uint8_t led);

#endif //LED_H
