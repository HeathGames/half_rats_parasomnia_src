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
// Maskboy
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "zombie.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CMaskboy : public CZombie
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
	void AlertSound( void );
	void AttackSound( void );
	void IdleSound( void );
	void PainSound( void );

	// Sounds arrays
	static const char *pAlertSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];
};
LINK_ENTITY_TO_CLASS( monster_maskboy, CMaskboy );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CMaskboy::pAlertSounds[] =
{
	"maskboy/mb_alert1.wav",
	"maskboy/mb_alert2.wav",
	"maskboy/mb_alert3.wav"
};

const char *CMaskboy::pAttackHitSounds[] =
{
	"maskboy/mb_hook_hit1.wav",
	"maskboy/mb_hook_hit2.wav",
	"maskboy/mb_hook_hit3.wav"
};

const char *CMaskboy::pAttackMissSounds[] =
{
	"maskboy/mb_hook_miss1.wav",
	"maskboy/mb_hook_miss2.wav"
};

const char *CMaskboy::pAttackSounds[] =
{
	"maskboy/mb_attack1.wav",
	"maskboy/mb_attack2.wav"
};

const char *CMaskboy::pIdleSounds[] =
{
	"maskboy/mb_idle1.wav",
	"maskboy/mb_idle2.wav",
	"maskboy/mb_idle3.wav",
	"maskboy/mb_idle4.wav"
};

const char *CMaskboy::pPainSounds[] =
{
	"maskboy/mb_pain1.wav",
	"maskboy/mb_pain2.wav"
};

void CMaskboy::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/maskboy.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.maskboyHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	MonsterInit();
}

void CMaskboy::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/maskboy.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND( (char *)pAlertSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackHitSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackMissSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );
}

void CMaskboy::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.maskboyDmgSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = -18.0f;
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackHitSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		case ZOMBIE_AE_ATTACK_LEFT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.maskboyDmgSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = 18.0f;
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackHitSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

int CMaskboy::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK --- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CMaskboy::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAlertSounds[RANDOM_LONG( 0, ARRAYSIZE( pAlertSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CMaskboy::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CMaskboy::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CMaskboy::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}
