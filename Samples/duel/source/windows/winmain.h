//-----------------------------------------------------------------------------
// File: WinMain.cpp
//
// Desc: Multi-player game
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __WINMAIN_H__
#define __WINMAIN_H__

// Make sure DirectPlay higher APIs are available
#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

// Force Windows.h strict type checking
#ifndef STRICT
#define STRICT
#endif

// Needed for types
#include <windows.h>

extern char g_strAppName[256];

// Duel's guid
extern GUID g_AppGUID;

#endif
