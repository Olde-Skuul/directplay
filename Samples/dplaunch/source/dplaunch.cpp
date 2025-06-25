//-----------------------------------------------------------------------------
// File: DPLaunch.cpp
//
// Desc: Implementation of a DirectPlay launching utility
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#define INITGUID

#include <windows.h>
#include <windowsx.h>

#include <cguid.h>
#include <objbase.h>
#include <stdio.h>
#include <wtypes.h>

#include "dplay.h"
#include "dplobby.h"
#include "resource.h"

#if defined(UNICODE) || defined(_UNICODE)
#error This app does not support UNICODE
#endif

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#define NAMEMAX 200       // maximum size of a string name
#define ADDRESSTYPEMAX 10 // maximum no. address types
#define MAXSTRLEN 200     // maximum size of temp strings

// GUID for sessions this application creates
DEFINE_GUID(MY_SESSION_GUID, 0xd559fc00, 0xdc12, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0,
	0xc9, 0x5, 0x42, 0x5e);

// List of address types
struct ADDRESSTYPELIST {
	DWORD dwCount;
	GUID guidAddressTypes[ADDRESSTYPEMAX];
};

//-----------------------------------------------------------------------------
// Name: EnumApp()
// Desc: Enumerates the applications registered with DirectPlay.
//-----------------------------------------------------------------------------
static BOOL FAR PASCAL EnumApp(
	const DPLAPPINFO* pAppInfo, VOID* pContext, DWORD /* dwFlags */)
{
	HWND hWnd = (HWND)pContext;

	// Store application name in combo box
	LRESULT iIndex = SendDlgItemMessageA(
		hWnd, IDC_APPCOMBO, CB_ADDSTRING, 0, (LPARAM)pAppInfo->lpszAppNameA);
	if (iIndex == LB_ERR) {
		return TRUE;
	}

	// Make space for application GUID
	GUID* pGuid = static_cast<GUID*>(GlobalAllocPtr(GHND, sizeof(GUID)));
	if (pGuid == NULL) {
		return TRUE;
	}

	// Store pointer to GUID in combo box
	*pGuid = pAppInfo->guidApplication;
	SendDlgItemMessageA(
		hWnd, IDC_APPCOMBO, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)pGuid);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: EnumSP()
// Desc: Enumerates service providers registered with DirectPlay.
//-----------------------------------------------------------------------------
static BOOL FAR PASCAL EnumSP(const GUID* pSPGUID, VOID* /* pConnection */,
	DWORD /* dwConnectionSize */, const DPNAME* pName, DWORD /* dwFlags */,
	VOID* pContext)
{
	HWND hWnd = (HWND)pContext;

	// Store service provider name in combo box
	LRESULT iIndex = SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_ADDSTRING, 0, (LPARAM)pName->lpszShortNameA);
	if (iIndex == LB_ERR) {
		return TRUE;
	}

	// Make space for service provider GUID
	GUID* pGuid = static_cast<GUID*>(GlobalAllocPtr(GHND, sizeof(GUID)));
	if (pGuid == NULL) {
		return TRUE;
	}

	// Store pointer to GUID in combo box
	*pGuid = *pSPGUID;
	SendDlgItemMessageA(
		hWnd, IDC_SPCOMBO, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)pGuid);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: EnumModemAddress()
