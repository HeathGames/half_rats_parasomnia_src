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
// Bernard
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_entity_bernard_puke.h"
#include "monsters.h"
#include "zombie.h"

//=========================================================
// Monster's Class Declaration Go Here
//=========================================================
class CBernard : public CZombie
{
public:
	// Spawn and precache
	void Spawn( void );
	void Precache( void );

	// Relationship and yaw speed
	int Classify( void ) { return CLASS_ALIEN_MONSTER; }

	// Attack stuff
	BOOL CheckRangeAttack1( float flDot, float flDist );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	// Animation and damage stuff
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );

	// Sound stuff
	void AlertSound( void );
	void AttackSound( void );
	void IdleSound( void );
	void PainSound( void );

	// Sounds arrays
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pPainSounds[];

	// Save/restore stuff
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
private:
	BOOL m_bLastVomitCheck;
	float m_flCheckVomitTime;
	float m_flNextVomitTime;
};
LINK_ENTITY_TO_CLASS( monster_bernard, CBernard );

//=========================================================
// Monster's Save Data Go Here
//=========================================================
TYPEDESCRIPTION CBernard::m_SaveData[] =
{
	DEFINE_FIELD( CBernard, m_bLastVomitCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBernard, m_flCheckVomitTime, FIELD_TIME ),
	DEFINE_FIELD( CBernard, m_flNextVomitTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CBernard, CZombie );

//=========================================================
// Monster's Sounds Arrays Go Here
//=========================================================
const char *CBernard::pAttackHitSounds[] =
{
	"bernard/bern_hit1.wav",
	"bernard/bern_hit2.wav"
};

const char *CBernard::pAttackMissSounds[] =
{
	"bernard/bern_melee1.wav",
	"bernard/bern_melee2.wav"
};

const char *CBernard::pPainSounds[] =
{
	"bernard/bern_pain1.wav",
	"bernard/bern_pain2.wav",
	"bernard/bern_pain3.wav",
	"bernard/bern_pain4.wav"
};

void CBernard::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/bernard.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, Vector( 16.0f, 16.0f, 128.0f ) );
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( !pev->health )
		pev->health		= gSkillData.bernardHealth;

	pev->view_ofs		= VEC_VIEW;
	m_flFieldOfView		= 0.5f;
	m_MonsterState		= MONSTERSTATE_NONE;
	m_afCapability		= bits_CAP_DOORS_GROUP;

	m_flNextVomitTime	= gpGlobals->time;
	MonsterInit();
}

void CBernard::Precache( void )
{
	int i;

	if ( pev->model )
		PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
	else
		PRECACHE_MODEL( "models/bernard.mdl" );

	for ( i = 0; i < ARRAYSIZE( pAttackHitSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackHitSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pAttackMissSounds ); i++ )
		PRECACHE_SOUND( (char *)pAttackMissSounds[i] );

	for ( i = 0; i < ARRAYSIZE( pPainSounds ); i++ )
		PRECACHE_SOUND( (char *)pPainSounds[i] );

	PRECACHE_SOUND( "bernard/bern_atk.wav" );
	PRECACHE_SOUND( "bernard/bern_idle.wav" );
	PRECACHE_SOUND( "bernard/bern_puke.wav" );
	PRECACHE_SOUND( "bernard/bern_step.wav" );

	UTIL_PrecacheOther( "bernard_puke" );
}

BOOL CBernard::CheckRangeAttack1( float flDot, float flDist )
{
	// Too soon to vomit
	if ( gpGlobals->time <= m_flNextVomitTime )
		return FALSE;

	// Close to my enemy and looking at it
	if ( flDist <= 512.0f && flDot >= 0.5f )
	{
		// Too soon to check
		if ( gpGlobals->time > m_flCheckVomitTime )
		{
			TraceResult pTr;
			Vector vecShootOrigin = pev->origin + Vector( 0.0f, 0.0f, 55.0f );
			CBaseEntity *pEnemy = m_hEnemy;
			Vector vecShootTarget = (pEnemy->BodyTarget(vecShootOrigin) - pEnemy->pev->origin) + m_vecEnemyLKP;
			UTIL_TraceLine( vecShootOrigin, vecShootTarget, dont_ignore_monsters, ENT( pev ), &pTr );
			if ( pTr.flFraction == 1.0f || (pTr.pHit != NULL && CBaseEntity::Instance( pTr.pHit ) == pEnemy) )
				m_bLastVomitCheck = TRUE;
			else
				m_bLastVomitCheck = FALSE;

			m_flCheckVomitTime = gpGlobals->time + 1.0f;
		}
		return m_bLastVomitCheck;
	}
	return FALSE;
}

int CBernard::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// HACK HACK --- until we fix this.
	if ( IsAlive() )
		PainSound();

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

void CBernard::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch ( pEvent->event )
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.bernardDmgSlash, DMG_SLASH );
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
			CBaseEntity *pHurt = CheckTraceHullAttack( 70.0f, gSkillData.bernardDmgSlash, DMG_SLASH );
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
			Vector vecPukeOrg;
			//Vector vecPukeOrg = pev->origin + (gpGlobals->v_forward * 64.0f);
			Vector vecUseless;
			GetAttachment( 0, vecPukeOrg, vecUseless );
			vecPukeOrg = vecPukeOrg + gpGlobals->v_forward * 32.0f;
			Vector vecPukeDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - vecPukeOrg).Normalize();

			CBernardPuke::Shoot( pev, vecPukeOrg, vecPukeDir * 2048.0f );
			EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bernard/bern_puke.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
			m_flNextVomitTime = gpGlobals->time + 3.0f;
		}
		break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

void CBernard::TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType )
{
	switch ( ptr->iHitgroup )
	{
	case 1:
		if (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB))
		{
			UTIL_Ricochet( ptr->vecEndPos, 1.0f );
			flDamage = 0.01f;
		}
		break;
	}
	CZombie::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CBernard::AlertSound( void )
{
	return;
}

void CBernard::AttackSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bernard/bern_atk.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CBernard::IdleSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bernard/bern_idle.wav", 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}

void CBernard::PainSound( void )
{
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, pPainSounds[RANDOM_LONG( 0, ARRAYSIZE( pPainSounds ) - 1)], 1.0f, ATTN_NORM, 0, 100 + RANDOM_LONG( -5, 5 ) );
}
