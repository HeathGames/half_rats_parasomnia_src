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
// Shotgun
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "hrp_weapon_shotgun.h"

extern int gmsgSetBody;
extern int gmsgWeapShot;

LINK_ENTITY_TO_CLASS( weapon_shotgun, CShotgun ); // Half-Life backward compatibility maps

enum shotgun_e {
	SHOTGUN_IDLE1 = 0,
	SHOTGUN_IDLE2,
	SHOTGUN_IDLE3,
	SHOTGUN_HOLSTER,
	SHOTGUN_DRAW,
	SHOTGUN_FIRE_RIGHT,
	SHOTGUN_FIRE_LEFT,
	SHOTGUN_FIRE_BOTH,
	SHOTGUN_RELOAD
};

void CShotgun::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_shotgun" );
	m_iId = WEAPON_SHOTGUN;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_shotgun.mdl" );

	m_bIsLeftBarrelEmpty = false;
	m_bIsRightBarrelEmpty = false;
	m_iParaExtraStuff = 0;
	m_iParaExtraStuff2 = 0;
	m_iDefaultAmmo = 2;
	FallInit();
}

void CShotgun::Precache( void )
{
	PRECACHE_MODEL( "models/p_shotgun.mdl" );
	PRECACHE_MODEL( "models/v_shotgun.mdl" );
	PRECACHE_MODEL( "models/w_shotgun.mdl" );

	PRECACHE_SOUND( "weapons/shotgun_cock.wav" );
	PRECACHE_SOUND( "weapons/shotgun_fire.wav" );

	m_usEvent = PRECACHE_EVENT( 1, "events/shotgun.sc" );
}

int CShotgun::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 2;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SHOTGUN;
	p->iWeight = 25;
	return 1;
}

BOOL CShotgun::Deploy( void )
{
	if ( DefaultDeploy( "models/v_shotgun.mdl", "models/p_shotgun.mdl", SHOTGUN_DRAW, "shotgun", UseDecrement(), m_iParaExtraStuff2 ) )
	{
		UpdateHUD();
		return true;
	}

	return false;
}

void CShotgun::Holster( int skiplocal )
{
#ifndef CLIENT_DLL
	if ( m_pPlayer && m_pPlayer->edict() )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgSetBody, NULL, m_pPlayer->edict() );
			WRITE_BYTE( 0 );
		MESSAGE_END();
	}
#endif
	m_fInReload = FALSE;
	m_fInSpecialReload = FALSE;
	SendWeaponAnim( SHOTGUN_HOLSTER, UseDecrement(), m_iParaExtraStuff2 );
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CShotgun::PrimaryAttack( void )
{
	// Don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 || m_iParaExtraStuff & 1 )
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
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/shotgun_cock.wav", 1.0f, ATTN_NORM );
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_bIsLeftBarrelEmpty = true;
	m_iParaExtraStuff = (m_bIsLeftBarrelEmpty ? 1 : 0) | (m_bIsRightBarrelEmpty ? 2 : 0);

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 8, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEvent, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	SendWeaponAnim( SHOTGUN_FIRE_LEFT, UseDecrement(), 0 );
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/shotgun_fire.wav", 1.0f, ATTN_NORM );
	m_pPlayer->pev->punchangle.x -= 5.0f;

	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); // HEV suit - indicate out of ammo condition

	UpdateHUD();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CShotgun::SecondaryAttack( void )
{
	// Don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 || m_iParaExtraStuff & 2 )
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	if ( m_iClip <= 0 )
	{
		if ( !m_fFireOnEmpty )
			Reload();
		else
		{
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/shotgun_cock.wav", 1.0f, ATTN_NORM );
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		}
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_bIsRightBarrelEmpty = true;
	m_iParaExtraStuff = (m_bIsLeftBarrelEmpty ? 1 : 0) | (m_bIsRightBarrelEmpty ? 2 : 0);

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 8, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEvent, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	m_pPlayer->pev->punchangle.x -= 5.0f;
	SendWeaponAnim( SHOTGUN_FIRE_RIGHT, UseDecrement(), m_iParaExtraStuff2 );
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/shotgun_fire.wav", 1.0f, ATTN_NORM );

	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); // HEV suit - indicate out of ammo condition

	UpdateHUD();
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CShotgun::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip >= 2 )
		return;

	if ( DefaultReload( 2, SHOTGUN_RELOAD, 3.75f, m_iParaExtraStuff2 ) )
	{
		UpdateHUD();
		m_fInSpecialReload = TRUE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.74f;
	}
}

void CShotgun::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_fInSpecialReload )
	{
		if ( m_iClip == 2 )
		{
			m_bIsLeftBarrelEmpty = m_bIsRightBarrelEmpty = false;
			m_iParaExtraStuff = 0;
		}
		else
		{
			m_bIsLeftBarrelEmpty = false;
			m_iParaExtraStuff = 1;
		}

		UpdateHUD();
		m_fInSpecialReload = FALSE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.01f;
	}

	SendWeaponAnim( SHOTGUN_IDLE1 + RANDOM_LONG( 0, 2 ), UseDecrement(), m_iParaExtraStuff2 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0f;
}

void CShotgun::UpdateHUD( void )
{
	if ( m_fInReload )
		m_iParaExtraStuff2 = 3;
	else
	{
		if ( !m_bIsLeftBarrelEmpty && !m_bIsRightBarrelEmpty )
			m_iParaExtraStuff2 = 3;
		else if ( m_bIsLeftBarrelEmpty && !m_bIsRightBarrelEmpty )
			m_iParaExtraStuff2 = 2;
		else if ( !m_bIsLeftBarrelEmpty && m_bIsRightBarrelEmpty )
			m_iParaExtraStuff2 = 1;
		else
			m_iParaExtraStuff2 = 0;
	}

	pev->body = m_iParaExtraStuff2;
#ifndef CLIENT_DLL
	if ( m_pPlayer && m_pPlayer->edict() )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgSetBody, NULL, m_pPlayer->edict() );
			WRITE_BYTE( m_iParaExtraStuff2 );
		MESSAGE_END();

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapShot, NULL, m_pPlayer->edict() );
			WRITE_BYTE( m_bIsLeftBarrelEmpty );
			WRITE_BYTE( m_bIsRightBarrelEmpty );
		MESSAGE_END();
	}
#endif
}

#ifndef CLIENT_DLL
TYPEDESCRIPTION CShotgun::m_SaveData[] =
{
	DEFINE_FIELD( CShotgun, m_bIsLeftBarrelEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( CShotgun, m_bIsRightBarrelEmpty, FIELD_BOOLEAN ),
	DEFINE_FIELD( CShotgun, m_iParaExtraStuff, FIELD_INTEGER ),
	DEFINE_FIELD( CShotgun, m_iParaExtraStuff2, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CShotgun, CBasePlayerWeapon );
#endif
