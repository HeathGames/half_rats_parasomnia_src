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

//=========================================================
// HR:P weapons IDs definition
//=========================================================

// The reason of why some weapons IDs are here is because they are both used by
// the client and server project. The other weapons IDs are stored in
// "weapons.h" server-side because they aren't needed in the client.

#ifndef HRP_WEAPONS_DEFINES_H
#define HRP_WEAPONS_DEFINES_H

#define WEAPON_DEBUG	2
#define WEAPON_COLT		3
#define WEAPON_HENRY	4
#define WEAPON_SHOTGUN	5
#ifdef USE_MP_WEAPONS
#define WEAPON_BOW		6
#endif
#define WEAPON_GATLING	7

#endif // HRP_WEAPONS_DEFINES_H
