/***************************************

	Helper functions for managing dialog controls

	By Rebecca Ann Heineman

***************************************/

#ifndef __DPDIALOGS_H__
#define __DPDIALOGS_H__

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#include <Windows.h>

extern const char* GetDirectPlayErrStr(HRESULT hr);
extern void ErrorBox(const char* pErrorMessage, HRESULT hr);
extern void EnableDlgButton(HWND hDialog, int iDialogItem, BOOL bEnable);
extern void CheckDlgItem(HWND hDialog, int iDialogItem, BOOL bCheck);
extern BOOL DlgItemIsChecked(HWND hDialog, int iDialogItem);
extern void AppendTextToEditControl(
	HWND hDialog, TCHAR* pNewLogLine, int iDialogItem, int iSecondLine);
extern HRESULT GetComboBoxGuid(
	HWND hWindow, LONG iDialogItem, GUID* pReturnGUID);
extern void DeleteConnectionList(HWND hDialog, int iDialsogItem);
extern void DeleteSessionInstanceList(HWND hDialog, int iDialogItem);
extern HRESULT GetSessionInstanceGuid(
	HWND hDialog, GUID* pSessionInstanceGUID, int iDialogItem);
extern void SelectSessionInstance(
	HWND hDialog, GUID* pSessionInstanceGUID, int iDialogItem);
#endif
