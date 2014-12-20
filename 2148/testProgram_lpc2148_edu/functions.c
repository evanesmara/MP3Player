#include "functions.h"
#include "lpc2xxx.h"
#include <string.h>
#include "lcd.h"
#include <printf_P.h>

FRESULT listDir (CHAR* path, tBool first)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i;

	//	if (first == TRUE) {
	//		printf("D-Directory R-Read_Only H-Hidden S-System A-Archive\n");
	//		printf("DRHSA path/fileName\n");
	//		printf("-------------------------------------------------------\n");
	//	}

	res = pf_opendir (&dir, path);
	if (res == FR_OK)
	{
		i = strlen (path);
		for (;;)
		{
			res = pf_readdir (&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
				break;
			if (fno.fattrib & AM_DIR)
			{
				char tmp[strlen (path) + 1 + strlen (fno.fname)];
				strcpy (tmp, path);
				if (path[1] != "/" && path[1] != 0)
					strncat (tmp, "/", 1);
				strncat (tmp, fno.fname, strlen (fno.fname));

				res = listDir (tmp, FALSE); //rekursywne przeszukiwanie katalogów
				if (res != FR_OK)
					break;
			}
		}
	}

	return res;
}

uint32 filesList (CHAR* path, CHAR* list)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	uint32 i = 0;

	res = pf_opendir (&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			res = pf_readdir (&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
			{
				break;
			}
			if (fno.fattrib & AM_DIR)
			{
				continue;
			} else
			{
//				if (fno.fname[9] != 'W' || fno.fname[10] != 'A' || fno.fname[11] != 'V')
//					continue;
				if (i >= 256)
				{
					break;
				}
				char *p = &list[i * 12];
				strncpy (p, fno.fname, 12);
				i++;
			}
		}
	}

	return i;
}
