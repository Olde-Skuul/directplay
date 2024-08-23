//-----------------------------------------------------------------------------
// File: Duel.cpp
//
// Desc: Multi-player game
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

// Generate DirectX GUIDs in this file only
#define INITGUID

#include "duel.h"
#include "resource.h"

#include "diutil.h"
#include "dputil.h"
#include "gameproc.h"
#include "gfx.h"
#include "lobby.h"

#include "dpconnect.h"
#include "dsutil.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

static BOOL g_bReinitialize; // Used for switching display modes

// Main application window handle
HWND g_hwndMain;

// Duel registry key handle
HKEY g_hDuelKey = NULL;

// Application instance handle
HINSTANCE g_hInst;

// Show FPS ?
BOOL g_bShowFrameCount = TRUE;

// Is the application active ?
BOOL g_bIsActive;

// User keyboard input
DWORD g_dwKeys;

// Last frame's keyboard input
DWORD g_dwOldKeys;

// Window or FullScreen mode ?
BOOL g_bFullscreen = FALSE;

// client rectangle of main window
RECT g_rcWindow;

// sends are reliable
BOOL g_bReliable;

// asynchronous sends
BOOL g_bAsync;

// asynchronous sends supported
BOOL g_bAsyncSupported;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc:
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	MSG msg;
	BOOL bLaunchedByLobby;
	HRESULT hr;

	g_hInst = hInstance;

	// Read information from registry
	RegCreateKeyExA(HKEY_CURRENT_USER, DUEL_KEY, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &g_hDuelKey, NULL);

	ReadRegKey(
		g_hDuelKey, "Player Name", g_strLocalPlayerName, MAX_PLAYER_NAME, "");
	ReadRegKey(
		g_hDuelKey, "Session Name", g_strSessionName, MAX_SESSION_NAME, "");
	ReadRegKey(g_hDuelKey, "Preferred Provider", g_strPreferredProvider,
		MAX_SESSION_NAME, "");

	CoInitialize(NULL);

	if (FAILED(InitApplication(hInstance))) {
		return 0;
	}

	// See if we were launched from a lobby server
	hr = DPConnect_CheckForLobbyLaunch(&bLaunchedByLobby);
	if (FAILED(hr)) {
		return 1;
	}

	if (bLaunchedByLobby) {
		// Start game
		PostMessage(g_hwndMain, UM_LAUNCH, 0, 0);
		g_bIsActive = TRUE;
	}

	g_dwFrameTime = timeGetTime();

	for (;;) {
		if (g_bIsActive) {
			// Any windows messages ? (returns immediately)
			if (PeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				if (!GetMessageA(&msg, NULL, 0, 0))
					break;

				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			} else {
				// Poll our receive queue. Polling is used in the sample only
				// for simplicity. Receiving messages using an event is the
				// recommended way.
				if (g_nProgramState != PS_SPLASH) {
					ReceiveMessages();
					LobbyMessageReceive(LMR_PROPERTIES);
				}

				// update screen
				if (!UpdateFrame()) {
					ExitGame(); // posts QUIT msg
				}
			}
		} else {
			// Any windows messages ? (blocks until a message arrives)
			if (!GetMessageA(&msg, NULL, 0, 0)) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	CoUninitialize();

	// Write information to the registry
	WriteRegKey(g_hDuelKey, "Player Name", g_strLocalPlayerName);
	WriteRegKey(g_hDuelKey, "Session Name", g_strSessionName);
	WriteRegKey(g_hDuelKey, "Preferred Provider", g_strPreferredProvider);

	return (int)msg.wParam;
}

//-----------------------------------------------------------------------------
// Name: MainWndproc()
// Desc: Callback for all Windows messages
//-----------------------------------------------------------------------------
LONG WINAPI MainWndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg) {
	case WM_SIZE:
	case WM_MOVE:
		// Get the client rectangle
		if (g_bFullscreen) {
			SetRect(&g_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN),
				GetSystemMetrics(SM_CYSCREEN));
		} else {
			GetClientRect(hWnd, &g_rcWindow);
			ClientToScreen(hWnd, (POINT*)&g_rcWindow);
			ClientToScreen(hWnd, (POINT*)&g_rcWindow + 1);
		}
		break;

	case WM_ACTIVATE:
		// Ignore this message during reinitializing graphics
		if (g_bReinitialize) {
			return 0;
		}

		// When we are deactivated, although we don't update our screen, we
		// still need to to empty our receive queue periodically as
		// messages will pile up otherwise. Polling the receive queue
		// continuously even when we are deactivated causes our app to
		// consume all the CPU time. To avoid hogging the CPU, we block on
		// GetMessage() WIN API and setup a timer to wake ourselves up at
		// regular intervals to process our messages.

		if (LOWORD(wParam) == WA_INACTIVE) {
			// Aeactivated
			g_bIsActive = FALSE;
			if (PS_ACTIVE == g_nProgramState) {
				SetTimer(hWnd, RECEIVE_TIMER_ID, RECEIVE_TIMEOUT, NULL);
			}
		} else {
			// Activated
			g_bIsActive = TRUE;
			if (PS_ACTIVE == g_nProgramState) {
				KillTimer(hWnd, RECEIVE_TIMER_ID);
			}
		}

		// set game palette, if activated in game mode
		if (g_bIsActive && (g_nProgramState != PS_SPLASH)) {
			SetGamePalette();
		}

		DIUtil_ReacquireInputDevices();

		return 0;

	case WM_CREATE:
		break;

	case WM_SYSKEYUP:
		switch (wParam) {
		// Handle ALT+ENTER (fullscreen/window mode)
		case VK_RETURN:
			// Mode switch is allowed only during the game
			if (g_nProgramState == PS_ACTIVE) {
				g_bReinitialize = TRUE;
				ReleaseLocalData(); // only sound buffers have to be rels'd
									// anyway.
				CleanupGameSounds();
				DIUtil_CleanupInput();
				CleanupGraphics();
				DestroyWindow(g_hwndMain);
				g_bFullscreen = !g_bFullscreen;
				InitGraphics();
				DIUtil_InitInput(g_hwndMain);
				InitializeGameSounds();
				InitLocalSoundData();
				g_bReinitialize = FALSE;
			}
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case 'a':
		case 'A':
			// Toggle Async sends on/off
			if (g_bAsyncSupported) {
				g_bAsync = !g_bAsync;
				UpdateTitle(); // caption bar status
			}
			break;

		case 'r':
		case 'R':
			// Toggle reliable sends
			g_bReliable = !g_bReliable;
			UpdateTitle();
			break;

		case VK_F1:
			// Display help
			DoHelp();
			break;

		case VK_F5:
			// Toggle frame rate display
			g_bShowFrameCount = !g_bShowFrameCount;
			if (g_bShowFrameCount) {
				g_dwFrameCount = 0;
				g_dwFrameTime = timeGetTime();
			}
			break;

		case VK_RETURN:
			// Launch game setup wizard
			if ((g_nProgramState == PS_SPLASH) && !g_bFullscreen) {
				int nExitCode;
				nExitCode = DPConnect_StartDirectPlayConnect(g_hInst, FALSE);

				// Figure out what happened, and post a reflecting message
				if (nExitCode == EXITCODE_FORWARD) {
					PostMessageA(g_hwndMain, UM_LAUNCH, 0, 0);
				}

				if (nExitCode == EXITCODE_QUIT) {
					PostMessageA(g_hwndMain, UM_ABORT, 0, 0);
				}

				if (nExitCode == EXITCODE_LOBBYCONNECT) {
					PostMessageA(g_hwndMain, UM_LAUNCH, 0, 0);
				}

				if (nExitCode == EXITCODE_ERROR) {
					MessageBoxA(g_hwndMain,
						"Mutliplayer connect failed. "
						"The sample will now quit.",
						"DirectPlay Sample", MB_OK | MB_ICONERROR);
					PostMessageA(g_hwndMain, UM_ABORT, 0, 0);
				}
			}
			break;

		case VK_ESCAPE:
		case VK_F12:
			// Exit the game
			ExitGame();
			return 0;
		}
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (g_nProgramState == PS_SPLASH) {
			// Display the splash screen
			BltSplashScreen(NULL);
		}

		EndPaint(hWnd, &ps);
		return 1;

	case UM_LAUNCH:
	case UM_ABORT:
		// if we were launched by the lobby and not (failed to finish a lobby
		// launch) where wParam is bLobbyLaunched
		if (msg == UM_LAUNCH) {
			// Init lobby msg support for reporting score
			// Note that we don't release the lobby object
			LobbyMessageInit();

			// Start the game in rest mode
			g_nProgramState = PS_REST;
			LaunchGame();
			return 1;
		}
		// Else aborting
		ExitGame();
		return 1;

	case WM_TIMER:
		ReceiveMessages();
		LobbyMessageReceive(LMR_PROPERTIES);
		break;

	case WM_DESTROY:
		// If g_bReinitialize is TRUE don't quit, we are just switching
		// display modes
		if (!g_bReinitialize) {
			CleanupApplication();
			PostQuitMessage(0);
		}
		return 0;

	default:
		break;
	}

	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// Name: InitApplication()
