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
// HR:P VGUI credits
//=========================================================

// TODO : Instead of hardcoding the credits here, use a file

#include "vgui_int.h"
#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

extern void IN_SetVisibleMouse( bool visible );
extern void IN_ResetMouse( void );

#define CREDITS_WINDOW_X		XRES( 10 )
#define CREDITS_WINDOW_Y		YRES( 10 )
#define CREDITS_WINDOW_SIZE_X	XRES( 620 )
#define CREDITS_WINDOW_SIZE_Y	YRES( 4000 )

const static char *gcCreditsData[NUMBER_OF_LINES] = {
	"#Heath Games is...",
	"*Half-Rats",
	"2D Art, Characters, Concept Art,",
	"Design, Half-Rats\' VA,",
	"Level Design, Modeling,",
	"Sound Effects, Story.",
	"*Alejandro Alonso",
	"2D Art, Animation, Modeling,",
	"Rigging.",
	"*Devan Miller",
	"Heath Games\' Resident VA.",
	// Shepard - Volvo plz T_T
#ifdef WIN32
	"*Joël \"Shepard62FR\" Troch",
#else
	"*Joel \"Shepard62FR\" Troch",
#endif
	"Lead AI Programmer, Lead Linux",
	"Programmer, Lead WON/Xash3D",
	"Programmer, Gameplay Programmer,",
	"Quality Assurance.",
	"*Nicholas \"CliffyV\" Welch",
	"2D Art, Animation, Rigging.",
	"*Netoxy",
	"2D Art, Animation, Modeling,",
	"Rigging.",
	"*Osiris",
	"Lead Gameplay Programmer,",
	"Design Help, Suggestions.",
	"*Thanos",
	"Design Help, Level Design,",
	"Suggestions, Writing Help.",
	"",
	"#Heath Games\' \"Satellites\"",
	"*Anjelica Jade",
	"Female NPC VA.",
	"*Claudio \"SysOp\" Ficara",
	"AI marker (fish) code from",
	"ARRANGEMENT.",
	"*Dominik \"ripieces\" Tugend",
	"Raw input fixes for Linux and Mac.",
	"*Laurie Cheers",
	"Spirit of Half-Life 1.2 creator",
	"(state system, multisource, light",
	"custom model/health, vanilla Half-Life",
	"fixes, multi_manager, freeze player",
	"and player speech system).",
	"*Marc-Antoine \"malortie\" Lortie",
	"Code fixes for modern GCC (Linux) and",
	"Visual Studio versions (Windows).",
	"*Mike Peterson",
	"Crazed Hermit VA.",
	"*Sam \"Solokiller\" Vanheer",
	"Armored Fucker\'s shield AI programming",
	"assistance, Doktor/Huntman\'s aiming",
	"code programming assistance, networking",
	"programming assistance.",
	"*abbadon (from The Whole Half-Life)",
	"Armored Fucker\'s shield AI programming",
	"assistance.",
	"*EXU_SCULLY",
	"Undead Pig concept.",
	"*Kruk",
	"Bernard Animations, Model, UV Maps.",
	"*MagnumPI",
	"Henry Rifle Model, UV Maps.",
	"*Qwertyus",
	"Fixes for missing NPCs eyes,",
	"Fixes for third person weapons models.",
	"*ts2do (from The Wavelength)",
	"Dynamic NPC step sound code based on",
	"materials.",
	"*ZikShadow",
	"Undead Cop with Henry rifle concept,",
	"various models/skins and Steam UI",
	"improvements.",
	"#Original Soundtrack by",
	"*Ague",
	"www.facebook.com/agueforyou",
	"#\"Waltz of the Carnies\"",
	"*Kevin MacLeod",
	"incompetech.com",
	"",
	"#Beta Testers",
	"*Seth \"Aldarith\" Byrnes",
	"*23-Down",
	"*EXU_SCULLY",
	"*GeneralRain01",
	"*Kirit0o",
	"*Niddala",
	"*PeliCan",
	"*Qwertyus",
	"*Thefunny711",
	"*Urby",
	"*Weperlol",
	"*ZikShadow",
	"",
	"#Special Thanks",
	"*Bolloxed",
	"www.bolloxed.com",
	"",
	"*Nyanner",
	"\"Because she has to be there <3\"",
	"Shepard",
	"",
	"*Oliver \"FRAG\" Curtis",
	"www.youtube.com/user/TGW121",
	"",
	"*ModDB",
	"www.moddb.com",
	"",
	"*PC Gamer",
	"www.pcgamer.com",
	"",
	"*Phillip Marlowe and DonAKAUnq from",
	"*RunThinkShootLive",
	"*(previously PlanetPhillip)",
	"www.runthinkshootlive.com",
	"",
	"*Source 2 Source - Discord",
	"www.source2source.org",
	"",
	"*Source Modding Community - Discord",
	"discord.gg/wpFf2f6",
	"",
	"*The Whole Half-Life forums",
	"twhl.info",
	"",
	"*The Gold Source modding community",
	"",
	"*We hope you enjoyed",
	"*\"Half-Rats: Parasomnia\"!",
	"",
	"#Half-Rats on Facebook!",
	"*www.facebook.com/hrseries",
	"",
	"#Check out our next game in the making:",
	"#Half-Rats: Man Out of Place And Time",
	"",
	"#Legal",
	"",
	"*Half-Rats: Parasomnia is powered by the Gold Source engine",
	"*by Valve Corporation (c) 1998-2017. Gold Source contains",
	"*code from Quake by Id Software (c) 1996-2017. Id Software",
	"*is a subsidiary of ZeniMax Media Inc since 2009.",
	"",
	"*Half-Rats: Parasomnia\'s music system is powered by FMOD",
	"*Studio Low Level API (c) 2016-2017.",
	"",
	"*FMOD Studio (c) Firelight Technologies Pty Ltd.",
	"",
	"*The following apply to the Xash3D version only:",
	"*Xash3D is made by Unkle Mike (g-cont) with contributions",
	"*from adamix, Crazy Russian, n00b and XaeroX.",
	"",
	"*Xash3D is using parts of the following games/technologies:",
	"*AVIKit (library for manipulating AVI files) by randomnine.",
	"*Darkplaces (Quake engine port) by Darkplaces Team.",
	"*Half-Life (Gold Source engine) by Valve Software.",
	"*Quake engine, id Tech 2 (Quake II) and id Tech 3 (Quake III",
	"*Arena) engines by Id Software.",
	"*Quake II Evolved (enhanced id Tech 2 engine) by Team Blur.",
	"",
	"#You have unlocked \"New Game+\"!",
	"#Start a new game to see the details!"
};

