/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

/*
======================================================================
( == [ GoldAudio engine ] == )
======================================================================

Programmed by
Joël "Shepard62700FR, Shepard62FR" Troch

With help from
Alex "HAWK0044" Bakanov
Claudio "SysOp" Ficara

----------------------------------------------------------------------

The GoldAudio engine is an implementation of FMOD Studio Low Level API
within the GoldSource engine by Valve Software.

This implementation has been mainly designed for the following Half-Life
total conversions: ARRANGEMENT and Revenge of Love.

Joël "Shepard62700FR, Shepard62FR" Troch allow his work to be used within
Half-Rats: Parasomnia but only the music part (and the required system
parts) of the engine.

You are free to use "GoldAudio" in your projects as long as :
1) This notice is included without any modification in both the main header
files and their sources.
2) Credits to the people above are given (in other words, no stealing,
don't claim it's your work).
3) You won't do anything commercial with it (selling, trading...)
4) Most importantly: you agree to FMOD Studio's Non-Commercial/Educational
licenses which are available here: http://www.fmod.org/sales/

If you don't agree with a single rule above, you may remove this
implementation from your project.

FMOD Studio, Copyright (C) Firelight Technologies Pty, Ltd., 2012-2016

======================================================================
( == [ GoldAudio engine ] == )
======================================================================
*/

#ifndef CFMODENGINE_H
#define CFMODENGINE_H

#include <FMOD/fmod.h>
#include <FMOD/fmod_errors.h>
#include <string>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#define FOLDER_ADDON			0
#define FOLDER_HD				1

class CFMODEngine
{
public:
	void Initialize( void );
	void InitializeHUD( void );
	void PlayMusic( const char *_cMusic, const float _flVol, const bool _bLooped );
	void Shutdown( void );
	void Update( void );

	int m_iSystemState;
private:
	bool InitializeCheckFMODVersion( void );
	bool InitializeGetFunctionsPointers( void );
	bool InitializeLoadLibrary( void );

	bool m_bFadeOut;

	char m_cPathMusic[64];

	cvar_t *m_cvVolumeMusic;

	float m_flVolumeMultiplier;
	float m_flVolumeToSet;

	FMOD_CHANNEL *m_pChannelMusic;

	FMOD_RESULT m_pResult;

#ifdef WIN32
	FMOD_RESULT (__stdcall * m_pFMODChannelIsPlaying)	( FMOD_CHANNEL *channel, FMOD_BOOL *isplaying );
	FMOD_RESULT (__stdcall * m_pFMODChannelSetPaused)	( FMOD_CHANNEL *channel, FMOD_BOOL paused );
	FMOD_RESULT (__stdcall * m_pFMODChannelSetPosition)	( FMOD_CHANNEL *channel, unsigned int position, FMOD_TIMEUNIT postype );
	FMOD_RESULT (__stdcall * m_pFMODChannelSetVolume)	( FMOD_CHANNEL *channel, float volume );
	FMOD_RESULT (__stdcall * m_pFMODSoundRelease)		( FMOD_SOUND *sound );
	FMOD_RESULT (__stdcall * m_pFMODSystemCreate)		( FMOD_SYSTEM **system );
	FMOD_RESULT (__stdcall * m_pFMODSystemCreateSound)	( FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound );
	FMOD_RESULT (__stdcall * m_pFMODSystemGetVersion)	( FMOD_SYSTEM *system, unsigned int *version );
	FMOD_RESULT (__stdcall * m_pFMODSystemInit)			( FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata );
	FMOD_RESULT (__stdcall * m_pFMODSystemPlaySound)	( FMOD_SYSTEM *system, FMOD_SOUND *sound, FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL paused, FMOD_CHANNEL **channel );
	FMOD_RESULT (__stdcall * m_pFMODSystemRelease)		( FMOD_SYSTEM *system );
	FMOD_RESULT (__stdcall * m_pFMODSystemUpdate)		( FMOD_SYSTEM *system );
#else
	FMOD_RESULT (*m_pFMODChannelIsPlaying)( FMOD_CHANNEL *channel, FMOD_BOOL *isplaying );
	FMOD_RESULT (*m_pFMODChannelSetPaused)( FMOD_CHANNEL *channel, FMOD_BOOL paused );
	FMOD_RESULT (*m_pFMODChannelSetPosition)( FMOD_CHANNEL *channel, unsigned int position, FMOD_TIMEUNIT postype );
	FMOD_RESULT (*m_pFMODChannelSetVolume)( FMOD_CHANNEL *channel, float volume );
	FMOD_RESULT (*m_pFMODSoundRelease)( FMOD_SOUND *sound );
	FMOD_RESULT (*m_pFMODSystemCreate)( FMOD_SYSTEM **system );
	FMOD_RESULT (*m_pFMODSystemCreateSound)( FMOD_SYSTEM *system, const char *name_or_data, FMOD_MODE mode, FMOD_CREATESOUNDEXINFO *exinfo, FMOD_SOUND **sound );
	FMOD_RESULT (*m_pFMODSystemGetVersion)( FMOD_SYSTEM *system, unsigned int *version );
	FMOD_RESULT (*m_pFMODSystemInit)( FMOD_SYSTEM *system, int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata );
	FMOD_RESULT (*m_pFMODSystemPlaySound)( FMOD_SYSTEM *system, FMOD_SOUND *sound, FMOD_CHANNELGROUP *channelgroup, FMOD_BOOL paused, FMOD_CHANNEL **channel );
	FMOD_RESULT (*m_pFMODSystemRelease)( FMOD_SYSTEM *system );
	FMOD_RESULT (*m_pFMODSystemUpdate)( FMOD_SYSTEM *system );
#endif

	FMOD_SOUND *m_pSoundMusic;

	FMOD_SYSTEM *m_pSystem;
#ifdef WIN32
	HINSTANCE m_pFMODLib;
#else
	void *m_pFMODLib;
#endif
};

extern CFMODEngine gFMOD;

#endif // CFMODENGINE_H
