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
// Dynamite ammo box
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "hrp_ammo_dynamitebox.h"

LINK_ENTITY_TO_CLASS( ammo_dynamitebox, CAmmoDynamiteBox );

void CAmmoDynamiteBox::Spawn( void )
{
	Precache();
	SET_MODEL( ENT( pev ), "models/w_dynabox.mdl" );
	CBasePlayerAmmo::Spawn();
}

void CAmmoDynamiteBox::Precache( void )
{
	PRECACHE_MODEL( "models/w_dynabox.mdl" );
	PRECACHE_SOUND( "items/9mmclip1.wav" );
}

BOOL CAmmoDynamiteBox::AddAmmo( CBaseEntity *pOther )
{
	if ( pOther->GiveAmmo( DYNAMITE_MAX_CARRY, "dynamite", DYNAMITE_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pOther->pev ), CHAN_ITEM, "items/9mmclip1.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
	return FALSE;
}
