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
// Bernard's puke
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "decals.h"
#include "hrp_entity_bernard_puke.h"
#include "skill.h"

LINK_ENTITY_TO_CLASS( bernard_puke, CBernardPuke );

TYPEDESCRIPTION CBernardPuke::m_SaveData[] =
{
	DEFINE_FIELD( CBernardPuke, m_iSpriteMaxFrames, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CBernardPuke, CBaseEntity );

void CBernardPuke::Spawn( void )
{
	Precache();
	pev->movetype = MOVETYPE_TOSS;
	pev->classname = MAKE_STRING( "bernard_puke" );

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL( ENT( pev ), "sprites/bigspit.spr" );
	pev->frame = 0;
	pev->scale = 1.0f;

	UTIL_SetSize( pev, Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ) );
	m_iSpriteMaxFrames = (float)MODEL_FRAMES( pev->modelindex ) - 1;
}

void CBernardPuke::Precache( void )
{
	m_iBernardPukeSprite = PRECACHE_MODEL( "sprites/bigspit.spr" );
	PRECACHE_MODEL( "sprites/tinyspit.spr" );

	PRECACHE_SOUND( "bullchicken/bc_acid1.wav" );
	PRECACHE_SOUND( "bullchicken/bc_spithit1.wav" );
	PRECACHE_SOUND( "bullchicken/bc_spithit2.wav" );
}

void CBernardPuke::PukeAnimate( void )
{
	pev->nextthink = gpGlobals->time + 0.1f;
	if ( pev->frame++ )
	{
		if ( pev->frame > m_iSpriteMaxFrames )
			pev->frame = 0;
	}
}

void CBernardPuke::Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CBernardPuke *pPuke = GetClassPtr( (CBernardPuke *)NULL );
	pPuke->Spawn();

	UTIL_SetOrigin( pPuke->pev, vecStart );
	pPuke->pev->velocity = vecVelocity;
	pPuke->pev->owner = ENT( pevOwner );

	pPuke->SetThink( &CBernardPuke::PukeAnimate );
	pPuke->pev->nextthink = gpGlobals->time + 0.1f;
}

void CBernardPuke::Touch( CBaseEntity *pOther )
{
	int iPitch = RANDOM_FLOAT( 90.0f, 110.0f );
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1.0f, ATTN_NORM, 0, iPitch );

	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1.0f, ATTN_NORM, 0, iPitch ); break;
	case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1.0f, ATTN_NORM, 0, iPitch ); break;
	}

	if ( !pOther->pev->takedamage )
	{
		TraceResult pTr;
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &pTr );
		UTIL_DecalTrace( &pTr, DECAL_SPIT1 + RANDOM_LONG( 0, 1 ) );
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pTr.vecEndPos );
			WRITE_BYTE( TE_SPRITE_SPRAY );
			WRITE_COORD( pTr.vecEndPos.x );
			WRITE_COORD( pTr.vecEndPos.y );
			WRITE_COORD( pTr.vecEndPos.z );
			WRITE_COORD( pTr.vecPlaneNormal.x );
			WRITE_COORD( pTr.vecPlaneNormal.y );
			WRITE_COORD( pTr.vecPlaneNormal.z );
			WRITE_COORD( m_iBernardPukeSprite );
			WRITE_BYTE( 5 );
			WRITE_BYTE( 30 );
			WRITE_BYTE( 80 );
		MESSAGE_END();
	}
	else
		pOther->TakeDamage( pev, pev, gSkillData.bernardDmgPuke, DMG_POISON );

	SetThink( &CBernardPuke::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}
