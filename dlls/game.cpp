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
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"

cvar_t	displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
cvar_t	fragsleft	= {"mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
cvar_t	timeleft	= {"mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER };
cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t	falldamage	= {"mp_falldamage","0", FCVAR_SERVER };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
cvar_t	flashlight	= {"mp_flashlight","0", FCVAR_SERVER };
cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t	teamoverride = {"mp_teamoverride","1" };
cvar_t	defaultteam = {"mp_defaultteam","0" };
cvar_t	allowmonsters={"mp_allowmonsters","0", FCVAR_SERVER };

cvar_t  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER };		// 0 prevents players from being spectators

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };

cvar_t  hrp_bonus_debug_weapon = {"sv_hrp_bonus_debug_weapon", "0", FCVAR_SERVER}; // Shepard : Mutator "Debug weapon"
cvar_t  hrp_bonus_drunk = { "sv_hrp_bonus_drunk", "0", FCVAR_SERVER }; // Shepard : Mutator "Always drunk"
cvar_t  hrp_bonus_fragile = { "sv_hrp_bonus_fragile", "0", FCVAR_SERVER }; // Shepard : Mutator "Fragile"

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;

//CVARS FOR SKILL LEVEL SETTINGS
// Scientist
cvar_t	sk_scientist_health1 = {"sk_scientist_health1","0"};
cvar_t	sk_scientist_health2 = {"sk_scientist_health2","0"};
cvar_t	sk_scientist_health3 = {"sk_scientist_health3","0"};


// PLAYER WEAPONS

// Cavalry saber whack
cvar_t sk_plr_saber1 = { "sk_plr_saber1", "0" };
cvar_t sk_plr_saber2 = { "sk_plr_saber2", "0" };
cvar_t sk_plr_saber3 = { "sk_plr_saber3", "0" };

// Colt SAA Round
cvar_t sk_plr_colt_bullet1 = { "sk_plr_colt_bullet1", "0" };
cvar_t sk_plr_colt_bullet2 = { "sk_plr_colt_bullet2", "0" };
cvar_t sk_plr_colt_bullet3 = { "sk_plr_colt_bullet3", "0" };

// Henry rifle Round
cvar_t sk_plr_henry_bullet1 = { "sk_plr_henry_bullet1", "0" };
cvar_t sk_plr_henry_bullet2 = { "sk_plr_henry_bullet2", "0" };
cvar_t sk_plr_henry_bullet3 = { "sk_plr_henry_bullet3", "0" };

// Double barrel shotgun buckshot
cvar_t sk_plr_buckshot1 = { "sk_plr_buckshot1", "0" };
cvar_t sk_plr_buckshot2 = { "sk_plr_buckshot2", "0" };
cvar_t sk_plr_buckshot3 = { "sk_plr_buckshot3", "0" };

#ifdef USE_MP_WEAPONS
// Bow Arrow
cvar_t sk_plr_bow_arrow1 = { "sk_plr_bow_arrow1", "0" };
cvar_t sk_plr_bow_arrow2 = { "sk_plr_bow_arrow2", "0" };
cvar_t sk_plr_bow_arrow3 = { "sk_plr_bow_arrow3", "0" };
#endif

// Gatling Round
cvar_t sk_plr_gatling_bullet1 = { "sk_plr_gatling_bullet1", "0" };
cvar_t sk_plr_gatling_bullet2 = { "sk_plr_gatling_bullet2", "0" };
cvar_t sk_plr_gatling_bullet3 = { "sk_plr_gatling_bullet3", "0" };

// Dynamite
cvar_t sk_plr_dynamite1 = { "sk_plr_dynamite1", "0" };
cvar_t sk_plr_dynamite2 = { "sk_plr_dynamite2", "0" };
cvar_t sk_plr_dynamite3 = { "sk_plr_dynamite3", "0" };


// WORLD WEAPONS
cvar_t sk_colt_bullet1 = { "sk_colt_bullet1", "0" };
cvar_t sk_colt_bullet2 = { "sk_colt_bullet2", "0" };
cvar_t sk_colt_bullet3 = { "sk_colt_bullet3", "0" };

cvar_t sk_henry_bullet1 = { "sk_henry_bullet1", "0" };
cvar_t sk_henry_bullet2 = { "sk_henry_bullet2", "0" };
cvar_t sk_henry_bullet3 = { "sk_henry_bullet3", "0" };

cvar_t	sk_12mm_bullet1 = { "sk_12mm_bullet1","0" };
cvar_t	sk_12mm_bullet2 = { "sk_12mm_bullet2","0" };
cvar_t	sk_12mm_bullet3 = { "sk_12mm_bullet3","0" };

