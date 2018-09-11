/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "../hud.h"
#include "../cl_util.h"
#include "event_api.h"

extern "C"
{
// HR:P
void EV_ColtFire( struct event_args_s *args );
void EV_HenryFire( struct event_args_s *args );
void EV_ShotgunFire( struct event_args_s *args );
void EV_GatlingFire( struct event_args_s *args );

void EV_TrainPitchAdjust( struct event_args_s *args );
}

/*
======================
Game_HookEvents

Associate script file name with callback functions.  Callback's must be extern "C" so
 the engine doesn't get confused about name mangling stuff.  Note that the format is
 always the same.  Of course, a clever mod team could actually embed parameters, behavior
 into the actual .sc files and create a .sc file parser and hook their functionality through
 that.. i.e., a scripting system.

That was what we were going to do, but we ran out of time...oh well.
======================
*/
void Game_HookEvents( void )
{
	gEngfuncs.pfnHookEvent( "events/colt.sc",					EV_ColtFire );
	gEngfuncs.pfnHookEvent( "events/henry.sc",					EV_HenryFire );
	gEngfuncs.pfnHookEvent( "events/shotgun.sc",				EV_ShotgunFire );
	gEngfuncs.pfnHookEvent( "events/gatling.sc",				EV_GatlingFire );

	gEngfuncs.pfnHookEvent( "events/train.sc",					EV_TrainPitchAdjust );
}
