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
extern const char g_strAppName[];

/***************************************

	Functions

***************************************/

extern INT APIENTRY WinMain(
	HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, INT nCmdShow);

#endif
