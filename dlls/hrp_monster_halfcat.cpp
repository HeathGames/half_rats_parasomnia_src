/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Half-Cat
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "defaultai.h"
#include "game.h"
#include "monsters.h"
#include "player.h"
#include "schedule.h"
#include "soundent.h"
#include "talkmonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define HC_AE_JUMP		( 1 )
#define HC_AE_ATTACK	( 2 )

class CHalfCat : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );

	void SetYawSpeed( void );
	int  ISoundMask( void );
	int  Classify ( void );

	Vector Center( void );
	Vector BodyTarget( const Vector &posSrc );

	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	void PrescheduleThink( void );
	virtual int	ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	BOOL CheckRangeAttack1( float flDot, float flDist );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Killed( entvars_t *pevAttacker, int iGib );

	void DeclineFollowing( void );
	void TalkInit( void );

	void AlertSound( void );
	void PainSound( void );

	Schedule_t *GetScheduleOfType( int Type );
	Schedule_t *GetSchedule( void );

	void EXPORT HCFollowerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;
private:
	int m_iAIMarkerTries;
};
LINK_ENTITY_TO_CLASS( monster_halfcat, CHalfCat );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CHalfCat::m_SaveData[] =
{
	DEFINE_FIELD( CHalfCat, m_iAIMarkerTries, FIELD_INTEGER )
};
IMPLEMENT_SAVERESTORE( CHalfCat, CTalkMonster );

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
Task_t tlHalfCatFollow[] =
{
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t slHalfCatFollow[] =
{
	{
		tlHalfCatFollow,
		ARRAYSIZE( tlHalfCatFollow ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HalfCatFollow"
	},
};

Task_t tlHalfCatFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t slHalfCatFaceTarget[] =
{
	{
		tlHalfCatFaceTarget,
		ARRAYSIZE( tlHalfCatFaceTarget ),
		bits_COND_CLIENT_PUSH	|
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"HalfCatFaceTarget"
	},
};

Task_t tlHalfCatIdleStand[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		}, // Repick IDLESTAND every two seconds.
};

Schedule_t slHalfCatIdleStand[] =
{
	{ 
		tlHalfCatIdleStand,
		ARRAYSIZE( tlHalfCatIdleStand ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		| // Sound flags - change these, and you'll break the talking code.

		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			| // Scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"HalfCatIdleStand"
	},
};

Task_t tlHalfCatRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_RANDOM,			(float)0.5		},
};

Schedule_t slHalfCatRangeAttack1[] =
{
	{ 
		tlHalfCatRangeAttack1,
		ARRAYSIZE( tlHalfCatRangeAttack1 ), 
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED,
		0,
		"HalfCatRangeAttack1"
	},
};

Task_t tlHalfCatGoToAIMarker[] =
{
	{ TASK_STOP_MOVING,						(float)0 },
	{ TASK_FACE_IDEAL,						(float)0 },
	{ TASK_GET_PATH_TO_AI_MARKER,			(float)0 },
	{ TASK_WALK_PATH,						(float)0 },
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0.1 },
	{ TASK_WAIT,							(float)0.1 },
	{ TASK_SET_SCHEDULE,					(float)SCHED_TARGET_FACE },
};

Schedule_t slHalfCatGoToAIMarker[] =
{
	{
		tlHalfCatGoToAIMarker,
		ARRAYSIZE( tlHalfCatGoToAIMarker ),
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER,
		"HalfCatGoToAIMarker"
	},
};

DEFINE_CUSTOM_SCHEDULES( CHalfCat )
{
	slHalfCatFollow,
	slHalfCatFaceTarget,
	slHalfCatIdleStand,
	slHalfCatRangeAttack1,
	slHalfCatGoToAIMarker
};
IMPLEMENT_CUSTOM_SCHEDULES( CHalfCat, CTalkMonster );

void CHalfCat::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/halfcat.mdl" );

	UTIL_SetSize( pev, Vector( -4.0f, -16.0f, 0.0f ), Vector( 4.0f, 20.0f, 20.0f ) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 ) // LRC
		pev->health		= 100;

	pev->view_ofs		= Vector( 0.0f, -16.0f, 16.0f );
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_HEAR | bits_CAP_DOORS_GROUP;

	m_iAIMarkerTries	= 0;

	MonsterInit();
	SetUse( &CHalfCat::HCFollowerUse );
}

