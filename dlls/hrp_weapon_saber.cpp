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
// Cavalry saber
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "skill.h"
#include "weapons.h"

#include "hrp_weapon_saber.h"

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	float distance = 1e6f;

	Vector vecHullEnd = tr.vecEndPos;
	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	TraceResult tmpTrace;
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );

	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	int i;
	for ( i = 0; i < 2; i++ )
	{
		int j;
		for ( j = 0; j < 2; j++ )
		{
			int k;
			for ( k = 0; k < 2; k++ )
			{
				float *minmaxs[2] = { mins, maxs };
				Vector vecEnd;
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

LINK_ENTITY_TO_CLASS( weapon_saber, CSaber );
LINK_ENTITY_TO_CLASS( weapon_crowbar, CSaber ); // Half-Life backward compatibility maps

enum saber_e {
	SABER_IDLE = 0,
	SABER_DRAW,
	SABER_HOLSTER,
	SABER_ATTACK1,
	SABER_KICK,
	SABER_ATTACK2,
	SABER_UNUSED,
	SABER_BLOCK_START,
	SABER_BLOCK_IDLE,
	SABER_BLOCK_HIT,
	SABER_BLOCK_END
};

void CSaber::Spawn( void )
{
	pev->classname = MAKE_STRING( "weapon_saber" );
	m_iId = WEAPON_SABER;

	Precache();
	SET_MODEL( ENT( pev ), "models/w_saber.mdl" );

	m_iClip = -1;
	FallInit();
}

void CSaber::Precache( void )
{
	PRECACHE_MODEL( "models/p_saber.mdl" );
	PRECACHE_MODEL( "models/v_saber.mdl" );
	PRECACHE_MODEL( "models/w_saber.mdl" );

	PRECACHE_SOUND( "weapons/sab_block1.wav" );
	PRECACHE_SOUND( "weapons/sab_block2.wav" );
	PRECACHE_SOUND( "weapons/sab_block3.wav" );
	PRECACHE_SOUND( "weapons/sab_hitbod1.wav" );
	PRECACHE_SOUND( "weapons/sab_hitbod2.wav" );
	PRECACHE_SOUND( "weapons/sab_hitbod3.wav" );
	PRECACHE_SOUND( "weapons/sab_hitbod4.wav" );
	PRECACHE_SOUND( "weapons/sab_hit1.wav" );
	PRECACHE_SOUND( "weapons/sab_hit2.wav" );
	PRECACHE_SOUND( "weapons/sab_hit3.wav" );
	PRECACHE_SOUND( "weapons/sab_kick.wav" );
	PRECACHE_SOUND( "weapons/sab_kick_hit.wav" );
	PRECACHE_SOUND( "weapons/sab1.wav" );
	PRECACHE_SOUND( "weapons/sab2.wav" );
}

int CSaber::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY;
	p->iId = m_iId = WEAPON_SABER;
	p->iWeight = 10;
	return 1;
}

BOOL CSaber::Deploy( void )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_NORMAL );
	m_bPlayBlockHitAnim = false;
	m_iParaExtraStuff = SABER_BLOCK_STATE_NO_BLOCK;
	return DefaultDeploy( "models/v_saber.mdl", "models/p_saber.mdl", SABER_DRAW, "crowbar", UseDecrement(), 0 );
}

void CSaber::Holster( int skiplocal )
{
	m_pPlayer->SetSpeed( PLAYER_SPEED_NORMAL );
	m_iParaExtraStuff = SABER_BLOCK_STATE_NO_BLOCK;
	SendWeaponAnim( SABER_HOLSTER, UseDecrement(), 0 );
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
}

