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
// HR:P triggers
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "hrp_triggers.h"
#include "player.h"
#include "weapons.h"

LINK_ENTITY_TO_CLASS( func_buy, CBuyZone );

void CBuyZone::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( env_state, CEnvState );

TYPEDESCRIPTION CEnvState::m_SaveData[] =
{
	DEFINE_FIELD( CEnvState, m_flTurnOffTime, FIELD_TIME ),
	DEFINE_FIELD( CEnvState, m_flTurnOnTime, FIELD_TIME ),
	DEFINE_FIELD( CEnvState, m_iState, FIELD_INTEGER ),
	DEFINE_FIELD( CEnvState, m_sMaster, FIELD_STRING )
};
IMPLEMENT_SAVERESTORE( CEnvState, CPointEntity );

void CEnvState::Spawn( void )
{
	m_iState = (pev->spawnflags & SF_ENV_STATE_START_ON) ? STATE_ON : STATE_OFF;
	SetThink( &CEnvState::StateThink );
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CEnvState::KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq (pkvd->szKeyName, "turnontime" ) )
	{
		m_flTurnOnTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "turnontime" ) )
	{
		m_flTurnOffTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "turnontime" ) )
	{
		m_sMaster = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

void CEnvState::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType ) || IsLockedByMaster() )
		return;

	switch ( m_iState )
	{
	case STATE_ON:
	case STATE_TURN_ON:
		if ( m_flTurnOffTime )
		{
			m_iState = STATE_TURN_OFF;
			pev->nextthink = gpGlobals->time + m_flTurnOffTime;
		}
		else
		{
			m_iState = STATE_OFF;
			FireTargets( STRING( pev->target ), pActivator, this, USE_OFF, 0.0f );
			FireTargets( STRING( pev->noise2 ), pActivator, this, USE_TOGGLE, 0.0f );
			pev->nextthink = 0.0f;
		}
		break;
	case STATE_OFF:
	case STATE_TURN_OFF:
		if ( m_flTurnOnTime )
		{
			m_iState = STATE_TURN_ON;
			pev->nextthink = gpGlobals->time + m_flTurnOnTime;
		}
		else
		{
			m_iState = STATE_ON;
			FireTargets( STRING( pev->target ), pActivator, this, USE_ON, 0.0f );
			FireTargets( STRING( pev->noise2 ), pActivator, this, USE_TOGGLE, 0.0f );
			pev->nextthink = 0.0f;
		}
		break;
	}
}

void CEnvState::StateThink( void )
{
	if ( m_iState == STATE_TURN_ON )
	{
		m_iState = STATE_ON;
		FireTargets( STRING( pev->target ), this, this, USE_ON, 0.0f );
		FireTargets( STRING( pev->noise2 ), this, this, USE_TOGGLE, 0.0f );
	}
	else if ( m_iState == STATE_TURN_OFF )
	{
		m_iState = STATE_OFF;
		FireTargets( STRING( pev->target ), this, this, USE_OFF, 0.0f );
		FireTargets( STRING( pev->noise2 ), this, this, USE_TOGGLE, 0.0f );
	}
	pev->nextthink = gpGlobals->time + 0.1f;
}

// Shepard : In SoHL, this is from CBaseEntity.
BOOL CEnvState::ShouldToggle( USE_TYPE useType )
{
	if ( useType != USE_TOGGLE && useType != USE_SET )
	{
		switch ( m_iState )
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if ( useType == USE_ON )
				return FALSE;

			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if ( useType == USE_OFF )
				return FALSE;

			break;
		}
	}
	return TRUE;
}

//
//--------------------------------------------------
//

extern int gmsgFMOD;

LINK_ENTITY_TO_CLASS( env_music, CMusic );

void CMusic::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

void CMusic::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	MESSAGE_BEGIN( MSG_ALL, gmsgFMOD );
		WRITE_STRING( (char *)STRING( pev->target ) );
		WRITE_COORD( pev->speed );
		WRITE_BYTE( (pev->spawnflags & 1) ? TRUE : FALSE );
	MESSAGE_END();
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( player_freeze, CPlayerFreeze );

void CPlayerFreeze::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
	m_hActivator = NULL;
}

void CPlayerFreeze::Think( void )
{
	Use( m_hActivator, this, USE_ON, 0.0f );
}

