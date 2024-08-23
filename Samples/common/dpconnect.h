/***************************************

	Declarations for dpconnect.cpp

***************************************/

#ifndef __DPCONNECT_H__
#define __DPCONNECT_H__

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

// Dplay.h has to be included after windows.h
#include <dplay.h>
#include <dplobby.h>

/***************************************

	Constants

***************************************/

// Length of a player's name for session packets
#define MAX_PLAYER_NAME 14

// Max length of a session's name
#define MAX_SESSION_NAME 256

// Dialog success, so go forward
#define EXITCODE_FORWARD 1

// Dialog canceled, show previous dialog
#define EXITCODE_BACKUP 2

// Dialog quit, close app
#define EXITCODE_QUIT 3

// Dialog error, terminate app
#define EXITCODE_ERROR 4

// Dialog connected from lobby, connect success
#define EXITCODE_LOBBYCONNECT 5

// The player was rejected from the game, so
// the previous connection dialog should be shown
#define EXITCODE_REJECTED 6

// The DirectPlay session was lost, so
// the previous connection dialog should be shown
#define EXITCODE_SESSIONLOST 7

/***************************************

	Structures

***************************************/

// Information about a session
struct DPSessionInfo {
	GUID guidSession;
	CHAR szSession[MAX_SESSION_NAME];
	DPSessionInfo* pDPSINext;
};

extern CHAR g_strPreferredProvider[MAX_SESSION_NAME];
extern CHAR g_strSessionName[MAX_SESSION_NAME];
extern CHAR g_strLocalPlayerName[MAX_PLAYER_NAME];

// DirectPlay object pointer
extern LPDIRECTPLAY4 g_pDP;

// Lobby object pointer
extern LPDIRECTPLAYLOBBY3 g_pDPLobby;

// Connection settings (Allocated as array of BYTE)
extern LPDPLCONNECTION g_pDPLConnection;

// Our player id
extern DPID g_LocalPlayerDPID;

// Head session of the linked list
extern DPSessionInfo g_DPSIHead;

// True if looking for a game
extern BOOL g_bSearchingForSessions;

// True if waiting for connection
extern BOOL g_bWaitForConnectionSettings;

// Flag indicating if we are hosting
extern BOOL g_bHostPlayer;

// DirectPlay Protocol messaging
extern BOOL g_bUseProtocol;

// DP Message event, if used
extern HANDLE g_hDPMessageEvent;

/***************************************

	Functions

***************************************/

extern INT_PTR DPConnect_StartDirectPlayConnect(
	HINSTANCE hInst, BOOL bBackTrack);
extern INT_PTR CALLBACK DPConnect_ConnectionsDlgProc(
	HWND, UINT, WPARAM, LPARAM);
extern HRESULT DPConnect_ConnectionsDlgFillListBox(HWND hDlg);
extern BOOL FAR PASCAL DPConnect_EnumConnectionsCallback(
	LPCGUID, VOID*, DWORD, LPCDPNAME, DWORD, VOID*);
extern HRESULT DPConnect_ConnectionsDlgOnOK(HWND hDlg);
extern VOID DPConnect_ConnectionsDlgCleanup(HWND hDlg);
extern INT_PTR CALLBACK DPConnect_SessionsDlgProc(HWND, UINT, WPARAM, LPARAM);
extern VOID DPConnect_SessionsDlgInitListbox(HWND hDlg);
extern HRESULT DPConnect_SessionsDlgShowGames(HWND hDlg);
extern BOOL FAR PASCAL DPConnect_EnumSessionsCallback(
	LPCDPSESSIONDESC2, DWORD*, DWORD, VOID*);
extern HRESULT DPConnect_SessionsDlgJoinGame(HWND hDlg);
extern HRESULT DPConnect_SessionsDlgCreateGame(HWND hDlg);
extern INT_PTR CALLBACK DPConnect_CreateSessionDlgProc(
	HWND, UINT, WPARAM, LPARAM);
extern VOID DPConnect_SessionsDlgCleanup();
extern HRESULT DPConnect_WaitForLobbyLaunch(HWND hParentDlg);
extern HRESULT DPConnect_CheckForLobbyLaunch(BOOL* pbLaunchedByLobby);
extern HRESULT DPConnect_DoLobbyLaunch();
extern INT_PTR CALLBACK DPConnect_LobbyWaitDlgProc(HWND, UINT, WPARAM, LPARAM);
extern HRESULT DPConnect_GetLobbyConnectionSettingsMessage();

#endif
