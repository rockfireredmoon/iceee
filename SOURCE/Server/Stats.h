#pragma once

#ifndef STATS_H
#define STATS_H

#include <string>
#include <stdio.h>
#include <string.h>
#include "ByteBuffer.h"
#include "CommonTypes.h"

//Macro to get the size of a structure member.
//Derived from the offsetof() macro, which (for the 32 bit version) is defined as:
//  #define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#define msizeof(s,m)   sizeof(((s *)0)->m)

//Quick macro which calles both the msizeof() macro above, and the offsetof() macro
//to fill in some structure data for the stats.
//#define SIZEOFFSET(s, m)    {msizeof(s, m), offsetof(s, m)},

struct CharacterStatSet
{
	std::string appearance;  //char appearance[4096];
	std::string eq_appearance;  //char eq_appearance[512];
	short level;
	char display_name[64];
	short strength;
	short dexterity;
	short constitution;
	short psyche;
	short spirit;
	short armor_rating;

	short melee_attack_speed;
	short magic_attack_speed;
	short base_damage_melee;
	short base_damage_fire;
	short base_damage_frost;
	short base_damage_mystic;
	short base_damage_death;

	short damage_resist_melee;
	short damage_resist_fire;
	short damage_resist_frost;
	short damage_resist_mystic;
	short damage_resist_death;

	short base_movement;
	short base_luck;
	short base_health;

	short will_max;
	float will_regen;
	short might_max;
	float might_regen;

	short base_dodge;
	short base_deflect;
	short base_parry;
	short base_block;

	short base_melee_to_hit;
	short base_melee_critical;
	short base_magic_success;
	short base_magic_critical;

	short offhand_weapon_damage;
	short casting_setback_chance;
	short channeling_break_chance;

	short base_healing;
	short aggro_radius_mod;
	short hate_gain_rate;
	short experience_gain_rate;
	short coin_gain_rate;
	short magic_loot_drop_rate;
	short inventory_capacity;
	short vis_weapon;

	char sub_name[64];
	char alternate_idle_anim[32];
	char selective_eq_override[32];

	int health;  //Note: Official clients use short, but expanded here with client modifications.
	short will;
	short will_charges;
	short might;
	short might_charges;
	short size;
	short profession;

	short weapon_damage_2h;
	short weapon_damage_1h;
	short weapon_damage_pole;
	short weapon_damage_small;
	short weapon_damage_box;
	short weapon_damage_thrown;
	short weapon_damage_wand;
	short extra_damage_fire;
	short extra_damage_frost;
	short extra_damage_mystic;
	short extra_damage_death;

	std::string appearance_override;
	float invisibility_distance;
	std::string translocate_destination;

	std::string lootable_player_ids;

	short master;
	short rez_pending;

	std::string loot_seeable_player_ids;
	char loot[64];
	char base_stats[64];

	short health_mod;
	short heroism;

	int experience;
	short total_ability_points;
	short current_ability_points;

	int copper;
	int credits;

	char ai_module[64];
	char creature_category[32];
	short rarity;
	short spawn_table_points;
	char ai_package[64];
	
	short bonus_health;

	int dr_mod_melee;
	int dr_mod_fire;
	int dr_mod_frost;
	int dr_mod_mystic;
	int dr_mod_death;

	int dmg_mod_melee;
	int dmg_mod_fire;
	int dmg_mod_frost;
	int dmg_mod_mystic;
	int dmg_mod_death;

	short pvp_team;
	int pvp_kills;
	int pvp_deaths;

	//int pvp_score;  //removed by 88?
	char pvp_state[32];

	int pvp_flag_captures;

	float mod_melee_to_hit;
	float mod_melee_to_crit;
	float mod_magic_to_hit;
	float mod_magic_to_crit;
	float mod_parry;
	float mod_block;
	int mod_movement;
	float mod_health_regen;
	float mod_attack_speed;
	float mod_casting_speed;
	float mod_healing;
	float total_size;
	short aggro_players;

