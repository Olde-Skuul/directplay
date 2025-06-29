//-----------------------------------------------------------------------------
// File: Dialog.cpp
//
// Desc: A dialog to query the user for connection settings and establish a
//       connection.
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#include "dialog.h"
#include "bellhop.h"
#include "resource.h"

#include <cguid.h>
#include <stdio.h>
#include <windowsx.h>
#include <wtypes.h>

// constants
const DWORD MAXNAMELEN = 200;    // max size of a session or player name
const UINT TIMERID = 1;          // timer ID to use
const UINT TIMERINTERVAL = 1000; // timer interval

//-----------------------------------------------------------------------------
// Name: EnableDlgButton()
// Desc:
//-----------------------------------------------------------------------------
static VOID EnableDlgButton(HWND hDlg, int nIDDlgItem, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hDlg, nIDDlgItem), bEnable);
}

//-----------------------------------------------------------------------------
// Name: SecurityCredentialsWndProc()
// Desc:
//-----------------------------------------------------------------------------
static INT_PTR CALLBACK SecurityCredentialsWndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DPLAYINFO* pDPInfo;
	HWND hwndName = NULL;
	HWND hwndPassword = NULL;
	HWND hwndDomain = NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
		pDPInfo = (DPLAYINFO*)lParam;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			hwndName = GetDlgItem(hWnd, IDC_SECURENAME);
			hwndPassword = GetDlgItem(hWnd, IDC_SECUREPASSWORD);
			hwndDomain = GetDlgItem(hWnd, IDC_SECUREDOMAIN);

			Edit_GetText(hwndName, pDPInfo->strSecureName, 256);
			Edit_GetText(hwndPassword, pDPInfo->strSecurePassword, 256);
			Edit_GetText(hwndDomain, pDPInfo->strSecureDomain, 256);
			EndDialog(hWnd, TRUE);
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

//-----------------------------------------------------------------------------
// Name: JoinSession()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT JoinSession(HWND hWnd, LPDIRECTPLAY4A pDP,
	LPDIRECTPLAYLOBBY3A pDPLobby, GUID* pguidSessionInstance,
	DWORD dwSessionFlags, LPSTR strPlayerName, DWORD dwPlayerFlags,
	DPLAYINFO* pDPInfo)
{
	DPID dpidPlayer;
	DPNAME dpName;
	DPSESSIONDESC2 sessionDesc;
	HRESULT hr;

	// Check for valid interface
	if (pDP == NULL) {
		return DPERR_INVALIDOBJECT;
	}

	// Spectator or regular player
	pDPInfo->dwPlayerFlags = dwPlayerFlags;

	// Prepare a session description
	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.guidInstance = *pguidSessionInstance;
	sessionDesc.dwFlags = dwSessionFlags;

	if (DPSESSION_SECURESERVER & dwSessionFlags) {
		hr = pDP->SecureOpen(&sessionDesc, DPOPEN_JOIN, NULL, NULL);

		if (DPERR_LOGONDENIED == hr) {
			// We need to collect security credentials and try again.
			if (DialogBoxParamA(g_hInstance,
					MAKEINTRESOURCEA(IDD_SECURITYCREDENTIALSDIALOG), hWnd,
					SecurityCredentialsWndProc, (LPARAM)&pDPInfo)) {
				DPCREDENTIALS dpcr;
				dpcr.dwSize = sizeof(DPCREDENTIALS);
				dpcr.dwFlags = 0;
				dpcr.lpszUsernameA = pDPInfo->strSecureName;
				dpcr.lpszPasswordA = pDPInfo->strSecurePassword;
				dpcr.lpszDomainA = pDPInfo->strSecureDomain;

				hr = pDP->SecureOpen(&sessionDesc, DPOPEN_JOIN, NULL, &dpcr);
				if (FAILED(hr)) {
					// Conceivably, we could cycle back and try to get
					// credentials again but in this sample, we'll just drop out
					// on the error.
					pDP->Close();
					return hr;
				}

				pDPInfo->bSecureSession = TRUE;
			} else {
				// abort. user clicked cancel.
				pDP->Close();
				return hr;
			}
		}
	} else {
		// Session does not require security
		hr = pDP->Open(&sessionDesc, DPOPEN_JOIN);
		if (FAILED(hr)) {
			pDP->Close();
			return hr;
		}
	}

	// Fill out name structure
	ZeroMemory(&dpName, sizeof(DPNAME));
	dpName.dwSize = sizeof(DPNAME);
	dpName.lpszShortNameA = strPlayerName;
	dpName.lpszLongNameA = NULL;

	// Create a player with this name
	hr = pDP->CreatePlayer(
		&dpidPlayer, &dpName, pDPInfo->hPlayerEvent, NULL, 0, dwPlayerFlags);
	if (FAILED(hr)) {
		pDP->Close();
		return hr;
	}

	// Return connection info
	pDPInfo->pDP = pDP;
	pDPInfo->pDPLobby = pDPLobby;
	pDPInfo->dpidPlayer = dpidPlayer;
	pDPInfo->bIsHost = FALSE;

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: GetSessionInfo()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT GetSessionInfo(
	HWND hWnd, GUID* pguidSessionInstance, DWORD* pdwFlags)
{
	// Get guid for session
	LONG iIndex =
		SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_GETCURSEL, 0, 0);
	if (iIndex == LB_ERR) {
		return DPERR_GENERIC;
	}
	iIndex = SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, (WPARAM)iIndex, 0);
	if ((iIndex == LB_ERR) || (iIndex == 0)) {
		return DPERR_GENERIC;
	}

	SESSIONINFO* p = (SESSIONINFO*)iIndex;
	*pguidSessionInstance = p->guidInstance;
	*pdwFlags = p->dwFlags;

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteConnectionList()
// Desc:
//-----------------------------------------------------------------------------
static VOID DeleteConnectionList(HWND hWnd)
{
	WPARAM i = 0;

	// Destroy the GUID's stored with each service provider name
	for (;;) {
		// Get data pointer stored with item
		LONG pData = SendDlgItemMessageA(
			hWnd, IDC_SPCOMBO, CB_GETITEMDATA, (WPARAM)i, 0);
		if (pData == CB_ERR) {
			break;
		}

		if (pData != 0) {
			GlobalFreePtr((VOID*)pData);
		}
		i++;
	}

	// Delete all items in combo box
	SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_RESETCONTENT, 0, 0);
}

