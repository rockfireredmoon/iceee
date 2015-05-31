#include <stdlib.h>
#include <string.h>
#include "Stats.h"
#include "StringList.h"
#include "Util.h"

#ifndef WINDOWS_PLATFORM
#include <stddef.h>
#endif

/*
DAMAGE_RESIST_MYSTIC  - Armor rating (from equipment)
DR_MOD_MYSTIC         - Elemental resistance (positive value, percent reduction from incoming damage)
DMG_MOD_MYSTIC        - Element specialization (positive value, percent amplification to outgoing damage)
BASE_DAMAGE_MYSTIC    - Elemental buff (applied on auto-attacks)
BONUS_HEALTH          - Numeric amount added by the Frost Shield skill.  Intended to absorb damage.
*/

bool StatDefinition::isNumericalType(void)
{
	switch(etype)
	{
	case StatType::FLOAT:
	case StatType::INTEGER:
	case StatType::SHORT:
		return true;
	default:
		return false;
	}
	return false;
}

StatDefinition StatList[] =
{
	//ID								Type(Enum)	 type(string)        name           prettyName    abbrev, debuff,  size,  offset     

	//10
	{STAT::APPEARANCE                  , StatType::STRING,  "string", "appearance",    "Appearance",           NULL, 0, msizeof(CharacterStatSet, appearance), offsetof(CharacterStatSet, appearance), SUT_All | SUT_Appearance },
	{STAT::EQ_APPEARANCE               , StatType::STRING,  "string", "eq_appearance", "Equipment Appearance", NULL, 0, msizeof(CharacterStatSet, eq_appearance), offsetof(CharacterStatSet, eq_appearance), SUT_All | SUT_Appearance },
	{STAT::LEVEL                       , StatType::SHORT,     "short",  "level",         "Level",                NULL, 0, msizeof(CharacterStatSet, level), offsetof(CharacterStatSet, level), SUT_All },
	{STAT::DISPLAY_NAME                , StatType::CSTRING,  "string", "display_name",  "Display Name",         NULL, 0, msizeof(CharacterStatSet, display_name), offsetof(CharacterStatSet, display_name), SUT_All },
	{STAT::STRENGTH                    , StatType::SHORT,     "short",  "strength",      "Strength",            "STR", DebuffType::STAT, msizeof(CharacterStatSet, strength), offsetof(CharacterStatSet, strength), SUT_All },
	{STAT::DEXTERITY                   , StatType::SHORT,     "short",  "dexterity",     "Dexterity",           "DEX", DebuffType::STAT, msizeof(CharacterStatSet, dexterity), offsetof(CharacterStatSet, dexterity), SUT_All },
	{STAT::CONSTITUTION                , StatType::SHORT,     "short",  "constitution",  "Constitution",        "CON", DebuffType::STAT, msizeof(CharacterStatSet, constitution), offsetof(CharacterStatSet, constitution), SUT_All },
	{STAT::PSYCHE                      , StatType::SHORT,     "short",  "psyche",        "Psyche",              "PSY", DebuffType::STAT, msizeof(CharacterStatSet, psyche), offsetof(CharacterStatSet, psyche), SUT_All },
	{STAT::SPIRIT                      , StatType::SHORT,     "short",  "spirit",        "Spirit",              "SPI", DebuffType::STAT, msizeof(CharacterStatSet, spirit), offsetof(CharacterStatSet, spirit), SUT_All },
	{STAT::ARMOR_RATING                , StatType::SHORT,     "short",  "armor_rating",  "Armor Rating",         "AR", DebuffType::STAT, msizeof(CharacterStatSet, armor_rating), offsetof(CharacterStatSet, armor_rating), SUT_None },

	{STAT::MELEE_ATTACK_SPEED          , StatType::SHORT,     "short",  "melee_attack_speed",   "Melee Attack Speed",   NULL, DebuffType::CONTROL, msizeof(CharacterStatSet, melee_attack_speed), offsetof(CharacterStatSet, melee_attack_speed), SUT_All },
	{STAT::MAGIC_ATTACK_SPEED          , StatType::SHORT,     "short",  "magic_attack_speed",   "Magic Attack Speed",   NULL, DebuffType::CONTROL, msizeof(CharacterStatSet, magic_attack_speed), offsetof(CharacterStatSet, magic_attack_speed), SUT_All },
	{STAT::BASE_DAMAGE_MELEE           , StatType::SHORT,     "short",  "base_damage_melee",    "Melee Damage",         NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_damage_melee), offsetof(CharacterStatSet, base_damage_melee), SUT_All },
	{STAT::BASE_DAMAGE_FIRE            , StatType::SHORT,     "short",  "base_damage_fire",     "Fire Damage",          NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_damage_fire), offsetof(CharacterStatSet, base_damage_fire), SUT_All },
	{STAT::BASE_DAMAGE_FROST           , StatType::SHORT,     "short",  "base_damage_frost",    "Frost Damage",         NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_damage_frost), offsetof(CharacterStatSet, base_damage_frost), SUT_All },
	{STAT::BASE_DAMAGE_MYSTIC          , StatType::SHORT,     "short",  "base_damage_mystic",   "Mystic Damage",        NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_damage_mystic), offsetof(CharacterStatSet, base_damage_mystic), SUT_All },
	{STAT::BASE_DAMAGE_DEATH           , StatType::SHORT,     "short",  "base_damage_death",    "Death Damage",         NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_damage_death), offsetof(CharacterStatSet, base_damage_death), SUT_All },
	{STAT::DAMAGE_RESIST_MELEE         , StatType::SHORT,     "short",  "damage_resist_melee",  "Melee Resistance",     NULL, DebuffType::STAT, msizeof(CharacterStatSet, damage_resist_melee), offsetof(CharacterStatSet, damage_resist_melee), SUT_All },
	{STAT::DAMAGE_RESIST_FIRE          , StatType::SHORT,     "short",  "damage_resist_fire",   "Fire Resistance",      NULL, DebuffType::STAT, msizeof(CharacterStatSet, damage_resist_fire), offsetof(CharacterStatSet, damage_resist_fire), SUT_All },
	{STAT::DAMAGE_RESIST_FROST         , StatType::SHORT,     "short",  "damage_resist_frost",  "Frost Resistance",     NULL, DebuffType::STAT, msizeof(CharacterStatSet, damage_resist_frost), offsetof(CharacterStatSet, damage_resist_frost), SUT_All },
	{STAT::DAMAGE_RESIST_MYSTIC        , StatType::SHORT,     "short",  "damage_resist_mystic", "Mystic Resistance",    NULL, DebuffType::STAT, msizeof(CharacterStatSet, damage_resist_mystic), offsetof(CharacterStatSet, damage_resist_mystic), SUT_All },
	{STAT::DAMAGE_RESIST_DEATH         , StatType::SHORT,     "short",  "damage_resist_death",  "Death Resistance",     NULL, DebuffType::STAT, msizeof(CharacterStatSet, damage_resist_death), offsetof(CharacterStatSet, damage_resist_death), SUT_All },
	{STAT::BASE_MOVEMENT               , StatType::SHORT,     "short",  "base_movement",        "Movement",             NULL, DebuffType::CONTROL, msizeof(CharacterStatSet, base_movement), offsetof(CharacterStatSet, base_movement), SUT_All },
	{STAT::BASE_LUCK                   , StatType::SHORT,     "short",  "base_luck",            "Luck",                 NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_luck), offsetof(CharacterStatSet, base_luck), SUT_All },
	{STAT::BASE_HEALTH                 , StatType::SHORT,     "short",  "base_health",          "Health",               NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_health), offsetof(CharacterStatSet, base_health), SUT_All },
	{STAT::WILL_MAX                    , StatType::SHORT,     "short",  "will_max",             "Willpower Max",        NULL, DebuffType::STAT, msizeof(CharacterStatSet, will_max), offsetof(CharacterStatSet, will_max), SUT_All },
	
	{STAT::WILL_REGEN                  , StatType::FLOAT,    "float",  "will_regen",           "Willpower Regen",      NULL, DebuffType::STAT, msizeof(CharacterStatSet, will_regen), offsetof(CharacterStatSet, will_regen), SUT_All },
	{STAT::MIGHT_MAX                   , StatType::SHORT,    "short",  "might_max",            "Might Max",            NULL, DebuffType::STAT, msizeof(CharacterStatSet, might_max), offsetof(CharacterStatSet, might_max), SUT_All },
	{STAT::MIGHT_REGEN                 , StatType::FLOAT,    "float",  "might_regen",          "Might Regen",          NULL, DebuffType::STAT, msizeof(CharacterStatSet, might_regen), offsetof(CharacterStatSet, might_regen), SUT_All },
	{STAT::BASE_DODGE                  , StatType::SHORT,    "short",  "base_dodge",           "Dodge",                NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_dodge), offsetof(CharacterStatSet, base_dodge), SUT_All },
	{STAT::BASE_DEFLECT                , StatType::SHORT,    "short",  "base_deflect",         "Deflect",              NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_deflect), offsetof(CharacterStatSet, base_deflect), SUT_All },
	{STAT::BASE_PARRY                  , StatType::SHORT,    "short",  "base_parry",           "Parry",                NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_parry), offsetof(CharacterStatSet, base_parry), SUT_All },
	{STAT::BASE_BLOCK                  , StatType::SHORT,    "short",  "base_block",           "Blocking",             NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_block), offsetof(CharacterStatSet, base_block), SUT_All },
	{STAT::BASE_MELEE_TO_HIT           , StatType::SHORT,    "short",  "base_melee_to_hit",    "Melee To-Hit",         NULL, 0, msizeof(CharacterStatSet, base_melee_to_hit), offsetof(CharacterStatSet, base_melee_to_hit), SUT_All },
	{STAT::BASE_MELEE_CRITICAL         , StatType::SHORT,    "short",  "base_melee_critical",  "Melee Critical",       NULL, 0, msizeof(CharacterStatSet, base_melee_critical), offsetof(CharacterStatSet, base_melee_critical), SUT_All },
	{STAT::BASE_MAGIC_SUCCESS          , StatType::SHORT,    "short",  "base_magic_success",   "Magic Success",        NULL, 0, msizeof(CharacterStatSet, base_magic_success), offsetof(CharacterStatSet, base_magic_success), SUT_All },
	{STAT::BASE_MAGIC_CRITICAL         , StatType::SHORT,    "short",  "base_magic_critical",  "Magic Critical",       NULL, 0, msizeof(CharacterStatSet, base_magic_critical), offsetof(CharacterStatSet, base_magic_critical), SUT_All },
	{STAT::OFFHAND_WEAPON_DAMAGE       , StatType::SHORT,    "short",  "offhand_weapon_damage", "Off-Hand Weapon Damage",    NULL, DebuffType::STAT, msizeof(CharacterStatSet, offhand_weapon_damage), offsetof(CharacterStatSet, offhand_weapon_damage), SUT_All },
	{STAT::CASTING_SETBACK_CHANCE      , StatType::SHORT,    "short",  "casting_setback_chance", "Casting Setback Chance",   NULL, 0, msizeof(CharacterStatSet, casting_setback_chance), offsetof(CharacterStatSet, casting_setback_chance), SUT_All },
	{STAT::CHANNELING_BREAK_CHANCE     , StatType::SHORT,    "short",  "channeling_break_chance", "Channeling Break Chance", NULL, 0, msizeof(CharacterStatSet, channeling_break_chance), offsetof(CharacterStatSet, channeling_break_chance), SUT_All },
	{STAT::BASE_HEALING                , StatType::SHORT,    "short",  "base_healing",            "Healing",           NULL, DebuffType::STAT, msizeof(CharacterStatSet, base_healing), offsetof(CharacterStatSet, base_healing), SUT_All },
	{STAT::AGGRO_RADIUS_MOD            , StatType::SHORT,    "short",  "aggro_radius_mod",        "Aggro Radius",      NULL, DebuffType::STAT, msizeof(CharacterStatSet, aggro_radius_mod), offsetof(CharacterStatSet, aggro_radius_mod), SUT_All },
	{STAT::HATE_GAIN_RATE              , StatType::SHORT,    "short",  "hate_gain_rate",          "Hatred Gain Rate",  NULL, 0, msizeof(CharacterStatSet, hate_gain_rate), offsetof(CharacterStatSet, hate_gain_rate), SUT_All },
	{STAT::EXPERIENCE_GAIN_RATE        , StatType::SHORT,    "short",  "experience_gain_rate",    "XP Gain Rate",      NULL, 0, msizeof(CharacterStatSet, experience_gain_rate), offsetof(CharacterStatSet, experience_gain_rate), SUT_All },
	{STAT::COIN_GAIN_RATE              , StatType::SHORT,    "short",  "coin_gain_rate",          "Coin Gain Rate",    NULL, 0, msizeof(CharacterStatSet, coin_gain_rate), offsetof(CharacterStatSet, coin_gain_rate), SUT_All },
	{STAT::MAGIC_LOOT_DROP_RATE        , StatType::SHORT,    "short",  "magic_loot_drop_rate",    "Magical Item Drops", NULL, 0, msizeof(CharacterStatSet, magic_loot_drop_rate), offsetof(CharacterStatSet, magic_loot_drop_rate), SUT_All },
	{STAT::INVENTORY_CAPACITY          , StatType::SHORT,    "short",  "inventory_capacity",      "Inventory Capacity", NULL, 0, msizeof(CharacterStatSet, inventory_capacity), offsetof(CharacterStatSet, inventory_capacity), SUT_None },
	{STAT::VIS_WEAPON                  , StatType::SHORT,    "short",  "vis_weapon",              "Visible Weapon",    NULL, 0, msizeof(CharacterStatSet, vis_weapon), offsetof(CharacterStatSet, vis_weapon), SUT_All },

	{STAT::SUB_NAME                    , StatType::CSTRING, "string",  "sub_name",                "Sub Name",          NULL, 0, msizeof(CharacterStatSet, sub_name), offsetof(CharacterStatSet, sub_name), SUT_All },
	{STAT::ALTERNATE_IDLE_ANIM         , StatType::CSTRING, "string",  "alternate_idle_anim",     "Idle Animation",    NULL, 0, msizeof(CharacterStatSet, alternate_idle_anim), offsetof(CharacterStatSet, alternate_idle_anim), SUT_None },
	{STAT::SELECTIVE_EQ_OVERRIDE       , StatType::CSTRING, "string",  "selective_eq_override",   "Selective Equipment Override", NULL, 0, msizeof(CharacterStatSet, selective_eq_override), offsetof(CharacterStatSet, selective_eq_override), SUT_All },

	// IMPORTANT: Health is a special case.  We're using the integer data type in the struct, but the
	// original client needs to interpret the value as a short, unless modified to support integers.
	// If the client is modified, use the UseIntegerHealth config property.
	{STAT::HEALTH                      , StatType::SHORT,   "short",  "health",        "Health",         "HP", DebuffType::STAT, msizeof(CharacterStatSet, health), offsetof(CharacterStatSet, health), SUT_All },
	{STAT::WILL                        , StatType::SHORT,   "short",  "will",          "Will",           "W",  DebuffType::STAT, msizeof(CharacterStatSet, will), offsetof(CharacterStatSet, will), SUT_All },
	{STAT::WILL_CHARGES                , StatType::SHORT,   "short",  "will_charges",  "Will Charges",  "WCh", 0, msizeof(CharacterStatSet, will_charges), offsetof(CharacterStatSet, will_charges), SUT_All },
	{STAT::MIGHT                       , StatType::SHORT,   "short",  "might",         "Might",           "M", DebuffType::STAT, msizeof(CharacterStatSet, might), offsetof(CharacterStatSet, might), SUT_All },
	{STAT::MIGHT_CHARGES               , StatType::SHORT,   "short",  "might_charges", "Might Charges", "MCh", 0, msizeof(CharacterStatSet, might_charges), offsetof(CharacterStatSet, might_charges), SUT_All },
	{STAT::SSIZE                       , StatType::SHORT,   "short",  "size",          "Size",           "sz", 0, msizeof(CharacterStatSet, size), offsetof(CharacterStatSet, size), SUT_All },
	{STAT::PROFESSION                  , StatType::SHORT,   "short",  "profession",    "Profession",     NULL, 0, msizeof(CharacterStatSet, profession), offsetof(CharacterStatSet, profession), SUT_All },

	{STAT::WEAPON_DAMAGE_2H            , StatType::SHORT,   "short",  "weapon_damage_2h",     "Weapon Damage: Two-Handed", NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_2h), offsetof(CharacterStatSet, weapon_damage_2h), SUT_All },
	{STAT::WEAPON_DAMAGE_1H            , StatType::SHORT,   "short",  "weapon_damage_1h",     "Weapon Damage: One-Handed", NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_1h), offsetof(CharacterStatSet, weapon_damage_1h), SUT_All },
	{STAT::WEAPON_DAMAGE_POLE          , StatType::SHORT,   "short",  "weapon_damage_pole",   "Weapon Damage: Pole",       NULL, 0, msizeof(CharacterStatSet, weapon_damage_pole), offsetof(CharacterStatSet, weapon_damage_pole), SUT_All },
	{STAT::WEAPON_DAMAGE_SMALL         , StatType::SHORT,   "short",  "weapon_damage_small",  "Weapon Damage: Small",      NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_small), offsetof(CharacterStatSet, weapon_damage_small), SUT_All },

	//Note: the files call it "weapon_damage_box", not bow.  Rolled with that for a while, but the new ability system requires stat
	//lookups by names, and they got that part correct, and I'm not continuing a typo for that.
	{STAT::WEAPON_DAMAGE_BOW           , StatType::SHORT,   "short",  "weapon_damage_bow",    "Weapon Damage: Bow",        NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_box), offsetof(CharacterStatSet, weapon_damage_box), SUT_All },
	{STAT::WEAPON_DAMAGE_THROWN        , StatType::SHORT,   "short",  "weapon_damage_thrown", "Weapon Damage: Thrown",     NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_thrown), offsetof(CharacterStatSet, weapon_damage_thrown), SUT_All },
	{STAT::WEAPON_DAMAGE_WAND          , StatType::SHORT,   "short",  "weapon_damage_wand",   "Weapon Damage: Wand",       NULL, DebuffType::STAT, msizeof(CharacterStatSet, weapon_damage_wand), offsetof(CharacterStatSet, weapon_damage_wand), SUT_All },
	{STAT::EXTRA_DAMAGE_FIRE           , StatType::SHORT,   "short",  "extra_damage_fire",    "Extra Damage: Fire",        NULL, DebuffType::STAT, msizeof(CharacterStatSet, extra_damage_fire), offsetof(CharacterStatSet, extra_damage_fire), SUT_All },
	{STAT::EXTRA_DAMAGE_FROST          , StatType::SHORT,   "short",  "extra_damage_frost",   "Extra Damage: Frost",       NULL, DebuffType::STAT, msizeof(CharacterStatSet, extra_damage_frost), offsetof(CharacterStatSet, extra_damage_frost), SUT_None },
	{STAT::EXTRA_DAMAGE_MYSTIC         , StatType::SHORT,   "short",  "extra_damage_mystic",  "Extra Damage: Mystic",      NULL, DebuffType::STAT, msizeof(CharacterStatSet, extra_damage_mystic), offsetof(CharacterStatSet, extra_damage_mystic), SUT_None },
	{STAT::EXTRA_DAMAGE_DEATH          , StatType::SHORT,   "short",  "extra_damage_death",   "Extra Damage: Death",       NULL, DebuffType::STAT, msizeof(CharacterStatSet, extra_damage_death), offsetof(CharacterStatSet, extra_damage_death), SUT_None },

	{STAT::APPEARANCE_OVERRIDE         , StatType::STRING, "string",  "appearance_override",     "Appearance Override",     NULL, 0, msizeof(CharacterStatSet, appearance_override), offsetof(CharacterStatSet, appearance_override), SUT_All },
	{STAT::INVISIBILITY_DISTANCE       , StatType::FLOAT,   "float",  "invisibility_distance",   "Invisibility Distance",   NULL, 0, msizeof(CharacterStatSet, invisibility_distance), offsetof(CharacterStatSet, invisibility_distance), SUT_None },
	{STAT::TRANSLOCATE_DESTINATION     , StatType::STRING, "string",  "translocate_destination", "Translocate Destination", NULL, 0, msizeof(CharacterStatSet, translocate_destination), offsetof(CharacterStatSet, translocate_destination), SUT_All },

	{STAT::LOOTABLE_PLAYER_IDS         , StatType::STRING, "string",  "lootable_player_ids",     "Lootable Player ID's",    NULL, 0, msizeof(CharacterStatSet, lootable_player_ids), offsetof(CharacterStatSet, lootable_player_ids), SUT_All },
	{STAT::MASTER                      , StatType::SHORT,    "short",  "master",                  "Master",                  NULL, 0, msizeof(CharacterStatSet, master), offsetof(CharacterStatSet, master), SUT_None },
	{STAT::REZ_PENDING                 , StatType::SHORT,    "short",  "rez_pending",             "Pending Resurrection Offer", NULL, 0, msizeof(CharacterStatSet, rez_pending), offsetof(CharacterStatSet, rez_pending), SUT_None },
	{STAT::LOOT_SEEABLE_PLAYER_IDS     , StatType::STRING, "string",  "loot_seeable_player_ids", "Loot Seeable Player ID's",   NULL, 0, msizeof(CharacterStatSet, loot_seeable_player_ids), offsetof(CharacterStatSet, loot_seeable_player_ids), SUT_All },
	{STAT::LOOT                        , StatType::CSTRING, "string",  "loot",            "Loot",	     NULL, 0, msizeof(CharacterStatSet, loot), offsetof(CharacterStatSet, loot), SUT_None },
	{STAT::BASE_STATS                  , StatType::CSTRING, "string",  "base_stats",      "Base Stats",  NULL, 0, msizeof(CharacterStatSet, base_stats), offsetof(CharacterStatSet, base_stats), SUT_All },
	{STAT::HEALTH_MOD                  , StatType::SHORT,    "short",  "health_mod",      "Health Mod",  NULL, 0, msizeof(CharacterStatSet, health_mod), offsetof(CharacterStatSet, health_mod), SUT_All },
	{STAT::HEROISM                     , StatType::SHORT,    "short",  "heroism",         "Heroism",     NULL, 0, msizeof(CharacterStatSet, heroism), offsetof(CharacterStatSet, heroism), SUT_All },
	{STAT::EXPERIENCE                  , StatType::INTEGER,    "int",  "experience",            "Experience Points",        "XP", 0, msizeof(CharacterStatSet, experience), offsetof(CharacterStatSet, experience), SUT_All },
	{STAT::TOTAL_ABILITY_POINTS        , StatType::SHORT,    "short",  "total_ability_points",   "Total Ability Points",   "TAP", 0, msizeof(CharacterStatSet, total_ability_points), offsetof(CharacterStatSet, total_ability_points), SUT_All },
	{STAT::CURRENT_ABILITY_POINTS      , StatType::SHORT,    "short",  "current_ability_points", "Current Ability Points", "CAP", 0, msizeof(CharacterStatSet, current_ability_points), offsetof(CharacterStatSet, current_ability_points), SUT_All },
	
	{STAT::COPPER                      , StatType::INTEGER,    "int",  "copper",          "Copper",  NULL, 0, msizeof(CharacterStatSet, copper), offsetof(CharacterStatSet, copper), SUT_All },
	{STAT::CREDITS                     , StatType::INTEGER,    "int",  "credits",         "Credits", NULL, 0, msizeof(CharacterStatSet, credits), offsetof(CharacterStatSet, credits), SUT_All },
	
	{STAT::AI_MODULE                   , StatType::CSTRING, "string",  "ai_module",           "AI Module",          NULL, 0, msizeof(CharacterStatSet, ai_module), offsetof(CharacterStatSet, ai_module), SUT_All },
	{STAT::CREATURE_CATEGORY           , StatType::CSTRING, "string",  "creature_category",   "Creature Category",  NULL, 0, msizeof(CharacterStatSet, creature_category), offsetof(CharacterStatSet, creature_category), SUT_All },
	{STAT::RARITY                      , StatType::SHORT,    "short",  "rarity",              "Rarity",             NULL, 0, msizeof(CharacterStatSet, rarity), offsetof(CharacterStatSet, rarity), SUT_All },
	{STAT::SPAWN_TABLE_POINTS          , StatType::SHORT,    "short",  "spawn_table_points",  "Spawn Table Points", NULL, 0, msizeof(CharacterStatSet, spawn_table_points), offsetof(CharacterStatSet, spawn_table_points), SUT_None },
	{STAT::AI_PACKAGE                  , StatType::CSTRING, "string",  "ai_package",          "AI Package",         NULL, 0, msizeof(CharacterStatSet, ai_package), offsetof(CharacterStatSet, ai_package), SUT_All },

	{STAT::BONUS_HEALTH                , StatType::SHORT,    "short",  "bonus_health",        "Bonus Health",       NULL, 0, msizeof(CharacterStatSet, bonus_health), offsetof(CharacterStatSet, bonus_health), SUT_All },

	{STAT::DR_MOD_MELEE                , StatType::INTEGER,    "int",  "dr_mod_melee",   "Melee Damage Reduction",  NULL, 0, msizeof(CharacterStatSet, dr_mod_melee), offsetof(CharacterStatSet, dr_mod_melee), SUT_All },
	{STAT::DR_MOD_FIRE                 , StatType::INTEGER,    "int",  "dr_mod_fire",    "Fire Damage Reduction",   NULL, 0, msizeof(CharacterStatSet, dr_mod_fire), offsetof(CharacterStatSet, dr_mod_fire), SUT_All },
	{STAT::DR_MOD_FROST                , StatType::INTEGER,    "int",  "dr_mod_frost",   "Frost Damage Reduction",  NULL, 0, msizeof(CharacterStatSet, dr_mod_frost), offsetof(CharacterStatSet, dr_mod_frost), SUT_All },
	{STAT::DR_MOD_MYSTIC               , StatType::INTEGER,    "int",  "dr_mod_mystic",  "Mystic Damage Reduction", NULL, 0, msizeof(CharacterStatSet, dr_mod_mystic), offsetof(CharacterStatSet, dr_mod_mystic), SUT_All },
	{STAT::DR_MOD_DEATH                , StatType::INTEGER,    "int",  "dr_mod_death",   "Death Damage Reduction",  NULL, 0, msizeof(CharacterStatSet, dr_mod_death), offsetof(CharacterStatSet, dr_mod_death), SUT_All },

	{STAT::DMG_MOD_MELEE               , StatType::INTEGER,    "int",  "dmg_mod_melee",  "Melee Damage Modifier",   NULL, 0, msizeof(CharacterStatSet, dmg_mod_melee), offsetof(CharacterStatSet, dmg_mod_melee), SUT_All },
	{STAT::DMG_MOD_FIRE                , StatType::INTEGER,    "int",  "dmg_mod_fire",   "Fire Damage Modifier",    NULL, 0, msizeof(CharacterStatSet, dmg_mod_fire), offsetof(CharacterStatSet, dmg_mod_fire), SUT_All },
	{STAT::DMG_MOD_FROST               , StatType::INTEGER,    "int",  "dmg_mod_frost",  "Frost Damage Modifier",   NULL, 0, msizeof(CharacterStatSet, dmg_mod_frost), offsetof(CharacterStatSet, dmg_mod_frost), SUT_All },
	{STAT::DMG_MOD_MYSTIC              , StatType::INTEGER,    "int",  "dmg_mod_mystic", "Mystic Damage Modifier",  NULL, 0, msizeof(CharacterStatSet, dmg_mod_mystic), offsetof(CharacterStatSet, dmg_mod_mystic), SUT_All },
	{STAT::DMG_MOD_DEATH               , StatType::INTEGER,    "int",  "dmg_mod_death",  "Death Damage Modifier",   NULL, 0, msizeof(CharacterStatSet, dmg_mod_death), offsetof(CharacterStatSet, dmg_mod_death), SUT_All },

	{STAT::PVP_TEAM                    , StatType::SHORT,    "short",  "pvp_team",  "PvP Team",  NULL, 0, msizeof(CharacterStatSet, pvp_team), offsetof(CharacterStatSet, pvp_team), SUT_None },
	
	{STAT::PVP_KILLS                   , StatType::INTEGER,    "int",  "pvp_kills", "PvP Kills", NULL, 0, msizeof(CharacterStatSet, pvp_kills), offsetof(CharacterStatSet, pvp_kills), SUT_None },
	{STAT::PVP_DEATHS                  , StatType::INTEGER,    "int",  "pvp_deaths", "PvP Deaths", NULL, 0, msizeof(CharacterStatSet, pvp_deaths), offsetof(CharacterStatSet, pvp_deaths), SUT_None },

	//Score doesn't exist in 8.8
	//STAT::PVP_SCORE                  ,  StatType::INTEGER,   "int",  "pvp_score", "PvP Score", NULL, 0, msizeof(CharacterStatSet, pvp_score), offsetof(CharacterStatSet, pvp_score), SUT_None,

	{STAT::PVP_STATE                   , StatType::CSTRING, "string",  "pvp_state", "PvP State", NULL, 0, msizeof(CharacterStatSet, pvp_state), offsetof(CharacterStatSet, pvp_state), SUT_All },

	{STAT::PVP_FLAG_CAPTURES           , StatType::INTEGER,"integer",  "pvp_flag_captures", "PvP Flag Captures", NULL, 0, msizeof(CharacterStatSet, pvp_flag_captures), offsetof(CharacterStatSet, pvp_flag_captures), SUT_None },
	
	{STAT::MOD_MELEE_TO_HIT            , StatType::FLOAT,    "float",  "mod_melee_to_hit",  "Mod Melee To Hit",  NULL, 0, msizeof(CharacterStatSet, mod_melee_to_hit), offsetof(CharacterStatSet, mod_melee_to_hit), SUT_None },
	{STAT::MOD_MELEE_TO_CRIT           , StatType::FLOAT,    "float",  "mod_melee_to_crit", "Mod Melee To Crit", NULL, 0, msizeof(CharacterStatSet, mod_melee_to_crit), offsetof(CharacterStatSet, mod_melee_to_crit), SUT_None },
	
	{STAT::MOD_MAGIC_TO_HIT            , StatType::FLOAT,    "float",  "mod_magic_to_hit",  "Mod Magic To Hit",  NULL, 0, msizeof(CharacterStatSet, mod_magic_to_hit), offsetof(CharacterStatSet, mod_magic_to_hit), SUT_None },
	{STAT::MOD_MAGIC_TO_CRIT           , StatType::FLOAT,    "float",  "mod_magic_to_crit", "Mod Magic To Crit", NULL, 0, msizeof(CharacterStatSet, mod_magic_to_crit), offsetof(CharacterStatSet, mod_magic_to_crit), SUT_None },

	{STAT::MOD_PARRY                   , StatType::FLOAT,    "float",  "mod_parry",         "Mod Parry",         NULL, 0, msizeof(CharacterStatSet, mod_parry), offsetof(CharacterStatSet, mod_parry), SUT_None },
	{STAT::MOD_BLOCK                   , StatType::FLOAT,    "float",  "mod_block",         "Mod Block",         NULL, 0, msizeof(CharacterStatSet, mod_block), offsetof(CharacterStatSet, mod_block), SUT_None },
	{STAT::MOD_MOVEMENT                , StatType::INTEGER,    "int",  "mod_movement",      "Mod Movement",      NULL, 0, msizeof(CharacterStatSet, mod_movement), offsetof(CharacterStatSet, mod_movement), SUT_All },
	{STAT::MOD_HEALTH_REGEN            , StatType::FLOAT,    "float",  "mod_health_regen",  "Mod Health Regen",  NULL, 0, msizeof(CharacterStatSet, mod_health_regen), offsetof(CharacterStatSet, mod_health_regen), SUT_None },
	{STAT::MOD_ATTACK_SPEED            , StatType::FLOAT,    "float",  "mod_attack_speed",  "Mod Attack Speed",  NULL, 0, msizeof(CharacterStatSet, mod_attack_speed), offsetof(CharacterStatSet, mod_attack_speed), SUT_None },
	{STAT::MOD_CASTING_SPEED           , StatType::FLOAT,    "float",  "mod_casting_speed", "Mod Casting Speed", NULL, 0, msizeof(CharacterStatSet, mod_casting_speed), offsetof(CharacterStatSet, mod_casting_speed), SUT_All },
	{STAT::MOD_HEALING                 , StatType::FLOAT,    "float",  "mod_healing",       "Mod Healing",       NULL, 0, msizeof(CharacterStatSet, mod_healing), offsetof(CharacterStatSet, mod_healing), SUT_None },
	{STAT::TOTAL_SIZE                  , StatType::FLOAT,    "float",  "total_size",        "Total Size",        NULL, 0, msizeof(CharacterStatSet, total_size), offsetof(CharacterStatSet, total_size), SUT_All },
	{STAT::AGGRO_PLAYERS               , StatType::SHORT,    "short",  "aggro_players",     "Aggro Players",     NULL, 0, msizeof(CharacterStatSet, aggro_players), offsetof(CharacterStatSet, aggro_players), SUT_NonPlayer },

	{STAT::MOD_LUCK                    , StatType::FLOAT,    "float",  "mod_luck",          "Mod Luck",          NULL, 0, msizeof(CharacterStatSet, mod_luck), offsetof(CharacterStatSet, mod_luck), SUT_None },
	{STAT::HEALTH_REGEN                , StatType::SHORT,    "short",  "health_regen",      "Health Regen",      NULL, 0, msizeof(CharacterStatSet, health_regen), offsetof(CharacterStatSet, health_regen), SUT_None },
	{STAT::BLEEDING                    , StatType::SHORT,    "short",  "bleeding",          "Bleeding",          NULL, 0, msizeof(CharacterStatSet, bleeding), offsetof(CharacterStatSet, bleeding), SUT_None },
	{STAT::DAMAGE_SHIELD               , StatType::INTEGER,    "int",    "damage_shield",     "Damage Shield",     NULL, 0, msizeof(CharacterStatSet, damage_shield), offsetof(CharacterStatSet, damage_shield), SUT_None },
	{STAT::HIDE_NAMEBOARD              , StatType::SHORT,    "short",  "hide_nameboard",    "Hide Nameboard",    NULL, 0, msizeof(CharacterStatSet, hide_nameboard), offsetof(CharacterStatSet, hide_nameboard), SUT_None },

	//Note: Does not exist in 0.8.6.  Does exist in 0.8.8.
	{STAT::HIDE_MINIMAP                , StatType::SHORT,    "short",  "hide_minimap",      "Hide Minimap",      NULL, 0, msizeof(CharacterStatSet, hide_minimap), offsetof(CharacterStatSet, hide_minimap), SUT_CDefExt },

	//ICEEE
	{STAT::CREDIT_DROPS                , StatType::SHORT,    "short",  "credit_drops",          "Credit Drops",  NULL, 0, msizeof(CharacterStatSet, credit_drops), offsetof(CharacterStatSet, credit_drops), SUT_All }
};
const int MaxStatList = sizeof(StatList) / sizeof(StatList[0]);

