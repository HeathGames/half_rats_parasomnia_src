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
// HR:P VGUI new game
//=========================================================

#include "vgui_int.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

#define NEWGAMEMENU_WINDOW_X			XRES( 10 )
#define NEWGAMEMENU_WINDOW_Y			YRES( 10 )
#define NEWGAMEMENU_WINDOW_SIZE_X		XRES( 620 )
#define NEWGAMEMENU_WINDOW_SIZE_Y		YRES( 460 )

#define NEWGAMEMENU_TITLE_X				XRES( 20 )
#define NEWGAMEMENU_TITLE_Y				YRES( 20 )

#define NEWGAMEMENU_BUTTON_X			XRES( 20 )
#define NEWGAMEMENU_BUTTON_Y			80
#define NEWGAMEMENU_BUTTON_SIZE_X		XRES( 150 )
#define NEWGAMEMENU_BUTTON_SIZE_Y		YRES( 20 )

#define NEWGAMEMENU_SCROLL_PANEL_X		XRES( 180 )
#define NEWGAMEMENU_SCROLL_PANEL_Y		YRES( 60 )
#define NEWGAMEMENU_SCROLL_PANEL_SIZE_X	XRES( 420 )
#define NEWGAMEMENU_SCROLL_PANEL_SIZE_Y	YRES( 120 )

