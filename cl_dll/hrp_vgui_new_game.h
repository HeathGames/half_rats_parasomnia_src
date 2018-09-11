#ifndef HRP_VGUI_NEW_GAME_INCLUDED
#define HRP_VGUI_NEW_GAME_INCLUDED
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

#define BUYMENU_MAX_BUTTONS	8
#define BUYMENU_MAX_IMAGES	7

class CHRPNewGameMenuPanel : public CMenuPanel, public ICheckButton2Handler
{
private:
	CTransparentPanel *m_pWindow;
	Label *m_pTitle;
	CommandButton *m_pButtonEasy;
	CommandButton *m_pButtonMedium;
	CommandButton *m_pButtonHard;
	CTFScrollPanel *m_pScrollPanel;
	TextPanel *m_pInfoText;
	CCheckButton2 *m_pCheckBoxBonusDebugWeapon;
	CCheckButton2 *m_pCheckBoxBonusDrunk;
	CCheckButton2 *m_pCheckBoxBonusFragile;

public:
	CHRPNewGameMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );

	bool m_bGameIsFinished;

	virtual bool SlotInput( int iSlot );
	virtual void SetActiveInfo( int iInput );

	virtual void Initialize( void );
	virtual void Reset( void );

	virtual void StateChanged( CCheckButton2 *pButton );
	void UpdatePositions( void );
};

#endif // HRP_VGUI_NEW_GAME_INCLUDED
