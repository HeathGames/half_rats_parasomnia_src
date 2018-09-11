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
// Dynamite
//=========================================================

#ifndef CDYNAMITE_H
#define CDYNAMITE_H

#pragma once

class CDynamite : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );

	int iItemSlot( void ) { return 5; }
	int GetItemInfo( ItemInfo *p );

	BOOL CanHolster( void ) { return m_flStartThrow == 0.0f; }
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );

	void WeaponIdle( void );

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	bool m_bToss;
};

#endif // CDYNAMITE_H
