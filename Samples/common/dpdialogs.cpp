/***************************************

	Helper functions for managing dialog controls

	By Rebecca Ann Heineman

***************************************/

#include "dpdialogs.h"
#include <dplay.h>
#include <dplay8.h>
#include <stdio.h>
#include <tchar.h>
#include <windowsx.h>

/***************************************

	Convert a DirectPlay error into a string

***************************************/

const char* GetDirectPlayErrStr(HRESULT hr)
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
		//  case DPERR_INVALIDPARAM: return "DPERR_INVALIDPARAM";  dup value
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
		//  case DPERR_NOMEMORY: return "DPERR_NOMEMORY";     dup value
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
	static char strTemp[16];
	sprintf(strTemp, "0x%08X", hr);
	return strTemp;
}

/***************************************

	Display an error message box

	Press "OK" to continue

***************************************/

void ErrorBox(const char* pErrorMessage, HRESULT hr)
{
	char Temp[256];
	sprintf(Temp, pErrorMessage, GetDirectPlayErrStr(hr));
	MessageBoxA(NULL, Temp, "Error found", MB_OK);
}

/***************************************

	Enable or disable a dialog item

***************************************/

void EnableDlgButton(HWND hDialog, int iDialogItem, BOOL bEnable)
{
	EnableWindow(GetDlgItem(hDialog, iDialogItem), bEnable);
}

/***************************************

	Enable or disable a checkbox

***************************************/

void CheckDlgItem(HWND hDialog, int iDialogItem, BOOL bCheck)
{
	SendDlgItemMessageA(hDialog, iDialogItem, BM_SETCHECK,
		static_cast<WPARAM>(bCheck ? BST_CHECKED : BST_UNCHECKED), 0);
}

/***************************************

	Get the check state of a dialog box

***************************************/

BOOL DlgItemIsChecked(HWND hDialog, int iDialogItem)
{
	return SendDlgItemMessageA(hDialog, iDialogItem, BM_GETCHECK, 0, 0) ==
		BST_CHECKED;
}

/***************************************

	Appends a string of text to an edit control

***************************************/

void AppendTextToEditControl(
	HWND hDialog, TCHAR* pNewLogLine, int iDialogItem, int iSecondLine)
{
	static TCHAR strText[1024 * 10];

	HWND hEdit = GetDlgItem(hDialog, iDialogItem);
	SendMessage(hEdit, WM_SETREDRAW, FALSE, 0);
	GetWindowText(hEdit, strText, 1024 * 9);

	_tcscat(strText, pNewLogLine);

	int iSecndIndex = 0;
	if (SendMessage(hEdit, EM_GETLINECOUNT, 0, 0) > iSecondLine) {
		iSecndIndex = static_cast<int>(SendMessage(hEdit, EM_LINEINDEX, 1, 0));
	}
	SetWindowText(hEdit, &strText[iSecndIndex]);

	SendMessage(hEdit, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hEdit, NULL, TRUE);
	UpdateWindow(hEdit);
}

/***************************************

	Returns GUID stored with a combo box item

***************************************/

HRESULT GetComboBoxGuid(HWND hWindow, LONG iDialogItem, GUID* pReturnGUID)
{
	// Get index of selected item
	LRESULT iIndex =
		SendDlgItemMessageA(hWindow, iDialogItem, CB_GETCURSEL, 0, 0);
	if (iIndex == CB_ERR) {
		return DPNERR_GENERIC;
	}

	// Get data associated with this item
	iIndex = SendDlgItemMessageA(
		hWindow, iDialogItem, CB_GETITEMDATA, (WPARAM)iIndex, 0);
	if ((iIndex == CB_ERR) || (iIndex == 0)) {
		return DPNERR_GENERIC;
	}

	// Data is a pointer to a guid
	*pReturnGUID = *((GUID*)iIndex);
	return DPN_OK;
}

/***************************************

	Delete a list of connections

***************************************/

void DeleteConnectionList(HWND hDialog, int iDialogItem)
{
	// Destroy the GUID's stored with each service provider name
	WPARAM i = 0;
	for (;;) {
		// Get data pointer stored with item
		LRESULT pData =
			SendDlgItemMessageA(hDialog, iDialogItem, CB_GETITEMDATA, i, 0);
		if (pData == CB_ERR) {
			break;
		}

		if (pData != 0) {
			GlobalFreePtr((VOID*)pData);
		}
		++i;
	}

	// Delete all items in combo box
	SendDlgItemMessageA(hDialog, iDialogItem, CB_RESETCONTENT, 0, 0);
}

/***************************************

	Delete a list of sessions

***************************************/

void DeleteSessionInstanceList(HWND hDialog, int iDialogItem)
{
	// Destroy the GUID's stored with each session name
	WPARAM i = 0;
	for (;;) {
		// Get data pointer stored with item
		LRESULT pData =
			SendDlgItemMessageA(hDialog, iDialogItem, LB_GETITEMDATA, i, 0);

		// Error?
		if (pData == CB_ERR) {
			break;
		}

		// Valid data to delete?
		if (pData) {
			GlobalFreePtr((VOID*)pData);
		}
		++i;
	}

	// Delete all items in list
	SendDlgItemMessageA(hDialog, iDialogItem, LB_RESETCONTENT, 0, 0);
}

/***************************************

	Get the GUID from a session list

***************************************/

HRESULT GetSessionInstanceGuid(
	HWND hDialog, GUID* pSessionInstanceGUID, int iDialogItem)
{
	// Get guid for session
	LRESULT iIndex =
		SendDlgItemMessageA(hDialog, iDialogItem, LB_GETCURSEL, 0, 0);
	if (iIndex == LB_ERR) {
		return DPNERR_GENERIC;
	}

	iIndex = SendDlgItemMessageA(
		hDialog, iDialogItem, LB_GETITEMDATA, (WPARAM)iIndex, 0);
	if ((iIndex == LB_ERR) || (iIndex == 0)) {
		return DPNERR_GENERIC;
	}

	*pSessionInstanceGUID = *((GUID*)iIndex);
	return DPN_OK;
}

/***************************************

	Select a session instance using a GUID

***************************************/

void SelectSessionInstance(
	HWND hDialog, GUID* pSessionInstanceGUID, int iDialogItem)
{
	WPARAM i = 0;
	WPARAM iIndex = 0;

	// Loop over the GUID's stored with each session name
	// to find the one that matches what was passed in
	for (;;) {
		// Get data pointer stored with item
		LRESULT pData =
			SendDlgItemMessageA(hDialog, iDialogItem, LB_GETITEMDATA, i, 0);
		if (pData == CB_ERR) {
			break;
		}

		if (pData == 0) {
			continue;
		}

		// Guid matches
		if (IsEqualGUID(*pSessionInstanceGUID, *((GUID*)pData))) {
			iIndex = i; // Store index of this string
			break;
		}

		++i;
	}

	// Select this item
	SendDlgItemMessageA(hDialog, iDialogItem, LB_SETCURSEL, iIndex, 0);
}