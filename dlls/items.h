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
#ifndef ITEMS_H
#define ITEMS_H

// Shepard
#define ITEM_MONEY_GIVE_AMOUNT_NORMAL 50
#define ITEM_MONEY_GIVE_AMOUNT_GOLD 150
#define MAX_ITEM_CARRY 9
#define MAX_MONEY_CARRY 999

class CItem : public CBaseEntity
{
public:
	void	Spawn( void );
	CBaseEntity*	Respawn( void );
	void	EXPORT ItemTouch( CBaseEntity *pOther );
	void	EXPORT Materialize( void );
	virtual BOOL MyTouch( CBasePlayer *pPlayer ) { return FALSE; };
};

#endif // ITEMS_H