StatusEffectBitInfo StatusEffectBitData[] = {
	{StatusEffects::DEAD,                  0, (1 << 0),  "DEAD" },
	{StatusEffects::SILENCE,               0, (1 << 1),  "SILENCE" },
	{StatusEffects::DISARM,                0, (1 << 2),  "DISARM" },
	{StatusEffects::STUN,                  0, (1 << 3),  "STUN" },

	{StatusEffects::DAZE,                  0, (1 << 4),  "DAZE" },
	{StatusEffects::CHARM,                 0, (1 << 5),  "CHARM" },
	{StatusEffects::FEAR,                  0, (1 << 6),  "FEAR" },
	{StatusEffects::ROOT,                  0, (1 << 7),  "ROOT" },
	
	{StatusEffects::LOCKED,                0, (1 << 8),  "LOCKED" },
	{StatusEffects::BROKEN,                0, (1 << 9),  "BROKEN" },
	{StatusEffects::CAN_USE_WEAPON_2H,     0, (1 << 10), "CAN_USE_WEAPON_2H" },
	{StatusEffects::CAN_USE_WEAPON_1H,     0, (1 << 11), "CAN_USE_WEAPON_1H" },

	{StatusEffects::CAN_USE_WEAPON_SMALL,  0, (1 << 12), "CAN_USE_WEAPON_SMALL" },
	{StatusEffects::CAN_USE_WEAPON_POLE,   0, (1 << 13), "CAN_USE_WEAPON_POLE" },
	{StatusEffects::CAN_USE_WEAPON_BOW,    0, (1 << 14), "CAN_USE_WEAPON_BOW" },
	{StatusEffects::CAN_USE_WEAPON_THROWN, 0, (1 << 15), "CAN_USE_WEAPON_THROWN" },

	{StatusEffects::CAN_USE_WEAPON_WAND,   0, (1 << 16), "CAN_USE_WEAPON_WAND" },
	{StatusEffects::CAN_USE_DUAL_WIELD,    0, (1 << 17), "CAN_USE_DUAL_WIELD" },
	{StatusEffects::CAN_USE_PARRY,         0, (1 << 18), "CAN_USE_PARRY" },
	{StatusEffects::CAN_USE_BLOCK,         0, (1 << 19), "CAN_USE_BLOCK" },

	{StatusEffects::CAN_USE_ARMOR_CLOTH,   0, (1 << 20), "CAN_USE_ARMOR_CLOTH" },
	{StatusEffects::CAN_USE_ARMOR_LIGHT,   0, (1 << 21), "CAN_USE_ARMOR_LIGHT" },
	{StatusEffects::CAN_USE_ARMOR_MEDIUM,  0, (1 << 22), "CAN_USE_ARMOR_MEDIUM" },
	{StatusEffects::CAN_USE_ARMOR_HEAVY,   0, (1 << 23), "CAN_USE_ARMOR_HEAVY" },

	{StatusEffects::INVISIBLE,             0, (1 << 24), "INVISIBLE" },
	{StatusEffects::ALL_SEEING,            0, (1 << 25), "ALL_SEEING" },
	{StatusEffects::IN_COMBAT_STAND,       0, (1 << 26), "IN_COMBAT_STAND" },
	{StatusEffects::INVINCIBLE,            0, (1 << 27), "INVINCIBLE" },

	{StatusEffects::FLEE,                  0, (1 << 28), "FLEE" },
	{StatusEffects::NO_AGGRO_GAINED,       0, (1 << 29), "NO_AGGRO_GAINED" },
	{StatusEffects::UNATTACKABLE,          0, (1 << 30), "UNATTACKABLE" },
	{StatusEffects::IS_USABLE,             0, (1 << 31), "IS_USABLE" },

	// New array index
	{StatusEffects::CLIENT_LOADING,        1, (1 << 0), "CLIENT_LOADING" },
	{StatusEffects::IMMUNE_INTERRUPT,      1, (1 << 1), "IMMUNE_INTERRUPT" },
	{StatusEffects::IMMUNE_SILENCE,        1, (1 << 2), "IMMUNE_SILENCE" },
	{StatusEffects::IMMUNE_DISARM,         1, (1 << 3), "IMMUNE_DISARM" },

	{StatusEffects::IMMUNE_BLEED,          1, (1 << 4), "IMMUNE_BLEED" },
	{StatusEffects::IMMUNE_DAMAGE_FIRE,    1, (1 << 5), "IMMUNE_DAMAGE_FIRE" },
	{StatusEffects::IMMUNE_DAMAGE_FROST,   1, (1 << 6), "IMMUNE_DAMAGE_FROST" },
	{StatusEffects::IMMUNE_DAMAGE_MYSTIC,  1, (1 << 7), "IMMUNE_DAMAGE_MYSTIC" },

	{StatusEffects::IMMUNE_DAMAGE_DEATH,   1, (1 << 8), "IMMUNE_DAMAGE_DEATH" },
	{StatusEffects::IMMUNE_DAMAGE_MELEE,   1, (1 << 9), "IMMUNE_DAMAGE_MELEE" },
	{StatusEffects::DISABLED,              1, (1 << 10), "DISABLED" },
	{StatusEffects::AUTO_ATTACK,           1, (1 << 11), "AUTO_ATTACK" },

	{StatusEffects::AUTO_ATTACK_RANGED,    1, (1 << 12), "AUTO_ATTACK_RANGED" },
	{StatusEffects::CARRYING_RED_FLAG,     1, (1 << 13), "CARRYING_RED_FLAG" },
	{StatusEffects::CARRYING_BLUE_FLAG,    1, (1 << 14), "CARRYING_BLUE_FLAG" },
	{StatusEffects::HENGE,                 1, (1 << 15), "HENGE" },

	{StatusEffects::TRANSFORMER,           1, (1 << 16), "TRANSFORMER" },
	{StatusEffects::PVPABLE,               1, (1 << 17), "PVPABLE" },
	{StatusEffects::IN_COMBAT,             1, (1 << 18), "IN_COMBAT" },
	{StatusEffects::WALK_IN_SHADOWS,       1, (1 << 19), "WALK_IN_SHADOWS" },

	{StatusEffects::EVADE,                 1, (1 << 20), "EVADE" },
	{StatusEffects::TAUNTED,               1, (1 << 21), "TAUNTED" },
	{StatusEffects::XP_BOOST,              1, (1 << 22), "XP_BOOST" },
	{StatusEffects::REAGENT_GENERATOR,     1, (1 << 23), "REAGENT_GENERATOR" },

	{StatusEffects::RES_PENALTY,           1, (1 << 24), "RES_PENALTY" },
	{StatusEffects::IMMUNE_STUN,           1, (1 << 25), "IMMUNE_STUN" },
	{StatusEffects::IMMUNE_DAZE,           1, (1 << 26), "IMMUNE_DAZE" },
	{StatusEffects::GM_FROZEN,             1, (1 << 27), "GM_FROZEN" },

	{StatusEffects::GM_INVISIBLE,          1, (1 << 28), "GM_INVISIBLE" },
	{StatusEffects::GM_SILENCED,           1, (1 << 29), "GM_SILENCED" },

	{StatusEffects::UNKILLABLE,            1, (1 << 30), "UNKILLABLE" },
	{StatusEffects::TRANSFORMED,           1, (1 << 31), "TRANSFORMED" },

	// New array index
	{StatusEffects::INVISIBLE_EQUIPMENT,   2, (1 << 0), "INVISIBLE_EQUIPMENT" },
};
const int MAX_STATUSEFFECT = sizeof(StatusEffectBitData) / sizeof(StatusEffectBitInfo);

