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
// AI marker for Half-Cat
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "hrp_entity_ai_marker.h"

LINK_ENTITY_TO_CLASS( ai_marker, CAIMarker );

void CAIMarker::Spawn( void )
{
	Precache();
	pev->classname = MAKE_STRING( "ai_marker" );
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_NOT;
	SET_MODEL( ENT( pev ), "models/fish.mdl" );
}

void CAIMarker::Precache( void )
{
	PRECACHE_MODEL( "models/fish.mdl" );
}
