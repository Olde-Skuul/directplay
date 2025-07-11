//-----------------------------------------------------------------------------
// File: Client.cpp
//
// Desc: Slot machine client DirectPlay.
//
// Copyright (C) 1996-1999 Microsoft Corporation.  All Rights Reserved.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>
#include <stdio.h>

#include "dpdialogs.h"
#include "dpslots.h"
#include "resource.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
const DWORD SLOTWIDTH = 110;                         // Width of slot
const DWORD SLOTHEIGHT = 119;                        // Height of slot
const DWORD SLOTBORDER = 9;                          // Space between slots
const DWORD REVSPERSECOND = 1;                       // # revolutions per second
const DWORD PIXELSPERSLOT = SLOTHEIGHT - SLOTBORDER; // Dimensions
const DWORD PIXELSPERREV = PIXELSPERSLOT * SLOTSPERWHEEL;   // Rate
const DWORD PIXELSPERSECOND = PIXELSPERREV * REVSPERSECOND; // Rate
const UINT TIMERID = 1;                                     // Timer ID to use
const UINT TIMERINTERVAL = 50;                              // Timer interval
const UINT MAXSTRING = 200; // Max size of a string

// Window messages
const UINT WM_USER_UPDATEBALANCE = WM_USER + BALANCERESPONSE;
const UINT WM_USER_STARTSPINNING = WM_USER + SPINRESPONSE;

// Main window
HWND g_hwndClient = NULL;

// WHEELINFO struct for data on the slot machine wheel
struct WHEELINFO {
	DWORD dwIndex;      // Index of wheel slot to show
	DWORD dwStartTicks; // Time wheel started spinning
	DWORD dwDuration;   // Duration wheel should spin
};

#define RECTWIDTH(lpRect) ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect) ((lpRect)->bottom - (lpRect)->top)

//-----------------------------------------------------------------------------
// Name: PaintBitmap()
// Desc:
//-----------------------------------------------------------------------------
static BOOL PaintBitmap(HDC hDC, RECT* pDCRect, HBITMAP hDDB, RECT* pDDBRect)
{
	BOOL bSuccess = FALSE; // Success/fail flag

	// Create a memory DC
	HDC hMemDC = CreateCompatibleDC(hDC);
	if (!hMemDC) {
		return FALSE;
	}

	// Select bitmap into the memory DC
	// Handle to previous bitmap
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hDDB);

	// Make sure to use the stretching mode best for color pictures
	SetStretchBltMode(hDC, COLORONCOLOR);

	// Determine whether to call StretchBlt() or BitBlt()
	if ((RECTWIDTH(pDCRect) == RECTWIDTH(pDDBRect)) &&
		(RECTHEIGHT(pDCRect) == RECTHEIGHT(pDDBRect))) {
		bSuccess = BitBlt(hDC, pDCRect->left, pDCRect->top,
			pDCRect->right - pDCRect->left, pDCRect->bottom - pDCRect->top,
			hMemDC, pDDBRect->left, pDDBRect->top, SRCCOPY);
	} else {
		bSuccess = StretchBlt(hDC, pDCRect->left, pDCRect->top,
			pDCRect->right - pDCRect->left, pDCRect->bottom - pDCRect->top,
			hMemDC, pDDBRect->left, pDDBRect->top,
			pDDBRect->right - pDDBRect->left, pDDBRect->bottom - pDDBRect->top,
			SRCCOPY);
	}
	// Clean up
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);

	return bSuccess;
}

