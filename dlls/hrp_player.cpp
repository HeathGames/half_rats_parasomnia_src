/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
// HR:P player stuff
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "talkmonster.h"
#include "weapons.h"
#include "../engine/shake.h"

#include "hrp_weapon_saber.h"

// Constants for items
#define COCAINE_DURATION			10.0f
#define COCAINE_SWITCH_TIME			2.0f

#define DMG_BOOST_COCAINE_DRYMEAT	5.0f
#define DMG_BOOST_COCAINE			-5.0f
#define DMG_BOOST_DRYMEAT			10.0f

#define DRYMEAT_ARMOR_GIVE			10.0f
#define DRYMEAT_DURATION			20.0f
#define DRYMEAT_HEALTH_GIVE			4.0f

#define MEATPIE_HEALTH_GIVE			20.0f

#define WHISKEY_ARMOR_GIVE			20.0f
#define WHISKEY_DURATION			15.0f
#define WHISKEY_DRUNK_AMOUNT_MP		3
#define WHISKEY_DRUNK_AMOUNT_SP		2
#define WHISKEY_FLASH_TIME			2.0f

// HUD messages
int gmsgBuyZone = 0;
int gmsgConcuss = 0;
int gmsgGameEnd = 0;
int gmsgFMOD = 0;
int gmsgHenSight = 0;
int gmsgItemCocaine = 0;
int gmsgItemDrymeat = 0;
int gmsgItemMeatpie = 0;
int gmsgItemWhiskey = 0;
int gmsgItemWhaleOil = 0;
int gmsgItemSelect = 0;
int gmsgMoney = 0;
int gmsgSetBody = 0;
int gmsgVGUIMenu = 0;
int gmsgWeapShot = 0;

// HUD messages from player.cpp
extern int gmsgFlashBattery;
extern int gmsgFlashlight;
extern int gmsgShowMenu;

void LinkUserMessagesHRP( void )
{
	// Already taken care of?
	if ( gmsgBuyZone )
		return;

	gmsgBuyZone = REG_USER_MSG( "BuyZone", 1 );
	gmsgConcuss = REG_USER_MSG( "Concuss", 1 );
	gmsgGameEnd = REG_USER_MSG( "GameEnd", 1 );
	gmsgFMOD = REG_USER_MSG( "FMOD", -1 );
	gmsgHenSight = REG_USER_MSG( "HenSight", 1 );
	gmsgItemCocaine = REG_USER_MSG( "ItemCocaine", 1 );
	gmsgItemDrymeat = REG_USER_MSG( "ItemDrymeat", 1 );
	gmsgItemMeatpie = REG_USER_MSG( "ItemMeatpie", 1 );
	gmsgItemWhiskey = REG_USER_MSG( "ItemWhiskey", 1 );
	gmsgItemWhaleOil = REG_USER_MSG( "ItemOil", 1 );
	gmsgItemSelect = REG_USER_MSG( "ItemSelect", 2 );
	gmsgMoney = REG_USER_MSG( "Money", 2 );
	gmsgSetBody = REG_USER_MSG( "SetBody", 1 );
	gmsgVGUIMenu = REG_USER_MSG( "VGUIMenu", 2 );
	gmsgWeapShot = REG_USER_MSG( "WeapShot", 2 );
}