	//Were in 8.8 but not 06
	float mod_luck;
	short health_regen;
	short bleeding;
	int damage_shield;
	short hide_nameboard;
	short hide_minimap;

	//ICEEE
	short credit_drops;
	short heroism_gain_rate;
	short quest_exp_gain_rate;
	short drop_gain_rate;

	CharacterStatSet();
	void CopyFrom(CharacterStatSet *source);
	void Clear(void);
	void SetAppearance(const char *data);
	void SetEqAppearance(const char *data);
	void SetAIPackage(const char *data);
	void SetDisplayName(const char *name);
	void SetSubName(const char *name);
	bool IsPropAppearance(void);
	void ClearLootSeeablePlayerIDs();
	void ClearLootablePlayerIDs();
};

// This information is specific to the server, to assist in managing raw data within the
// character stat set.
struct StatType
{
	enum
	{
		SHORT   = 0,    //2 bytes
		INTEGER = 1,    //4 bytes
		FLOAT   = 2,    //4 bytes
		CSTRING = 3,    //char[] array, application defined fixed-length
		STRING  = 4     //std::string
	};
};

struct StatDefinition
{
	short ID;
	int etype;  //Enumerated type ID, corresponds to StatType::*
	const char *type;   //String type name
	const char *name;
	const char *prettyName;
	const char *abbrev;
	short debuff;

	//The following three fields were added specifically for the server program
	//The rest of the are converted exactly from their official tables in the game files 
	short size;      //Memory size of the associated variable in the structure [sizeof()]
	short offset;    //Data offset into the CharacterStatSet structure [offsetof()]
	long updflag;    //Update flags, controls which stat updates are sent to the client on certain operations

	bool isNumericalType(void);
};

int GetStatType(const char *name);

struct DebuffType
{
	enum DebuffTypeEnum
	{
		NONE = 0,
		CONTROL = 1,
		STAT = 2
	};
};

enum StatUpdateType
{
	SUT_None        = 0x00,    //Don't use this stat in any updates
	SUT_Player      = 0x01,    //Use this in an avatar update
	SUT_NonPlayer   = 0x02,    //Use this in a mob update (hostile creature)
	SUT_Appearance  = 0x04,
	SUT_CDefExt     = 0x08,    //For extended stats found in versions above 0.8.6.

	//Auto combine the important ones
	SUT_All = SUT_Player | SUT_NonPlayer
};

namespace STAT
{
	enum StatEnum
	{
	APPEARANCE               = 0,
	EQ_APPEARANCE            = 1,
	LEVEL                    = 2,

	DISPLAY_NAME             = 3,
	STRENGTH                 = 4,
	DEXTERITY                = 5,
	CONSTITUTION             = 6,
	PSYCHE                   = 7,
	SPIRIT                   = 8,

	ARMOR_RATING             = 30,
	MELEE_ATTACK_SPEED       = 31,
	MAGIC_ATTACK_SPEED       = 32,
	BASE_DAMAGE_MELEE        = 33,
	BASE_DAMAGE_FIRE         = 34,
	BASE_DAMAGE_FROST        = 35,
	BASE_DAMAGE_MYSTIC       = 36,
	BASE_DAMAGE_DEATH        = 37,
	DAMAGE_RESIST_MELEE      = 38,

	DAMAGE_RESIST_FIRE       = 39,
	DAMAGE_RESIST_FROST      = 40,
	DAMAGE_RESIST_MYSTIC     = 41,
	DAMAGE_RESIST_DEATH      = 42,
	BASE_MOVEMENT            = 43,
	BASE_LUCK                = 44,
	BASE_HEALTH              = 45,
	WILL_MAX                 = 46,
	WILL_REGEN               = 47,
	MIGHT_MAX                = 48,
	MIGHT_REGEN              = 49,
	BASE_DODGE               = 50,

