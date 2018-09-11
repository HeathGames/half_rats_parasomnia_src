/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Shepard's Debug weapon (aka Thompson 1928)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "soundent.h"
#include "skill.h"
#include "weapons.h"

enum debug_e
{
	DEBUG_IDLE = 0,
	DEBUG_DRAW,
	DEBUG_SHOOT1,
	DEBUG_SHOOT2,
	DEBUG_SHOOT3,
	DEBUG_RELOAD
};

LINK_ENTITY_TO_CLASS( weapon_debug, CDebug );

void CDebug::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_debug" );
	m_iId = WEAPON_DEBUG;
	Precache();
	SET_MODEL( ENT( pev ), "models/scissors.mdl" );
	m_iDefaultAmmo = 50;
	m_iMode = 0;
	FallInit();
}

void CDebug::Precache( void )
{
	PRECACHE_MODEL( "models/paper.mdl" );
	PRECACHE_MODEL( "models/rock.mdl" );
	PRECACHE_MODEL( "models/scissors.mdl" );

	m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

	PRECACHE_SOUND( "items/drunk.wav" );
	PRECACHE_SOUND( "items/drunk2.wav" );
	PRECACHE_SOUND( "items/drunk4.wav" );
	PRECACHE_SOUND( "items/drunk5.wav" );
	PRECACHE_SOUND( "ambience/paper.wav" );
	PRECACHE_SOUND( "weapons/colt_cock.wav" );
	PRECACHE_SOUND( "weapons/glauncher.wav" );

	m_usEvent = PRECACHE_EVENT( 1, "events/henry.sc" );
}

int CDebug::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "debug";
	p->iMaxAmmo1 = 50;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 50;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DEBUG;
	p->iWeight = 500;
	return 1;
}

int CDebug::AddToPlayer( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	if ( g_pGameRules->IsMultiplayer() )
		return FALSE;
#endif
#ifdef DEBUG
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
#else
	if ( pPlayer->m_iGameEnd == 1 && CVAR_GET_FLOAT( "sv_hrp_bonus_debug_weapon" ) > 0.0f && CBasePlayerWeapon::AddToPlayer( pPlayer ) )
#endif
		return TRUE;

	return FALSE;
}

BOOL CDebug::Deploy( void )
{
	return DefaultDeploy( "models/rock.mdl", "models/paper.mdl", DEBUG_DRAW, "mp5", UseDecrement(), 0 );
}

void CDebug::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

#ifndef CLIENT_DLL
extern int gmsgFMOD;
#endif

