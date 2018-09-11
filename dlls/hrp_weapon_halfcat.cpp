#ifdef USE_MP_WEAPONS
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
// Half-Cat (Weapon)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum halfcat_weapon_e {
	HALFCAT_IDLE = 0,
	HALFCAT_FIDGETFIT,
	HALFCAT_FIDGETNIP,
	HALFCAT_DOWN,
	HALFCAT_UP,
	HALFCAT_THROW
};

LINK_ENTITY_TO_CLASS( weapon_halfcat, CHalfCatWeapon );
LINK_ENTITY_TO_CLASS( weapon_snark, CHalfCatWeapon );

void CHalfCatWeapon::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_halfcat" );
	m_iId = WEAPON_HALFCAT;
	Precache();
	SET_MODEL( ENT( pev ), "models/w_halfcat.mdl" );
	m_iDefaultAmmo = 5;
	pev->sequence = 4;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0f;
	FallInit();
}

void CHalfCatWeapon::Precache( void )
{
	PRECACHE_MODEL( "models/p_halfcat.mdl" );
	PRECACHE_MODEL( "models/v_halfcat.mdl" );
	PRECACHE_MODEL( "models/w_halfcat.mdl" );

	PRECACHE_SOUND( "weapons/cat_throw.wav" );

	UTIL_PrecacheOther( "monster_hc_snark" );
}

int CHalfCatWeapon::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "Half-Cats";
	p->iMaxAmmo1 = HALFCAT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_HALFCAT;
	p->iWeight = HALFCAT_WEIGHT;
	return 1;
}

BOOL CHalfCatWeapon::Deploy( void )
{
	return DefaultDeploy( "models/v_halfcat.mdl", "models/p_halfcat.mdl", HALFCAT_UP, "squeak", UseDecrement(), 0 );
}

void CHalfCatWeapon::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] ) // Just holster it
		SendWeaponAnim( HALFCAT_DOWN, UseDecrement(), 0 );
	else
	{
		// No more Half-Cats!
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_HALFCAT);
		SetThink( &CHalfCatWeapon::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "common/null.wav", 1.0f, ATTN_NORM );
}

void CHalfCatWeapon::PrimaryAttack( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		TraceResult pTr;
		Vector vecTraceOrigin;

		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		vecTraceOrigin = m_pPlayer->pev->origin;
		if ( m_pPlayer->pev->flags & FL_DUCKING )
			vecTraceOrigin = vecTraceOrigin - (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

		// Find place to toss monster
		UTIL_TraceLine( vecTraceOrigin + gpGlobals->v_forward * 20.0f, vecTraceOrigin + gpGlobals->v_forward * 64.0f, dont_ignore_monsters, NULL, &pTr );

		SendWeaponAnim( HALFCAT_THROW, UseDecrement(), 0 );

		if ( pTr.fAllSolid == 0 && pTr.fStartSolid == 0 && pTr.flFraction > 0.25f )
		{
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
#ifndef CLIENT_DLL
			CBaseEntity *pHalfCat = CBaseEntity::Create( "monster_hc_snark", pTr.vecEndPos, m_pPlayer->pev->v_angle, m_pPlayer->edict() );
			pHalfCat->pev->velocity = gpGlobals->v_forward * 200.0f + m_pPlayer->pev->velocity;
#endif
			EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "weapons/cat_throw.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
			m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_bJustThrown = TRUE;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
		}
	}
}

void CHalfCatWeapon::SecondaryAttack( void )
{
	PrimaryAttack();
}

void CHalfCatWeapon::WeaponIdle( void )
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_bJustThrown )
	{
		m_bJustThrown = FALSE;
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			RetireWeapon();
			return;
		}
		SendWeaponAnim( HALFCAT_UP, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
		return;
	}

	int iAnim;
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0: iAnim = HALFCAT_IDLE; break;
	case 1: iAnim = HALFCAT_FIDGETFIT; break;
	default: iAnim = HALFCAT_FIDGETNIP;
	}
	SendWeaponAnim( iAnim, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
}
#endif
