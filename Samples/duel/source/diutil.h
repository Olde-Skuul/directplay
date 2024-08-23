//-----------------------------------------------------------------------------
// File: DIUtil.h
//
// Desc: Input routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DIUTIL_H__
#define __DIUTIL_H__

#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

extern HRESULT DIUtil_InitInput(HWND hWnd);
extern VOID DIUtil_ReadKeys(DWORD* pdwKey);
extern VOID DIUtil_CleanupInput();
extern HRESULT DIUtil_ReacquireInputDevices();

#endif