void CSaber::PrimaryAttack( void )
{
	if ( m_iParaExtraStuff != SABER_BLOCK_STATE_NO_BLOCK )
		return;

	// Initialize stuff
	TraceResult pTraceResult;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32.0f;

	// Trace a line first
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &pTraceResult );
#ifndef CLIENT_DLL
	if ( pTraceResult.flFraction >= 1.0 )
	{
		// Now trace a hull
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &pTraceResult );
		if ( pTraceResult.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( pTraceResult.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, pTraceResult, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );

			vecEnd = pTraceResult.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	// Swing animations and sounds
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	if ( m_bUseSecondarySwing )
	{
		EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/sab2.wav", 1.0f, ATTN_NORM );
		SendWeaponAnim( SABER_ATTACK2, UseDecrement(), 0 );
	}
	else
	{
		EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/sab1.wav", 1.0f, ATTN_NORM );
		SendWeaponAnim( SABER_ATTACK1, UseDecrement(), 0 );
	}
	m_bUseSecondarySwing = !m_bUseSecondarySwing;

	// We hit something
	if ( pTraceResult.flFraction < 1.0f )
	{
		// Deal damage
		CBaseEntity *pEntity = CBaseEntity::Instance( pTraceResult.pHit );
		ClearMultiDamage();
		pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgSaber, gpGlobals->v_forward, &pTraceResult, DMG_SLASH );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		if ( pEntity )
		{
			// Looks like we hit flesh
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				switch( RANDOM_LONG( 0, 3 ) )
				{
				case 0: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hitbod1.wav", 1.0f, ATTN_NORM ); break;
				case 1: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hitbod2.wav", 1.0f, ATTN_NORM ); break;
				case 2: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hitbod3.wav", 1.0f, ATTN_NORM ); break;
				case 3: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hitbod4.wav", 1.0f, ATTN_NORM ); break;
				}
				m_pPlayer->m_iWeaponVolume = 128;
			}
			else // We hit something else then
			{
#ifndef CLIENT_DLL
				float flVolume = g_pGameRules->IsMultiplayer() ? 1.0f : TEXTURETYPE_PlaySound( &pTraceResult, vecSrc, vecSrc + (vecEnd - vecSrc) * 2.0f, BULLET_PLAYER_SABER );
				switch( RANDOM_LONG( 0, 2 ) )
				{
				case 0: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hit1.wav", flVolume, ATTN_NORM ); break;
				case 1: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hit2.wav", flVolume, ATTN_NORM ); break;
				case 2: EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_hit3.wav", flVolume, ATTN_NORM ); break;
				}
				m_pPlayer->m_iWeaponVolume = 512.0f * flVolume;
#endif
				m_pTraceResult = pTraceResult;
				SetThink( &CSaber::Smack );
				pev->nextthink = UTIL_WeaponTimeBase() + 0.25f;
			}
		}
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
}

void CSaber::SecondaryAttack( void )
{
	// Not blocking yet
	if ( m_iParaExtraStuff == SABER_BLOCK_STATE_NO_BLOCK )
	{
		SendWeaponAnim( SABER_BLOCK_START, UseDecrement(), 0 );
		m_iParaExtraStuff = SABER_BLOCK_STATE_STARTING;
		m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.25f;
	}
	else if ( m_iParaExtraStuff == SABER_BLOCK_STATE_STARTING )
		m_iParaExtraStuff = SABER_BLOCK_STATE_BLOCKING;
	else if ( m_iParaExtraStuff == SABER_BLOCK_STATE_BLOCKING )
	{
		SendWeaponAnim( SABER_BLOCK_END, UseDecrement(), 0 );
		m_iParaExtraStuff = SABER_BLOCK_STATE_STOPPING;
		m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.25f;
	}
	else
		m_iParaExtraStuff = SABER_BLOCK_STATE_NO_BLOCK;
}

void CSaber::Smack( void )
{
	DecalGunshot( &m_pTraceResult, BULLET_PLAYER_SABER );
}