void CDebug::PrimaryAttack( void )
{
	if ( m_iMode == 1 ) // Music
	{
		// 2 "switch" are necessary otherwise GS will yell at you "I can't send a message while you are preparing another one"
#ifndef CLIENT_DLL
		MESSAGE_BEGIN( MSG_ALL, gmsgFMOD, NULL );
			int iMusic = RANDOM_LONG( 0, 8 );
			switch ( iMusic )
			{
			case 0: WRITE_STRING( "ambience/AgueAHerosBaptism.mp3" ); break;
			case 1: WRITE_STRING( "ambience/AgueDreamsPaintedScarlet.mp3" ); break;
			case 2: WRITE_STRING( "ambience/AgueInfluencesofAgenciesMalign.mp3" ); break;
			case 3: WRITE_STRING( "ambience/AgueMourningYear.mp3" ); break;
			case 4: WRITE_STRING( "ambience/AgueOccamsRazor.mp3" ); break;
			case 5: WRITE_STRING( "ambience/AgueOfDukesandDimwits.mp3" ); break;
			case 6: WRITE_STRING( "ambience/AgueSoMuchforSobriety.mp3" ); break;
			case 7: WRITE_STRING( "ambience/AgueSyphilis.mp3"); break;
			case 8: WRITE_STRING( "" ); break;
			}
			WRITE_COORD( 1.0f );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		switch ( iMusic )
		{
		case 0: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - A Hero\'s Baptism" ); break;
		case 1: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Dreams Painted Scarlet" ); break;
		case 2: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Influences of Agencies Malign" ); break;
		case 3: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Mourning Year" ); break;
		case 4: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Occam\'s Razor" ); break;
		case 5: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Of Dukes and Dimwits" ); break;
		case 6: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - So Much for Sobriety" ); break;
		case 7: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Ague - Syphilis" ); break;
		case 8: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Nothing" ); break;
		}
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}
	else if ( m_iMode == 3 ) // Just insult
	{
		float flTime;
		switch ( RANDOM_LONG( 0, 3 ) )
		{
		case 0:
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_VOICE, "items/drunk.wav", 1.0f, ATTN_NORM );
			flTime = 1.5f;
			break;
		case 1:
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_VOICE, "items/drunk2.wav", 1.0f, ATTN_NORM );
			flTime = 1.5f;
			break;
		case 2:
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_VOICE, "items/drunk4.wav", 1.0f, ATTN_NORM );
			flTime = 4.5f;
			break;
		case 3:
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_VOICE, "items/drunk5.wav", 1.0f, ATTN_NORM );
			flTime = 2.5f;
			break;
		}
		CSoundEnt::InsertSound( bits_SOUND_PLAYER, m_pPlayer->pev->origin, 512.0f, flTime );
		// Timings
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0f;
		return;
	}

	// You are not allowed to fire empty
	if ( m_iClip <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	// Weapon's volume and muzzleflash
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;

	// Remove a bullet
	m_iClip--;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 50 - m_iClip;

	// Third person anim and effects and firing sound
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	switch ( m_iMode )
	{
	case 2: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/glauncher.wav", 1.0f, ATTN_NORM ); break;
	default: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "ambience/paper.wav", 1.0f, ATTN_NORM );
	}

	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	if ( m_iMode == 2 ) // Shoot a grenade
	{
		CGrenade::ShootContact( m_pPlayer->pev, m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16.0f, gpGlobals->v_forward * 800.0f );

		// Punch and firing animation (depending on the mode)
		m_pPlayer->pev->punchangle.x -= 3.0f;
		SendWeaponAnim( DEBUG_SHOOT3, UseDecrement(), 0 );
	}
	else // Fire that bullet
	{
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecAim = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
		Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAim, VECTOR_CONE_2DEGREES, 8192, BULLET_PLAYER_HENRY, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

		// Call the event
		int iEventFlags;
#if defined( CLIENT_WEAPONS )
		iEventFlags = FEV_NOTHOST;
#else
		iEventFlags = 0;
#endif
		PLAYBACK_EVENT_FULL( iEventFlags, m_pPlayer->edict(), m_usEvent, 0.0f, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, WEAPON_DEBUG, 0, 0, 0 );

		// Punch and firing animation (depending on the mode)
		m_pPlayer->pev->punchangle.x -= 1.0f;
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: SendWeaponAnim( DEBUG_SHOOT1, UseDecrement(), 0 ); break;
		case 1: SendWeaponAnim( DEBUG_SHOOT2, UseDecrement(), 0 ); break;
		}
	}

	// Timings
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CDebug::SecondaryAttack( void )
{
	// Apply the timings
	if ( m_iMode == 3 )
		m_iMode = 0;
	else
		m_iMode++;

	switch ( m_iMode )
	{
	case 0: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Thompson 1928 (Full Auto) Mode" ); break;
	case 1: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Jukebox Mode" ); break;
	case 2: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "M203 Grenade Launcher Mode" ); break;
	case 3: ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "Half-Rats Insults Mode" ); break;
	}
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/colt_cock.wav", 1.0f, ATTN_NORM );
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
}

void CDebug::Reload( void )
{
	// No more ammo
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		return;

	// Reload and apply the timings
	if ( DefaultReload( 50, DEBUG_RELOAD, 3.75f, 0 ) )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.25f;
}

void CDebug::WeaponIdle( void )
{
	// Common to all firing weapons
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// Just play the idle animation
	SendWeaponAnim( DEBUG_IDLE, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}
