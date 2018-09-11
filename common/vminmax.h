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

// Shepard : Fix for GCC 6 and VS 2015 (thanks malortie)

#ifndef VMINMAX_H
#define VMINMAX_H

#define V_max(a, b)	((a > b) ? a : b)
#define V_min(a, b)	((a < b) ? a : b)
#define V_fabs(x)	((x > 0) ? x : 0 - x)

#endif // VMINMAX_H