void CSaber::Reload( void )
{
	// Deny the kick if we are doing something
	if ( m_pPlayer->m_flNextAttack > UTIL_WeaponTimeBase() || m_flNextPrimaryAttack > UTIL_WeaponTimeBase() ||
			m_flNextSecondaryAttack > UTIL_WeaponTimeBase() || m_iParaExtraStuff != SABER_BLOCK_STATE_NO_BLOCK )
		return;

	// Nullify the velocity if we are on the ground
	if ( m_pPlayer->pev->flags & FL_ONGROUND )
		m_pPlayer->pev->velocity = Vector( 0.0f, 0.0f, 0.0f );

	// Kick anim/sound and third person anim
	SendWeaponAnim( SABER_KICK );
	EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/sab_kick.wav", 1.0f, ATTN_NORM );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Kick dynamites and clown bombs!
	CBaseEntity *pBombOrDynamite = UTIL_FindEntityInSphere( m_pPlayer, m_pPlayer->pev->origin, 64.0f );
#ifdef CLIENT_DLL
	if ( pBombOrDynamite && FClassnameIs( pBombOrDynamite->pev, "grenade" ) )
#else
	if ( pBombOrDynamite && FClassnameIs( pBombOrDynamite->pev, "grenade" ) && IsFacing( m_pPlayer->pev, pBombOrDynamite->pev->origin ) )
#endif
	{
		// Kick hit sound
		EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_kick_hit.wav", 1.0f, ATTN_NORM );
		m_pPlayer->m_iWeaponVolume = 128;

		// That entity can be moved so punch it
		Vector vecDir = pBombOrDynamite->pev->origin - (m_pPlayer->pev->absmin + m_pPlayer->pev->absmax) * 0.5f;
		vecDir = vecDir.Normalize();
		pBombOrDynamite->pev->velocity = pBombOrDynamite->pev->velocity + vecDir * 1000.0f;

		// Timings
		m_pPlayer->m_flNextAttack = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.25f;
		return;
	}

	// The following is basically a copy/paste from PrimaryAttack
	// Initialize stuff
	TraceResult pTraceResult;

	UTIL_MakeVectors( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32.0f;

	// Trace a line first
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &pTraceResult );
#ifndef CLIENT_DLL
	if ( pTraceResult.flFraction >= 1.0 )
	{
		// Now trace a hull
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &pTraceResult );
		if ( pTraceResult.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( pTraceResult.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, pTraceResult, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );

			vecEnd = pTraceResult.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	// We hit something
	if ( pTraceResult.flFraction < 1.0f )
	{
		// Deal damage
		CBaseEntity *pEntity = CBaseEntity::Instance( pTraceResult.pHit );
		ClearMultiDamage();
		pEntity->TraceAttack( m_pPlayer->pev, gSkillData.plrDmgSaber / 3.0f, gpGlobals->v_forward, &pTraceResult, DMG_CLUB );
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		if ( pEntity )
		{
			// Looks like we hit flesh
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_kick_hit.wav", 1.0f, ATTN_NORM );
				m_pPlayer->m_iWeaponVolume = 128;
			}
#ifndef CLIENT_DLL
			else // We hit something else then
			{
				float flVolume = g_pGameRules->IsMultiplayer() ? 1.0f : TEXTURETYPE_PlaySound( &pTraceResult, vecSrc, vecSrc + (vecEnd - vecSrc) * 2.0f, BULLET_PLAYER_SABER );
				EMIT_SOUND( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/sab_kick_hit.wav", flVolume, ATTN_NORM );
				m_pPlayer->m_iWeaponVolume = 512.0f * flVolume;
			}
#endif

			// That entity can be moved so punch it
			if ( pEntity->pev->movetype == MOVETYPE_WALK || pEntity->pev->movetype == MOVETYPE_STEP )
			{
				Vector vecDir = pEntity->pev->origin - (m_pPlayer->pev->absmin + m_pPlayer->pev->absmax) * 0.5f;
				vecDir = vecDir.Normalize();
				pEntity->pev->velocity = pEntity->pev->velocity + vecDir * 1000.0f;
			}
		}
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5f;
}

void CSaber::WeaponIdle( void )
{
	m_pPlayer->SetSpeed( m_iParaExtraStuff != SABER_BLOCK_STATE_NO_BLOCK ? PLAYER_SPEED_SABER : PLAYER_SPEED_NORMAL );
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_iParaExtraStuff == SABER_BLOCK_STATE_STARTING ) // Start blocking animation is done, move on to blocking loop
	{
		SendWeaponAnim( SABER_BLOCK_IDLE, UseDecrement(), 0 );
		m_iParaExtraStuff = SABER_BLOCK_STATE_BLOCKING;
	}
	else if ( m_iParaExtraStuff == SABER_BLOCK_STATE_BLOCKING ) // We are blocking
	{
		if ( m_bPlayBlockHitAnim ) // We are asked to play the "block hit" animation/sounds
		{
			switch ( RANDOM_LONG( 0, 2 ) )
			{
			case 0:	EMIT_SOUND( m_pPlayer->edict(), CHAN_WEAPON, "weapons/sab_block1.wav", 1.0f, ATTN_NORM ); break;
			case 1:	EMIT_SOUND( m_pPlayer->edict(), CHAN_WEAPON, "weapons/sab_block2.wav", 1.0f, ATTN_NORM ); break;
			case 2:	EMIT_SOUND( m_pPlayer->edict(), CHAN_WEAPON, "weapons/sab_block3.wav", 1.0f, ATTN_NORM ); break;
			}
#ifndef CLIENT_DLL
			// Sparks and minor light
			UTIL_Sparks( m_pPlayer->GetGunPosition() );
			MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, m_pPlayer->GetGunPosition() );
				WRITE_BYTE( TE_DLIGHT );
				WRITE_COORD( m_pPlayer->GetGunPosition().x );
				WRITE_COORD( m_pPlayer->GetGunPosition().y );
				WRITE_COORD( m_pPlayer->GetGunPosition().z );
				WRITE_BYTE( 10 );
				WRITE_BYTE( 255 );
				WRITE_BYTE( 255 );
				WRITE_BYTE( 128 );
				WRITE_BYTE( 1 );
				WRITE_BYTE( 0 );
			MESSAGE_END();
#endif
			SendWeaponAnim( SABER_BLOCK_HIT, UseDecrement(), 0 );
			m_bPlayBlockHitAnim = false;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
		}
		else // Just blocking
		{
			SendWeaponAnim( SABER_BLOCK_IDLE, UseDecrement(), 0 );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.01f;
		}
	}
	else if ( m_iParaExtraStuff == SABER_BLOCK_STATE_STOPPING ) // Stop blocking animation is done, move on to idle
	{
		SendWeaponAnim( SABER_IDLE, UseDecrement(), 0 );
		m_iParaExtraStuff = SABER_BLOCK_STATE_NO_BLOCK;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.01f;
	}
	else // Doing nothing, just play the idle animation
	{
		SendWeaponAnim( SABER_IDLE, UseDecrement(), 0 );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
	}
}

#ifndef CLIENT_DLL
TYPEDESCRIPTION CSaber::m_SaveData[] =
{
	DEFINE_FIELD( CSaber, m_bPlayBlockHitAnim, FIELD_BOOLEAN ),
	DEFINE_FIELD( CSaber, m_iParaExtraStuff, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CSaber, CBasePlayerWeapon );
#endif