BOOL CBasePlayer::IsBlocking( CBaseEntity *pAttacker )
{
	// Check we are using the Saber
	if ( m_pActiveItem && m_pActiveItem->m_iId == WEAPON_SABER )
	{
		// Are we blocking?
		CSaber *pSaber = (CSaber *)m_pActiveItem;
		if ( pSaber->m_iParaExtraStuff == SABER_BLOCK_STATE_BLOCKING )
		{
			// Check the LoS and the direction of the attack
			Vector2D vec2LoS;
			float flDot;

			UTIL_MakeVectors( pev->angles );
			vec2LoS = (pAttacker->pev->origin - pev->origin).Make2D();
			vec2LoS = vec2LoS.Normalize();
			flDot = DotProduct( vec2LoS, gpGlobals->v_forward.Make2D() );

			// Check if we're not being attacked from the side or the back
			if ( flDot > m_flFieldOfView )
			{
				pSaber->m_bPlayBlockHitAnim = true;
				pSaber->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.01f;
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CBasePlayer::IsInBuyZone( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityInSphere( pEntity, pev->origin, 1.0f )) != NULL)
	{
		if ( FClassnameIs( pEntity->pev, "func_buy" ) )
			return TRUE;
	}

	return FALSE;
}

float CBasePlayer::GetDamageBoost( void )
{
	if ( m_flCocaineTime > gpGlobals->time && m_flDrymeatTime > gpGlobals->time ) // Cocaine + dried meat
		return DMG_BOOST_COCAINE_DRYMEAT;
	else if ( m_flCocaineTime > gpGlobals->time ) // Cocaine
		return DMG_BOOST_COCAINE;
	else if ( m_flDrymeatTime > gpGlobals->time ) // Dried meat
		return DMG_BOOST_DRYMEAT;
	else // Nothing
		return 0.0f;
}

void CBasePlayer::ApplyItem( const int iItemID )
{
	// Don't use an item if frozen
	if ( pev->flags & FL_FROZEN )
		return;

	switch ( iItemID )
	{
	case ITEM_COCAINE:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/cocaine.wav", 1.0f, ATTN_NORM );
		if ( g_pGameRules->IsMultiplayer() )
			m_flCocaineTime = gpGlobals->time + (COCAINE_DURATION * 2.0f);
		else
			m_flCocaineTime = gpGlobals->time + COCAINE_DURATION;

		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 1 );
		MESSAGE_END();
		break;
	case ITEM_DRYMEAT:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/food2.wav", 1.0f, ATTN_NORM );
		TakeHealth( DRYMEAT_HEALTH_GIVE, DMG_GENERIC );
		if ( pev->armorvalue + DRYMEAT_ARMOR_GIVE > MAX_NORMAL_BATTERY )
			pev->armorvalue = MAX_NORMAL_BATTERY;
		else
			pev->armorvalue += DRYMEAT_ARMOR_GIVE;

		if ( m_flDrymeatTime >= gpGlobals->time )
			m_flDrymeatTime += DRYMEAT_DURATION;
		else
			m_flDrymeatTime = gpGlobals->time + DRYMEAT_DURATION;

		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 1 );
		MESSAGE_END();
		break;
	case ITEM_MEATPIE:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/food.wav", 1.0f, ATTN_NORM );
		TakeHealth( MEATPIE_HEALTH_GIVE, DMG_GENERIC );
		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 1 );
		MESSAGE_END();
		break;
	case ITEM_WHISKEY:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/whiskey.wav", 1.0f, ATTN_NORM );
		m_flWhiskeyTime = gpGlobals->time + WHISKEY_DURATION;
		m_iWhiskeyCount++;
		if ( pev->armorvalue + WHISKEY_ARMOR_GIVE > MAX_NORMAL_BATTERY )
			pev->armorvalue = MAX_NORMAL_BATTERY;
		else
			pev->armorvalue += WHISKEY_ARMOR_GIVE;

		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 1 );
		MESSAGE_END();
		break;
	case ITEM_WHALEOIL:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/whaleoil.wav", 1.0f, ATTN_NORM );
		if ( m_iFlashBattery + gSkillData.whaleOilCapacity > 100 )
			m_iFlashBattery = 100;
		else
			m_iFlashBattery += gSkillData.whaleOilCapacity;

		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
			WRITE_BYTE( FlashlightIsOn() );
			WRITE_BYTE( m_iFlashBattery );
		MESSAGE_END();
		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 1 );
		MESSAGE_END();
		break;
	}
}

void CBasePlayer::ForceClientDllUpdateHRP( void )
{
	// Reset all client variables
	m_iClientBuyZone = -1;
	m_iClientGameEnd = -1;
	m_iClientHenryIronsight = -1;
	m_iClientFlashlightBattery = -1;
	m_iClientFlashlightState = -1;
	m_iClientMoney = -1;
	m_iClientItemCocaine = -1;
	m_iClientItemDrymeat = -1;
	m_iClientItemMeatpie = -1;
	m_iClientItemWhiskey = -1;
	m_iClientItemWhaleOil = -1;
	m_iClientSelectedItem = -1;
}

