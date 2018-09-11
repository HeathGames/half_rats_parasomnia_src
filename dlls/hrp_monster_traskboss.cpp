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
// Traskboss
//=========================================================

// Shepard : A little bit of story here: if you think Traskboss
// is hardcoded, then yes it is. We were 2 weeks from release
// to finish all the programming and level design tasks. Traskboss
// didn't existed and those 2 weekends were busy, so don't expect
// clean code or clean methods of doing stuff.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_entity_bernard_puke.h"
#include "hrp_monster_base_shooter.h"
#include "monsters.h"
#include "saverestore.h"
#include "soundent.h"
#include "weapons.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CTraskboss : public CBaseMonster
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }
	void SetYawSpeed( void );

	// Attack stuff
	BOOL CheckMeleeAttack1( float flDot, float flDist );
	BOOL CheckMeleeAttack2( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack1( float flDot, float flDist );
	BOOL CheckRangeAttack2( float flDot, float flDist ) { return FALSE; }

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Killed( entvars_t *pevAttacker, int iGib );
	void EXPORT RealKill( void ); // This is needed to actually kill Traskboss
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ); // Because trigger_hurt failed us

	// Sound stuff
	void DeathSound( void );

	// Schedule stuff
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );

	// AI stuff
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;
private:
	BOOL m_bLastAttackCheck;
	float m_flCheckAttackTime;
};
LINK_ENTITY_TO_CLASS( monster_traskboss, CTraskboss );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CTraskboss::m_SaveData[] = 
{
	DEFINE_FIELD( CTraskboss, m_bLastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CTraskboss, m_flCheckAttackTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CTraskboss, CBaseMonster );

//=========================================================
// Monster's Schedules And Tasks Go Here
//=========================================================
Task_t tlTraskbossHurt[] =
{
	{ TASK_STOP_MOVING,					(float)0 },
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_SMALL_FLINCH },
	{ TASK_EAT,							(float)0 }
};

Schedule_t slTraskbossHurt[] =
{
	{
		tlTraskbossHurt,
		ARRAYSIZE( tlTraskbossHurt ),
		0,
		0,
		"Traskboss Hurt"
	},
};

DEFINE_CUSTOM_SCHEDULES( CTraskboss )
{
	slTraskbossHurt
};
IMPLEMENT_CUSTOM_SCHEDULES( CTraskboss, CBaseMonster );

void CTraskboss::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/traskboss.mdl" );

	//UTIL_SetSize( pev, Vector( -200.0f, -400.0f, -352.0f ), Vector( 200.0f, 400.0f, 352.0f ) );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_FLY;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 )
		pev->health		= 1;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= VIEW_FIELD_FULL;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_HEAR;

	MonsterInit();
}

void CTraskboss::Precache( void )
{
	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/traskboss.mdl" );

	PRECACHE_SOUND( "bernard/bern_puke.wav" );
	PRECACHE_SOUND( "traskboss/tboss_die.wav" );
	PRECACHE_SOUND( "traskboss/tboss_pain.wav" );

	UTIL_PrecacheOther( "bernard_puke" );
}

void CTraskboss::SetYawSpeed( void )
{
	pev->yaw_speed = 512;
}

// BUG: This doesn't works
BOOL CTraskboss::CheckMeleeAttack1( float flDot, float flDist )
{
	if ( flDist <= 192.0f && flDot >= 0.7f && m_hEnemy != NULL )
		return TRUE;

	return FALSE;
}

BOOL CTraskboss::CheckRangeAttack1( float flDot, float flDist )
{
	// Looking at the enemy
	if ( flDot >= 0.5f )
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

void CTraskboss::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		// BUG: Doesn't work
		case 2: // Melee attack
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 192.0f, gSkillData.zombieDmgBothSlash, DMG_CLUB );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 10.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
				}
				// Insert melee hit sound here
			}
			/*else
				// Insert melee miss sound here */

			//AttackSound();
		}
		break;
		// Ranged attack, just a modified version of Bernard's puke
		case AE_SHOOT:
		{
			Vector vecPukeOrg;
			Vector vecUseless;

			// Where the puke should start from ?
			GetAttachment( 0, vecPukeOrg, vecUseless );
			vecPukeOrg = vecPukeOrg + gpGlobals->v_forward * 32.0f;

			// Where should the puke hit to ?
			Vector vecPukeDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - vecPukeOrg).Normalize();
			vecPukeDir = vecPukeDir + Vector( 0.0f, 0.0f, 0.075f ); // Without this offset, Traskboss would always miss his target

			// Taken from HGrunt's throw grenade code, predict enemy movement
			// This works perfectly on the test map but never worked on "para13"
			//vecPukeDir = vecPukeDir + ((vecPukeDir - pev->origin).Length() / 2048.0f) * m_hEnemy->pev->velocity;

			// Shoot it
			CBernardPuke::Shoot( pev, vecPukeOrg, vecPukeDir * 2048.0f );
			EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bernard/bern_puke.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
		default: CBaseMonster::HandleAnimEvent( pEvent ); break;
	}
}

int CTraskboss::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( !(bitsDamageType & DMG_POISON) )
		flDamage = 0.00f;
	else
	{
		SetActivity( ACT_SMALL_FLINCH );
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "traskboss/tboss_pain.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
	}

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CTraskboss::Killed( entvars_t *pevAttacker, int iGib )
{
	// Taken from Barnacle's code, "Killed" doesn't really kill Traskboss, this is why "RealKill" is here
	SetActivity( ACT_DIESIMPLE );
	SetThink( &CTraskboss::RealKill );
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CTraskboss::RealKill( void )
{
	// Kill Traskboss when he finishes his death animation
	if ( m_fSequenceFinished )
		UTIL_Remove( this );

	pev->nextthink = gpGlobals->time + 0.1f;
}

void CTraskboss::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// A little bit of explanation here, our original plan was to use a "trigger_hurt" with DMG_POISON 
	// that would be toggled on/off, for an unknown reason, it never worked, when mapping techniques
	// fails, this is where code intervene. This could cause bugs if Traskboss is used by "scripted_sequence"
	// but we don't have time to test this.
	TakeDamage( pCaller->pev, pActivator->pev, 1.0f, DMG_POISON );
}

void CTraskboss::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "traskboss/tboss_die.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

Schedule_t *CTraskboss::GetSchedule( void )
{
	return (m_Activity == ACT_SMALL_FLINCH) ? GetScheduleOfType( SCHED_COWER ) : CBaseMonster::GetSchedule();
}

Schedule_t *CTraskboss::GetScheduleOfType( int Type )
{
	return (Type == SCHED_COWER) ? slTraskbossHurt : CBaseMonster::GetScheduleOfType( Type );
}

void CTraskboss::RunTask( Task_t *pTask )
{
	if ( pTask->iTask == TASK_EAT )
	{
		m_IdealActivity = ACT_IDLE;
		TaskComplete();
	}
	else
		CBaseMonster::RunTask( pTask );
}

void CTraskboss::StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_EAT:
		// Just do nothing
		break;
	default:
		CBaseMonster::StartTask( pTask );
		break;
	}
}