// HEALTH/CHARGE
cvar_t	sk_suitcharger1	= { "sk_suitcharger1","0" };
cvar_t	sk_suitcharger2	= { "sk_suitcharger2","0" };		
cvar_t	sk_suitcharger3	= { "sk_suitcharger3","0" };		

cvar_t	sk_battery1	= { "sk_battery1","0" };			
cvar_t	sk_battery2	= { "sk_battery2","0" };			
cvar_t	sk_battery3	= { "sk_battery3","0" };			

cvar_t	sk_healthcharger1	= { "sk_healthcharger1","0" };		
cvar_t	sk_healthcharger2	= { "sk_healthcharger2","0" };		
cvar_t	sk_healthcharger3	= { "sk_healthcharger3","0" };		

cvar_t	sk_healthkit1	= { "sk_healthkit1","0" };		
cvar_t	sk_healthkit2	= { "sk_healthkit2","0" };		
cvar_t	sk_healthkit3	= { "sk_healthkit3","0" };		

cvar_t	sk_scientist_heal1	= { "sk_scientist_heal1","0" };	
cvar_t	sk_scientist_heal2	= { "sk_scientist_heal2","0" };	
cvar_t	sk_scientist_heal3	= { "sk_scientist_heal3","0" };	

cvar_t	sk_whaleoil1 = { "sk_whaleoil1", "0" };
cvar_t	sk_whaleoil2 = { "sk_whaleoil2", "0" };
cvar_t	sk_whaleoil3 = { "sk_whaleoil3", "0" };

// monster damage adjusters
cvar_t	sk_monster_head1	= { "sk_monster_head1","2" };
cvar_t	sk_monster_head2	= { "sk_monster_head2","2" };
cvar_t	sk_monster_head3	= { "sk_monster_head3","2" };

cvar_t	sk_monster_chest1	= { "sk_monster_chest1","1" };
cvar_t	sk_monster_chest2	= { "sk_monster_chest2","1" };
cvar_t	sk_monster_chest3	= { "sk_monster_chest3","1" };

cvar_t	sk_monster_stomach1	= { "sk_monster_stomach1","1" };
cvar_t	sk_monster_stomach2	= { "sk_monster_stomach2","1" };
cvar_t	sk_monster_stomach3	= { "sk_monster_stomach3","1" };

cvar_t	sk_monster_arm1	= { "sk_monster_arm1","1" };
cvar_t	sk_monster_arm2	= { "sk_monster_arm2","1" };
cvar_t	sk_monster_arm3	= { "sk_monster_arm3","1" };

cvar_t	sk_monster_leg1	= { "sk_monster_leg1","1" };
cvar_t	sk_monster_leg2	= { "sk_monster_leg2","1" };
cvar_t	sk_monster_leg3	= { "sk_monster_leg3","1" };

// player damage adjusters
cvar_t	sk_player_head1	= { "sk_player_head1","2" };
cvar_t	sk_player_head2	= { "sk_player_head2","2" };
cvar_t	sk_player_head3	= { "sk_player_head3","2" };

cvar_t	sk_player_chest1	= { "sk_player_chest1","1" };
cvar_t	sk_player_chest2	= { "sk_player_chest2","1" };
cvar_t	sk_player_chest3	= { "sk_player_chest3","1" };

cvar_t	sk_player_stomach1	= { "sk_player_stomach1","1" };
cvar_t	sk_player_stomach2	= { "sk_player_stomach2","1" };
cvar_t	sk_player_stomach3	= { "sk_player_stomach3","1" };

cvar_t	sk_player_arm1	= { "sk_player_arm1","1" };
cvar_t	sk_player_arm2	= { "sk_player_arm2","1" };
cvar_t	sk_player_arm3	= { "sk_player_arm3","1" };

cvar_t	sk_player_leg1	= { "sk_player_leg1","1" };
cvar_t	sk_player_leg2	= { "sk_player_leg2","1" };
cvar_t	sk_player_leg3	= { "sk_player_leg3","1" };

// HR:P MONSTERS

// Armored Fucker
cvar_t sk_armorman_health1 = { "sk_armorman_health1", "0" };
cvar_t sk_armorman_health2 = { "sk_armorman_health2", "0" };
cvar_t sk_armorman_health3 = { "sk_armorman_health3", "0" };

// Baph
cvar_t sk_baph_dmg_slash1 = { "sk_baph_dmg_slash1", "0" };
cvar_t sk_baph_dmg_slash2 = { "sk_baph_dmg_slash2", "0" };
cvar_t sk_baph_dmg_slash3 = { "sk_baph_dmg_slash3", "0" };

