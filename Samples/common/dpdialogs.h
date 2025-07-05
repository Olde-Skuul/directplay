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

extern void AppendTextToEditControl(
	HWND hDialog, TCHAR* pNewLogLine, int iDialogItem, int iSecondLine);

#endif
