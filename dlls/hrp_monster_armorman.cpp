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
// Armored Fucker
//=========================================================

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
// Monster's Body Group Go Here
//=========================================================
#define BODY_GUN_HAND	0
#define BODY_GUN_NONE	1

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CArmorMan : public CSquadMonster
{
public:
	// Spawn and precache
	void KeyValue(  KeyValueData *pkvd );
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
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );

	// Sound stuff
	void AlertSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void PainSound( void );

	// Schedule stuff
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );

	// Shooter stuff
	void CheckAmmo( void );
	void PrescheduleThink( void );

	// AI stuff
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );

	// Sounds arrays
	static const char *pAlertSounds[];
	static const char *pDeathSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];
	static const char *pStepSounds[];

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
	float m_flNextShieldTime;
	int m_cAmmoLoaded;
};
LINK_ENTITY_TO_CLASS( monster_armorman, CArmorMan );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CArmorMan::m_SaveData[] = 
{
	DEFINE_FIELD( CArmorMan, m_bGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CArmorMan, m_bLastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CArmorMan, m_cAmmoLoaded, FIELD_INTEGER ),
	DEFINE_FIELD( CArmorMan, m_flCheckAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CArmorMan, m_flNextShieldTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CArmorMan, CSquadMonster );

//=========================================================
// Monster's Schedules And Tasks Go Here
//=========================================================
Task_t tlArmorManEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM	},
};

Schedule_t slArmorManEnemyDraw[] = 
{
	{
		tlArmorManEnemyDraw,
		ARRAYSIZE( tlArmorManEnemyDraw ),
		0,
		0,
		"ArmorMan Enemy Draw"
	}
};

Task_t tlArmorManShield[] =
{
	{ TASK_STOP_MOVING,					(float)0			},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_EXCITED	},
	{ TASK_EAT,							(float)0			}
};

Schedule_t slArmorManShield[] =
{
	{
		tlArmorManShield,
		ARRAYSIZE( tlArmorManShield ),
		0,
		0,
		"Armor Man Shield"
	},
};

Task_t tlArmorManReload[] =
{
	{ TASK_STOP_MOVING,				(float)0 },
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD },
	{ TASK_FACE_ENEMY,				(float)0 },
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD },
};

Schedule_t slArmorManReload[] =
{
	{
		tlArmorManReload,
		ARRAYSIZE( tlArmorManReload ),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,

	bits_SOUND_DANGER,
	"ArmorManReload"
	}
};

DEFINE_CUSTOM_SCHEDULES( CArmorMan )
{
	slArmorManEnemyDraw,
	slArmorManShield,
	slArmorManReload
};
IMPLEMENT_CUSTOM_SCHEDULES( CArmorMan, CSquadMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CArmorMan::pAlertSounds[] =
{
	"armorman/am_alert1.wav",
	"armorman/am_alert2.wav",
	"armorman/am_alert3.wav"
};

const char *CArmorMan::pDeathSounds[] =
{
	"armorman/am_die1.wav",
	"armorman/am_die2.wav"
};

const char *CArmorMan::pIdleSounds[] =
{
	"armorman/am_idle1.wav",
	"armorman/am_idle2.wav",
	"armorman/am_idle3.wav"
};

const char *CArmorMan::pPainSounds[] =
{
	"armorman/am_pain1.wav",
	"armorman/am_pain2.wav"
};

const char *CArmorMan::pStepSounds[] =
{
	"armorman/am_step1.wav",
	"armorman/am_step2.wav",
	"armorman/am_step3.wav",
	"armorman/am_step4.wav"
};

void CArmorMan::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "iuser1" ) )
	{
		pev->iuser1 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CSquadMonster::KeyValue( pkvd );
}

void CArmorMan::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/armorman.mdl" );

	UTIL_SetSize( pev, Vector( -24.0f, -24.0f, 0.0f ), Vector( 24.0f, 24.0f, 108.0f ) );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 )
		pev->health		= gSkillData.armormanHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_HEAR | bits_CAP_DOORS_GROUP | bits_CAP_TURN_HEAD;

	if ( pev->iuser1 == -1 )
		pev->iuser1 = RANDOM_LONG( 0, 1 );

	m_bGunDrawn = pev->iuser1 ? TRUE : FALSE;
	m_flNextShieldTime = gpGlobals->time;
	m_cAmmoLoaded = 2;

	MonsterInit();
}

