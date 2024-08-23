//----------------------------------------------------------------------------
// File: WinMain.h
//
// Desc: The main game file for the SimpleConnect sample.  It connects
//       players together with two dialog boxes to prompt users on the
//       connection settings to join or create a session. After the user
//       connects to a sesssion, the sample displays a multiplayer stage.
//
//       After a new game has started the sample begins a very simplistic
//       game called "The Greeting Game".  When two or more players are
//       connected to the game, the players have the option of sending a single
//       simple DirectPlay message to all of the other players.
//
// Copyright (c) 1999 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef __WINMAIN_H__
#define __WINMAIN_H__

// Make sure DirectPlay higher APIs are available
#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

// Force Windows.h strict type checking
#ifndef STRICT
#define STRICT
#endif

// Needed for types
#include <windows.h>

#include <dplay.h>

struct GAMEMSG_GENERIC {
	DWORD dwType;
};

extern char g_strAppName[256];

// App's guid
extern GUID g_AppGUID;

extern BYTE* g_pvDPMsgBuffer;
extern DWORD g_dwDPMsgBufferSize;

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------

extern INT APIENTRY WinMain(
	HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow);
extern HRESULT DoConnectAndGame(HINSTANCE hInst);
extern int DoGreetingGame(HINSTANCE hInst);
extern BOOL CALLBACK GreetingDlgProc(
	HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern HRESULT OnInitDialog(HWND hDlg);
extern VOID DisplayNumberPlayersInGame(HWND hDlg);
extern HRESULT ProcessDirectPlayMessages(HWND hDlg);
extern HRESULT HandleAppMessages(HWND, GAMEMSG_GENERIC*, DWORD, DPID, DPID);
extern HRESULT HandleSystemMessages(HWND, DPMSG_GENERIC*, DWORD, DPID, DPID);
extern HRESULT WaveToAllPlayers();
extern HRESULT DisplayPlayerWave(HWND hDlg, DPID idFrom);
extern HRESULT ReadRegKey(
	HKEY hKey, char* strName, char* strValue, DWORD dwLength, char* strDefault);
extern HRESULT WriteRegKey(HKEY hKey, char* strName, char* strValue);

#endif
