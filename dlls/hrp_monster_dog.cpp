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
// Dog
//=========================================================

// Shepard : Dog's jump attack has been disabled, if you can fix
// the animation, then you can re-enable the code

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "game.h"
#include "monsters.h"
#include "saverestore.h"
#include "squadmonster.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CDog : public CSquadMonster
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
#ifdef DOG_JUMP_ATTACK
	BOOL CheckRangeAttack1( float flDot, float flDist );
#else
	BOOL CheckRangeAttack1( float flDot, float flDist ) { return FALSE; };
#endif

	// Sound stuff
	void AlertSound( void );
	void AttackSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void PainSound( void );

	// AI stuff
	int IgnoreConditions( void );
	void PrescheduleThink( void );
#ifdef DOG_JUMP_ATTACK
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	Schedule_t *GetScheduleOfType( int Type );
#endif

	// Sounds arrays
	static const char *pAlertSounds[];
	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

#ifdef DOG_JUMP_ATTACK
	CUSTOM_SCHEDULES;
#endif
private:
	float m_flNextFlinch;
};
LINK_ENTITY_TO_CLASS( monster_dog, CDog );

#ifdef DOG_JUMP_ATTACK
//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t tlDogRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0 },
	{ TASK_FACE_IDEAL,			(float)0 },
	{ TASK_RANGE_ATTACK1,		(float)0 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_IDEAL,			(float)0 },
	{ TASK_WAIT_RANDOM,			(float)0.5 },
};

Schedule_t slDogRangeAttack1[] =
{
	{
		tlDogRangeAttack1,
		ARRAYSIZE( tlDogRangeAttack1 ),
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
	0,
	"DogRangeAttack1"
	},
};

DEFINE_CUSTOM_SCHEDULES( CDog )
{
	slDogRangeAttack1
};
IMPLEMENT_CUSTOM_SCHEDULES( CDog, CSquadMonster );
#endif

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CDog::m_SaveData[] = 
{
	DEFINE_FIELD( CDog, m_flNextFlinch, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CDog, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CDog::pAlertSounds[] =
{
	"dog/dog_alert1.wav",
	"dog/dog_alert2.wav"
};

const char *CDog::pAttackSounds[] =
{
	"dog/dog_attack1.wav",
	"dog/dog_attack2.wav"
};

const char *CDog::pIdleSounds[] =
{
	"dog/dog_idle1.wav",
	"dog/dog_idle2.wav"
};

const char *CDog::pPainSounds[] =
{
	"dog/dog_pain1.wav",
	"dog/dog_pain2.wav"
};

void CDog::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/dog.mdl" );

	UTIL_SetSize( pev, Vector( -32.0f, -32.0f, 0.0f ), Vector( 32.0f, 32.0f, 64.0f ) );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.dogHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	MonsterInit();
}

void CDog::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/dog.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND( (char *)pAlertSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "dog/dog_bite.wav" );
	PRECACHE_SOUND( "dog/dog_die.wav" );
}

void CDog::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

void CDog::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case 0x01:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.dogDmgBite, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100.0f;
				}
				EMIT_SOUND_DYN ( ENT( pev ), CHAN_WEAPON, "dog/dog_bite.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
#ifdef DOG_JUMP_ATTACK
		case 0x02:
		{
			// Make Dog a little bit airbone to prevent "engine reset"
			ClearBits( pev->flags, FL_ONGROUND );
			UTIL_SetOrigin( pev, pev->origin + Vector( 0.0f, 0.0f, 1.0f ) );
			UTIL_MakeVectors( pev->angles );

			// Where should I jump ?
			Vector vecJumpDir;
			if ( m_hEnemy == NULL )
			{
				vecJumpDir = Vector( gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_forward.z ) * 350.0f; // Just straight ahead
				pev->velocity = vecJumpDir;
				m_flNextAttack = gpGlobals->time + 2.0f;
				return;
			}

			float flGravity = g_psv_gravity->value;
			if ( flGravity <= 1.0f )
				flGravity = 1.0f;

			// How fast does Half-Cat need to travel to reach that height given gravity ?
			float flHeight = (m_hEnemy->pev->origin.z + m_hEnemy->pev->view_ofs.z - pev->origin.z);
			if ( flHeight < 16.0f )
				flHeight = 16.0f;

			float flSpeed = sqrt( 2.0f * flGravity * flHeight );
			float flTime = flSpeed / flGravity;

			// Scale the sideways velocity to get there at the right time
			vecJumpDir = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - pev->origin);
			vecJumpDir = vecJumpDir * (1.0f / flTime);

			// Speed to offset gravity at the desired height
			vecJumpDir.z = flSpeed;

			// Don't jump too far/fast
			float flDistance = vecJumpDir.Length();
			if ( flDistance > 650.0f )
				vecJumpDir = vecJumpDir * (650.0f / flDistance);

			pev->velocity = vecJumpDir;
			m_flNextAttack = gpGlobals->time + 2.0f;
		}
		break;
#endif
		default:
			CSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

int CDog::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK --- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

#ifdef DOG_JUMP_ATTACK
BOOL CDog::CheckRangeAttack2( float flDot, float flDist )
{
	if ( FBitSet( pev->flags, FL_ONGROUND ) && flDist <= 256.0f && flDot >= 0.05f )
		return TRUE;

	return FALSE;
}
#endif

void CDog::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAlertSounds[RANDOM_LONG( 0, ARRAYSIZE( pAlertSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDog::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDog::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "dog/dog_die.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDog::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CDog::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

int CDog::IgnoreConditions( void )
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

void CDog::PrescheduleThink( void )
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

#ifdef DOG_JUMP_ATTACK
void CDog::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
		if ( m_fSequenceFinished )
		{
			TaskComplete();
			m_IdealActivity = ACT_IDLE;
		}
		break;
	default:
		CSquadMonster::RunTask( pTask );
	}
}

void CDog::StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
		m_IdealActivity = ACT_RANGE_ATTACK1;
		break;
	default:
		CSquadMonster::StartTask( pTask );
	}
}

Schedule_t *CDog::GetScheduleOfType( int Type )
{
	switch ( Type )
	{
	case SCHED_RANGE_ATTACK1:
		return &slDogRangeAttack1[0];
	default:
		return CSquadMonster::GetScheduleOfType( Type );
	}
}
#endif
