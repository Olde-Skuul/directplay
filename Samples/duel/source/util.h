//-----------------------------------------------------------------------------
// File: util.h
//
// Desc: Misc routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __UTIL_H__
#define __UTIL_H__

#define TRACE dtrace

extern int randInt(int low, int high);
extern double randDouble(double low, double high);
extern void dtrace(char* strFormat, ...);

#endif
