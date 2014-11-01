#ifndef _EEPROM_H
#define _EEPROM_H

#include "i2c.h"

tS8 eepromWrite(tU16 addr,
                tU8* pData,
                tU16 len);

tS8 eepromPageRead(tU16 address, 
                   tU8* pBuf, 
                   tU16 len);

tS8 eepromPoll(void);

tS8 lm75Read(tU8 address, tU8* pBuf, tU16 len);


#endif
