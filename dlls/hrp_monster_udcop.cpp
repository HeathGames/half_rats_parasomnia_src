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
// UD Cop
//=========================================================

// FIXME : Cops with the Colt SAA should use the Colt reload animation, not the Henry one!

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_monster_base_shooter.h"
#include "animation.h"
#include "monsters.h"
#include "saverestore.h"
#include "soundent.h"
#include "squadmonster.h"
#include "weapons.h"

//=========================================================
// Monster's Body Group Go Here
//=========================================================
#define BODY_GUN_SAFE_COLT		0
#define BODY_GUN_HAND_COLT		1
#define BODY_GUN_HAND_HENRY		2
#define BODY_GUN_NONE			3

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CUDCop : public CSquadMonster
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
	BOOL CheckRangeAttack2( float flDot, float flDist );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );
	void SetActivity( Activity NewActivity );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void DeathSound( void );

	// Schedule stuff
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );

	// Shooter stuff
	void CheckAmmo( void );
	void PrescheduleThink( void );

	// Sounds arrays
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pDeathSounds[];

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
	int m_cAmmoLoaded;
};
LINK_ENTITY_TO_CLASS( monster_udcop, CUDCop );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CUDCop::m_SaveData[] = 
{
	DEFINE_FIELD( CUDCop, m_bGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CUDCop, m_bLastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CUDCop, m_cAmmoLoaded, FIELD_INTEGER ),
	DEFINE_FIELD( CUDCop, m_flCheckAttackTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CUDCop, CSquadMonster );

//=========================================================
// Monster's Schedules And Tasks Go Here
//=========================================================
Task_t tlUDCopEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM	},
};

Schedule_t slUDCopEnemyDraw[] = 
{
	{
		tlUDCopEnemyDraw,
		ARRAYSIZE( tlUDCopEnemyDraw ),
		0,
		0,
		"UDCop Enemy Draw"
	}
};

DEFINE_CUSTOM_SCHEDULES( CUDCop )
{
	slUDCopEnemyDraw
};
IMPLEMENT_CUSTOM_SCHEDULES( CUDCop, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CUDCop::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CUDCop::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CUDCop::pDeathSounds[] =
{
	"udcop/cop_die1.wav",
	"udcop/cop_die2.wav",
	"udcop/cop_die3.wav"
};

void CUDCop::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "iuser1" ) )
	{
		pev->iuser1 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "iuser2" ) )
	{
		pev->iuser2 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CSquadMonster::KeyValue( pkvd );
}

void CUDCop::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/udcop.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.udcopHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_HEAR | bits_CAP_DOORS_GROUP | bits_CAP_TURN_HEAD;

	if ( pev->iuser1 == -1 )
		pev->iuser1 = RANDOM_LONG( 0, 2 );

	if ( pev->iuser2 == 1 )
	{
		m_bGunDrawn = TRUE;
		m_cAmmoLoaded = 15;
		pev->body = 2;
	}
	else
	{
		m_bGunDrawn = pev->iuser1 > 0;
		m_cAmmoLoaded = 6;
		pev->body = m_bGunDrawn;
	}

	MonsterInit();
}

void CUDCop::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/udcop.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackHitSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackMissSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND( (char *)pDeathSounds[i] );

	PRECACHE_SOUND( "udcop/cop_fire_colt.wav" );
	PRECACHE_SOUND( "udcop/cop_fire_henry.wav" );
}

void CUDCop::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

