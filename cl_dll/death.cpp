/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
// death notice
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "vgui_TeamFortressViewport.h"

DECLARE_MESSAGE( m_DeathNotice, DeathMsg );

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	char szWeapon[32]; // Shepard
	int iSuicide;
	int iTeamKill;
	int iNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

float g_ColorBlue[3]	= { 0.6, 0.8, 1.0 };
float g_ColorRed[3]		= { 1.0, 0.25, 0.25 };
float g_ColorGreen[3]	= { 0.6, 1.0, 0.6 };
float g_ColorYellow[3]	= { 1.0, 0.7, 0.0 };
float g_ColorGrey[3]	= { 0.8, 0.8, 0.8 };
float g_ColorWhite[3] = { 1.0, 1.0, 1.0 }; // Shepard

float *GetClientColor( int clientIndex )
{
	switch ( g_PlayerExtraInfo[clientIndex].teamnumber )
	{
	case 1:	return g_ColorBlue;
	case 2: return g_ColorRed;
	case 3: return g_ColorYellow;
	case 4: return g_ColorGreen;
	case 5: return g_ColorWhite; // Shepard
	case 0: return g_ColorYellow;

		default	: return g_ColorGrey;
	}

	return NULL;
}

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	CVAR_CREATE( "hud_deathnotice_time", "6", 0 );

	return 1;
}


void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}


int CHudDeathNotice :: VidInit( void )
{
	return 1;
}

// Shepard
const char *GetWeaponNiceName( char *killedwith )
{
	if ( !strcmp( killedwith, "saber" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Saber" )->pMessage;
	else if ( !strcmp( killedwith, "debug" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Debug" )->pMessage;
	else if ( !strcmp( killedwith, "colt" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Colt" )->pMessage;
	else if ( !strcmp( killedwith, "henry" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Henry" )->pMessage;
	else if ( !strcmp( killedwith, "shotgun" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Shotgun" )->pMessage;
#ifdef USE_MP_WEAPONS
	else if ( !strcmp( killedwith, "bow" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Bow" )->pMessage;
#endif
	else if ( !strcmp( killedwith, "gatling" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Gatling" )->pMessage;
	else if ( !strcmp( killedwith, "dynamite" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_Dynamite" )->pMessage;
#ifdef USE_MP_WEAPONS
	else if ( !strcmp( killedwith, "halfcat" ) )
		return gEngfuncs.pfnTextMessageGet( "MP_Death_HalfCat" )->pMessage;
#endif
	else
		return gEngfuncs.pfnTextMessageGet( "MP_Death_World" )->pMessage;
}

int CHudDeathNotice :: Draw( float flTime )
{
	int x, y;

	for ( int i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if ( rgDeathNoticeList[i].flDisplayTime < flTime )
		{ // display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i+1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i) );
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = V_min( rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME );

		// Only draw if the viewport will let me
		if ( gViewPort && gViewPort->AllowedToPrintText() )
		{
			// Draw the death notice
			y = DEATHNOTICE_TOP + 2 + (20 * i);  //!!!

			// Shepard : reworked this part of the code to be text based instead of sprite based
			// Draw victim's name (if it was a player that was killed)
			x = ScreenWidth - ConsoleStringLen( rgDeathNoticeList[i].szVictim ) - 5;
			if ( rgDeathNoticeList[i].iNonPlayerKill == FALSE )
			{
				if ( rgDeathNoticeList[i].VictimColor )
					gEngfuncs.pfnDrawSetTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );

				x = DrawConsoleString( x, y, rgDeathNoticeList[i].szVictim );
				x -= ConsoleStringLen( rgDeathNoticeList[i].szVictim );
			}

			// Draw death weapon (with the correct color if it's a teamkill or not)
			if ( rgDeathNoticeList[i].iTeamKill )
				gEngfuncs.pfnDrawSetTextColor( g_ColorGreen[0], g_ColorGreen[1], g_ColorGreen[2] );
			else
				gEngfuncs.pfnDrawSetTextColor( g_ColorWhite[0], g_ColorWhite[1], g_ColorWhite[2] );

			const char *cWeaponName = GetWeaponNiceName( rgDeathNoticeList[i].szWeapon );
			x -= (5 + ConsoleStringLen( cWeaponName ));
			x = 5 + DrawConsoleString( x, y, cWeaponName );
			x -= (5 + ConsoleStringLen( cWeaponName ) );

			// Draw killer's name
			if ( !rgDeathNoticeList[i].iSuicide )
			{
				x -= (5 + ConsoleStringLen( rgDeathNoticeList[i].szKiller ) );
				if ( rgDeathNoticeList[i].KillerColor )
					gEngfuncs.pfnDrawSetTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );

				x = 5 + DrawConsoleString( x, y, rgDeathNoticeList[i].szKiller );
			}
		}
	}

	return 1;
}

// This message handler may be better off elsewhere
int CHudDeathNotice :: MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );

	int killer = READ_BYTE();
	int victim = READ_BYTE();

	// Shepard : I don't need that stupid "d_" part
	char killedwith[32];
	strncpy( killedwith, READ_STRING(), 32 );

	if (gViewPort)
		gViewPort->DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage(victim);

	int i;
	for ( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if ( i == MAX_DEATHNOTICES )
	{ // move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList+1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	if (gViewPort)
		gViewPort->GetAllPlayersInfo();

	// Get the Killer's name
	char *killer_name = g_PlayerInfoList[ killer ].name;
	if ( !killer_name )
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strncpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Get the Victim's name
	char *victim_name = NULL;
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if ( ((char)victim) != -1 )
		victim_name = g_PlayerInfoList[ victim ].name;
	if ( !victim_name )
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strncpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Is it a non-player object kill?
	if ( ((char)victim) == -1 )
	{
		rgDeathNoticeList[i].iNonPlayerKill = TRUE;

		// Store the object's name in the Victim slot
		strcpy( rgDeathNoticeList[i].szVictim, killedwith );
	}
	else
	{
		if ( killer == victim || killer == 0 )
			rgDeathNoticeList[i].iSuicide = TRUE;

		if ( !strcmp( killedwith, "teammate" ) )
			rgDeathNoticeList[i].iTeamKill = TRUE;
	}

	rgDeathNoticeList[i].iId = 1; // Shepard : Dirty hack to get the messages showing

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT( "hud_deathnotice_time" );
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	if (rgDeathNoticeList[i].iNonPlayerKill)
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed a " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
		ConsolePrint( "\n" );
	}
	else
	{
		// record the death notice in the console
		if ( rgDeathNoticeList[i].iSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			if ( !strcmp( killedwith, "world" ) )
			{
				ConsolePrint( " died" );
			}
			else
			{
				ConsolePrint( " killed self" );
			}
		}
		else if ( rgDeathNoticeList[i].iTeamKill )
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed his teammate " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}
		else
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}

		if ( killedwith && *killedwith && (*killedwith > 13 ) && strcmp( killedwith, "world" ) && !rgDeathNoticeList[i].iTeamKill )
		{
			ConsolePrint( " with " );
			ConsolePrint( killedwith );
		}

		ConsolePrint( "\n" );
	}

	// Shepard : The most important change in that message
	strncpy( rgDeathNoticeList[i].szWeapon, killedwith, 32 );

	return 1;
}
