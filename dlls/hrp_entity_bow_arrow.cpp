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
// Bow arrow
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_entity_bow_arrow.h"
#include "skill.h"
#include "weapons.h"

LINK_ENTITY_TO_CLASS( bow_arrow, CBowArrow );

CBowArrow *CBowArrow::ArrowCreate( void )
{
	CBowArrow *pArrow = GetClassPtr( (CBowArrow *)NULL );
	pArrow->pev->classname = MAKE_STRING( "arrow" );
	pArrow->Spawn();
	return pArrow;
}

void CBowArrow::Spawn( void )
{
	Precache();
	SET_MODEL( ENT( pev ), "models/bow_arrow.mdl" );

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.5f;

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize( pev, Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ) );

	SetTouch( &CBowArrow::ArrowTouch );
	SetThink( &CBowArrow::ArrowThink );
	pev->nextthink = gpGlobals->time + 0.2f;
}

void CBowArrow::Precache( void )
{
	PRECACHE_MODEL( "models/bow_arrow.mdl" );

	PRECACHE_SOUND( "weapons/bow_hitbod1.wav" );
	PRECACHE_SOUND( "weapons/bow_hitbod2.wav" );
	PRECACHE_SOUND( "weapons/bow_fly.wav" );
	PRECACHE_SOUND( "weapons/bow_hit.wav" );
}

void CBowArrow::ArrowThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1f;
	if ( pev->waterlevel == 0 )
		return;

	UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1f, pev->origin, 1.0f );
}

void CBowArrow::ArrowTouch( CBaseEntity *pOther )
{
	SetThink( NULL );
	SetTouch( NULL );

	if ( pOther->pev->takedamage )
	{
		TraceResult pTr = UTIL_GetGlobalTrace();
		entvars_t *pOwner = VARS( pev->owner );

		ClearMultiDamage();
		pOther->TraceAttack( pOwner, gSkillData.plrDmgBowArrow, pev->velocity.Normalize(), &pTr, DMG_NEVERGIB );
		ApplyMultiDamage( pev, pOwner );

		pev->velocity = Vector( 0.0f, 0.0f, 0.0f );
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: EMIT_SOUND( ENT( pev ), CHAN_BODY, "weapons/bow_hitbod1.wav", 1.0f, ATTN_NORM ); break;
		case 1: EMIT_SOUND( ENT( pev ), CHAN_BODY, "weapons/bow_hitbod2.wav", 1.0f, ATTN_NORM ); break;
		}

		Killed( pev, GIB_NEVER );
	}
	else
	{
		EMIT_SOUND_DYN( ENT( pev ), CHAN_BODY, "weapons/bow_hit.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
		pev->nextthink = gpGlobals->time;
		if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		{
			Vector vecDir = pev->velocity.Normalize();
			UTIL_SetOrigin( pev, pev->origin - vecDir * 12.0f );
			pev->angles = UTIL_VecToAngles( vecDir );
			pev->solid = SOLID_NOT;
			pev->velocity = Vector( 0.0f, 0.0f, 0.0f );
			pev->avelocity.z = 0.0f;
			pev->angles.z = RANDOM_LONG( 0, 360 );
			pev->nextthink = gpGlobals->time + 10.0f;
			if ( UTIL_PointContents( pev->origin ) != CONTENTS_WATER )
				UTIL_Sparks( pev->origin );
		}
	}
}
#endif
