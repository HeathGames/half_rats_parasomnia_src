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
// Pig
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "explode.h"
#include "monsters.h"
#include "saverestore.h"
#include "squadmonster.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CPig : public CSquadMonster
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }
	void SetYawSpeed( void );

	// Attack stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void AlertSound( void );
	void AttackSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void PainSound( void );

	// AI stuff
	int IgnoreConditions( void );
	void PrescheduleThink( void );

	// Sounds arrays
	static const char *pAlertSounds[];
	static const char *pAttackSounds[];
	static const char *pDeathSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
private:
	float m_flNextFlinch;
};
LINK_ENTITY_TO_CLASS( monster_pig, CPig );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CPig::m_SaveData[] = 
{
	DEFINE_FIELD( CPig, m_flNextFlinch, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CPig, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CPig::pAlertSounds[] =
{
	"pig/pig_alert1.wav",
	"pig/pig_alert2.wav"
};

const char *CPig::pAttackSounds[] =
{
	"pig/pig_attack1.wav",
	"pig/pig_attack2.wav"
};

const char *CPig::pDeathSounds[] =
{
	"pig/pig_die1.wav",
	"pig/pig_die2.wav"
};

const char *CPig::pIdleSounds[] =
{
	"pig/pig_idle1.wav",
	"pig/pig_idle2.wav",
	"pig/pig_idle3.wav"
};

const char *CPig::pPainSounds[] =
{
	"pig/pig_pain1.wav",
	"pig/pig_pain2.wav",
	"pig/pig_pain3.wav"
};

void CPig::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/pig.mdl" );

	UTIL_SetSize( pev, Vector( -32.0f, -32.0f, 0.0f ), Vector( 32.0f, 32.0f, 64.0f ) );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.pigHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	if ( pev->body == -1 )
		pev->body = RANDOM_LONG( 0, 1 );

	MonsterInit();
}

void CPig::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/pig.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND( (char *)pAlertSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND( (char *)pDeathSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "pig/pig_bite.wav" );
	PRECACHE_SOUND( "pig/pig_explode.wav" );
}

void CPig::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

BOOL CPig::CheckRangeAttack1( float flDot, float flDist )
{
	return pev->health <= (gSkillData.pigHealth / 2.0f) && flDist <= 192.0f && flDot >= 0.3f;
}

void CPig::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case 0x01:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.pigDmgBite, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100.0f;
				}
				EMIT_SOUND_DYN ( ENT( pev ), CHAN_WEAPON, "pig/pig_bite.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		case 0x02:
			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "pig/pig_explode.wav", 1.0f, ATTN_NORM );
			break;
		case 0x03:
			ExplosionCreate( pev->origin, pev->angles, edict(), 100.0f, TRUE );
			TakeDamage( pev, pev, 999.0f, DMG_CRUSH );
			break;
		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

int CPig::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK --- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CPig::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAlertSounds[RANDOM_LONG( 0, ARRAYSIZE( pAlertSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CPig::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CPig::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pDeathSounds[RANDOM_LONG( 0, ARRAYSIZE( pDeathSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CPig::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CPig::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

int CPig::IgnoreConditions( void )
{
	int iIgnore = CSquadMonster::IgnoreConditions();
	if ( m_Activity == ACT_MELEE_ATTACK1 )
	{
		if ( m_flNextFlinch >= gpGlobals->time )
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ( m_Activity == ACT_SMALL_FLINCH || m_Activity == ACT_BIG_FLINCH )
	{
		if ( m_flNextFlinch < gpGlobals->time )
			m_flNextFlinch = gpGlobals->time + 2.0f;
	}
	return iIgnore;
}

void CPig::PrescheduleThink( void )
{
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions( bits_COND_SEE_ENEMY ) )
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time; // Update the squad's last enemy sighting time.
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5.0f )
				MySquadLeader()->m_fEnemyEluded = TRUE; // Been a while since we've seen the enemy
		}
	}
}
