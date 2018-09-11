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
// Dynamite
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "skill.h"
#include "weapons.h"

#include "hrp_weapon_dynamite.h"

LINK_ENTITY_TO_CLASS( weapon_dynamite, CDynamite );
LINK_ENTITY_TO_CLASS( weapon_handgrenade, CDynamite ); // Half-Life backward compatibility maps

enum dynamite_e
{
	DYNAMITE_IDLE = 0,
	DYNAMITE_FIDGET,
	DYNAMITE_FUSE,
	DYNAMITE_THROW1,
	DYNAMITE_THROW2,
	DYNAMITE_THROW3,
	DYNAMITE_HOLSTER,
	DYNAMITE_DRAW,
	DYNAMITE_FUSE_ROLL,
	DYNAMITE_THROW_ROLL
};

void CDynamite::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_dynamite" );
	m_iId = WEAPON_DYNAMITE;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_dynamite.mdl" );

#ifndef CLIENT_DLL
	pev->dmg = gSkillData.plrDmgDynamite;
#endif
	m_iDefaultAmmo = 1;
	FallInit();
}

void CDynamite::Precache( void )
{
	PRECACHE_MODEL( "models/p_dynamite.mdl" );
	PRECACHE_MODEL( "models/v_dynamite.mdl" );
	PRECACHE_MODEL( "models/w_dynamite.mdl" );
}

int CDynamite::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "dynamite";
	p->iMaxAmmo1 = DYNAMITE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_DYNAMITE;
	p->iWeight = 5;
	return 1;
}

BOOL CDynamite::Deploy( void )
{
	m_flReleaseThrow = -1.0f;
	return DefaultDeploy( "models/v_dynamite.mdl", "models/p_dynamite.mdl", DYNAMITE_DRAW, "crowbar", UseDecrement(), 0 );
}

void CDynamite::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
	{
		m_pPlayer->pev->weapons &= ~( 1 << WEAPON_DYNAMITE);
		SetThink( &CDynamite::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1f;
	}
	else
	{
		SendWeaponAnim( DYNAMITE_HOLSTER, UseDecrement(), 0 );
	}
}

void CDynamite::PrimaryAttack( void )
{
	// I still have some dynamites and I'm not throwing one already, so lit the fuse
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0.0f;

		SendWeaponAnim( DYNAMITE_FUSE, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
		m_bToss = false;
	}
}

void CDynamite::SecondaryAttack( void )
{
	// I still have some dynamites and I'm not throwing one already, so lit the fuse
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0.0f;

		SendWeaponAnim( DYNAMITE_FUSE_ROLL, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;
		m_bToss = true;
	}
}

void CDynamite::WeaponIdle( void )
{
	if ( m_flReleaseThrow == 0.0f && m_flStartThrow )
		m_flReleaseThrow = gpGlobals->time;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// I'm holding a dynamite in my hand
	if ( m_flStartThrow )
	{
		Vector vecAngThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if ( vecAngThrow.x < 0.0f )
			vecAngThrow.x = -10.0f + vecAngThrow.x * (80.0f / 90.0f);
		else
			vecAngThrow.x = -10.0f + vecAngThrow.x * (100.0f / 90.0f);

		float flVel = (90.0f - vecAngThrow.x) * 4.0f;
		if ( flVel > 500.0f )
			flVel = 500.0f;

		UTIL_MakeVectors( vecAngThrow );
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16.0f;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		if ( m_bToss )
		{
			vecSrc = m_pPlayer->pev->origin;
			vecThrow = gpGlobals->v_forward * 274.0f + m_pPlayer->pev->velocity;
		}

		// Always explode 3 seconds after the fuse was lit
		float flTime = m_flStartThrow - gpGlobals->time + 3.0f;
		if ( flTime < 0.0f )
			flTime = 0.0f;

		// Spawn the "real dynamite"
		CGrenade::ShootTimed( m_pPlayer->pev, vecSrc, vecThrow, flTime );

		if ( m_bToss )
		{
			SendWeaponAnim( DYNAMITE_THROW_ROLL, UseDecrement(), 0 );
		}
		else
		{
			if ( flVel < 500.0f )
				SendWeaponAnim( DYNAMITE_THROW1, UseDecrement(), 0 );
			else if ( flVel < 1000.0f )
				SendWeaponAnim( DYNAMITE_THROW2, UseDecrement(), 0 );
			else
				SendWeaponAnim( DYNAMITE_THROW3, UseDecrement(), 0 );
		}

		// Third person anim
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		// Reset everything and timings
		m_flReleaseThrow = 0.0f;
		m_flStartThrow = 0.0f;
		m_flNextPrimaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5f;

		// Remove a dynamite from my pocket
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			// Just threw last dynamite
			// Set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f; // Ensure that the animation can finish playing
		}
		return;
	}
	else if ( m_flReleaseThrow > 0.0f )
	{
		// We've finished the throw, restart.
		m_flStartThrow = 0.0f;

		if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			SendWeaponAnim( DYNAMITE_DRAW );
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0f, 15.0f );
		m_flReleaseThrow = -1.0f;
		return;
	}

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		int iAnim;
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: iAnim = DYNAMITE_IDLE; break;
		case 1: iAnim = DYNAMITE_FIDGET; break;
		}
		SendWeaponAnim( iAnim, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
	}
}
