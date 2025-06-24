/*==========================================================================
 *
 *  Copyright (C) 1996-1997 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dialog.cpp
 *  Content:    Creates a dialog to query the user for connection settings
 *                              and establish a connection.
 *
 ***************************************************************************/

#define INITGUID

#include <windows.h>
#include <windowsx.h>
#include <wtypes.h>

#include <cguid.h>

#include "dpchat.h"
#include "resource.h"
#include <dplobby.h> // to init guids

// constants
const DWORD MAXNAMELEN = 200;    // max size of a session or player name
const UINT TIMERID = 1;          // timer ID to use
const UINT TIMERINTERVAL = 3000; // timer interval

static void EnableDlgButton(HWND hDlg, int nIDDlgItem, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hDlg, nIDDlgItem), bEnable);
}

static BOOL FAR PASCAL DirectPlayEnumConnectionsCallback(LPCGUID /* lpguidSP */,
	LPVOID lpConnection, DWORD dwConnectionSize, LPCDPNAME lpName,
	DWORD /* dwFlags */, LPVOID lpContext)
{

	HWND hWnd = (HWND)lpContext;
	LRESULT iIndex;
	LPVOID lpConnectionBuffer;

	// store service provider name in combo box
	iIndex = SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_ADDSTRING, 0, (LPARAM)lpName->lpszShortNameA);
	if (iIndex != CB_ERR) {

		// make space for connection shortcut
		lpConnectionBuffer = GlobalAllocPtr(GHND, dwConnectionSize);
		if (lpConnectionBuffer != NULL) {

			// store pointer to connection shortcut in combo box
			memcpy(lpConnectionBuffer, lpConnection, dwConnectionSize);
			SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_SETITEMDATA,
				(WPARAM)iIndex, (LPARAM)lpConnectionBuffer);
		}
	}
	return TRUE;
}

static HRESULT CreateDirectPlayInterface(LPDIRECTPLAY4A* lplpDirectPlay4A)
{
	// Create an IDirectPlay interface
	return CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
		IID_IDirectPlay4A, (LPVOID*)lplpDirectPlay4A);
}

static void DeleteSessionInstanceList(HWND hWnd)
{
	LONG lpData;

	// destroy the GUID's stored with each session name
	WPARAM i = 0;
	for (;;) {
		// get data pointer stored with item
		lpData = SendDlgItemMessageA(
			hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
		if (lpData == CB_ERR) { // error getting data
			break;
		}

		if (lpData == 0) { // no data to delete
			continue;
		}

		GlobalFreePtr((LPVOID)lpData);
		i += 1;
	}

	// delete all items in list
	SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
}

static HRESULT DestroyDirectPlayInterface(
	HWND hWnd, LPDIRECTPLAY4A lpDirectPlay4A)
{
	HRESULT hr = DP_OK;

	if (lpDirectPlay4A) {
		DeleteSessionInstanceList(hWnd);
		EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);

		hr = static_cast<HRESULT>(lpDirectPlay4A->Release());
	}

	return hr;
}

static HRESULT HostSession(LPDIRECTPLAY4A lpDirectPlay4A, LPSTR lpszSessionName,
	LPSTR lpszPlayerName, LPDPLAYINFO lpDPInfo)
{
	DPID dpidPlayer;
	DPNAME dpName;
	DPSESSIONDESC2 sessionDesc;
	HRESULT hr;

	// check for valid interface
	if (lpDirectPlay4A == NULL) {
		return DPERR_INVALIDOBJECT;
	}

	// host a new session
	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.dwFlags = DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
	sessionDesc.guidApplication = DPCHAT_GUID;
	sessionDesc.dwMaxPlayers = MAXPLAYERS;
	sessionDesc.lpszSessionNameA = lpszSessionName;

	hr = lpDirectPlay4A->Open(&sessionDesc, DPOPEN_CREATE);
	if (SUCCEEDED(hr)) {

		// fill out name structure
		ZeroMemory(&dpName, sizeof(DPNAME));
		dpName.dwSize = sizeof(DPNAME);
		dpName.lpszShortNameA = lpszPlayerName;
		dpName.lpszLongNameA = NULL;

		// create a player with this name
		hr = lpDirectPlay4A->CreatePlayer(
			&dpidPlayer, &dpName, lpDPInfo->hPlayerEvent, NULL, 0, 0);
		if (SUCCEEDED(hr)) {

			// return connection info
			lpDPInfo->lpDirectPlay4A = lpDirectPlay4A;
			lpDPInfo->dpidPlayer = dpidPlayer;
			lpDPInfo->bIsHost = TRUE;

			return DP_OK;
		}
	}
	lpDirectPlay4A->Close();
	return (hr);
}

