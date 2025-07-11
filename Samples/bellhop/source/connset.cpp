//-----------------------------------------------------------------------------
// File: ConnSet.cpp
//
// Desc: A dialog to set and get Group connection settings for a lobby
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#include "connset.h"
#include "Bellhop.h"
#include "dialog.h"
#include "dpdialogs.h"

// Disable a warning on Windows calls for CodeWarrior
#if __MWERKS__
#pragma warn_no_side_effect off
#endif

//-----------------------------------------------------------------------------
// Name: InitConnectionSettingsDialog()
// Desc:
//-----------------------------------------------------------------------------
static BOOL InitConnectionSettingsDialog(HWND hWnd, LOBBYGROUPCONTEXT* pContext)
{
	int i, iNum = 0;
	int lParam;

	CONNECTIONINFO* pCI = NULL;
	GUID* pGuid = NULL;
	DPLCONNECTION* pConn = NULL;
	ENUMCONNSTRUCT enStruct;

	HWND hSPComboBox = GetDlgItem(hWnd, IDC_GROUPCONNECTIONSPCOMBO);
	HWND hAppComboBox = GetDlgItem(hWnd, IDC_APPCOMBO);

	// Put all the DirectPlay applications in a combo box
	pContext->pDPInfo->pDPLobby->EnumLocalApplications(EnumApp, hWnd, 0);

	// Put all the available connections in a combo box
	enStruct.hWnd = hWnd;
	enStruct.idCombo = IDC_GROUPCONNECTIONSPCOMBO;

	HRESULT hr = pContext->pDPInfo->pDP->EnumConnections(&BELLHOP_GUID,
		DirectPlayEnumConnectionsCallback, &enStruct, DPCONNECTION_DIRECTPLAY);
	if (FAILED(hr)) {
		return FALSE;
	}

	DWORD dwSize = 0;
	hr = pContext->pDPInfo->pDP->GetGroupConnectionSettings(
		0, pContext->dpidRoom, pConn, &dwSize);
	if (DPERR_BUFFERTOOSMALL != hr) {
		return FALSE;
	}

	pConn = (DPLCONNECTION*)GlobalAllocPtr(GHND, dwSize);
	if (NULL == pConn) {
		return FALSE;
	}

	hr = pContext->pDPInfo->pDP->GetGroupConnectionSettings(
		0, pContext->dpidRoom, pConn, &dwSize);
	if (FAILED(hr)) {
		return FALSE;
	}

	iNum = ComboBox_GetCount(hSPComboBox);

	BOOL bFound = FALSE;
	for (i = 0; i < iNum; i++) {
		lParam = ComboBox_GetItemData(hSPComboBox, i);

		if ((lParam) && (CB_ERR != lParam)) {
			pCI = (CONNECTIONINFO*)lParam;
			if (IsEqualGUID(pCI->guidSP, pConn->guidSP)) {
				bFound = TRUE;
				ComboBox_SetCurSel(hSPComboBox, i);
				break;
			}
			pCI = NULL;
		}

		lParam = NULL;
	}

	if (FALSE == bFound) {
		// No match.
		ComboBox_AddString(hSPComboBox, "<Unknown Service Provider>");
		ComboBox_SetItemData(hSPComboBox, 0, 0);
		ComboBox_SetCurSel(hSPComboBox, 0);
	}

	iNum = ComboBox_GetCount(hAppComboBox);

	bFound = FALSE;
	for (i = 0; i < iNum; i++) {
		lParam = ComboBox_GetItemData(hAppComboBox, i);

		if ((lParam) && (CB_ERR != lParam)) {
			pGuid = (GUID*)lParam;
			if (IsEqualGUID(*pGuid, pConn->lpSessionDesc->guidApplication)) {
				bFound = TRUE;
				ComboBox_SetCurSel(hAppComboBox, i);
				break;
			}
			pGuid = NULL;
		}
		lParam = NULL;
	}

	if (FALSE == bFound) {
		// No match.
		ComboBox_AddString(hAppComboBox, "<Unknown Application>");
		ComboBox_SetItemData(hAppComboBox, 0, 0);
		ComboBox_SetCurSel(hAppComboBox, 0);
	}

	// Initialize max players
	SetDlgItemInt(hWnd, IDC_MAXPLAYERSEDIT, 0, FALSE);
	SetDlgItemTextA(
		hWnd, IDC_PASSWORDEDIT, pConn->lpSessionDesc->lpszPasswordA);

	return TRUE;
}