void CBasePlayer::PreThinkHRP( void )
{
	//ALERT( at_console, "%2f %2f %2f - %2f %2f %2f\n", pev->origin.x, pev->origin.y, pev->origin.z, pev->angles.x, pev->angles.y, pev->angles.z );

	// Under cocaine effect
	if ( m_flCocaineTime >= gpGlobals->time )
	{
		// Just fade in/out the screen to red at a certain interval
		if ( m_flCocaineSwitchTime < gpGlobals->time )
		{
			if ( m_flCinematicFadeTime < gpGlobals->time )
				UTIL_ScreenFade( this, Vector( 64.0f, 0.0f, 0.0f ), 2.0f, 0.0f, 64, (m_bCocaineFadeOut) ? FFADE_OUT : FFADE_IN );

			m_bCocaineFadeOut = !m_bCocaineFadeOut;
			m_flCocaineSwitchTime = gpGlobals->time + COCAINE_SWITCH_TIME;
		}
	}

	// Mutator : "Always drunk"
	if ( m_iGameEnd == 1 && CVAR_GET_FLOAT( "sv_hrp_bonus_drunk" ) > 0.0f && (m_iWhiskeyCount < 2 || m_iWhiskeyCount > 3) )
		m_iWhiskeyCount = 2;

	// Under whiskey effect (handle Half-Rats drunk insults)
	if ( m_iWhiskeyCount > 0 && m_flWhiskeyTime < gpGlobals->time )
	{
		if ( m_iWhiskeyCount > 1 && gpGlobals->time > CTalkMonster::g_talkWaitTime )
		{
			switch ( RANDOM_LONG( 0, 4 ) )
			{
			case 0: EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/drunk.wav", 1.0f, ATTN_NORM ); break;
			case 1: EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/drunk2.wav", 1.0f, ATTN_NORM ); break;
			case 2: EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/drunk3.wav", 1.0f, ATTN_NORM ); break;
			case 3: EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/drunk4.wav", 1.0f, ATTN_NORM ); break;
			case 4: EMIT_SOUND( ENT( pev ), CHAN_VOICE, "items/drunk5.wav", 1.0f, ATTN_NORM ); break;
			}
		}
		if ( m_iGameEnd == 1 && CVAR_GET_FLOAT( "sv_hrp_bonus_drunk" ) > 0.0f ) // Mutator : "Always drunk"
			m_iWhiskeyCount = m_iWhiskeyCount == 3 ? 2 : 3;
		else
			m_iWhiskeyCount--;

		m_flWhiskeyTime = gpGlobals->time + WHISKEY_DURATION;
	}

	// Under whiskey effect (handle the concussion and flashing screen)
	if ( (g_pGameRules->IsMultiplayer() && m_iWhiskeyCount >= WHISKEY_DRUNK_AMOUNT_MP) ||
		(!g_pGameRules->IsMultiplayer() && m_iWhiskeyCount >= WHISKEY_DRUNK_AMOUNT_SP) )
	{
		if ( m_flWhiskeyNextDrunkTime < gpGlobals->time )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgConcuss, NULL, pev );
				WRITE_BYTE( 50 );
			MESSAGE_END();
			if ( m_flCinematicFadeTime < gpGlobals->time )
				UTIL_ScreenFade( this, Vector( RANDOM_LONG( 0, 255 ), RANDOM_LONG( 0, 255 ), RANDOM_LONG( 0, 255 ) ), 1.0f, 0.5f, 64, FFADE_OUT );

			m_flWhiskeyNextDrunkTime = gpGlobals->time + WHISKEY_FLASH_TIME;
		}
	}
	else // No longer under whiskey effect (denies the concussion)
	{
		// TODO : Don't send the message every frame
		MESSAGE_BEGIN( MSG_ONE, gmsgConcuss, NULL, pev );
			WRITE_BYTE( 0 );
		MESSAGE_END();
	}

	// Handle Teddy freeze attack
	if ( gpGlobals->time >= m_flTeddyUnfreezeTime )
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "freeze", "0" );
		m_flTeddyUnfreezeTime = -1.0f;
	}
	else
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "freeze", pev->movetype == MOVETYPE_NOCLIP ? "0" : "1" );

	if ( m_pActiveItem && m_pActiveItem->m_iId == WEAPON_GATLING ) // Shepard : Fix Gatling gun speed reduction not being applied while firing
		SetSpeed( PLAYER_SPEED_GATLING );
	else
		SetSpeed();
}