static HRESULT JoinSession(LPDIRECTPLAY4A lpDirectPlay4A,
	LPGUID lpguidSessionInstance, LPSTR lpszPlayerName, LPDPLAYINFO lpDPInfo)
{
	DPID dpidPlayer;
	DPNAME dpName;
	DPSESSIONDESC2 sessionDesc;

	// check for valid interface
	if (lpDirectPlay4A == NULL) {
		return DPERR_INVALIDOBJECT;
	}

	// join existing session
	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.guidInstance = *lpguidSessionInstance;

	HRESULT hr = lpDirectPlay4A->Open(&sessionDesc, DPOPEN_JOIN);
	if (SUCCEEDED(hr)) {

		// fill out name structure
		ZeroMemory(&dpName, sizeof(DPNAME));
		dpName.dwSize = sizeof(DPNAME);
		dpName.lpszShortNameA = lpszPlayerName;
		dpName.lpszLongNameA = NULL;

		// create a player with this name
		hr = lpDirectPlay4A->CreatePlayer(
			&dpidPlayer, &dpName, lpDPInfo->hPlayerEvent, NULL, 0, 0);
		if (SUCCEEDED(hr)) {

			// return connection info
			lpDPInfo->lpDirectPlay4A = lpDirectPlay4A;
			lpDPInfo->dpidPlayer = dpidPlayer;
			lpDPInfo->bIsHost = FALSE;

			return DP_OK;
		}
	}
	lpDirectPlay4A->Close();
	return hr;
}

static BOOL FAR PASCAL EnumSessionsCallback(LPCDPSESSIONDESC2 lpSessionDesc,
	LPDWORD /* lpdwTimeOut */, DWORD dwFlags, LPVOID lpContext)
{
	HWND hWnd = (HWND)lpContext;
	LPGUID lpGuid;

	// see if last session has been enumerated
	if (dwFlags & DPESC_TIMEDOUT) {
		return FALSE;
	}

	// store session name in list
	LONG iIndex = SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_ADDSTRING,
		(WPARAM)0, (LPARAM)lpSessionDesc->lpszSessionNameA);

	if (iIndex != LB_ERR) {

		// make space for session instance guid
		lpGuid = (LPGUID)GlobalAllocPtr(GHND, sizeof(GUID));
		if (lpGuid != NULL) {

			// store pointer to guid in list
			*lpGuid = lpSessionDesc->guidInstance;
			SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_SETITEMDATA,
				(WPARAM)iIndex, (LPARAM)lpGuid);
		}
	}
	return TRUE;
}

static HRESULT GetSessionInstanceGuid(HWND hWnd, LPGUID lpguidSessionInstance)
{
	// get guid for session
	LONG iIndex = SendDlgItemMessage(
		hWnd, IDC_SESSIONLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (iIndex == LB_ERR) {
		return DPERR_GENERIC;
	}

	iIndex = SendDlgItemMessage(
		hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, (WPARAM)iIndex, (LPARAM)0);
	if ((iIndex == LB_ERR) || (iIndex == 0)) {
		return DPERR_GENERIC;
	}

	*lpguidSessionInstance = *((LPGUID)iIndex);

	return DP_OK;
}

static void SelectSessionInstance(HWND hWnd, LPGUID lpguidSessionInstance)
{
	// loop over the GUID's stored with each session name
	// to find the one that matches what was passed in
	WPARAM i = 0;
	WPARAM iIndex = 0;
	for (;;) {
		// get data pointer stored with item
		LONG lpData = SendDlgItemMessageA(
			hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
		if (lpData == CB_ERR) { // error getting data
			break;
		}

		if (lpData == 0) { // no data to compare to
			continue;
		}

		// guid matches
		if (IsEqualGUID(*lpguidSessionInstance, *((LPGUID)lpData))) {
			iIndex = i; // store index of this string
			break;
		}

		i += 1;
	}

	// select this item
	SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_SETCURSEL, (WPARAM)iIndex, (LPARAM)0);
}

static HRESULT EnumSessions(HWND hWnd, LPDIRECTPLAY4A lpDirectPlay4A)
{
	DPSESSIONDESC2 sessionDesc;

	// check for valid interface
	if (lpDirectPlay4A == NULL) {
		return DPERR_INVALIDOBJECT;
	}

	// get guid of currently selected session
	GUID guidSessionInstance = GUID_NULL;
	HRESULT hr = GetSessionInstanceGuid(hWnd, &guidSessionInstance);

	// delete existing session list
	DeleteSessionInstanceList(hWnd);

	// add sessions to session list
	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.guidApplication = DPCHAT_GUID;

	hr = lpDirectPlay4A->EnumSessions(&sessionDesc, 0, EnumSessionsCallback,
		hWnd, DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC);

	// select the session that was previously selected
	SelectSessionInstance(hWnd, &guidSessionInstance);

	// hilite "Join" button only if there are sessions to join
	LONG iIndex = SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

	EnableDlgButton(hWnd, IDC_JOINBUTTON, (iIndex > 0) ? TRUE : FALSE);

	return hr;
}

static HRESULT GetConnection(HWND hWnd, LPVOID* lplpConnection)
{
	// get index of the item currently selected in the combobox
	LONG iIndex = SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (iIndex == CB_ERR) {
		return DPERR_GENERIC;
	}

	// get the pointer to the connection shortcut associated with
	// the item
	iIndex = SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_GETITEMDATA, (WPARAM)iIndex, (LPARAM)0);
	if (iIndex == CB_ERR) {
		return DPERR_GENERIC;
	}

	*lplpConnection = (LPVOID)iIndex;
	return DP_OK;
}