//-----------------------------------------------------------------------------
// Name: SpinWheels()
// Desc:
//-----------------------------------------------------------------------------
static BOOL SpinWheels(HWND hWnd, WHEELINFO* pWheels, HBITMAP hWheelBitmap)
{
	RECT rectBounds;
	RECT rectDC;
	RECT rectSlot;

	if (hWheelBitmap == NULL) {
		return FALSE;
	}

	HDC hDC = GetWindowDC(GetDlgItem(hWnd, IDC_SLOTS));
	if (hDC == NULL) {
		return FALSE;
	}

	if (!GetWindowRect(GetDlgItem(hWnd, IDC_SLOTS), &rectBounds)) {
		return FALSE;
	}

	const RECT* pBoundsRect = &rectBounds;

	DWORD dwWidth = static_cast<DWORD>(pBoundsRect->right - pBoundsRect->left);
	DWORD dwHeight = static_cast<DWORD>(pBoundsRect->bottom - pBoundsRect->top);
	DWORD dwXOffset = (dwWidth - (SLOTWIDTH * NUMWHEELS)) / (NUMWHEELS + 1);
	DWORD dwYOffset = (dwHeight - SLOTHEIGHT) / 2;

	SetRect(&rectDC, static_cast<int>(dwXOffset), static_cast<int>(dwYOffset),
		static_cast<int>(dwXOffset + SLOTWIDTH),
		static_cast<int>(dwYOffset + SLOTHEIGHT));

	DWORD dwStoppedCount = 0;
	for (DWORD i = 0; i < NUMWHEELS; i++) {
		if (pWheels[i].dwDuration == 0) {
			dwStoppedCount++;
		} else {
			DWORD dwTicks = GetTickCount() - pWheels[i].dwStartTicks;
			DWORD dwYStart = (dwTicks * PIXELSPERSECOND) / 1000;
			dwYStart %= PIXELSPERREV;

			if (dwTicks >= pWheels[i].dwDuration) {
				// pWheels[i].value = ((dwYStart + (PIXELSPERSLOT -
				// 1)) / PIXELSPERSLOT) % SLOTSPERWHEEL;

				SetRect(&rectSlot, 0, 0, SLOTWIDTH, SLOTHEIGHT);
				OffsetRect(&rectSlot, 0,
					static_cast<int>(pWheels[i].dwIndex * PIXELSPERSLOT));
				PaintBitmap(hDC, &rectDC, hWheelBitmap, &rectSlot);

				pWheels[i].dwDuration = 0;
				if (dwStoppedCount == (NUMWHEELS - 1)) {
					PlaySoundA(MAKEINTRESOURCEA(IDR_STOPWAVE), ghInstance,
						SND_RESOURCE);
				} else {
					PlaySoundA(MAKEINTRESOURCEA(IDR_STOPWAVE), ghInstance,
						SND_ASYNC | SND_RESOURCE);
				}
			} else {
				SetRect(&rectSlot, 0, 0, SLOTWIDTH, SLOTHEIGHT);
				OffsetRect(&rectSlot, 0, static_cast<int>(dwYStart));
				if (rectSlot.bottom > PIXELSPERREV) {
					RECT rectSlotTmp, rectDCTmp;

					// copy from bottom end of bitmap
					DWORD height = PIXELSPERREV - rectSlot.top;
					rectSlotTmp = rectSlot;
					rectSlotTmp.bottom =
						static_cast<int>(rectSlotTmp.top + height);
					rectDCTmp = rectDC;
					rectDCTmp.bottom = static_cast<int>(rectDCTmp.top + height);
					PaintBitmap(hDC, &rectDCTmp, hWheelBitmap, &rectSlotTmp);

					height = rectSlot.bottom - PIXELSPERREV;
					rectSlotTmp = rectSlot;
					rectSlotTmp.top = 0;
					rectSlotTmp.bottom = static_cast<int>(height);
					rectDCTmp = rectDC;
					rectDCTmp.top = static_cast<int>(rectDCTmp.bottom - height);
					PaintBitmap(hDC, &rectDCTmp, hWheelBitmap, &rectSlotTmp);
				} else {
					PaintBitmap(hDC, &rectDC, hWheelBitmap, &rectSlot);
				}
			}
		}
		OffsetRect(&rectDC, static_cast<int>(SLOTWIDTH + dwXOffset), 0);
	}

	return (dwStoppedCount != NUMWHEELS);
}