void CPlayerFreeze::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator )
		return;

	if ( !(pActivator->pev->flags & FL_CLIENT) )
		return;

	if ( !ShouldToggle( useType, (pActivator->pev->flags & FL_FROZEN) ) )
		return;

	if ( pActivator->pev->flags & FL_FROZEN )
	{
		((CBasePlayer *)((CBaseEntity *)pActivator))->EnableControl( TRUE );
		m_hActivator = NULL;
		SetThink( NULL );
	}
	else
	{
		((CBasePlayer *)((CBaseEntity *)pActivator))->EnableControl( FALSE );
		m_hActivator = pActivator;
		pev->nextthink = gpGlobals->time + m_flDelay;
	}
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( multi_watcher, CStateWatcher );
TYPEDESCRIPTION CStateWatcher::m_SaveData[] =
{
	DEFINE_FIELD( CStateWatcher, m_cTargets, FIELD_INTEGER ),
	DEFINE_FIELD( CStateWatcher, m_iLogic, FIELD_INTEGER ),
	DEFINE_ARRAY( CStateWatcher, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS )
};
IMPLEMENT_SAVERESTORE( CStateWatcher, CBaseToggle );

void CStateWatcher::Spawn( void )
{
	pev->solid = SOLID_NOT;
	if ( pev->target )
		pev->nextthink = gpGlobals->time + 0.5f;
}

void CStateWatcher::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iLogic" ) )
	{
		m_iLogic = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iszWatch" ) )
	{
		if ( m_cTargets < MAX_MULTI_TARGETS )
		{
			m_iTargetName[m_cTargets] = ALLOC_STRING( pkvd->szValue );
			m_cTargets++;
			pkvd->fHandled = TRUE;
		}
		else
			ALERT( at_console, "multi_watcher - Too many targets\n" );
	}
	else
	{
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char cTemp[128];
			UTIL_StripToken( pkvd->szKeyName, cTemp );
			m_iTargetName[m_cTargets] = ALLOC_STRING( cTemp );
			m_cTargets++;
			pkvd->fHandled = TRUE;
		}
		else
			ALERT( at_console, "multi_watcher - Too many targets\n" );
	}
}

void CStateWatcher::Think( void )
{
	pev->nextthink = gpGlobals->time + 0.1f;
	int iOldFlag = (pev->spawnflags & SF_STATE_WATCHER_VALID);

	if ( EvalLogic( NULL ) )
		pev->spawnflags |= SF_STATE_WATCHER_VALID;
	else
		pev->spawnflags &= ~SF_STATE_WATCHER_VALID;

	if ( (pev->spawnflags & SF_STATE_WATCHER_VALID) != iOldFlag )
	{
		if ( iOldFlag )
		{
			if ( !FBitSet( pev->spawnflags, SF_STATE_WATCHER_DONT_SEND_OFF ) )
			{
				if ( pev->spawnflags & SF_STATE_WATCHER_SEND_TOGGLE )
					SUB_UseTargets( this, USE_TOGGLE, 0.0f );
				else
					SUB_UseTargets( this, USE_OFF, 0.0f );
			}
		}
		else
		{
			if ( !FBitSet( pev->spawnflags, SF_STATE_WATCHER_DONT_SEND_ON ) )
			{
				if ( pev->spawnflags & SF_STATE_WATCHER_SEND_TOGGLE )
					SUB_UseTargets( this, USE_TOGGLE, 0.0f );
				else
					SUB_UseTargets( this, USE_ON, 0.0f );
			}
		}
	}
}

BOOL CStateWatcher::EvalLogic( CBaseEntity *pActivator )
{
	BOOL xorgot = FALSE; // Shepard : Nice variable name

	for (int i = 0; i < m_cTargets; i++)
	{
		CBaseEntity *pEntity = UTIL_FindEntityByTargetname( NULL, STRING( m_iTargetName[i] ), pActivator );
		if ( pEntity == NULL )
			continue;

		BOOL b = FALSE; // Shepard : Nice variable name (again)
		switch ( pEntity->GetState() )
		{
		case STATE_ON:
			if ( !(pev->spawnflags & SF_STATE_WATCHER_NOT_ON) )
				b = TRUE;
			break;
		case STATE_OFF:
			if ( pev->spawnflags & SF_STATE_WATCHER_OFF )
				b = TRUE;
			break;
		case STATE_TURN_ON:
			if ( pev->spawnflags & SF_STATE_WATCHER_TURN_ON )
				b = TRUE;
			break;
		case STATE_TURN_OFF:
			if ( pev->spawnflags & SF_STATE_WATCHER_TURN_ON )
				b = TRUE;
			break;
		case STATE_IN_USE:
			if ( pev->spawnflags & SF_STATE_WATCHER_IN_USE )
				b = TRUE;
			break;
		}

		if ( b )
		{
			switch ( m_iLogic )
			{
			case STATE_WATCHER_LOGIC_OR:
				return TRUE;
			case STATE_WATCHER_LOGIC_NOR:
				return FALSE;
			case STATE_WATCHER_LOGIC_XOR:
				if ( xorgot )
					return FALSE;

				xorgot = TRUE;
				break;
			case STATE_WATCHER_LOGIC_XNOR:
				if ( xorgot )
					return TRUE;

				xorgot = TRUE;
				break;
			}
		}
		else
		{
			switch ( m_iLogic )
			{
			case STATE_WATCHER_LOGIC_AND:
				return FALSE;
			case STATE_WATCHER_LOGIC_NAND:
				return TRUE;
			}
		}
	}

	switch ( m_iLogic )
	{
	case STATE_WATCHER_LOGIC_AND:
	case STATE_WATCHER_LOGIC_NOR:
		return TRUE;
	case STATE_WATCHER_LOGIC_XOR:
		return xorgot;
	case STATE_WATCHER_LOGIC_XNOR:
		return !xorgot;
	default:
		return FALSE;
	}
}

