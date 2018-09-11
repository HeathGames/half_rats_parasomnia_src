
/****
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
// Baph's fire ball
//=========================================================

class CBaphFireBall : public CBaseMonster
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	
	void FireBallInit( CBaseEntity *pTarget );
	void EXPORT FireBallThink( void );
	void EXPORT FireBallTouch( CBaseEntity *pOther );

	void MoveToOrigin( Vector _vecTarget );
	void Explode( void );

private:
	EHANDLE m_hEnemy;
	Vector m_vecIdeal;
};