CHRPNewGameMenuPanel::CHRPNewGameMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iTrans, iRemoveMe, x, y, wide, tall )
{
	// Create the base window
	m_pWindow = new CTransparentPanel( 255, NEWGAMEMENU_WINDOW_X, NEWGAMEMENU_WINDOW_Y, NEWGAMEMENU_WINDOW_SIZE_X, NEWGAMEMENU_WINDOW_SIZE_Y );
	m_pWindow->setParent( this );
	m_pWindow->setBorder( new LineBorder( Color( 255 * 0.7, 255 * 0.7, 255 * 0.7, 0 ) ) );

	// Get the scheme
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "Briefing Text" );

	// Get the fonts
	Font *pTitleFont = pSchemes->getFont( hTitleScheme );
	Font *pTextFont = pSchemes->getFont( hTextScheme );
	int iRed, iGreen, iBlue, iAlpha;

	// Create the title
	m_pTitle = new Label( "", NEWGAMEMENU_TITLE_X, NEWGAMEMENU_TITLE_Y );
	m_pTitle->setFont( pTitleFont );
	m_pTitle->setParent( m_pWindow );
	pSchemes->getFgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setFgColor( iRed, iGreen, iBlue, iAlpha );
	pSchemes->getBgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setBgColor( iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setContentAlignment( vgui::Label::a_west );
	m_pTitle->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Title" ) );

	// Create the buttons
	m_pButtonEasy = new CommandButton( "", 0, 0, NEWGAMEMENU_BUTTON_SIZE_X, NEWGAMEMENU_BUTTON_SIZE_Y, true );
	m_pButtonEasy->setParent( m_pWindow );
	m_pButtonEasy->setContentAlignment( vgui::Label::a_west );
	m_pButtonEasy->setBorder( new LineBorder( Color( 255 * 0.7, 255 * 0.7, 255 * 0.7, 0 ) ) );
	m_pButtonEasy->setBoundKey( '1' );
	m_pButtonEasy->addActionSignal( new CMenuHandler_StringCommand( "skill 1", true ) );
	//m_pButtonEasy->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
	m_pButtonEasy->addInputSignal( new CHandler_MenuButtonOver( this, 0 ) );
	m_pButtonEasy->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Easy" ) );
	m_pButtonEasy->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + (NEWGAMEMENU_BUTTON_SIZE_Y + 10)) );

	m_pButtonMedium = new CommandButton( "", 0, 0, NEWGAMEMENU_BUTTON_SIZE_X, NEWGAMEMENU_BUTTON_SIZE_Y, true );
	m_pButtonMedium->setParent( m_pWindow );
	m_pButtonMedium->setContentAlignment( vgui::Label::a_west );
	m_pButtonMedium->setBorder( new LineBorder( Color( 255 * 0.7, 255 * 0.7, 255 * 0.7, 0 ) ) );
	m_pButtonMedium->setBoundKey( '2' );
	m_pButtonMedium->addActionSignal( new CMenuHandler_StringCommand( "skill 2", true ) );
	//m_pButtonMedium->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
	m_pButtonMedium->addInputSignal( new CHandler_MenuButtonOver( this, 1 ) );
	m_pButtonMedium->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Medium" ) );
	m_pButtonMedium->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + ((NEWGAMEMENU_BUTTON_SIZE_Y + 10) * 2)) );

	m_pButtonHard = new CommandButton( "", 0, 0, NEWGAMEMENU_BUTTON_SIZE_X, NEWGAMEMENU_BUTTON_SIZE_Y, true );
	m_pButtonHard->setParent( m_pWindow );
	m_pButtonHard->setContentAlignment( vgui::Label::a_west );
	m_pButtonHard->setBorder( new LineBorder( Color( 255 * 0.7, 255 * 0.7, 255 * 0.7, 0 ) ) );
	m_pButtonHard->setBoundKey( '3' );
	m_pButtonHard->addActionSignal( new CMenuHandler_StringCommand( "skill 3", true ) );
	//m_pButtonHard->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );
	m_pButtonHard->addInputSignal( new CHandler_MenuButtonOver( this, 2 ) );
	m_pButtonHard->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Hard" ) );
	m_pButtonHard->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + ((NEWGAMEMENU_BUTTON_SIZE_Y + 10) * 3)) );

	// Create the description container
	m_pScrollPanel = new CTFScrollPanel( NEWGAMEMENU_SCROLL_PANEL_X, NEWGAMEMENU_SCROLL_PANEL_Y, NEWGAMEMENU_SCROLL_PANEL_SIZE_X, NEWGAMEMENU_SCROLL_PANEL_SIZE_Y );
	m_pScrollPanel->setParent( m_pWindow );
	m_pScrollPanel->setScrollBarAutoVisible( false, false );
	m_pScrollPanel->setScrollBarVisible( false, false );

	m_pInfoText = new TextPanel( "", 0, 0, NEWGAMEMENU_SCROLL_PANEL_SIZE_X, NEWGAMEMENU_SCROLL_PANEL_SIZE_Y );
	m_pInfoText->setParent( m_pScrollPanel->getClient() );
	m_pInfoText->setFont( pTextFont );
	pSchemes->getFgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setFgColor( iRed, iGreen, iBlue, iAlpha );
	pSchemes->getBgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setBgColor( iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setText( "" );
	m_pScrollPanel->validate();

	m_pCheckBoxBonusDebugWeapon = new CCheckButton2();
	m_pCheckBoxBonusDebugWeapon->setParent( m_pWindow );
	m_pCheckBoxBonusDebugWeapon->SetImages( "gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga" );
	m_pCheckBoxBonusDebugWeapon->SetText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Debug_Weapon" ) );
	m_pCheckBoxBonusDebugWeapon->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + ((NEWGAMEMENU_BUTTON_SIZE_Y + 10) * 4)) );
	m_pCheckBoxBonusDebugWeapon->SetCheckboxLeft( true );
	m_pCheckBoxBonusDebugWeapon->SetChecked( false );
	m_pCheckBoxBonusDebugWeapon->addInputSignal( new CHandler_MenuButtonOver( this, 3 ) );
	m_pCheckBoxBonusDebugWeapon->SetHandler( this );

	m_pCheckBoxBonusDrunk = new CCheckButton2();
	m_pCheckBoxBonusDrunk->setParent( m_pWindow );
	m_pCheckBoxBonusDrunk->SetImages( "gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga" );
	m_pCheckBoxBonusDrunk->SetText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Drunk" ) );
	m_pCheckBoxBonusDrunk->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + ((NEWGAMEMENU_BUTTON_SIZE_Y + 10) * 5)) );
	m_pCheckBoxBonusDrunk->SetCheckboxLeft( true );
	m_pCheckBoxBonusDrunk->SetChecked( false );
	m_pCheckBoxBonusDrunk->addInputSignal( new CHandler_MenuButtonOver( this, 4 ) );
	m_pCheckBoxBonusDrunk->SetHandler( this );

	m_pCheckBoxBonusFragile = new CCheckButton2();
	m_pCheckBoxBonusFragile->setParent( m_pWindow );
	m_pCheckBoxBonusFragile->SetImages( "gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga" );
	m_pCheckBoxBonusFragile->SetText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Fragile" ) );
	m_pCheckBoxBonusFragile->setPos( NEWGAMEMENU_BUTTON_X, (NEWGAMEMENU_BUTTON_Y + ((NEWGAMEMENU_BUTTON_SIZE_Y + 10) * 6)) );
	m_pCheckBoxBonusFragile->SetCheckboxLeft( true );
	m_pCheckBoxBonusFragile->SetChecked( false );
	m_pCheckBoxBonusFragile->addInputSignal( new CHandler_MenuButtonOver( this, 5 ) );
	m_pCheckBoxBonusFragile->SetHandler( this );

	// Finalize
	Initialize();
}

