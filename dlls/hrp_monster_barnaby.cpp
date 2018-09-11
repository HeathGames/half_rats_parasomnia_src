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
// Barnaby
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "defaultai.h"
#include "hrp_monster_base_shooter.h"
#include "monsters.h"
#include "saverestore.h"
#include "soundent.h"
#include "talkmonster.h"
#include "weapons.h"

//=========================================================
// Monster's Body Group Go Here
//=========================================================
#define BODY_GUN_SAFE	0
#define BODY_GUN_HAND	1
#define BODY_GUN_NONE	2

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CBarnaby : public CTalkMonster
{
public:
	// Spawn and precache
	void KeyValue( KeyValueData *pkvd );
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_PLAYER_ALLY; }
	virtual int	ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void SetYawSpeed( void );

	// Attack stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void DeathSound( void );
	void PainSound( void );

	// Schedule stuff
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );

	// Shooter stuff
	void CheckAmmo( void );

	// Sounds arrays
	static const char *pDeathSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	// Follow stuff
	void DeclineFollowing( void );
	void TalkInit( void );

	// Define custom schedule
	CUSTOM_SCHEDULES;
private:
	BOOL m_bGunDrawn;
	BOOL m_bLastAttackCheck;
	float m_flCheckAttackTime;
	float m_flPainTime;
	int m_cAmmoLoaded;
};
LINK_ENTITY_TO_CLASS( monster_barnaby, CBarnaby );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CBarnaby::m_SaveData[] = 
{
	DEFINE_FIELD( CBarnaby, m_bGunDrawn, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnaby, m_bLastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnaby, m_flCheckAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CBarnaby, m_flPainTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CBarnaby, CTalkMonster );

//=========================================================
// Monster's Schedules And Tasks Go Here
//=========================================================
Task_t tlBarnabyEnemyDraw[] =
{
	{ TASK_STOP_MOVING,					0				},
	{ TASK_FACE_ENEMY,					0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_ARM	},
};

Schedule_t slBarnabyEnemyDraw[] = 
{
	{
		tlBarnabyEnemyDraw,
		ARRAYSIZE( tlBarnabyEnemyDraw ),
		0,
		0,
		"Barnaby Enemy Draw"
	}
};

Task_t tlBarnabyFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		}, // Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t slBarnabyFollow[] =
{
	{
		tlBarnabyFollow,
		ARRAYSIZE( tlBarnabyFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"BarnabyFollow"
	},
};

Task_t tlBarnabyFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t slBarnabyFaceTarget[] =
{
	{
		tlBarnabyFaceTarget,
		ARRAYSIZE( tlBarnabyFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"BarnabyFaceTarget"
	},
};

Task_t tlIdleBarnabyStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // Repick IDLESTAND every two seconds.
	{ TASK_TLK_HEADRESET,		(float)0		}, // Reset head position
};

Schedule_t slIdleBarnabyStand[] =
{
	{ 
		tlIdleBarnabyStand,
		ARRAYSIZE( tlIdleBarnabyStand ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		| // Sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			| // Scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"BarnabyIdleStand"
	},
};

DEFINE_CUSTOM_SCHEDULES( CBarnaby )
{
	slBarnabyEnemyDraw,
	slBarnabyFollow,
	slBarnabyFaceTarget,
	slIdleBarnabyStand
};
IMPLEMENT_CUSTOM_SCHEDULES( CBarnaby, CTalkMonster );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CBarnaby::pDeathSounds[] =
{
	"barnaby/barn_die1.wav",
	"barnaby/barn_die2.wav",
	"barnaby/barn_die3.wav"
};

const char *CBarnaby::pPainSounds[] =
{
	"barnaby/barn_pain1.wav",
	"barnaby/barn_pain2.wav",
	"barnaby/barn_pain3.wav"
};

void CBarnaby::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "iuser1" ) )
	{
		pev->iuser1 = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CTalkMonster::KeyValue( pkvd );
}

