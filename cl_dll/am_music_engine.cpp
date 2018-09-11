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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "am_music_engine.h"

// Returning address of local variable or temporary
#pragma warning(disable: 4172)

CFMODEngine gFMOD;

// HUD message to play music
int __MsgFunc_FMOD( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *cSound = READ_STRING();
	float flVol = READ_COORD();
	int iLooped = READ_BYTE();
	gFMOD.PlayMusic( cSound, flVol, iLooped == 0 ? false : true );
	return 1;
}

// Initialize the engine
void CFMODEngine::Initialize( void )
{
	if ( m_iSystemState > 0 )
		return;

	if ( !InitializeLoadLibrary() )
	{
		m_iSystemState = 2;
		return;
	}

	if ( !InitializeGetFunctionsPointers() )
	{
		m_iSystemState = 2;
		return;
	}

	m_pResult = m_pFMODSystemCreate( &m_pSystem );
	if ( m_pResult != FMOD_OK )
	{
		gEngfuncs.Con_Printf( "[FMOD] Failed to create the FMOD\'s system! (%s)\n", FMOD_ErrorString( m_pResult ) );
		m_iSystemState = 2;
		return;
	}

	if ( !InitializeCheckFMODVersion() )
	{
		m_iSystemState = 2;
		return;
	}

	m_pResult = m_pFMODSystemInit( m_pSystem, 1, FMOD_INIT_THREAD_UNSAFE, 0 );
	if ( m_pResult != FMOD_OK )
	{
		gEngfuncs.Con_Printf( "[FMOD] Failed to init the system! (%s)\n", FMOD_ErrorString( m_pResult ) );
		m_iSystemState = 2;
		return;
	}

#ifdef HRP_XASH
	m_cvVolumeMusic = gEngfuncs.pfnGetCvarPointer( "musicvolume" );
#else
	m_cvVolumeMusic = gEngfuncs.pfnGetCvarPointer( "MP3Volume" );
#endif
	m_flVolumeMultiplier = 1.0f;
	m_bFadeOut = false;
	m_iSystemState = 1;
}

// Initialize the CVARs
void CFMODEngine::InitializeHUD( void )
{
	HOOK_MESSAGE( FMOD );
}