STATE CStateWatcher::GetState( void )
{
	return EvalLogic( NULL ) ? STATE_ON : STATE_OFF;
}

STATE CStateWatcher::GetState( CBaseEntity *pActivator )
{
	return EvalLogic( pActivator ) ? STATE_ON : STATE_OFF;
}

//
//--------------------------------------------------
//

extern int gmsgVGUIMenu;

LINK_ENTITY_TO_CLASS( trigger_credits, CTriggerCredits );

void CTriggerCredits::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

void CTriggerCredits::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( g_pGameRules->IsMultiplayer() )
		return;

	if ( !pActivator )
		return;

	if ( !(pActivator->pev->flags & FL_CLIENT) )
		return;

	CBasePlayer *plr = (CBasePlayer *)pActivator;
	plr->m_iGameEnd = 1;
	MESSAGE_BEGIN( MSG_ONE, gmsgVGUIMenu, NULL, pActivator->edict() );
		WRITE_BYTE( 11 );
		WRITE_BYTE( 0 );
	MESSAGE_END();
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( inout_register, CInOutRegister );
TYPEDESCRIPTION CInOutRegister::m_SaveData[] =
{
	DEFINE_FIELD( CInOutRegister, m_pField, FIELD_CLASSPTR ),
	DEFINE_FIELD( CInOutRegister, m_pNext, FIELD_CLASSPTR ),
	DEFINE_FIELD( CInOutRegister, m_hValue, FIELD_EHANDLE ),
};
IMPLEMENT_SAVERESTORE( CInOutRegister, CPointEntity );

BOOL CInOutRegister::IsRegistered(CBaseEntity *pValue)
{
	if ( m_hValue == pValue )
		return TRUE;
	else if ( m_pNext )
		return m_pNext->IsRegistered( pValue );
	else
		return FALSE;
}

CInOutRegister *CInOutRegister::Add(CBaseEntity *pValue)
{
	if ( m_hValue == pValue )
		return this;
	else if (m_pNext)
	{
		m_pNext = m_pNext->Add( pValue );
		return this;
	}
	else
	{
		CInOutRegister *pResult = GetClassPtr( (CInOutRegister *)NULL );
		pResult->m_hValue = pValue;
		pResult->m_pNext = this;
		pResult->m_pField = m_pField;
		pResult->pev->classname = MAKE_STRING( "inout_register" );
		m_pField->FireOnEntry( pValue );
		return pResult;
	}
}

CInOutRegister *CInOutRegister::Prune( void )
{
	if ( m_hValue )
	{
		if ( m_pField->Intersects( m_hValue ) )
		{
			m_pNext = m_pNext->Prune();
			return this;
		}
		else
		{
			m_pField->FireOnLeaving( m_hValue );
			SetThink( &CBaseEntity::SUB_Remove );
			pev->nextthink = gpGlobals->time + 0.1f;
			return m_pNext->Prune();
		}
	}
	else
	{
		if ( m_pNext )
		{
			SetThink( &CBaseEntity::SUB_Remove );
			pev->nextthink = gpGlobals->time + 0.1f;
			return m_pNext->Prune();
		}
		else
			return this;
	}
}

LINK_ENTITY_TO_CLASS( trigger_inout, CTriggerInOut );

TYPEDESCRIPTION CTriggerInOut::m_SaveData[] =
{
	DEFINE_FIELD( CTriggerInOut, m_iszAltTarget, FIELD_STRING ),
	DEFINE_FIELD( CTriggerInOut, m_iszBothTarget, FIELD_STRING ),
	DEFINE_FIELD( CTriggerInOut, m_pRegister, FIELD_CLASSPTR )
};
IMPLEMENT_SAVERESTORE( CTriggerInOut, CBaseTrigger );