void CArmorMan::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/armorman.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAlertSounds ); i++ )
		PRECACHE_SOUND( (char *)pAlertSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND( (char *)pDeathSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pStepSounds ); i++ )
		PRECACHE_SOUND( (char *)pStepSounds[i] );

	PRECACHE_SOUND( "armorman/am_fire.wav" );
	PRECACHE_SOUND( "weapons/shotgun_reload_close.wav" );
	PRECACHE_SOUND( "weapons/shotgun_reload_insert.wav" );
	PRECACHE_SOUND( "weapons/shotgun_reload_open.wav" );
	PRECACHE_SOUND( "weapons/shotgun_reload_shell_drop.wav" );
}

void CArmorMan::SetYawSpeed( void )
{
	int iYawSpeed = 200;
	if ( m_Activity == ACT_COWER )
		iYawSpeed = 100;

	pev->yaw_speed = iYawSpeed;
}

BOOL CArmorMan::CheckRangeAttack1( float flDot, float flDist )
{
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

void CArmorMan::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case AE_DRAW: // Draw the gun
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

			BOOL m_bBoth = FALSE;
			if ( m_hEnemy )
				m_bBoth = (m_cAmmoLoaded == 2 && (m_hEnemy->pev->origin - pev->origin).Length() <= 256.0f);

			FireBullets( m_bBoth ? 12 : 6, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 1024, BULLET_PLAYER_BUCKSHOT, 0 );

			EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "armorman/am_fire.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
			if ( m_bBoth )
			{
				EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "armorman/am_fire.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM );
				m_cAmmoLoaded = 0;
			}
			else
				m_cAmmoLoaded--;

			CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384.0f, 0.3f );
		}
		break;
		case AE_HOLSTER: // Holster the gun
			m_bGunDrawn = FALSE;
			break;
		case AE_RELOAD: // Reload the gun
			m_cAmmoLoaded = 2;
			ClearConditions( bits_COND_NO_AMMO_LOADED );
			break;
		default: CSquadMonster::HandleAnimEvent( pEvent ); break;
	}
}

void CArmorMan::Killed( entvars_t *pevAttacker, int iGib )
{
	if ( pev->body < BODY_GUN_NONE ) // Drop some ammo
	{
		Vector vecGunPos, vecGunAng;
		pev->body = BODY_GUN_NONE;
		GetAttachment( 0, vecGunPos, vecGunAng );
		vecGunAng.x = vecGunAng.z = 0.0f;
		DropItem( "ammo_buckshot", vecGunPos, vecGunAng );
	}
	CSquadMonster::Killed( pevAttacker, iGib );
}

int CArmorMan::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CArmorMan::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	switch ( ptr->iHitgroup )
	{
	case 1:
		if ( bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB) )
		{
			UTIL_Ricochet( ptr->vecEndPos, 1.0f );
			flDamage = 0.01f;
		}
		ptr->iHitgroup = HITGROUP_LEFTARM;
		break;
	}
	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CArmorMan::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAlertSounds[RANDOM_LONG( 0, ARRAYSIZE( pAlertSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CArmorMan::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pDeathSounds[RANDOM_LONG( 0, ARRAYSIZE( pDeathSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CArmorMan::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CArmorMan::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

Schedule_t *CArmorMan::GetSchedule( void )
{
	switch ( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		if ( !m_bGunDrawn )
			return GetScheduleOfType( SCHED_ARM_WEAPON );

		if ( m_hEnemy != NULL && !HasConditions( bits_COND_NO_AMMO_LOADED ) && pev->health <= (gSkillData.armormanHealth / 2.0f) && gpGlobals->time > m_flNextShieldTime )
			return GetScheduleOfType( SCHED_COWER );

		if ( HasConditions( bits_COND_NO_AMMO_LOADED ) )
			return GetScheduleOfType( SCHED_RELOAD );

		if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
	}
	return CSquadMonster::GetSchedule();
}

Schedule_t *CArmorMan::GetScheduleOfType( int Type )
{
	switch ( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
			return slArmorManEnemyDraw;
		break;
	case SCHED_COWER:
		return slArmorManShield;
		break;
	case SCHED_RELOAD:
		return slArmorManReload;
		break;
	}
	return CSquadMonster::GetScheduleOfType( Type );
}

void CArmorMan::CheckAmmo( void )
{
	if ( m_cAmmoLoaded < 1 )
		SetConditions( bits_COND_NO_AMMO_LOADED );
}

void CArmorMan::PrescheduleThink( void )
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

void CArmorMan::RunTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;
	case TASK_EAT:
		TaskComplete();
		break;
	default:
		CSquadMonster::RunTask( pTask );
		break;
	}
}

void CArmorMan::StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_EAT:
		m_flNextShieldTime = gpGlobals->time + 15.0f;
		break;
	default:
		CSquadMonster::StartTask( pTask );
		break;
	}
}
