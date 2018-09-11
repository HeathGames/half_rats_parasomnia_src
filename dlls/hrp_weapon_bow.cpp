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
// Bow
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#ifndef CLIENT_DLL
#include "hrp_entity_bow_arrow.h"
#endif
#include "player.h"
#include "skill.h"
#include "weapons.h"

#define ARROW_AIR_VELOCITY		2000
#define ARROW_WATER_VELOCITY	1000

enum bow_e
{
	BOW_IDLE = 0,
	BOW_IDLE_EMPTY,
	BOW_DRAW,
	BOW_DRAW_EMPTY,
	BOW_HOLSTER,
	BOW_HOLSTER_EMPTY,
	BOW_SHOOT,
	BOW_RELOAD
};

LINK_ENTITY_TO_CLASS( weapon_bow, CBow );
LINK_ENTITY_TO_CLASS( weapon_crossbow, CBow );

void CBow::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_bow" );
	m_iId = WEAPON_BOW;
	Precache();
	SET_MODEL( ENT( pev ), "models/w_bow.mdl" );
	m_iDefaultAmmo = BOW_MAX_CLIP;
	FallInit();
}

void CBow::Precache( void )
{
	PRECACHE_MODEL( "models/p_bow.mdl" );
	PRECACHE_MODEL( "models/v_bow.mdl" );
	PRECACHE_MODEL( "models/w_bow.mdl" );

	PRECACHE_SOUND( "weapons/bow_fire.wav" );

	UTIL_PrecacheOther( "bow_arrow" );
}

int CBow::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "arrow";
	p->iMaxAmmo1 = ARROW_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BOW_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_BOW;
	p->iWeight = BOW_WEIGHT;
	return 1;
}

BOOL CBow::Deploy( void )
{
	return DefaultDeploy( "models/v_bow.mdl", "models/p_bow.mdl", (m_iClip == 0) ? BOW_DRAW_EMPTY : BOW_DRAW, "bow", UseDecrement(), 0 );
}

void CBow::Holster( int skiplocal )
{
	m_fInReload = FALSE;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	SendWeaponAnim( (m_iClip == 0) ? BOW_HOLSTER_EMPTY : BOW_HOLSTER, UseDecrement(), 0 );
}

void CBow::PrimaryAttack( void )
{
	// You are not allowed to fire empty
	if ( m_iClip <= 0 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15f;
		return;
	}

	// Weapon's volume and muzzleflash
	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	// Remove an arrow
	m_iClip--;

	// Third person anim and firing sound
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/bow_fire.wav", 1.0f, ATTN_NORM );

	// Perform some calculations to fire the arrow
	Vector vecAnglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors( vecAnglesAim );
	vecAnglesAim.x = -vecAnglesAim.x;
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2.0f;
	Vector vecDir = gpGlobals->v_forward;

#ifndef CLIENT_DLL
	// Create the arrow
	CBowArrow *pArrow = CBowArrow::ArrowCreate();
	pArrow->pev->origin = vecSrc;
	pArrow->pev->angles = vecAnglesAim;
	pArrow->pev->owner = m_pPlayer->edict();

	// Update arrow velocity/depending depending if we are underwater or not
	if ( m_pPlayer->pev->waterlevel == 3 )
	{
		pArrow->pev->velocity = vecDir * ARROW_WATER_VELOCITY;
		pArrow->pev->speed = ARROW_WATER_VELOCITY;
	}
	else
	{
		pArrow->pev->velocity = vecDir * ARROW_AIR_VELOCITY;
		pArrow->pev->speed = ARROW_AIR_VELOCITY;
	}
	pArrow->pev->avelocity.z = 10.0f;
#endif

	// Punch and firing animation
	m_pPlayer->pev->punchangle.x -= 10.0f;
	SendWeaponAnim( BOW_SHOOT, UseDecrement(), 0 );

	// Play the "out of ammo" sentence
	if ( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

	// Timings
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.25f;
}

void CBow::SecondaryAttack( void )
{
	return;
}

void CBow::Reload( void )
{
	// No more ammo
	if ( m_pPlayer->ammo_arrow <= 0 )
		return;

	// Reload and apply the timings
	if ( DefaultReload( BOW_MAX_CLIP, BOW_RELOAD, 2.0f, 0 ) )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25f;
}

void CBow::WeaponIdle( void )
{
	// Common to all firing weapons
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// Just play an idle animation
	SendWeaponAnim( (m_iClip == 0) ? BOW_IDLE_EMPTY : BOW_IDLE, UseDecrement(), 0 );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

class CBowAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_arrowbox.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_arrowbox.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if ( pOther->GiveAmmo( AMMO_ARROWBOX_GIVE, "arrow", ARROW_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pOther->pev ), CHAN_ITEM, "items/9mmclip1.wav", 1.0f, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};
#endif
