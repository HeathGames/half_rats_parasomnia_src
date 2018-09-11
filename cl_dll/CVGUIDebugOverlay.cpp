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
// CVGUIDebugOverlay - VGUI debugging overlay
// @author Shepard
//=========================================================

#include "hud.h"
#include "cl_util.h"

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"

#include "CVGUIDebugOverlay.h"

#include "git.h"

// Static information
#ifdef WIN32
const char *g_cDebugOverlayPlatformValue = "Win32";
#elif LINUX
const char *g_cDebugOverlayPlatformValue = "Linux";
#else
const char *g_cDebugOverlayPlatformValue = "Unknown";
#endif

#ifdef _DEBUG
const char *g_cDebugOverlayConfigurationValue = "Debug";
#else
const char *g_cDebugOverlayConfigurationValue = "Release";
#endif

CVGUIDebugOverlay::CVGUIDebugOverlay() : CTransparentPanel( 255, DEBUG_OVERLAY_WINDOW_X, DEBUG_OVERLAY_WINDOW_Y, DEBUG_OVERLAY_WINDOW_SIZE_X, DEBUG_OVERLAY_WINDOW_SIZE_Y )
{
	CSchemeManager *pSchemeManager = gViewPort->GetSchemeManager();
	SchemeHandle_t hSchemeText = pSchemeManager->getSchemeHandle( "Briefing Text" );

	// Title
	Label *pLabelTitle = new Label( "Half-Rats: Parasomnia - DEBUGGING OVERLAY", XRES( 0 ), YRES( 0 ) );
	pLabelTitle->setParent( this );
	pLabelTitle->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelTitle->setBgColor( 0, 0, 0, 255 );
	pLabelTitle->setFgColor( DEBUG_OVERLAY_TITLE_COLOR_R, DEBUG_OVERLAY_TITLE_COLOR_G, DEBUG_OVERLAY_TITLE_COLOR_B, 0 );
	pLabelTitle->setContentAlignment( vgui::Label::a_west );

	// Version
	Label *pLabelVersionKey = new Label( "Version:", XRES( 0 ), YRES( 16 ) );
	pLabelVersionKey->setParent( this );
	pLabelVersionKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelVersionKey->setBgColor( 0, 0, 0, 255 );
	pLabelVersionKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelVersionKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelVersionValue = new Label( "1.0b", XRES( 128 ), YRES( 16 ) );
	pLabelVersionValue->setParent( this );
	pLabelVersionValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelVersionValue->setBgColor( 0, 0, 0, 255 );
	pLabelVersionValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelVersionValue->setContentAlignment( vgui::Label::a_west );

	// Platform - Win32 / Linux / Mac
	Label *pLabelPlatformKey = new Label( "Platform:", XRES( 0 ), YRES( 32 ) );
	pLabelPlatformKey->setParent( this );
	pLabelPlatformKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelPlatformKey->setBgColor( 0, 0, 0, 255 );
	pLabelPlatformKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelPlatformKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelPlatformValue = new Label( g_cDebugOverlayPlatformValue, XRES( 128 ), YRES( 32 ) );
	pLabelPlatformValue->setParent( this );
	pLabelPlatformValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelPlatformValue->setBgColor( 0, 0, 0, 255 );
	pLabelPlatformValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelPlatformValue->setContentAlignment( vgui::Label::a_west );

	// Engine - Legacy (WON/Xash3D) / Steam
	Label *pLabelEngineKey = new Label( "Engine:", XRES( 0 ), YRES( 48 ) );
	pLabelEngineKey->setParent( this );
	pLabelEngineKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelEngineKey->setBgColor( 0, 0, 0, 255 );
	pLabelEngineKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelEngineKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelEngineValue = new Label( "Steam", XRES( 128 ), YRES( 48 ) );
	pLabelEngineValue->setParent( this );
	pLabelEngineValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelEngineValue->setBgColor( 0, 0, 0, 255 );
	pLabelEngineValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelEngineValue->setContentAlignment( vgui::Label::a_west );

	// Configuration - Debug / Release
	Label *pLabelConfigurationKey = new Label( "Configuration:", XRES( 0 ), YRES( 64 ) );
	pLabelConfigurationKey->setParent( this );
	pLabelConfigurationKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelConfigurationKey->setBgColor( 0, 0, 0, 255 );
	pLabelConfigurationKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelConfigurationKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelConfigurationValue = new Label( g_cDebugOverlayConfigurationValue, XRES( 128 ), YRES( 64 ) );
	pLabelConfigurationValue->setParent( this );
	pLabelConfigurationValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelConfigurationValue->setBgColor( 0, 0, 0, 255 );
	pLabelConfigurationValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelConfigurationValue->setContentAlignment( vgui::Label::a_west );

	// Git commit hash - SHA-1
	Label *pLabelGitCommitHashKey = new Label( "Git commit hash:", XRES( 0 ), YRES( 80 ) );
	pLabelGitCommitHashKey->setParent( this );
	pLabelGitCommitHashKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelGitCommitHashKey->setBgColor( 0, 0, 0, 255 );
	pLabelGitCommitHashKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelGitCommitHashKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelGitCommitHashValue = new Label( gcGitCommitHash, XRES( 128 ), YRES( 80 ) );
	pLabelGitCommitHashValue->setParent( this );
	pLabelGitCommitHashValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelGitCommitHashValue->setBgColor( 0, 0, 0, 255 );
	pLabelGitCommitHashValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelGitCommitHashValue->setContentAlignment( vgui::Label::a_west );

	// Build timestamp - XXXX-YY-ZZ AA:BB
	Label *pLabelBuildTimestampKey = new Label( "Build timestamp:", XRES( 0 ), YRES( 96 ) );
	pLabelBuildTimestampKey->setParent( this );
	pLabelBuildTimestampKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelBuildTimestampKey->setBgColor( 0, 0, 0, 255 );
	pLabelBuildTimestampKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelBuildTimestampKey->setContentAlignment( vgui::Label::a_west );
	Label *pLabelBuildTimestampValue = new Label( "", XRES( 128 ), YRES( 96 ) );
	pLabelBuildTimestampValue->setParent( this );
	pLabelBuildTimestampValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelBuildTimestampValue->setBgColor( 0, 0, 0, 255 );
	pLabelBuildTimestampValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	pLabelBuildTimestampValue->setContentAlignment( vgui::Label::a_west );
	pLabelBuildTimestampValue->setText( "%s %s", __DATE__, __TIME__);

	// Current map - Unknown (likely main menu) / paraX
	Label *pLabelMapKey = new Label( "Current map:", XRES( 0 ), YRES( 128 ) );
	pLabelMapKey->setParent( this );
	pLabelMapKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelMapKey->setBgColor( 0, 0, 0, 255 );
	pLabelMapKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelMapKey->setContentAlignment( vgui::Label::a_west );
	m_pLabelMapValue = new Label( "Unknown (likely main menu)", XRES( 128 ), YRES( 128 ) );
	m_pLabelMapValue->setParent( this );
	m_pLabelMapValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelMapValue->setBgColor( 0, 0, 0, 255 );
	m_pLabelMapValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	m_pLabelMapValue->setContentAlignment( vgui::Label::a_west );

	// Origin - X, Y, Z
	Label *pLabelOriginKey = new Label( "Origin (X, Y, Z):", XRES( 0 ), YRES( 144 ) );
	pLabelOriginKey->setParent( this );
	pLabelOriginKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelOriginKey->setBgColor( 0, 0, 0, 255 );
	pLabelOriginKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelOriginKey->setContentAlignment( vgui::Label::a_west );
	m_pLabelOriginValue = new Label( "Unknown", XRES( 128 ), YRES( 144 ) );
	m_pLabelOriginValue->setParent( this );
	m_pLabelOriginValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelOriginValue->setBgColor( 0, 0, 0, 255 );
	m_pLabelOriginValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	m_pLabelOriginValue->setContentAlignment( vgui::Label::a_west );

	// Angles - X, Y, Z
	Label *pLabelAnglesKey = new Label( "Angles (Pitch, Yaw, Roll):", XRES( 0 ), YRES( 160 ) );
	pLabelAnglesKey->setParent( this );
	pLabelAnglesKey->setFont( pSchemeManager->getFont( hSchemeText ) );
	pLabelAnglesKey->setBgColor( 0, 0, 0, 255 );
	pLabelAnglesKey->setFgColor( DEBUG_OVERLAY_KEY_COLOR_R, DEBUG_OVERLAY_KEY_COLOR_G, DEBUG_OVERLAY_KEY_COLOR_B, 0 );
	pLabelAnglesKey->setContentAlignment( vgui::Label::a_west );
	m_pLabelAnglesValue = new Label( "Unknown", XRES( 128 ), YRES( 160 ) );
	m_pLabelAnglesValue->setParent( this );
	m_pLabelAnglesValue->setFont( pSchemeManager->getFont( hSchemeText ) );
	m_pLabelAnglesValue->setBgColor( 0, 0, 0, 255 );
	m_pLabelAnglesValue->setFgColor( DEBUG_OVERLAY_VALUE_COLOR_R, DEBUG_OVERLAY_VALUE_COLOR_G, DEBUG_OVERLAY_VALUE_COLOR_B, 0 );
	m_pLabelAnglesValue->setContentAlignment( vgui::Label::a_west );
}

void CVGUIDebugOverlay::Update()
{
	if ( !strcmp( "", gEngfuncs.pfnGetLevelName() ) )
		m_pLabelMapValue->setText( "Unknown (likely main menu)" );
	else
		m_pLabelMapValue->setText( "%s", gEngfuncs.pfnGetLevelName() );

	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
	if ( pPlayer )
	{
		m_pLabelOriginValue->setText( "%f %f %f", pPlayer->origin.x, pPlayer->origin.y, pPlayer->origin.z );
		m_pLabelAnglesValue->setText( "%f %f %f", pPlayer->angles.x, pPlayer->angles.y, pPlayer->angles.z );
	}
}