void CBarnaby::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/barnaby.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 )
		pev->health		= 100;

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

	m_cAmmoLoaded = 6;

	MonsterInit();
	SetUse( &CBarnaby::FollowerUse );
}

void CBarnaby::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/barnaby.mdl" );

	for ( i = 0; i < ARRAYSIZE( pDeathSounds ); i++ )
		PRECACHE_SOUND( (char *)pDeathSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "barnaby/barn_fire.wav" );

	// Every new Barnaby must call this, otherwise when a level is loaded
	// nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CBarnaby::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

BOOL CBarnaby::CheckRangeAttack1( float flDot, float flDist )
{
	// Gun is empty? Reload it
	if ( m_cAmmoLoaded < 1 )
	{
		m_flCheckAttackTime = gpGlobals->time + 3.0f;
		m_IdealActivity = ACT_RELOAD;
		return FALSE;
	}

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

void CBarnaby::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case AE_DRAW: // Draw the gun
			pev->body = BODY_GUN_HAND;
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

			FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_COLT );

			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "barnaby/barn_fire.wav", 1.0f, ATTN_NORM );
			CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384.0f, 0.3f );
			m_cAmmoLoaded--;
		}
		break;
		case AE_HOLSTER: // Holster the gun
			pev->body = BODY_GUN_SAFE;
			m_bGunDrawn = FALSE;
			break;
		case AE_RELOAD: // Reload the gun
			m_cAmmoLoaded = 6;
			ClearConditions( bits_COND_NO_AMMO_LOADED );
			break;
		default: CTalkMonster::HandleAnimEvent( pEvent ); break;
	}
}

void CBarnaby::Killed( entvars_t *pevAttacker, int iGib )
{
	if ( pev->body < BODY_GUN_NONE ) // Drop some ammo
	{
		Vector vecGunPos, vecGunAng;
		pev->body = BODY_GUN_NONE;
		GetAttachment( 0, vecGunPos, vecGunAng );
		vecGunAng.x = vecGunAng.z = 0.0f;
		DropItem( "ammo_45", vecGunPos, vecGunAng );
	}

	SetUse( NULL );
	CTalkMonster::Killed( pevAttacker, iGib );
}

int CBarnaby::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	int iResult = CTalkMonster::TakeDamage( pevInflictor, pevAttacker, 0.0f, bitsDamageType );
	if ( !IsAlive() || pev->deadflag == DEAD_DYING )
		return iResult;

	if ( m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) )
	{
		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if ( m_hEnemy == NULL )
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if ( (m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing( pevAttacker, pev->origin ) )
			{
				// Alright, now I'm pissed!
				PlaySentence( "BA_MAD", 4.0f, VOL_NORM, ATTN_NORM );
				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
			}
			else
			{
				// Hey, be careful with that
				PlaySentence( "BA_SHOT", 4, VOL_NORM, ATTN_NORM );
				Remember( bits_MEMORY_SUSPICIOUS );
			}
		}
		else if ( !(m_hEnemy->IsPlayer()) && pev->deadflag == DEAD_NO )
			PlaySentence( "BA_SHOT", 4.0f, VOL_NORM, ATTN_NORM );
	}
	return iResult;
}

