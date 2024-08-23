//-----------------------------------------------------------------------------
// File: dsutil.cpp
//
// Desc: Routines for dealing with sounds from resources
//
// Copyright (C) 1995-1999 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifndef __DSUTIL_H__
#define __DSUTIL_H__

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

// Depends on mmsystem.h
#include <dsound.h>

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
extern LPDIRECTSOUND g_pDS;

//-----------------------------------------------------------------------------
// Helper routines
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_FillSoundBuffer(
	LPDIRECTSOUNDBUFFER pDSB, BYTE* pbWaveData, DWORD dwWaveSize);
extern HRESULT DSUtil_ParseWaveResource(VOID* pvRes,
	WAVEFORMATEX** ppWaveHeader, BYTE** ppbWaveData, DWORD* pdwWaveSize);

//-----------------------------------------------------------------------------
// Name: DSUtil_LoadSoundBuffer()
// Desc: Loads an IDirectSoundBuffer from a Win32 resource in the current
//       application.
//-----------------------------------------------------------------------------
extern LPDIRECTSOUNDBUFFER DSUtil_LoadSoundBuffer(
	LPDIRECTSOUND pDS, const char* strName);

//-----------------------------------------------------------------------------
// Name: DSUtil_ReloadSoundBuffer()
// Desc: Reloads an IDirectSoundBuffer from a Win32 resource in the current
//       application. normally used to handle a DSERR_BUFFERLOST error.
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_ReloadSoundBuffer(
	LPDIRECTSOUNDBUFFER pDSB, LPCTSTR strName);

//-----------------------------------------------------------------------------
// Name: DSUtil_GetWaveResource()
// Desc: Finds a WAV resource in a Win32 module.
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_GetWaveResource(HMODULE hModule, LPCTSTR strName,
	WAVEFORMATEX** ppWaveHeader, BYTE** ppbWaveData, DWORD* pdwWaveSize);

//-----------------------------------------------------------------------------
// Name: DSUtil_InitDirectSound()
// Desc:
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_InitDirectSound(HWND hWnd);

//-----------------------------------------------------------------------------
// Name: DSUtil_FreeDirectSound()
// Desc:
//-----------------------------------------------------------------------------
extern VOID DSUtil_FreeDirectSound();

//-----------------------------------------------------------------------------
// Name: struct SoundObject
// Desc: Used to manage individual sounds which need to be played multiple
//       times concurrently.  A SoundObject represents a queue of
//       IDirectSoundBuffer objects which all refer to the same buffer memory.
//-----------------------------------------------------------------------------
struct SoundObject {
	BYTE* pbWaveData;                 // Ptr into wave resource (for restore)
	DWORD cbWaveSize;                 // Size of wave data (for restore)
	DWORD dwNumBuffers;               // Number of sound buffers.
	DWORD dwCurrent;                  // Current sound buffer
	LPDIRECTSOUNDBUFFER* pdsbBuffers; // List of sound buffers
};

//-----------------------------------------------------------------------------
// Name: DSUtil_CreateSound()
// Desc: Loads a SoundObject from a Win32 resource in the current application.
//-----------------------------------------------------------------------------
extern SoundObject* DSUtil_CreateSound(LPCTSTR strName, DWORD dwNumBuffers);

//-----------------------------------------------------------------------------
// Name: DSUtil_DestroySound()
// Desc: Frees a SoundObject and releases all of its buffers.
//-----------------------------------------------------------------------------
extern VOID DSUtil_DestroySound(SoundObject* pSound);

//-----------------------------------------------------------------------------
// Name: DSUtil_PlayPannedSound()
// Desc: Play a sound, but first set the panning according to where the
//       object is on the screen. fScreenXPos is between -1.0f (left) and
//       1.0f (right).
//-----------------------------------------------------------------------------
extern VOID DSUtil_PlayPannedSound(SoundObject* pSound, FLOAT fScreenXPos);

//-----------------------------------------------------------------------------
// Name: DSUtil_PlaySound()
// Desc: Plays a buffer in a SoundObject.
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_PlaySound(SoundObject* pSound, DWORD dwPlayFlags);

//-----------------------------------------------------------------------------
// Name: DSUtil_StopSound()
// Desc: Stops one or more buffers in a SoundObject.
//-----------------------------------------------------------------------------
extern HRESULT DSUtil_StopSound(SoundObject* pSound);

//-----------------------------------------------------------------------------
// Name: DSUtil_GetFreeSoundBuffer()
// Desc: Returns one of the cloned buffers that is not currently playing
//-----------------------------------------------------------------------------
extern LPDIRECTSOUNDBUFFER DSUtil_GetFreeSoundBuffer(SoundObject* pSound);

#endif