CHRPCreditsPanel::CHRPCreditsPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall ) : CMenuPanel( iTrans, iRemoveMe, x, y, wide, tall )
{
	// Create the base window
	m_pWindow = new CTransparentPanel( iTrans, CREDITS_WINDOW_X, CREDITS_WINDOW_Y, CREDITS_WINDOW_SIZE_X, CREDITS_WINDOW_SIZE_Y );
	m_pWindow->setParent( this );
	m_bFileHasBeenCreated = false;
	m_bInitialized = false;

	// Finalize
	Initialize();
}

void CHRPCreditsPanel::Initialize( void )
{
	// Get the scheme
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hPersonScheme = pSchemes->getSchemeHandle( "Scoreboard Title Text" );
	SchemeHandle_t hTextScheme = pSchemes->getSchemeHandle( "Scoreboard Small Text" );

	// Get the fonts
	Font *pTitleFont = pSchemes->getFont( hTitleScheme );
	Font *pPersonFont = pSchemes->getFont( hPersonScheme );
	Font *pTextFont = pSchemes->getFont( hTextScheme );
	int iRed, iGreen, iBlue, iAlpha;

	// Create the texts
	int i;
	for ( i = 1; i <= NUMBER_OF_LINES; i++ )
	{
		m_pText[i - 1] = new Label( gcCreditsData[i - 1], XRES( 10 ), YRES( ((i - 1) * 24.0f) ) );
		if ( gcCreditsData[i - 1][0] == '#' )
		{
			m_pText[i - 1]->setFont( pTitleFont );
			pSchemes->getFgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setFgColor( iRed, iGreen, iBlue, iAlpha );
			pSchemes->getBgColor( hTitleScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setBgColor( iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setText( ++gcCreditsData[i - 1] );
		}
		else if ( gcCreditsData[i - 1][0] == '*' )
		{
			m_pText[i - 1]->setFont( pPersonFont );
			pSchemes->getFgColor( hPersonScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setFgColor( iRed, iGreen, iBlue, iAlpha );
			pSchemes->getBgColor( hPersonScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setBgColor( iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setText( ++gcCreditsData[i - 1] );
		}
		else
		{
			m_pText[i - 1]->setFont( pTextFont );
			pSchemes->getFgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setFgColor( iRed, iGreen, iBlue, iAlpha );
			pSchemes->getBgColor( hTextScheme, iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setBgColor( iRed, iGreen, iBlue, iAlpha );
			m_pText[i - 1]->setText( gcCreditsData[i - 1] );
		}
		m_pText[i - 1]->setParent( m_pWindow );
		m_pText[i - 1]->setContentAlignment( vgui::Label::a_center );
	}

	m_bInitialized = true;
}

void CHRPCreditsPanel::Reset( void )
{
	if ( !m_bInitialized )
		return;

	m_pWindow->setPos( XRES( 10 ), YRES( 500 ) );
	m_flLastTime = gHUD.m_flTime;
	CMenuPanel::Reset();
}

void CHRPCreditsPanel::Scroll( void )
{
	if ( !m_bInitialized )
		return;

	if ( (gHUD.m_flTime - m_flLastTime) <= 0.025f )
		return;

	if ( !m_bFileHasBeenCreated )
	{
		char cPath[256];
		sprintf( cPath, "%s/SAVE/game.finished", gEngfuncs.pfnGetGameDirectory() );
		FILE *pFile = fopen( cPath, "w" );
		if ( pFile )
			fclose( pFile );
		else
			gEngfuncs.Con_Printf( "[HR:P] Failed to create \"game finished\" file!\n" );

		m_bFileHasBeenCreated = true;
	}

	m_flLastTime = gHUD.m_flTime;

	// Just scroll the window
	int iX;
	int iY;
	m_pWindow->getPos( iX, iY );
	m_pWindow->setPos( iX, --iY );

	//gEngfuncs.Con_Printf( "m_pWindow - iY = %d\n", iY );
	if ( iY < -5500 )
	{
		Close();
		// The code below is different from the WON/Xash3D build due to differences with the input code
		App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor( Scheme::scu_none ) );
		IN_SetVisibleMouse( false );
		IN_ResetMouse();
	}
}
