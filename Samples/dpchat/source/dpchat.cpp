/*==========================================================================
 *
 *  Copyright (C) 1996-1997 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dpchat.cpp
 *  Content:	Simple chat program using DirectPlay.
 *
 ***************************************************************************/

#include <windows.h>
#include <windowsx.h>

#include <stdio.h>

#include "dpchat.h"
#include "dpdialogs.h"
#include "resource.h"

#if defined(UNICODE) || defined(_UNICODE)
#error This app does not support UNICODE
#endif

// constants
const DWORD APPMSG_CHATSTRING = 0; // message type for chat string
const UINT WM_USER_ADDSTRING =
	WM_USER + 257;           // window message to add string to chat string list
const DWORD MAXSTRLEN = 200; // max size of a temporary string

// structures

// message structure used to send a chat string to another player
typedef struct {
	DWORD dwType;  // message type (APPMSG_CHATSTRING)
	char szMsg[1]; // message string (variable length)
} MSG_CHATSTRING, *LPMSG_CHATSTRING;

// globals
HANDLE ghReceiveThread = NULL;    // handle of receive thread
DWORD gidReceiveThread = 0;       // id of receive thread
HANDLE ghKillReceiveEvent = NULL; // event used to kill receive thread
HWND ghChatWnd = NULL;            // main chat window

static HRESULT GetChatPlayerName(
	LPDIRECTPLAY4A lpDirectPlay4A, DPID dpidPlayer, LPDPNAME* lplpName)
{
	LPDPNAME lpName = NULL;
	DWORD dwNameSize;

	// get size of player name data
	HRESULT hr = lpDirectPlay4A->GetPlayerName(dpidPlayer, NULL, &dwNameSize);
	if (hr == DPERR_BUFFERTOOSMALL) {

		// make room for it
		lpName = static_cast<LPDPNAME>(GlobalAllocPtr(GHND, dwNameSize));
		if (lpName == NULL) {
			hr = DPERR_OUTOFMEMORY;
		} else {

			// get player name data
			hr = lpDirectPlay4A->GetPlayerName(dpidPlayer, lpName, &dwNameSize);
			if (SUCCEEDED(hr)) {

				// return pointer to name structure
				*lplpName = lpName;
				return DP_OK;
			}
		}
	}
	if (lpName) {
		GlobalFreePtr(lpName);
	}
	return hr;
}

static HRESULT NewChatString(LPDIRECTPLAY4A lpDirectPlay4A, DPID dpidPlayer,
	LPSTR lpszMsg, LPSTR* lplpszStr)
{
	LPDPNAME lpName = NULL;
	LPSTR lpszStr = NULL;
	LPSTR lpszPlayerName;
	LPSTR szDisplayFormat = "%s>\t%s\r\n";

	// get name of player
	HRESULT hr = GetChatPlayerName(lpDirectPlay4A, dpidPlayer, &lpName);
	if (SUCCEEDED(hr)) {

		if (lpName->lpszShortNameA) {
			lpszPlayerName = lpName->lpszShortNameA;
		} else {
			lpszPlayerName = "unknown";
		}

		// allocate space for display string
		lpszStr = static_cast<LPSTR>(GlobalAllocPtr(GHND,
			strlen(szDisplayFormat) + strlen(lpszPlayerName) + strlen(lpszMsg) +
				1));
		if (lpszStr == NULL) {
			hr = DPERR_OUTOFMEMORY;
		} else {

			// build string
			sprintf(lpszStr, szDisplayFormat, lpszPlayerName, lpszMsg);

			*lplpszStr = lpszStr;
			lpszStr = NULL;
		}
	}
	if (lpszStr) {
		GlobalFreePtr(lpszStr);
	}
	if (lpName) {
		GlobalFreePtr(lpName);
	}
	return hr;
}