//const int MAX_LEVEL = 70;


//As characters progress in levels, their base stats will rise in a certain way.
//Each of the base stats will be high, medium, or low.  For example, knights have high
//base strength but low base psyche.  Mages have high psyche but low strength.
short LevelBaseStats[71][3] = {
	{0,    0,   0},   // Level 0
	{10,   8,   7},   //  1
	{11,   8,   7},   //  2
	{12,   9,   8},   //  3
	{13,  10,   9},   //  4
	{14,  11,  10},   //  5
	{15,  12,  10},   //  6
	{16,  13,  11},   //  7
	{17,  14,  12},   //  8
	{18,  15,  12},   //  9
	{19,  16,  13},   // 10
	{20,  17,  14},   // 11
	{21,  18,  14},   // 12
	{22,  18,  15},   // 13
	{23,  19,  16},   // 14
	{24,  19,  16},   // 15
	{25,  20,  17},   // 16
	{26,  21,  18},   // 17
	{27,  22,  18},   // 18
	{28,  23,  19},   // 19
	{29,  24,  20},   // 20
	{30,  25,  20},   // 21
	{31,  26,  21},   // 22
	{32,  27,  22},   // 23
	{33,  28,  22},   // 24
	{34,  29,  23},   // 25
	{35,  30,  24},   // 26
	{36,  31,  24},   // 27
	{37,  32,  25},   // 28
	{38,  33,  26},   // 29
	{40,  34,  28},   // 30
	{42,  36,  28},   // 31
	{44,  38,  30},   // 32
	{46,  39,  32},   // 33
	{48,  41,  32},   // 34
	{50,  43,  34},   // 35
	{52,  45,  36},   // 36
	{54,  46,  36},   // 37
	{56,  48,  38},   // 38
	{58,  50,  40},   // 39
	{60,  51,  42},   // 40
	{62,  53,  42},   // 41
	{64,  55,  44},   // 42
	{66,  56,  46},   // 43
	{68,  58,  46},   // 44
	{70,  60,  48},   // 45
	{72,  62,  50},   // 46
	{74,  63,  50},   // 47
	{76,  65,  52},   // 48
	{78,  67,  54},   // 49
	{80,  68,  56},   // 50
	{82,  70,  56},   // 51
	{84,  72,  58},   // 52
	{86,  73,  60},   // 53
	{88,  75,  60},   // 54
	{90,  77,  62},   // 55
	{92,  79,  64},   // 56
	{94,  80,  64},   // 57
	{96,  82,  66},   // 58
	{98,  84,  68},   // 59
	{100, 85,  70},   // 60
	{102, 87,  70},   // 61
	{104, 89,  72},   // 62
	{106, 90,  74},   // 63
	{108, 92,  74},   // 64
	{110, 94,  76},   // 65
	{112, 96,  78},   // 66
	{114, 97,  78},   // 67
	{116, 99,  80},   // 68
	{118, 101, 82},   // 69
	{120, 102, 84},   // 70
};

