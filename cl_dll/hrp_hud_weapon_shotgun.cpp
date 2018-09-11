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
// HR:P HUD weapon shotgun
//=========================================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE( m_WeaponShotgun, WeapShot );

int CHudWeaponShotgun::Init( void )
{
	HOOK_MESSAGE( WeapShot );
	gHUD.AddHudElem( this );
	m_bLeftShellState = m_bOldLeftShellState = true;
	m_bRightShellState = m_bOldRightShellState = true;
	m_flFade = 0.0f;
	m_hLeftShellIcon = 0;
	m_hRightShellIcon = 0;
	return 1;
}

int CHudWeaponShotgun::VidInit( void )
{
	m_hLeftShellIcon = gHUD.GetSpriteIndex( "buckshot" );
	m_hRightShellIcon = gHUD.GetSpriteIndex( "buckshot" );
	return 1;
}

int CHudWeaponShotgun::MsgFunc_WeapShot( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iFlags |= HUD_ACTIVE;
	m_bLeftShellState = READ_BYTE() ? true : false;
	m_bRightShellState = READ_BYTE() ? true : false;
	if ( m_bOldLeftShellState != m_bLeftShellState || m_bRightShellState != m_bOldRightShellState )
		m_flFade = 200.0f;

	m_bOldLeftShellState = m_bLeftShellState;
	m_bOldRightShellState = m_bRightShellState;
	return 1;
}

int CHudWeaponShotgun::Draw( float flTime )
{
	// Don't show this if the player don't want to
	if ( gEngfuncs.pfnGetCvarFloat( "cl_hrp_show_shotgun_indicators" ) <= 0.0f )
		return 1;

	// Don't show this if the health HUD is ordered to hide or we are a spectator
	if ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH || gEngfuncs.IsSpectateOnly() )
		return 1;

	// Don't show this if we don't have the HUD (suit)
	if ( !(gHUD.m_iWeaponBits & (1 << WEAPON_SUIT)) )
		return 1;

	// Don't show this if we don't have the shotgun in our hands
	if ( gHUD.m_Ammo.m_pWeapon && gHUD.m_Ammo.m_pWeapon->iId != 5 ) // 5 = WEAPON_SHOTGUN
		return 1;

	// Calculate fading
	int iAlpha = (int)V_max( MIN_ALPHA, m_flFade );
	if ( m_flFade > 0.0f )
		m_flFade -= gHUD.m_flTimeDelta * 20.0f;

	// Determine which color to use for the left shell
	int iRed, iGreen, iBlue;
	UnpackRGB( iRed, iGreen, iBlue, (m_bLeftShellState) ? RGB_REDISH : RGB_YELLOWISH );
	ScaleColors( iRed, iGreen, iBlue, iAlpha );

	// Draw the left shell icon
	int iX;
	int iY;
	if ( gEngfuncs.pfnGetCvarFloat( "cl_hrp_show_shotgun_indicators" ) == 1.0f )
	{
		iX = ScreenWidth - 192.0f;
		iY = ScreenHeight - gHUD.m_iFontHeight * 2.0f - 16.0f;
	}
	else
	{
		iX = ScreenWidth / 2.0f - 32.0f;
		iY = ScreenHeight / 2.0f - 12.0f;
	}
	SPR_Set( gHUD.GetSprite( m_hLeftShellIcon ), iRed, iGreen, iBlue );
	SPR_DrawAdditive( 0, iX, iY, &gHUD.GetSpriteRect( m_hLeftShellIcon ) );

	// Determine which color to use for the right shell
	UnpackRGB( iRed, iGreen, iBlue, (m_bRightShellState) ? RGB_REDISH : RGB_YELLOWISH );
	ScaleColors( iRed, iGreen, iBlue, iAlpha );

	// Draw the right shell icon
	iX += gEngfuncs.pfnGetCvarFloat( "cl_hrp_show_shotgun_indicators" ) == 1.0f ? 16.0f : 40.0f;
	SPR_Set( gHUD.GetSprite( m_hRightShellIcon ), iRed, iGreen, iBlue );
	SPR_DrawAdditive( 0, iX, iY, &gHUD.GetSpriteRect( m_hRightShellIcon ) );

	return 1;
}
