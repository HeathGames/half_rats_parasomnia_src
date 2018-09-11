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
// Half-Cat (as a Snark)
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "gamerules.h"
#include "soundent.h"
#include "weapons.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CHalfCatSnark : public CGrenade
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship
	int Classify( void );

	// Damage stuff
	void GibMonster( void ) {};
	void Killed( entvars_t *pevAttacker, int iGib );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void EXPORT HuntThink( void );
	void EXPORT SuperBounceTouch( CBaseEntity *pOther );

	// Half-Cat specific stuff
	EHANDLE m_hOwner;

	static float m_flNextBounceSoundTime;
	float m_flDie;
	float m_flNextHit;
	float m_flNextHunt;

	int m_iMyClass;

	Vector m_vecPosPrev;
	Vector m_vecTarget;
};
LINK_ENTITY_TO_CLASS( monster_hc_snark, CHalfCatSnark );
LINK_ENTITY_TO_CLASS( monster_snark, CHalfCatSnark );

float CHalfCatSnark::m_flNextBounceSoundTime = 0.0f;

enum hc_snark_e {
	HALFCAT_IDLE = 0,
	HALFCAT_FIDGET,
	HALFCAT_JUMP,
	HALFCAT_RUN
};

void CHalfCatSnark::Spawn( void )
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SET_MODEL( ENT( pev ), "models/w_halfcat.mdl" );
	UTIL_SetSize( pev, Vector( -4.0f, -4.0f, 0 ), Vector( 4.0f, 4.0f, 8.0f ) );
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CHalfCatSnark::SuperBounceTouch );
	SetThink( &CHalfCatSnark::HuntThink );
	pev->nextthink = gpGlobals->time + 0.1f;
	m_flNextHunt = gpGlobals->time + 1.0f;

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;
	pev->health = gSkillData.headcrabHealth;
	pev->gravity = 0.5f;
	pev->friction = 0.5f;

	m_flDie = gpGlobals->time + 15.0f;

	m_flFieldOfView = 0.0f; // 180 degrees

	if ( pev->owner )
		m_hOwner = Instance( pev->owner );

	m_flNextBounceSoundTime = gpGlobals->time; // Reset each time a Half-Cat is spawned.

	pev->sequence = HALFCAT_RUN;
	ResetSequenceInfo();
}

void CHalfCatSnark::Precache( void )
{
	PRECACHE_MODEL( "models/w_halfcat.mdl" );

	PRECACHE_SOUND( "cat/cat_atk1.wav" );
	PRECACHE_SOUND( "cat/cat_atk2.wav" );
	PRECACHE_SOUND( "cat/cat_die.wav" );
	PRECACHE_SOUND( "cat/cat_hit1.wav" );
	PRECACHE_SOUND( "cat/cat_hit2.wav" );
}

int CHalfCatSnark::Classify( void )
{
	if ( m_iMyClass != 0 )
		return m_iMyClass; // Protect against recursion

	if ( m_hEnemy != NULL )
	{
		m_iMyClass = CLASS_INSECT; // No one cares about it
		switch ( m_hEnemy->Classify() )
		{
		case CLASS_PLAYER:
		case CLASS_HUMAN_PASSIVE:
		case CLASS_HUMAN_MILITARY:
			m_iMyClass = 0;
			return CLASS_ALIEN_MILITARY; // Barney's get mad, Grunts get mad at it
		}
		m_iMyClass = 0;
	}

	return CLASS_ALIEN_BIOWEAPON;
}

void CHalfCatSnark::Killed( entvars_t *pevAttacker, int iGib )
{
	pev->model = iStringNull; // Make invisible
	SetThink( &CHalfCatSnark::SUB_Remove );
	SetTouch( NULL );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->takedamage = DAMAGE_NO;

	EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "cat/cat_die.wav", 1.0f, 0.5f, 0, PITCH_NORM );
	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, SMALL_EXPLOSION_VOLUME, 3.0 );

	UTIL_BloodDrips( pev->origin, g_vecZero, BloodColor(), 80 );

	// Reset owner so death message happens
	if ( m_hOwner != NULL )
		pev->owner = m_hOwner->edict();

	CBaseMonster::Killed( pevAttacker, GIB_ALWAYS );
}

int CHalfCatSnark::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	return CGrenade::TakeDamage( pevInflictor, pevAttacker, 0.0f, bitsDamageType );
}

