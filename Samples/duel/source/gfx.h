//-----------------------------------------------------------------------------
// File: gfx.h
//
// Desc: Graphics routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __GFX_H__
#define __GFX_H__

#include <ddraw.h>

#include "gameproc.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// DirectDraw interface
extern LPDIRECTDRAW g_pDD;

// Game screen palette
extern LPDIRECTDRAWPALETTE g_pArtPalette;

// Splash screen palette
extern LPDIRECTDRAWPALETTE g_pSplashPalette;

// primary surface
extern LPDIRECTDRAWSURFACE g_pddsFrontBuffer;

// back buffer for animation
extern LPDIRECTDRAWSURFACE g_pddsBackBuffer;

// ship bitmaps
extern LPDIRECTDRAWSURFACE g_pddsShip[NUM_SHIP_TYPES];

// Numbers bitmap
extern LPDIRECTDRAWSURFACE g_pddsNumbers;

// Background fill color
extern DWORD g_dwFillColor;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

extern HRESULT InitGraphics();
extern VOID CleanupGraphics();
extern HRESULT BltSplashScreen(RECT* prc);
extern HRESULT BltNumber(char* strScore, int x, int y);
extern HRESULT BltObject(
	int x, int y, LPDIRECTDRAWSURFACE pdds, RECT* src, DWORD dwFlags);
extern VOID EraseScreen();
extern VOID FlipScreen();
extern HRESULT RestoreSurfaces();
extern VOID SetGamePalette();

#endif