//Determines which column (high, med, or low) to select from the above table when selecting
//the base amount of each stat of each class.
short ProfBaseStats[6][5] = {
	//Str           Dex             Con             Psy             Spi
	{BaseStat::Med,  BaseStat::Med,  BaseStat::Med,  BaseStat::Med,  BaseStat::Med},  // 0 None
	{BaseStat::High, BaseStat::Med,  BaseStat::High, BaseStat::Low,  BaseStat::Low},  // 1 Knight
	{BaseStat::High, BaseStat::High, BaseStat::Med,  BaseStat::Low,  BaseStat::Low},  // 2 Rogue
	{BaseStat::Low,  BaseStat::Med,  BaseStat::Low,  BaseStat::High, BaseStat::High}, // 3 Mage
	{BaseStat::Low,  BaseStat::Low,  BaseStat::High, BaseStat::Med,  BaseStat::High}, // 4 Druid
	{BaseStat::Med,  BaseStat::Med,  BaseStat::Med,  BaseStat::Med,  BaseStat::Med},  // 5 Monster
};

CharacterStatSet :: CharacterStatSet()
{
	size = 1.0F;
	damage_resist_frost = 0;
	damage_resist_fire = 0;
	damage_resist_death = 0;
	damage_resist_mystic = 0;
	damage_resist_melee = 0;
	will_regen = 1.0F;
	might_regen = 1.0F;
	mod_casting_speed = 1.4013e-045F;
}
void CharacterStatSet :: CopyFrom(CharacterStatSet *source)
{
	if(source == this)
		return;
	
	for(size_t i = 0; i < MaxStatList; i++)
	{
		char *dstData = (char*)this + StatList[i].offset;
		char *srcData = (char*)source + StatList[i].offset;
		switch(StatList[i].etype)
		{
		case StatType::SHORT:   *(short*)dstData = *(short*)srcData; break;
		case StatType::INTEGER: *(int*)dstData = *(int*)srcData; break;
		case StatType::FLOAT:   *(float*)dstData = *(float*)srcData; break;
		case StatType::CSTRING: memcpy(dstData, srcData, StatList[i].size); break;
		case StatType::STRING:  ((std::string*)dstData)->assign(*(std::string*)srcData); break;
		}
	}
}
void CharacterStatSet :: Clear(void)
{
	for(size_t i = 0; i < MaxStatList; i++)
	{
		char *data = (char*)this + StatList[i].offset;
		switch(StatList[i].etype)
		{
		case StatType::SHORT:   *(short*)data = 0; break;
		case StatType::INTEGER: *(int*)data = 0; break;
		case StatType::FLOAT:   *(float*)data = 0.0F; break;
		case StatType::CSTRING: memset(data, 0, StatList[i].size); break;
		case StatType::STRING:  ((std::string*)data)->clear(); break;
		}
	}
}

