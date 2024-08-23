//-----------------------------------------------------------------------------
// File: DPStage.h
//
// Desc: Support file for a DirectPlay stage.  The stage allows all
//       players connected to the same session to chat, and start a new game
//       at the same time when everyone is ready and the host player decides
//       to begin.  The host player may also reject players or close player
//       slots.  This allows host player to control who is allowed to join
//       the game.
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __DPSTAGE_H__
#define __DPSTAGE_H__

// Make sure DirectPlay higher APIs are available
#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

// Force Windows.h strict type checking
#ifndef STRICT
#define STRICT
#endif

#ifndef __DPCONNECT_H__
#include "dpconnect.h"
#endif

//-----------------------------------------------------------------------------
// Defines, and structures
//-----------------------------------------------------------------------------

#define MAX_PLAYER_SLOTS 10
#define MAX_CHAT_STRINGS 50

#define SLOT_BUTTON_MASK 0x0F
#define SLOT_BUTTON_OPEN 0x00
#define SLOT_BUTTON_CLOSED 0x01
#define SLOT_BUTTON_FULL 0x02

#define SLOT_READY_MASK 0xF0
#define SLOT_READY_CHECKED 0x10

//-----------------------------------------------------------------------------
// App specific DirectPlay messages and structures
//-----------------------------------------------------------------------------
#define STAGEMSG_SETSLOTID 1
#define STAGEMSG_SETSTAGEDATA 2
#define STAGEMSG_REJECTPLAYER 3
#define STAGEMSG_SLOTCHECK 4
#define STAGEMSG_SLOTUNCHECK 5
#define STAGEMSG_STARTGAME 6
#define STAGEMSG_CANCELGAME 7

struct STAGEMSG_GENERIC {
	DWORD dwType;
};

struct STAGEMSG_SLOT: public STAGEMSG_GENERIC {
	DWORD dwSlotNumber;
};

struct STAGEMSG_STAGEDATA: public STAGEMSG_GENERIC {
	DWORD dwSlotStatus[MAX_PLAYER_SLOTS];
	char strSlotName[MAX_PLAYER_SLOTS][MAX_PLAYER_NAME];
};

//-----------------------------------------------------------------------------
// Global variables valid for all players
//-----------------------------------------------------------------------------

extern char g_strDlgTitle[256];
extern DWORD g_dwLocalSlotNumber;
extern BOOL g_bLocalPlayerReady;
extern BOOL g_bAllowPlayerJoin;
extern DWORD g_dwNumberOfFullSlots;

//-----------------------------------------------------------------------------
// Global variables valid for only host player
//-----------------------------------------------------------------------------

extern BOOL g_dwSlotStatus[MAX_PLAYER_SLOTS];
extern char g_strSlotName[MAX_PLAYER_SLOTS][MAX_PLAYER_NAME];
extern DWORD g_dwSlotDPID[MAX_PLAYER_SLOTS];
extern DWORD g_dwNumberSlotsOpen;

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------

extern int DPStage_StartDirectPlayStage(HINSTANCE hInst);
extern BOOL CALLBACK DPStage_StageDlgProc(HWND, UINT, WPARAM, LPARAM);
extern HRESULT DPStage_StageDlgInit(HWND hDlg);
extern HRESULT DPStage_ProcessDirectPlayMessages(HWND hDlg);
extern HRESULT DPStage_HandleAppMessages(
	HWND hDlg, STAGEMSG_GENERIC* pMsg, DWORD dwMsgSize, DPID idFrom, DPID idTo);
extern HRESULT DPStage_HandleSystemMessages(
	HWND hDlg, DPMSG_GENERIC* pMsg, DWORD dwMsgSize, DPID idFrom, DPID idTo);
extern HRESULT DPStage_SendChatMessage(HWND hDlg);
extern VOID DPStage_AddChatStringToListBox(HWND hDlg, LPSTR strMsgText);
extern HRESULT DPStage_GetFreePlayerSlot(DWORD* pdwPlayerSlotID);
extern HRESULT DPStage_SendSlotDataToPlayers(HWND hDlg);
extern VOID DPStage_DisplaySlotData(
	HWND hDlg, STAGEMSG_STAGEDATA* pStageDataMsg);
extern HRESULT DPStage_AddPlayerToStage(
	HWND hDlg, DPMSG_CREATEPLAYERORGROUP* pCreateMsg);
extern HRESULT DPStage_RemovePlayerToStage(
	HWND hDlg, DPMSG_DESTROYPLAYERORGROUP* pDeleteMsg);
extern HRESULT DPStage_UpdateSessionDesc();
extern HRESULT DPStage_ChangePlayerReadyStatus(HWND hDlg, BOOL bPlayerReady);
extern HRESULT DPStage_ChangePlayerSlotStatus(HWND hDlg, DWORD dwSlot);
extern HRESULT DPStage_StartGame(HWND hDlg);
extern HRESULT DPStage_CancelGame();

#endif
