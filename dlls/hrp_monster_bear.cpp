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
// Bear
//=========================================================

// TODO : Fix crouch/stand up code.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "saverestore.h"
#include "zombie.h"

#define VEC_BEAR_HULL_MIN Vector( -5.0f, -5.0f, 0.0f ) 
#define VEC_BEAR_HULL_MAX Vector( 5.0f, 5.0f, 72.0f ) // Thin hull, maybe it will work better

class CBear : public CZombie
{
public:
	void Spawn( void );
	void Precache( void );

	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Killed( entvars_t *pevAttacker, int iGib ) { CZombie::Killed( pevAttacker, GIB_NEVER ); }

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void DeathSound( void );

	void MonsterThink( void );
	void SetActivity( Activity NewActivity );
	Activity GetDeathActivity( void );

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pPainSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

#ifdef USE_CROUCH_CODE
	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
#endif
private:
#ifdef USE_CROUCH_CODE
	BOOL m_bCanCrouch;
	BOOL m_bHullUpdated;
	BOOL m_bIsCrouched;
#endif
};
LINK_ENTITY_TO_CLASS( monster_bear, CBear );

#ifdef USE_CROUCH_CODE
TYPEDESCRIPTION CBear::m_SaveData[] =
{
	DEFINE_FIELD( CBear, m_bCanCrouch, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBear, m_bHullUpdated, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBear, m_bIsCrouched, FIELD_BOOLEAN )
};
IMPLEMENT_SAVERESTORE( CBear, CZombie );
#endif

const char *CBear::pAttackHitSounds[] =
{
	"bear/bear_clawhit1.wav",
	"bear/bear_clawhit2.wav"
};

const char *CBear::pAttackMissSounds[] =
{
	"bear/bear_claw1.wav",
	"bear/bear_claw2.wav"
};

const char *CBear::pAttackSounds[] =
{
	"bear/bear_attack1.wav",
	"bear/bear_attack2.wav"
};

const char *CBear::pIdleSounds[] =
{
	"bear/bear_idle1.wav",
	"bear/bear_idle2.wav"
};

const char *CBear::pPainSounds[] =
{
	"bear/bear_pain1.wav",
	"bear/bear_pain2.wav",
	"bear/bear_pain3.wav",
};

void CBear::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/bear.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 )
		pev->health		= gSkillData.bearHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

#ifdef USE_CROUCH_CODE
	m_bCanCrouch		= TRUE;
#endif

	MonsterInit();
}

void CBear::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/bear.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackHitSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackMissSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pIdleSounds ); i++ )
		PRECACHE_SOUND( (char *)pIdleSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "bear/bear_alert.wav" );
	PRECACHE_SOUND( "bear/bear_die.wav" );
}

void CBear::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			// Do stuff for this event.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.bearDmgOneSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.z = -18.0f;
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100.0f;
				}
				// Play a random attack hit sound
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackHitSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else // Play a random attack miss sound
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		case ZOMBIE_AE_ATTACK_LEFT:
		{
			// Do stuff for this event.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.bearDmgOneSlash, DMG_SLASH );
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
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		case ZOMBIE_AE_ATTACK_BOTH:
		{
			// Do stuff for this event.
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.bearDmgBothSlash, DMG_SLASH );
			if ( pHurt )
			{
				if ( pHurt->pev->flags & (FL_MONSTER | FL_CLIENT) )
				{
					pHurt->pev->punchangle.x = 5.0f;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * -100.0f;
				}
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackHitSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			}
			else
				EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackMissSounds ) - 1 )], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );

			if ( RANDOM_LONG( 0, 1 ) )
				AttackSound();
		}
		break;
		default: CBaseMonster::HandleAnimEvent( pEvent ); break;
	}
}

int CBear::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK -- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CBear::PainSound( void )
{
	int iPitch = 100 + RANDOM_LONG( -5, 5 );

	// Play a random pain sound
	if ( RANDOM_LONG( 0, 5 ) < 2 )
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, iPitch );
}

void CBear::AlertSound( void )
{
	int iPitch = 100 + RANDOM_LONG( -5, 5 );

	// Play a random alert sound
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bear/bear_alert.wav", 1.0f, ATTN_NORM, 0, iPitch );
}

void CBear::IdleSound( void )
{
	int iPitch = 100 + RANDOM_LONG( -5, 5 );

	// Play a random idle sound
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pIdleSounds[RANDOM_LONG( 0, ARRAYSIZE( pIdleSounds ) - 1 )], 1.0f, ATTN_NORM, 0, iPitch );
}

void CBear::AttackSound( void )
{
	int iPitch = 100 + RANDOM_LONG( -5, 5 );

	// Play a random attack sound
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pAttackSounds[RANDOM_LONG( 0, ARRAYSIZE( pAttackSounds ) - 1 )], 1.0f, ATTN_NORM, 0, iPitch );
}

