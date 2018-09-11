#ifndef HRP_VGUI_BUY_MENU_INCLUDED
#define HRP_VGUI_BUY_MENU_INCLUDED
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

#define BUYMENU_MAX_BUTTONS	8
#define BUYMENU_MAX_IMAGES	7

class CHRPBuyMenuPanel : public CMenuPanel
{
private:
	CTransparentPanel *m_pWindow;
	Label *m_pTitle;
	CommandButton *m_pButtons[BUYMENU_MAX_BUTTONS];
	CTFScrollPanel *m_pScrollPanel;
	TextPanel *m_pInfoText;
	CImageLabel *m_pItemsImages[BUYMENU_MAX_IMAGES];

public:
	CHRPBuyMenuPanel( int iTrans, int iRemoveMe, int x, int y, int wide, int tall );

	virtual bool SlotInput( int iSlot );
	virtual void SetActiveInfo( int iInput );

	virtual void Initialize( void );
	virtual void Reset( void );
};

#endif // HRP_VGUI_BUY_MENU_INCLUDED
