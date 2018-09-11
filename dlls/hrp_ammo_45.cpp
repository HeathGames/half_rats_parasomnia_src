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
// .45 ammo box
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "hrp_ammo_45.h"

LINK_ENTITY_TO_CLASS( ammo_45, CAmmo45 );
LINK_ENTITY_TO_CLASS( ammo_357, CAmmo45 ); // Backward compatibility with Half-Life maps
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CAmmo45 ); // Backward compatibility with Half-Life maps
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CAmmo45 ); // Backward compatibility with Half-Life maps
LINK_ENTITY_TO_CLASS( ammo_glockclip, CAmmo45 ); // Backward compatibility with Half-Life maps

void CAmmo45::Spawn( void )
{
	Precache();
	SET_MODEL( ENT( pev ), "models/w_45ammobox.mdl" );
	CBasePlayerAmmo::Spawn();
}

void CAmmo45::Precache( void )
{
	PRECACHE_MODEL( "models/w_45ammobox.mdl" );
	PRECACHE_SOUND( "items/9mmclip1.wav" );
}

BOOL CAmmo45::AddAmmo( CBaseEntity *pOther )
{
	if ( pOther->GiveAmmo( 10, "45", _45_MAX_CARRY ) != -1 )
	{
		EMIT_SOUND( ENT( pOther->pev ), CHAN_ITEM, "items/9mmclip1.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
	return FALSE;
}
