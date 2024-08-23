//-----------------------------------------------------------------------------
// File: Lobby.h
//
// Desc: DP lobby related routines include file
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __LOBBY_H__
#define __LOBBY_H__

#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

#include <dplobby.h>

//-----------------------------------------------------------------------------
// LobbyMessageReceive Modes
//-----------------------------------------------------------------------------
#define LMR_PROPERTIES 0
#define LMR_CONNECTIONSETTINGS 1

// Lobby messages are supported
extern BOOL g_bLobbyMsgSupported;

// This player in this session
extern GUID g_guidPlayer;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
extern HRESULT DPLobbyRelease();
extern BOOL DoingLobbyMessages();
extern HRESULT LobbyMessageInit();
extern HRESULT LobbyMessageReceive(DWORD dwMode);
extern HRESULT DPLobbySetConnectionSettings();
extern HRESULT LobbyMessageSetProperty(
	const GUID* pPropTagGUID, VOID* pData, DWORD dwDataSize);

#endif