void CharacterStatSet :: SetAppearance(const char *data)
{
	appearance = data;
}
void CharacterStatSet :: SetEqAppearance(const char *data)
{
	eq_appearance = data;
}

void CharacterStatSet :: SetAIPackage(const char *data)
{
	Util::SafeCopy(ai_package, data, sizeof(ai_package));
}

void CharacterStatSet :: SetDisplayName(const char *name)
{
	Util::SafeCopy(display_name, name, sizeof(display_name));
}

void CharacterStatSet :: SetSubName(const char *name)
{
	Util::SafeCopy(sub_name, name, sizeof(sub_name));
}

bool CharacterStatSet :: IsPropAppearance(void)
{
	return (appearance.find("p1:") != string::npos);
}

void CharacterStatSet :: ClearLootSeeablePlayerIDs(void)
{
	loot_seeable_player_ids.clear();
}

void CharacterStatSet :: ClearLootablePlayerIDs(void)
{
	lootable_player_ids.clear();
}

int GetStatIndex(short StatID)
{
	//Returns the index of the internal stat list
	int a;
	for(a = 0; a < NumStats; a++)
		if(StatID == StatList[a].ID)
			return a;
	
	return -1;
}

int GetStatIndexByName(const char *name)
{
	//Returns the index of the internal stat list by searching for a stat name.
	//The name must be converted to match case before it is passed to this function.
	for(int i = 0; i < NumStats; i++)
		if(strcmp(StatList[i].name, name) == 0)
			return i;
	
	return -1;
}