//-----------------------------------------------------------------------------
// Name: DeleteSessionInstanceList()
// Desc:
//-----------------------------------------------------------------------------
static VOID DeleteSessionInstanceList(HWND hWnd)
{
	WPARAM i = 0;

	// Destroy the GUID's stored with each session name
	for (;;) {
		// Get data pointer stored with item
		LONG pData = SendDlgItemMessageA(
			hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, (WPARAM)i, 0);
		if (pData == CB_ERR) {
			break;
		}

		if (pData == 0) {
			continue;
		}

		GlobalFreePtr((VOID*)pData);
		i++;
	}

	// Delete all items in list
	SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_RESETCONTENT, 0, 0);
}

//-----------------------------------------------------------------------------
// Name: EnumSessionsCallback()
// Desc:
//-----------------------------------------------------------------------------
static BOOL FAR PASCAL EnumSessionsCallback(const DPSESSIONDESC2* pSessionDesc,
	DWORD* /* pdwTimeOut */, DWORD dwFlags, VOID* pContext)
{
	HWND hWnd = (HWND)pContext;
	CHAR strBuffer[256];

	// See if last session has been enumerated
	if (dwFlags & DPESC_TIMEDOUT) {
		return FALSE;
	}

	if (DPSESSION_SECURESERVER & pSessionDesc->dwFlags) {
		sprintf(strBuffer, "%s (SECURE)", pSessionDesc->lpszSessionNameA);
	} else {
		sprintf(strBuffer, "%s", pSessionDesc->lpszSessionNameA);
	}

	// Store session name in list
	LONG iIndex = SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_ADDSTRING, 0, (LPARAM)strBuffer);

	if (iIndex == LB_ERR) {
		return TRUE;
	}

	// Make space for session instance guid
	SESSIONINFO* pSessionInfo =
		(SESSIONINFO*)GlobalAllocPtr(GHND, sizeof(SESSIONINFO));
	if (pSessionInfo == NULL) {
		return TRUE;
	}

	// Extract the data we need from the session description
	pSessionInfo->guidInstance = pSessionDesc->guidInstance;
	pSessionInfo->dwFlags = pSessionDesc->dwFlags;

	// Store pointer to guid in list
	SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_SETITEMDATA, (WPARAM)iIndex,
		(LPARAM)pSessionInfo);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: SelectSessionInstance()