//-----------------------------------------------------------------------------
// Name: StartSpinning()
// Desc:
//-----------------------------------------------------------------------------
static VOID StartSpinning(WHEELINFO* lpWheels)
{
	DWORD i;

	for (i = 0; i < NUMWHEELS; i++) {
		lpWheels[i].dwStartTicks = GetTickCount();
		lpWheels[i].dwStartTicks -=
			lpWheels[i].dwIndex * PIXELSPERSLOT * 1000 / PIXELSPERREV;
		lpWheels[i].dwDuration = 1000 * (i + 1) + 1000;
	}
}

//-----------------------------------------------------------------------------
// Name: DrawWheels()
// Desc:
//-----------------------------------------------------------------------------
static VOID DrawWheels(
	WHEELINFO* pWheels, HBITMAP hWheelBitmap, HDC hDC, RECT* pBoundsRect)
{
	if (hWheelBitmap == NULL) {
		return;
	}

	RECT rectDC, rectSlot;
	DWORD dwWidth = static_cast<DWORD>(pBoundsRect->right - pBoundsRect->left);
	DWORD dwHeight = static_cast<DWORD>(pBoundsRect->bottom - pBoundsRect->top);
	DWORD dwXOffset = (dwWidth - (SLOTWIDTH * NUMWHEELS)) / (NUMWHEELS + 1);
	DWORD dwYOffset = (dwHeight - SLOTHEIGHT) / 2;

	SetRect(&rectDC, static_cast<int>(dwXOffset), static_cast<int>(dwYOffset),
		static_cast<int>(dwXOffset + SLOTWIDTH),
		static_cast<int>(dwYOffset + SLOTHEIGHT));

	for (DWORD i = 0; i < NUMWHEELS; i++) {
		SetRect(&rectSlot, 0, 0, SLOTWIDTH, SLOTHEIGHT);
		OffsetRect(
			&rectSlot, 0, static_cast<int>(pWheels[i].dwIndex * PIXELSPERSLOT));

		PaintBitmap(hDC, &rectDC, hWheelBitmap, &rectSlot);

		OffsetRect(&rectDC, static_cast<int>(SLOTWIDTH + dwXOffset), 0);
	}
}

//-----------------------------------------------------------------------------
// Name: SendBalanceRequest()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT SendBalanceRequest(DPLAYINFO* pDPInfo)
{
	MSG_BALANCEREQUEST Msg;

	ZeroMemory(&Msg, sizeof(MSG_BALANCEREQUEST));
	Msg.dwType = BALANCEREQUEST;

	return pDPInfo->pDPlay->Send(pDPInfo->dpidPlayer, DPID_SERVERPLAYER,
		static_cast<DWORD>(SENDFLAGS(pDPInfo->bIsSecure)), &Msg,
		sizeof(MSG_BALANCEREQUEST));
}

//-----------------------------------------------------------------------------
// Name: SendSpinRequest()
// Desc:
//-----------------------------------------------------------------------------
static HRESULT SendSpinRequest(DPLAYINFO* pDPInfo, DWORD dwAmountBet)
{
	MSG_SPINREQUEST Msg;

	ZeroMemory(&Msg, sizeof(MSG_SPINREQUEST));
	Msg.dwType = SPINREQUEST;
	Msg.dwAmountBet = dwAmountBet;

	return pDPInfo->pDPlay->Send(pDPInfo->dpidPlayer, DPID_SERVERPLAYER,
		static_cast<DWORD>(SENDFLAGS(pDPInfo->bIsSecure)), &Msg,
		sizeof(MSG_SPINREQUEST));
}

