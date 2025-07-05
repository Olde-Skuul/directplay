/***************************************

	Helper functions for managing dialog controls

	By Rebecca Ann Heineman

***************************************/

#include "dpdialogs.h"
#include <tchar.h>

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
