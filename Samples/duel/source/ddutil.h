//-----------------------------------------------------------------------------
// File: DDutil.h
//
// Desc: Routines for loading bitmap and palettes from resources
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DDUTIL_H__
#define __DDUTIL_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <ddraw.h>

extern HRESULT DDUtil_CopyBitmap(
	LPDIRECTDRAWSURFACE pdds, HBITMAP hbm, int x, int y, int dx, int dy);
extern LPDIRECTDRAWPALETTE DDUtil_LoadPalette(
	LPDIRECTDRAW pDD, char* strBitmap);
extern DWORD DDUtil_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb);
extern HRESULT DDUtil_SetColorKey(LPDIRECTDRAWSURFACE pdds, COLORREF rgb);

#endif