// Desc: Enumerates the DirectPlay address chunks. If the chunk contains modem
//       strings, add them to the control.
//-----------------------------------------------------------------------------
static BOOL FAR PASCAL EnumModemAddress(REFGUID guidDataType,
	DWORD /* dwDataSize */, const VOID* pData, VOID* pContext)
{
	HWND hWnd = (HWND)pContext;
	LPSTR strStr = (LPSTR)pData;

	// Check for modem
	if (IsEqualGUID(guidDataType, DPAID_Modem)) {
		// Loop over all strings in list
		while (strlen(strStr)) {
			// Store modem name in combo box
			SendDlgItemMessageA(
				hWnd, IDC_ADDRESSCOMBO, CB_ADDSTRING, 0, (LPARAM)strStr);

			// Skip to next string
			strStr += strlen(strStr) + 1;
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: FillModemComboBox()
// Desc: Fills combo box with modem names
//-----------------------------------------------------------------------------
static HRESULT FillModemComboBox(
	HWND hWnd, LPDIRECTPLAYLOBBY3A pDPLobby, GUID* pServiceProviderGUID)
{
	LPDIRECTPLAY pDP1 = NULL;
	LPDIRECTPLAY4A pDP = NULL;
	DWORD dwAddressSize = 0;

	// Use the obsolete DirectPlayCreate() as quick way to load a specific
	// service provider.  Trade off using DirectPlayCreate() and
	// QueryInterface() vs using CoCreateInitialize() and building a
	// DirectPlay Address for InitializeConnection().
	HRESULT hr = DirectPlayCreate(pServiceProviderGUID, &pDP1, NULL);
	if (FAILED(hr)) {
		return hr;
	}

	// Query for an ANSI DirectPlay4 interface
	hr = pDP1->QueryInterface(IID_IDirectPlay4A, (VOID**)&pDP);

	// Done with DirectPlay1 interface
	pDP1->Release();

	// Fail if we couldn't get a DirectPlay4 interface
	if (FAILED(hr)) {
		return hr;
	}

	// Get size of player address for player zero
	hr = pDP->GetPlayerAddress(DPID_ALLPLAYERS, NULL, &dwAddressSize);
	if (hr != DPERR_BUFFERTOOSMALL) {
		pDP->Release();
		return hr;
	}

	// Make room for it
	VOID* pAddress = GlobalAllocPtr(GHND, dwAddressSize);
	if (pAddress == NULL) {
		pDP->Release();
		return DPERR_NOMEMORY;
	}

	// Get the address
	hr = pDP->GetPlayerAddress(DPID_ALLPLAYERS, pAddress, &dwAddressSize);
	if (FAILED(hr)) {
		GlobalFreePtr(pAddress);
		pDP->Release();
		return hr;
	}

	// Get modem strings from address and put them in the combo box
	hr = pDPLobby->EnumAddress(EnumModemAddress, pAddress, dwAddressSize, hWnd);
	if (FAILED(hr)) {
		GlobalFreePtr(pAddress);
		pDP->Release();
		return hr;
	}

	// Select first item in list
	SendDlgItemMessageA(hWnd, IDC_ADDRESSCOMBO, CB_SETCURSEL, 0, 0);

	// Cleanup and exit
	GlobalFreePtr(pAddress);
	pDP->Release();
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: GetComboBoxGuid()
// Desc: Returns GUID stored with a combo box item
//-----------------------------------------------------------------------------
static HRESULT GetComboBoxGuid(HWND hWnd, LONG iDialogItem, GUID* pReturnGUID)
{
	// Get index of selected item
	LONG iIndex = SendDlgItemMessageA(hWnd, iDialogItem, CB_GETCURSEL, 0, 0);
	if (iIndex == CB_ERR) {
		return DPERR_GENERIC;
	}

	// Get data associated with this item
	iIndex = SendDlgItemMessageA(
		hWnd, iDialogItem, CB_GETITEMDATA, (WPARAM)iIndex, 0);
	if ((iIndex == CB_ERR) || (iIndex == 0)) {
		return DPERR_GENERIC;
	}

	// Data is a pointer to a guid
	*pReturnGUID = *((GUID*)iIndex);
	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: EnumAddressTypes()
// Desc: Enumerates the address types supported by the given Service Provider
//       and returns them in a list.
//-----------------------------------------------------------------------------
static BOOL FAR PASCAL EnumAddressTypes(
	REFGUID guidAddressType, VOID* pContext, DWORD /* dwFlags */)
{
	ADDRESSTYPELIST* pAddressTypes = (ADDRESSTYPELIST*)pContext;

	// Make sure there is room
	if (pAddressTypes->dwCount < ADDRESSTYPEMAX) {
		// Save the address type guid in the list
		pAddressTypes->guidAddressTypes[pAddressTypes->dwCount] =
			guidAddressType;
		pAddressTypes->dwCount++;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: UpdateAddressInfo()
// Desc: Updates address information elements in dialog. Calls
//       EnumAddressTypes() to determine what address information should be
//       displayed and arranges dialog to display and collect the needed
//       information.
//-----------------------------------------------------------------------------
static HRESULT UpdateAddressInfo(HWND hWnd, LPDIRECTPLAYLOBBY3A pDPLobby)
{
	GUID guidServiceProvider, guidAddressType;
	ADDRESSTYPELIST addressTypeList;

	// Get guid of currently selected service provider
	HRESULT hr = GetComboBoxGuid(hWnd, IDC_SPCOMBO, &guidServiceProvider);
	if (FAILED(hr)) {
		return hr;
	}

	// Get the list of address types for this service provider
	ZeroMemory(&addressTypeList, sizeof(ADDRESSTYPELIST));
	hr = pDPLobby->EnumAddressTypes(
		EnumAddressTypes, guidServiceProvider, &addressTypeList, 0L);
	if (FAILED(hr)) {
		return hr;
	}

	// Clear and hide address dialog items
	SendDlgItemMessageA(hWnd, IDC_ADDRESSCOMBO, CB_RESETCONTENT, 0, 0);
	ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSCOMBO), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSCOMBOLABEL), SW_HIDE);

	SetDlgItemTextA(hWnd, IDC_ADDRESSEDIT, "");
	ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDIT), SW_HIDE);
	ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDITLABEL), SW_HIDE);

	SetDlgItemTextA(hWnd, IDC_PORTEDIT, "");
	ShowWindow(GetDlgItem(hWnd, IDC_PORTEDIT), SW_HIDE);

	// Loop over the address types
	DWORD i;
	for (i = 0; i < addressTypeList.dwCount; i++) {
		guidAddressType = addressTypeList.guidAddressTypes[i];

		if (IsEqualGUID(guidAddressType, DPAID_Phone)) {
			// Phone number
			SetDlgItemTextA(hWnd, IDC_ADDRESSEDITLABEL, "Phone number");
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDIT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDITLABEL), SW_SHOW);
		} else if (IsEqualGUID(guidAddressType, DPAID_Modem)) {
			// Modem
			SetDlgItemTextA(hWnd, IDC_ADDRESSCOMBOLABEL, "Modem");
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSCOMBO), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSCOMBOLABEL), SW_SHOW);
			FillModemComboBox(hWnd, pDPLobby, &guidServiceProvider);
		} else if (IsEqualGUID(guidAddressType, DPAID_INet)) {
			// Internet address
			SetDlgItemTextA(hWnd, IDC_ADDRESSEDITLABEL, "IP address");
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDIT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSEDITLABEL), SW_SHOW);
		}

		else if (IsEqualGUID(guidAddressType, DPAID_INetPort)) {
			// Internet address port
			SetDlgItemTextA(hWnd, IDC_ADDRESSCOMBOLABEL, "Port");
			ShowWindow(GetDlgItem(hWnd, IDC_PORTEDIT), SW_SHOW);
			ShowWindow(GetDlgItem(hWnd, IDC_ADDRESSCOMBOLABEL), SW_SHOW);
		}
	}

	return hr;
}

