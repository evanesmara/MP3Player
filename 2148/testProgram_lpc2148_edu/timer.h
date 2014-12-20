#ifndef TIMER_H
#define TIMER_H
/*
 This file declares used to pause program for desire amount of miliseconds.
*/

#include "../startup/lpc2xxx.h"
#include "../pre_emptive_os/api/general.h"
#include "../startup/config.h"


//some timer parameters
#define CRYSTAL_FREQUENCY FOSC
#define PLL_FACTOR        PLL_MUL
#define VPBDIV_FACTOR     PBSD


//zatrzymaj program na okreœlon¹ iloœæ mikrosekund
void zegarek(uint32 delayInUs);

#endif
