/***************************************

	Helper functions for directplay

	By Rebecca Ann Heineman

***************************************/

#ifndef __DPUTILS_H__
#define __DPUTILS_H__

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#include <Windows.h>

#include <dplay8.h>

extern HRESULT EnumAdapters(IDirectPlay8Peer* pDirectPlay8Peer, HWND hDialog,
	GUID* pSPGuid, int iDialogItem);
extern HRESULT EnumServiceProviders(IDirectPlay8Peer* pDirectPlay8Peer,
	HWND hDialog, int iDialogItem, const TCHAR* pPreferred);

#endif
