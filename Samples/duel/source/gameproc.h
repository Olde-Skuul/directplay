//-----------------------------------------------------------------------------
// File: GameProc.h
//
// Desc: Game processing routines
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __GAMEPROC_H__
#define __GAMEPROC_H__

#ifndef IDIRECTPLAY2_OR_GREATER
#define IDIRECTPLAY2_OR_GREATER
#endif

// Include first
#include <windows.h>

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

#include <ddraw.h>
#include <dplay.h>

#ifndef __DSUTIL_H__
#include "dsutil.h"
#endif

#define MAX_SHIP_X (MAX_SCREEN_X - 32)
#define MAX_SHIP_Y (MAX_SCREEN_Y - 32)
#define MAX_SHIP_FRAME 40
#define MAX_BULLET_X (MAX_SCREEN_X - 3)
#define MAX_BULLET_Y (MAX_SCREEN_Y - 3)
#define MAX_BULLET_FRAME 400

#define NUM_SHIP_TYPES 4

#define DEF_SHOW_DELAY (2000)
#define MAX_BUFFER_SIZE 256

// interval between position updates in milliseconds (25 FPS)
#define UPDATE_INTERVAL 40
// synchronize once every second
#define SYNC_INTERVAL 1000
// time for which a ship is disabled after a hit
#define HIDE_TIMEOUT 5000

// Keyboard commands
#define KEY_STOP 0x00000001l
#define KEY_DOWN 0x00000002l
#define KEY_LEFT 0x00000004l
#define KEY_RIGHT 0x00000008l
#define KEY_UP 0x00000010l
#define KEY_FIRE 0x00000020l
#define KEY_ENGINEOFF 0x00000040l

// Offsets for the bullet bitmap
#define BULLET_X 304
#define BULLET_Y 0

struct FRAG {
	double dPosX;
	double dPosY;
	double dVelX;
	double dVelY;
	LPDIRECTDRAWSURFACE pdds;
	RECT src;
	BOOL valid;
};

// Note, 8 byte alignment when possible
struct SHIP {
	double dPosX, dPosY;             // ship x and y position
	double dBulletPosX, dBulletPosY; // bullet x and y position
	DWORD dwBulletFrame;             // bullet frame
	char cFrame;                     // current ship frame
	BYTE byType;                     // ship type
	BOOL bEnable;                    // is this ship active?
	BOOL bBulletEnable;              // Is there an active bullet?

	// ship x and y velocity (pixels/millisecond)
	double dVelX, dVelY;
	// bullet x and y velocity (pixels/millisecond)
	double dBulletVelX, dBulletVelY;

	// current score
	DWORD dwScore;
	// most recent time stamp
	DWORD dwLastTick;
	// flag used to synchronize ship explosions
	BOOL bIgnore;
	// enable time-out
	int iCountDown;
	// number of frames since beginning of time
	DWORD dwFrameCount;

	// DSound members
	// the keyboard state
	DWORD dwKeys;
	// These BOOLs keep track of the ship's
	BOOL bEngineRunning;
	// last condition so we can play sounds when they change
	BOOL bMoving;
	BOOL bBounced;
	BOOL bBlockHit;
	BOOL bDeath;
	BOOL bFiring;
};

struct BLOCKS {
	BYTE bits[30][5];
};

//-----------------------------------------------------------------------------
// communication packet structures
//-----------------------------------------------------------------------------

// message containing field layout, sent by host
#define MSG_HOST 0x11

// block hit message
#define MSG_BLOCKHIT 0x22

// ship hit message
#define MSG_SHIPHIT 0x33

// add block message
#define MSG_ADDBLOCK 0x44

// game keys message
#define MSG_CONTROL 0x55

// synchronization message containing the rendezvous location
#define MSG_SYNC 0x66

// Message packets, note data padding for alignment

struct GENERICMSG {
	BYTE byType;
};

struct OLDHOSTMSG {
	BYTE byType;
	BLOCKS Blocks; // 150 bytes
};

struct HOSTMSG {
	BYTE byType;
	BLOCKS Blocks;  // 150 bytes
	BYTE m_Padding; // Align to int
	int usedShipTypes[NUM_SHIP_TYPES];
};

struct BLOCKHITMSG {
	BYTE byType;
	BYTE byRow;
	BYTE byCol;
	BYTE byMask;
};

