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
// Colt SAA
//=========================================================

#ifndef CCOLT_H
#define CCOLT_H

#pragma once

enum colt_gangsta_state_e {
	COLT_GANGSTA_STATE_NO = 0,
	COLT_GANGSTA_STATE_LOOPING,
	COLT_GANGSTA_STATE_STOPPING,
};

class CColt : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );

	int iItemSlot( void ) { return 2; }
	int GetItemInfo( ItemInfo *p );

	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void ColtFire( bool bIsGangsta );

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

#ifndef CLIENT_DLL
	int Save( CSave &save );
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];
#endif

private:
	unsigned short m_usEvent;
};

#endif // CCOLT_H