	BASE_DEFLECT             = 51,
	BASE_PARRY               = 52,
	BASE_BLOCK               = 53,
	BASE_MELEE_TO_HIT        = 54,
	BASE_MELEE_CRITICAL      = 55,
	BASE_MAGIC_SUCCESS       = 56,
	BASE_MAGIC_CRITICAL      = 57,
	OFFHAND_WEAPON_DAMAGE    = 58,
	CASTING_SETBACK_CHANCE   = 59,
	CHANNELING_BREAK_CHANCE  = 60,
	BASE_HEALING             = 61,
	AGGRO_RADIUS_MOD         = 62,
	HATE_GAIN_RATE           = 63,
	EXPERIENCE_GAIN_RATE     = 64,
	COIN_GAIN_RATE           = 65,
	MAGIC_LOOT_DROP_RATE     = 66,

	INVENTORY_CAPACITY       = 67,
	VIS_WEAPON               = 69,
	SUB_NAME                 = 70,
	ALTERNATE_IDLE_ANIM      = 71,
	SELECTIVE_EQ_OVERRIDE    = 72,

	HEALTH                   = 80,
	WILL                     = 81,
	WILL_CHARGES             = 82,
	MIGHT                    = 83,
	MIGHT_CHARGES            = 84,
	SSIZE                    = 85,    //Compiler error due to reserved definition "SIZE"
	PROFESSION               = 86,


	WEAPON_DAMAGE_2H         = 87,
	WEAPON_DAMAGE_1H         = 88,
	WEAPON_DAMAGE_POLE       = 89,
	WEAPON_DAMAGE_SMALL      = 90,
	WEAPON_DAMAGE_BOW        = 91,
	WEAPON_DAMAGE_THROWN     = 92,
	WEAPON_DAMAGE_WAND       = 93,
	EXTRA_DAMAGE_FIRE        = 94,
	EXTRA_DAMAGE_FROST       = 95,
	EXTRA_DAMAGE_MYSTIC      = 96,
	EXTRA_DAMAGE_DEATH       = 97,

	APPEARANCE_OVERRIDE      = 98,
	INVISIBILITY_DISTANCE    = 99,
	TRANSLOCATE_DESTINATION  = 100,
	LOOTABLE_PLAYER_IDS      = 101,

	MASTER                   = 102,
	REZ_PENDING              = 103,
	LOOT_SEEABLE_PLAYER_IDS  = 104,
	LOOT                     = 105,
	BASE_STATS               = 107,
	HEALTH_MOD               = 108,
	HEROISM                  = 110,
	EXPERIENCE               = 111,
	TOTAL_ABILITY_POINTS     = 112,
	CURRENT_ABILITY_POINTS   = 113,

	COPPER                   = 114,
	CREDITS                  = 115,

	AI_MODULE                = 120,
	CREATURE_CATEGORY        = 121,
	RARITY                   = 122,
	SPAWN_TABLE_POINTS       = 123,
	AI_PACKAGE               = 124,
	BONUS_HEALTH             = 125,
	DR_MOD_MELEE             = 126,
	DR_MOD_FIRE              = 127,
	DR_MOD_FROST             = 128,
	DR_MOD_MYSTIC            = 129,
	DR_MOD_DEATH             = 130,

	DMG_MOD_MELEE            = 131,
	DMG_MOD_FIRE             = 132,
	DMG_MOD_FROST            = 133,
	DMG_MOD_MYSTIC           = 134,
	DMG_MOD_DEATH            = 135,

	PVP_TEAM                 = 136,

	PVP_KILLS                = 137,   //Thought they were deleted in 88 but apparently not
	PVP_DEATHS               = 138,

	//PVP_SCORE                = 138,
	PVP_STATE                = 139,
	PVP_FLAG_CAPTURES        = 140,

