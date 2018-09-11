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
// Huntsman
//=========================================================

// WARNING: The "shoot bolt code" might be hardcoded, please forgive me

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_monster_base_shooter.h"
#include "monsters.h"
#include "saverestore.h"
#include "soundent.h"
#include "squadmonster.h"
#include "weapons.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define AE_MELEE 6

//=========================================================
// Monster's Body Group Go Here
//=========================================================
#define BODY_GUN_SAFE	0
#define BODY_GUN_HAND	1
#define BODY_GUN_NONE	2

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CHuntsman : public CSquadMonster
{
public:
	// Spawn and precache
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }
	void SetYawSpeed( void );

	// Attack stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void AttackSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void PainSound( void );

	// Schedule stuff
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );
	void RunTask( Task_t *pTask );

	// Shooter stuff
	void CheckAmmo( void );
	void PrescheduleThink( void );

	// Sounds arrays
	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	// Define custom schedule
	CUSTOM_SCHEDULES;
private:
	BOOL m_bGunDrawn;
	BOOL m_bLastAttackCheck;
	float m_flCheckAttackTime;
	float m_flNextRangeAttackTime;
	int m_cAmmoLoaded;
};
LINK_ENTITY_TO_CLASS( monster_huntsman, CHuntsman );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CHuntsman::m_SaveData[] = 
{
	DEFINE_FIELD( CHuntsman, m_bGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHuntsman, m_bLastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHuntsman, m_flCheckAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CHuntsman, m_flNextRangeAttackTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CHuntsman, CSquadMonster );

//=========================================================
// Monster's Schedules And Tasks Go Here
//=========================================================
Task_t tlHuntsmanEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM	},
};

Schedule_t slHuntsmanEnemyDraw[] = 
{
	{
		tlHuntsmanEnemyDraw,
		ARRAYSIZE( tlHuntsmanEnemyDraw ),
		0,
		0,
		"Huntsman Enemy Draw"
	}
};

DEFINE_CUSTOM_SCHEDULES( CHuntsman )
{
	slHuntsmanEnemyDraw
};
IMPLEMENT_CUSTOM_SCHEDULES( CHuntsman, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CHuntsman::pAttackSounds[] =
{
	"huntsman/hunt_atk1.wav",
	"huntsman/hunt_atk2.wav"
};

const char *CHuntsman::pIdleSounds[] =
{
	"huntsman/hunt_idle1.wav",
	"huntsman/hunt_idle2.wav",
	"huntsman/hunt_idle3.wav"
};

const char *CHuntsman::pPainSounds[] =
{
	"huntsman/hunt_pain1.wav",
	"huntsman/hunt_pain2.wav"
};

void CHuntsman::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "iuser1" ) )
	{
		pev->iuser1 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CSquadMonster::KeyValue( pkvd );
}

void CHuntsman::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/huntsman.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 )
		pev->health		= gSkillData.huntsmanHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_HEAR | bits_CAP_DOORS_GROUP | bits_CAP_TURN_HEAD;

	if ( pev->iuser1 == -1 )
		pev->iuser1 = RANDOM_LONG( 0, 1 );

	if ( pev->iuser1 )
	{
		pev->body = BODY_GUN_HAND;
		m_bGunDrawn = TRUE;
	}
	else
	{
		pev->body = BODY_GUN_SAFE;
		m_bGunDrawn = FALSE;
	}

	m_cAmmoLoaded = 1;
	m_flNextRangeAttackTime = gpGlobals->time;

	MonsterInit();
}

void CHuntsman::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/huntsman.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "huntsman/hunt_bolt_hit.wav" );
	PRECACHE_SOUND( "huntsman/hunt_bolt_hit_body.wav" );
	PRECACHE_SOUND( "huntsman/hunt_bow_fire.wav" );
	PRECACHE_SOUND( "huntsman/hunt_die.wav" );
	PRECACHE_SOUND( "huntsman/hunt_hit.wav" );
	PRECACHE_SOUND( "huntsman/hunt_swing.wav" );

	UTIL_PrecacheOther( "huntsman_bolt" );
}

void CHuntsman::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