// Desc:
//-----------------------------------------------------------------------------
static VOID SelectSessionInstance(HWND hWnd, GUID* pguidSessionInstance)
{
	WPARAM i = 0;
	WPARAM iIndex = 0;

	// Loop over the GUID's stored with each session name
	// to find the one that matches what was passed in
	for (;;) {
		// Get data pointer stored with item
		LONG pData =
			SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_GETITEMDATA, i, 0);
		if (pData == CB_ERR) {
			break;
		}

		if (pData == 0) {
			continue;
		}

		// Guid matches
		if (IsEqualGUID(*pguidSessionInstance, *((GUID*)pData))) {
			iIndex = i; // Store index of this string
			break;
		}

		i++;
	}

	// Delect this item
	SendDlgItemMessageA(hWnd, IDC_SESSIONLIST, LB_SETCURSEL, iIndex, 0);
}

//-----------------------------------------------------------------------------
// Name: EnumSessions()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT EnumSessions(HWND hWnd, LPDIRECTPLAY4A pDP)
{
	DPSESSIONDESC2 sessionDesc;
	GUID guidSessionInstance;
	DWORD dwFlags;

	// Check for valid interface
	if (pDP == NULL) {
		return DPERR_INVALIDOBJECT;
	}

	// get guid of currently selected session
	guidSessionInstance = GUID_NULL;
	HRESULT hr = GetSessionInfo(hWnd, &guidSessionInstance, &dwFlags);

	// delete existing session list
	DeleteSessionInstanceList(hWnd);

	// add sessions to session list
	ZeroMemory(&sessionDesc, sizeof(DPSESSIONDESC2));
	sessionDesc.dwSize = sizeof(DPSESSIONDESC2);
	sessionDesc.guidApplication = BELLHOP_GUID;

	hr = pDP->EnumSessions(&sessionDesc, 0, EnumSessionsCallback, hWnd,
		DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_ASYNC);

	// select the session that was previously selected
	SelectSessionInstance(hWnd, &guidSessionInstance);

	// hilite "Join" button only if there are sessions to join
	LONG iIndex = SendDlgItemMessageA(
		hWnd, IDC_SESSIONLIST, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

	EnableDlgButton(hWnd, IDC_JOINBUTTON, (iIndex > 0) ? TRUE : FALSE);
	EnableDlgButton(hWnd, IDC_SPECTATORBUTTON, (iIndex > 0) ? TRUE : FALSE);

	return hr;
}

//-----------------------------------------------------------------------------
// Name: DestroyDirectPlayInterface()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT DestroyDirectPlayInterface(HWND hWnd, LPDIRECTPLAY4A pDP)
{
	if (NULL == pDP) {
		return DP_OK;
	}

	DeleteSessionInstanceList(hWnd);
	EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);
	EnableDlgButton(hWnd, IDC_SPECTATORBUTTON, FALSE);

	return static_cast<HRESULT>(pDP->Release());
}

//-----------------------------------------------------------------------------
// Name: GetConnection()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT GetConnection(HWND hWnd, int idCombo, VOID** ppConnection)
{
	// Get index of the item currently selected in the combobox
	LONG iIndex = SendDlgItemMessageA(hWnd, idCombo, CB_GETCURSEL, 0, 0);
	if (iIndex == CB_ERR) {
		return DPERR_GENERIC;
	}

	// Get the pointer to the connection shortcut associated with the item
	iIndex =
		SendDlgItemMessageA(hWnd, idCombo, CB_GETITEMDATA, (WPARAM)iIndex, 0);

	if ((CB_ERR == iIndex) || (NULL == iIndex)) {
		return DPERR_GENERIC;
	}

	*ppConnection = &((CONNECTIONINFO*)iIndex)->Connection;

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: DestroyDirectPlayLobbyInterface()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT DestroyDirectPlayLobbyInterface(
	HWND /* hWnd */, LPDIRECTPLAYLOBBY3A pDPLobby)
{
	if (NULL == pDPLobby) {
		return DP_OK;
	}

	return static_cast<HRESULT>(pDPLobby->Release());
}

//-----------------------------------------------------------------------------
// Name: CreateDirectPlayInterface()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT CreateDirectPlayInterface(LPDIRECTPLAY4A* ppDP)
{
	// Create an IDirectPlay interface
	return CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
		IID_IDirectPlay4A, (VOID**)ppDP);
}

//-----------------------------------------------------------------------------
// Name: CreateDirectPlayLobbyInterface()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT CreateDirectPlayLobbyInterface(LPDIRECTPLAYLOBBY3A* ppDPLobby)
{
	// Create an IDirectPlayLobby interface
	return CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
		IID_IDirectPlayLobby3A, (VOID**)ppDPLobby);
}