cvar_t sk_baph_health1 = { "sk_baph_health1", "0" };
cvar_t sk_baph_health2 = { "sk_baph_health2", "0" };
cvar_t sk_baph_health3 = { "sk_baph_health3", "0" };

// Bear
cvar_t sk_bear_dmg_both_slash1 = { "sk_bear_dmg_both_slash1", "0" };
cvar_t sk_bear_dmg_both_slash2 = { "sk_bear_dmg_both_slash2", "0" };
cvar_t sk_bear_dmg_both_slash3 = { "sk_bear_dmg_both_slash3", "0" };

cvar_t sk_bear_dmg_one_slash1 = { "sk_bear_dmg_one_slash1", "0" };
cvar_t sk_bear_dmg_one_slash2 = { "sk_bear_dmg_one_slash2", "0" };
cvar_t sk_bear_dmg_one_slash3 = { "sk_bear_dmg_one_slash3", "0" };

cvar_t sk_bear_health1 = { "sk_bear_health1", "0" };
cvar_t sk_bear_health2 = { "sk_bear_health2", "0" };
cvar_t sk_bear_health3 = { "sk_bear_health3", "0" };

// Bernard
cvar_t sk_bernard_dmg_puke1 = { "sk_bernard_dmg_puke1", "0" };
cvar_t sk_bernard_dmg_puke2 = { "sk_bernard_dmg_puke2", "0" };
cvar_t sk_bernard_dmg_puke3 = { "sk_bernard_dmg_puke3", "0" };

cvar_t sk_bernard_dmg_slash1 = { "sk_bernard_dmg_slash1", "0" };
cvar_t sk_bernard_dmg_slash2 = { "sk_bernard_dmg_slash2", "0" };
cvar_t sk_bernard_dmg_slash3 = { "sk_bernard_dmg_slash3", "0" };

cvar_t sk_bernard_health1 = { "sk_bernard_health1", "0" };
cvar_t sk_bernard_health2 = { "sk_bernard_health2", "0" };
cvar_t sk_bernard_health3 = { "sk_bernard_health3", "0" };

// Caleb
cvar_t sk_caleb_dmg_both_slash1 = { "sk_caleb_dmg_both_slash1", "0" };
cvar_t sk_caleb_dmg_both_slash2 = { "sk_caleb_dmg_both_slash2", "0" };
cvar_t sk_caleb_dmg_both_slash3 = { "sk_caleb_dmg_both_slash3", "0" };

cvar_t sk_caleb_dmg_one_slash1 = { "sk_caleb_dmg_one_slash1", "0" };
cvar_t sk_caleb_dmg_one_slash2 = { "sk_caleb_dmg_one_slash2", "0" };
cvar_t sk_caleb_dmg_one_slash3 = { "sk_caleb_dmg_one_slash3", "0" };

cvar_t sk_caleb_health1 = { "sk_caleb_health1", "0" };
cvar_t sk_caleb_health2 = { "sk_caleb_health2", "0" };
cvar_t sk_caleb_health3 = { "sk_caleb_health3", "0" };

// Clown
cvar_t sk_clown_dmg_both_slash1 = { "sk_clown_dmg_both_slash1", "0" };
cvar_t sk_clown_dmg_both_slash2 = { "sk_clown_dmg_both_slash2", "0" };
cvar_t sk_clown_dmg_both_slash3 = { "sk_clown_dmg_both_slash3", "0" };

cvar_t sk_clown_dmg_one_slash1 = { "sk_clown_dmg_one_slash1", "0" };
cvar_t sk_clown_dmg_one_slash2 = { "sk_clown_dmg_one_slash2", "0" };
cvar_t sk_clown_dmg_one_slash3 = { "sk_clown_dmg_one_slash3", "0" };

cvar_t sk_clown_health1 = { "sk_clown_health1", "0" };
cvar_t sk_clown_health2 = { "sk_clown_health2", "0" };
cvar_t sk_clown_health3 = { "sk_clown_health3", "0" };

// Dog
cvar_t sk_dog_dmg_bite1 = { "sk_dog_dmg_bite1", "0" };
cvar_t sk_dog_dmg_bite2 = { "sk_dog_dmg_bite2", "0" };
cvar_t sk_dog_dmg_bite3 = { "sk_dog_dmg_bite3", "0" };

cvar_t sk_dog_health1 = { "sk_dog_health1", "0" };
cvar_t sk_dog_health2 = { "sk_dog_health2", "0" };
cvar_t sk_dog_health3 = { "sk_dog_health3", "0" };

