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
// Cult hermit
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "saverestore.h"
#include "talkmonster.h"

class CCultHermit : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );

	void SetYawSpeed( void );
	int Classify( void );

	virtual int ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	void Killed( entvars_t *pevAttacker, int iGib );

	void PainSound( void );

	void DeclineFollowing( void );
	void TalkInit( void );

	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
private:
	float m_flPainTime;
};
LINK_ENTITY_TO_CLASS( monster_culthermit, CCultHermit );

TYPEDESCRIPTION CCultHermit::m_SaveData[] =
{
	DEFINE_FIELD( CCultHermit, m_flPainTime, FIELD_TIME )
};
IMPLEMENT_SAVERESTORE( CCultHermit, CTalkMonster );

void CCultHermit::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/culthermit.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	if ( pev->health == 0 ) // LRC
		pev->health		= 10;

	pev->view_ofs		= Vector( 0.0f, 0.0f, 50.0f );
	m_flFieldOfView		= VIEW_FIELD_WIDE;
	m_MonsterState		= MONSTERSTATE_NONE;

	m_afCapability		= bits_CAP_TURN_HEAD | bits_CAP_USE;
	SetBits( pev->spawnflags, SF_MONSTER_PREDISASTER );

	MonsterInit();
	SetUse( &CCultHermit::FollowerUse );
}

void CCultHermit::Precache( void )
{
	if ( pev->model ) // LRC
		PRECACHE_MODEL( (char *)STRING( pev->model ) );
	else
		PRECACHE_MODEL( "models/culthermit.mdl" );

	PRECACHE_SOUND( "culthermit/herm_pain1.wav" );
	PRECACHE_SOUND( "culthermit/herm_pain2.wav" );
	PRECACHE_SOUND( "culthermit/herm_pain3.wav" );
	PRECACHE_SOUND( "culthermit/herm_pain4.wav" );

	// Every new Cult hermit must call this, otherwise when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CCultHermit::SetYawSpeed( void )
{
	int iYawSpeed = 90;
	switch ( m_Activity )
	{
	case ACT_WALK:
		iYawSpeed = 180;
		break;
	case ACT_RUN:
		iYawSpeed = 150;
		break;
	case ACT_IDLE:
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		iYawSpeed = 120;
		break;
	}
	pev->yaw_speed = iYawSpeed;
}

int CCultHermit::Classify( void )
{
	return CLASS_HUMAN_PASSIVE;
}

void CCultHermit::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );
	CTalkMonster::Killed( pevAttacker, iGib );
}

void CCultHermit::PainSound( void )
{
	if ( gpGlobals->time < m_flPainTime )
		return;

	switch ( RANDOM_LONG( 0, 3 ) )
	{
	case 0: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "culthermit/herm_pain1.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM ); break;
	case 1: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "culthermit/herm_pain2.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM ); break;
	case 2: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "culthermit/herm_pain3.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM ); break;
	case 3: EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "culthermit/herm_pain4.wav", 1.0f, ATTN_NORM, 0, PITCH_NORM ); break;
	}
	m_flPainTime = gpGlobals->time + 2.0f;
}

void CCultHermit::DeclineFollowing( void )
{
	Talk( 10.0f );
	m_hTalkTarget = m_hEnemy;
	PlaySentence( "CU_POK", 0.8f, 1.0f, ATTN_NORM );
}

void CCultHermit::TalkInit( void )
{
	CTalkMonster::TalkInit();

	m_szGrp[TLK_ANSWER]  =		NULL;
	m_szGrp[TLK_QUESTION] =		NULL;
	m_szGrp[TLK_IDLE] =			"CU_IDLE";
	m_szGrp[TLK_STARE] =		NULL;
	m_szGrp[TLK_USE] =			NULL;
	m_szGrp[TLK_UNUSE] =		NULL;
	m_szGrp[TLK_STOP] =			NULL;

	m_szGrp[TLK_NOSHOOT] =		NULL;
	m_szGrp[TLK_HELLO] =		NULL;

	m_szGrp[TLK_PLHURT1] =		NULL;
	m_szGrp[TLK_PLHURT2] =		NULL; 
	m_szGrp[TLK_PLHURT3] =		NULL;

	m_szGrp[TLK_PHELLO] =		NULL;
	m_szGrp[TLK_PIDLE] =		NULL;
	m_szGrp[TLK_PQUESTION] =	NULL;

	m_szGrp[TLK_SMELL] =		NULL;

	m_szGrp[TLK_WOUND] =		NULL;
	m_szGrp[TLK_MORTAL] =		NULL;

	m_voicePitch = 100;
}