//-----------------------------------------------------------------------------
// Name: SetGroupConnection()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT SetGroupConnection(HWND hWnd, LOBBYGROUPCONTEXT* pContext)
{
	CHAR strPassword[MAXSTRLEN];
	GUID guidApplication;
	GUID guidSP;
	DWORD dwSize = 0;

	// Pull the info from the dialog
	GetDlgItemTextA(hWnd, IDC_PASSWORDEDIT, strPassword, MAXSTRLEN);
	DWORD dwMaxPlayers = GetDlgItemInt(hWnd, IDC_MAXPLAYERSEDIT, NULL, FALSE);
	HRESULT hr = GetComboBoxGuid(hWnd, IDC_APPCOMBO, &guidApplication);

	if (FAILED(hr)) {
		ErrorBox("Please select a different application. (%s)", hr);
		return hr;
	}

	GetConnectionSPGuid(hWnd, IDC_GROUPCONNECTIONSPCOMBO, &guidSP);

	if (FAILED(hr)) {
		ErrorBox("Please select a different service provider. (%s)", hr);
		return hr;
	}

	// Get the old connection settings.
	hr = pContext->pDPInfo->pDP->GetGroupConnectionSettings(
		0, pContext->dpidRoom, NULL, &dwSize);

	DPLCONNECTION* p = (DPLCONNECTION*)GlobalAllocPtr(GHND, dwSize);
	if (p) {
		hr = pContext->pDPInfo->pDP->GetGroupConnectionSettings(
			0, pContext->dpidRoom, (VOID*)p, &dwSize);

		p->lpSessionDesc->dwMaxPlayers = dwMaxPlayers;
		p->lpSessionDesc->lpszPasswordA = strPassword;
		p->lpSessionDesc->guidApplication = guidApplication;
		p->guidSP = guidSP;

		hr = pContext->pDPInfo->pDP->SetGroupConnectionSettings(
			0, pContext->dpidRoom, p);
	} else {
		hr = DPERR_OUTOFMEMORY;
	}

	return hr;
}

//-----------------------------------------------------------------------------
// Name: ConnectionSettingsDialogProc()
// Desc:
//-----------------------------------------------------------------------------
BOOL CALLBACK ConnectionSettingsDialogProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LOBBYGROUPCONTEXT* pContext =
		(LOBBYGROUPCONTEXT*)GetWindowLong(hWnd, DWL_USER);
	HRESULT hr;

	switch (uMsg) {
	case WM_INITDIALOG:
		// Context passed in lParam
		pContext = (LOBBYGROUPCONTEXT*)lParam;

		// Save the globals with the window
		SetWindowLongA(hWnd, DWL_USER, (LONG)pContext);
		InitConnectionSettingsDialog(hWnd, pContext);
		break;

	case WM_DESTROY: {

		// Destroy the GUID's stored with each app name
		WPARAM index = 0;
		LRESULT pData;
		for (;;) {
			pData = SendDlgItemMessageA(
				hWnd, IDC_APPCOMBO, CB_GETITEMDATA, (WPARAM)index, 0);
			if ((pData == CB_ERR) || (pData == 0)) {
				break;
			}

			GlobalFreePtr((VOID*)pData);
			index += 1;
		}

		// destroy the connection info in the combo box.
		index = 0;
		for (;;) {
			pData = SendDlgItemMessageA(hWnd, IDC_GROUPCONNECTIONSPCOMBO,
				CB_GETITEMDATA, (WPARAM)index, 0);
			if ((pData == CB_ERR) || (pData == 0)) {
				break;
			}

			GlobalFreePtr((VOID*)pData);
			index += 1;
		}
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			// Save changes they made
			hr = SetGroupConnection(hWnd, pContext);

			if (SUCCEEDED(hr)) {
				// Return success
				EndDialog(hWnd, TRUE);
			}
			break;

		case IDCANCEL:
			// Return failure
			EndDialog(hWnd, FALSE);
			break;

		case IDC_STAGINGAREA: {
			int i =
				SendDlgItemMessageA(hWnd, IDC_STAGINGAREA, BM_GETCHECK, 0, 0);
			EnableWindow(
				GetDlgItem(hWnd, IDC_PASSWORDEDIT), (BST_CHECKED == i));
			EnableWindow(GetDlgItem(hWnd, IDC_APPCOMBO), (BST_CHECKED == i));
			EnableWindow(
				GetDlgItem(hWnd, IDC_MAXPLAYERSEDIT), (BST_CHECKED == i));
			EnableWindow(GetDlgItem(hWnd, IDC_GROUPCONNECTIONSPCOMBO),
				(BST_CHECKED == i));
			break;
		}
		}
		break;
	}

	// Allow for default processing
	return FALSE;
}
