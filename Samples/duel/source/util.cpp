//-----------------------------------------------------------------------------
// File: Util.cpp
//
// Desc: Misc routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "util.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Name: randInt()
// Desc: Returns a random integer in the specified range
//-----------------------------------------------------------------------------
int randInt(int low, int high)
{
	int range = high - low;
	int num = rand() % range;
	return (num + low);
}

//-----------------------------------------------------------------------------
// Name: randDouble()
// Desc: Returns a random double in the specified range
//-----------------------------------------------------------------------------
double randDouble(double low, double high)
{
	double range = high - low;
	double num = range * (double)rand() / (double)RAND_MAX;
	return (num + low);
}

//-----------------------------------------------------------------------------
// Name: dtrace()
// Desc: Diagnostic trace to OutputDebugString() (UNICODE supported)
//-----------------------------------------------------------------------------
void dtrace(char* strFormat, ...)
{
	int offset = 0;
	char strBuf[256];
	va_list ap;

	va_start(ap, strFormat);

	offset = sprintf(strBuf, "DUEL: ");
	offset += vsprintf(strBuf + offset, strFormat, ap);

	OutputDebugStringA(strBuf);

	va_end(ap);
}
