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
//=========================================================
// Colt SAA
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "hrp_weapon_colt.h"

LINK_ENTITY_TO_CLASS( weapon_colt, CColt );
LINK_ENTITY_TO_CLASS( weapon_357, CColt ); // Half-Life backward compatibility maps
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CColt ); // Half-Life backward compatibility maps
LINK_ENTITY_TO_CLASS( weapon_glock, CColt ); // Half-Life backward compatibility maps
LINK_ENTITY_TO_CLASS( weapon_python, CColt ); // Half-Life backward compatibility maps

enum colt_e {
	COLT_IDLE1 = 0,
	COLT_IDLE2,
	COLT_IDLE3,
	COLT_FIRE1,
	COLT_FIRE2,
	COLT_GANGSTA_START,
	COLT_GANGSTA_IDLE,
	COLT_GANGSTA_FIRE1,
	COLT_GANGSTA_FIRE2,
	COLT_GANGSTA_FIRE3,
	COLT_GANGSTA_RELOAD,
	COLT_GANGSTA_STOP,
	COLT_GANGSTA_HOLSTER,
	COLT_HOLSTER,
	COLT_DRAW,
	COLT_RELOAD
};

void CColt::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_colt" );
	m_iId = WEAPON_COLT;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_colt.mdl" );

	m_iDefaultAmmo = 6;
	FallInit();
}

void CColt::Precache( void )
{
	PRECACHE_MODEL( "models/p_colt.mdl" );
	PRECACHE_MODEL( "models/v_colt.mdl" );
	PRECACHE_MODEL( "models/w_colt.mdl" );

	PRECACHE_SOUND( "weapons/colt_cock.wav" );
	PRECACHE_SOUND( "weapons/colt_fire.wav" );

	m_usEvent = PRECACHE_EVENT( 1, "events/colt.sc" );
}

int CColt::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "45";
	p->iMaxAmmo1 = _45_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 6;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_COLT;
	p->iWeight = 15;
	return 1;
}

BOOL CColt::Deploy( void )
{
	m_iParaExtraStuff = COLT_GANGSTA_STATE_NO;
	return DefaultDeploy( "models/v_colt.mdl", "models/p_colt.mdl", COLT_DRAW, "onehanded", UseDecrement(), 0 );
}

void CColt::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	SendWeaponAnim( m_iParaExtraStuff != COLT_GANGSTA_STATE_NO ? COLT_GANGSTA_HOLSTER : COLT_HOLSTER, UseDecrement(), 0 );
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CColt::PrimaryAttack( void )
{
	if ( m_iParaExtraStuff == COLT_GANGSTA_STATE_LOOPING ) // Fire like a madman
		ColtFire( true );
	else
		ColtFire( false );
}

void CColt::SecondaryAttack( void )
{
	// Don't block Colt's function if we aren't in the appropriate mode
	if ( m_iParaExtraStuff == COLT_GANGSTA_STATE_STOPPING )
	{
		WeaponIdle();
		return;
	}

	if ( m_iParaExtraStuff == COLT_GANGSTA_STATE_NO ) // Not ready yet
	{
		SendWeaponAnim( COLT_GANGSTA_START, UseDecrement(), 0 );
		m_iParaExtraStuff = COLT_GANGSTA_STATE_LOOPING;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.26f;
	}
	else if ( m_iParaExtraStuff == COLT_GANGSTA_STATE_LOOPING ) // Fire like a madman
		ColtFire( true );
}

void CColt::ColtFire( bool bIsGangsta )
{
	// Don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	if ( m_iClip <= 0 )
	{
		if ( !m_fFireOnEmpty )
			Reload();
		else
		{
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/colt_cock.wav", 1.0f, ATTN_NORM );
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, bIsGangsta ? VECTOR_CONE_4DEGREES : VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_COLT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEvent, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	float flDelay;
	int iAnimation;
	if ( bIsGangsta )
	{
		switch ( RANDOM_LONG( 0, 2 ) )
		{
		case 0: iAnimation = COLT_GANGSTA_FIRE1; break;
		case 1: iAnimation = COLT_GANGSTA_FIRE2; break;
		case 2: iAnimation = COLT_GANGSTA_FIRE3; break;
		}
		flDelay = 0.5f;
		SendWeaponAnim( iAnimation, UseDecrement(), 0 );
		EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/colt_fire.wav", 1.0f, ATTN_NORM );
		m_pPlayer->pev->punchangle.y += 10.0f;
	}
	else
	{
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: iAnimation = COLT_FIRE1; break;
		case 1: iAnimation = COLT_FIRE2; break;
		}
		flDelay = 1.0f;
		SendWeaponAnim( iAnimation, UseDecrement(), 0 );
		EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/colt_fire.wav", 1.0f, ATTN_NORM );
		m_pPlayer->pev->punchangle.x -= 10.0f;
	}

	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); // HEV suit - indicate out of ammo condition

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flDelay;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flDelay + 0.5f;
}

void CColt::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip >= 6 )
		return;

	DefaultReload( 6, m_iParaExtraStuff != COLT_GANGSTA_STATE_NO ? COLT_GANGSTA_RELOAD : COLT_RELOAD, 3.5f, 0 );
}

void CColt::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_iParaExtraStuff == COLT_GANGSTA_STATE_LOOPING ) // Stop secondary attack
	{
		SendWeaponAnim( COLT_GANGSTA_STOP, UseDecrement(), 0 );
		m_iParaExtraStuff = COLT_GANGSTA_STATE_STOPPING;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.25f;
		return;
	}

	m_iParaExtraStuff = COLT_GANGSTA_STATE_NO;
	if ( m_iParaExtraStuff != COLT_GANGSTA_STATE_NO )
	{
		SendWeaponAnim( COLT_GANGSTA_IDLE, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
		return;
	}

	int iAnimation = COLT_IDLE1 + RANDOM_LONG( 0, 2 );
	SendWeaponAnim( iAnimation, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}

#ifndef CLIENT_DLL
TYPEDESCRIPTION CColt::m_SaveData[] =
{
	DEFINE_FIELD( CColt, m_iParaExtraStuff, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CColt, CBasePlayerWeapon );
#endif
