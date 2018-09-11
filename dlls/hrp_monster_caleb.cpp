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
// Caleb
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "game.h"
#include "monsters.h"
#include "zombie.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CCaleb : public CZombie
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Sound stuff
	void AlertSound( void );
	void AttackSound( void );
	void DeathSound( void );
	void IdleSound( void ) {};
	void PainSound( void );

	// Caleb stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );
	BOOL CanBlock( void );
	void RunTask( Task_t *pTask );
	void MonsterThink( void );

	// Sounds arrays
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pAttackSounds[];
	static const char *pBlockSounds[];
	static const char *pPainSounds[];
};
LINK_ENTITY_TO_CLASS( monster_caleb, CCaleb );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CCaleb::pAttackHitSounds[] =
{
	"caleb/cal_hit1.wav",
	"caleb/cal_hit2.wav",
	"caleb/cal_hit3.wav",
	"caleb/cal_hit4.wav"
};

const char *CCaleb::pAttackMissSounds[] =
{
	"caleb/cal_miss1.wav",
	"caleb/cal_miss2.wav"
};

const char *CCaleb::pAttackSounds[] =
{
	"caleb/cal_attack1.wav",
	"caleb/cal_attack2.wav",
	"caleb/cal_attack3.wav"
};

const char *CCaleb::pBlockSounds[] =
{
	"caleb/cal_block1.wav",
	"caleb/cal_block2.wav",
	"caleb/cal_block3.wav"
};

const char *CCaleb::pPainSounds[] =
{
	"caleb/cal_pain1.wav",
	"caleb/cal_pain2.wav",
	"caleb/cal_pain3.wav",
	"caleb/cal_pain4.wav"
};

void CCaleb::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/caleb.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.calebHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP | bits_CAP_TURN_HEAD;

	MonsterInit();
}

void CCaleb::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/caleb.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackHitSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackMissSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pBlockSounds ); i++ )
		PRECACHE_SOUND( (char *)pBlockSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "caleb/cal_death.wav" );
	PRECACHE_SOUND( "caleb/cal_jump.wav" );
	PRECACHE_SOUND( "caleb/cal_laugh.wav" );
}

void CCaleb::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.calebDmgOneSlash, DMG_SLASH );
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
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.calebDmgOneSlash, DMG_SLASH );
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
		case ZOMBIE_AE_ATTACK_BOTH:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.calebDmgBothSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackHitSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
		break;
		case 4:
		{
			pev->iuser3++;
			ClearBits( pev->flags, FL_ONGROUND );

			UTIL_SetOrigin( pev, pev->origin + Vector( 0.0f, 0.0f, 1.0f ) ); // Take him off ground so engine doesn't instantly reset onground 
			UTIL_MakeVectors( pev->angles );

			Vector vecJumpDir;
			if ( m_hEnemy != NULL )
			{
				float flGravity = g_psv_gravity->value;
				if ( flGravity <= 1.0f )
					flGravity = 1.0f;

				// How fast does the headcrab need to travel to reach that height given gravity?
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
			}
			else
			{
				// Jump hop, don't care where
				vecJumpDir = Vector( gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z ) * 350.0f;
			}
			EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "caleb/cal_jump.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			pev->velocity = vecJumpDir;
			m_flNextAttack = gpGlobals->time + 2.0f;
		}
		break;
		case 6:
		{
			Vector vecArmPos, vecArmDir;
			GetAttachment( 0, vecArmPos, vecArmDir );
			UTIL_BloodStream( vecArmPos, UTIL_RandomBloodVector(), (BYTE)70, 550 );
		}
		break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

void CCaleb::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	if ( pev->deadflag == DEAD_NO )
	{
		if ( bitsDamageType & (DMG_BULLET | DMG_SLASH) )
		{
			if ( CanBlock() )
			{
				SetActivity( ACT_TWITCH );
				Vector vecTemp = UTIL_VecToAngles( m_vecEnemyLKP - pev->origin );
				SetBlending( 0, vecTemp.x );
				UTIL_Ricochet( ptr->vecEndPos, 1.0f );
				pev->nextthink = gpGlobals->time + 0.75f;
				flDamage = 0.0f;
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pBlockSounds[RANDOM_LONG( 0, ARRAYSIZE( pBlockSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
		}
	}
	CBaseMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

int CCaleb::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( bitsDamageType & (DMG_BULLET | DMG_SLASH) )
	{
		if ( CanBlock() )
		{
			flDamage = 0.0f;
			if ( RANDOM_LONG( 0, 99 ) < 25 )
				EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "caleb/cal_laugh.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
		}
	}
	else
	{
		// HACK HACK -- until we fix this.
		if ( IsAlive() )
			PainSound();
	}
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CCaleb::AlertSound( void )
{
	return;
}

void CCaleb::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CCaleb::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "caleb/cal_death.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CCaleb::PainSound( void )
{
	if ( !CanBlock() )
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

BOOL CCaleb::CheckRangeAttack1( float flDot, float flDist )
{
	if ( pev->fuser1 > gpGlobals->time )
		return FALSE;

	if ( pev->iuser2 == 1 )
		return FALSE;

	if ( FBitSet( pev->flags, FL_ONGROUND ) && flDist <= 375.0f && flDist >= 100.0f && flDot >= 0.65f )
		return TRUE;

	return FALSE;
}

BOOL CCaleb::CanBlock( void )
{
	if ( pev->deadflag != DEAD_NO )
		return FALSE;

	if ( m_hEnemy == NULL )
		return FALSE;

	if ( !FBitSet( pev->flags, FL_ONGROUND ) )
		return FALSE;

	UTIL_MakeVectors ( pev->angles );
	Vector2D vec2LOS = ( m_hEnemy->pev->origin - pev->origin ).Make2D();
	vec2LOS = vec2LOS.Normalize();
	float flDot = DotProduct( vec2LOS , gpGlobals->v_forward.Make2D() );
	if ( flDot > 0.5f )
		return TRUE;
	else
		return FALSE;
}

void CCaleb :: RunTask ( Task_t *pTask )
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
			if ( m_hEnemy != NULL && m_hEnemy->IsPlayer() ) 
				WALK_MOVE( ENT( pev ), pev->ideal_yaw, m_flGroundSpeed * (gpGlobals->time - pev->animtime), WALKMOVE_NORMAL );

			if ( m_fSequenceFinished )
			{
				m_Activity = ACT_RESET;
				TaskComplete();
			}
			break;
		}
		default:
			CBaseMonster::RunTask ( pTask );
			break;
	}
}

void CCaleb::MonsterThink( void )
{
	if ( pev->iuser3 > 4 )
	{
		pev->iuser3 = 0;
		pev->fuser1 = gpGlobals->time + 20.0f;
	}

	Vector vecTemp = UTIL_VecToAngles( m_vecEnemyLKP - pev->origin );
	SetBlending( 0, vecTemp.x );
	float flYaw = VecToYaw( m_vecEnemyLKP - pev->origin ) - pev->angles.y;
	if ( flYaw > 180.0f )
		flYaw -= 360.0f;

	if ( flYaw < -180.0f )
		flYaw += 360.0f;

	SetBoneController( 0, flYaw );
	CBaseMonster::MonsterThink();
}
