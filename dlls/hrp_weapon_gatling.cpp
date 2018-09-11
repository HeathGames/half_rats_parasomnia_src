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
// Gatling
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "hrp_weapon_gatling.h"

LINK_ENTITY_TO_CLASS( weapon_gatling, CGatling );
LINK_ENTITY_TO_CLASS( weapon_rpg, CGatling ) // Backward compatibility with Half-Life maps
LINK_ENTITY_TO_CLASS( ammo_rpgclip, CGatling ) // Backward compatibility with Half-Life maps

enum gatling_e {
	GATLING_IDLE = 0,
	GATLING_DRAW,
	GATLING_DRAW_MOUNT,
	GATLING_FIRE,
	GATLING_HOLSTER
};

void CGatling::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_gatling" );
	m_iId = WEAPON_GATLING;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_gatling.mdl" );

	m_iDefaultAmmo = 150;
	FallInit();
}

void CGatling::Precache( void )
{
	PRECACHE_MODEL( "models/p_gatling.mdl" );
	PRECACHE_MODEL( "models/v_gatling.mdl" );
	PRECACHE_MODEL( "models/w_gatling.mdl" );

	m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

	PRECACHE_SOUND( "weapons/gatling_cock.wav" );
	PRECACHE_SOUND( "weapons/gatling_fire1.wav" );
	PRECACHE_SOUND( "weapons/gatling_fire2.wav" );
	PRECACHE_SOUND( "weapons/gatling_fire3.wav" );
	PRECACHE_SOUND( "weapons/gatling_fire4.wav" );

	m_usEvent = PRECACHE_EVENT( 1, "events/gatling.sc" );
}

int CGatling::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "gatling";
	p->iMaxAmmo1 = 150;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_EXHAUSTIBLE | ITEM_FLAG_NOAUTOSWITCHEMPTY;;
	p->iId = m_iId = WEAPON_GATLING;
	p->iWeight = 100;
	return 1;
}

BOOL CGatling::Deploy( void )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_GATLING );
	m_pPlayer->pev->gravity = 4.0f;
	return DefaultDeploy( "models/v_gatling.mdl", "models/p_gatling.mdl", GATLING_DRAW, "mp5", UseDecrement(), 0 );
}

void CGatling::Holster( int skiplocal )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_NORMAL );
	m_pPlayer->pev->gravity = 1.0f;
	SendWeaponAnim( GATLING_HOLSTER, UseDecrement(), 0 );
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CGatling::PrimaryAttack( void )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_GATLING );

	// Don't fire underwater
	if ( m_pPlayer->pev->waterlevel == 3 || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_PERFECT, 8192, BULLET_PLAYER_GATLING, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	SendWeaponAnim( GATLING_FIRE, UseDecrement(), 0 );
	switch ( RANDOM_LONG( 0, 3 ) )
	{
	case 0: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/gatling_fire1.wav", 1.0f, ATTN_NORM ); break;
	case 1: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/gatling_fire2.wav", 1.0f, ATTN_NORM ); break;
	case 2: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/gatling_fire3.wav", 1.0f, ATTN_NORM ); break;
	case 3: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/gatling_fire4.wav", 1.0f, ATTN_NORM ); break;
	}

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usEvent, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );
	m_pPlayer->pev->punchangle.x -= 2.0f;

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); // HEV suit - indicate out of ammo condition

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.12f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CGatling::SecondaryAttack( void )
{
	m_pPlayer->DropPlayerItem();
}

void CGatling::WeaponIdle( void )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_GATLING );
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	SendWeaponAnim( GATLING_IDLE, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}