	MOD_MELEE_TO_HIT         = 141,
	MOD_MELEE_TO_CRIT        = 142,
	MOD_MAGIC_TO_HIT         = 143,
	MOD_MAGIC_TO_CRIT        = 144,
	MOD_PARRY                = 145,
	MOD_BLOCK                = 146,

	MOD_MOVEMENT             = 147,
	MOD_HEALTH_REGEN         = 148,
	MOD_ATTACK_SPEED         = 149,
	MOD_CASTING_SPEED        = 150,
	MOD_HEALING              = 151,

	TOTAL_SIZE               = 152,
	AGGRO_PLAYERS            = 153,

	//New ones found in 8.6, not sure about between versions
	MOD_LUCK                 = 154,
	HEALTH_REGEN             = 155,
	BLEEDING                 = 156,
	DAMAGE_SHIELD            = 157,
	HIDE_NAMEBOARD           = 158,

	//New ones found in 8.8, not sure about between versions
	HIDE_MINIMAP             = 159,

	//ICEEE - Credit drops and bonus gain rates
	CREDIT_DROPS             = 160,
	HEROISM_GAIN_RATE        = 161,
	QUEST_EXP_GAIN_RATE      = 162,
	DROP_GAIN_RATE      	 = 163,
	};
}

namespace StatusEffects
{
	enum
	{
		DEAD = 0,
		SILENCE = 1,
		DISARM = 2,
		STUN = 3,
		DAZE = 4,
		CHARM = 5,
		FEAR = 6,
		ROOT = 7,
		LOCKED = 8,
		BROKEN = 9,
		CAN_USE_WEAPON_2H = 10,
		CAN_USE_WEAPON_1H = 11,
		CAN_USE_WEAPON_SMALL = 12,
		CAN_USE_WEAPON_POLE = 13,
		CAN_USE_WEAPON_BOW = 14,
		CAN_USE_WEAPON_THROWN = 15,
		CAN_USE_WEAPON_WAND = 16,
		CAN_USE_DUAL_WIELD = 17,
		CAN_USE_PARRY = 18,
		CAN_USE_BLOCK = 19,
		CAN_USE_ARMOR_CLOTH = 20,
		CAN_USE_ARMOR_LIGHT = 21,
		CAN_USE_ARMOR_MEDIUM = 22,
		CAN_USE_ARMOR_HEAVY = 23,
		INVISIBLE = 24,
		ALL_SEEING = 25,
		IN_COMBAT_STAND = 26,
		INVINCIBLE = 27,
		FLEE = 28,
		NO_AGGRO_GAINED = 29,
		UNATTACKABLE = 30,
		IS_USABLE = 31,
		CLIENT_LOADING = 32,
		IMMUNE_INTERRUPT = 33,
		IMMUNE_SILENCE = 34,
		IMMUNE_DISARM = 35,
		IMMUNE_BLEED = 36,
		IMMUNE_DAMAGE_FIRE = 37,
		IMMUNE_DAMAGE_FROST = 38,
		IMMUNE_DAMAGE_MYSTIC = 39,
		IMMUNE_DAMAGE_DEATH = 40,
		IMMUNE_DAMAGE_MELEE = 41,
		DISABLED = 42,
		AUTO_ATTACK = 43,
		AUTO_ATTACK_RANGED = 44,
		CARRYING_RED_FLAG = 45,
		CARRYING_BLUE_FLAG = 46,
		HENGE = 47,
		TRANSFORMER = 48,
		PVPABLE = 49,
		IN_COMBAT = 50,
		WALK_IN_SHADOWS = 51,
		EVADE = 52,
		TAUNTED = 53,
		XP_BOOST = 54,
		REAGENT_GENERATOR = 55,
		RES_PENALTY = 56,
		IMMUNE_STUN = 57,
		IMMUNE_DAZE = 58,
		GM_FROZEN = 59,
		GM_INVISIBLE = 60,
		GM_SILENCED = 61,