// Desc: Do that initialization stuff...
//-----------------------------------------------------------------------------
HRESULT InitApplication(HINSTANCE hInst)
{
	WNDCLASSA wndClass = {CS_DBLCLKS, MainWndproc, 0, 0, hInst,
		LoadIconA(hInst, MAKEINTRESOURCE(IDI_MAIN)),
		LoadCursorA(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL,
		"DuelClass"};
	RegisterClassA(&wndClass);

	// Initialize all components
	if (FAILED(InitGraphics())) {
		return E_FAIL;
	}

	if (FAILED(DIUtil_InitInput(g_hwndMain))) {
		return E_FAIL;
	}

	if (FAILED(InitializeGameSounds())) {
		// Can play game without sound. Do not exit
	}

	// Start in splash mode
	g_nProgramState = PS_SPLASH;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CleanupApplication()
// Desc: Calls clean up on all components
//-----------------------------------------------------------------------------
VOID CleanupApplication()
{
	CleanupComm();
	CleanupGameSounds();
	CleanupGraphics();
	DIUtil_CleanupInput();
	DPLobbyRelease(); // in case we were doing lobby messages
}

//-----------------------------------------------------------------------------
// Name: ShowError()
// Desc: Displays error to the user
//-----------------------------------------------------------------------------
VOID ShowError(int iStrID)
{
	char strMsg[MAX_ERRORMSG];
	LoadStringA(g_hInst, (UINT)iStrID, strMsg, MAX_ERRORMSG);
	MessageBoxA(g_hwndMain, strMsg, "Duel Message", MB_OK);
}

//-----------------------------------------------------------------------------
// Name: UpdateTitle()
// Desc: Updates the window title based on application status
//-----------------------------------------------------------------------------
VOID UpdateTitle()
{
	// Build the window title
	char strTitle[MAX_WINDOWTITLE] = "Duel";

	// State options in window title
	if (g_bHostPlayer | g_bUseProtocol | g_bReliable | g_bAsync) {
		strcat(strTitle, " - |");
		if (g_bHostPlayer) {
			strcat(strTitle, " Host |");
		}
		if (g_bUseProtocol) {
			strcat(strTitle, " Protocol |");
		}
		if (g_bReliable) {
			strcat(strTitle, " Reliable |");
		}
		if (g_bAsync) {
			strcat(strTitle, " Async |");
		}
	}

	// Change window title
	SetWindowTextA(g_hwndMain, strTitle);
}

//-----------------------------------------------------------------------------
// Name: DoHelp()
// Desc: Display a Help summary in a message box.
//-----------------------------------------------------------------------------
VOID DoHelp()
{
	char strHelpMsg[MAX_HELPMSG];
	LoadStringA(g_hInst, IDS_DUEL_HELP, strHelpMsg, MAX_HELPMSG);
	MessageBoxA(g_hwndMain, strHelpMsg, "DUEL", MB_OK);
}

//-----------------------------------------------------------------------------
// Name: ReadRegKey()
// Desc: Read a registry key
//-----------------------------------------------------------------------------
HRESULT ReadRegKey(
	HKEY hKey, char* strName, char* strValue, DWORD dwLength, char* strDefault)
{
	DWORD dwType;

	LONG bResult = RegQueryValueExA(
		hKey, strName, 0, &dwType, (LPBYTE)strValue, &dwLength);
	if (bResult != ERROR_SUCCESS) {
		strcpy(strValue, strDefault);
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: WriteRegKey()
// Desc: Writes a registry key
//-----------------------------------------------------------------------------
HRESULT WriteRegKey(HKEY hKey, char* strName, char* strValue)
{
	LONG bResult = RegSetValueExA(
		hKey, strName, 0, REG_SZ, (LPBYTE)strValue, strlen(strValue) + 1);
	if (bResult != ERROR_SUCCESS) {
		return E_FAIL;
	}

	return S_OK;
}
