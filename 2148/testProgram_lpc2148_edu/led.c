#include "../startup/lpc2xxx.h"                      /* LPC213x definitions  */
#include "led.h"

void initLeds(){
	initLed(0);
	initLed(1);
	initLed(2);
	initLed(3);
	initLed(4);
}

void initLed(uint8_t led){
	turnOffLed(led);
	if( led <= 3 )
		IO1DIR = IO1DIR  | (1 << (16 + led));
	else
		IO0DIR = IO0DIR  | (1 << 14); 
}

void turnOnLed(uint8_t led){
	if( led <= 3 )
		IO1CLR = (1 << (16 + led));
	else
		IO0CLR = (1 << 14); 
}

void turnOffLed(uint8_t led){
	if( led <= 3 )
		IO1SET = (1 << (16 + led));
	else
		IO0SET = (1 << 14); 
}
