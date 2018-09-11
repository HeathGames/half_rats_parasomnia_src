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
// HR:P items
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "items.h"
#include "player.h"
#include "weapons.h"

extern int gmsgFlashlight;

class CItemCocaine : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_cocaine.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_cocaine.mdl" );

		PRECACHE_SOUND( "items/pickup.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			pPlayer->ApplyItem( ITEM_COCAINE );
			return TRUE;
		}

		if ( pPlayer->m_rgItems[ITEM_COCAINE] >= MAX_ITEM_CARRY )
			return FALSE;

		pPlayer->m_rgItems[ITEM_COCAINE]++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/pickup.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_cocaine, CItemCocaine );
LINK_ENTITY_TO_CLASS( weapon_gauss, CItemCocaine );

//
//--------------------------------------------------
//

class CItemDrymeat : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_drymeat.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_drymeat.mdl" );

		PRECACHE_SOUND( "items/pickup.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			pPlayer->ApplyItem( ITEM_DRYMEAT );
			return TRUE;
		}

		if ( pPlayer->m_rgItems[ITEM_DRYMEAT] >= MAX_ITEM_CARRY )
			return FALSE;

		pPlayer->m_rgItems[ITEM_DRYMEAT]++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/pickup.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_drymeat, CItemDrymeat );
LINK_ENTITY_TO_CLASS( weapon_egon, CItemDrymeat );

//
//--------------------------------------------------
//

class CItemMeatpie : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_meatpie.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_meatpie.mdl" );

		PRECACHE_SOUND( "items/pickup.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			if ( pPlayer->pev->health >= pPlayer->pev->max_health )
				return FALSE;

			pPlayer->ApplyItem( ITEM_MEATPIE );
			return TRUE;
		}

		if ( pPlayer->m_rgItems[ITEM_MEATPIE] >= MAX_ITEM_CARRY )
			return FALSE;

		pPlayer->m_rgItems[ITEM_MEATPIE]++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/pickup.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_healthkit, CItemMeatpie );
LINK_ENTITY_TO_CLASS( item_meatpie, CItemMeatpie );

//
//--------------------------------------------------
//

class CItemMoney : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_money.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_money.mdl" );

		PRECACHE_SOUND( "items/money.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->m_iMoney == 999 )
			return FALSE;

		switch ( pev->body )
		{
		case 0:
			if ( pPlayer->m_iMoney + ITEM_MONEY_GIVE_AMOUNT_NORMAL >= MAX_MONEY_CARRY )
				pPlayer->m_iMoney = MAX_MONEY_CARRY;
			else
				pPlayer->m_iMoney += ITEM_MONEY_GIVE_AMOUNT_NORMAL;
			break;
		case 1:
			if ( pPlayer->m_iMoney + ITEM_MONEY_GIVE_AMOUNT_GOLD >= MAX_MONEY_CARRY )
				pPlayer->m_iMoney = MAX_MONEY_CARRY;
			else
				pPlayer->m_iMoney += ITEM_MONEY_GIVE_AMOUNT_GOLD;
			break;
		}
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/money.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_money, CItemMoney );

//
//--------------------------------------------------
//

class CItemWhaleOil : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_whaleoil.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_whaleoil.mdl" );

		PRECACHE_SOUND( "items/pickup.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			if ( pPlayer->m_iFlashBattery >= 100 )
				return FALSE;

			pPlayer->ApplyItem( ITEM_WHALEOIL );
			return TRUE;
		}

		if ( pPlayer->m_rgItems[ITEM_WHALEOIL] >= MAX_ITEM_CARRY )
			return FALSE;

		pPlayer->m_rgItems[ITEM_WHALEOIL]++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/pickup.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_whaleoil, CItemWhaleOil );
LINK_ENTITY_TO_CLASS( ammo_gaussclip, CItemWhaleOil );

//
//--------------------------------------------------
//

class CItemWhiskey : public CItem
{
	void Spawn( void )
	{
		Precache();
		if ( pev->model )
			SET_MODEL( ENT( pev ), STRING( pev->model ) ); // LRC
		else
			SET_MODEL( ENT( pev ), "models/w_whiskey.mdl" );

		CItem::Spawn();
	}
	void Precache( void )
	{
		if ( pev->model )
			PRECACHE_MODEL( (char *)STRING( pev->model ) ); // LRC
		else
			PRECACHE_MODEL( "models/w_whiskey.mdl" );

		PRECACHE_SOUND( "items/pickup.wav" );
	}
	BOOL MyTouch( CBasePlayer *pPlayer )
	{
		if ( g_pGameRules->IsMultiplayer() )
		{
			if ( pPlayer->pev->armorvalue >= MAX_NORMAL_BATTERY )
				return FALSE;

			pPlayer->ApplyItem( ITEM_WHISKEY );
			return TRUE;
		}

		if ( pPlayer->m_rgItems[ITEM_WHISKEY] >= MAX_ITEM_CARRY )
			return FALSE;

		pPlayer->m_rgItems[ITEM_WHISKEY]++;
		EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/pickup.wav", 1.0f, ATTN_NORM );
		return TRUE;
	}
};
LINK_ENTITY_TO_CLASS( item_battery, CItemWhiskey );
LINK_ENTITY_TO_CLASS( item_whiskey, CItemWhiskey );