static HRESULT SendChatMessage(HWND hWnd, LPDPLAYINFO lpDPInfo)
{
	LPSTR lpszChatStr = NULL;
	LPSTR lpszStr = NULL;
	LPMSG_CHATSTRING lpChatMessage = NULL;
	DWORD dwChatMessageSize;
	HRESULT hr;

	// get length of item text
	LONG lStrLen = SendDlgItemMessageA(
		hWnd, IDC_SENDEDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);

	// make room for it
	lpszChatStr = (LPSTR)GlobalAllocPtr(GHND, static_cast<SIZE_T>(lStrLen + 1));
	if (lpszChatStr == NULL) {
		hr = DPERR_OUTOFMEMORY;
	} else {

		// get item text
		lStrLen = static_cast<LONG>(
			GetDlgItemTextA(hWnd, IDC_SENDEDIT, lpszChatStr, lStrLen + 1));

		// create string to display this text
		hr = NewChatString(lpDPInfo->lpDirectPlay4A, lpDPInfo->dpidPlayer,
			lpszChatStr, &lpszStr);
		if (SUCCEEDED(hr)) {

			// display this string
			PostMessageA(hWnd, WM_USER_ADDSTRING, (WPARAM)0, (LPARAM)lpszStr);
			lpszStr = NULL; // set to NULL so we don't delete it below

			// create space for message plus string (string length included in
			// message header)
			dwChatMessageSize = sizeof(MSG_CHATSTRING) + strlen(lpszChatStr);
			lpChatMessage =
				(LPMSG_CHATSTRING)GlobalAllocPtr(GHND, dwChatMessageSize);
			if (lpChatMessage == NULL) {
				hr = DPERR_OUTOFMEMORY;
			} else {

				// build message
				lpChatMessage->dwType = APPMSG_CHATSTRING;
				strcpy(lpChatMessage->szMsg, lpszChatStr);

				// send this string to all other players
				hr = lpDPInfo->lpDirectPlay4A->Send(lpDPInfo->dpidPlayer,
					DPID_ALLPLAYERS, DPSEND_GUARANTEED, lpChatMessage,
					dwChatMessageSize);
				if (SUCCEEDED(hr)) {
				}
			}
		}
	}
	if (lpszChatStr) {
		GlobalFreePtr(lpszChatStr);
	}
	if (lpszStr) {
		GlobalFreePtr(lpszStr);
	}
	if (lpChatMessage) {
		GlobalFreePtr(lpChatMessage);
	}
	SetDlgItemTextA(hWnd, IDC_SENDEDIT, "");

	return hr;
}

static BOOL CALLBACK ChatWndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static LPDPLAYINFO lpDPInfo;
	DWORD dwTextLen;

	switch (uMsg) {
	case WM_INITDIALOG:
		// Save the connection info pointer
		lpDPInfo = (LPDPLAYINFO)lParam;

		// store global window
		ghChatWnd = hWnd;
		break;

	case WM_DESTROY:
		ghChatWnd = NULL;
		break;

	// this is a user-defined message used to add strings to the log window
	case WM_USER_ADDSTRING:
		// get length of text in log window
		dwTextLen = static_cast<DWORD>(SendDlgItemMessageA(
			hWnd, IDC_LOGEDIT, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0));

		// put selection at end
		dwTextLen = static_cast<DWORD>(SendDlgItemMessageA(hWnd, IDC_LOGEDIT,
			EM_SETSEL, (WPARAM)dwTextLen, (LPARAM)dwTextLen));

		// add string in lParam to log window
		SendDlgItemMessageA(
			hWnd, IDC_LOGEDIT, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)lParam);
		GlobalFreePtr((LPVOID)lParam);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SENDBUTTON:
			SendChatMessage(hWnd, lpDPInfo);
			break;

		case IDCANCEL:
			EndDialog(hWnd, FALSE);
			break;
		}
		break;
	}

	// Allow for default processing
	return FALSE;
}