void CBarnaby::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pDeathSounds[RANDOM_LONG( 0, ARRAYSIZE( pDeathSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CBarnaby::PainSound( void )
{
	if ( gpGlobals->time < m_flPainTime )
		return;

	m_flPainTime = gpGlobals->time + RANDOM_FLOAT( 0.5f, 0.75f );
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

Schedule_t *CBarnaby::GetSchedule( void )
{
	// Take cover from danger
	if ( HasConditions( bits_COND_HEAR_SOUND ) )
	{
		CSound *pSound;
		pSound = PBestSound();
		ASSERT( pSound != NULL );
		if ( pSound && (pSound->m_iType & bits_SOUND_DANGER) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
	}
	// Be happy about killing an enemy
	if ( HasConditions( bits_COND_ENEMY_DEAD ) && FOkToSpeak() )
		PlaySentence( "BA_KILL", 4.0f, VOL_NORM, ATTN_NORM );

	switch ( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		// Call base class, all code to handle dead enemies is centralized there.
		if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			return CBaseMonster :: GetSchedule();

		// Always act surprized with a new enemy
		if ( HasConditions( bits_COND_NEW_ENEMY ) && HasConditions( bits_COND_LIGHT_DAMAGE) )
			return GetScheduleOfType( SCHED_SMALL_FLINCH );

		if ( !m_bGunDrawn )
			return GetScheduleOfType( SCHED_ARM_WEAPON );

		if ( HasConditions( bits_COND_HEAVY_DAMAGE ) )
			return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
		break;
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		// Flinch if hurt
		if ( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
			return GetScheduleOfType( SCHED_SMALL_FLINCH );

		if ( m_hEnemy == NULL && IsFollowing() )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing( FALSE );
				break;
			}
			else
			{
				if ( HasConditions( bits_COND_CLIENT_PUSH ) )
					return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );

				return GetScheduleOfType( SCHED_TARGET_FACE );
			}
		}

		if ( HasConditions( bits_COND_CLIENT_PUSH ) )
			return GetScheduleOfType( SCHED_MOVE_AWAY );

		// Try to say something about smells
		TrySmellTalk();
		break;
	}
	return CTalkMonster::GetSchedule();
}

Schedule_t *CBarnaby::GetScheduleOfType( int Type )
{
	Schedule_t *pSched;

	switch ( Type )
	{
	case SCHED_ARM_WEAPON:
		if ( m_hEnemy != NULL )
			return slBarnabyEnemyDraw;
		break;
	case SCHED_TARGET_FACE:
		// Call base class so that Barnaby will talk when "used"
		pSched = CTalkMonster::GetScheduleOfType( Type );
		return (pSched == slIdleStand) ? slBarnabyFaceTarget : pSched;
	case SCHED_TARGET_CHASE:
		return slBarnabyFollow;
	case SCHED_IDLE_STAND:
		pSched = CTalkMonster::GetScheduleOfType( Type );
		// Call base class so that Barnaby will talk when standing idle
		return (pSched == slIdleStand) ? slIdleBarnabyStand : pSched;
	}
	return CTalkMonster::GetScheduleOfType( Type );
}

void CBarnaby::CheckAmmo( void )
{
	if ( m_cAmmoLoaded < 1 )
		SetConditions( bits_COND_NO_AMMO_LOADED );
}

void CBarnaby::DeclineFollowing( void )
{
	PlaySentence( "BA_POK", 2.0f, VOL_NORM, ATTN_NORM );
}

void CBarnaby::TalkInit( void )
{
	CTalkMonster::TalkInit();

	m_szGrp[TLK_ANSWER] =	NULL;
	m_szGrp[TLK_QUESTION] =	NULL;
	m_szGrp[TLK_IDLE] =		"BA_IDLE";
	m_szGrp[TLK_STARE] =	NULL;
	m_szGrp[TLK_USE] =		"BA_OK";
	m_szGrp[TLK_UNUSE] =	"BA_WAIT";
	m_szGrp[TLK_STOP] =		"BA_STOP";

	m_szGrp[TLK_NOSHOOT] =	NULL;
	m_szGrp[TLK_HELLO] =	NULL;

	m_szGrp[TLK_PLHURT1] =	NULL;
	m_szGrp[TLK_PLHURT2] =	NULL; 
	m_szGrp[TLK_PLHURT3] =	NULL;

	m_szGrp[TLK_PHELLO] =	NULL;
	m_szGrp[TLK_PIDLE] =	NULL;
	m_szGrp[TLK_PQUESTION] = NULL;

	m_szGrp[TLK_SMELL] =	"BA_SMELL";
	
	m_szGrp[TLK_WOUND] =	"BA_WOUND";
	m_szGrp[TLK_MORTAL] =	"BA_MORTAL";

	m_voicePitch = 100;
}