// Doktor
cvar_t sk_doktor_dmg_both_slash1 = { "sk_doktor_dmg_both_slash1", "0" };
cvar_t sk_doktor_dmg_both_slash2 = { "sk_doktor_dmg_both_slash2", "0" };
cvar_t sk_doktor_dmg_both_slash3 = { "sk_doktor_dmg_both_slash3", "0" };

cvar_t sk_doktor_dmg_dart1 = { "sk_doktor_dmg_dart1", "0" };
cvar_t sk_doktor_dmg_dart2 = { "sk_doktor_dmg_dart2", "0" };
cvar_t sk_doktor_dmg_dart3 = { "sk_doktor_dmg_dart3", "0" };

cvar_t sk_doktor_dmg_one_slash1 = { "sk_doktor_dmg_one_slash1", "0" };
cvar_t sk_doktor_dmg_one_slash2 = { "sk_doktor_dmg_one_slash2", "0" };
cvar_t sk_doktor_dmg_one_slash3 = { "sk_doktor_dmg_one_slash3", "0" };

cvar_t sk_doktor_health1 = { "sk_doktor_health1", "0" };
cvar_t sk_doktor_health2 = { "sk_doktor_health2", "0" };
cvar_t sk_doktor_health3 = { "sk_doktor_health3", "0" };

// Duck
cvar_t sk_duck_dmg_slash1 = { "sk_duck_dmg_slash1", "0" };
cvar_t sk_duck_dmg_slash2 = { "sk_duck_dmg_slash2", "0" };
cvar_t sk_duck_dmg_slash3 = { "sk_duck_dmg_slash3", "0" };

cvar_t sk_duck_health1 = { "sk_duck_health1", "0" };
cvar_t sk_duck_health2 = { "sk_duck_health2", "0" };
cvar_t sk_duck_health3 = { "sk_duck_health3", "0" };

// Half-Cat
cvar_t sk_halfcat_dmg_bite1 = { "sk_halfcat_dmg_bite1", "0" };
cvar_t sk_halfcat_dmg_bite2 = { "sk_halfcat_dmg_bite2", "0" };
cvar_t sk_halfcat_dmg_bite3 = { "sk_halfcat_dmg_bite3", "0" };

#ifdef USE_MP_WEAPONS
// Half-Cat (as a Snark)
cvar_t sk_hc_snark_dmg_bite1 = { "sk_hc_snark_dmg_bite1", "0" };
cvar_t sk_hc_snark_dmg_bite2 = { "sk_hc_snark_dmg_bite2", "0" };
cvar_t sk_hc_snark_dmg_bite3 = { "sk_hc_snark_dmg_bite3", "0" };
#endif

// Huntsman
cvar_t sk_huntsman_dmg_bolt1 = { "sk_huntsman_dmg_bolt1", "0" };
cvar_t sk_huntsman_dmg_bolt2 = { "sk_huntsman_dmg_bolt2", "0" };
cvar_t sk_huntsman_dmg_bolt3 = { "sk_huntsman_dmg_bolt3", "0" };

cvar_t sk_huntsman_dmg_melee1 = { "sk_huntsman_dmg_melee1", "0" };
cvar_t sk_huntsman_dmg_melee2 = { "sk_huntsman_dmg_melee2", "0" };
cvar_t sk_huntsman_dmg_melee3 = { "sk_huntsman_dmg_melee3", "0" };

cvar_t sk_huntsman_health1 = { "sk_huntsman_health1", "0" };
cvar_t sk_huntsman_health2 = { "sk_huntsman_health2", "0" };
cvar_t sk_huntsman_health3 = { "sk_huntsman_health3", "0" };

// Maskboy
cvar_t sk_maskboy_dmg_slash1 = { "sk_maskboy_dmg_slash1", "0" };
cvar_t sk_maskboy_dmg_slash2 = { "sk_maskboy_dmg_slash2", "0" };
cvar_t sk_maskboy_dmg_slash3 = { "sk_maskboy_dmg_slash3", "0" };

cvar_t sk_maskboy_health1 = { "sk_maskboy_health1", "0" };
cvar_t sk_maskboy_health2 = { "sk_maskboy_health2", "0" };
cvar_t sk_maskboy_health3 = { "sk_maskboy_health3", "0" };

// Pig
cvar_t sk_pig_dmg_bite1 = { "sk_pig_dmg_bite1", "0" };
cvar_t sk_pig_dmg_bite2 = { "sk_pig_dmg_bite2", "0" };
cvar_t sk_pig_dmg_bite3 = { "sk_pig_dmg_bite3", "0" };

cvar_t sk_pig_health1 = { "sk_pig_health1", "0" };
cvar_t sk_pig_health2 = { "sk_pig_health2", "0" };
cvar_t sk_pig_health3 = { "sk_pig_health3", "0" };