static void HandleSystemMessage(LPDPLAYINFO lpDPInfo, LPDPMSG_GENERIC lpMsg,
	DWORD /* dwMsgSize */, DPID /* idFrom */, DPID /* idTo */)
{
	LPSTR lpszStr = NULL;

	// The body of each case is there so you can set a breakpoint and examine
	// the contents of the message received.
	switch (lpMsg->dwType) {
	case DPSYS_CREATEPLAYERORGROUP: {
		LPDPMSG_CREATEPLAYERORGROUP lp = (LPDPMSG_CREATEPLAYERORGROUP)lpMsg;
		LPSTR lpszPlayerName;
		LPSTR szDisplayFormat = "\"%s\" has joined\r\n";

		// get pointer to player name
		if (lp->dpnName.lpszShortNameA) {
			lpszPlayerName = lp->dpnName.lpszShortNameA;
		} else {
			lpszPlayerName = "unknown";
		}

		// allocate space for string
		lpszStr = (LPSTR)GlobalAllocPtr(
			GHND, strlen(szDisplayFormat) + strlen(lpszPlayerName) + 1);
		if (lpszStr == NULL) {
			break;
		}

		// build string
		sprintf(lpszStr, szDisplayFormat, lpszPlayerName);
	} break;

	case DPSYS_DESTROYPLAYERORGROUP: {
		LPDPMSG_DESTROYPLAYERORGROUP lp = (LPDPMSG_DESTROYPLAYERORGROUP)lpMsg;
		LPSTR lpszPlayerName;
		LPSTR szDisplayFormat = "\"%s\" has left\r\n";

		// get pointer to player name
		if (lp->dpnName.lpszShortNameA) {
			lpszPlayerName = lp->dpnName.lpszShortNameA;
		} else {
			lpszPlayerName = "unknown";
		}

		// allocate space for string
		lpszStr = (LPSTR)GlobalAllocPtr(
			GHND, strlen(szDisplayFormat) + strlen(lpszPlayerName) + 1);
		if (lpszStr == NULL) {
			break;
		}

		// build string
		sprintf(lpszStr, szDisplayFormat, lpszPlayerName);
	} break;

	case DPSYS_ADDPLAYERTOGROUP: {
		// LPDPMSG_ADDPLAYERTOGROUP lp = (LPDPMSG_ADDPLAYERTOGROUP)lpMsg;
	} break;

	case DPSYS_DELETEPLAYERFROMGROUP: {
		// LPDPMSG_DELETEPLAYERFROMGROUP lp =
		// (LPDPMSG_DELETEPLAYERFROMGROUP)lpMsg;
	} break;

	case DPSYS_SESSIONLOST: {
		// LPDPMSG_SESSIONLOST lp = (LPDPMSG_SESSIONLOST)lpMsg;
	} break;

	case DPSYS_HOST: {
		// LPDPMSG_HOST lp = (LPDPMSG_HOST)lpMsg;
		LPSTR szDisplayFormat = "You have become the host\r\n";

		// allocate space for string
		lpszStr = (LPSTR)GlobalAllocPtr(GHND, strlen(szDisplayFormat) + 1);
		if (lpszStr == NULL) {
			break;
		}

		// build string
		strcpy(lpszStr, szDisplayFormat);

		// we are now the host
		lpDPInfo->bIsHost = TRUE;
	} break;

	case DPSYS_SETPLAYERORGROUPDATA: {
		// LPDPMSG_SETPLAYERORGROUPDATA lp =
		// (LPDPMSG_SETPLAYERORGROUPDATA)lpMsg;
	} break;

	case DPSYS_SETPLAYERORGROUPNAME: {
		// LPDPMSG_SETPLAYERORGROUPNAME lp =
		// (LPDPMSG_SETPLAYERORGROUPNAME)lpMsg;
	} break;
	}

	// post string to chat window
	if (lpszStr) {
		// make sure window is still valid
		if (ghChatWnd) {
			PostMessageA(
				ghChatWnd, WM_USER_ADDSTRING, (WPARAM)0, (LPARAM)lpszStr);
		} else {
			GlobalFreePtr(lpszStr);
		}
	}
}

static void HandleApplicationMessage(LPDPLAYINFO lpDPInfo,
	LPDPMSG_GENERIC lpMsg, DWORD /* dwMsgSize */, DPID idFrom, DPID /* idTo */)
{
	LPSTR lpszStr = NULL;
	HRESULT hr;

	switch (lpMsg->dwType) {
	case APPMSG_CHATSTRING: {
		LPMSG_CHATSTRING lp = (LPMSG_CHATSTRING)lpMsg;

		// create string to display
		hr = NewChatString(
			lpDPInfo->lpDirectPlay4A, idFrom, lp->szMsg, &lpszStr);
		if (FAILED(hr)) {
			break;
		}
	} break;
	}

	// post string to chat window
	if (lpszStr) {
		// make sure window is still valid
		if (ghChatWnd) {
			PostMessageA(
				ghChatWnd, WM_USER_ADDSTRING, (WPARAM)0, (LPARAM)lpszStr);
		} else {
			GlobalFreePtr(lpszStr);
		}
	}
}

static HRESULT ReceiveMessage(LPDPLAYINFO lpDPInfo)
{
	DPID idFrom, idTo;
	HRESULT hr;

	LPVOID lpvMsgBuffer = NULL;
	DWORD dwMsgBufferSize = 0;

	// loop to read all messages in queue
	do {
		// loop until a single message is successfully read
		do {
			// read messages from any player, including system player
			idFrom = 0;
			idTo = 0;

			hr = lpDPInfo->lpDirectPlay4A->Receive(
				&idFrom, &idTo, DPRECEIVE_ALL, lpvMsgBuffer, &dwMsgBufferSize);

			// not enough room, so resize buffer
			if (hr == DPERR_BUFFERTOOSMALL) {
				if (lpvMsgBuffer) {
					GlobalFreePtr(lpvMsgBuffer);
				}
				lpvMsgBuffer = GlobalAllocPtr(GHND, dwMsgBufferSize);
				if (lpvMsgBuffer == NULL) {
					hr = DPERR_OUTOFMEMORY;
				}
			}
		} while (hr == DPERR_BUFFERTOOSMALL);

		if ((SUCCEEDED(hr)) && // successfully read a message
			(dwMsgBufferSize >= sizeof(DPMSG_GENERIC))) // and it is big enough
		{
			// check for system message
			if (idFrom == DPID_SYSMSG) {
				HandleSystemMessage(lpDPInfo, (LPDPMSG_GENERIC)lpvMsgBuffer,
					dwMsgBufferSize, idFrom, idTo);
			} else {
				HandleApplicationMessage(lpDPInfo,
					(LPDPMSG_GENERIC)lpvMsgBuffer, dwMsgBufferSize, idFrom,
					idTo);
			}
		}
	} while (SUCCEEDED(hr));

	// free any memory we created
	if (lpvMsgBuffer) {
		GlobalFreePtr(lpvMsgBuffer);
	}
	return DP_OK;
}

