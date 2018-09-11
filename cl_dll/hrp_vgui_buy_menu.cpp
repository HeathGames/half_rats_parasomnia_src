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
// HR:P VGUI buy menu
//=========================================================

#include "vgui_int.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

#define BUYMENU_WINDOW_X		XRES( 10 )
#define BUYMENU_WINDOW_Y		YRES( 10 )
#define BUYMENU_WINDOW_SIZE_X	XRES( 620 )
#define BUYMENU_WINDOW_SIZE_Y	YRES( 460 )

#define BUYMENU_TITLE_X			XRES( 20 )
#define BUYMENU_TITLE_Y			YRES( 20 )

#define BUYMENU_BUTTON_X		XRES( 20 )
#define BUYMENU_BUTTON_Y		80
#define BUYMENU_BUTTON_SIZE_X	XRES( 150 )
#define BUYMENU_BUTTON_SIZE_Y	YRES( 20 )

#define BUYMENU_SCROLL_PANEL_X		XRES( 180 )
#define BUYMENU_SCROLL_PANEL_Y		YRES( 60 )
#define BUYMENU_SCROLL_PANEL_SIZE_X	XRES( 420 )
#define BUYMENU_SCROLL_PANEL_SIZE_Y	YRES( 120 )

#define BUYMENU_IMAGE_X			XRES( 180 )
#define BUYMENU_IMAGE_Y			YRES( 140 )

