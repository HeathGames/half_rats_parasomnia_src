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
// HR:P HUD item selected
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#define ITEM_COCAINE	5
#define ITEM_DRYMEAT	6
#define ITEM_MEATPIE	7
#define ITEM_WHISKEY	8
#define ITEM_WHALEOIL	9

DECLARE_MESSAGE( m_ItemSelect, ItemSelect );

int CHudItemSelected::Init( void )
{
	HOOK_MESSAGE( ItemSelect );
	gHUD.AddHudElem( this );
	m_flFade = 0.0f;
	m_flUseTime = 0.0f;
	m_iItemID = ITEM_COCAINE;
	m_hIconSelected = 0;
	m_hIconUsed = 0;
	return 1;
}

int CHudItemSelected::VidInit( void )
{
	m_hIconSelected = gHUD.GetSpriteIndex( "select" );
	m_hIconUsed = gHUD.GetSpriteIndex( "use" );
	return 1;
}

int CHudItemSelected::MsgFunc_ItemSelect( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iFlags |= HUD_ACTIVE;
	m_iItemID = READ_BYTE();
	int iUse = READ_BYTE();
	if ( iUse )
		m_flUseTime = gEngfuncs.GetClientTime() + 1.0f;
	else
		m_flFade = 200.0f;

	return 1;
}

int CHudItemSelected::Draw( float flTime )
{
	// Don't show this if we are in a multiplayer game
	if ( gEngfuncs.GetMaxClients() > 1 )
		return 1;

	// Don't show this if the health HUD is ordered to hide or we are a spectator
	if ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH || gEngfuncs.IsSpectateOnly() )
		return 1;

	// Don't show this if we don't have the HUD (suit)
	if ( !(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)) )
		return 1;

	// Use standard HUD colors
	int iRed, iGreen, iBlue;
	UnpackRGB( iRed, iGreen, iBlue, RGB_YELLOWISH );

	// Calculate fading
	int iAlpha = (int)V_max( MIN_ALPHA, m_flFade );
	if ( m_flFade > 0.0f )
		m_flFade -= gHUD.m_flTimeDelta * 20.0f;

	ScaleColors( iRed, iGreen, iBlue, iAlpha );

	// Draw the selected icon
	int iX = ScreenWidth - 96.0f;
	int iY;
	switch ( m_iItemID )
	{
	case ITEM_COCAINE: iY = 160.0f; break;
	case ITEM_DRYMEAT: iY = 224.0f; break;
	case ITEM_MEATPIE: iY = 288.0f; break;
	case ITEM_WHISKEY: iY = 352.0f; break;
	case ITEM_WHALEOIL: iY = 416.0f; break;
	}

	// This is required to force the glow sprite during level transitions
	if ( strcmp( cOldMapName, gEngfuncs.pfnGetLevelName() ) != 0 )
	{
		m_flUseTime = -1.0f;
		strcpy( cOldMapName, gEngfuncs.pfnGetLevelName() );
	}

	// Draw the appropriate sprite depending if we used it or not
	if ( m_flUseTime > gEngfuncs.GetClientTime() )
	{
		SPR_Set( gHUD.GetSprite( m_hIconUsed ), iRed, iGreen, iBlue );
		SPR_DrawAdditive( 0, iX + 32, iY + 32, &gHUD.GetSpriteRect( m_hIconUsed ) );
	}
	else
	{
		SPR_Set( gHUD.GetSprite( m_hIconSelected ), iRed, iGreen, iBlue );
		SPR_DrawAdditive( 0, iX, iY, &gHUD.GetSpriteRect( m_hIconSelected ) );
		m_flUseTime = -1.0f;
	}
	return 1;
}
