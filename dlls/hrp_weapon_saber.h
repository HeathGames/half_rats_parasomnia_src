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
// Cavalry saber
//=========================================================

#ifndef CSABER_H
#define CSABER_H

#pragma once

enum saber_block_state_e {
	SABER_BLOCK_STATE_NO_BLOCK = 0,
	SABER_BLOCK_STATE_STARTING,
	SABER_BLOCK_STATE_BLOCKING,
	SABER_BLOCK_STATE_STOPPING,
};

class CSaber : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );

	int iItemSlot( void ) { return 1; }
	int GetItemInfo( ItemInfo *p );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void EXPORT Smack( void );

	void Reload( void );
	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

	BOOL m_bPlayBlockHitAnim;

#ifndef CLIENT_DLL
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
#endif

private:
	bool m_bUseSecondarySwing;

	TraceResult m_pTraceResult;
};

#endif // CSABER_H