StatDefinition* GetStatDefByName(const char *name)
{
	int r = GetStatIndexByName(name);
	if(r >= 0)
		return &StatList[r];

	return NULL;
}

int GetStatusIDByName(const char *name)
{
	//Returns the index of the internal status effect list by searching for a status
	//effect name.  The name must be converted to match case before it is passed to
	//this function.
	int a;
	for(a = 0; a < MAX_STATUSEFFECT; a++)
		if(strcmp(StatusEffectBitData[a].name, name) == 0)
			return StatusEffectBitData[a].effectID;
	
	return -1;
}

const char* GetStatusNameByID(int id)
{
	static const char *unk = "UNKNOWN";
	for(int i = 0; i < MAX_STATUSEFFECT; i++)
		if(StatusEffectBitData[i].effectID == id)
			return StatusEffectBitData[i].name;

	return unk;
}

int WriteCurrentStatToBuffer(char *buffer, short StatID, CharacterStatSet *css)
{
	//Writes the current value of the given stat to the output buffer.
	//This is to aid in the preparation of stat updates which are based on
	//arbitrary Stat IDs.
	int r = GetStatIndex(StatID);
	if(r == -1)
	{
		g_Log.AddMessageFormat("[ERROR] StatID not found: %d", StatID);
		return 0;
	}

	char *base = (char*)css + StatList[r].offset;
	switch(StatList[r].etype)
	{
	case StatType::SHORT:     return PutShort(buffer, *(short*)base);
	case StatType::INTEGER:   return PutInteger(buffer, *(int*)base);
	case StatType::FLOAT:     return PutFloat(buffer, *(float*)base);
	case StatType::CSTRING:   return PutStringUTF(buffer, base);
	case StatType::STRING:    return PutStringUTF(buffer, ((std::string*)base)->c_str());
	default:
		g_Log.AddMessageFormat("[ERROR] Unhandled stat type [%s] for StatID [%d]", StatList[r].type, StatID);
	}
	return 0;
}

