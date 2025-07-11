//-----------------------------------------------------------------------------
// File: DPSlots.cpp
//
// Desc: Main DPSlots file
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>

#include "dpdialogs.h"
#include "dpslots.h"
#include "resource.h"
#include <stdio.h>

#if defined(UNICODE) || defined(_UNICODE)
#error This app does not support UNICODE
#endif

// globals
HANDLE ghReceiveThread = NULL;     // handle of receive thread
DWORD gidReceiveThread = 0;        // id of receive thread
HANDLE ghKillReceiveEvent = NULL;  // event used to kill receive thread
HINSTANCE ghInstance = NULL;       // application instance
CHAR g_strDatabaseName[MAXSTRLEN]; // database name

//-----------------------------------------------------------------------------
// Name: ShutdownConnection()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT ShutdownConnection(DPLAYINFO* pDPInfo)
{
	if (ghReceiveThread) {
		// Wake up receive thread and wait for it to quit
		SetEvent(ghKillReceiveEvent);
		WaitForSingleObject(ghReceiveThread, INFINITE);

		CloseHandle(ghReceiveThread);
		ghReceiveThread = NULL;
	}

	if (ghKillReceiveEvent) {
		CloseHandle(ghKillReceiveEvent);
		ghKillReceiveEvent = NULL;
	}

	if (pDPInfo->pDPlay) {
		if (pDPInfo->dpidPlayer) {
			pDPInfo->pDPlay->DestroyPlayer(pDPInfo->dpidPlayer);
			pDPInfo->dpidPlayer = 0;
		}
		pDPInfo->pDPlay->Close();
		pDPInfo->pDPlay->Release();
		pDPInfo->pDPlay = NULL;
	}

	if (pDPInfo->hPlayerEvent) {
		CloseHandle(pDPInfo->hPlayerEvent);
		pDPInfo->hPlayerEvent = NULL;
	}

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: HandleSystemMessage()
// Desc:
//-----------------------------------------------------------------------------
static VOID HandleSystemMessage(DPLAYINFO* pDPInfo, DPMSG_GENERIC* pMsg,
	DWORD dwMsgSize, DPID idFrom, DPID idTo)
{
	if (pDPInfo->bIsHost) {
		ServerSystemMessage(pDPInfo, pMsg, dwMsgSize, idFrom, idTo);
	} else {
		ClientSystemMessage(pDPInfo, pMsg, dwMsgSize, idFrom, idTo);
	}
}

//-----------------------------------------------------------------------------
// Name: HandleApplicationMessage()
// Desc:
//-----------------------------------------------------------------------------
static VOID HandleApplicationMessage(DPLAYINFO* pDPInfo, DPMSG_GENERIC* pMsg,
	DWORD dwMsgSize, DPID idFrom, DPID idTo)
{
	// When using a secure session we should not get any messages here
	// because encrypted messages come through as system messages. Therefore,
	// it is a security hole to process messages here.

	if (pDPInfo->bIsSecure) {
		return;
	}

	if (pDPInfo->bIsHost) {
		ServerApplicationMessage(pDPInfo, pMsg, dwMsgSize, idFrom, idTo);
	} else {
		ClientApplicationMessage(pDPInfo, pMsg, dwMsgSize, idFrom, idTo);
	}
}

//-----------------------------------------------------------------------------
// Name: ReceiveMessage()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT ReceiveMessage(DPLAYINFO* pDPInfo)
{
	DPID idFrom, idTo;
	VOID* pMsgBuffer = NULL;
	DWORD dwMsgBufferSize = 0;
	HRESULT hr;

	// Loop to read all messages in queue
	do {
		// Loop until a single message is successfully read
		do {
			// Read messages from any player, including system player
			idFrom = 0;
			idTo = 0;

			hr = pDPInfo->pDPlay->Receive(
				&idFrom, &idTo, DPRECEIVE_ALL, pMsgBuffer, &dwMsgBufferSize);

			// Not enough room, so resize buffer
			if (hr == DPERR_BUFFERTOOSMALL) {
				if (pMsgBuffer) {
					GlobalFreePtr(pMsgBuffer);
				}
				pMsgBuffer = GlobalAllocPtr(GHND, dwMsgBufferSize);
				if (pMsgBuffer == NULL) {
					hr = DPERR_OUTOFMEMORY;
				}
			}
		} while (hr == DPERR_BUFFERTOOSMALL);

		if ((SUCCEEDED(hr)) && (dwMsgBufferSize >= sizeof(DPMSG_GENERIC))) {
			// Check for system message
			if (idFrom == DPID_SYSMSG) {
				HandleSystemMessage(pDPInfo, (DPMSG_GENERIC*)pMsgBuffer,
					dwMsgBufferSize, idFrom, idTo);
			} else {
				HandleApplicationMessage(pDPInfo, (DPMSG_GENERIC*)pMsgBuffer,
					dwMsgBufferSize, idFrom, idTo);
			}
		}
	} while (SUCCEEDED(hr));

	// Free any memory we created
	if (pMsgBuffer) {
		GlobalFreePtr(pMsgBuffer);
	}
	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: ReceiveThread()
// Desc:
//-----------------------------------------------------------------------------
static DWORD WINAPI ReceiveThread(VOID* lpThreadParameter)
{
	DPLAYINFO* pDPInfo = (DPLAYINFO*)lpThreadParameter;
	HANDLE eventHandles[2];

	eventHandles[0] = pDPInfo->hPlayerEvent;
	eventHandles[1] = ghKillReceiveEvent;

	// loop waiting for player events. If the kill event is signaled
	// the thread will exit
	while (WaitForMultipleObjects(2, eventHandles, FALSE, INFINITE) ==
		WAIT_OBJECT_0) {
		// receive any messages in the queue
		ReceiveMessage(pDPInfo);
	}

	ExitThread(0);

	return 0;
}

//-----------------------------------------------------------------------------
// Name: SetupConnection()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT SetupConnection(HINSTANCE hInstance, DPLAYINFO* pDPInfo)
{
	HRESULT hr;

	ZeroMemory(pDPInfo, sizeof(DPLAYINFO));

	// create event used by DirectPlay to signal a message has arrived
	pDPInfo->hPlayerEvent = CreateEvent(NULL, // no security
		FALSE,                                // auto reset
		FALSE,                                // initial event reset
		NULL);                                // no name
	if (pDPInfo->hPlayerEvent == NULL) {
		hr = DPERR_NOMEMORY;
	} else {

		// create event used to signal that the receive thread should exit
		ghKillReceiveEvent = CreateEvent(NULL, // no security
			FALSE,                             // auto reset
			FALSE,                             // initial event reset
			NULL);                             // no name
		if (ghKillReceiveEvent == NULL) {
			hr = DPERR_NOMEMORY;
		} else {

			// create thread to receive player messages
			ghReceiveThread = CreateThread(NULL, // default security
				0,                               // default stack size
				ReceiveThread,                   // pointer to thread routine
				pDPInfo,                         // argument for thread
				0,                               // start it right away
				&gidReceiveThread);
			if (ghReceiveThread == NULL) {
				hr = DPERR_NOMEMORY;
			} else {

				// try to connect using the lobby
				hr = ConnectUsingLobby(pDPInfo);
				if (SUCCEEDED(hr)) {
					return DP_OK;
				}

				// if the error returned is DPERR_NOTLOBBIED, that means we
				// were not launched by a lobby and we should ask the user
				// for connection settings. If any other error is returned
				// it means we were launched by a lobby but there was an
				// error making the connection.

				if (hr != DPERR_NOTLOBBIED) {
					ErrorBox(
						"Could not connect using lobby because of error %s",
						hr);
				}
				// if there is no lobby connection, ask the user for
				// settings
				hr = ConnectUsingDialog(hInstance, pDPInfo);
				if (SUCCEEDED(hr)) {
					return DP_OK;
				}
			}
		}
	}
	ShutdownConnection(pDPInfo);

	return hr;
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc:
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	DPLAYINFO DPInfo;
	int iResult = 0;

	ghInstance = hInstance;
	srand(GetTickCount());
	strcpy(g_strDatabaseName, DEFAULTDATABASE);

	// Initialize COM library
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {

		// Setup the connection
		hr = SetupConnection(hInstance, &DPInfo);
		if (SUCCEEDED(hr)) {

			if (DPInfo.bIsHost) {
				// Show the server window
				iResult = DialogBoxParamA(hInstance,
					MAKEINTRESOURCEA(IDD_SERVERDIALOG), NULL,
					(DLGPROC)ServerWndProc, (LPARAM)&DPInfo);
			} else {
				// Show the client window
				iResult = DialogBoxParamA(hInstance,
					MAKEINTRESOURCEA(IDD_CLIENTDIALOG), NULL,
					(DLGPROC)ClientWndProc, (LPARAM)&DPInfo);
			}
		}
	}
	// Shut down the connection
	hr = ShutdownConnection(&DPInfo);

	// Uninitialize the COM library
	CoUninitialize();

	return iResult;
}
