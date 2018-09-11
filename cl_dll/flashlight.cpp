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
//
// flashlight.cpp
//
// implementation of CHudFlashlight class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>



DECLARE_MESSAGE(m_Flash, FlashBat)
DECLARE_MESSAGE(m_Flash, Flashlight)

#define BAT_NAME "sprites/%d_Flashlight.spr"

int CHudFlashlight::Init(void)
{
	m_fFade = 0;
	m_fOn = 0;

	HOOK_MESSAGE(Flashlight);
	HOOK_MESSAGE(FlashBat);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudFlashlight::Reset(void)
{
	m_fFade = 0;
	m_fOn = 0;
}

int CHudFlashlight::VidInit(void)
{
	int HUD_flash_empty = gHUD.GetSpriteIndex( "flash_empty" );
	int HUD_flash_full = gHUD.GetSpriteIndex( "flash_full" );
	int HUD_flash_beam = gHUD.GetSpriteIndex( "flash_beam" );

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	// HR:P lantern
	int HUD_lantern_off = gHUD.GetSpriteIndex( "lantern_off" );
	int HUD_lantern_on = gHUD.GetSpriteIndex( "lantern_on" );
	m_hLanternOff = gHUD.GetSprite( HUD_lantern_off );
	m_hLanternOn = gHUD.GetSprite( HUD_lantern_on );

	return 1;
};

int CHudFlashlight:: MsgFunc_FlashBat(const char *pszName,  int iSize, void *pbuf )
{

	
	BEGIN_READ( pbuf, iSize );
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x)/100.0;

	return 1;
}

int CHudFlashlight:: MsgFunc_Flashlight(const char *pszName,  int iSize, void *pbuf )
{

	BEGIN_READ( pbuf, iSize );
	m_fOn = READ_BYTE();
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x)/100.0;

	return 1;
}

int CHudFlashlight::Draw(float flTime)
{
	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_FLASHLIGHT | HIDEHUD_ALL ) )
		return 1;

	int r, g, b, x, y, a;
	wrect_t rc;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)) ))
		return 1;

	if (m_fOn)
		a = 225;
	else
		a = MIN_ALPHA;

	if (m_flBat < 0.20)
		UnpackRGB(r,g,b, RGB_REDISH);
	else
		UnpackRGB(r,g,b, RGB_YELLOWISH);

	ScaleColors(r, g, b, a);

	y = (m_prc1->bottom - m_prc2->top)/2;
	x = ScreenWidth - m_iWidth - m_iWidth/2 ;

	// HR:P lantern
	rc = *m_prc2;
	m_prc1 = &gHUD.GetSpriteRect( gHUD.GetSpriteIndex( "flash_empty" ) );
	m_prc2 = &gHUD.GetSpriteRect( gHUD.GetSpriteIndex( "flash_full" ) );

	int m_iHeight = m_prc2->bottom - m_prc1->top;
	rc.top += m_iHeight * ((float)(100.0f - (V_min( 100, m_iBat ))) * 0.01f);

	int iOffset = (m_prc1->bottom - m_prc1->top) / 6;
	SPR_Set( m_hSprite1, r, g, b );
	SPR_DrawHoles( 0, x, y - iOffset, m_prc1 );

	if ( rc.bottom > rc.top )
	{
		SPR_Set( m_hSprite2, r, g, b );
		SPR_DrawHoles( 0, x, y - iOffset + (rc.top - m_prc2->top), &rc );
	}

	// Draw the flashlight beam
	// Shepard : HACK
	wrect_t *pFix = &gHUD.GetSpriteRect( m_fOn ? m_hLanternOn : m_hLanternOff );
	pFix->right += 32.0f;
	pFix->bottom += 32.0f;

	x = ScreenWidth - 64.0f;
	SPR_Set( m_fOn ? m_hLanternOn : m_hLanternOff, 255, 255, 255 );
	SPR_DrawAdditive( 0, x, y - 12.0f, pFix );
	return 1;
}
