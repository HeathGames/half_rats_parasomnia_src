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
// Henry rifle
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "hrp_weapon_henry.h"

extern int gmsgHenSight;

LINK_ENTITY_TO_CLASS( weapon_henry, CHenry );
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CHenry ); // Half-Life backward compatibility maps
LINK_ENTITY_TO_CLASS( weapon_mp5, CHenry ); // Half-Life backward compatibility maps

enum henry_e {
	HENRY_IDLE1 = 0,
	HENRY_IDLE2,
	HENRY_DRAW,
	HENRY_RELOAD,
	HENRY_RELOAD_EMPTY,
	HENRY_FIRE,
	HENRY_IRON_START,
	HENRY_IRON_IDLE,
	HENRY_IRON_FIRE,
	HENRY_IRON_RELOAD,
	HENRY_IRON_RELOAD_EMPTY,
	HENRY_IRON_STOP,
	HENRY_HOLSTER
};

void CHenry::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_henry" );
	m_iId = WEAPON_HENRY;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_henry.mdl" );

	m_iDefaultAmmo = 15;
	FallInit();
}

void CHenry::Precache( void )
{
	PRECACHE_MODEL( "models/p_henry.mdl" );
	PRECACHE_MODEL( "models/v_henry.mdl" );
	PRECACHE_MODEL( "models/w_henry.mdl" );

	PRECACHE_SOUND( "weapons/henry_cock.wav" );
	PRECACHE_SOUND( "weapons/henry_fire.wav" );

	m_usEvent = PRECACHE_EVENT( 1, "events/henry.sc" );
}

int CHenry::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "45";
	p->iMaxAmmo1 = _45_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 15;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_HENRY;
	p->iWeight = 20;
	return 1;
}

BOOL CHenry::Deploy( void )
{
	m_iParaExtraStuff = 0;
	return DefaultDeploy( "models/v_henry.mdl", "models/p_henry.mdl", HENRY_DRAW, "mp5", UseDecrement(), 0 );
}

void CHenry::Holster( int skiplocal )
{
#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgHenSight, NULL, m_pPlayer->edict() );
		WRITE_BYTE( 0 );
	MESSAGE_END();
#endif
	m_pPlayer->m_iClientHenryIronsight = 0;
	m_pPlayer->SetSpeed( PLAYER_SPEED_NORMAL );
	m_fInReload = FALSE;
	SendWeaponAnim( HENRY_HOLSTER, UseDecrement(), 0 );
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CHenry::PrimaryAttack( void )
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
			EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/henry_cock.wav", 1.0f, ATTN_NORM );
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
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, m_iParaExtraStuff ? VECTOR_CONE_PERFECT : VECTOR_CONE_4DEGREES, 8192, BULLET_PLAYER_HENRY, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEvent, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	SendWeaponAnim( m_iParaExtraStuff ? HENRY_IRON_FIRE : HENRY_FIRE, UseDecrement(), 0 );
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/henry_fire.wav", 1.0f, ATTN_NORM );
	m_pPlayer->pev->punchangle.x -= 5.0f;

	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); // HEV suit - indicate out of ammo condition

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7f;
}

void CHenry::SecondaryAttack( void )
{
	SendWeaponAnim( m_iParaExtraStuff ? HENRY_IRON_STOP : HENRY_IRON_START, UseDecrement(), 0 );
	m_iParaExtraStuff = m_iParaExtraStuff == 1 ? 0 : 1;
#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgHenSight, NULL, m_pPlayer->edict() );
		WRITE_BYTE( m_iParaExtraStuff );
	MESSAGE_END();
#endif
	m_pPlayer->m_iClientHenryIronsight = m_iParaExtraStuff;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.525f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.525f;
}

void CHenry::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip >= 15 )
		return;

	if ( m_iClip == 0 )
		DefaultReload( 15, m_iParaExtraStuff ? HENRY_IRON_RELOAD_EMPTY : HENRY_RELOAD_EMPTY, 3.5f, 0 );
	else
		DefaultReload( 15, m_iParaExtraStuff ? HENRY_IRON_RELOAD : HENRY_RELOAD, 3.25f, 0 );
}

void CHenry::WeaponIdle( void )
{
	// Set the player's speed according to the iron state
	m_pPlayer->SetSpeed( m_iParaExtraStuff ? PLAYER_SPEED_SABER : PLAYER_SPEED_NORMAL );
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnimation = HENRY_IRON_IDLE;
	if ( !m_iParaExtraStuff )
		iAnimation = HENRY_IDLE1 + RANDOM_LONG( 0, 1 );

	SendWeaponAnim( iAnimation, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}

#ifndef CLIENT_DLL
TYPEDESCRIPTION CHenry::m_SaveData[] =
{
	DEFINE_FIELD( CHenry, m_iParaExtraStuff, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CHenry, CBasePlayerWeapon );
#endif
