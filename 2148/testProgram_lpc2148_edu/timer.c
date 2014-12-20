#include "timer.h"


/*Delays program for passed amount of microseconds*/
void zegarek(uint32 delayInUs){
    T1TCR = 0x02;          //stop and reset timer 1
    T1PR  = 0x00;          //set prescaler to zero
    //set match register to desire microseconds delay
	T1MR0 = (delayInUs - 36)*
			((CRYSTAL_FREQUENCY * PLL_FACTOR)/ (1000000 * VPBDIV_FACTOR));
    T1IR  = 0xff;          //reset all interrrupt flags
    T1MCR = 0x04;          //set match behavior to stop timer on match
    T1TCR = 0x01;          //start timer

    //wait until delay time has elapsed
    //if time has elapsed T1TCR will be 0x00(disabled)
	while (T1TCR & 0x01) asm("NOP");
}
