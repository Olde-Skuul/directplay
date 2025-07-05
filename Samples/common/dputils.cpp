/***************************************

	Helper functions for directplay

	By Rebecca Ann Heineman

***************************************/

#include "dputils.h"
#include "dpmacros.h"
#include "dxutil.h"
#include <dxerr8.h>
#include <tchar.h>

/***************************************

	Fills the combobox with adapters for a specified SP

***************************************/

HRESULT EnumAdapters(IDirectPlay8Peer* pDirectPlay8Peer, HWND hDialog,
	GUID* pSPGuid, int iDialogItem)
{
	TCHAR strName[MAX_PATH];
	DWORD dwItems = 0;
	DWORD dwSize = 0;
	int nAllAdaptersIndex = 0;

	// Clear the dialog
	SendDlgItemMessage(hDialog, iDialogItem, CB_RESETCONTENT, 0, 0);

	// Enumerate all DirectPlay service providers, and store them in the listbox
	HRESULT hr = pDirectPlay8Peer->EnumServiceProviders(
		pSPGuid, NULL, NULL, &dwSize, &dwItems, 0);

	// No adapters found
	if (SUCCEEDED(hr)) {
		return S_OK;
	}

	if (hr != DPNERR_BUFFERTOOSMALL) {
		return DXTRACE_ERR(TEXT("EnumServiceProviders"), hr);
	}

	DPN_SERVICE_PROVIDER_INFO* pdnSPInfo =
		(DPN_SERVICE_PROVIDER_INFO*)new BYTE[dwSize];
	if (FAILED(hr = pDirectPlay8Peer->EnumServiceProviders(
				   pSPGuid, NULL, pdnSPInfo, &dwSize, &dwItems, 0)))
		return DXTRACE_ERR(TEXT("EnumServiceProviders"), hr);

	DPN_SERVICE_PROVIDER_INFO* pdnSPInfoEnum = pdnSPInfo;
	for (DWORD i = 0; i < dwItems; i++) {
		DXUtil_ConvertWideStringToGeneric(strName, pdnSPInfoEnum->pwszName);

		// Found a service provider, so put it in the listbox
		int nIndex = (int)SendDlgItemMessage(
			hDialog, iDialogItem, CB_ADDSTRING, 0, (LPARAM)strName);

		if (_tcscmp(strName, TEXT("All Adapters")) == 0) {
			nAllAdaptersIndex = nIndex;
		}

		// Store pointer to GUID in listbox
		GUID* pGuid = new GUID;
		memcpy(pGuid, &pdnSPInfoEnum->guid, sizeof(GUID));

		SendDlgItemMessage(hDialog, iDialogItem, CB_SETITEMDATA,
			static_cast<WPARAM>(nIndex), (LPARAM)pGuid);

		pdnSPInfoEnum++;
	}

	SAFE_DELETE_ARRAY(pdnSPInfo);

	SendDlgItemMessage(hDialog, iDialogItem, CB_SETCURSEL,
		static_cast<WPARAM>(nAllAdaptersIndex), 0);

	return S_OK;
}

/***************************************

	Fills the combobox with Service Providers

***************************************/

HRESULT EnumServiceProviders(IDirectPlay8Peer* pDirectPlay8Peer, HWND hDlg,
	int iDialogItem, const TCHAR* pPreferred)
{
	DPN_SERVICE_PROVIDER_INFO* pdnSPInfo = NULL;
	DWORD dwItems = 0;
	DWORD dwSize = 0;
	int nIndex;

	// Enumerate all DirectPlay service providers, and store them in the listbox
	HRESULT hr = pDirectPlay8Peer->EnumServiceProviders(
		NULL, NULL, pdnSPInfo, &dwSize, &dwItems, 0);
	if (hr != DPNERR_BUFFERTOOSMALL) {
		return DXTRACE_ERR(TEXT("EnumServiceProviders"), hr);
	}

	pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*)new BYTE[dwSize];
	if (FAILED(hr = pDirectPlay8Peer->EnumServiceProviders(
				   NULL, NULL, pdnSPInfo, &dwSize, &dwItems, 0))) {
		return DXTRACE_ERR(TEXT("EnumServiceProviders"), hr);
	}

	DPN_SERVICE_PROVIDER_INFO* pdnSPInfoEnum = pdnSPInfo;
	for (DWORD i = 0; i < dwItems; i++) {
		TCHAR strName[MAX_PATH];
		DXUtil_ConvertWideStringToGeneric(strName, pdnSPInfoEnum->pwszName);

		// Found a service provider, so put it in the listbox
		nIndex = (int)SendDlgItemMessage(
			hDlg, iDialogItem, CB_ADDSTRING, 0, (LPARAM)strName);

		// Store pointer to GUID in listbox
		GUID* pGuid = new GUID;
		memcpy(pGuid, &pdnSPInfoEnum->guid, sizeof(GUID));
		SendDlgItemMessage(hDlg, iDialogItem, CB_SETITEMDATA,
			static_cast<WPARAM>(nIndex), (LPARAM)pGuid);

		pdnSPInfoEnum++;
	}

	SAFE_DELETE_ARRAY(pdnSPInfo);

	// Try to select the default preferred provider
	nIndex = (int)SendDlgItemMessage(
		hDlg, iDialogItem, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pPreferred);
	if (nIndex != LB_ERR) {
		SendDlgItemMessage(
			hDlg, iDialogItem, CB_SETCURSEL, static_cast<WPARAM>(nIndex), 0);
	} else {
		SendDlgItemMessage(hDlg, iDialogItem, CB_SETCURSEL, 0, 0);
	}
	return S_OK;
}