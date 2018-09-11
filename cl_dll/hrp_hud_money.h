#ifndef HRP_HUD_MONEY_INCLUDED
#define HRP_HUD_MONEY_INCLUDED
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
// HR:P HUD money
//=========================================================

class CHudMoney : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	int MsgFunc_Money( const char *pszName, int iSize, void *pbuf );

private:
	float m_flFade;
	HLSPRITE m_hIcon;
	int m_iMoney;
};

#endif // HRP_HUD_MONEY_INCLUDED
