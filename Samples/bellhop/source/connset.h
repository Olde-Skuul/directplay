//-----------------------------------------------------------------------------
// File: Connset.h
//
// Desc: Header file for bellhop
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __CONNSET_H__
#define __CONNSET_H__

#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

#include <windows.h>

extern BOOL CALLBACK ConnectionSettingsDialogProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif
