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
// HR:P HUD item meat pie
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE( m_ItemMeatpie, ItemMeatpie );

int CHudItemMeatpie::Init( void )
{
	HOOK_MESSAGE( ItemMeatpie );
	gHUD.AddHudElem( this );
	m_flFade = 0.0f;
	m_iCount = 0;
	m_hIcon = 0;
	return 1;
}

int CHudItemMeatpie::VidInit( void )
{
	m_hIcon = gHUD.GetSpriteIndex( "meatpie" );
	return 1;
}

int CHudItemMeatpie::MsgFunc_ItemMeatpie( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iFlags |= HUD_ACTIVE;
	m_flFade = 200.0f;
	m_iCount = READ_BYTE();
	return 1;
}

int CHudItemMeatpie::Draw( float flTime )
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

	// Draw the meat pie icon
	int iX = ScreenWidth - 64.0f;
	int iY = 320.0f;
	SPR_Set( gHUD.GetSprite( m_hIcon ), iRed, iGreen, iBlue );
	SPR_DrawAdditive( 0, iX, iY, &gHUD.GetSpriteRect( m_hIcon ) );

	// Draw the amount of meat pie
	int iNumberWidth = gHUD.GetSpriteRect( gHUD.m_HUD_number_0 ).right - gHUD.GetSpriteRect( gHUD.m_HUD_number_0 ).left;
	iX -= gHUD.GetNumWidth( m_iCount, DHN_DRAWZERO );
	gHUD.DrawHudNumber( iX - 40.0f, iY + 16.0f, DHN_DRAWZERO, m_iCount, iRed, iGreen, iBlue );

	return 1;
}