// Play a music
void CFMODEngine::PlayMusic( const char *_cMusic, const float _flVol, const bool _bLooped )
{
	if ( m_iSystemState != 1 )
		return;

	if ( !strcmp( _cMusic, "" ) )
	{
		m_bFadeOut = true;
		return;
	}
	m_bFadeOut = false;

	char cFullPath[64];
	sprintf( cFullPath, "%s/sound/%s", gEngfuncs.pfnGetGameDirectory(), _cMusic );
#ifdef WIN32
	if ( access( cFullPath, 4 ) != 0 )
#else
	if ( access( cFullPath, R_OK ) != 0 )
#endif
	{
		gEngfuncs.Con_Printf( "[FMOD] File \"%s\" not found or not readable\n!", _cMusic );
		return;
	}

	if ( m_pChannelMusic && m_pSoundMusic )
	{
		if ( !strcmp( _cMusic, m_cPathMusic ) )
		{
			FMOD_BOOL bIsPlaying;
			m_pResult = m_pFMODChannelIsPlaying( m_pChannelMusic, &bIsPlaying );
			if ( m_pResult != FMOD_OK && m_pResult != FMOD_ERR_INVALID_HANDLE )
			{
				gEngfuncs.Con_Printf( "[FMOD] Failed to get music state! (%s)\n", FMOD_ErrorString( m_pResult ) );
				return;
			}
			if ( bIsPlaying )
			{
				m_pResult = m_pFMODChannelSetPosition( m_pChannelMusic, 0, FMOD_TIMEUNIT_MS );
				if ( m_pResult != FMOD_OK )
				{
					gEngfuncs.Con_Printf( "[FMOD] Failed to rewind music! (%s)\n", FMOD_ErrorString( m_pResult ) );
					return;
				}
				return;
			}
		}
		else
		{
			m_pResult = m_pFMODSoundRelease( m_pSoundMusic );
			if ( m_pResult != FMOD_OK )
				gEngfuncs.Con_Printf( "[FMOD] Failed to release the music! (%s)\n", FMOD_ErrorString( m_pResult ) );

			if ( m_pChannelMusic )
				m_pChannelMusic = NULL;
		}
	}

	FMOD_MODE pMode = _bLooped ? FMOD_LOOP_NORMAL | FMOD_CREATECOMPRESSEDSAMPLE : FMOD_LOOP_OFF | FMOD_CREATECOMPRESSEDSAMPLE;
	m_pResult = m_pFMODSystemCreateSound( m_pSystem, cFullPath, pMode, NULL, &m_pSoundMusic );
	if ( m_pResult != FMOD_OK )
	{
		gEngfuncs.Con_Printf( "[FMOD] Failed to create music! (%s)\n", FMOD_ErrorString( m_pResult ) );
		return;
	}

	m_pResult = m_pFMODSystemPlaySound( m_pSystem, m_pSoundMusic, NULL, 1, &m_pChannelMusic );
	if ( m_pResult != FMOD_OK )
	{
		gEngfuncs.Con_Printf( "[FMOD] Failed to play music! (%s)\n", FMOD_ErrorString( m_pResult ) );
		return;
	}

	m_flVolumeToSet = m_cvVolumeMusic->value * _flVol;
	m_pResult = m_pFMODChannelSetVolume( m_pChannelMusic, m_flVolumeToSet );
	m_flVolumeMultiplier = _flVol;
	if ( m_pResult != FMOD_OK )
		gEngfuncs.Con_Printf( "[FMOD] Failed to set music volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

	m_pResult = m_pFMODChannelSetPaused( m_pChannelMusic, 0 );
	if ( m_pResult != FMOD_OK )
		gEngfuncs.Con_Printf( "[FMOD] Failed to unpause music! (%s)\n", FMOD_ErrorString( m_pResult ) );

	strcpy( m_cPathMusic, _cMusic );
}

// Shutdown the system
void CFMODEngine::Shutdown( void )
{
	if ( m_pSoundMusic )
	{
		m_pResult = m_pFMODSoundRelease( m_pSoundMusic );
		if ( m_pResult != FMOD_OK )
			gEngfuncs.Con_Printf( "[FMOD] Failed to release music! (%s)\n", FMOD_ErrorString( m_pResult ) );

		m_pSoundMusic = NULL;
	}

	if ( m_pChannelMusic )
		m_pChannelMusic = NULL;

	if ( m_pSystem )
	{
		m_pResult = m_pFMODSystemRelease( m_pSystem );
		if ( m_pResult != FMOD_OK )
			gEngfuncs.Con_Printf( "[FMOD] Failed to release the system! (%s)\n", FMOD_ErrorString( m_pResult ) );

		m_pSystem = NULL;
	}

	m_pFMODChannelIsPlaying = NULL;
	m_pFMODChannelSetPaused = NULL;
	m_pFMODChannelSetPosition = NULL;
	m_pFMODChannelSetVolume = NULL;
	m_pFMODSoundRelease = NULL;
	m_pFMODSystemCreate = NULL;
	m_pFMODSystemCreateSound = NULL;
	m_pFMODSystemGetVersion = NULL;
	m_pFMODSystemInit = NULL;
	m_pFMODSystemPlaySound = NULL;
	m_pFMODSystemRelease = NULL;
	m_pFMODSystemUpdate = NULL;
	if ( m_pFMODLib )
#ifdef WIN32
		FreeLibrary( m_pFMODLib );
#else
		dlclose( m_pFMODLib );
#endif

	m_iSystemState = 0;
}

// Update the system
void CFMODEngine::Update( void )
{
	if ( m_iSystemState != 1 )
		return;

	m_pResult = m_pFMODSystemUpdate( m_pSystem );
	if ( m_pResult != FMOD_OK )
		gEngfuncs.Con_Printf( "[FMOD] Failed to update system! (%s)\n", FMOD_ErrorString( m_pResult ) );

	if ( m_pSoundMusic )
	{
		FMOD_BOOL bIsPlaying = 0;
		m_pResult = m_pFMODChannelIsPlaying( m_pChannelMusic, &bIsPlaying );
		if ( m_pResult != FMOD_OK && m_pResult != FMOD_ERR_INVALID_HANDLE )
		{
			gEngfuncs.Con_Printf( "[FMOD] Failed to get music playing status! (%s)\n", FMOD_ErrorString( m_pResult ) );
			return;
		}

		if ( m_bFadeOut && m_flVolumeToSet > 0.0f )
			m_flVolumeToSet -= 0.005f;
		else
			m_flVolumeToSet = m_cvVolumeMusic->value * m_flVolumeMultiplier;

		if ( m_flVolumeToSet < 0.0f )
			m_flVolumeToSet = 0.0f;

		m_pResult = m_pFMODChannelSetVolume( m_pChannelMusic, m_flVolumeToSet );
		if ( m_pResult != FMOD_OK && m_pResult != FMOD_ERR_INVALID_HANDLE )
			gEngfuncs.Con_Printf( "[FMOD] Failed to set music volume! (%s)\n", FMOD_ErrorString( m_pResult ) );

		if ( m_bFadeOut && m_flVolumeToSet == 0.0f )
		{
			m_pResult = m_pFMODSoundRelease( m_pSoundMusic );
			if ( m_pResult != FMOD_OK )
				gEngfuncs.Con_Printf( "[FMOD] Failed to release music! (%s)\n", FMOD_ErrorString( m_pResult ) );

			m_pSoundMusic = NULL;
			if ( m_pChannelMusic )
				m_pChannelMusic = NULL;

			m_bFadeOut = false;
		}
	}
}

// Check which version of FMOD the user is running and the one used for compiling
bool CFMODEngine::InitializeCheckFMODVersion( void )
{
	unsigned int uiUserVersion = 0;
	m_pResult = m_pFMODSystemGetVersion( m_pSystem, &uiUserVersion );
	if ( m_pResult != FMOD_OK )
	{
		gEngfuncs.Con_Printf( "[FMOD] Failed to check FMOD\'s version! (%s)\n", FMOD_ErrorString( m_pResult ) );
		return false;
	}
	gEngfuncs.Con_Printf( "[FMOD] Using version %u (built with %u)\n", uiUserVersion, FMOD_VERSION );
	return true;
}

// Hook the engine with FMOD
bool CFMODEngine::InitializeGetFunctionsPointers( void )
{
#ifdef WIN32
	(FARPROC &)m_pFMODChannelIsPlaying = GetProcAddress( m_pFMODLib, "_FMOD5_Channel_IsPlaying@8" );
	(FARPROC &)m_pFMODChannelSetPaused = GetProcAddress( m_pFMODLib, "_FMOD5_Channel_SetPaused@8" );
	(FARPROC &)m_pFMODChannelSetPosition = GetProcAddress( m_pFMODLib, "_FMOD5_Channel_SetPosition@12" );
	(FARPROC &)m_pFMODChannelSetVolume = GetProcAddress( m_pFMODLib, "_FMOD5_Channel_SetVolume@8" );
	(FARPROC &)m_pFMODSoundRelease = GetProcAddress( m_pFMODLib, "_FMOD5_Sound_Release@4" );
	(FARPROC &)m_pFMODSystemCreate = GetProcAddress( m_pFMODLib, "_FMOD5_System_Create@4" );
	(FARPROC &)m_pFMODSystemCreateSound = GetProcAddress( m_pFMODLib, "_FMOD5_System_CreateSound@20" );
	(FARPROC &)m_pFMODSystemGetVersion = GetProcAddress( m_pFMODLib, "_FMOD5_System_GetVersion@8" );
	(FARPROC &)m_pFMODSystemInit = GetProcAddress( m_pFMODLib, "_FMOD5_System_Init@16" );
	(FARPROC &)m_pFMODSystemPlaySound = GetProcAddress( m_pFMODLib, "_FMOD5_System_PlaySound@20" );
	(FARPROC &)m_pFMODSystemRelease = GetProcAddress( m_pFMODLib, "_FMOD5_System_Release@4" );
	(FARPROC &)m_pFMODSystemUpdate = GetProcAddress( m_pFMODLib, "_FMOD5_System_Update@4" );
#else
	m_pFMODChannelIsPlaying = (FMOD_RESULT (*)(FMOD_CHANNEL *, FMOD_BOOL *))dlsym( m_pFMODLib, "FMOD5_Channel_IsPlaying" );
	m_pFMODChannelSetPaused = (FMOD_RESULT (*)(FMOD_CHANNEL *, FMOD_BOOL))dlsym( m_pFMODLib, "FMOD5_Channel_SetPaused" );
	m_pFMODChannelSetPosition = (FMOD_RESULT (*)(FMOD_CHANNEL *, unsigned int, FMOD_TIMEUNIT))dlsym( m_pFMODLib, "FMOD5_Channel_SetPosition" );
	m_pFMODChannelSetVolume = (FMOD_RESULT (*)(FMOD_CHANNEL *, float))dlsym( m_pFMODLib, "FMOD5_Channel_SetVolume" );
	m_pFMODSoundRelease = (FMOD_RESULT (*)(FMOD_SOUND *))dlsym( m_pFMODLib, "FMOD5_Sound_Release" );
	m_pFMODSystemCreate = (FMOD_RESULT (*)(FMOD_SYSTEM **))dlsym( m_pFMODLib, "FMOD5_System_Create" );
	m_pFMODSystemCreateSound = (FMOD_RESULT (*)(FMOD_SYSTEM *, const char *, FMOD_MODE, FMOD_CREATESOUNDEXINFO *, FMOD_SOUND **))dlsym( m_pFMODLib, "FMOD5_System_CreateSound" );
	m_pFMODSystemGetVersion = (FMOD_RESULT (*)(FMOD_SYSTEM *, unsigned int *))dlsym( m_pFMODLib, "FMOD5_System_GetVersion" );
	m_pFMODSystemInit = (FMOD_RESULT (*)(FMOD_SYSTEM *, int, FMOD_INITFLAGS, void *))dlsym( m_pFMODLib, "FMOD5_System_Init" );
	m_pFMODSystemPlaySound = (FMOD_RESULT (*)(FMOD_SYSTEM *, FMOD_SOUND *, FMOD_CHANNELGROUP *, FMOD_BOOL, FMOD_CHANNEL **))dlsym( m_pFMODLib, "FMOD5_System_PlaySound" );
	m_pFMODSystemRelease = (FMOD_RESULT (*)(FMOD_SYSTEM *))dlsym( m_pFMODLib, "FMOD5_System_Release" );
	m_pFMODSystemUpdate = (FMOD_RESULT (*)(FMOD_SYSTEM *))dlsym( m_pFMODLib, "FMOD5_System_Update" );
#endif
	if ( m_pFMODChannelIsPlaying && m_pFMODChannelSetPaused && m_pFMODChannelSetPosition && m_pFMODChannelSetVolume && m_pFMODSoundRelease &&
		m_pFMODSystemCreate && m_pFMODSystemCreateSound && m_pFMODSystemGetVersion && m_pFMODSystemInit && m_pFMODSystemPlaySound &&
		m_pFMODSystemRelease && m_pFMODSystemUpdate )
		return true;

#ifdef WIN32
	gEngfuncs.Con_Printf( "[FMOD] Failed to load FMOD functions! (%d)\n", GetLastError() );
#else
	gEngfuncs.Con_Printf( "[FMOD] Failed to load FMOD functions! (%s)\n", dlerror() );
#endif
	return false;
}

// Load the FMOD library
bool CFMODEngine::InitializeLoadLibrary( void )
{
	// Get the path to the library
	char cFMODLib[256];
#ifdef WIN32
	sprintf( cFMODLib, "%s\\cl_dlls\\fmod.dll", gEngfuncs.pfnGetGameDirectory() );
	m_pFMODLib = LoadLibrary( cFMODLib );
#else
#ifdef __linux__
	sprintf( cFMODLib, "%s/cl_dlls/libfmod.so", gEngfuncs.pfnGetGameDirectory() );
#else
	sprintf( cFMODLib, "%s/cl_dlls/libfmod.dylib", gEngfuncs.pfnGetGameDirectory() );
#endif
	m_pFMODLib = dlopen( cFMODLib, RTLD_LAZY );
#endif

	if ( !m_pFMODLib )
	{
#ifdef WIN32
		gEngfuncs.Con_Printf( "[FMOD] Failed to load FMOD library! (%d)\n", GetLastError() );
#else
		gEngfuncs.Con_Printf( "[FMOD] Failed to load FMOD library! (%s)", dlerror() );
#endif
		return false;
	}
	return true;
}

