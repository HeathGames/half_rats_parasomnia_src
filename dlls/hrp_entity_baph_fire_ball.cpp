/****
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Baph's fire ball
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"
#include "hrp_entity_baph_fire_ball.h"

LINK_ENTITY_TO_CLASS( baph_fire_ball, CBaphFireBall );

TYPEDESCRIPTION	CBaphFireBall::m_SaveData[] =
{
	DEFINE_FIELD( CBaphFireBall, m_hEnemy, FIELD_CLASSPTR ),
	DEFINE_FIELD( CBaphFireBall, m_vecIdeal, FIELD_VECTOR),
};
IMPLEMENT_SAVERESTORE( CBaphFireBall, CBaseMonster );

void CBaphFireBall::Spawn( void )
{
	Precache();
	SET_MODEL( edict(), "sprites/xfireball3.spr" );
	UTIL_SetSize( pev, Vector( -4.0f, -4.0f, -4.0f ), Vector( 4.0f, 4.0f, 4.0f ) );

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLY;
	pev->velocity.z *= 0.2f;

	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255.0f;
	pev->rendercolor.x = 255.0f;
	pev->rendercolor.y = 255.0f;
	pev->rendercolor.z = 255.0f;
}

void CBaphFireBall::Precache( void )
{
	PRECACHE_MODEL( "sprites/xfireball3.spr" );

	PRECACHE_SOUND( "x/x_teleattack1.wav" );
}

void CBaphFireBall::FireBallInit( CBaseEntity *pTarget )
{
	m_hEnemy = pTarget;

	SetThink( &CBaphFireBall::FireBallThink );
	SetTouch( &CBaphFireBall::FireBallTouch );
	pev->nextthink = gpGlobals->time + 0.1f;

	EMIT_SOUND( edict(), CHAN_WEAPON, "x/x_teleattack1.wav", 0.2f, ATTN_NORM );
}

void CBaphFireBall::FireBallThink( void )
{
	pev->nextthink = gpGlobals->time + 0.1f;

	// Check world boundaries
	if ( m_hEnemy == NULL || !m_hEnemy->IsAlive() || pev->origin.x < -4096.0f || pev->origin.x > 4096.0f || pev->origin.y < -4096.0f || pev->origin.y > 4096.0f || pev->origin.z < -4096.0f || pev->origin.z > 4096.0f )
	{
		Explode();
		return;
	}

	if ( (m_hEnemy->Center() - pev->origin).Length() < 16.0f )
	{
		Explode();
		return;
	}
	else
		MoveToOrigin( m_hEnemy->Center() );

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_ELIGHT );
		WRITE_SHORT( entindex() );		// Entity, attachment
		WRITE_COORD( pev->origin.x );	// Origin
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( 256 );				// Radius
		WRITE_BYTE( 255 );				// Red
		WRITE_BYTE( 0 );				// Green
		WRITE_BYTE( 0 );				// Blue
		WRITE_BYTE( 10 );				// Life * 10
		WRITE_COORD( 256 );				// Decay
	MESSAGE_END();

	pev->frame = (int)(pev->frame + 1) % 20;
}

void CBaphFireBall::FireBallTouch( CBaseEntity *pOther )
{
	if ( pOther->edict() != pev->owner )
		Explode();
}

void CBaphFireBall::MoveToOrigin( Vector _vecTarget )
{
	if ( m_vecIdeal == Vector( 0.0f, 0.0f, 0.0f ) )
		m_vecIdeal = pev->velocity;

	// Accelerate
	float flSpeed = m_vecIdeal.Length();
	if ( flSpeed > 300.0f )
		m_vecIdeal = m_vecIdeal.Normalize() * 300.0f;

	m_vecIdeal = m_vecIdeal + (_vecTarget - pev->origin).Normalize() * 300.0f;
	pev->velocity = m_vecIdeal;
}

void CBaphFireBall::Explode( void )
{
	STOP_SOUND( edict(), CHAN_WEAPON, "x/x_teleattack1.wav" );
	ExplosionCreate( pev->origin, pev->angles, pev->owner, 10.0f, TRUE );
	SetTouch( NULL );
	UTIL_Remove( this );
	pev->nextthink = gpGlobals->time + 0.2f;
}
