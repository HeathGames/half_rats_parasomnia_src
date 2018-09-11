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
// Publican
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"

class CPublican : public CTalkMonster
{
public:
	void Spawn( void );
	void Precache( void );

	void SetYawSpeed( void );
	int Classify( void );

	virtual int ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }

	void Killed( entvars_t *pevAttacker, int iGib );

	void DeclineFollowing( void );
	void TalkInit( void );
};
LINK_ENTITY_TO_CLASS( monster_publican, CPublican );

void CPublican::Spawn( void )
{
	Precache();
	if ( pev->model )
		SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
	else
		SET_MODEL( ENT( pev ), "models/publican.mdl" );

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

	if ( pev->body == -1 )
		pev->body = RANDOM_LONG( 0, 2 );

	if ( pev->skin == -1 )
		pev->skin = RANDOM_LONG( 0, 1 );

	MonsterInit();
	SetUse( &CPublican::FollowerUse );
}

void CPublican::Precache( void )
{
	if ( pev->model ) // LRC
		PRECACHE_MODEL( (char *)STRING( pev->model ) );
	else
		PRECACHE_MODEL( "models/publican.mdl" );

	// Every new publican must call this, otherwise when a level is loaded, nobody will talk (time is reset to 0)
	TalkInit();
	CTalkMonster::Precache();
}

void CPublican::SetYawSpeed( void )
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

int CPublican::Classify( void )
{
	return CLASS_HUMAN_PASSIVE;
}

void CPublican::Killed( entvars_t *pevAttacker, int iGib )
{
	SetUse( NULL );
	CTalkMonster::Killed( pevAttacker, iGib );
}

void CPublican::DeclineFollowing( void )
{
	Talk( 10.0f );
	m_hTalkTarget = m_hEnemy;
	PlaySentence( "PU_POK", 0.8f, 1.0f, ATTN_NORM );
}

void CPublican::TalkInit( void )
{
	CTalkMonster::TalkInit();

	// Publican will try to talk to friends in this order
	m_szFriends[0] = "monster_publican";
	m_szFriends[1] = "monster_man";
	m_szFriends[2] = "monster_woman";
	m_szFriends[3] = "monster_upperclassman";

	m_szGrp[TLK_ANSWER]  =		"PU_ANSWER";
	m_szGrp[TLK_QUESTION] =		NULL;
	m_szGrp[TLK_IDLE] =			NULL;
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
	m_szGrp[TLK_PQUESTION] =	"PU_PQUESTION";

	m_szGrp[TLK_SMELL] =		NULL;

	m_szGrp[TLK_WOUND] =		NULL;
	m_szGrp[TLK_MORTAL] =		NULL;

	switch ( pev->body )
	{
	case 0: m_voicePitch = 95; break;
	case 1: m_voicePitch = 100; break;
	case 2: m_voicePitch = 105; break;
	}
}
