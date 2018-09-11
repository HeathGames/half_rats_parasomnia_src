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
// Buckshot ammo box
//=========================================================

#ifndef CAMMOBUCKSHOT_H
#define CAMMOBUCKSHOT_H

#pragma once

class CAmmoBuckshot : public CBasePlayerAmmo
{
public:
	void Spawn( void );
	void Precache( void );
	BOOL AddAmmo( CBaseEntity *pOther );
};

#endif // CAMMOBUCKSHOT_H