void CBasePlayer::SetSpeed( float _flSpeed )
{
	m_flSpeed = _flSpeed;
	if ( gpGlobals->time >= m_flTeddyUnfreezeTime )
		g_engfuncs.pfnSetClientMaxspeed( ENT( pev ), m_flSpeed );
	else
		g_engfuncs.pfnSetClientMaxspeed( ENT( pev ), 0.0f );
}

void CBasePlayer::SetSpeed( void )
{
	SetSpeed( m_flSpeed );
}

void CBasePlayer::ShowMenu( int bitsValidSlots, int nDisplayTime, BOOL fNeedMore, char *pszText, int iMenuID )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pev );
		WRITE_SHORT( bitsValidSlots );
		WRITE_CHAR( nDisplayTime );
		WRITE_BYTE( fNeedMore );
		WRITE_STRING( pszText );
	MESSAGE_END();
	m_iMenuID = iMenuID;
}

void CBasePlayer::SpawnHRP( void )
{
	m_bNewGamePlusTriggersEnabled = FALSE;

	m_flCinematicFadeTime = -1.0f;

	m_iClientBuyZone = -1;
	m_iMenuID = 0;

	m_iClientFlashlightBattery = -1;
	m_iClientFlashlightState = -1;

	if ( !g_pGameRules->IsMultiplayer() )
	{
		char cPath[255];
		GET_GAME_DIR( cPath );
		strcat( cPath, "/SAVE/game.finished" );

		byte *pFile = LOAD_FILE_FOR_ME( cPath, 0 );
		if ( pFile )
		{
			m_iGameEnd = 1;
			FREE_FILE( pFile );
		}
		else
			m_iGameEnd = 0;
	}
	else
		m_iGameEnd = 0;

	m_iClientGameEnd = -1;

	m_iMoney = MONEY_START;
	m_iClientMoney = -1;

	m_rgItems[ITEM_COCAINE] = 0;
	m_iClientItemCocaine = -1;
	m_flCocaineTime = 0.0f;
	m_flCocaineSwitchTime = 0.0f;

	m_rgItems[ITEM_DRYMEAT] = 0;
	m_iClientItemDrymeat = -1;
	m_flDrymeatTime = 0.0f;

	m_rgItems[ITEM_MEATPIE] = 0;
	m_iClientItemMeatpie = -1;

	m_rgItems[ITEM_WHISKEY] = 0;
	m_iClientItemWhiskey = -1;
	m_flWhiskeyTime = 0.0f;
	m_iWhiskeyCount = 0;

	m_rgItems[ITEM_WHALEOIL] = 0;
	m_iClientItemWhaleOil = -1;

	m_iSelectedItem = ITEM_COCAINE;
	m_iClientSelectedItem = -1;

	m_iClientHenryIronsight = -1;

	SetSpeed( PLAYER_SPEED_NORMAL );
}

void CBasePlayer::SwitchItem( void )
{
	if ( m_iSelectedItem == ITEM_WHALEOIL )
		m_iSelectedItem = ITEM_COCAINE;
	else
		m_iSelectedItem++;
}