// Teddy
cvar_t sk_teddy_dmg_bite1 = { "sk_teddy_dmg_bite1", "0" };
cvar_t sk_teddy_dmg_bite2 = { "sk_teddy_dmg_bite2", "0" };
cvar_t sk_teddy_dmg_bite3 = { "sk_teddy_dmg_bite3", "0" };

cvar_t sk_teddy_health1 = { "sk_teddy_health1", "0" };
cvar_t sk_teddy_health2 = { "sk_teddy_health2", "0" };
cvar_t sk_teddy_health3 = { "sk_teddy_health3", "0" };

// UD Cop
cvar_t sk_udcop_dmg_kick1 = { "sk_udcop_dmg_kick1", "0" };
cvar_t sk_udcop_dmg_kick2 = { "sk_udcop_dmg_kick2", "0" };
cvar_t sk_udcop_dmg_kick3 = { "sk_udcop_dmg_kick3", "0" };

cvar_t sk_udcop_health1 = { "sk_udcop_health1", "0" };
cvar_t sk_udcop_health2 = { "sk_udcop_health2", "0" };
cvar_t sk_udcop_health3 = { "sk_udcop_health3", "0" };

// Zombie
cvar_t sk_zombie_dmg_both_slash1 = { "sk_zombie_dmg_both_slash1", "0" };
cvar_t sk_zombie_dmg_both_slash2 = { "sk_zombie_dmg_both_slash2", "0" };
cvar_t sk_zombie_dmg_both_slash3 = { "sk_zombie_dmg_both_slash3", "0" };

cvar_t sk_zombie_dmg_one_slash1 = { "sk_zombie_dmg_one_slash1", "0" };
cvar_t sk_zombie_dmg_one_slash2 = { "sk_zombie_dmg_one_slash2", "0" };
cvar_t sk_zombie_dmg_one_slash3 = { "sk_zombie_dmg_one_slash3", "0" };

cvar_t sk_zombie_health1 = { "sk_zombie_health1", "0" };
cvar_t sk_zombie_health2 = { "sk_zombie_health2", "0" };
cvar_t sk_zombie_health3 = { "sk_zombie_health3", "0" };

// END Cvars for Skill Level settings

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );

	CVAR_REGISTER (&displaysoundlist);
	CVAR_REGISTER( &allow_spectators );

	CVAR_REGISTER (&teamplay);
	CVAR_REGISTER (&fraglimit);
	CVAR_REGISTER (&timelimit);

	CVAR_REGISTER (&fragsleft);
	CVAR_REGISTER (&timeleft);

	CVAR_REGISTER (&friendlyfire);
	CVAR_REGISTER (&falldamage);
	CVAR_REGISTER (&weaponstay);
	CVAR_REGISTER (&forcerespawn);
	CVAR_REGISTER (&flashlight);
	CVAR_REGISTER (&aimcrosshair);
	CVAR_REGISTER (&decalfrequency);
	CVAR_REGISTER (&teamlist);
	CVAR_REGISTER (&teamoverride);
	CVAR_REGISTER (&defaultteam);
	CVAR_REGISTER (&allowmonsters);

	CVAR_REGISTER (&mp_chattime);

	CVAR_REGISTER( &hrp_bonus_debug_weapon ); // Shepard : Mutator "Debug weapon"
	CVAR_REGISTER( &hrp_bonus_drunk ); // Shepard : Mutator "Always drunk"
	CVAR_REGISTER( &hrp_bonus_fragile ); // Shepard : Mutator "Fragile"

// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Scientist
	CVAR_REGISTER ( &sk_scientist_health1 );// {"sk_scientist_health1","0"};
	CVAR_REGISTER ( &sk_scientist_health2 );// {"sk_scientist_health2","0"};
	CVAR_REGISTER ( &sk_scientist_health3 );// {"sk_scientist_health3","0"};

	// PLAYER WEAPONS

	// Cavalry saber whack
	CVAR_REGISTER( &sk_plr_saber1 );
	CVAR_REGISTER( &sk_plr_saber2 );
	CVAR_REGISTER( &sk_plr_saber3 );

	// Colt SAA Round
	CVAR_REGISTER( &sk_plr_colt_bullet1 );
	CVAR_REGISTER( &sk_plr_colt_bullet2 );
	CVAR_REGISTER( &sk_plr_colt_bullet3 );

	// Henry rifle Round
	CVAR_REGISTER( &sk_plr_henry_bullet1 );
	CVAR_REGISTER( &sk_plr_henry_bullet2 );
	CVAR_REGISTER( &sk_plr_henry_bullet3 );

	// Double barrel shotgun buckshot
	CVAR_REGISTER( &sk_plr_buckshot1 );
	CVAR_REGISTER( &sk_plr_buckshot2 );
	CVAR_REGISTER( &sk_plr_buckshot3 );