int WriteStatToBuffer(char *buffer, short StatID, float value)
{
	int r = GetStatIndex(StatID);
	if(r == -1)
	{
		g_Log.AddMessageFormat("[ERROR] WriteStatToBuffer() Invalid StatID: %d", StatID);
		return 0;
	}

	switch(StatList[r].etype)
	{
	case StatType::SHORT:
		{
		short wval = (short)value;
		return PutShort(buffer, (short)wval);
		}
	case StatType::INTEGER:
		{
		int ival = (int)value;
		return PutInteger(buffer, ival);
		}
	case StatType::FLOAT:
		return PutFloat(buffer, value);
	case StatType::CSTRING:
		return PutStringUTF(buffer, "");
	case StatType::STRING:
		return PutStringUTF(buffer, "");
	default:
		g_Log.AddMessageFormat("[ERROR] Unhandled stat type [%s] for StatID [%d]", StatList[r].type, StatID);
	}
	return 0;
}

int WriteStatToSet(int StatIndex, const char *value, CharacterStatSet *css)
{
	int r = StatIndex;
	if(r == -1)
		return -1;

	char *base = (char*)css + StatList[r].offset;
	switch(StatList[r].etype)
	{
	case StatType::SHORT:
		{
		short val = atoi(value);
		memcpy(base, &val, sizeof(short));
		return sizeof(short);
		}
	case StatType::INTEGER:
		{
		int val = atoi(value);
		memcpy(base, &val, sizeof(int));
		return sizeof(int);
		}
	case StatType::FLOAT:
		{
		float val = (float)atof(value);
		memcpy(base, &val, sizeof(float));
		return sizeof(float);
		}
	case StatType::CSTRING:
		{
		int len = strlen(value);
		if(len > StatList[r].size - 1)
		{
			g_Log.AddMessageFormat("Warning: text is too long to fit variable size of %d (%s)", len, value);
			len = StatList[r].size - 1;
		}
		strncpy(base, value, len);
		base[len] = 0;
		return len;
		}
	case StatType::STRING:
		((std::string*)base)->assign(value);
		return ((std::string*)base)->size();
		break;
	}
	return -1;
}

int WriteStatToSetByName(const char *name, const char *value, CharacterStatSet *css)
{
	return WriteStatToSet(GetStatIndexByName(name), value, css);
}

int WriteStatToFile(int StatIndex, CharacterStatSet *css, FILE *file)
{
	int r = StatIndex;
	if(r == -1)
		return -1;

	char *base = (char*)css + StatList[r].offset;

	switch(StatList[r].etype)
	{
	case StatType::SHORT:
		{
		short val = 0;
		memcpy(&val, base, sizeof(short));
		return fprintf(file, "%s=%d\r\n", StatList[r].name, val);
		}
	case StatType::INTEGER:
		{
		int val = 0;
		memcpy(&val, base, sizeof(int));
		return fprintf(file, "%s=%d\r\n", StatList[r].name, val);
		}
	case StatType::FLOAT:
		{
		float val = 0.0F;
		memcpy(&val, base, sizeof(float));
		return fprintf(file, "%s=%g\r\n", StatList[r].name, val);
		}
	case StatType::CSTRING:
		return fprintf(file, "%s=%s\r\n", StatList[r].name, base);
	case StatType::STRING:
		return fprintf(file, "%s=%s\r\n", StatList[r].name, ((std::string*)base)->c_str());
	}

	return -1;
}

bool isStatZero(int StatIndex, CharacterStatSet *css)
{
	if(StatIndex == -1)
		return true;

	char *base = (char*)css + StatList[StatIndex].offset;

	switch(StatList[StatIndex].etype)
	{
	case StatType::SHORT:
		if(*(short*)base == 0)
			return true;
		break;
	case StatType::INTEGER:
		if(*(int*)base == 0)
			return true;
		break;
	case StatType::FLOAT:
		if(*(float*)base == 0.0F)
			return true;
		break;
	case StatType::CSTRING:
		if(base[0] == 0)
			return true;
		break;
	case StatType::STRING:
		if(((std::string*)base)->length() == 0)
			return true;
		break;
	}
	return false;
}

bool isStatEqual(int StatIndex, CharacterStatSet *css1, CharacterStatSet *css2)
{
	if(StatIndex == -1)
		return false;

	char *base1 = (char*)css1 + StatList[StatIndex].offset;
	char *base2 = (char*)css2 + StatList[StatIndex].offset;
	switch(StatList[StatIndex].etype)
	{
	case StatType::SHORT:
		if(*(short*)base1 == *(short*)base2)
			return true;
		break;
	case StatType::INTEGER:
		if(*(int*)base1 == *(int*)base2)
			return true;
		break;
	case StatType::FLOAT:
		if(*(float*)base1 == *(float*)base2)
			return true;
		break;
	case StatType::CSTRING:
		if(strcmp(base1, base2) == 0)
			return true;
		break;
	case StatType::STRING:
		if(((std::string*)base1)->compare(*(std::string*)base2))
			return true;
		break;
	}
	return false;
}

const char * GetStatValueAsString(int StatIndex, char *ConvBuf, CharacterStatSet *css)
{
	if(StatIndex == -1)
		return NULL;

	char *base = (char*)css + StatList[StatIndex].offset;
	switch(StatList[StatIndex].etype)
	{
	case StatType::SHORT:     sprintf(ConvBuf, "%d", *(short*)base); break;
	case StatType::INTEGER:   sprintf(ConvBuf, "%d", *(int*)base); break;
	case StatType::FLOAT:     sprintf(ConvBuf, "%g", *(float*)base); break;
	case StatType::CSTRING:   return base;
	case StatType::STRING:    return ((std::string*)base)->c_str();
	}
	return ConvBuf;
}