void CTriggerInOut::Spawn( void )
{
	InitTrigger();
	m_pRegister = GetClassPtr( (CInOutRegister *)NULL );
	m_pRegister->m_hValue = NULL;
	m_pRegister->m_pNext = NULL;
	m_pRegister->m_pField = this;
	m_pRegister->pev->classname = MAKE_STRING( "inout_register" );
}

void CTriggerInOut::KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iszAltTarget" ) )
	{
		m_iszAltTarget = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iszBothTarget" ) )
	{
		m_iszBothTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue( pkvd );
}

void CTriggerInOut::Think( void )
{
	m_pRegister = m_pRegister->Prune();
	if ( m_pRegister->IsEmpty() )
		pev->nextthink = 0.0f;
	else
		pev->nextthink = gpGlobals->time + 0.1f;
}

void CTriggerInOut::Touch( CBaseEntity *pOther )
{
	if ( !CanTouch( pOther->pev ) )
		return;

	m_pRegister = m_pRegister->Add( pOther );
	if ( pev->nextthink <= 0.0f && !m_pRegister->IsEmpty() )
		pev->nextthink = gpGlobals->time + 0.1f;
}

BOOL CTriggerInOut::CanTouch( entvars_t *pevToucher )
{
	if (!pev->netname)
	{
		if ( pevToucher->flags & FL_CLIENT )
			return !(pev->spawnflags & SF_TRIGGER_NOCLIENTS);
		else if ( pevToucher->flags & FL_MONSTER )
			return (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS);
		else if ( FClassnameIs( pevToucher, "func_pushable" ) )
			return (pev->spawnflags & SF_TRIGGER_PUSHABLES);
		else
			return TRUE;
	}
	else
	{
		if ( !FClassnameIs( pevToucher, STRING( pev->netname ) ) &&
			(!pevToucher->targetname || !FStrEq( STRING( pevToucher->targetname ), STRING( pev->netname ) )) )
			return FALSE;
	}
	return TRUE;
}

void CTriggerInOut::FireOnEntry( CBaseEntity *pOther )
{
	if ( UTIL_IsMasterTriggered( m_sMaster, pOther ) )
	{
		FireTargets( STRING( m_iszBothTarget ), pOther, this, USE_ON, 0.0f );
		FireTargets( STRING( pev->target ), pOther, this, USE_TOGGLE, 0.0f );
	}
}

void CTriggerInOut::FireOnLeaving( CBaseEntity *pOther )
{
	if ( UTIL_IsMasterTriggered( m_sMaster, pOther ) )
	{
		FireTargets( STRING( m_iszBothTarget ), pOther, this, USE_OFF, 0.0f );
		FireTargets( STRING( m_iszAltTarget ), pOther, this, USE_TOGGLE, 0.0f );
	}
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( trigger_new_game_plus, CTriggerNewGamePlus );

void CTriggerNewGamePlus::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

void CTriggerNewGamePlus::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( g_pGameRules->IsMultiplayer() )
		return;

	if ( !pActivator )
		return;

	if ( !(pActivator->pev->flags & FL_CLIENT) )
		return;

	CBasePlayer *plr = (CBasePlayer *)pActivator;
	if ( plr->m_iGameEnd != 1 )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgVGUIMenu, NULL, pActivator->edict() );
		WRITE_BYTE( 12 );
		WRITE_BYTE( 0 );
	MESSAGE_END();
}

//
//--------------------------------------------------
//

LINK_ENTITY_TO_CLASS( trigger_reset, CTriggerReset );

void CTriggerReset::Spawn( void )
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL( ENT( pev ), STRING( pev->model ) );
}

void CTriggerReset::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !pActivator )
		return;

	if ( !(pActivator->pev->flags & FL_CLIENT) )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pActivator;
	if ( !pPlayer )
		return;

	pPlayer->pev->health = pPlayer->pev->max_health;
	pPlayer->pev->armorvalue = 0;
	pPlayer->m_iMoney = MONEY_START;
	pPlayer->m_iSelectedItem = ITEM_COCAINE;
	pPlayer->RemoveAllItems( TRUE );
	pPlayer->pev->gravity = 1.0f;
	pPlayer->m_flCocaineTime = -1.0f;
	pPlayer->m_flCocaineTime = -1.0f;
	pPlayer->m_flDrymeatTime = -1.0f;
	pPlayer->m_iWhiskeyCount = 0;
	for ( int i = ITEM_COCAINE; i < ITEM_WHALEOIL; i++ )
		pPlayer->m_rgItems[i] = 0;
}
