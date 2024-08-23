//-----------------------------------------------------------------------------
// File: dputil.h
//
// Desc: Communication routines include file
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DPUTIL_H__
#define __DPUTIL_H__

#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

#include <dplay.h>

// Current session description
extern DPSESSIONDESC2* g_pdpsd;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

extern HRESULT DPUtil_FreeDirectPlay();
extern HRESULT DPUtil_InitDirectPlay(VOID* pCon);
extern HRESULT DPUtil_CreatePlayer(DPID* ppidID, LPTSTR pPlayerName,
	HANDLE hEvent, VOID* pData, DWORD dwDataSize);
extern HRESULT DPUtil_CreateSession(char* strSessionName);
extern HRESULT DPUtil_DestroyPlayer(DPID pid);
extern HRESULT DPUtil_EnumPlayers(GUID* pSessionGuid,
	LPDPENUMPLAYERSCALLBACK2 lpEnumCallback, VOID* pContext, DWORD dwFlags);
extern HRESULT DPUtil_EnumSessions(DWORD dwTimeout,
	LPDPENUMSESSIONSCALLBACK2 lpEnumCallback, VOID* pContext, DWORD dwFlags);
extern HRESULT DPUtil_GetPlayerLocalData(
	DPID pid, VOID* pData, DWORD* pdwDataSize);
extern HRESULT DPUtil_GetSessionDesc();
extern BOOL DPUtil_IsDPlayInitialized();
extern HRESULT DPUtil_OpenSession(GUID* pSessionGuid);
extern HRESULT DPUtil_Receive(
	DPID* pidFrom, DPID* pidTo, DWORD dwFlags, VOID* pData, DWORD* pdwDataSize);
extern HRESULT DPUtil_Release();
extern HRESULT DPUtil_Send(
	DPID idFrom, DPID idTo, DWORD dwFlags, VOID* pData, DWORD dwDataSize);
extern HRESULT DPUtil_SetPlayerLocalData(DPID pid, VOID* pData, DWORD dwSize);

#endif
