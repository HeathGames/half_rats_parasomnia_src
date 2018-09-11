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
// Doktor
//=========================================================

// WARNING: The "shoot dart code" might be hardcoded, please forgive me

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "squadmonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define AE_MELEE_RIGHT	1
#define AE_MELEE_LEFT	2
#define AE_MELEE_BOTH	3
#define AE_RANGE		4

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CDoktor : public CSquadMonster
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }
	void SetYawSpeed( void );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Attack stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );
	BOOL CheckRangeAttack2( float flDot, float flDist ) { return FALSE; }

	// Sound stuff
	void AlertSound( void );
	void IdleSound( void );
	void PainSound( void );

	// Squad stuff
	void PrescheduleThink( void );

	// Sounds arrays
	static const char *pAlertSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

private:
	float m_flNextSyringeAttack;
};
LINK_ENTITY_TO_CLASS( monster_doktor, CDoktor );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CDoktor::m_SaveData[] = 
{
	DEFINE_FIELD( CDoktor, m_flNextSyringeAttack, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CDoktor, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CDoktor::pAlertSounds[] =
{
	"doktor/dok_alert1.wav",
	"doktor/dok_alert2.wav"
};

const char *CDoktor::pIdleSounds[] =
{
	"doktor/dok_idle1.wav",
	"doktor/dok_idle2.wav"
};

const char *CDoktor::pPainSounds[] =
{
	"doktor/dok_pain1.wav",
	"doktor/dok_pain2.wav"
};

void CDoktor::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/doktor.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.doktorHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	m_flNextSyringeAttack = gpGlobals->time;

	MonsterInit();
}

void CDoktor::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/doktor.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND( (char *)pAlertSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "doktor/dok_die.wav" );
	PRECACHE_SOUND( "doktor/dok_melee_hit.wav" );
	PRECACHE_SOUND( "doktor/dok_melee_miss.wav" );
	PRECACHE_SOUND( "doktor/dok_syringe.wav" );
	PRECACHE_SOUND( "doktor/dok_wheel.wav" );

	UTIL_PrecacheOther( "doktor_dart" );
}

void CDoktor::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

void CDoktor::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
	case AE_MELEE_RIGHT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.doktorDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = -18.0f;
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_hit.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_miss.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
	case AE_MELEE_LEFT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.doktorDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = 18.0f;
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_hit.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_miss.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
	case AE_MELEE_BOTH:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.doktorDmgBothSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_hit.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_melee_miss.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
	case AE_RANGE:
		{
			Vector vecAngDir, vecArmLeftDir, vecArmLeftPos, vecArmRightDir, vecArmRightPos, vecDirToEnemy;

			if ( HasConditions( bits_COND_SEE_ENEMY ) )
			{
				vecDirToEnemy = (m_hEnemy->pev->origin - pev->origin);
				// Taken from HGrunt throw grenade code, predict enemy movement
				vecDirToEnemy = vecDirToEnemy + ((vecDirToEnemy - pev->origin).Length() / 8192.0f) * m_hEnemy->pev->velocity;
				vecAngDir = UTIL_VecToAngles( vecDirToEnemy );
				vecDirToEnemy = vecDirToEnemy.Normalize();
				vecAngDir.x = -vecAngDir.x; // It seems that pitch is reversed
				vecAngDir.x += 5.0f;
			}
			else
			{
				vecAngDir = pev->angles;
				UTIL_MakeAimVectors( vecAngDir );
				vecDirToEnemy = gpGlobals->v_forward;
			}

			SetBlending( 0, vecAngDir.x );
			GetAttachment( 0, vecArmLeftPos, vecArmLeftDir );
			GetAttachment( 1, vecArmRightPos, vecArmRightDir );

			vecArmLeftPos = vecArmLeftPos + vecDirToEnemy * 32.0f;
			vecArmRightPos = vecArmRightPos + vecDirToEnemy * 32.0f;

			// Without this offset, the player could just stand between the darts without being hurt
			Vector vecOffset = gpGlobals->v_right * 3.0f;
			CBaseEntity *pSyringeLeft = CBaseEntity::Create( "doktor_dart", vecArmLeftPos + vecOffset, vecAngDir, edict() );
			UTIL_MakeVectors( pSyringeLeft->pev->angles );
			CBaseEntity *pSyringeRight = CBaseEntity::Create( "doktor_dart", vecArmRightPos - vecOffset, vecAngDir, edict() );
			UTIL_MakeVectors( pSyringeRight->pev->angles );

			EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "doktor/dok_syringe.wav", 1.0f, ATTN_NORM, 0, 100 );
		}
		break;
	default:
		CSquadMonster::HandleAnimEvent( pEvent );
		break;
	}
}

int CDoktor::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK --- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

BOOL CDoktor::CheckRangeAttack1( float flDot, float flDist )
{
	// Close (but not too much) to my enemy and looking at it
	if ( flDist >= 64.0f && flDist <= 512.0f && flDot >= 0.5f )
	{
		// Time to shoot?
		if ( gpGlobals->time > m_flNextSyringeAttack )
		{
			TraceResult pTr;
			Vector vecShootOrigin = pev->origin + Vector( 0.0f, 0.0f, 16.0f );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector vecShootTarget = (pEnemy->BodyTarget( vecShootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP;
			UTIL_TraceLine( vecShootOrigin, vecShootTarget, dont_ignore_monsters, ENT( pev ), &pTr );
			m_flNextSyringeAttack = gpGlobals->time + 1.0f;
			if ( pTr.flFraction == 1.0f || (pTr.pHit != NULL && CBaseEntity::Instance( pTr.pHit ) == pEnemy) )
				return TRUE;
			else
				return FALSE;
		}
	}

	return FALSE;
}

void CDoktor::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAlertSounds[RANDOM_LONG( 0, ARRAYSIZE( pAlertSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDoktor::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDoktor::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDoktor::PrescheduleThink( void )
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