void CHRPNewGameMenuPanel::Initialize( void )
{
}

void CHRPNewGameMenuPanel::Reset( void )
{
	CMenuPanel::Reset();
}

bool CHRPNewGameMenuPanel::SlotInput( int iSlot ) 
{ 
	// Not part of a bind
	if ( iSlot < 0 || iSlot > 2 )
		return false;

	switch ( iSlot )
	{
	case 0:
		m_pButtonEasy->fireActionSignal();
		return true;
		break;
	case 1:
		m_pButtonMedium->fireActionSignal();
		return true;
		break;
	case 2:
		m_pButtonHard->fireActionSignal();
		return true;
		break;
	default: return false;
	}
}

void CHRPNewGameMenuPanel::SetActiveInfo( int iInput ) 
{
	m_pButtonEasy->setArmed( false );
	m_pButtonMedium->setArmed( false );
	m_pButtonHard->setArmed( false );
	m_pInfoText->setText( "" );

	switch ( iInput )
	{
	case 0: m_pButtonEasy->setArmed( true ); break;
	case 1: m_pButtonMedium->setArmed( true ); break;
	case 2: m_pButtonHard->setArmed( true ); break;
	}

	if ( iInput < 0 || iInput > 5 )
		return;

	char cDescription[1024];
	switch ( iInput )
	{
	case 0: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Easy_Desc" ) ); break;
	case 1: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Medium_Desc" ) ); break;
	case 2: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Hard_Desc" ) ); break;
	case 3: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Debug_Weapon_Desc" ) ); break;
	case 4: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Drunk_Desc" ) ); break;
	case 5: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_NewGameMenu_Bonus_Fragile_Desc" ) ); break;
	}
	m_pInfoText->setText( cDescription );
}

void CHRPNewGameMenuPanel::StateChanged( CCheckButton2 *pButton )
{
	if ( pButton == m_pCheckBoxBonusDebugWeapon )
	{
		if ( pButton->IsChecked() )
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_debug_weapon 1" );
		else
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_debug_weapon 0" );
	}
	else if ( pButton == m_pCheckBoxBonusDrunk )
	{
		if ( pButton->IsChecked() )
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_drunk 1" );
		else
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_drunk 0" );
	}
	else if ( pButton == m_pCheckBoxBonusFragile )
	{
		if ( pButton->IsChecked() )
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_fragile 1" );
		else
			gEngfuncs.pfnClientCmd( "sv_hrp_bonus_fragile 0" );
	}
}

void CHRPNewGameMenuPanel::UpdatePositions( void )
{
	if ( m_bGameIsFinished )
	{
		m_pCheckBoxBonusDebugWeapon->setVisible( true );
		m_pCheckBoxBonusDrunk->setVisible( true );
		m_pCheckBoxBonusFragile->setVisible( true );
	}
	else
	{
		m_pCheckBoxBonusDebugWeapon->setVisible( false );
		m_pCheckBoxBonusDrunk->setVisible( false );
		m_pCheckBoxBonusFragile->setVisible( false );
	}
}