//-----------------------------------------------------------------------------
// Name: InitializeLauncherWindow()
// Desc: Initializes the window for the Launcher.
//-----------------------------------------------------------------------------
static HRESULT InitializeLauncherWindow(
	HWND hWnd, LPDIRECTPLAYLOBBY3A* ppDPLobby)
{
	LPDIRECTPLAY4A pDP = NULL;
	LPDIRECTPLAYLOBBY3A pDPLobby = NULL;

	// Create a temporary ANSI DirectPlay4 interface
	HRESULT hr = CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
		IID_IDirectPlay4A, (VOID**)&pDP);
	if (FAILED(hr)) {
		return hr;
	}

	// Get ANSI DirectPlayLobby interface
	hr = CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
		IID_IDirectPlayLobby3A, (VOID**)&pDPLobby);
	if (FAILED(hr)) {
		pDP->Release();
		return hr;
	}

	// Put all the DirectPlay applications in a combo box
	pDPLobby->EnumLocalApplications(EnumApp, hWnd, 0);

	// Put all the service providers in a combo box
	pDP->EnumConnections(NULL, EnumSP, hWnd, 0);

	// Done with the DirectPlay4 interface
	pDP->Release();

	// Initialize the controls
	SendDlgItemMessageA(hWnd, IDC_APPCOMBO, CB_SETCURSEL, 0, 0);
	SendDlgItemMessageA(hWnd, IDC_SPCOMBO, CB_SETCURSEL, 0, 0);
	SendDlgItemMessageA(
		hWnd, IDC_HOSTRADIO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

	// Update the address info display
	hr = UpdateAddressInfo(hWnd, pDPLobby);

	// Return the ANSI lobby interface
	*ppDPLobby = pDPLobby;

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: DestroyLauncherWindow()
// Desc: Destroys the launcher window.
//-----------------------------------------------------------------------------
static VOID DestroyLauncherWindow(HWND hWnd, LPDIRECTPLAYLOBBY3A pDPLobby)
{
	LRESULT pData;

	// Destroy the GUID's stored with each app name
	WPARAM index = 0;
	for (;;) {
		pData = SendDlgItemMessageA(
			hWnd, IDC_APPCOMBO, CB_GETITEMDATA, (WPARAM)index, 0);
		if ((pData == CB_ERR) || (pData == 0)) {
			break;
		}

		GlobalFreePtr((VOID*)pData);
		index++;
	}

	// Destroy the GUID's stored with each service provider name
	index = 0;
	for (;;) {
		pData = SendDlgItemMessageA(
			hWnd, IDC_SPCOMBO, CB_GETITEMDATA, (WPARAM)index, 0);
		if ((pData == CB_ERR) || (pData == 0)) {
			break;
		}

		GlobalFreePtr((VOID*)pData);
		index++;
	}

	// Release the lobby interface
	if (pDPLobby) {
		pDPLobby->Release();
	}
}

//-----------------------------------------------------------------------------
// Name: CreateAddress()
// Desc: Creates a DPADDRESS using the address information from the dialog.
//-----------------------------------------------------------------------------
static HRESULT CreateAddress(HWND hWnd, LPDIRECTPLAYLOBBY3A pDPLobby,
	GUID* pServiceProviderGUID, VOID** ppAddress, DWORD* pdwAddressSize)
{
	ADDRESSTYPELIST addressTypeList;
	DPCOMPOUNDADDRESSELEMENT addressElements[1 + ADDRESSTYPEMAX];
	GUID guidAddressType;
	CHAR strPhoneNumberString[NAMEMAX];
	CHAR strModemString[NAMEMAX];
	CHAR strIPAddressString[NAMEMAX];
	CHAR strPort[NAMEMAX];
	VOID* pAddress = NULL;
	DWORD dwAddressSize = 0;

	// Get the list of address types for this service provider
	ZeroMemory(&addressTypeList, sizeof(ADDRESSTYPELIST));
	HRESULT hr = pDPLobby->EnumAddressTypes(
		EnumAddressTypes, *pServiceProviderGUID, &addressTypeList, 0L);
	if (FAILED(hr)) {
		return hr;
	}

	// All DPADDRESS's must have a service provider chunk
	DWORD dwElementCount = 0;
	addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
	addressElements[dwElementCount].dwDataSize = sizeof(GUID);
	addressElements[dwElementCount].lpData = pServiceProviderGUID;
	dwElementCount++;

	// Loop over the address types
	DWORD i;
	for (i = 0; i < addressTypeList.dwCount; i++) {
		guidAddressType = addressTypeList.guidAddressTypes[i];

		if (IsEqualGUID(guidAddressType, DPAID_Phone)) {
			// Add a phone number chunk
			GetDlgItemTextA(
				hWnd, IDC_ADDRESSEDIT, strPhoneNumberString, NAMEMAX);
			addressElements[dwElementCount].guidDataType = DPAID_Phone;
			addressElements[dwElementCount].dwDataSize =
				strlen(strPhoneNumberString) + 1;
			addressElements[dwElementCount].lpData = strPhoneNumberString;
			dwElementCount++;
		} else if (IsEqualGUID(guidAddressType, DPAID_Modem)) {
			// Add a modem chunk
			GetDlgItemTextA(hWnd, IDC_ADDRESSCOMBO, strModemString, NAMEMAX);
			addressElements[dwElementCount].guidDataType = DPAID_Modem;
			addressElements[dwElementCount].dwDataSize =
				strlen(strModemString) + 1;
			addressElements[dwElementCount].lpData = strModemString;
			dwElementCount++;
		} else if (IsEqualGUID(guidAddressType, DPAID_INet)) {
			// Add an IP address chunk
			GetDlgItemTextA(hWnd, IDC_ADDRESSEDIT, strIPAddressString, NAMEMAX);
			addressElements[dwElementCount].guidDataType = DPAID_INet;
			addressElements[dwElementCount].dwDataSize =
				strlen(strIPAddressString) + 1;
			addressElements[dwElementCount].lpData = strIPAddressString;
			dwElementCount++;
		} else if (IsEqualGUID(guidAddressType, DPAID_INetPort)) {
			memset(strPort, 0, sizeof(strPort));
			// Add a Port chunk
			GetDlgItemTextA(hWnd, IDC_PORTEDIT, strPort, NAMEMAX);
			WORD wPort = (WORD)atoi(strPort);
			addressElements[dwElementCount].guidDataType = DPAID_INetPort;
			addressElements[dwElementCount].dwDataSize = sizeof(WORD);
			addressElements[dwElementCount].lpData = (VOID*)&wPort;
			dwElementCount++;
		}
	}

	// Bail if no address data is available
	if (dwElementCount == 1) {
		return DPERR_GENERIC;
	}

	// See how much room is needed to store this address
	hr = pDPLobby->CreateCompoundAddress(
		addressElements, dwElementCount, NULL, &dwAddressSize);
	if (hr != DPERR_BUFFERTOOSMALL) {
		return hr;
	}

	// Allocate space
	pAddress = GlobalAllocPtr(GHND, dwAddressSize);
	if (pAddress == NULL) {
		return DPERR_NOMEMORY;
	}

	// Create the address
	hr = pDPLobby->CreateCompoundAddress(
		addressElements, dwElementCount, pAddress, &dwAddressSize);
	if (FAILED(hr)) {
		GlobalFreePtr(pAddress);
		return hr;
	}

	// Return the address info
	*ppAddress = pAddress;
	*pdwAddressSize = dwAddressSize;

	return DP_OK;
}

//-----------------------------------------------------------------------------
// Name: RunApplication()
// Desc: Wrapper for the IDirectPlayLobby::RunApplication() method.
//-----------------------------------------------------------------------------
static HRESULT RunApplication(LPDIRECTPLAYLOBBY3A pDPLobby,
	GUID* pApplicationGUID, GUID* pInstanceGUID, GUID* pServiceProviderGUID,
	VOID* pAddress, DWORD dwAddressSize, LPSTR strSessionName,
	LPSTR strPlayerName, BOOL bHostSession)
{
	DWORD appID;
	DPSESSIONDESC2 sessionInfo;
	DPNAME playerName;
	DPLCONNECTION connectInfo;

	if (pDPLobby == NULL) {
		return DPERR_NOINTERFACE;
	}

	// Fill out session description
	ZeroMemory(&sessionInfo, sizeof(DPSESSIONDESC2));
	sessionInfo.dwSize = sizeof(DPSESSIONDESC2);
	// DPSESSION_xxx flags
	sessionInfo.dwFlags = 0;
	// ID for the session instance
	sessionInfo.guidInstance = *pInstanceGUID;
	// GUID of the DirectPlay application.
	sessionInfo.guidApplication = *pApplicationGUID;
	// Maximum # players allowed in session
	sessionInfo.dwMaxPlayers = 0;
	// Current # players in session (read only)
	sessionInfo.dwCurrentPlayers = 0;
	// ANSI name of the session
	sessionInfo.lpszSessionNameA = strSessionName;
	// ANSI password of the session (optional)
	sessionInfo.lpszPasswordA = NULL;
	// Reserved for future MS use.
	sessionInfo.dwReserved1 = 0;
	sessionInfo.dwReserved2 = 0;
	// For use by the application
	sessionInfo.dwUser1 = 0;
	sessionInfo.dwUser2 = 0;
	sessionInfo.dwUser3 = 0;
	sessionInfo.dwUser4 = 0;

	// Fill out player name
	ZeroMemory(&playerName, sizeof(DPNAME));
	playerName.dwSize = sizeof(DPNAME);
	playerName.dwFlags = 0;                    // Not used. Must be zero.
	playerName.lpszShortNameA = strPlayerName; // ANSI short or friendly name
	playerName.lpszLongNameA = strPlayerName;  // ANSI long or formal name

	// Fill out connection description
	ZeroMemory(&connectInfo, sizeof(DPLCONNECTION));
	connectInfo.dwSize = sizeof(DPLCONNECTION);
	// Pointer to session desc to use on connect
	connectInfo.lpSessionDesc = &sessionInfo;
	// Pointer to Player name structure
	connectInfo.lpPlayerName = &playerName;
	// GUID of the DPlay SP to use
	connectInfo.guidSP = *pServiceProviderGUID;
	// Address for service provider
	connectInfo.lpAddress = pAddress;
	// Size of address data
	connectInfo.dwAddressSize = dwAddressSize;
	if (bHostSession) {
		connectInfo.dwFlags = DPLCONNECTION_CREATESESSION;
	} else {
		connectInfo.dwFlags = DPLCONNECTION_JOINSESSION;
	}

	// Launch and connect the game
	return pDPLobby->RunApplication(0, &appID, &connectInfo, NULL);
}

//-----------------------------------------------------------------------------
// Name: LaunchDirectPlayApplication()
// Desc: Gathers information from the dialog and runs the application.
//-----------------------------------------------------------------------------
static VOID LaunchDirectPlayApplication(HWND hWnd, LPDIRECTPLAYLOBBY3A pDPLobby)
{
	GUID guidApplication, guidSession, guidServiceProvider;
	LPSTR pPlayerName, pSessionName;
	LPVOID pAddress = NULL;
	DWORD dwAddressSize = 0;
	CHAR strPlayerName[NAMEMAX], strSessionName[NAMEMAX];

	SetDlgItemTextA(hWnd, IDC_STATUSEDIT, "Launching...");

	// Get guid of application to launch
	HRESULT hr = GetComboBoxGuid(hWnd, IDC_APPCOMBO, &guidApplication);
	if (FAILED(hr)) {
		SetDlgItemTextA(hWnd, IDC_STATUSEDIT, "Launch failed");
		return;
	}

	// Get guid of service provider to use
	hr = GetComboBoxGuid(hWnd, IDC_SPCOMBO, &guidServiceProvider);
	if (FAILED(hr)) {
		SetDlgItemTextA(hWnd, IDC_STATUSEDIT, "Launch failed");
		return;
	}

	// Get address to use with this service provider
	hr = CreateAddress(
		hWnd, pDPLobby, &guidServiceProvider, &pAddress, &dwAddressSize);
	// Ignore the error because pAddress will just be null

	// Get guid of session to create.
	guidSession = MY_SESSION_GUID;

	// Get name of our player
	GetDlgItemTextA(hWnd, IDC_PLAYEREDIT, strPlayerName, NAMEMAX);
	pPlayerName = strPlayerName;

	// Get host vs. join flag
	LRESULT iHost = SendDlgItemMessageA(hWnd, IDC_HOSTRADIO, BM_GETCHECK, 0, 0);
	if (iHost == BST_CHECKED) {
		// We are hosting a session
		iHost = TRUE;

		// Get name of session
		GetDlgItemTextA(hWnd, IDC_SESSIONEDIT, strSessionName, NAMEMAX);
		pSessionName = strSessionName;
	} else {
		// We are joining an existing session
		iHost = FALSE;

		// Don't need a session name if we are joining
		pSessionName = NULL;
	}

	// Launch the application
	hr = RunApplication(pDPLobby, &guidApplication, &guidSession,
		&guidServiceProvider, pAddress, dwAddressSize, pSessionName,
		pPlayerName, iHost);
	if (FAILED(hr)) {
		if (pAddress) {
			GlobalFreePtr(pAddress);
		}
		SetDlgItemTextA(hWnd, IDC_STATUSEDIT, "Launch failed");
		return;
	}

	SetDlgItemTextA(hWnd, IDC_STATUSEDIT, "Launch successful");

	if (pAddress) {
		GlobalFreePtr(pAddress);
	}
}

//-----------------------------------------------------------------------------
// Name: GetDirectPlayErrStr()
// Desc:
//-----------------------------------------------------------------------------
static const CHAR* GetDirectPlayErrStr(HRESULT hr)
{
	switch (hr) {
	case DP_OK:
		return "DP_OK";
	case DPERR_ALREADYINITIALIZED:
		return "DPERR_ALREADYINITIALIZED";
	case DPERR_ACCESSDENIED:
		return "DPERR_ACCESSDENIED";
	case DPERR_ACTIVEPLAYERS:
		return "DPERR_ACTIVEPLAYERS";
	case DPERR_BUFFERTOOSMALL:
		return "DPERR_BUFFERTOOSMALL";
	case DPERR_CANTADDPLAYER:
		return "DPERR_CANTADDPLAYER";
	case DPERR_CANTCREATEGROUP:
		return "DPERR_CANTCREATEGROUP";
	case DPERR_CANTCREATEPLAYER:
		return "DPERR_CANTCREATEPLAYER";
	case DPERR_CANTCREATESESSION:
		return "DPERR_CANTCREATESESSION";
	case DPERR_CAPSNOTAVAILABLEYET:
		return "DPERR_CAPSNOTAVAILABLEYET";
	case DPERR_EXCEPTION:
		return "DPERR_EXCEPTION";
	case DPERR_GENERIC:
		return "DPERR_GENERIC";
	case DPERR_INVALIDFLAGS:
		return "DPERR_INVALIDFLAGS";
	case DPERR_INVALIDOBJECT:
		return "DPERR_INVALIDOBJECT";
		//	case DPERR_INVALIDPARAM: return "DPERR_INVALIDPARAM";	 dup value
	case DPERR_INVALIDPARAMS:
		return "DPERR_INVALIDPARAMS";
	case DPERR_INVALIDPLAYER:
		return "DPERR_INVALIDPLAYER";
	case DPERR_INVALIDGROUP:
		return "DPERR_INVALIDGROUP";
	case DPERR_NOCAPS:
		return "DPERR_NOCAPS";
	case DPERR_NOCONNECTION:
		return "DPERR_NOCONNECTION";
		//	case DPERR_NOMEMORY: return "DPERR_NOMEMORY";		dup value
	case DPERR_OUTOFMEMORY:
		return "DPERR_OUTOFMEMORY";
	case DPERR_NOMESSAGES:
		return "DPERR_NOMESSAGES";
	case DPERR_NONAMESERVERFOUND:
		return "DPERR_NONAMESERVERFOUND";
	case DPERR_NOPLAYERS:
		return "DPERR_NOPLAYERS";
	case DPERR_NOSESSIONS:
		return "DPERR_NOSESSIONS";
	case DPERR_PENDING:
		return "DPERR_PENDING";
	case DPERR_SENDTOOBIG:
		return "DPERR_SENDTOOBIG";
	case DPERR_TIMEOUT:
		return "DPERR_TIMEOUT";
	case DPERR_UNAVAILABLE:
		return "DPERR_UNAVAILABLE";
	case DPERR_UNSUPPORTED:
		return "DPERR_UNSUPPORTED";
	case DPERR_BUSY:
		return "DPERR_BUSY";
	case DPERR_USERCANCEL:
		return "DPERR_USERCANCEL";
	case DPERR_NOINTERFACE:
		return "DPERR_NOINTERFACE";
	case DPERR_CANNOTCREATESERVER:
		return "DPERR_CANNOTCREATESERVER";
	case DPERR_PLAYERLOST:
		return "DPERR_PLAYERLOST";
	case DPERR_SESSIONLOST:
		return "DPERR_SESSIONLOST";
	case DPERR_UNINITIALIZED:
		return "DPERR_UNINITIALIZED";
	case DPERR_NONEWPLAYERS:
		return "DPERR_NONEWPLAYERS";
	case DPERR_INVALIDPASSWORD:
		return "DPERR_INVALIDPASSWORD";
	case DPERR_CONNECTING:
		return "DPERR_CONNECTING";
	case DPERR_CONNECTIONLOST:
		return "DPERR_CONNECTIONLOST";
	case DPERR_UNKNOWNMESSAGE:
		return "DPERR_UNKNOWNMESSAGE";
	case DPERR_CANCELFAILED:
		return "DPERR_CANCELFAILED";
	case DPERR_INVALIDPRIORITY:
		return "DPERR_INVALIDPRIORITY";
	case DPERR_NOTHANDLED:
		return "DPERR_NOTHANDLED";
	case DPERR_CANCELLED:
		return "DPERR_CANCELLED";
	case DPERR_ABORTED:
		return "DPERR_ABORTED";
	case DPERR_BUFFERTOOLARGE:
		return "DPERR_BUFFERTOOLARGE";
	case DPERR_CANTCREATEPROCESS:
		return "DPERR_CANTCREATEPROCESS";
	case DPERR_APPNOTSTARTED:
		return "DPERR_APPNOTSTARTED";
	case DPERR_INVALIDINTERFACE:
		return "DPERR_INVALIDINTERFACE";
	case DPERR_NOSERVICEPROVIDER:
		return "DPERR_NOSERVICEPROVIDER";
	case DPERR_UNKNOWNAPPLICATION:
		return "DPERR_UNKNOWNAPPLICATION";
	case DPERR_NOTLOBBIED:
		return "DPERR_NOTLOBBIED";
	case DPERR_SERVICEPROVIDERLOADED:
		return "DPERR_SERVICEPROVIDERLOADED";
	case DPERR_ALREADYREGISTERED:
		return "DPERR_ALREADYREGISTERED";
	case DPERR_NOTREGISTERED:
		return "DPERR_NOTREGISTERED";
	case DPERR_AUTHENTICATIONFAILED:
		return "DPERR_AUTHENTICATIONFAILED";
	case DPERR_CANTLOADSSPI:
		return "DPERR_CANTLOADSSPI";
	case DPERR_ENCRYPTIONFAILED:
		return "DPERR_ENCRYPTIONFAILED";
	case DPERR_SIGNFAILED:
		return "DPERR_SIGNFAILED";
	case DPERR_CANTLOADSECURITYPACKAGE:
		return "DPERR_CANTLOADSECURITYPACKAGE";
	case DPERR_ENCRYPTIONNOTSUPPORTED:
		return "DPERR_ENCRYPTIONNOTSUPPORTED";
	case DPERR_CANTLOADCAPI:
		return "DPERR_CANTLOADCAPI";
	case DPERR_NOTLOGGEDIN:
		return "DPERR_NOTLOGGEDIN";
	case DPERR_LOGONDENIED:
		return "DPERR_LOGONDENIED";
	}

	// For errors not in the list, return HRESULT string
	static CHAR strTemp[16];
	sprintf(strTemp, "0x%08X", hr);
	return strTemp;
}

//-----------------------------------------------------------------------------
// Name: ErrorBox()
// Desc:
//-----------------------------------------------------------------------------
static VOID ErrorBox(LPSTR strError, HRESULT hr)
{
	CHAR str[MAXSTRLEN];

	sprintf(str, strError, GetDirectPlayErrStr(hr));
	MessageBoxA(NULL, str, "DPLaunch Error", MB_OK);
}

//-----------------------------------------------------------------------------
// Name: LauncherWndProc()
// Desc: Message callback function for Launcher dialog.
//-----------------------------------------------------------------------------
static INT_PTR CALLBACK LauncherWndProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInst;
	static LPDIRECTPLAYLOBBY3A pDPLobby;
	HRESULT hr;

	switch (uMsg) {
	case WM_INITDIALOG:
		// Save the instance handle
		hInst = (HINSTANCE)lParam;

		// Initialize dialog with launcher information
		pDPLobby = NULL;
		hr = InitializeLauncherWindow(hWnd, &pDPLobby);
		if (FAILED(hr)) {
			ErrorBox("Could not initialize. Error %s", hr);
			EndDialog(hWnd, FALSE);
		}
		return TRUE;

	case WM_DESTROY:
		// Destroy launcher information in dialog
		DestroyLauncherWindow(hWnd, pDPLobby);
		break; // continue with default handling

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SPCOMBO:
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE:
				// update the address info display
				UpdateAddressInfo(hWnd, pDPLobby);
				return TRUE;
			}
			break;

		case IDC_RUNAPPBUTTON:
			// get settings and launch application
			LaunchDirectPlayApplication(hWnd, pDPLobby);
			return TRUE;

		case IDCANCEL:
			// Return failure
			EndDialog(hWnd, TRUE);
			return TRUE;
		}
		break;
	}

	// Allow for default processing
	return FALSE;
}

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Windows entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
	LPSTR /* lpCmdLine */, int /* nCmdShow */)
{
	// Initialize the COM library
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		ErrorBox("CoInitialize failed. Error %s", hr);
		return 0;
	}

	int iResult =
		DialogBoxParamA(hInstance, MAKEINTRESOURCEA(IDD_LAUNCHERDIALOG), NULL,
			LauncherWndProc, (LPARAM)hInstance);

	CoUninitialize();
	return iResult;
}