//-----------------------------------------------------------------------------
// Name: ConnectWndProc()
// Desc:
//-----------------------------------------------------------------------------
static INT_PTR CALLBACK ConnectWndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DPLAYINFO* pDPInfo;
	static LPDIRECTPLAY4A pDP;
	static LPDIRECTPLAYLOBBY3A pDPLobby;
	static UINT idTimer = 0;
	GUID guidSessionInstance;
	CHAR strPlayerName[MAXNAMELEN];
	DWORD dwNameSize;
	HRESULT hr;
	VOID* pConnection = NULL;
	ENUMCONNSTRUCT enStruct;
	DWORD dwSessionFlags;
	DWORD dwPlayerFlags = NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
		// Save the connection info pointer
		pDPInfo = (DPLAYINFO*)lParam;
		pDP = NULL;
		pDPLobby = NULL;

		// Create an IDirectPlay interface
		hr = CreateDirectPlayInterface(&pDP);
		if (SUCCEEDED(hr)) {

			// Create an IDirectLobby2 interface
			hr = CreateDirectPlayLobbyInterface(&pDPLobby);
			if (SUCCEEDED(hr)) {

				// Set first item in the connections combo box
				SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_ADDSTRING, 0,
					(LPARAM) "<Select a lobby provider>");
				SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_SETITEMDATA, 0, 0);
				SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_SETCURSEL, 0, 0);

				// Put all the available connections in a combo box
				enStruct.hWnd = hWnd;
				enStruct.idCombo = IDC_SPCOMBO;

				pDP->EnumConnections(&BELLHOP_GUID,
					DirectPlayEnumConnectionsCallback, &enStruct,
					DPCONNECTION_DIRECTPLAYLOBBY);

				// Setup initial button state
				EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);
				EnableDlgButton(hWnd, IDC_SPECTATORBUTTON, FALSE);
				break;
			}
		}

		ErrorBox("Could not create DirectPlay object because of error %s", hr);
		EndDialog(hWnd, FALSE);
		break;

	case WM_DESTROY:
		// Delete information stored along with the lists
		DeleteConnectionList(hWnd);
		DeleteSessionInstanceList(hWnd);
		break;

	case WM_TIMER:
		// Refresh the session list
		// Guard against leftover timer messages after timer has been killed
		if (idTimer) {
			hr = EnumSessions(hWnd, pDP);
			if (FAILED(hr) && hr != DPERR_CONNECTING) {
				KillTimer(hWnd, idTimer);
				idTimer = 0;
				if (hr != DPERR_USERCANCEL) {
					ErrorBox("Enumerating sessions has failed; error %s", hr);
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
				}
				idTimer = 0;

				hr = DestroyDirectPlayInterface(hWnd, pDP);
				pDP = NULL;

				// Get pointer to the selected connection
				hr = GetConnection(hWnd, IDC_SPCOMBO, &pConnection);
				if (FAILED(hr)) {
					goto SP_FAILURE;
				}

				if (pConnection) {
					// Create a new DPlay interface.
					hr = CreateDirectPlayInterface(&pDP);

					if ((FAILED(hr)) || (NULL == pDP)) {
						goto SP_FAILURE;
					}

					// Initialize the connection
					hr = pDP->InitializeConnection(pConnection, 0);
					if (FAILED(hr)) {
						goto SP_FAILURE;
					}

					// Start enumerating the sessions
					hr = EnumSessions(hWnd, pDP);
					if (FAILED(hr)) {
						goto SP_FAILURE;
					}
					// Set a timer to refresh the session list
					idTimer = SetTimer(hWnd, TIMERID, TIMERINTERVAL, NULL);
				} else {
					// They've selected the generic option "<Select a service
					// provider>"
					EnableDlgButton(hWnd, IDC_JOINBUTTON, FALSE);
					EnableDlgButton(hWnd, IDC_SPECTATORBUTTON, FALSE);
				}
				break;
			}
			break;

		SP_FAILURE:
			if (hr != DPERR_USERCANCEL) {
				ErrorBox(
					"Could not select service provider because of error %s",
					hr);
			}
			break;

		case IDC_SPECTATORBUTTON:
			// Joining as a spectator is the same as a regular join
			// just with different flags.
			dwPlayerFlags = DPPLAYER_SPECTATOR;
			// Fall through to case IDC_JOINBUTTON:

		case IDC_JOINBUTTON:
			// Should have an interface by now
			if (pDP == NULL) {
				break;
			}

			if (idTimer) {
				KillTimer(hWnd, idTimer);
			}
			idTimer = 0;

			// Get guid of selected session instance
			hr = GetSessionInfo(hWnd, &guidSessionInstance, &dwSessionFlags);
			if (SUCCEEDED(hr)) {

				// Use computer name for player name
				dwNameSize = MAXNAMELEN;
				if (!GetComputerNameA(strPlayerName, &dwNameSize)) {
					strcpy(strPlayerName, "unknown");
				}

				// Borland CBuilder3 is missing a _strlwr(), so here's one:
				{
					CHAR* psz = strPlayerName;
					while (*psz) {
						if (isupper(*psz)) {
							*psz = (CHAR)tolower(*psz);
						}
						psz++;
					}
				}

				// Join this session
				hr = JoinSession(hWnd, pDP, pDPLobby, &guidSessionInstance,
					dwSessionFlags, strPlayerName, dwPlayerFlags, pDPInfo);
				if (SUCCEEDED(hr)) {
					// Dismiss dialog if we succeeded in joining
					EndDialog(hWnd, TRUE);
					break;
				}
			}
			ErrorBox("Could not join session because of error %s", hr);
			break;

		case IDCANCEL:
			// Delete any interface created if cancelling
			if (idTimer) {
				KillTimer(hWnd, idTimer);
			}
			idTimer = 0;

			hr = DestroyDirectPlayInterface(hWnd, pDP);
			pDP = NULL;

			hr = DestroyDirectPlayLobbyInterface(hWnd, pDPLobby);
			pDPLobby = NULL;

			EndDialog(hWnd, FALSE);
			break;

		default:
			// Message was not handled
			return FALSE;
		}
		break;

	default:
		// Message was not handled
		return FALSE;
	}

	// Message was handled
	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: ConnectUsingDialog()