static DWORD WINAPI ReceiveThread(LPVOID lpThreadParameter)
{
	LPDPLAYINFO lpDPInfo = (LPDPLAYINFO)lpThreadParameter;
	HANDLE eventHandles[2];

	eventHandles[0] = lpDPInfo->hPlayerEvent;
	eventHandles[1] = ghKillReceiveEvent;

	// loop waiting for player events. If the kill event is signaled
	// the thread will exit
	while (WaitForMultipleObjects(2, eventHandles, FALSE, INFINITE) ==
		WAIT_OBJECT_0) {
		// receive any messages in the queue
		ReceiveMessage(lpDPInfo);
	}

	ExitThread(0);
	return 0;
}

static HRESULT ShutdownConnection(LPDPLAYINFO lpDPInfo)
{
	if (ghReceiveThread) {
		// wake up receive thread and wait for it to quit
		SetEvent(ghKillReceiveEvent);
		WaitForSingleObject(ghReceiveThread, INFINITE);

		CloseHandle(ghReceiveThread);
		ghReceiveThread = NULL;
	}

	if (ghKillReceiveEvent) {
		CloseHandle(ghKillReceiveEvent);
		ghKillReceiveEvent = NULL;
	}

	if (lpDPInfo->lpDirectPlay4A) {
		if (lpDPInfo->dpidPlayer) {
			lpDPInfo->lpDirectPlay4A->DestroyPlayer(lpDPInfo->dpidPlayer);
			lpDPInfo->dpidPlayer = 0;
		}
		lpDPInfo->lpDirectPlay4A->Close();
		lpDPInfo->lpDirectPlay4A->Release();
		lpDPInfo->lpDirectPlay4A = NULL;
	}

	if (lpDPInfo->hPlayerEvent) {
		CloseHandle(lpDPInfo->hPlayerEvent);
		lpDPInfo->hPlayerEvent = NULL;
	}

	return DP_OK;
}

static HRESULT SetupConnection(HINSTANCE hInstance, LPDPLAYINFO lpDPInfo)
{
	HRESULT hr;

	ZeroMemory(lpDPInfo, sizeof(DPLAYINFO));

	// create event used by DirectPlay to signal a message has arrived
	lpDPInfo->hPlayerEvent = CreateEvent(NULL, // no security
		FALSE,                                 // auto reset
		FALSE,                                 // initial event reset
		NULL);                                 // no name
	if (lpDPInfo->hPlayerEvent == NULL) {
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
				lpDPInfo,                        // argument for thread
				0,                               // start it right away
				&gidReceiveThread);
			if (ghReceiveThread == NULL) {
				hr = DPERR_NOMEMORY;
			} else {

				// try to connect using the lobby
				hr = ConnectUsingLobby(lpDPInfo);
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
				hr = ConnectUsingDialog(hInstance, lpDPInfo);
				if (SUCCEEDED(hr)) {
					return DP_OK;
				}
			}
		}
	}

	ShutdownConnection(lpDPInfo);
	return hr;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
	LPSTR /* lpCmdLine */, int /* nCmdShow */)
{
	DPLAYINFO DPInfo;
	int iResult = 0;

	// Initialize COM library
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {

		// setup the connection
		hr = SetupConnection(hInstance, &DPInfo);
		if (SUCCEEDED(hr)) {

			// show the chat window
			iResult =
				DialogBoxParamA(hInstance, MAKEINTRESOURCEA(IDD_CHATDIALOG),
					NULL, (DLGPROC)ChatWndProc, (LPARAM)&DPInfo);
		}
	}
	// shut down the connection
	hr = ShutdownConnection(&DPInfo);

	// Uninitialize the COM library
	CoUninitialize();

	return iResult;
}
