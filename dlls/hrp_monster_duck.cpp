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
// Duck
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "zombie.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CDuck : public CZombie
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void AlertSound( void ) {};
	void DeathSound( void );
	void IdleSound( void ) {};
	void PainSound( void ) {};
};
LINK_ENTITY_TO_CLASS( monster_duck, CDuck );

void CDuck::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/duck.mdl" );

	UTIL_SetSize( pev, Vector( -32.0f, -32.0f, 0.0f ), Vector( 32.0f, 32.0f, 64.0f ) );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.duckHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	MonsterInit();
}

void CDuck::Precache( void )
{
	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/duck.mdl" );

	PRECACHE_SOUND( "duck/duck_die.wav" );
	PRECACHE_SOUND( "duck/duck_rolling.wav" );
	PRECACHE_SOUND( "duck/duck_stab_hit.wav" );
	PRECACHE_SOUND( "duck/duck_stab_miss.wav" );
}

void CDuck::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.duckDmgSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100.0f;
				}
				EMIT_SOUND_DYN ( ENT( pev ), CHAN_WEAPON, "duck/duck_stab_hit.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "duck/duck_stab_miss.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

int CDuck::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CDuck::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "duck/duck_die.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}