BOOL CUDCop::CheckRangeAttack1( float flDot, float flDist )
{
	// Gun is empty? Reload it
	if ( m_cAmmoLoaded < 1 )
	{
		m_flCheckAttackTime = gpGlobals->time + 3.0f;
		m_IdealActivity = ACT_RELOAD;
		return FALSE;
	}

	// Close to my enemy and looking at it
	if ( flDist >= 64.0f && flDist <= 4096.0f && flDot >= 0.5f )
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

BOOL CUDCop::CheckRangeAttack2( float flDot, float flDist )
{
	// Gun is empty? Reload it
	if ( m_cAmmoLoaded < 1 )
	{
		m_flCheckAttackTime = gpGlobals->time + 3.0f;
		m_IdealActivity = ACT_EAT;
		return FALSE;
	}

	// Close to my enemy and looking at it
	if ( flDist >= 64.0f && flDist <= 8192.0f && flDot >= 0.5f )
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

void CUDCop::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case AE_DRAW: // Draw the gun (Colt SAA only)
			pev->body = BODY_GUN_HAND_COLT;
			m_bGunDrawn = TRUE;
			break;
		case AE_SHOOT: // Shoot the gun
		{
			Vector vecShootOrigin;

			UTIL_MakeVectors( pev->angles );
			vecShootOrigin = pev->origin + Vector( 0.0f, 0.0f, 55.0f );
			Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

			Vector vecAngDir = UTIL_VecToAngles( vecShootDir );
			SetBlending( 0, vecAngDir.x );
			pev->effects = EF_MUZZLEFLASH;

			if ( pev->iuser2 == 1 )
			{
				FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 8192, BULLET_MONSTER_HENRY );
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "udcop/cop_fire_henry.wav", 1.0f, ATTN_NORM );
			}
			else
			{
				FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_4DEGREES, 4096, BULLET_MONSTER_COLT );
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "udcop/cop_fire_colt.wav", 1.0f, ATTN_NORM );
			}

			CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384.0f, 0.3f );
			m_cAmmoLoaded--;
		}
		break;
		case AE_HOLSTER: // Holster the gun (Colt SAA only)
			pev->body = BODY_GUN_SAFE_COLT;
			m_bGunDrawn = FALSE;
			break;
		case AE_RELOAD: // Reload the gun
			m_cAmmoLoaded = pev->iuser2 == 1 ? 15 : 6;
			ClearConditions( bits_COND_NO_AMMO_LOADED );
			break;
		case 6:
		{
			// do stuff for this event.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.udcopDmgKick, DMG_CLUB );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER|FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100;
				}
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			}
			else
				EMIT_SOUND_DYN ( ENT(pev), CHAN_WEAPON, pAttackMissSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackMissSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
		}
		break;
		default: CSquadMonster::HandleAnimEvent( pEvent ); break;
	}
}

void CUDCop::Killed( entvars_t *pevAttacker, int iGib )
{
	if ( pev->body < BODY_GUN_NONE ) // Drop some ammo
	{
		Vector vecGunPos, vecGunAng;
		pev->body = BODY_GUN_NONE;
		GetAttachment( 0, vecGunPos, vecGunAng );
		vecGunAng.x = vecGunAng.z = 0.0f;
		DropItem( "ammo_45", vecGunPos, vecGunAng );
	}

	CSquadMonster::Killed( pevAttacker, iGib );
}

void CUDCop::SetActivity( Activity NewActivity )
{
	if ( NewActivity == ACT_EAT )
	{
		int iSequence = ACTIVITY_NOT_AVAILABLE;
		if ( pev->iuser2 == 1 )
			iSequence = LookupSequence( "henryreload" );
		else
			iSequence = LookupSequence( "reload" );

		// Set to the desired anim, or default anim if the desired is not present
		if ( iSequence > ACTIVITY_NOT_AVAILABLE )
		{
			if ( pev->sequence != iSequence || !m_fSequenceLoops )
			{
				// don't reset frame between walk and run
				if ( !(m_Activity == ACT_WALK || m_Activity == ACT_RUN) || !(NewActivity == ACT_WALK || NewActivity == ACT_RUN))
					pev->frame = 0;
			}

			pev->sequence = iSequence;	// Set to the reset anim (if it's there)
			ResetSequenceInfo();
			SetYawSpeed();
		}
		else
		{
			// Not available try to get default anim
			ALERT ( at_aiconsole, "%s has no sequence for act:%d\n", STRING( pev->classname ), NewActivity );
			pev->sequence = 0;	// Set to the reset anim (if it's there)
		}

		m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

		// In case someone calls this with something other than the ideal activity
		m_IdealActivity = m_Activity;
	}
	else
		CSquadMonster::SetActivity( NewActivity );
}

int CUDCop::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CUDCop::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pDeathSounds[RANDOM_LONG( 0, ARRAYSIZE( pDeathSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

Schedule_t *CUDCop::GetSchedule( void )
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

Schedule_t *CUDCop::GetScheduleOfType( int Type )
{
	if ( Type == SCHED_ARM_WEAPON )
	{
		if ( m_hEnemy != NULL )
			return slUDCopEnemyDraw;
	}
	return CSquadMonster::GetScheduleOfType( Type );
}

void CUDCop::CheckAmmo( void )
{
	if ( m_cAmmoLoaded < 1 )
		SetConditions( bits_COND_NO_AMMO_LOADED );
}

void CUDCop::PrescheduleThink( void )
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