void CBear::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bear/bear_die.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CBear::MonsterThink( void )
{
	if ( pev->deadflag == DEAD_NO )
	{
#ifdef USE_CROUCH_CODE
		// Crouch hull begin
		if ( m_bCanCrouch && !m_bHullUpdated )
		{
			if ( m_bIsCrouched )
			{
				UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX / 2.0f );
				pev->view_ofs = VEC_DUCK_VIEW;
				m_bHullUpdated = TRUE;
				m_IdealActivity = ACT_MELEE_ATTACK1;
			}
			else
			{
				UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
				pev->view_ofs = VEC_VIEW;
				m_bHullUpdated = TRUE;
				m_IdealActivity = ACT_MELEE_ATTACK1;
			}
		}
		// Crouch hull end
#endif

		if ( m_hEnemy != NULL )
		{
			if ( (m_hEnemy->pev->origin - pev->origin).Length() <= 70.0f ) // A better check melee attack
			{
				MakeIdealYaw( m_vecEnemyLKP );
				m_MonsterState = MONSTERSTATE_COMBAT;
				m_IdealActivity = ACT_MELEE_ATTACK1;
			}

			if ( !FBitSet( pev->flags, FL_ONGROUND ) )
			{
				MakeIdealYaw( m_hEnemy->pev->origin );
				ChangeYaw( pev->yaw_speed );
				pev->velocity.x = m_hEnemy->pev->origin.x - pev->origin.x;
				pev->velocity.y = m_hEnemy->pev->origin.y - pev->origin.y;

				if ( (m_hEnemy->pev->origin - pev->origin).Length() > 70.0f )
					m_IdealActivity = ACT_HOP;
			}

			if ( HasConditions( bits_COND_TASK_FAILED ) || m_Activity == ACT_IDLE || m_IdealActivity == ACT_IDLE )
			{
				MakeIdealYaw( m_hEnemy->pev->origin );
				ChangeYaw( pev->yaw_speed );
				pev->velocity.x = m_hEnemy->pev->origin.x - pev->origin.x;
				pev->velocity.y = m_hEnemy->pev->origin.y - pev->origin.y;
				if ( (m_hEnemy->pev->origin - pev->origin).Length() > 70.0f )
					m_IdealActivity = ACT_RUN;
			}

#ifdef USE_CROUCH_CODE
			// Combat crouch stuff begin
			if ( m_bCanCrouch )
			{
				if ( m_hEnemy->IsPlayer() )
				{
					TraceResult pTr;
					UTIL_TraceLine( pev->origin, pev->origin + Vector( 0.0f, 0.0f, 80.0f ), ignore_monsters, ENT( pev ), &pTr );
					Vector vecDestEnemy = m_hEnemy->pev->origin + Vector( 0.0f, 0.0f, VEC_HUMAN_HULL_MAX.z );
					TraceResult pTrEnemy;
					UTIL_TraceLine( m_hEnemy->pev->origin, vecDestEnemy, ignore_monsters, ENT( pev ), &pTrEnemy );
					if ( (m_hEnemy->pev->flags & FL_DUCKING) && FBitSet( m_hEnemy->pev->flags, FL_ONGROUND ) && pTrEnemy.flFraction != 1.0f )
					{	
						if ( !m_bIsCrouched )
							m_bHullUpdated = FALSE;

						m_bIsCrouched = TRUE;
					}
					else if ( (pTr.vecEndPos.z - pev->origin.z) >= 72.0f && UTIL_FindEntityInSphere( this, pev->origin, 80.0f ) == NULL )
					{
						if ( m_bIsCrouched )
							m_bHullUpdated = FALSE;

						m_bIsCrouched = FALSE;
					}
				}
			}
			// Combat crouch stuff end
#endif
		}
	}
	CBaseMonster::MonsterThink();
}

void CBear::SetActivity( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;
	void *pModel = GET_MODEL_PTR( ENT( pev ) );

	switch ( NewActivity )
	{
	case ACT_BIG_FLINCH: iSequence = LookupActivity( ACT_SMALL_FLINCH ); break;
	case ACT_FLINCH_HEAD:
	case ACT_FLINCH_CHEST:
	case ACT_FLINCH_STOMACH:
	case ACT_FLINCH_LEFTARM:
	case ACT_FLINCH_RIGHTARM:
	case ACT_FLINCH_LEFTLEG:
	case ACT_FLINCH_RIGHTLEG:
	case ACT_SMALL_FLINCH:
#ifdef USE_CROUCH_CODE
		if ( m_bIsCrouched )
			iSequence = LookupSequence( "crouch_flinch" );
		else
#endif
			LookupActivity( NewActivity );
		break;
	case ACT_WALK:
	case ACT_RUN:
#ifdef USE_CROUCH_CODE
		if ( m_bIsCrouched )
			iSequence = LookupSequence( "runcrouch1" );
		else
#endif
			iSequence = LookupActivity( NewActivity );
		break;
	case ACT_IDLE:
#ifdef USE_CROUCH_CODE
		if ( m_bIsCrouched )
			iSequence = LookupSequence( "crouch_idle" );
#endif
		iSequence = LookupActivity( NewActivity );
		break;
	case ACT_DIE_HEADSHOT:
	case ACT_DIE_GUTSHOT:
	case ACT_DIEFORWARD:
	case ACT_DIEBACKWARD:
	case ACT_DIESIMPLE:
#ifdef USE_CROUCH_CODE
		if ( m_bIsCrouched )
			iSequence = LookupSequence( "crouch_die" );
		else
#endif
			iSequence = LookupActivity( NewActivity );
		break;
	case ACT_MELEE_ATTACK1:
	case ACT_MELEE_ATTACK2:
#ifdef USE_CROUCH_CODE
		if ( m_bIsCrouched )
			iSequence = LookupSequence( "crouchmeele" );
		else
#endif
			iSequence = LookupActivity( NewActivity );
		break;
	default: iSequence = LookupActivity( NewActivity ); break;
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
			pev->frame = 0;

		pev->sequence = iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT( at_console, "[HR:P] \"%s\" has no sequence for act: %d\n", FStringNull( pev->targetname ) ? "monster_bear" : STRING( pev->targetname ), NewActivity );
		pev->sequence = 0;	// Set to the reset anim (if it's there)
	}
}

Activity CBear::GetDeathActivity( void )
{
	return (m_Activity == ACT_BIG_FLINCH) ? ACT_FALL : CBaseMonster::GetDeathActivity();
}