void CBasePlayer::UpdateClientDataHRP( void )
{
	// Update lantern fuel
	if ( m_iFlashBattery != m_iClientFlashlightBattery )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgFlashBattery, NULL, pev );
			WRITE_BYTE( m_iFlashBattery );
		MESSAGE_END();
		m_iClientFlashlightBattery = m_iFlashBattery;
	}

	// Update lantern state
	if ( FlashlightIsOn() != m_iClientFlashlightState )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
			WRITE_BYTE( FlashlightIsOn() );
			WRITE_BYTE( m_iFlashBattery );
		MESSAGE_END();
		m_iClientFlashlightState = FlashlightIsOn();
	}

	if ( g_pGameRules->IsMultiplayer() )
		return;

	// Update buy zone icon
	if ( (int)IsInBuyZone() != m_iClientBuyZone )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgBuyZone, NULL, pev );
			WRITE_BYTE( (int)IsInBuyZone() );
		MESSAGE_END();
		m_iClientBuyZone = (int)IsInBuyZone();
	}

	// Update amount of cocaine
	if ( m_rgItems[ITEM_COCAINE] != m_iClientItemCocaine )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemCocaine, NULL, pev );
			WRITE_BYTE( m_rgItems[ITEM_COCAINE] );
		MESSAGE_END();
		m_iClientItemCocaine = m_rgItems[ITEM_COCAINE];
	}

	// Update amount of dried meat
	if ( m_rgItems[ITEM_DRYMEAT] != m_iClientItemDrymeat )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemDrymeat, NULL, pev );
			WRITE_BYTE( m_rgItems[ITEM_DRYMEAT] );
		MESSAGE_END();
		m_iClientItemDrymeat = m_rgItems[ITEM_DRYMEAT];
	}

	// Update amount of meat pies
	if ( m_rgItems[ITEM_MEATPIE] != m_iClientItemMeatpie )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemMeatpie, NULL, pev );
			WRITE_BYTE( m_rgItems[ITEM_MEATPIE] );
		MESSAGE_END();
		m_iClientItemMeatpie = m_rgItems[ITEM_MEATPIE];
	}

	// Update amount of whiskey
	if ( m_rgItems[ITEM_WHISKEY] != m_iClientItemWhiskey )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemWhiskey, NULL, pev );
			WRITE_BYTE( m_rgItems[ITEM_WHISKEY] );
		MESSAGE_END();
		m_iClientItemWhiskey = m_rgItems[ITEM_WHISKEY];
	}

	// Update amount of whale oil
	if ( m_rgItems[ITEM_WHALEOIL] != m_iClientItemWhaleOil )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemWhaleOil, NULL, pev );
			WRITE_BYTE( m_rgItems[ITEM_WHALEOIL] );
		MESSAGE_END();
		m_iClientItemWhaleOil = m_rgItems[ITEM_WHALEOIL];
	}

	// Update selected item
	if ( m_iSelectedItem != m_iClientSelectedItem )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgItemSelect, NULL, pev );
			WRITE_BYTE( m_iSelectedItem );
			WRITE_BYTE( 0 );
		MESSAGE_END();
		m_iClientSelectedItem = m_iSelectedItem;
	}

	// Update amount of money
	if ( m_iMoney != m_iClientMoney )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgMoney, NULL, pev );
			WRITE_SHORT( m_iMoney );
		MESSAGE_END();
		m_iClientMoney = m_iMoney;
	}

	// Update game end state
	if ( m_iGameEnd != m_iClientGameEnd )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgGameEnd, NULL, pev );
			WRITE_BYTE( m_iGameEnd );
		MESSAGE_END();
		m_iClientGameEnd = m_iGameEnd;
	}

	// Trigger New Game+ specific entities
	if ( !m_bNewGamePlusTriggersEnabled && gpGlobals->time >= 3.0f && m_iGameEnd == 1 )
	{
		FireTargets( "new_game_plus", this, this, USE_TOGGLE, 0.0f );
		m_bNewGamePlusTriggersEnabled = TRUE;
	}
}

void CBasePlayer::UseItem( void )
{
	// Don't use an item if frozen
	if ( pev->flags & FL_FROZEN )
		return;

	// Don't use an item if we don't have it anymore
	if ( m_rgItems[m_iSelectedItem] <= 0 )
		return;

	switch ( m_iSelectedItem )
	{
	case ITEM_MEATPIE:
		if ( pev->health >= pev->max_health )
			return;
		break;
	case ITEM_WHISKEY:
		if ( pev->armorvalue >= MAX_NORMAL_BATTERY )
			return;
		break;
	case ITEM_WHALEOIL:
		if ( m_iFlashBattery >= 100 )
			return;
		break;
	}
	m_rgItems[m_iSelectedItem]--;
	ApplyItem( m_iSelectedItem );
}