struct SHIPHITMSG {
	BYTE byType;
	BYTE m_Padding[3]; // Align to int
	DPID Id;
};

struct ADDBLOCKMSG {
	BYTE byType;
	BYTE byX;
	BYTE byY;
};

struct CONTROLMSG {
	BYTE byType;
	BYTE byState;
};

struct SYNCMSG {
	BYTE byType;
	// this is needed only when sends are unreliable
	BYTE byShipType;
	char cFrame;

	BYTE m_Padding[5]; // Align to double
	double dPosX;
	double dPosY;
};

// ship framents
extern FRAG g_Frags[64];

// field layout
extern BLOCKS g_Blocks;

// buffer to store received messages
extern VOID* g_pvReceiveBuffer;

// size of buffer
extern DWORD g_dwReceiveBufferSize;

// do we need to do host initializaton
extern BOOL g_bHaveHostInit;

// current state of the game
extern int g_nProgramState;

// used for fps calc
extern DWORD g_dwFrameCount;
extern DWORD g_dwFramesLast;
extern DWORD g_dwFrameTime;

// TRUE if player data needs to be updated
extern BOOL g_bUpdate;

// did we get disconnected ?
extern BOOL g_bSessionLost;

// message buffer
extern HOSTMSG g_HostMsg;

extern BLOCKHITMSG g_BlockHitMsg;
extern SHIPHITMSG g_ShipHitMsg;
extern ADDBLOCKMSG g_AddBlockMsg;
extern CONTROLMSG g_ControlMsg;
extern SYNCMSG g_SyncMsg;

extern SoundObject* g_pBulletFiringSound;
extern SoundObject* g_pShipExplodeSound;
extern SoundObject* g_pShipEngineSound;
extern SoundObject* g_pShipStartSound;
extern SoundObject* g_pShipStopSound;
extern SoundObject* g_pShipBounceSound;
extern SoundObject* g_pBlockExplodeSound;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

extern VOID InitMessageBuffers();
extern VOID LaunchGame();
extern VOID ExitGame();
extern HRESULT InitOurShip();
extern BOOL WINAPI SetPlayerLocalSoundDataCB(DPID dpId, DWORD dwPlayerType,
	LPCDPNAME pName, DWORD dwFlags, VOID* pContext);
extern HRESULT InitLocalSoundData();
extern VOID ReleasePlayerLocalSoundData(SHIP* pShip);
extern BOOL WINAPI ReleasePlayerLocalDataCB(DPID dpId, DWORD dwPlayerType,
	LPCDPNAME pName, DWORD dwFlags, VOID* pContext);
extern VOID ReleaseLocalData();
extern VOID ProcessUserInput(SHIP* pShip);
extern VOID UpdateState(SHIP* pShip, DWORD dwControls);
extern VOID SendSync(SHIP* pShip);
extern VOID UpdateDisplayStatus(SHIP* pShip);
extern VOID UpdatePosition(DPID dpId, SHIP* ship);
extern BOOL IsHit(int x, int y);
extern VOID InitField();
extern VOID AddBlock();
extern BOOL setBlock(int x, int y);
extern VOID AddFrag(SHIP* pShip, int offX, int offY);
extern VOID UpdateFragment(int i);
extern VOID DestroyShip(SHIP* pShip);
extern BOOL UpdateFrame();
extern VOID ProcessSoundFlags(SHIP* pShip);
extern BOOL WINAPI RenderPlayerCB(DPID dpId, DWORD dwPlayerType,
	LPCDPNAME pName, DWORD dwFlags, VOID* pContext);
extern BOOL DrawScreen();
extern VOID DisplayFrameRate();
extern BOOL DrawScore();
extern VOID DrawShip(SHIP* pShip);
extern VOID DrawBlock(int x, int y);
extern VOID DrawBullet(SHIP* pShip);
extern VOID DrawFragments();
extern HRESULT ReceiveMessages();
extern VOID DoSystemMessage(
	DPMSG_GENERIC* pMsg, DWORD dwMsgSize, DPID idFrom, DPID idTo);
extern VOID DoApplicationMessage(
	GENERICMSG* pMsg, DWORD dwMsgSize, DPID idFrom, DPID idTo);
extern VOID SendGameMessage(GENERICMSG* pMsg, DPID idTo);
extern VOID CleanupComm();
extern HRESULT InitializeGameSounds();
extern VOID CleanupGameSounds();
#endif
