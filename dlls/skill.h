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
// skill.h - skill level concerns
//=========================================================

struct skilldata_t
{

	int iSkillLevel; // game skill level

// Player Weapons
	float plrDmgSaber;
	float plrDmgColt;
	float plrDmgHenry;
	float plrDmgBuckshot;
#ifdef USE_MP_WEAPONS
	float plrDmgBowArrow;
#endif
	float plrDmgGatling;
	float plrDmgDynamite;

// weapons shared by monsters
	float monDmgColt;
	float monDmgHenry;
	float monDmg12MM;

// health/suit charge
	float whaleOilCapacity;

// monster damage adj
	float monHead;
	float monChest;
	float monStomach;
	float monLeg;
	float monArm;

// player damage adj
	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;

// HR:P Monsters
	float armormanHealth;
	float baphDmgSlash;
	float baphHealth;
	float bearDmgBothSlash;
	float bearDmgOneSlash;
	float bearHealth;
	float bernardDmgPuke;
	float bernardDmgSlash;
	float bernardHealth;
	float calebDmgBothSlash;
	float calebDmgOneSlash;
	float calebHealth;
	float clownDmgBothSlash;
	float clownDmgOneSlash;
	float clownHealth;
	float dogDmgBite;
	float dogHealth;
	float doktorDmgBothSlash;
	float doktorDmgDart;
	float doktorDmgOneSlash;
	float doktorHealth;
	float duckDmgSlash;
	float duckHealth;
	float halfcatDmgBite;
#ifdef USE_MP_WEAPONS
	float hcSnarkDmgBite;
#endif
	float huntsmanDmgBolt;
	float huntsmanDmgMelee;
	float huntsmanHealth;
	float maskboyDmgSlash;
	float maskboyHealth;
	float pigDmgBite;
	float pigHealth;
	float teddyDmgBite;
	float teddyHealth;
	float udcopDmgKick;
	float udcopHealth;
	float zombieDmgBothSlash;
	float zombieDmgOneSlash;
	float zombieHealth;
};

extern	DLL_GLOBAL	skilldata_t	gSkillData;
float GetSkillCvar( char *pName );

extern DLL_GLOBAL int		g_iSkillLevel;

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
