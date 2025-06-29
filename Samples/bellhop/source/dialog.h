//-----------------------------------------------------------------------------
// File: Dialog.h
//
// Desc: Header file for bellhop
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DIALOG_H__
#define __DIALOG_H__

#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

#include <windows.h>

#include <dplay.h>

struct DPLAYINFO;

extern HRESULT ConnectUsingDialog(HINSTANCE hInstance, DPLAYINFO* pDPInfo);
extern BOOL FAR PASCAL DirectPlayEnumConnectionsCallback(const GUID* pguidSP,
	VOID* pConnection, DWORD dwSize, const DPNAME* pName, DWORD dwFlags,
	VOID* pContext);
extern HRESULT GetConnectionSPGuid(HWND hWnd, int idCombo, GUID* pGuidSP);

#endif
