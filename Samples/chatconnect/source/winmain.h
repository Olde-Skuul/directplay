/***************************************

	Declarations for winmain.cpp

***************************************/

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

// Dplay.h has to be included after windows.h
#include <dplay.h>
#include <dplobby.h>

// Maximum number of chat strings
#define MAX_CHAT_STRINGS 50

extern GUID g_AppGUID;
extern HKEY g_hDPlaySampleRegKey;
extern BYTE* g_pvDPMsgBuffer;
extern DWORD g_dwDPMsgBufferSize;
extern DWORD g_dwNumberOfActivePlayers;
extern CHAR g_strAppName[256];

/***************************************

	Functions

***************************************/

extern INT APIENTRY WinMain(
	HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow);
extern HRESULT DoConnectAndGame(HINSTANCE hInst);
extern int DoChatClient(HINSTANCE hInst);
extern INT_PTR CALLBACK ChatDlgProc(
	HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern HRESULT OnInitDialog(HWND hDlg);
extern VOID DisplayNumberPlayersInChat(HWND hDlg);
extern HRESULT ProcessDirectPlayMessages(HWND hDlg);
extern HRESULT HandleSystemMessages(HWND, DPMSG_GENERIC*, DWORD, DPID, DPID);
extern HRESULT SendChatMessage(HWND hDlg);
extern VOID AddChatStringToListBox(HWND hDlg, LPSTR strMsgText);
extern HRESULT ReadRegKey(
	HKEY hKey, CHAR* strName, CHAR* strValue, DWORD dwLength, CHAR* strDefault);
extern HRESULT WriteRegKey(HKEY hKey, CHAR* strName, CHAR* strValue);

#endif