		//Custom modded status effects
		UNKILLABLE = 62,
		TRANSFORMED = 63,
		INVISIBLE_EQUIPMENT = 64,
		USABLE_BY_COMBATANT = 65

	};
}

struct StatusEffectBitInfo
{
	short effectID;
	short arrayIndex;
	unsigned int bit;
	const char *name;
};

struct CreatureRarityType
{
	enum Enum
	{
		NORMAL = 0,
		HEROIC = 1,
		EPIC   = 2,
		LEGEND = 3
	};
};

const int MAX_RARITY_INDEX = 3;
const float RarityTypeHealthModifier[4] = 
{
	1.0F,  //Normal
	2.5F,  //Heroic
	5.0F,  //Epic
	10.0F, //Legend
};

struct BaseStat
{
	enum Enum
	{
		High = 0,
		Med  = 1,
		Low  = 2
	};
};

struct Professions
{
	enum Enum
	{
		NONE    = 0,
		KNIGHT  = 1,
		ROGUE   = 2,
		MAGE    = 3,
		DRUID   = 4,
		MONSTER = 5,
		MAX     = 5
	};
	static const char *GetAbbreviation(int professionID)
	{
		switch(professionID)
		{
		case KNIGHT: return "K";
		case ROGUE: return "R";
		case MAGE: return "M";
		case DRUID: return "D";
		}
		return "x";
	}
};

struct BaseStatMap
{
	enum Enum
	{
		Str = 0,
		Dex = 1,
		Con = 2,
		Psy = 3,
		Spi = 4
	};
};

namespace StatManager
{
	char GetStatType(int statID);
	void SetHealthToInteger(bool setting);
}

extern StatusEffectBitInfo StatusEffectBitData[];
#define MAX_STATUSEFFECTBYTES  3

extern const int MAX_LEVEL;
extern short LevelBaseStats[71][3];
extern short ProfBaseStats[6][5];

const int NumStats = 129;
extern StatDefinition StatList[129];

int GetStatIndex(short StatID);
int GetStatIndexByName(const char *name);
StatDefinition* GetStatDefByName(const char *name);
int GetStatusIDByName(const char *name);
const char* GetStatusNameByID(int id);

int WriteCurrentStatToBuffer(char *buffer, short StatID, CharacterStatSet *css);
int WriteStatToBuffer(char *buffer, short StatID, float value);
int WriteStatToSet(int StatID, const char *value, CharacterStatSet *css);
int WriteStatToSetByName(const char *name, const char *value, CharacterStatSet *css);

bool isStatZero(int StatIndex, CharacterStatSet *css);
bool isStatEqual(int StatIndex, CharacterStatSet *css1, CharacterStatSet *css2);
const char * GetStatValueAsString(int StatIndex, char *ConvBuf, CharacterStatSet *css);
int WriteStatToFile(int StatIndex, CharacterStatSet *css, FILE *file);
int WriteStatToFileByName(char *name, CharacterStatSet *css, FILE *file);
float GetStatValueByID(int statID, CharacterStatSet *css);
int WriteValueToStat(int StatID, float value, CharacterStatSet *css);

void SetHealthToInteger(bool setting);

namespace StatInfo
{
	void GeneratePrettyStatTable(MULTISTRING &output, CharacterStatSet *css);
}

int PrepExt_SendExperience(char *buffer, int CreatureID, int ExpAmount);
int PrepExt_SendEqAppearance(char *buffer, int creatureDefID, const char *eqAppearance);
int PrepExt_SendVisWeapon(char *buffer, int CreatureID, short visWeapon);
int PrepExt_SendHealth(char *buffer, long CreatureID, int healthAmount);
int PrepExt_UpdateCreatureDef(char *buffer, int CDefID, int defHints, std::vector<short>& statID, CharacterStatSet *css);
int WriteCharacterStats(CharacterStatSet *clIndex, char *buffer, int &wpos, int flagMask);

#endif // STATS_H
