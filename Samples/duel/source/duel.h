//-----------------------------------------------------------------------------
// File: Duel.h
//
// Desc: Duel include file
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DUEL_H__
#define __DUEL_H__

// bcc32 does not support nameless unions in C mode
#if defined(__BORLANDC__) && !defined(__cplusplus)
#define NONAMELESSUNION
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <windowsx.h>

// Older compilers complain about mmsystem.h
#if defined(_MSC_VER)
#pragma warning(push)
// nameless struct/union
#pragma warning(disable : 4201)
#endif

// Depends on windows.h
#include <mmsystem.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

//-----------------------------------------------------------------------------
// Application constants
//-----------------------------------------------------------------------------

// User messages
#define UM_LAUNCH WM_USER
#define UM_ABORT WM_USER + 1
#define UM_RESTARTTIMER WM_USER + 2

// program states
enum { PS_SPLASH, PS_ACTIVE, PS_REST };

#define MAX_SCREEN_X 639
#define MAX_SCREEN_Y 479
#define MAX_SPNAME 50
#define MAX_CLASSNAME 50
#define MAX_WINDOWTITLE 50
#define MAX_ERRORMSG 256
#define MAX_FONTNAME 50
#define MAX_HELPMSG 512

#define RECEIVE_TIMER_ID 1
#define RECEIVE_TIMEOUT 1000 // in milliseconds

#define ENUM_TIMER_ID 2
#define ENUM_TIMEOUT 2000 // in milliseconds

// Default window size
#define MAX_DEFWIN_X 640
#define MAX_DEFWIN_Y 480

// Tree view image info
#define CX_BITMAP 25
#define CY_BITMAP 25
#define NUM_BITMAPS 2

// registry info
#define DUEL_KEY "Software\\Microsoft\\Duel"

// Main application window handle
extern HWND g_hwndMain;

// Duel registry key handle
extern HKEY g_hDuelKey;

// Application instance handle
extern HINSTANCE g_hInst;

// Show FPS ?
extern BOOL g_bShowFrameCount;

// Is the application active ?
extern BOOL g_bIsActive;

// User keyboard input
extern DWORD g_dwKeys;

// Last frame's keyboard input
extern DWORD g_dwOldKeys;

// Window or FullScreen mode ?
extern BOOL g_bFullscreen;

// client rectangle of main window
extern RECT g_rcWindow;

// sends are reliable
extern BOOL g_bReliable;

// asynchronous sends
extern BOOL g_bAsync;

// asynchronous sends supported
extern BOOL g_bAsyncSupported;

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

extern int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int);
extern LONG WINAPI MainWndproc(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern HRESULT InitApplication(HINSTANCE hInst);
extern VOID CleanupApplication();
extern VOID ShowError(int err);
extern VOID UpdateTitle();
extern VOID DoHelp();
extern HRESULT ReadRegKey(
	HKEY hKey, char* strName, char* strValue, DWORD dwLength, char* strDefault);
extern HRESULT WriteRegKey(HKEY hKey, char* strName, char* strValue);

#endif
