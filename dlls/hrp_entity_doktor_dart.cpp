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
// Doktor dart
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_entity_doktor_dart.h"
#include "skill.h"
#include "weapons.h"

#define SYRINGE_AIR_VELOCITY	2000
#define SYRINGE_WATER_VELOCITY	1000

LINK_ENTITY_TO_CLASS( doktor_dart, CDoktorDart );

void CDoktorDart::Spawn( void )
{
	Precache();
	pev->classname = MAKE_STRING( "doktor_dart" );
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.01f;
	pev->avelocity.z = 10.0f;

	SET_MODEL( ENT( pev ), "models/syringe.mdl" );

	UTIL_SetOrigin( pev, pev->origin );
	UTIL_SetSize( pev, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ) );

	pev->angles.x -= 30.0f;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 30.0f);

	pev->velocity = gpGlobals->v_forward * 250.0f;
	pev->gravity = 0.5f;

	SetTouch( &CDoktorDart::DartTouch );
	SetThink( &CDoktorDart::DartThink );
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CDoktorDart::Precache( void )
{
	PRECACHE_MODEL( "models/syringe.mdl" );

	PRECACHE_SOUND( "doktor/dok_syringe_hit.wav" );
	PRECACHE_SOUND( "weapons/xbow_hitbod1.wav" );
	PRECACHE_SOUND( "weapons/xbow_hitbod2.wav" );
	PRECACHE_SOUND( "weapons/xbow_fly1.wav" );
}

void CDoktorDart::DartTouch( CBaseEntity *pOther )
{
	SetThink( NULL );
	SetTouch( NULL );

	if ( pOther->pev->takedamage )
	{
		TraceResult pTr = UTIL_GetGlobalTrace();
		entvars_t *pOwner;
		pOwner = VARS( pev->owner );

		// UNDONE: this needs to call TraceAttack instead
		ClearMultiDamage();

		pOther->TraceAttack( pOwner, gSkillData.doktorDmgDart, pev->velocity.Normalize(), &pTr, DMG_POISON | DMG_NEVERGIB );

		ApplyMultiDamage( pev, pOwner );
		pev->velocity = Vector( 0.0f, 0.0f, 0.0f );

		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: EMIT_SOUND( ENT( pev ), CHAN_BODY, "weapons/xbow_hitbod1.wav", 1.0f, ATTN_NORM ); break;
		case 1: EMIT_SOUND( ENT( pev ), CHAN_BODY, "weapons/xbow_hitbod2.wav", 1.0f, ATTN_NORM ); break;
		}
		Killed( pev, GIB_NEVER );
	}
	else
	{
		EMIT_SOUND_DYN( ENT( pev ), CHAN_BODY, "doktor/dok_syringe_hit.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );

		SetThink( &CDoktorDart::SUB_Remove );
		pev->nextthink = gpGlobals->time;

		if ( FClassnameIs( pOther->pev, "worldspawn" ) )
		{
			Vector vecDir = pev->velocity.Normalize();
			UTIL_SetOrigin( pev, pev->origin - vecDir * 12.0f );
			pev->angles = UTIL_VecToAngles( vecDir );
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_FLY;
			pev->velocity = Vector( 0.0f, 0.0f, 0.0f );
			pev->avelocity.z = 0.0f;
			pev->angles.z = RANDOM_LONG( 0, 360 );
			pev->nextthink = gpGlobals->time + 10.0f;
		}

		if ( UTIL_PointContents( pev->origin ) != CONTENTS_WATER )
			UTIL_Sparks( pev->origin );
	}
}

void CDoktorDart::DartThink( void )
{
	pev->movetype = MOVETYPE_FLY;
	UTIL_MakeAimVectors( pev->angles );
	pev->angles = UTIL_VecToAngles( gpGlobals->v_forward );

	if ( pev->waterlevel == 3 )
	{
		pev->velocity = gpGlobals->v_forward * SYRINGE_WATER_VELOCITY;
		pev->speed = SYRINGE_WATER_VELOCITY;
		UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1f, pev->origin, 1.0f );
	}
	else
	{
		pev->velocity = gpGlobals->v_forward * SYRINGE_AIR_VELOCITY;
		pev->speed = SYRINGE_AIR_VELOCITY;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}