#ifdef USE_MP_WEAPONS
	// Bow Arrow
	CVAR_REGISTER( &sk_plr_bow_arrow1 );
	CVAR_REGISTER( &sk_plr_bow_arrow2 );
	CVAR_REGISTER( &sk_plr_bow_arrow3 );
#endif

	// Gatling Round
	CVAR_REGISTER( &sk_plr_gatling_bullet1 );
	CVAR_REGISTER( &sk_plr_gatling_bullet2 );
	CVAR_REGISTER( &sk_plr_gatling_bullet3 );

	// Dynamite
	CVAR_REGISTER( &sk_plr_dynamite1 );
	CVAR_REGISTER( &sk_plr_dynamite2 );
	CVAR_REGISTER( &sk_plr_dynamite3 );

	// WORLD WEAPONS
	CVAR_REGISTER( &sk_colt_bullet1 );
	CVAR_REGISTER( &sk_colt_bullet2 );
	CVAR_REGISTER( &sk_colt_bullet3 );

	CVAR_REGISTER( &sk_henry_bullet1 );
	CVAR_REGISTER( &sk_henry_bullet2 );
	CVAR_REGISTER( &sk_henry_bullet3 );

	CVAR_REGISTER ( &sk_12mm_bullet1 );// {"sk_12mm_bullet1","0"};
	CVAR_REGISTER ( &sk_12mm_bullet2 );// {"sk_12mm_bullet2","0"};
	CVAR_REGISTER ( &sk_12mm_bullet3 );// {"sk_12mm_bullet3","0"};

	// HEALTH/SUIT CHARGE DISTRIBUTION
	CVAR_REGISTER ( &sk_suitcharger1 );
	CVAR_REGISTER ( &sk_suitcharger2 );
	CVAR_REGISTER ( &sk_suitcharger3 );

	CVAR_REGISTER ( &sk_battery1 );
	CVAR_REGISTER ( &sk_battery2 );
	CVAR_REGISTER ( &sk_battery3 );

	CVAR_REGISTER ( &sk_healthcharger1 );
	CVAR_REGISTER ( &sk_healthcharger2 );
	CVAR_REGISTER ( &sk_healthcharger3 );

	CVAR_REGISTER ( &sk_healthkit1 );
	CVAR_REGISTER ( &sk_healthkit2 );
	CVAR_REGISTER ( &sk_healthkit3 );

	CVAR_REGISTER ( &sk_scientist_heal1 );
	CVAR_REGISTER ( &sk_scientist_heal2 );
	CVAR_REGISTER ( &sk_scientist_heal3 );

	CVAR_REGISTER( &sk_whaleoil1 );
	CVAR_REGISTER( &sk_whaleoil2 );
	CVAR_REGISTER( &sk_whaleoil3 );

// monster damage adjusters
	CVAR_REGISTER ( &sk_monster_head1 );
	CVAR_REGISTER ( &sk_monster_head2 );
	CVAR_REGISTER ( &sk_monster_head3 );

	CVAR_REGISTER ( &sk_monster_chest1 );
	CVAR_REGISTER ( &sk_monster_chest2 );
	CVAR_REGISTER ( &sk_monster_chest3 );

	CVAR_REGISTER ( &sk_monster_stomach1 );
	CVAR_REGISTER ( &sk_monster_stomach2 );
	CVAR_REGISTER ( &sk_monster_stomach3 );

	CVAR_REGISTER ( &sk_monster_arm1 );
	CVAR_REGISTER ( &sk_monster_arm2 );
	CVAR_REGISTER ( &sk_monster_arm3 );

	CVAR_REGISTER ( &sk_monster_leg1 );
	CVAR_REGISTER ( &sk_monster_leg2 );
	CVAR_REGISTER ( &sk_monster_leg3 );

