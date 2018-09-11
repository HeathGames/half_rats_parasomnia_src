#ifndef HRP_VGUI_CREDITS_INCLUDED
#define HRP_VGUI_CREDITS_INCLUDED
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

#define NUMBER_OF_LINES 160

class CHRPCreditsPanel : public CMenuPanel
{
private:
	bool m_bFileHasBeenCreated;
	CTransparentPanel *m_pWindow;
	float m_flLastTime;
	Label *m_pText[NUMBER_OF_LINES];

public:
	CHRPCreditsPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );

	virtual void Initialize( void );
	virtual void Reset( void );

	void Scroll( void );

	bool m_bInitialized;
};

#endif // HRP_VGUI_CREDITS_INCLUDED
