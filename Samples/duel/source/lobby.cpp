//-----------------------------------------------------------------------------
// File: Lobby.cpp
//
// Desc: DP lobby related routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "lobby.h"
#include "dpconnect.h"
#include "dpmacros.h"
#include "duel.h"
#include "util.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define TIMERID 1         // Timer ID to use
#define TIMERINTERVAL 250 // Timer interval

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

// Lobby messages are supported
BOOL g_bLobbyMsgSupported;

// This player in this session
GUID g_guidPlayer;

//-----------------------------------------------------------------------------
// Name: DPLobbyRelease()
// Desc: Wrapper for DirectPlayLobby Release API
//-----------------------------------------------------------------------------
HRESULT DPLobbyRelease()
{
	// Cleanup lobby
	SAFE_DELETE_ARRAY(g_pDPLConnection);
	SAFE_RELEASE(g_pDPLobby);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DoingLobbyMessages()
// Desc: Return TRUE if connected to a lobby and messages are supported.
//-----------------------------------------------------------------------------
BOOL DoingLobbyMessages()
{
	return (g_pDPLobby && g_bLobbyMsgSupported);
}

//-----------------------------------------------------------------------------
// Name: LobbyMessageInit()
// Desc: Initialize lobby message processing.  Must be done while a lobby
//       connection is open.
//-----------------------------------------------------------------------------
HRESULT LobbyMessageInit()
{
	// Not connected to a lobby
	if (NULL == g_pDPLobby) {
		return E_FAIL;
	}

	// Until we find out otherwise.
	g_bLobbyMsgSupported = FALSE;

	// Find out if lobby messages are supported by the lobby
	DPLMSG_GETPROPERTY msgGetProp;
	ZeroMemory(&msgGetProp, sizeof(DPLMSG_GETPROPERTY));
	msgGetProp.dwType = DPLSYS_GETPROPERTY;
	msgGetProp.dwRequestID = 1; // guidPlayer is not used
	msgGetProp.guidPropertyTag = DPLPROPERTY_MessagesSupported;

	return g_pDPLobby->SendLobbyMessage(
		DPLMSG_STANDARD, 0, &msgGetProp, sizeof(DPLMSG_GETPROPERTY));
}

//-----------------------------------------------------------------------------
// Name: LobbyMessageReceive()
// Desc: Check for any lobby messages and handle them
//       There are two modes:
//          LMR_CONNECTIONSETTINGS - Waiting only for settings from a lobby
//          LMR_PROPERTIES - Handle property message responses
//-----------------------------------------------------------------------------
HRESULT LobbyMessageReceive(DWORD dwMode)
{
	HRESULT hr = NOERROR;
	LPVOID lpMsgBuffer = NULL;
	DWORD dwBufferSize = 0;
	DWORD dwRequiredSize;
	DWORD dwMsgFlags;

	// No lobby interface
	if (NULL == g_pDPLobby) {
		return DPERR_NOINTERFACE;
	}

	// Get all queued messages
	while (SUCCEEDED(hr)) {
		dwRequiredSize = dwBufferSize;
		hr = g_pDPLobby->ReceiveLobbyMessage(
			0, 0, &dwMsgFlags, lpMsgBuffer, &dwRequiredSize);
		// Alloc msg buffer and try again
		if (hr == DPERR_BUFFERTOOSMALL) {

			if (NULL == lpMsgBuffer) {
				lpMsgBuffer = GlobalAllocPtr(GHND, dwRequiredSize);
			} else {
				lpMsgBuffer = GlobalReAllocPtr(lpMsgBuffer, dwRequiredSize, 0);
			}

			if (NULL == lpMsgBuffer) {
				return DPERR_NOMEMORY;
			}

			dwBufferSize = dwRequiredSize;
			hr = S_OK;
		} else if (SUCCEEDED(hr) && dwRequiredSize >= sizeof(DPLMSG_GENERIC)) {
			// Decode the message

			// Are we just looking for the CONNECTIONSETTINGS msg?
			if (dwMode == LMR_CONNECTIONSETTINGS) {
				if ((dwMsgFlags & DPLMSG_SYSTEM) &&
					((DPLMSG_GENERIC*)lpMsgBuffer)->dwType ==
						DPLSYS_NEWCONNECTIONSETTINGS) {
					break;
				} else {
					TRACE("Non CONNECTIONSETTINGS lobby message ignored\n");
				}
			}

			// Otherwise we handle only GetProperty responses
			else if ((dwMsgFlags & DPLMSG_STANDARD) &&
				((DPLMSG_GENERIC*)lpMsgBuffer)->dwType ==
					DPLSYS_GETPROPERTYRESPONSE) {
				DPLMSG_GETPROPERTYRESPONSE* lpMsgGPR =
					(DPLMSG_GETPROPERTYRESPONSE*)lpMsgBuffer;
				if (IsEqualGUID(lpMsgGPR->guidPropertyTag,
						DPLPROPERTY_MessagesSupported)) {
					// Supported
					if ((BOOL)lpMsgGPR->dwPropertyData[0]) {
						// So request our player instance guid
						DPLMSG_GETPROPERTY msgGetProp;
						ZeroMemory(&msgGetProp, sizeof(DPLMSG_GETPROPERTY));
						msgGetProp.dwType = DPLSYS_GETPROPERTY;
						msgGetProp.dwRequestID = 2;
						// guidPlayer is left NULL
						msgGetProp.guidPropertyTag = DPLPROPERTY_PlayerGuid;
						hr = g_pDPLobby->SendLobbyMessage(DPLMSG_STANDARD, 0,
							&msgGetProp, sizeof(DPLMSG_GETPROPERTY));
						hr = S_OK; // keep fetching messages
					} else {

						// not supported so close up shop
						TRACE("Lobby Messages not supported\n");
						DPLobbyRelease();
						break;
					}
				} else if (IsEqualGUID(lpMsgGPR->guidPropertyTag,
							   DPLPROPERTY_PlayerGuid)) {
					// Have our player guid, ready to send property msgs
					g_guidPlayer =
						((DPLDATA_PLAYERGUID*)&lpMsgGPR->dwPropertyData)
							->guidPlayer;
					g_bLobbyMsgSupported = TRUE;
				}
			} else {
				TRACE("Unrecognized lobby message ignored\n");
			}
		}
	}

	if (lpMsgBuffer) {
		GlobalFreePtr(lpMsgBuffer);
	}
	return hr;
}

//-----------------------------------------------------------------------------
// Name: LobbyMessageSetProperty()
// Desc: Send a SetProperty message
//-----------------------------------------------------------------------------
HRESULT LobbyMessageSetProperty(
	const GUID* pPropTagGUID, VOID* pData, DWORD dwDataSize)
{
	HRESULT hr;
	LPBYTE lpBuffer;
	LPDPLMSG_SETPROPERTY lpMsgSP;
	DWORD dwMsgSize;

	if (NULL == g_pDPLobby) {
		return DPERR_NOCONNECTION;
	}

	if (NULL == g_bLobbyMsgSupported) {
		return DPERR_UNAVAILABLE;
	}

	// Allocate and pack up the message
	// Property data starts at dwPropertyData[0] for the size calculation
	dwMsgSize = sizeof(DPLMSG_SETPROPERTY) - sizeof(DWORD) + dwDataSize;
	lpBuffer = (LPBYTE)GlobalAllocPtr(GHND, dwMsgSize);
	if (NULL == lpBuffer) {
		return DPERR_NOMEMORY;
	}

	lpMsgSP = (LPDPLMSG_SETPROPERTY)lpBuffer;
	lpMsgSP->dwType = DPLSYS_SETPROPERTY;
	lpMsgSP->dwRequestID = DPL_NOCONFIRMATION;
	lpMsgSP->guidPlayer = g_guidPlayer; // player property assumed
	lpMsgSP->guidPropertyTag = (*pPropTagGUID);
	lpMsgSP->dwDataSize = dwDataSize;
	memcpy(lpMsgSP->dwPropertyData, pData, dwDataSize);
	hr = g_pDPLobby->SendLobbyMessage(DPLMSG_STANDARD, 0, lpBuffer, dwMsgSize);

	GlobalFreePtr(lpBuffer);

	return hr;
}