// player damage adjusters
	CVAR_REGISTER ( &sk_player_head1 );
	CVAR_REGISTER ( &sk_player_head2 );
	CVAR_REGISTER ( &sk_player_head3 );

	CVAR_REGISTER ( &sk_player_chest1 );
	CVAR_REGISTER ( &sk_player_chest2 );
	CVAR_REGISTER ( &sk_player_chest3 );

	CVAR_REGISTER ( &sk_player_stomach1 );
	CVAR_REGISTER ( &sk_player_stomach2 );
	CVAR_REGISTER ( &sk_player_stomach3 );

	CVAR_REGISTER ( &sk_player_arm1 );
	CVAR_REGISTER ( &sk_player_arm2 );
	CVAR_REGISTER ( &sk_player_arm3 );

	CVAR_REGISTER ( &sk_player_leg1 );
	CVAR_REGISTER ( &sk_player_leg2 );
	CVAR_REGISTER ( &sk_player_leg3 );

	// HR:P MONSTERS

	// Armored Fucker
	CVAR_REGISTER( &sk_armorman_health1 );
	CVAR_REGISTER( &sk_armorman_health2 );
	CVAR_REGISTER( &sk_armorman_health3 );

	// Baph
	CVAR_REGISTER( &sk_baph_dmg_slash1 );
	CVAR_REGISTER( &sk_baph_dmg_slash2 );
	CVAR_REGISTER( &sk_baph_dmg_slash3 );

	CVAR_REGISTER( &sk_baph_health1 );
	CVAR_REGISTER( &sk_baph_health2 );
	CVAR_REGISTER( &sk_baph_health3 );

	// Bear
	CVAR_REGISTER( &sk_bear_dmg_both_slash1 );
	CVAR_REGISTER( &sk_bear_dmg_both_slash2 );
	CVAR_REGISTER( &sk_bear_dmg_both_slash3 );

	CVAR_REGISTER( &sk_bear_dmg_one_slash1 );
	CVAR_REGISTER( &sk_bear_dmg_one_slash2 );
	CVAR_REGISTER( &sk_bear_dmg_one_slash3 );

	CVAR_REGISTER( &sk_bear_health1 );
	CVAR_REGISTER( &sk_bear_health2 );
	CVAR_REGISTER( &sk_bear_health3 );

	// Bernard
	CVAR_REGISTER( &sk_bernard_dmg_puke1 );
	CVAR_REGISTER( &sk_bernard_dmg_puke2 );
	CVAR_REGISTER( &sk_bernard_dmg_puke3 );

	CVAR_REGISTER( &sk_bernard_dmg_slash1 );
	CVAR_REGISTER( &sk_bernard_dmg_slash2 );
	CVAR_REGISTER( &sk_bernard_dmg_slash3 );

	CVAR_REGISTER( &sk_bernard_health1 );
	CVAR_REGISTER( &sk_bernard_health2 );
	CVAR_REGISTER( &sk_bernard_health3 );

	// Caleb
	CVAR_REGISTER( &sk_caleb_dmg_both_slash1 );
	CVAR_REGISTER( &sk_caleb_dmg_both_slash2 );
	CVAR_REGISTER( &sk_caleb_dmg_both_slash3 );

	CVAR_REGISTER( &sk_caleb_dmg_one_slash1 );
	CVAR_REGISTER( &sk_caleb_dmg_one_slash2 );
	CVAR_REGISTER( &sk_caleb_dmg_one_slash3 );

	CVAR_REGISTER( &sk_caleb_health1 );
	CVAR_REGISTER( &sk_caleb_health2 );
	CVAR_REGISTER( &sk_caleb_health3 );

	// Clown
	CVAR_REGISTER( &sk_clown_dmg_both_slash1 );
	CVAR_REGISTER( &sk_clown_dmg_both_slash2 );
	CVAR_REGISTER( &sk_clown_dmg_both_slash3 );

	CVAR_REGISTER( &sk_clown_dmg_one_slash1 );
	CVAR_REGISTER( &sk_clown_dmg_one_slash2 );
	CVAR_REGISTER( &sk_clown_dmg_one_slash3 );

	CVAR_REGISTER( &sk_clown_health1 );
	CVAR_REGISTER( &sk_clown_health2 );
	CVAR_REGISTER( &sk_clown_health3 );

	// Dog
	CVAR_REGISTER( &sk_dog_dmg_bite1 );
	CVAR_REGISTER( &sk_dog_dmg_bite2 );
	CVAR_REGISTER( &sk_dog_dmg_bite3 );

	CVAR_REGISTER( &sk_dog_health1 );
	CVAR_REGISTER( &sk_dog_health2 );
	CVAR_REGISTER( &sk_dog_health3 );

	// Doktor
	CVAR_REGISTER( &sk_doktor_dmg_both_slash1 );
	CVAR_REGISTER( &sk_doktor_dmg_both_slash2 );
	CVAR_REGISTER( &sk_doktor_dmg_both_slash3 );

	CVAR_REGISTER( &sk_doktor_dmg_dart1 );
	CVAR_REGISTER( &sk_doktor_dmg_dart2 );
	CVAR_REGISTER( &sk_doktor_dmg_dart3 );

	CVAR_REGISTER( &sk_doktor_dmg_one_slash1 );
	CVAR_REGISTER( &sk_doktor_dmg_one_slash2 );
	CVAR_REGISTER( &sk_doktor_dmg_one_slash3 );

	CVAR_REGISTER( &sk_doktor_health1 );
	CVAR_REGISTER( &sk_doktor_health2 );
	CVAR_REGISTER( &sk_doktor_health3 );

	// Duck
	CVAR_REGISTER( &sk_duck_dmg_slash1 );
	CVAR_REGISTER( &sk_duck_dmg_slash2 );
	CVAR_REGISTER( &sk_duck_dmg_slash3 );

	CVAR_REGISTER( &sk_duck_health1 );
	CVAR_REGISTER( &sk_duck_health2 );
	CVAR_REGISTER( &sk_duck_health3 );

	// Half-Cat
	CVAR_REGISTER( &sk_halfcat_dmg_bite1 );
	CVAR_REGISTER( &sk_halfcat_dmg_bite2 );
	CVAR_REGISTER( &sk_halfcat_dmg_bite3 );

