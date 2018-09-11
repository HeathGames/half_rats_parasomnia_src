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
// GameRules.cpp
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"skill.h"
#include	"game.h"

extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

DLL_GLOBAL CGameRules*	g_pGameRules = NULL;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgMOTD;

int g_teamplay = 0;

//=========================================================
//=========================================================
BOOL CGameRules::CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry )
{
	int iAmmoIndex;

	if ( pszAmmoName )
	{
		iAmmoIndex = pPlayer->GetAmmoIndex( pszAmmoName );

		if ( iAmmoIndex > -1 )
		{
			if ( pPlayer->AmmoInventory( iAmmoIndex ) < iMaxCarry )
			{
				// player has room for more of this type of ammo
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
//=========================================================
edict_t *CGameRules :: GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	edict_t *pentSpawnSpot = EntSelectSpawnPoint( pPlayer );

	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
	pPlayer->pev->v_angle  = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;
	
	return pentSpawnSpot;
}

//=========================================================
//=========================================================
BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	// only living players can have items
	if ( pPlayer->pev->deadflag != DEAD_NO )
		return FALSE;

	if ( pWeapon->pszAmmo1() )
	{
		if ( !CanHaveAmmo( pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1() ) )
		{
			// we can't carry anymore ammo for this gun. We can only 
			// have the gun if we aren't already carrying one of this type
			if ( pPlayer->HasPlayerItem( pWeapon ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		// weapon doesn't use ammo, don't take another if you already have it.
		if ( pPlayer->HasPlayerItem( pWeapon ) )
		{
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}

//=========================================================
// load the SkillData struct with the proper values based on the skill level.
//=========================================================
void CGameRules::RefreshSkillData ( void )
{
	int	iSkill;

	iSkill = (int)CVAR_GET_FLOAT("skill");
	g_iSkillLevel = iSkill;

	if ( iSkill < 1 )
	{
		iSkill = 1;
	}
	else if ( iSkill > 3 )
	{
		iSkill = 3; 
	}

	gSkillData.iSkillLevel = iSkill;

	ALERT ( at_console, "\nGAME SKILL LEVEL:%d\n",iSkill );

// PLAYER WEAPONS

	// Cavalry saber whack
	gSkillData.plrDmgSaber = GetSkillCvar( "sk_plr_saber" );

	// Colt SAA Round
	gSkillData.plrDmgColt = GetSkillCvar( "sk_plr_colt_bullet" );

	// Henry rifle Round
	gSkillData.plrDmgHenry = GetSkillCvar( "sk_plr_henry_bullet" );

	// Double barrel shotgun buckshot
	gSkillData.plrDmgBuckshot = GetSkillCvar( "sk_plr_buckshot" );

#ifdef USE_MP_WEAPONS
	// Bow Arrow
	gSkillData.plrDmgBowArrow = GetSkillCvar( "sk_plr_bow_arrow" );
#endif

	// Gatling Round
	gSkillData.plrDmgGatling = GetSkillCvar( "sk_plr_gatling_bullet" );

	// Dynamite
	gSkillData.plrDmgDynamite = GetSkillCvar( "sk_plr_dynamite" );

	// MONSTER WEAPONS
	gSkillData.monDmgColt = GetSkillCvar( "sk_colt_bullet" );
	gSkillData.monDmgHenry = GetSkillCvar( "sk_henry_bullet" );
	gSkillData.monDmg12MM = GetSkillCvar( "sk_12mm_bullet");


	// HEALTH/CHARGE
	gSkillData.whaleOilCapacity = GetSkillCvar( "sk_whaleoil" );

	// monster damage adj
	gSkillData.monHead = GetSkillCvar( "sk_monster_head" );
	gSkillData.monChest = GetSkillCvar( "sk_monster_chest" );
	gSkillData.monStomach = GetSkillCvar( "sk_monster_stomach" );
	gSkillData.monLeg = GetSkillCvar( "sk_monster_leg" );
	gSkillData.monArm = GetSkillCvar( "sk_monster_arm" );

	// player damage adj
	gSkillData.plrHead = GetSkillCvar( "sk_player_head" );
	gSkillData.plrChest = GetSkillCvar( "sk_player_chest" );
	gSkillData.plrStomach = GetSkillCvar( "sk_player_stomach" );
	gSkillData.plrLeg = GetSkillCvar( "sk_player_leg" );
	gSkillData.plrArm = GetSkillCvar( "sk_player_arm" );

// HR:P MONSTERS

	// Armored Fucker
	gSkillData.armormanHealth = GetSkillCvar( "sk_armorman_health" );

	// Baph
	gSkillData.baphDmgSlash = GetSkillCvar( "sk_baph_dmg_slash" );
	gSkillData.baphHealth = GetSkillCvar( "sk_baph_health" );

	// Bear
	gSkillData.bearDmgBothSlash = GetSkillCvar( "sk_bear_dmg_both_slash" );
	gSkillData.bearDmgOneSlash = GetSkillCvar( "sk_bear_dmg_one_slash" );
	gSkillData.bearHealth = GetSkillCvar( "sk_bear_health" );

	// Bernard
	gSkillData.bernardDmgPuke = GetSkillCvar( "sk_bernard_dmg_puke" );
	gSkillData.bernardDmgSlash = GetSkillCvar( "sk_bernard_dmg_slash" );
	gSkillData.bernardHealth = GetSkillCvar( "sk_bernard_health" );

	// Caleb
	gSkillData.calebDmgBothSlash = GetSkillCvar( "sk_caleb_dmg_both_slash" );
	gSkillData.calebDmgOneSlash = GetSkillCvar( "sk_caleb_dmg_one_slash" );
	gSkillData.calebHealth = GetSkillCvar( "sk_caleb_health" );

	// Clown
	gSkillData.clownDmgBothSlash = GetSkillCvar( "sk_clown_dmg_both_slash" );
	gSkillData.clownDmgOneSlash = GetSkillCvar( "sk_clown_dmg_one_slash" );
	gSkillData.clownHealth = GetSkillCvar( "sk_clown_health" );

	// Dog
	gSkillData.dogDmgBite = GetSkillCvar( "sk_dog_dmg_bite" );
	gSkillData.dogHealth = GetSkillCvar( "sk_dog_health" );

	// Doktor
	gSkillData.doktorDmgBothSlash = GetSkillCvar( "sk_doktor_dmg_both_slash" );
	gSkillData.doktorDmgDart = GetSkillCvar( "sk_doktor_dmg_dart" );
	gSkillData.doktorDmgOneSlash = GetSkillCvar( "sk_doktor_dmg_one_slash" );
	gSkillData.doktorHealth = GetSkillCvar( "sk_doktor_health" );

	// Duck
	gSkillData.duckDmgSlash = GetSkillCvar( "sk_duck_dmg_slash" );
	gSkillData.duckHealth = GetSkillCvar( "sk_duck_health" );

	// Half-Cat
	gSkillData.halfcatDmgBite = GetSkillCvar( "sk_halfcat_dmg_bite" );

#ifdef USE_MP_WEAPONS
	// Half-Cat (as a Snark)
	gSkillData.hcSnarkDmgBite = GetSkillCvar( "sk_hc_snark_dmg_bite" );
#endif

	// Huntsman
	gSkillData.huntsmanDmgBolt = GetSkillCvar( "sk_huntsman_dmg_bolt" );
	gSkillData.huntsmanDmgMelee = GetSkillCvar( "sk_huntsman_dmg_melee" );
	gSkillData.huntsmanHealth = GetSkillCvar( "sk_huntsman_health" );

	// Maskboy
	gSkillData.maskboyDmgSlash = GetSkillCvar( "sk_maskboy_dmg_slash" );
	gSkillData.maskboyHealth = GetSkillCvar( "sk_maskboy_health" );

	// Pig
	gSkillData.pigDmgBite = GetSkillCvar( "sk_pig_dmg_bite" );
	gSkillData.pigHealth = GetSkillCvar( "sk_pig_health" );

	// Teddy
	gSkillData.teddyDmgBite = GetSkillCvar( "sk_teddy_dmg_bite" );
	gSkillData.teddyHealth = GetSkillCvar( "sk_teddy_health" );

	// UD Cop
	gSkillData.udcopDmgKick = GetSkillCvar( "sk_udcop_dmg_kick" );
	gSkillData.udcopHealth = GetSkillCvar( "sk_udcop_health" );

	// Zombie
	gSkillData.zombieDmgBothSlash = GetSkillCvar( "sk_zombie_dmg_both_slash" );
	gSkillData.zombieDmgOneSlash = GetSkillCvar( "sk_zombie_dmg_one_slash" );
	gSkillData.zombieHealth = GetSkillCvar( "sk_zombie_health" );
}

//=========================================================
// instantiate the proper game rules object
//=========================================================

CGameRules *InstallGameRules( void )
{
	SERVER_COMMAND( "exec game.cfg\n" );
	SERVER_EXECUTE( );

	if ( !gpGlobals->deathmatch )
	{
		// generic half-life
		g_teamplay = 0;
		return new CHalfLifeRules;
	}
	else
	{
		ALERT( at_console, "We are working on making multiplayer better!\n" );
		SERVER_COMMAND( "quit\n" );
		return NULL;
		/*if ( teamplay.value > 0 )
		{
			// teamplay

			g_teamplay = 1;
			return new CHalfLifeTeamplay;
		}
		if ((int)gpGlobals->deathmatch == 1)
		{
			// vanilla deathmatch
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}
		else
		{
			// vanilla deathmatch??
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}*/
	}
}