int WriteStatToFileByName(char *name, CharacterStatSet *css, FILE *file)
{
	return WriteStatToFile(GetStatIndexByName(name), css, file);
}

float GetStatValueByID(int statID, CharacterStatSet *css)
{
	//Look up the given statID, returning an integer with its current value.

	int StatIndex = GetStatIndex(statID);
	if(StatIndex == -1)
	{
		g_Log.AddMessageFormatW(MSG_WARN, "Stat ID not found: %d", statID);
		return 0;
	}

	char *base = (char*)css + StatList[StatIndex].offset;
	switch(StatList[StatIndex].etype)
	{
	case StatType::SHORT:   return (float)*(short*)base;
	case StatType::INTEGER: return (float)*(int*)base;
	case StatType::FLOAT:   return *(float*)base;
	case StatType::CSTRING: return (float)strtod(base, NULL);
	case StatType::STRING:  return (float)strtod(((std::string*)base)->c_str(), NULL);
	}
	return 0.0F;
}

int WriteValueToStat(int StatID, float value, CharacterStatSet *css)
{
	int r = GetStatIndex(StatID);
	if(r == -1) {
		return -1;
	}

	char *base = (char*)css + StatList[r].offset;
	switch(StatList[r].etype)
	{
	case StatType::SHORT:   *(short*)base = (short)value; break;
	case StatType::INTEGER: *(int*)base = (int)value; break;
	case StatType::FLOAT:   *(float*)base = value; break;
	case StatType::CSTRING: sprintf(base, "%d", (int)value); break;
	case StatType::STRING: 
		{
		char buffer[32];
		sprintf(buffer, "%d", (int)value);
		((std::string*)base)->assign(buffer);
		}
		break;
	}
	return -1;
}

char StatManager :: GetStatType(int statID)
{
	int index = GetStatIndex(statID);
	if(index == -1)
		return StatType::INTEGER;
	
	return StatList[index].etype;
}


// Optional hack to force health values as integers instead of the client default of shorts.
void StatManager :: SetHealthToInteger(bool setting)
{
	int r = GetStatIndex(STAT::HEALTH);
	if(r == -1)
		return;

	if(setting == true)
	{
		StatList[r].etype = StatType::INTEGER;
		StatList[r].type = "int";
	}
	else
	{
		StatList[r].etype = StatType::SHORT;
		StatList[r].type = "short";
	}
}

namespace StatInfo
{

struct StatInfoDisplay
{
	int StatID;
	const char *PrettyName;
	int Type;
	
	enum Types
	{
		INT,
		FLOAT,
		PERCENT,
		PLUSPERCENT,
		INTPERCENT,
		PLUSINTP,
		PLUSINT,
		FLOATMULT
	};
};

StatInfoDisplay statInfoDisplay[] = {
	{  STAT::DAMAGE_RESIST_MELEE,   "Physical Defense",  StatInfoDisplay::INT  },
	{  STAT::DAMAGE_RESIST_FIRE,    "Fire Defense",   StatInfoDisplay::INT  },
	{  STAT::DAMAGE_RESIST_FROST,   "Frost Defense",  StatInfoDisplay::INT  },
	{  STAT::DAMAGE_RESIST_MYSTIC,  "Mystic Defense", StatInfoDisplay::INT  },
	{  STAT::DAMAGE_RESIST_DEATH,   "Death Defense",  StatInfoDisplay::INT  },
	{  STAT::MOD_MELEE_TO_CRIT,   "Physical Critical Chance",  StatInfoDisplay::PLUSINTP  },
	{  STAT::MOD_MAGIC_TO_CRIT,   "Magic Critical Chance",     StatInfoDisplay::PLUSINTP  },
	{  STAT::MELEE_ATTACK_SPEED,   "Autoattack Speed",  StatInfoDisplay::PLUSINTP  },
	{  STAT::MAGIC_ATTACK_SPEED,   "Cast Speed",        StatInfoDisplay::PLUSINTP  },
	{  STAT::BASE_BLOCK,   "Block Chance",        StatInfoDisplay::PLUSINTP  },
	{  STAT::BASE_PARRY,   "Parry Chance",        StatInfoDisplay::PLUSINTP  },
	{  STAT::BASE_DODGE,   "Dodge Chance",        StatInfoDisplay::PLUSINTP  },
	{  STAT::MOD_MOVEMENT,   "Movement Speed",        StatInfoDisplay::PLUSPERCENT  },
	{  STAT::EXPERIENCE_GAIN_RATE,   "Experience Gain",        StatInfoDisplay::PLUSPERCENT  },
	{  STAT::DMG_MOD_FIRE,   "Fire Specialization",        StatInfoDisplay::PLUSINTP  },
	{  STAT::DMG_MOD_FROST,   "Frost Specialization",        StatInfoDisplay::PLUSINTP  },
	{  STAT::DMG_MOD_MYSTIC,   "Mystic Specialization",        StatInfoDisplay::PLUSINTP  },
	{  STAT::DMG_MOD_DEATH,   "Death Specialization",        StatInfoDisplay::PLUSINTP  },
	{  STAT::BASE_HEALING,   "Healing Specialization",        StatInfoDisplay::PLUSINTP  },
	{  STAT::OFFHAND_WEAPON_DAMAGE,   "Offhand Weapon Damage",   StatInfoDisplay::INTPERCENT  },
	{  STAT::CASTING_SETBACK_CHANCE,   "Casting Setback Chance",   StatInfoDisplay::INTPERCENT  },
	{  STAT::CHANNELING_BREAK_CHANCE,   "Channel Break Chance",    StatInfoDisplay::INTPERCENT  },
	{  STAT::MOD_HEALTH_REGEN,   "Bonus Health Regeneration",    StatInfoDisplay::PLUSINT  },
	{  STAT::BASE_HEALTH,   "Bonus Health",    StatInfoDisplay::INT  },
	{  STAT::BONUS_HEALTH,   "Damage Shield Hitpoints",    StatInfoDisplay::INT  },
	{  STAT::DAMAGE_SHIELD,   "Damage Reflect",    StatInfoDisplay::INT  },
	{  STAT::MIGHT_REGEN,   "Might Regeneration Speed",    StatInfoDisplay::FLOATMULT  },
	{  STAT::WILL_REGEN,   "Will Regeneration Speed",    StatInfoDisplay::FLOATMULT  },
	{  STAT::MOD_LUCK,   "Luck Multiplier",    StatInfoDisplay::PLUSINT  }
};
const int numStatInfoDisplay = sizeof(statInfoDisplay) / sizeof(statInfoDisplay[0]);

void GeneratePrettyStatTable(MULTISTRING &output, CharacterStatSet *css)
{
	char buffer[256];
	for(int i = 0; i < numStatInfoDisplay; i++)
	{
		int statIndex = GetStatIndex(statInfoDisplay[i].StatID);
		if(statIndex == -1)
			continue;

		const char *value = GetStatValueAsString(statIndex, buffer, css);
		float fvalue = static_cast<float>(atof(value));
		if(fvalue == 0.0F)
			continue;

		buffer[0] = 0;
		switch(statInfoDisplay[i].Type)
		{
		case StatInfoDisplay::INT: sprintf(buffer, "%g", fvalue); break;
		case StatInfoDisplay::FLOAT: sprintf(buffer, "%g", fvalue); break;
		case StatInfoDisplay::INTPERCENT: sprintf(buffer, "%g%%", fvalue / 10.0F); break;
		case StatInfoDisplay::PERCENT: sprintf(buffer, "%g%%", fvalue); break;
		case StatInfoDisplay::PLUSPERCENT: sprintf(buffer, "+%g%%", fvalue); break;
		case StatInfoDisplay::PLUSINTP: sprintf(buffer, "+%g%%", fvalue / 10.0F); break;
		case StatInfoDisplay::PLUSINT: sprintf(buffer, "+%g", fvalue); break;
		case StatInfoDisplay::FLOATMULT: sprintf(buffer, "%g%%", fvalue * 100.0F); break;
		}
		STRINGLIST row;
		row.push_back(statInfoDisplay[i].PrettyName);
		row.push_back(buffer);
		output.push_back(row);
	}
}

}
