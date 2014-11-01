/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    lcd.h
 *
 * Description:
 *    Expose public functions related to LCD functionality.
 *
 *****************************************************************************/
#ifndef _LCD_H_
#define _LCD_H_

#include "../pre_emptive_os/api/general.h"

void lcdInit(void);
void lcdOff(void);
void lcdContrast(uint8 contr);
void lcdClrscr(void);
void lcdPutchar(uint8 data);
void lcdPuts(char s[]);
void lcdPutsNumber(uint32 number);
void lcdGotoxy(uint8 x, uint8 y);
void lcdWindow(uint8 xp, uint8 yp, uint8 xe, uint8 ye);
void lcdColor(uint8 bkg, uint8 text);
void lcdRect(uint8 x, uint8 y, uint8 xLen, uint8 yLen, uint8 color);
void lcdRectBrd(uint8 x, uint8 y, uint8 xLen, uint8 yLen, uint8 color1, uint8 color2, uint8 color3);
void lcdIcon(uint8 x, uint8 y, uint8 xLen, uint8 yLen, uint8 compressionOn, uint8 escapeChar, const uint8* pData);

void lcdWrdata(uint8 data);
void lcdWrcmd(uint8 cmd);

#endif
