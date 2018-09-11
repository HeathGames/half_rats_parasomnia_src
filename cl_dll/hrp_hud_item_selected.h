#ifndef HRP_HUD_ITEM_SELECTED_INCLUDED
#define HRP_HUD_ITEM_SELECTED_INCLUDED
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
// HR:P HUD item selected
//=========================================================

class CHudItemSelected : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	int MsgFunc_ItemSelect( const char *pszName, int iSize, void *pbuf );

private:
	char cOldMapName[64];
	float m_flFade;
	float m_flUseTime;
	HLSPRITE m_hIconSelected;
	HLSPRITE m_hIconUsed;
	int m_iItemID;
};

#endif // HRP_HUD_ITEM_SELECTED_INCLUDED