void CHalfCat::Precache( void )
{
	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/halfcat.mdl" );

	PRECACHE_SOUND( "cat/cat_atk1.wav" );
	PRECACHE_SOUND( "cat/cat_atk2.wav" );

	PRECACHE_SOUND( "cat/cat_claw1.wav" );
	PRECACHE_SOUND( "cat/cat_claw2.wav" );

	PRECACHE_SOUND( "cat/cat_hit1.wav" );
	PRECACHE_SOUND( "cat/cat_hit2.wav" );

	PRECACHE_SOUND( "cat/cat_pain1.wav" );
	PRECACHE_SOUND( "cat/cat_pain2.wav" );
	PRECACHE_SOUND( "cat/cat_pain3.wav" );

	// Every new Half-Cat must call this, otherwise when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CHalfCat::SetYawSpeed( void )
{
	pev->yaw_speed = 200;
}

int CHalfCat::ISoundMask( void )
{
	// Even if Half-Cat don't smell/talk/get suspicious at suspicious sounds
	// those seems to be needed for the talk code
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE	|
			bits_SOUND_DANGER	|
			bits_SOUND_PLAYER;
}

int CHalfCat::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

Vector CHalfCat::Center( void )
{
	return Vector( pev->origin.x, pev->origin.y, pev->origin.z + 8.0f );
}

Vector CHalfCat::BodyTarget( const Vector &posSrc )
{
	return Center();
}

void CHalfCat::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
	case HC_AE_JUMP:
		{
			// Play the attack sound
			switch ( RANDOM_LONG( 0, 1 ) )
			{
			case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_atk1.wav", 1.0f, ATTN_IDLE, 0, m_voicePitch ); break;
			case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_atk2.wav", 1.0f, ATTN_IDLE, 0, m_voicePitch ); break;
			}

			// Make Half-Cat a little bit airbone to prevent "engine reset"
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
	case HC_AE_ATTACK:
		{
			switch ( RANDOM_LONG( 0, 1 ) )
			{
			case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "cat/cat_claw1.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
			case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "cat/cat_claw2.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
			}
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.halfcatDmgBite, DMG_SLASH, TRUE );
			if ( pHurt )
			{
				switch ( RANDOM_LONG( 0, 1 ) )
				{
				case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "cat/cat_hit1.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
				case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "cat/cat_hit2.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
				}
			}
		}
		break;
	default:
		CTalkMonster::HandleAnimEvent( pEvent );
	}
}

void CHalfCat::RunTask( Task_t *pTask )
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
		CTalkMonster::RunTask( pTask );
	}
}

void CHalfCat::StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;
	switch ( pTask->iTask )
	{
	case TASK_RANGE_ATTACK1:
		m_IdealActivity = ACT_RANGE_ATTACK1;
		break;
	case TASK_GET_PATH_TO_AI_MARKER:
		if ( IsFollowing() )
			StopFollowing( FALSE );

		CTalkMonster::StartTask( pTask );
		break;
	default:
		CTalkMonster::StartTask( pTask );
	}
}