void CHalfCatSnark::HuntThink( void )
{
	if ( !IsInWorld() )
	{
		SetTouch( NULL );
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1f;

	// Explode when ready
	if ( gpGlobals->time >= m_flDie )
	{
		g_vecAttackDir = pev->velocity.Normalize();
		pev->health = -1.0f;
		Killed( pev, 0 );
		return;
	}

	if ( pev->waterlevel != 0 )
	{
		if ( pev->movetype == MOVETYPE_BOUNCE )
			pev->movetype = MOVETYPE_FLY;

		pev->velocity = pev->velocity * 0.9f;
		pev->velocity.z += 8.0f;
	}
	else if ( pev->movetype = MOVETYPE_FLY )
		pev->movetype = MOVETYPE_BOUNCE;

	// Return if it's not the time to hunt
	if ( m_flNextHunt > gpGlobals->time )
		return;

	m_flNextHunt = gpGlobals->time + 2.0f;

	CBaseEntity *pOther = NULL;
	Vector vecDir;
	TraceResult pTr;

	Vector vecFlat = pev->velocity;
	vecFlat.z = 0.0f;
	vecFlat = vecFlat.Normalize();

	UTIL_MakeVectors( pev->angles );

	if ( m_hEnemy == NULL || !m_hEnemy->IsAlive() )
	{
		// Find target, bounce a bit towards it.
		Look( 512.0f );
		m_hEnemy = BestVisibleEnemy();
	}

	if ( m_hEnemy != NULL )
	{
		if ( FVisible( m_hEnemy ) )
		{
			vecDir = m_hEnemy->EyePosition() - pev->origin;
			m_vecTarget = vecDir.Normalize();
		}
		float flVel = pev->velocity.Length();
		float flAdj = 50.0f / (flVel + 10.0f);
		if ( flAdj > 1.2f )
			flAdj = 1.2f;

		pev->velocity = pev->velocity * flAdj + m_vecTarget * 300.0f;
	}

	if ( pev->flags & FL_ONGROUND )
		pev->avelocity = Vector( 0.0f, 0.0f, 0.0f );
	else
	{
		if ( pev->avelocity == Vector( 0.0f, 0.0f, 0.0f ) )
		{
			pev->avelocity.x = RANDOM_FLOAT( -100.0f, 100.0f );
			pev->avelocity.z = RANDOM_FLOAT( -100.0f, 100.0f );
		}
	}

	if ( (pev->origin - m_vecPosPrev).Length() < 1.0f )
	{
		pev->avelocity.x = RANDOM_FLOAT( -100.0f, 100.0f );
		pev->avelocity.y = RANDOM_FLOAT( -100.0f, 100.0f );
	}
	m_vecPosPrev = pev->origin;

	pev->angles = UTIL_VecToAngles( pev->velocity );
	pev->angles.z = 0.0f;
	pev->angles.x = 0.0f;
}

void CHalfCatSnark::SuperBounceTouch( CBaseEntity *pOther )
{
	TraceResult pTr = UTIL_GetGlobalTrace();

	// Don't hit the guy that launched this grenade
	if ( pev->owner && pOther->edict() == pev->owner )
		return;

	// At least until we've bounced once
	pev->owner = NULL;
	pev->angles.x = 0.0f;
	pev->angles.z = 0.0f;

	// Avoid bouncing too much
	if ( m_flNextHit > gpGlobals->time )
		return;

	if ( pOther->pev->takedamage && m_flNextAttack < gpGlobals->time )
	{
		// Attack! Make sure it's me who has touched them
		if ( pTr.pHit == pOther->edict() )
		{
			// And it's not another Half-Cat
			if ( pTr.pHit->v.modelindex != pev->modelindex )
			{
				ClearMultiDamage();
				pOther->TraceAttack( pev, gSkillData.hcSnarkDmgBite, gpGlobals->v_forward, &pTr, DMG_SLASH );
				if ( m_hOwner != NULL )
					ApplyMultiDamage( pev, m_hOwner->pev );
				else
					ApplyMultiDamage( pev, pev );

				switch ( RANDOM_LONG( 0, 1 ) )
				{
				case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "cat/cat_hit1.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
				case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "cat/cat_hit2.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
				}
				m_flNextAttack = gpGlobals->time + 0.5f;
			}
		}
	}

	m_flNextHit = gpGlobals->time + 0.1f;
	m_flNextHunt = gpGlobals->time;

	if ( g_pGameRules->IsMultiplayer() )
	{
		// In multiplayer, we limit how often Half-Cat can make their bounce sounds to prevent overflows.
		if ( gpGlobals->time < m_flNextBounceSoundTime ) // Too soon!
			return;
	}

	if ( !(pev->flags & FL_ONGROUND) )
	{
		// Play bounce sound
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_atk1.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
		case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_atk2.wav", 1, ATTN_NORM, 0, PITCH_NORM ); break;
		}
		CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 256.0f, 0.25f );
	}
	else
		CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 100.0f, 0.1f ); // skittering sound

	m_flNextBounceSoundTime = gpGlobals->time + 0.5f; // Half second.
}
#endif