BOOL CHuntsman::CheckRangeAttack1( float flDot, float flDist )
{
	// Gun is empty? Reload it
	if ( m_cAmmoLoaded < 1 )
	{
		m_flCheckAttackTime = gpGlobals->time + 3.0f;
		m_IdealActivity = ACT_RELOAD;
		return FALSE;
	}

	// Too soon to perform a range attack
	if ( gpGlobals->time <= m_flNextRangeAttackTime )
		return FALSE;

	// Close to my enemy and looking at it
	if ( flDist <= 1024.0f && flDot >= 0.5f )
	{
		// Too soon to check
		if ( gpGlobals->time > m_flCheckAttackTime )
		{
			TraceResult pTr;
			Vector vecShootOrigin = pev->origin + Vector( 0.0f, 0.0f, 55.0f );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector vecShootTarget = (pEnemy->BodyTarget( vecShootOrigin ) - pEnemy->pev->origin) + m_vecEnemyLKP;
			UTIL_TraceLine( vecShootOrigin, vecShootTarget, dont_ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f || (pTr.pHit != NULL && CBaseEntity::Instance( pTr.pHit ) == pEnemy) )
				m_bLastAttackCheck = TRUE;
			else
				m_bLastAttackCheck = FALSE;

			m_flCheckAttackTime = gpGlobals->time + 1.5f;
		}
		return m_bLastAttackCheck;
	}
	return FALSE;
}

void CHuntsman::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case AE_DRAW: // Draw the gun
			pev->body = BODY_GUN_HAND;
			m_bGunDrawn = TRUE;
			break;
		case AE_SHOOT: // Shoot the gun
		{
			Vector vecAngDir, vecArmDir, vecArmPos, vecDirToEnemy;

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
			GetAttachment( 0, vecArmPos, vecArmDir );

			vecArmPos = vecArmPos + vecDirToEnemy * 32.0f;

			CBaseEntity *pBolt = CBaseEntity::Create( "huntsman_bolt", vecArmPos, vecAngDir, edict() );
			UTIL_MakeVectors( pBolt->pev->angles );

			EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "huntsman/hunt_bow_fire.wav", 1.0f, ATTN_NORM, 0, 100 );
			m_cAmmoLoaded--;
			m_flNextRangeAttackTime = gpGlobals->time + 10.0f;
		}
		break;
		case AE_HOLSTER: // Holster the gun
			pev->body = BODY_GUN_SAFE;
			m_bGunDrawn = FALSE;
			break;
		case AE_RELOAD: // Reload the gun
			m_cAmmoLoaded = 1;
			ClearConditions( bits_COND_NO_AMMO_LOADED );
			break;
		case AE_MELEE:
		{
			// do stuff for this event.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.huntsmanDmgMelee, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "huntsman/hunt_hit.wav", 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "huntsman/hunt_swing.wav", 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		default: CSquadMonster::HandleAnimEvent( pEvent ); break;
	}
}

void CHuntsman::Killed( entvars_t *pevAttacker, int iGib )
{
	if ( pev->body < BODY_GUN_NONE ) // Drop some ammo
		pev->body = BODY_GUN_NONE;

	CSquadMonster::Killed( pevAttacker, iGib );
}

int CHuntsman::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CHuntsman::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CHuntsman::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "huntsman/hunt_die.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CHuntsman::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CHuntsman::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

Schedule_t *CHuntsman::GetSchedule( void )
{
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		if ( !m_bGunDrawn )
			return GetScheduleOfType( SCHED_ARM_WEAPON );

		if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
	}
	return CSquadMonster::GetSchedule();
}

Schedule_t *CHuntsman::GetScheduleOfType( int Type )
{
	if ( Type == SCHED_ARM_WEAPON )
	{
		if ( m_hEnemy != NULL )
			return slHuntsmanEnemyDraw;
	}
	return CSquadMonster::GetScheduleOfType( Type );
}

void CHuntsman::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1_NOTURN:
	case TASK_MELEE_ATTACK2_NOTURN:
	case TASK_MELEE_ATTACK1:
	case TASK_MELEE_ATTACK2:
	{
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );

		if ( m_hEnemy != NULL )
			WALK_MOVE( ENT( pev ), pev->ideal_yaw, m_flGroundSpeed * (gpGlobals->time - pev->animtime), WALKMOVE_NORMAL );

		if ( m_fSequenceFinished )
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		break;
	}
	default: CBaseMonster::RunTask( pTask );
	}
}

void CHuntsman::CheckAmmo( void )
{
	if ( m_cAmmoLoaded < 1 )
		SetConditions( bits_COND_NO_AMMO_LOADED );
}

void CHuntsman::PrescheduleThink( void )
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