void CHalfCat::PrescheduleThink( void )
{
	if ( m_MonsterState == MONSTERSTATE_COMBAT && RANDOM_FLOAT( 0.0f, 5.0f ) < 0.1f )
		PlaySentence( m_szGrp[TLK_IDLE], 4.0f, 1.0f, ATTN_NORM );

	// Marker code begin
	if ( m_iAIMarkerTries > 2 )
	{
		CBaseEntity *pAIMarker = UTIL_FindEntityByClassname( NULL, "ai_marker" );
		if ( pAIMarker )
		{
			UTIL_Remove( pAIMarker );
			/*for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );
				if ( pPlayer->m_pAIMarker == pAIMarker )
				{
					pPlayer->m_pAIMarker = NULL;
					ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "Friend has FAILED to reach destination." );
					break;
				}
			}*/
			ALERT( at_console, "[HR:P] Half-Cat - Failed to reach destination!\n" );
			m_iAIMarkerTries = 0;
		}
		else
			ALERT( at_console, "[HR:P] Half-Cat - Failed to retrieve the \"ai_marker\" entity!\n" );
	}
	// Marker code end

	// Teleport code begin
	if ( !IsFollowing() )
		return;

	CBaseEntity *pPlayer = UTIL_FindEntityByClassname( NULL, "player" );
	if ( !pPlayer ) // If the player doesn't exist, don't perform the check
		return;

	// Calculate the distance between Half-Cat and the player
	float flDistance = (pPlayer->pev->origin - pev->origin).Length2D();
	if ( flDistance > 1024.0f )
	{
		// Trace a line between Half-Cat and the player to see if anything block their view
		TraceResult pTr;
		UTIL_TraceLine( pev->origin + pev->view_ofs, pPlayer->pev->origin + pPlayer->pev->view_ofs, ignore_monsters, ENT( pev ), &pTr );
		if ( pTr.flFraction != 1.0f ) // Alright, it's good for teleport
		{
			ALERT( at_console, "[HR:P] Half-Cat - Calculating teleport position...\n", m_iAIMarkerTries );
			// See if he can be teleported on player's back
			UTIL_TraceLine( pPlayer->pev->origin, pPlayer->pev->origin + gpGlobals->v_forward * -48.0f, ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f )
			{
				UTIL_SetOrigin( pev, pPlayer->pev->origin + gpGlobals->v_forward * -48.0f );
				pev->movetype = MOVETYPE_STEP;
				m_hEnemy = NULL;
				if ( !DROP_TO_FLOOR( ENT( pev ) ) )
					ALERT( at_error, "[HR:P] Half-Cat - Fell out of level at %f, %f, %f (THIS CAN BE CRITICAL)\n", pev->origin.x, pev->origin.y, pev->origin.z );

				return;
			}

			// See if he can be teleported on player's left
			UTIL_TraceLine( pPlayer->pev->origin, pPlayer->pev->origin + gpGlobals->v_right * -48.0f, ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f )
			{
				UTIL_SetOrigin( pev, pPlayer->pev->origin + gpGlobals->v_right * -48.0f );
				pev->movetype = MOVETYPE_STEP;
				m_hEnemy = NULL;
				if ( !DROP_TO_FLOOR( ENT( pev ) ) )
					ALERT( at_error, "[HR:P] Half-Cat - Fell out of level at %f, %f, %f (THIS CAN BE CRITICAL)\n", pev->origin.x, pev->origin.y, pev->origin.z );

				return;
			}

			// See if he can be teleported on player's right
			UTIL_TraceLine( pPlayer->pev->origin, pPlayer->pev->origin + gpGlobals->v_right * 48.0f, ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f )
			{
				UTIL_SetOrigin( pev, pPlayer->pev->origin + gpGlobals->v_right * 48.0f );
				pev->movetype = MOVETYPE_STEP;
				m_hEnemy = NULL;
				if ( !DROP_TO_FLOOR( ENT( pev ) ) )
					ALERT( at_error, "[HR:P] Half-Cat - Fell out of level at %f, %f, %f (THIS CAN BE CRITICAL)\n", pev->origin.x, pev->origin.y, pev->origin.z );

				return;
			}

			// See if he can be teleported on player's front
			UTIL_TraceLine( pPlayer->pev->origin, pPlayer->pev->origin + gpGlobals->v_forward * 48.0f, ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f )
			{
				UTIL_SetOrigin( pev, pPlayer->pev->origin + gpGlobals->v_forward * 48.0f );
				m_hEnemy = NULL;
				if ( !DROP_TO_FLOOR( ENT( pev ) ) )
					ALERT( at_error, "[HR:P] Half-Cat - Fell out of level at %f, %f, %f (THIS CAN BE CRITICAL)\n", pev->origin.x, pev->origin.y, pev->origin.z );

				return;
			}
			// This is bad
			ALERT( at_console, "[HR:P] Half-Cat - All teleport positions are blocked!\n" );
		}
	}
	// Teleport code end
}

BOOL CHalfCat::CheckRangeAttack1( float flDot, float flDist )
{
	if ( FBitSet( pev->flags, FL_ONGROUND ) && flDist <= 256.0f && flDot >= 0.05f )
		return TRUE;

	return FALSE;
}

int CHalfCat::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	return CTalkMonster::TakeDamage( pevInflictor, pevAttacker, 0.0f, bitsDamageType );
}

void CHalfCat::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );
	CTalkMonster::Killed( pevAttacker, iGib );
}

void CHalfCat::DeclineFollowing( void )
{
	PlaySentence( "HC_POK", 0.8f, 1.0f, ATTN_NORM );
}