// Desc:
//-----------------------------------------------------------------------------
HRESULT ConnectUsingDialog(HINSTANCE hInstance, DPLAYINFO* pDPInfo)
{
	// Ask user for connection settings
	if (DialogBoxParamA(hInstance, MAKEINTRESOURCEA(IDD_CONNECTDIALOG), NULL,
			ConnectWndProc, (LPARAM)pDPInfo)) {
		return DP_OK;
	}
	return DPERR_USERCANCEL;
}

//-----------------------------------------------------------------------------
// Name: DirectPlayEnumConnectionsCallback()
// Desc:
//-----------------------------------------------------------------------------
BOOL FAR PASCAL DirectPlayEnumConnectionsCallback(const GUID* pguidSP,
	VOID* pConnection, DWORD dwSize, const DPNAME* pName, DWORD /* dwFlags */,
	VOID* pContext)
{
	ENUMCONNSTRUCT* p = (ENUMCONNSTRUCT*)pContext;
	CONNECTIONINFO* pConnectionBuffer = NULL;

	// Store service provider name in combo box
	LRESULT iIndex = SendDlgItemMessageA(
		p->hWnd, p->idCombo, CB_ADDSTRING, 0, (LPARAM)pName->lpszShortNameA);
	if (iIndex == CB_ERR) {
		return TRUE;
	}

	// Make space for Connection Shortcut
	pConnectionBuffer =
		(CONNECTIONINFO*)GlobalAllocPtr(GHND, dwSize + sizeof(CONNECTIONINFO));
	if (pConnectionBuffer == NULL) {
		return TRUE;
	}

	// Store pointer to GUID in combo box
	memcpy(pConnectionBuffer->Connection, pConnection, dwSize);
	pConnectionBuffer->guidSP = *pguidSP;
	SendDlgItemMessageA(p->hWnd, p->idCombo, CB_SETITEMDATA, (WPARAM)iIndex,
		(LPARAM)pConnectionBuffer);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: GetConnectionSPGuid()
// Desc:
//-----------------------------------------------------------------------------
HRESULT GetConnectionSPGuid(HWND hWnd, int idCombo, GUID* pGuidSP)
{
	// Get index of the item currently selected in the combobox
	LONG iIndex = SendDlgItemMessageA(hWnd, idCombo, CB_GETCURSEL, 0, 0);
	if (iIndex == CB_ERR) {
		return DPERR_GENERIC;
	}
	// Get the pointer to the connection shortcut associated with the item
	iIndex =
		SendDlgItemMessageA(hWnd, idCombo, CB_GETITEMDATA, (WPARAM)iIndex, 0);

	if ((iIndex == CB_ERR) || (iIndex == NULL)) {
		return DPERR_GENERIC;
	}

	*pGuidSP = ((CONNECTIONINFO*)iIndex)->guidSP;

	return DP_OK;
}
