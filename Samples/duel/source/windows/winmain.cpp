//-----------------------------------------------------------------------------
// File: WinMain.cpp
//
// Desc: Multi-player game
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "winmain.h"

// The name of the sample
char g_strAppName[256] = "Duel";

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for
// every instance of that game.  // {33925241-05F8-11d0-8063-00A0C90AE891}
GUID g_AppGUID = {
	0x33925241, 0x5f8, 0x11d0, {0x80, 0x63, 0x0, 0xa0, 0xc9, 0xa, 0xe8, 0x91}};