static void DeleteConnectionList(HWND hWnd)
{
	// destroy the GUID's stored with each service provider name
	WPARAM i = 0;
	for (;;) {
		// get data pointer stored with item
		LONG lpData = SendDlgItemMessageA(
			hWnd, IDC_SPCOMBO, CB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
		if (lpData == CB_ERR) { // error getting data
			break;
		}

		if (lpData != 0) { // no data to delete
			GlobalFreePtr((LPVOID)lpData);
		}
		i += 1;
	}

	// delete all items in combo box
	SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
}

static BOOL CALLBACK ConnectWndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static LPDPLAYINFO lpDPInfo;
	static LPDIRECTPLAY4A lpDirectPlay4A;
	static UINT idTimer = 0;
	GUID guidSessionInstance;
	char szSessionName[MAXNAMELEN];
	char szPlayerName[MAXNAMELEN];
	DWORD dwNameSize;
	HRESULT hr;
	LPVOID lpConnection = NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
		// save the connection info pointer
		lpDPInfo = (LPDPLAYINFO)lParam;
		lpDirectPlay4A = NULL;

		// Create an IDirectPlay3 interface
		hr = CreateDirectPlayInterface(&lpDirectPlay4A);
		if (SUCCEEDED(hr)) {

			// set first item in the connections combo box
			SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_ADDSTRING, (WPARAM)0,
				(LPARAM) "<Select a service provider>");
			SendDlgItemMessageA(
				hWnd, IDC_SPCOMBO, CB_SETITEMDATA, (WPARAM)0, (LPARAM)0);
			SendDlgItemMessageA(
				hWnd, IDC_SPCOMBO, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

			// put all the available connections in a combo box
			lpDirectPlay4A->EnumConnections(
				&DPCHAT_GUID, DirectPlayEnumConnectionsCallback, hWnd, 0);

			// setup initial button state
			EnableDlgButton(hWnd, IDC_HOSTBUTTON, FALSE);
			EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);
			break;
		}
		MessageBoxA(
			NULL, "This application requires DirectX 5 or later.", NULL, MB_OK);
		EndDialog(hWnd, FALSE);
		break;

	case WM_DESTROY:
		// delete information stored along with the lists
		DeleteConnectionList(hWnd);
		DeleteSessionInstanceList(hWnd);
		break;

	case WM_TIMER:
		// refresh the session list
		// guard against leftover timer messages after timer has been killed
		if (idTimer) {
			hr = EnumSessions(hWnd, lpDirectPlay4A);
			if (FAILED(hr)) {
				if (hr == DPERR_USERCANCEL) {
					KillTimer(hWnd, idTimer);
					idTimer = 0;
				} else if (hr != DPERR_CONNECTING) {
					ErrorBox("Enumerating sessions has failed; error %s", hr);
					if (MessageBoxA(NULL, "Retry enumerating sessions?",
							"Continue?", MB_OKCANCEL) == IDCANCEL) {
						KillTimer(hWnd, idTimer);
						idTimer = 0;
					}
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SPCOMBO:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
				// service provider changed, so rebuild display and
				// delete any existing DirectPlay interface
				if (idTimer) {
					KillTimer(hWnd, idTimer);
					idTimer = 0;
				}
				hr = DestroyDirectPlayInterface(hWnd, lpDirectPlay4A);
				lpDirectPlay4A = NULL;

				// get pointer to the selected connection
				hr = GetConnection(hWnd, &lpConnection);
				if (SUCCEEDED(hr)) {

					if (lpConnection) {
						/*
						 * Create a new DPlay interface.
						 */

						hr = CreateDirectPlayInterface(&lpDirectPlay4A);

						if ((SUCCEEDED(hr)) && (NULL != lpDirectPlay4A)) {

							// initialize the connection
							hr = lpDirectPlay4A->InitializeConnection(
								lpConnection, 0);
							if (SUCCEEDED(hr)) {

								// OK to host now
								EnableDlgButton(hWnd, IDC_HOSTBUTTON, TRUE);

								// start enumerating the sessions
								hr = EnumSessions(hWnd, lpDirectPlay4A);
								if (SUCCEEDED(hr)) {
									// set a timer to refresh the session list
									idTimer = SetTimer(
										hWnd, TIMERID, TIMERINTERVAL, NULL);
									break;
								}
							}
						}
					} else {
						// They've selected the generic option "<Select a
						// service provider>"
						EnableDlgButton(hWnd, IDC_HOSTBUTTON, FALSE);
						EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);
						break;
					}
				}

				if (hr != DPERR_USERCANCEL) {
					ErrorBox(
						"Could not select service provider because of error %s",
						hr);
				}
				break;
			}
			break;

		case IDC_HOSTBUTTON:
			// should have an interface by now
			if (lpDirectPlay4A == NULL) {
				break;
			}

			if (idTimer) {
				KillTimer(hWnd, idTimer);
				idTimer = 0;
			}
			// use computer name for session name
			dwNameSize = MAXNAMELEN;
			if (!GetComputerNameA(szSessionName, &dwNameSize)) {
				strcpy(szSessionName, "Session");
			}

			// use user name for player name
			dwNameSize = MAXNAMELEN;
			if (!GetUserNameA(szPlayerName, &dwNameSize)) {
				strcpy(szPlayerName, "unknown");
			}

			// host a new session on this service provider
			hr = HostSession(
				lpDirectPlay4A, szSessionName, szPlayerName, lpDPInfo);
			if (SUCCEEDED(hr)) {

				// dismiss dialog if we succeeded in hosting
				EndDialog(hWnd, TRUE);
				break;
			}
			ErrorBox("Could not host session because of error %s", hr);
			break;

		case IDC_JOINBUTTON:

			// should have an interface by now
			if (lpDirectPlay4A == NULL) {
				break;
			}

			if (idTimer) {
				KillTimer(hWnd, idTimer);
				idTimer = 0;
			}
			// get guid of selected session instance
			hr = GetSessionInstanceGuid(hWnd, &guidSessionInstance);
			if (SUCCEEDED(hr)) {

				// use user name for player name
				dwNameSize = MAXNAMELEN;
				if (!GetUserNameA(szPlayerName, &dwNameSize)) {
					strcpy(szPlayerName, "unknown");
				}

				// join this session
				hr = JoinSession(lpDirectPlay4A, &guidSessionInstance,
					szPlayerName, lpDPInfo);

				if (SUCCEEDED(hr)) {

					// dismiss dialog if we succeeded in joining
					EndDialog(hWnd, TRUE);
					break;
				}
			}
			ErrorBox("Could not join session because of error %s", hr);
			break;

		case IDCANCEL:
			if (idTimer) {
				KillTimer(hWnd, idTimer);
				idTimer = 0;
			}
			// delete any interface created if cancelling
			hr = DestroyDirectPlayInterface(hWnd, lpDirectPlay4A);
			lpDirectPlay4A = NULL;

			EndDialog(hWnd, FALSE);
			break;
		}

		break;
	}

	// Allow for default processing
	return FALSE;
}

HRESULT ConnectUsingDialog(HINSTANCE hInstance, LPDPLAYINFO lpDPInfo)
{
	// ask user for connection settings
	if (DialogBoxParamA(hInstance, MAKEINTRESOURCE(IDD_CONNECTDIALOG), NULL,
			(DLGPROC)ConnectWndProc, (LPARAM)lpDPInfo)) {
		return DP_OK;
	}
	return DPERR_USERCANCEL;
}
