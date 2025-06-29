//-----------------------------------------------------------------------------
// File: Bellhop.h
//
// Desc: Header file for bellhop
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

#include <windows.h>
#include <windowsx.h>

#include "CGRpTree.h"
#include <dplay.h>
#include <dplobby.h>

// constants
const DWORD MAXPLAYERS = 10; // max no. players in the session
const DWORD MAXSTRLEN = 200; // max. size of a string

// structure used to store DirectPlay information
struct DPLAYINFO {
	LPDIRECTPLAY4A pDP;           // DPlay4A interface pointer
	LPDIRECTPLAYLOBBY3A pDPLobby; // DPlayLobby3A interface pointer
	HANDLE hPlayerEvent;          // player event to use
	DPID dpidPlayer;              // ID of player created
	BOOL bIsHost;                 // TRUE if we are hosting the session
	BOOL bSecureSession;          // TRUE if the session is secure.
	DWORD dwPlayerFlags;
	CGroupTree* pGroupTree;
	int xPaneSplit;
	int xHalfSplitWidth;
	int ySpacing;
	int xSpacing;
	BOOL bSplitMove;
	CHAR strSecureName[256];
	CHAR strSecurePassword[256];
	CHAR strSecureDomain[256];
};

struct ENUMCONNSTRUCT {
	HWND hWnd;
	int idCombo;
};

struct CONNECTIONINFO {
	GUID guidSP;
	BYTE Connection[1];
};

struct SESSIONINFO {
	GUID guidInstance;
	DWORD dwFlags;
};

struct LOBBYGROUPCONTEXT {
	DPLAYINFO* pDPInfo;
	DPID dpidRoom;
};

struct APPNAMECONTEXT {
	GUID guidApplication;
	CHAR strAppName[MAXSTRLEN];
};

// guid for this application
// {4BF5D540-BDA5-11d0-9C4F-00A0C905425E}
DEFINE_GUID(BELLHOP_GUID, 0x4bf5d540, 0xbda5, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0,
	0xc9, 0x5, 0x42, 0x5e);

// Globals
extern HINSTANCE g_hInstance;

// prototypes
extern BOOL FAR PASCAL EnumApp(
	const DPLAPPINFO* pAppInfo, VOID* pContext, DWORD dwFlags);
extern HRESULT GetComboBoxGuid(HWND hWnd, LONG iDialogItem, GUID* pguidReturn);
extern VOID ErrorBox(LPSTR strError, HRESULT hr);