#ifdef USE_MP_WEAPONS
	// Half-Cat (as a Snark)
	CVAR_REGISTER( &sk_hc_snark_dmg_bite1 );
	CVAR_REGISTER( &sk_hc_snark_dmg_bite2 );
	CVAR_REGISTER( &sk_hc_snark_dmg_bite3 );
#endif

	// Huntsman
	CVAR_REGISTER( &sk_huntsman_dmg_bolt1 );
	CVAR_REGISTER( &sk_huntsman_dmg_bolt2 );
	CVAR_REGISTER( &sk_huntsman_dmg_bolt3 );

	CVAR_REGISTER( &sk_huntsman_dmg_melee1 );
	CVAR_REGISTER( &sk_huntsman_dmg_melee2 );
	CVAR_REGISTER( &sk_huntsman_dmg_melee3 );

	CVAR_REGISTER( &sk_huntsman_health1 );
	CVAR_REGISTER( &sk_huntsman_health2 );
	CVAR_REGISTER( &sk_huntsman_health3 );

	// Maskboy
	CVAR_REGISTER( &sk_maskboy_dmg_slash1 );
	CVAR_REGISTER( &sk_maskboy_dmg_slash2 );
	CVAR_REGISTER( &sk_maskboy_dmg_slash3 );

	CVAR_REGISTER( &sk_maskboy_health1 );
	CVAR_REGISTER( &sk_maskboy_health2 );
	CVAR_REGISTER( &sk_maskboy_health3 );

	// Pig
	CVAR_REGISTER( &sk_pig_dmg_bite1 );
	CVAR_REGISTER( &sk_pig_dmg_bite2 );
	CVAR_REGISTER( &sk_pig_dmg_bite3 );

	CVAR_REGISTER( &sk_pig_health1 );
	CVAR_REGISTER( &sk_pig_health2 );
	CVAR_REGISTER( &sk_pig_health3 );

	// Teddy
	CVAR_REGISTER( &sk_teddy_dmg_bite1 );
	CVAR_REGISTER( &sk_teddy_dmg_bite2 );
	CVAR_REGISTER( &sk_teddy_dmg_bite3 );

	CVAR_REGISTER( &sk_teddy_health1 );
	CVAR_REGISTER( &sk_teddy_health2 );
	CVAR_REGISTER( &sk_teddy_health3 );

	// UD Cop
	CVAR_REGISTER( &sk_udcop_dmg_kick1 );
	CVAR_REGISTER( &sk_udcop_dmg_kick2 );
	CVAR_REGISTER( &sk_udcop_dmg_kick3 );

	CVAR_REGISTER( &sk_udcop_health1 );
	CVAR_REGISTER( &sk_udcop_health2 );
	CVAR_REGISTER( &sk_udcop_health3 );

	// Zombie
	CVAR_REGISTER( &sk_zombie_dmg_both_slash1 );
	CVAR_REGISTER( &sk_zombie_dmg_both_slash2 );
	CVAR_REGISTER( &sk_zombie_dmg_both_slash3 );

	CVAR_REGISTER( &sk_zombie_dmg_one_slash1 );
	CVAR_REGISTER( &sk_zombie_dmg_one_slash2 );
	CVAR_REGISTER( &sk_zombie_dmg_one_slash3 );

	CVAR_REGISTER( &sk_zombie_health1 );
	CVAR_REGISTER( &sk_zombie_health2 );
	CVAR_REGISTER( &sk_zombie_health3 );
// END REGISTER CVARS FOR SKILL LEVEL STUFF

	SERVER_COMMAND( "exec skill.cfg\n" );
}