CHRPBuyMenuPanel::CHRPBuyMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iTrans, iRemoveMe, x, y, wide, tall )
{
	// Create the base window
	m_pWindow = new CTransparentPanel( 255, BUYMENU_WINDOW_X, BUYMENU_WINDOW_Y, BUYMENU_WINDOW_SIZE_X, BUYMENU_WINDOW_SIZE_Y );
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
	m_pTitle = new Label( "", BUYMENU_TITLE_X, BUYMENU_TITLE_Y );
	m_pTitle->setFont( pTitleFont );
	m_pTitle->setParent( m_pWindow );
	pSchemes->getFgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setFgColor( iRed, iGreen, iBlue, iAlpha );
	pSchemes->getBgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setBgColor( iRed, iGreen, iBlue, iAlpha );
	m_pTitle->setContentAlignment( vgui::Label::a_west );
	m_pTitle->setText( gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Title" ) );

	// Create the buttons
	int i;
	for ( i = 0; i < BUYMENU_MAX_BUTTONS; i++ )
	{
		// Set button's core stuff
		m_pButtons[i] = new CommandButton( "", 0, 0, BUYMENU_BUTTON_SIZE_X, BUYMENU_BUTTON_SIZE_Y, true );
		m_pButtons[i]->setParent( m_pWindow );
		m_pButtons[i]->setContentAlignment( vgui::Label::a_west );
		m_pButtons[i]->setBorder( new LineBorder( Color( 255 * 0.7, 255 * 0.7, 255 * 0.7, 0 ) ) );

		// Set button's slot input
		char cButton[2];
		sprintf( cButton, "%d", i + 1 );
		m_pButtons[i]->setBoundKey( cButton[0] );

		// If this isn't the close button, set the buy command
		if ( i < BUYMENU_MAX_BUTTONS - 1 )
		{
			char cCommand[6];
			sprintf( cCommand, "buy %d", i + 1 );
			m_pButtons[i]->addActionSignal( new CMenuHandler_StringCommand( cCommand, true ) );
		}
		else
			m_pButtons[i]->addActionSignal( new CMenuHandler_TextWindow( HIDE_TEXTWINDOW ) );

		// Make those buttons close the menu and also set the appropriate names
		m_pButtons[i]->addInputSignal( new CHandler_MenuButtonOver( this, i ) );
		char cItemName[32];
		switch ( i )
		{
		case 0: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Cocaine" ) ); break;
		case 1: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Drymeat" ) ); break;
		case 2: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Meatpie" ) ); break;
		case 3: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Whiskey" ) ); break;
		case 4: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_WhaleOil" ) ); break;
		case 5: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Ammo_45" ) ); break;
		case 6: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Ammo_Buckshot" ) ); break;
		case 7: sprintf( cItemName, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Close" ) ); break;
		}
		m_pButtons[i]->setText( cItemName );
		m_pButtons[i]->setPos( BUYMENU_BUTTON_X, (BUYMENU_BUTTON_Y + ((BUYMENU_BUTTON_SIZE_Y + 10) * i)) );
	}

	// Create the description container
	m_pScrollPanel = new CTFScrollPanel( BUYMENU_SCROLL_PANEL_X, BUYMENU_SCROLL_PANEL_Y, BUYMENU_SCROLL_PANEL_SIZE_X, BUYMENU_SCROLL_PANEL_SIZE_Y );
	m_pScrollPanel->setParent( m_pWindow );
	m_pScrollPanel->setScrollBarAutoVisible( false, false );
	m_pScrollPanel->setScrollBarVisible( false, false );

	//m_pInfoText = new TextPanel( "", BUYMENU_SCROLL_PANEL_X, BUYMENU_SCROLL_PANEL_Y, BUYMENU_SCROLL_PANEL_SIZE_X, BUYMENU_SCROLL_PANEL_SIZE_Y );
	m_pInfoText = new TextPanel( "", 0, 0, BUYMENU_SCROLL_PANEL_SIZE_X, BUYMENU_SCROLL_PANEL_SIZE_Y );
	m_pInfoText->setParent( m_pScrollPanel->getClient() );
	m_pInfoText->setFont( pTextFont );
	pSchemes->getFgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setFgColor( iRed, iGreen, iBlue, iAlpha );
	pSchemes->getBgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setBgColor( iRed, iGreen, iBlue, iAlpha );
	m_pInfoText->setText( "" );
	m_pScrollPanel->validate();

	// Create the images
	memset( m_pItemsImages, 0, sizeof( m_pItemsImages ) );
	m_pItemsImages[0] = new CImageLabel( "cocaine", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[1] = new CImageLabel( "drymeat", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[2] = new CImageLabel( "meatpie", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[3] = new CImageLabel( "whiskey", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[4] = new CImageLabel( "whaleoil", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[5] = new CImageLabel( "ammo_45", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	m_pItemsImages[6] = new CImageLabel( "ammo_buckshot", BUYMENU_IMAGE_X, BUYMENU_IMAGE_Y );
	for ( i = 0; i < BUYMENU_MAX_IMAGES; i++ )
	{
		//m_pItemsImages[i]->setSize( m_pItemsImages[i]->getImageWide(), m_pItemsImages[i]->getImageTall() );
		m_pItemsImages[i]->setParent( m_pWindow );
		m_pItemsImages[i]->setVisible( false );
	}

	// Finalize
	Initialize();
}

void CHRPBuyMenuPanel::Initialize( void )
{
}

void CHRPBuyMenuPanel::Reset( void )
{
	CMenuPanel::Reset();
}

bool CHRPBuyMenuPanel::SlotInput( int iSlot ) 
{ 
	// Not part of a bind
	if ( iSlot < 0 || iSlot > BUYMENU_MAX_BUTTONS - 1 )
		return false;

	if ( m_pButtons[iSlot] )
	{    
		m_pButtons[iSlot]->fireActionSignal();
		return true;
	} 

	return false;
}

void CHRPBuyMenuPanel::SetActiveInfo( int iInput ) 
{
	int i;
	for ( i = 0; i < BUYMENU_MAX_BUTTONS; i++ )
		m_pButtons[i]->setArmed( false );

	m_pInfoText->setText( "" );
	m_pButtons[iInput]->setArmed( true );

	for ( i = 0; i < BUYMENU_MAX_IMAGES; i++ )
		m_pItemsImages[i]->setVisible( false );

	if ( iInput == BUYMENU_MAX_BUTTONS - 1 )
		return;

	char cDescription[1024];
	switch ( iInput )
	{
	case 0: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Cocaine_Desc" ) ); break;
	case 1: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Drymeat_Desc" ) ); break;
	case 2: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Meatpie_Desc" ) ); break;
	case 3: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Whiskey_Desc" ) ); break;
	case 4: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_WhaleOil_Desc" ) ); break;
	case 5: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Ammo_45_Desc" ) ); break;
	case 6: sprintf( cDescription, "%s", gHUD.m_TextMessage.BufferedLocaliseTextString( "#VGUI_BuyMenu_Ammo_Buckshot_Desc" ) ); break;
	}
	m_pInfoText->setText( cDescription );
	m_pItemsImages[iInput]->setVisible( true );
}
