/*
 *  Copyright (C) Josef Gajdusek <atx@atx.name>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util.h"
#include "ch.h"
#include "hal.h"

extern Mutex uartMtx;


void strrev(char *str)
{
	unsigned int i;
	unsigned int len = strlen(str);
	char tmp;

	for (i = 0; i < len / 2; i++) {
		tmp = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = tmp;
	}
}

int nitoa(char *tgt, size_t max, unsigned int val)
{
	unsigned int i;

	for (i = 0; (i < max - 1) && val; i++) {
		tgt[i] = '0' + (val % 10);
		val /= 10;
	}
	tgt[i + 1] = 0;
	strrev(tgt);
	return i + 1;
}


void senddbgmsg(const char *format, ...)
{
	char message[100];
  va_list arg;

	va_start (arg, format);
	vsprintf(message, format, arg);

	if( chMtxTryLock(&uartMtx) == TRUE )
	{
//		uartStartSend(&UARTD2, strlen(format), format);
		uartStartSend(&UARTD2, strlen(message), message);
		
		chMtxUnlock();
	}
	va_end(arg);
}

/*
void senddbgmsg(const char *msge)
{
	if( chMtxTryLock(&uartMtx) == TRUE )
	{
		uartStartSend(&UARTD2, strlen(msge), msge);
		chMtxUnlock();
	}
}
*/