//-----------------------------------------------------------------------------
// Name: ClientWndProc()
// Desc:
//-----------------------------------------------------------------------------
BOOL CALLBACK ClientWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static DPLAYINFO* pDPInfo = NULL;
	static UINT idTimer = 0;
	static HBITMAP hWheelBitmap = NULL;
	static WHEELINFO WheelInfo[NUMWHEELS];
	static MSG_SPINRESPONSE SpinResponse;
	CHAR strStr[MAXSTRING];
	DWORD i;
	DWORD dwAmountBet;

	switch (uMsg) {
	case WM_INITDIALOG:
		// save the connection info pointer
		pDPInfo = (DPLAYINFO*)lParam;

		// store global window
		g_hwndClient = hWnd;

		// get slots bitmap
		hWheelBitmap =
			LoadBitmapA(ghInstance, MAKEINTRESOURCEA(IDB_SLOTSBITMAP));

		// initialize slots
		for (i = 0; i < NUMWHEELS; i++) {
			WheelInfo[i].dwIndex = ((DWORD)rand()) % SLOTSPERWHEEL;
		}
		// get starting balance
		SendBalanceRequest(pDPInfo);
		break;

	case WM_DESTROY:
		// stop the timer
		if (idTimer) {
			KillTimer(hWnd, idTimer);
			idTimer = 0;
		}

		// free the bitmap handle
		if (hWheelBitmap) {
			DeleteObject(hWheelBitmap);
			hWheelBitmap = NULL;
		}
		g_hwndClient = NULL;
		break;

	case WM_USER_UPDATEBALANCE:

		// balance is in lParam
		sprintf(strStr, "$%4d", lParam);

		// display new balance
		SetDlgItemTextA(hWnd, IDC_EDIT_BALANCE, strStr);
		break;

	case WM_USER_STARTSPINNING:

		// copy spin response message from lParam
		SpinResponse = *((MSG_SPINRESPONSE*)lParam);
		GlobalFreePtr((VOID*)lParam); // free memory

		// check for valid spin
		if (FAILED(SpinResponse.hr)) {
			SetDlgItemTextA(
				hWnd, IDC_RESULTEDIT, "You don't have enough money!");
		} else {
			// copy slot settings specified by server
			for (i = 0; i < NUMWHEELS; i++) {
				WheelInfo[i].dwIndex = SpinResponse.dwIndex[i];
			}

			// clear win or lose
			SetDlgItemTextA(hWnd, IDC_RESULTEDIT, "");

			// start things spinning
			StartSpinning(WheelInfo);
			idTimer = SetTimer(hWnd, TIMERID, TIMERINTERVAL, NULL);

			// disable spin button while spinning
			EnableDlgButton(hWnd, IDC_SPINBUTTON, FALSE);
		}
		break;

	case WM_TIMER:
		// readraw any spinning wheels
		if (!SpinWheels(hWnd, WheelInfo, hWheelBitmap)) {
			KillTimer(hWnd, idTimer);
			idTimer = 0;

			// display amount won or lost
			if (SpinResponse.dwAmountWonOrLost > 0) {
				sprintf(strStr, "You win $%d!", SpinResponse.dwAmountWonOrLost);
				PlaySoundA(MAKEINTRESOURCEA(IDR_WINWAVE), ghInstance,
					SND_ASYNC | SND_RESOURCE);
			} else if (SpinResponse.dwAmountWonOrLost < 0) {
				sprintf(
					strStr, "You lose $%d!", -SpinResponse.dwAmountWonOrLost);
				PlaySoundA(MAKEINTRESOURCEA(IDR_LOSEWAVE), ghInstance,
					SND_ASYNC | SND_RESOURCE);
			} else {
				strcpy(strStr, "");
			}

			// display win or loss
			SetDlgItemTextA(hWnd, IDC_RESULTEDIT, strStr);

			PostMessageA(
				hWnd, WM_USER_UPDATEBALANCE, 0, (LPARAM)SpinResponse.dwBalance);

			// enable spin button again
			EnableDlgButton(hWnd, IDC_SPINBUTTON, TRUE);
		}
		break;

	case WM_DRAWITEM: {
		DRAWITEMSTRUCT* diInfo;

		diInfo = (DRAWITEMSTRUCT*)lParam;

		switch (diInfo->CtlID) {
		case IDC_SLOTS:
			if (diInfo->itemAction == ODA_DRAWENTIRE) {
				DrawWheels(
					WheelInfo, hWheelBitmap, diInfo->hDC, &diInfo->rcItem);
			}
			break;
		}
	} break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_SPINBUTTON:
			// find out how much was bet
			dwAmountBet = 0;

			// one dollar
			if (DlgItemIsChecked(hWnd, IDC_BET1CHECK)) {
				dwAmountBet += 1;
			}

			// five dollars
			if (DlgItemIsChecked(hWnd, IDC_BET2CHECK)) {
				dwAmountBet += 5;
			}

			// ten dollars
			if (DlgItemIsChecked(hWnd, IDC_BET3CHECK)) {
				dwAmountBet += 10;
			}

			// ask server for results of spin using this bet
			SendSpinRequest(pDPInfo, dwAmountBet);
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
// Name: ClientApplicationMessage()
// Desc:
//-----------------------------------------------------------------------------
VOID ClientApplicationMessage(DPLAYINFO* /* pDPInfo */, DPMSG_GENERIC* pMsg,
	DWORD /* dwMsgSize */, DPID /* idFrom */, DPID /* idTo */)
{
	switch (pMsg->dwType) {
	case BALANCERESPONSE: {
		MSG_BALANCERESPONSE* pBalance = (MSG_BALANCERESPONSE*)pMsg;

		PostMessageA(g_hwndClient, WM_USER_UPDATEBALANCE, 0,
			static_cast<LPARAM>(pBalance->dwBalance));
		break;
	}

	case SPINRESPONSE: {
		MSG_SPINRESPONSE* pSpin = (MSG_SPINRESPONSE*)pMsg;

		// make a copy of the message so we can pass it to the wndproc
		MSG_SPINRESPONSE* pSpinCopy =
			(MSG_SPINRESPONSE*)GlobalAllocPtr(GHND, sizeof(MSG_SPINRESPONSE));
		if (pSpinCopy == NULL) {
			break;
		}

		*pSpinCopy = *pSpin;

		PostMessageA(g_hwndClient, WM_USER_STARTSPINNING, 0, (LPARAM)pSpinCopy);
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Name: ClientSystemMessage()
// Desc:
//-----------------------------------------------------------------------------
VOID ClientSystemMessage(DPLAYINFO* pDPInfo, DPMSG_GENERIC* pMsg,
	DWORD /* dwMsgSize */, DPID /* idFrom */, DPID idTo)
{
	// The body of each case is there so you can set a breakpoint and examine
	// the contents of the message received.
	switch (pMsg->dwType) {
	case DPSYS_CREATEPLAYERORGROUP: {
		// DPMSG_CREATEPLAYERORGROUP* p = (DPMSG_CREATEPLAYERORGROUP*)pMsg;
		break;
	}

	case DPSYS_DESTROYPLAYERORGROUP: {
		// DPMSG_DESTROYPLAYERORGROUP* p = (DPMSG_DESTROYPLAYERORGROUP*)pMsg;
		break;
	}

	case DPSYS_ADDPLAYERTOGROUP: {
		// DPMSG_ADDPLAYERTOGROUP* p = (DPMSG_ADDPLAYERTOGROUP*)pMsg;
		break;
	}

	case DPSYS_DELETEPLAYERFROMGROUP: {
		// DPMSG_DELETEPLAYERFROMGROUP* p = (DPMSG_DELETEPLAYERFROMGROUP*)pMsg;
		break;
	}

	case DPSYS_SESSIONLOST: {
		// DPMSG_SESSIONLOST* p = (DPMSG_SESSIONLOST*)pMsg;
		break;
	}

	case DPSYS_HOST: {
		// DPMSG_HOST* p = (DPMSG_HOST*)pMsg;
		break;
	}

	case DPSYS_SETPLAYERORGROUPDATA: {
		// DPMSG_SETPLAYERORGROUPDATA* p = (DPMSG_SETPLAYERORGROUPDATA*)pMsg;
		break;
	}

	case DPSYS_SETPLAYERORGROUPNAME: {
		// DPMSG_SETPLAYERORGROUPNAME* p = (DPMSG_SETPLAYERORGROUPNAME*)pMsg;
		break;
	}

	case DPSYS_SECUREMESSAGE: {
		DPMSG_SECUREMESSAGE* p = (DPMSG_SECUREMESSAGE*)pMsg;

		ClientApplicationMessage(pDPInfo, (DPMSG_GENERIC*)p->lpData,
			p->dwDataSize, p->dpIdFrom, idTo);

		break;
	}
	}
}
