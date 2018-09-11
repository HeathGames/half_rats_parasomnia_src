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

class CBuyZone : public CBaseEntity
{
public:
	void Spawn( void );
};

//
//--------------------------------------------------
//

#define SF_ENV_STATE_START_ON 1

class CEnvState : public CPointEntity
{
public:
	void Spawn( void );
	void EXPORT StateThink( void );
	void KeyValue( KeyValueData* pkvd );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	BOOL IsLockedByMaster( void ) { return !UTIL_IsMasterTriggered( m_sMaster, NULL ); }
	BOOL ShouldToggle( USE_TYPE useType ); // Shepard : In SoHL, this is from CBaseEntity.

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	STATE m_iState;
	float m_flTurnOffTime;
	float m_flTurnOnTime;
	string_t m_sMaster;
};

//
//--------------------------------------------------
//

class CMusic : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

//
//--------------------------------------------------
//

#define SF_STATE_WATCHER_SEND_TOGGLE	0x1
#define SF_STATE_WATCHER_DONT_SEND_ON	0x2
#define SF_STATE_WATCHER_DONT_SEND_OFF	0x4
#define SF_STATE_WATCHER_NOT_ON			0x8
#define SF_STATE_WATCHER_OFF			0x10
#define SF_STATE_WATCHER_TURN_ON		0x20
#define SF_STATE_WATCHER_TURN_OFF		0x40
#define SF_STATE_WATCHER_IN_USE			0x80
#define SF_STATE_WATCHER_VALID			0x200

#define STATE_WATCHER_LOGIC_AND			0
#define STATE_WATCHER_LOGIC_OR			1
#define STATE_WATCHER_LOGIC_NAND		2
#define STATE_WATCHER_LOGIC_NOR			3
#define STATE_WATCHER_LOGIC_XOR			4
#define STATE_WATCHER_LOGIC_XNOR		5

class CStateWatcher : public CBaseToggle
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );

	void EXPORT Think( void );

	BOOL EvalLogic( CBaseEntity *pActivator );

	virtual STATE GetState( void );
	virtual STATE GetState( CBaseEntity *pActivator );
	virtual int ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_cTargets;
	int m_iLogic;
	int m_iTargetName[MAX_MULTI_TARGETS];
};

//
//--------------------------------------------------
//

class CPlayerFreeze : public CBaseDelay
{
public:
	void Spawn( void );
	void Think( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

private:
	EHANDLE m_hActivator;
};

//
//--------------------------------------------------
//

class CTriggerCredits : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

//
//--------------------------------------------------
//

class CTriggerInOut;

class CInOutRegister : public CPointEntity
{
public:
	BOOL IsEmpty( void ) { return m_pNext ? FALSE : TRUE; }
	BOOL IsRegistered( CBaseEntity *pValue );

	CInOutRegister *Add( CBaseEntity *pValue );
	CInOutRegister *Prune(void);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	static TYPEDESCRIPTION m_SaveData[];

	CTriggerInOut *m_pField;
	CInOutRegister *m_pNext;
	EHANDLE m_hValue;
};

class CTriggerInOut : public CBaseTrigger
{
public:
	void Spawn( void );
	void KeyValue( KeyValueData *pkvd );

	void EXPORT Think( void );
	void EXPORT Touch( CBaseEntity *pOther );

	BOOL CanTouch( entvars_t *pevToucher );
	void FireOnEntry( CBaseEntity *pOther );
	void FireOnLeaving( CBaseEntity *pOther );

	STATE GetState() { return m_pRegister->IsEmpty() ? STATE_OFF : STATE_ON; }

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	string_t m_iszAltTarget;
	string_t m_iszBothTarget;
	CInOutRegister *m_pRegister;
};

//
//--------------------------------------------------
//

class CTriggerNewGamePlus : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};

//
//--------------------------------------------------
//

class CTriggerReset : public CPointEntity
{
public:
	void Spawn( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
};