void CHalfCat::TalkInit( void )
{
	CTalkMonster::TalkInit();

	m_szGrp[TLK_ANSWER]  =	NULL; // Half-Cat doesn't answer questions
	m_szGrp[TLK_QUESTION] =	NULL; // and doesn't ask them too
	m_szGrp[TLK_IDLE] =		"HC_IDLE";
	m_szGrp[TLK_STARE] =	"HC_STARE";
	m_szGrp[TLK_USE] =		"HC_OK";
	m_szGrp[TLK_UNUSE] =	"HC_WAIT";
	m_szGrp[TLK_STOP] =		"HC_STOP";

	m_szGrp[TLK_NOSHOOT] =	NULL; // Half-Cat is invincible, he don't care about the damage
	m_szGrp[TLK_HELLO] =	"HC_HELLO";

	m_szGrp[TLK_PLHURT1] =	NULL; // Half-Cat can't warn the player he's dying
	m_szGrp[TLK_PLHURT2] =	NULL; 
	m_szGrp[TLK_PLHURT3] =	NULL;

	m_szGrp[TLK_PHELLO] =	"HC_HELLO";
	m_szGrp[TLK_PIDLE] =	"HC_IDLE";
	m_szGrp[TLK_PQUESTION] = NULL; // Half-Cat doesn't answer questions

	m_szGrp[TLK_SMELL] =	NULL; // Half-Cat doesn't smell and talk about it

	m_szGrp[TLK_WOUND] =	NULL; // Half-Cat is invincible, he doesn't need this
	m_szGrp[TLK_MORTAL] =	NULL;

	m_voicePitch = 100;
}

void CHalfCat::AlertSound( void )
{
	if ( m_hEnemy != NULL && FOkToSpeak() )
		PlaySentence( "HC_ALERT", 3.6f, 1.0f, ATTN_NORM );
}

void CHalfCat::PainSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_pain1.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
	case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_pain2.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
	case 2: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "cat/cat_pain3.wav", 1.0f, ATTN_NORM, 0, m_voicePitch ); break;
	}
}

Schedule_t *CHalfCat::GetScheduleOfType( int Type )
{
	Schedule_t *pSched;

	switch ( Type )
	{
	case SCHED_RANGE_ATTACK1:
		return &slHalfCatRangeAttack1[0];
	case SCHED_TARGET_FACE:
	case SCHED_IDLE_STAND:
		// Call base class so that Half-Cat will "talk" when "used"
		pSched = CTalkMonster::GetScheduleOfType( Type );
		if ( Type == SCHED_TARGET_FACE )
			return (pSched == slIdleStand) ? slHalfCatFaceTarget : pSched;
		else
			return (pSched == slIdleStand) ? slHalfCatIdleStand : pSched;
	case SCHED_TARGET_CHASE:
		return slHalfCatFollow;
	case SCHED_RELOAD:
		m_iAIMarkerTries++;
		ALERT( at_console, "[HR:P] Half-Cat - Going to fish! (try %d / 3)\n", m_iAIMarkerTries );
		return &slHalfCatGoToAIMarker[0];
	default:
		return CTalkMonster::GetScheduleOfType( Type );
	}
}

Schedule_t *CHalfCat::GetSchedule( void )
{
	// Marker code begin
	if ( m_hEnemy == NULL )
	{
		CBaseEntity *pAIMarker = UTIL_FindEntityByClassname( NULL, "ai_marker" );
		if ( pAIMarker )
		{
			float flDist = (pAIMarker->Center() - pev->origin).Length();
			if ( flDist <= 64.0f ) // Already at the marker, find the player who spawned it and delete it
			{
				UTIL_Remove( pAIMarker );
				/*for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );
					if ( pPlayer->m_pAIMarker == pAIMarker )
					{
						pPlayer->m_pAIMarker = NULL;
						ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "Friend has reached destination." );
						break;
					}
				}*/
				ALERT( at_console, "[HR:P] Half-Cat - Destination reached!\n" );
				m_iAIMarkerTries = 0;
			}
			else
				return GetScheduleOfType( SCHED_RELOAD );
		}
		else
			m_iAIMarkerTries = 0;
	}
	// Marker code end

	switch ( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
		if ( m_hEnemy == NULL && IsFollowing() )
		{
			if ( !m_hTargetEnt->IsAlive() )
			{
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

		break;
	}
	return CTalkMonster::GetSchedule();
}

// Marker code begin
void CHalfCat::HCFollowerUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBaseEntity *pAIMarker = UTIL_FindEntityByClassname( NULL, "ai_marker" );
	if ( pAIMarker )
	{
		DeclineFollowing();
		StopFollowing( FALSE );
		ALERT( at_console, "[HR:P] Half-Cat - Can\'t follow you, you threw a fish!\n" );
		return;
	}

	CTalkMonster::FollowerUse( pActivator, pCaller, useType, value );
}
// Marker code end
