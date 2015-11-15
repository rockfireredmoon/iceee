this.ItemEquipSlot <- {
	NONE = -1,
	WEAPON_MAIN_HAND = 0,
	WEAPON_OFF_HAND = 1,
	WEAPON_RANGED = 2,
	ARMOR_HEAD = 3,
	ARMOR_NECK = 4,
	ARMOR_SHOULDER = 5,
	ARMOR_CHEST = 6,
	ARMOR_ARMS = 7,
	ARMOR_HANDS = 8,
	ARMOR_WAIST = 9,
	ARMOR_LEGS = 10,
	ARMOR_FEET = 11,
	ARMOR_RING_L = 12,
	ARMOR_RING_R = 13,
	ARMOR_AMULET = 14,
	FOCUS_FIRE = 15,
	FOCUS_FROST = 16,
	FOCUS_MYSTIC = 17,
	FOCUS_DEATH = 18,
	CONTAINER_0 = 19,
	CONTAINER_1 = 20,
	CONTAINER_2 = 21,
	CONTAINER_3 = 22,
	COSMETIC_SHOULDER_L = 23,
	COSMETIC_HIP_L = 24,
	COSMETIC_SHOULDER_R = 25,
	COSMETIC_HIP_R = 26,
	RED_CHARM = 27,
	GREEN_CHARM = 28,
	BLUE_CHARM = 29,
	ORANGE_CHARM = 30,
	YELLOW_CHARM = 31,
	PURPLE_CHARM = 32
};
this.ZoneUpdateFlags <- {
	ENTERING_ZONE = 1 << 0,
	TIME_OF_DAY = 1 << 1,
	PARTITION_TRANSFER = 1 << 2
};
this.InventoryMapping <- {
	inv = 0,
	eq = 1,
	bank = 2,
	shop = 3,
	buyback = 4,
	trade = 5,
	stamps = 7,
	delivery = 8,
	auction = 9
};
this.ItemEquipSlotToAttachmentPoint <- {
	[this.ItemEquipSlot.WEAPON_MAIN_HAND] = [
		"right_hand"
	],
	[this.ItemEquipSlot.WEAPON_RANGED] = [
		"right_hand"
	],
	[this.ItemEquipSlot.WEAPON_OFF_HAND] = [
		"left_hand"
	],
	[this.ItemEquipSlot.ARMOR_HEAD] = [
		"helmet"
	],
	[this.ItemEquipSlot.ARMOR_SHOULDER] = [
		"left_shoulder",
		"right_shoulder"
	],
	[this.ItemEquipSlot.ARMOR_NECK] = [
		"back_pack"
	]
};
this.ItemEquipSlotToClothing <- {
	[this.ItemEquipSlot.ARMOR_LEGS] = "leggings",
	[this.ItemEquipSlot.ARMOR_FEET] = "boots",
	[this.ItemEquipSlot.ARMOR_CHEST] = "chest",
	[this.ItemEquipSlot.ARMOR_ARMS] = "arms",
	[this.ItemEquipSlot.ARMOR_HANDS] = "gloves",
	[this.ItemEquipSlot.ARMOR_NECK] = "collar",
	[this.ItemEquipSlot.ARMOR_WAIST] = "belt"
};
this.ItemBindingType <- {
	BIND_NEVER = 0,
	BIND_ON_PICKUP = 1,
	BIND_ON_EQUIP = 2
};
this.ItemBindingTypeNameMapping <- {
	[this.ItemBindingType.BIND_NEVER] = {
		name = "Never"
	},
	[this.ItemBindingType.BIND_ON_PICKUP] = {
		name = "On Pickup"
	},
	[this.ItemBindingType.BIND_ON_EQUIP] = {
		name = "On Equip"
	}
};
this.ItemSlotType <- {
	UNKNOWN_SLOT = -1,
	ANY_SLOT = -2,
	EMPTY_SLOT = -3
};
this.ItemAutoTitleType <- {
	NONE = 0,
	FULL = 1,
	PREFIX = 2,
	SUFFIX = 3
};
this.ItemType <- {
	UNKNOWN = 0,
	SYSTEM = 1,
	WEAPON = 2,
	ARMOR = 3,
	CHARM = 4,
	CONSUMABLE = 5,
	CONTAINER = 6,
	BASIC = 7,
	SPECIAL = 8,
	QUEST = 9,
	RECIPE = 10
};
this.ItemTypeNameDef <- {
	[this.ItemType.UNKNOWN] = {
		name = "Unknown"
	},
	[this.ItemType.SYSTEM] = {
		name = "System"
	},
	[this.ItemType.WEAPON] = {
		name = "Weapon"
	},
	[this.ItemType.ARMOR] = {
		name = "Armor"
	},
	[this.ItemType.CHARM] = {
		name = "Charm"
	},
	[this.ItemType.CONSUMABLE] = {
		name = "Consumable"
	},
	[this.ItemType.CONTAINER] = {
		name = "Container"
	},
	[this.ItemType.BASIC] = {
		name = "Basic"
	},
	[this.ItemType.SPECIAL] = {
		name = "Special"
	},
	[this.ItemType.QUEST] = {
		name = "Quest"
	},
	[this.ItemType.RECIPE] = {
		name = "Recipe"
	}
};
this.SpecialItemType <- {
	NONE = 0,
	REAGENT_GENERATOR = 1,
	ITEM_GRINDER = 2,
	XP_BOOST = 3
};
this.SpecialItemTypeNameMapping <- {
	[this.SpecialItemType.NONE] = {
		name = "NONE"
	},
	[this.SpecialItemType.REAGENT_GENERATOR] = {
		name = "REAGENT_GENERATOR"
	},
	[this.SpecialItemType.ITEM_GRINDER] = {
		name = "ITEM_GRINDER"
	},
	[this.SpecialItemType.XP_BOOST] = {
		name = "XP_BOOST"
	}
};
this.CharmType <- {
	NONE = 0,
	RED = 1,
	GREEN = 2,
	BLUE = 3,
	ORANGE = 4,
	YELLOW = 5,
	PURPLE = 6
};
this.AbilityFlags <- {
	PARTY_CAST = 1 << 0
};
this.PVPGameType <- {
	NONE = 0,
	MILLEAGE = 1,
	MASSACRE = 2,
	TEAMSLAYER = 3,
	CTF = 4,
	DEATHMATCH = 5
};
this.PVPGameState <- {
	WAITING_TO_START = 0,
	WAITING_TO_CONTINUE = 1,
	PLAYING = 2,
	POST_GAME_LOBBY = 3
};
this.PVPTeams <- {
	NONE = 0,
	RED = 1,
	BLUE = 2,
	YELLOW = 3,
	GREEN = 4
};
this.FlagType <- {
	RED = 0,
	BLUE = 1
};
this.FlagEvents <- {
	TAKEN = 0,
	DROPPED = 1,
	CAPTURED = 2,
	RETURNED = 3
};
this.CurrencyCategory <- {
	COPPER = 0,
	CREDITS = 1
};
this.CurrencyType <- {
	COPPER = 0,
	SILVER = 1,
	GOLD = 2
};
this.Professions <- {
	NONE = 0,
	[0] = {
		name = "None"
	},
	KNIGHT = 1,
	[1] = {
		name = "Knight"
	},
	ROGUE = 2,
	[2] = {
		name = "Rogue"
	},
	MAGE = 3,
	[3] = {
		name = "Mage"
	},
	DRUID = 4,
	[4] = {
		name = "Druid"
	},
	MONSTER = 5,
	[5] = {
		name = "Monster"
	}
};
this.ItemEquipType <- {
	NONE = 0,
	WEAPON_1H = 1,
	WEAPON_1H_UNIQUE = 2,
	WEAPON_1H_MAIN = 3,
	WEAPON_1H_OFF = 4,
	WEAPON_2H = 5,
	WEAPON_RANGED = 6,
	ARMOR_SHIELD = 7,
	ARMOR_HEAD = 8,
	ARMOR_NECK = 9,
	ARMOR_SHOULDER = 10,
	ARMOR_CHEST = 11,
	ARMOR_ARMS = 12,
	ARMOR_HANDS = 13,
	ARMOR_WAIST = 14,
	ARMOR_LEGS = 15,
	ARMOR_FEET = 16,
	ARMOR_RING = 17,
	ARMOR_RING_UNIQUE = 18,
	ARMOR_AMULET = 19,
	FOCUS_FIRE = 20,
	FOCUS_FROST = 21,
	FOCUS_MYSTIC = 22,
	FOCUS_DEATH = 23,
	CONTAINER = 24,
	COSEMETIC_SHOULDER = 25,
	COSEMETIC_HIP = 26,
	RED_CHARM = 27,
	GREEN_CHARM = 28,
	BLUE_CHARM = 29,
	ORANGE_CHARM = 30,
	YELLOW_CHARM = 31,
	PURPLE_CHARM = 32
};
this.ItemEquipTypeNameMapping <- {
	[this.ItemEquipType.NONE] = {
		name = "NONE"
	},
	[this.ItemEquipType.WEAPON_1H] = {
		name = "WEAPON_1H"
	},
	[this.ItemEquipType.WEAPON_1H_UNIQUE] = {
		name = "WEAPON_1H_UNIQUE"
	},
	[this.ItemEquipType.WEAPON_1H_MAIN] = {
		name = "WEAPON_1H_MAIN"
	},
	[this.ItemEquipType.WEAPON_1H_OFF] = {
		name = "WEAPON_1H_OFF"
	},
	[this.ItemEquipType.WEAPON_2H] = {
		name = "WEAPON_2H"
	},
	[this.ItemEquipType.WEAPON_RANGED] = {
		name = "WEAPON_RANGED"
	},
	[this.ItemEquipType.ARMOR_SHIELD] = {
		name = "ARMOR_SHIELD"
	},
	[this.ItemEquipType.ARMOR_HEAD] = {
		name = "ARMOR_HEAD"
	},
	[this.ItemEquipType.ARMOR_NECK] = {
		name = "ARMOR_NECK"
	},
	[this.ItemEquipType.ARMOR_SHOULDER] = {
		name = "ARMOR_SHOULDER"
	},
	[this.ItemEquipType.ARMOR_CHEST] = {
		name = "ARMOR_CHEST"
	},
	[this.ItemEquipType.ARMOR_ARMS] = {
		name = "ARMOR_ARMS"
	},
	[this.ItemEquipType.ARMOR_HANDS] = {
		name = "ARMOR_HANDS"
	},
	[this.ItemEquipType.ARMOR_WAIST] = {
		name = "ARMOR_WAIST"
	},
	[this.ItemEquipType.ARMOR_LEGS] = {
		name = "ARMOR_LEGS"
	},
	[this.ItemEquipType.ARMOR_FEET] = {
		name = "ARMOR_FEET"
	},
	[this.ItemEquipType.ARMOR_RING] = {
		name = "ARMOR_RING"
	},
	[this.ItemEquipType.ARMOR_RING_UNIQUE] = {
		name = "ARMOR_RING_UNIQUE"
	},
	[this.ItemEquipType.ARMOR_AMULET] = {
		name = "ARMOR_AMULET"
	},
	[this.ItemEquipType.FOCUS_FIRE] = {
		name = "FOCUS_FIRE"
	},
	[this.ItemEquipType.FOCUS_FROST] = {
		name = "FOCUS_FROST"
	},
	[this.ItemEquipType.FOCUS_MYSTIC] = {
		name = "FOCUS_MYSTIC"
	},
	[this.ItemEquipType.FOCUS_DEATH] = {
		name = "FOCUS_DEATH"
	},
	[this.ItemEquipType.CONTAINER] = {
		name = "CONTAINER"
	},
	[this.ItemEquipType.COSEMETIC_SHOULDER] = {
		name = "COSEMETIC_SHOULDER"
	},
	[this.ItemEquipType.COSEMETIC_HIP] = {
		name = "COSEMETIC_HIP"
	},
	[this.ItemEquipType.RED_CHARM] = {
		name = "RED_CHARM"
	},
	[this.ItemEquipType.GREEN_CHARM] = {
		name = "GREEN_CHARM"
	},
	[this.ItemEquipType.BLUE_CHARM] = {
		name = "BLUE_CHARM"
	},
	[this.ItemEquipType.ORANGE_CHARM] = {
		name = "ORANGE_CHARM"
	},
	[this.ItemEquipType.YELLOW_CHARM] = {
		name = "YELLOW_CHARM"
	},
	[this.ItemEquipType.PURPLE_CHARM] = {
		name = "PURPLE_CHARM"
	}
};
this.ItemIntegerType <- {
	NONE = 0,
	STACKING = 1,
	DURABILITY = 2,
	CHARGES = 3,
	CAPACITY = 4,
	QUEST_ID = 5,
	RESULT_ITEM = 6,
	KEY_COMPONENT = 7,
	REQUIRE_ROLLING = 8,
	LIFETIME = 9,
	BONUS_VALUE = 10
};
this.ItemIntegerTypeNameMapping <- {
	[this.ItemIntegerType.NONE] = "None",
	[this.ItemIntegerType.STACKING] = "Stacking",
	[this.ItemIntegerType.DURABILITY] = "Durability",
	[this.ItemIntegerType.CHARGES] = "Charges",
	[this.ItemIntegerType.CAPACITY] = "Capacity",
	[this.ItemIntegerType.QUEST_ID] = "Quest Id",
	[this.ItemIntegerType.RESULT_ITEM] = "Result Item",
	[this.ItemIntegerType.KEY_COMPONENT] = "Key Component",
	[this.ItemIntegerType.REQUIRE_ROLLING] = "Require Rolling",
	[this.ItemIntegerType.LIFETIME] = "Lifetime",
	[this.ItemIntegerType.BONUS_VALUE] = "Bonus Value"
};
this.WeaponType <- {
	NONE = 0,
	SMALL = 1,
	ONE_HAND = 2,
	TWO_HAND = 3,
	POLE = 4,
	WAND = 5,
	BOW = 6,
	THROWN = 7,
	ARCANE_TOTEM = 8
};
this.WeaponTypeNameMapping <- {
	[this.WeaponType.NONE] = {
		name = "NONE"
	},
	[this.WeaponType.SMALL] = {
		name = "SMALL"
	},
	[this.WeaponType.ONE_HAND] = {
		name = "ONE_HAND"
	},
	[this.WeaponType.TWO_HAND] = {
		name = "TWO_HAND"
	},
	[this.WeaponType.POLE] = {
		name = "POLE"
	},
	[this.WeaponType.WAND] = {
		name = "WAND"
	},
	[this.WeaponType.BOW] = {
		name = "BOW"
	},
	[this.WeaponType.THROWN] = {
		name = "THROWN"
	},
	[this.WeaponType.ARCANE_TOTEM] = {
		name = "ARCANE_TOTEM"
	}
};
this.DamageType <- {
	MELEE = 0,
	FIRE = 1,
	FROST = 2,
	MYSTIC = 3,
	DEATH = 4,
	UNBLOCKABLE = 5
};
this.DamageTypeNameMapping <- {
	[this.DamageType.MELEE] = "Physical",
	[this.DamageType.FIRE] = "Fire",
	[this.DamageType.FROST] = "Frost",
	[this.DamageType.MYSTIC] = "Mystic",
	[this.DamageType.DEATH] = "Death",
	[this.DamageType.UNBLOCKABLE] = "Unblockable"
};
this.DamageTypeNameMappingLower <- {
	[this.DamageType.MELEE] = "physical",
	[this.DamageType.FIRE] = "fire",
	[this.DamageType.FROST] = "frost",
	[this.DamageType.MYSTIC] = "mystic",
	[this.DamageType.DEATH] = "death",
	[this.DamageType.UNBLOCKABLE] = "unblockable"
};
this.ArmorType <- {
	NONE = 0,
	CLOTH = 1,
	LIGHT = 2,
	MEDIUM = 3,
	HEAVY = 4,
	SHIELD = 5
};
this.ArmorTypeNameMapping <- {
	[this.ArmorType.NONE] = {
		name = "NONE"
	},
	[this.ArmorType.CLOTH] = {
		name = "CLOTH"
	},
	[this.ArmorType.LIGHT] = {
		name = "LIGHT"
	},
	[this.ArmorType.MEDIUM] = {
		name = "MEDIUM"
	},
	[this.ArmorType.HEAVY] = {
		name = "HEAVY"
	},
	[this.ArmorType.SHIELD] = {
		name = "SHIELD"
	}
};
this.PartyUpdateOpTypes <- {
	INVITE = 0,
	PROPOSE_INVITE = 1,
	INVITE_REJECTED = 2,
	ADD_MEMBER = 3,
	REMOVE_MEMBER = 4,
	JOINED_PARTY = 5,
	LEFT_PARTY = 6,
	IN_CHARGE = 7,
	STRATEGY_CHANGE = 8,
	STRATEGYFLAGS_CHANGE = 9,
	OFFER_LOOT = 10,
	LOOT_ROLL = 11,
	LOOT_WIN = 12,
	QUEST_INVITE = 13
};
this.TradeEventTypes <- {
	REQUEST = 0,
	REQUEST_ACCEPTED = 1,
	REQUEST_CLOSED = 2,
	ITEM_ADDED = 3,
	ITEM_REMOVED = 4,
	CURRENCY_OFFERED = 5,
	OFFER_MADE = 6,
	OFFER_ACCEPTED = 7,
	OFFER_CANCELED = 8,
	ITEMS_OFFERED = 9
};
this.CloseReasons <- {
	COMPLETE = 0,
	TIMEOUT = 1,
	DISTANCE = 2,
	CANCELED = 3,
	INSUFFICIENT_FUNDS = 4,
	INSUFFICIENT_SPACE = 5,
	INVALID_ITEMS = 6,
	PLAYER_DEAD = 7
};
this.AbilityStatus <- {
	WARMUP = 0,
	ACTIVATE = 1,
	SETBACK = 2,
	CHANNELING = 3,
	INTERRUPTED = 4,
	INITIAL_FAIL = 5,
	ABILITY_ERROR = 6
};
this.CreatureRarityType <- {
	NORMAL = 0,
	HEROIC = 1,
	EPIC = 2,
	LEGEND = 3
};
this.RarityTypeHealthModifier <- {
	NORMAL = 1.0,
	HEROIC = 2.5,
	EPIC = 5.0,
	LEGEND = 10.0
};
this.QualityLevel <- {
	POOR = 0,
	STANDARD = 1,
	GOOD = 2,
	SUPERIOR = 3,
	EPIC = 4,
	LEGENDARY = 5,
	ARTIFACT = 6
};
this.DebuffType <- {
	NONE = 0,
	CONTROL = 1,
	STAT = 2
};
this.AbilityIdType <- {
	TWO_HAND_WEAPONS = 1,
	ONE_HAND_WEAPONS = 2,
	SMALL_WEAPONS = 3,
	POLE_WEAPONS = 4,
	BOW_WEAPONS = 5,
	THROWN_WEAPONS = 6,
	WAND_WEAPONS = 7,
	DUAL_WIELD = 8,
	PARRY = 9,
	BLOCK = 10,
	CLOTH_ARMOR = 11,
	LIGHT_ARMOR = 13,
	MEDIUM_ARMOR = 14,
	HEAVY_ARMOR = 15
};
this.Stat <- {
	APPEARANCE = 0,
	[0] = {
		type = "string",
		name = "appearance",
		prettyName = "Appearance"
	},
	EQ_APPEARANCE = 1,
	[1] = {
		type = "string",
		name = "eq_appearance",
		prettyName = "Equipment Appearance"
	},
	LEVEL = 2,
	[2] = {
		type = "short",
		name = "level",
		prettyName = "Level"
	},
	DISPLAY_NAME = 3,
	[3] = {
		type = "string",
		name = "display_name",
		prettyName = "Display Name"
	},
	STRENGTH = 4,
	[4] = {
		type = "short",
		name = "strength",
		prettyName = "Strength",
		abbrev = "STR",
		baseStat = 4,
		debuff = this.DebuffType.STAT
	},
	DEXTERITY = 5,
	[5] = {
		type = "short",
		name = "dexterity",
		prettyName = "Dexterity",
		abbrev = "DEX",
		baseStat = 5,
		debuff = this.DebuffType.STAT
	},
	CONSTITUTION = 6,
	[6] = {
		type = "short",
		name = "constitution",
		prettyName = "Constitution",
		abbrev = "CON",
		baseStat = 6,
		debuff = this.DebuffType.STAT
	},
	PSYCHE = 7,
	[7] = {
		type = "short",
		name = "psyche",
		prettyName = "Psyche",
		abbrev = "PSY",
		baseStat = 7,
		debuff = this.DebuffType.STAT
	},
	SPIRIT = 8,
	[8] = {
		type = "short",
		name = "spirit",
		prettyName = "Spirit",
		abbrev = "SPI",
		baseStat = 8,
		debuff = this.DebuffType.STAT
	},
	ARMOR_RATING = 30,
	[30] = {
		type = "short",
		name = "armor_rating",
		prettyName = "Armor Rating",
		abbrev = "AR",
		debuff = this.DebuffType.STAT
	},
	MELEE_ATTACK_SPEED = 31,
	[31] = {
		type = "short",
		name = "melee_attack_speed",
		prettyName = "Melee Attack Speed",
		debuff = this.DebuffType.CONTROL
	},
	MAGIC_ATTACK_SPEED = 32,
	[32] = {
		type = "short",
		name = "magic_attack_speed",
		prettyName = "Magic Attack Speed",
		debuff = this.DebuffType.CONTROL
	},
	BASE_DAMAGE_MELEE = 33,
	[33] = {
		type = "short",
		name = "base_damage_melee",
		prettyName = "Physical Damage",
		debuff = this.DebuffType.STAT
	},
	BASE_DAMAGE_FIRE = 34,
	[34] = {
		type = "short",
		name = "base_damage_fire",
		prettyName = "Fire Damage",
		debuff = this.DebuffType.STAT
	},
	BASE_DAMAGE_FROST = 35,
	[35] = {
		type = "short",
		name = "base_damage_frost",
		prettyName = "Frost Damage",
		debuff = this.DebuffType.STAT
	},
	BASE_DAMAGE_MYSTIC = 36,
	[36] = {
		type = "short",
		name = "base_damage_mystic",
		prettyName = "Mystic Damage",
		debuff = this.DebuffType.STAT
	},
	BASE_DAMAGE_DEATH = 37,
	[37] = {
		type = "short",
		name = "base_damage_death",
		prettyName = "Death Damage",
		debuff = this.DebuffType.STAT
	},
	DAMAGE_RESIST_MELEE = 38,
	[38] = {
		type = "short",
		name = "damage_resist_melee",
		prettyName = "Melee Resistance",
		debuff = this.DebuffType.STAT
	},
	DAMAGE_RESIST_FIRE = 39,
	[39] = {
		type = "short",
		name = "damage_resist_fire",
		prettyName = "Fire Resistance",
		debuff = this.DebuffType.STAT
	},
	DAMAGE_RESIST_FROST = 40,
	[40] = {
		type = "short",
		name = "damage_resist_frost",
		prettyName = "Frost Resistance",
		debuff = this.DebuffType.STAT
	},
	DAMAGE_RESIST_MYSTIC = 41,
	[41] = {
		type = "short",
		name = "damage_resist_mystic",
		prettyName = "Mystic Resistance",
		debuff = this.DebuffType.STAT
	},
	DAMAGE_RESIST_DEATH = 42,
	[42] = {
		type = "short",
		name = "damage_resist_death",
		prettyName = "Death Resistance",
		debuff = this.DebuffType.STAT
	},
	BASE_MOVEMENT = 43,
	[43] = {
		type = "short",
		name = "base_movement",
		prettyName = "Movement",
		debuff = this.DebuffType.CONTROL
	},
	BASE_LUCK = 44,
	[44] = {
		type = "short",
		name = "base_luck",
		prettyName = "Luck",
		debuff = this.DebuffType.STAT
	},
	BASE_HEALTH = 45,
	[45] = {
		type = "short",
		name = "base_health",
		prettyName = "Health",
		debuff = this.DebuffType.STAT
	},
	WILL_MAX = 46,
	[46] = {
		type = "short",
		name = "will_max",
		prettyName = "Willpower Max",
		debuff = this.DebuffType.STAT
	},
	WILL_REGEN = 47,
	[47] = {
		type = "float",
		name = "will_regen",
		prettyName = "Willpower Regen",
		debuff = this.DebuffType.STAT
	},
	MIGHT_MAX = 48,
	[48] = {
		type = "short",
		name = "might_max",
		prettyName = "Might Max",
		debuff = this.DebuffType.STAT
	},
	MIGHT_REGEN = 49,
	[49] = {
		type = "float",
		name = "might_regen",
		prettyName = "Might Regen",
		debuff = this.DebuffType.STAT
	},
	BASE_DODGE = 50,
	[50] = {
		type = "short",
		name = "base_dodge",
		prettyName = "Dodge",
		debuff = this.DebuffType.STAT
	},
	BASE_DEFLECT = 51,
	[51] = {
		type = "short",
		name = "base_deflect",
		prettyName = "Deflect",
		debuff = this.DebuffType.STAT
	},
	BASE_PARRY = 52,
	[52] = {
		type = "short",
		name = "base_parry",
		prettyName = "Parry",
		debuff = this.DebuffType.STAT
	},
	BASE_BLOCK = 53,
	[53] = {
		type = "short",
		name = "base_block",
		prettyName = "Blocking",
		debuff = this.DebuffType.STAT
	},
	BASE_MELEE_TO_HIT = 54,
	[54] = {
		type = "short",
		name = "base_melee_to_hit",
		prettyName = "Melee To-Hit"
	},
	BASE_MELEE_CRITICAL = 55,
	[55] = {
		type = "short",
		name = "base_melee_critical",
		prettyName = "Melee Critical"
	},
	BASE_MAGIC_SUCCESS = 56,
	[56] = {
		type = "short",
		name = "base_magic_success",
		prettyName = "Magic Success"
	},
	BASE_MAGIC_CRITICAL = 57,
	[57] = {
		type = "short",
		name = "base_magic_critical",
		prettyName = "Magic Critical"
	},
	OFFHAND_WEAPON_DAMAGE = 58,
	[58] = {
		type = "short",
		name = "offhand_weapon_damage",
		prettyName = "Off-Hand Weapon Damage",
		debuff = this.DebuffType.STAT
	},
	CASTING_SETBACK_CHANCE = 59,
	[59] = {
		type = "short",
		name = "casting_setback_chance",
		prettyName = "Casting Setback Chance"
	},
	CHANNELING_BREAK_CHANCE = 60,
	[60] = {
		type = "short",
		name = "channeling_break_chance",
		prettyName = "Channeling Break Chance"
	},
	BASE_HEALING = 61,
	[61] = {
		type = "short",
		name = "base_healing",
		prettyName = "Healing",
		debuff = this.DebuffType.STAT
	},
	AGGRO_RADIUS_MOD = 62,
	[62] = {
		type = "short",
		name = "aggro_radius_mod",
		prettyName = "Aggro Radius",
		debuff = this.DebuffType.STAT
	},
	HATE_GAIN_RATE = 63,
	[63] = {
		type = "short",
		name = "hate_gain_rate",
		prettyName = "Hatred Gain Rate"
	},
	EXPERIENCE_GAIN_RATE = 64,
	[64] = {
		type = "short",
		name = "experience_gain_rate",
		prettyName = "XP Gain Rate"
	},
	COIN_GAIN_RATE = 65,
	[65] = {
		type = "short",
		name = "coin_gain_rate",
		prettyName = "Coin Gain Rate"
	},
	MAGIC_LOOT_DROP_RATE = 66,
	[66] = {
		type = "short",
		name = "magic_loot_drop_rate",
		prettyName = "Magical Item Drops"
	},
	INVENTORY_CAPACITY = 67,
	[67] = {
		type = "short",
		name = "inventory_capacity",
		prettyName = "Inventory Capacity"
	},
	VIS_WEAPON = 69,
	[69] = {
		type = "short",
		name = "vis_weapon",
		prettyName = "Visible Weapon"
	},
	SUB_NAME = 70,
	[70] = {
		type = "string",
		name = "sub_name",
		prettyName = "Sub Name"
	},
	ALTERNATE_IDLE_ANIM = 71,
	[71] = {
		type = "string",
		name = "alternate_idle_anim",
		prettyName = "Idle Animation"
	},
	SELECTIVE_EQ_OVERRIDE = 72,
	[72] = {
		type = "string",
		name = "selective_eq_override",
		prettyName = "Selective Equipment Override"
	},
	HEALTH = 80,
	[80] = {
		type = "short",
		name = "health",
		prettyName = "Health",
		abbrev = "HP",
		debuff = this.DebuffType.STAT
	},
	WILL = 81,
	[81] = {
		type = "short",
		name = "will",
		prettyName = "Will",
		abbrev = "W",
		debuff = this.DebuffType.STAT
	},
	WILL_CHARGES = 82,
	[82] = {
		type = "short",
		name = "will_charges",
		prettyName = "Will Charges",
		abbrev = "WCh"
	},
	MIGHT = 83,
	[83] = {
		type = "short",
		name = "might",
		prettyName = "Might",
		abbrev = "M",
		debuff = this.DebuffType.STAT
	},
	MIGHT_CHARGES = 84,
	[84] = {
		type = "short",
		name = "might_charges",
		prettyName = "Might Charges",
		abbrev = "MCh"
	},
	SIZE = 85,
	[85] = {
		type = "short",
		name = "size",
		prettyName = "Size",
		abbrev = "sz"
	},
	PROFESSION = 86,
	[86] = {
		type = "short",
		name = "profession",
		prettyName = "Profession"
	},
	WEAPON_DAMAGE_2H = 87,
	[87] = {
		type = "short",
		name = "weapon_damage_2h",
		prettyName = "Weapon Damage: Two-Handed",
		debuff = this.DebuffType.STAT
	},
	WEAPON_DAMAGE_1H = 88,
	[88] = {
		type = "short",
		name = "weapon_damage_1h",
		prettyName = "Weapon Damage: One-Handed",
		debuff = this.DebuffType.STAT
	},
	WEAPON_DAMAGE_POLE = 89,
	[89] = {
		type = "short",
		name = "weapon_damage_pole",
		prettyName = "Weapon Damage: Pole"
	},
	WEAPON_DAMAGE_SMALL = 90,
	[90] = {
		type = "short",
		name = "weapon_damage_small",
		prettyName = "Weapon Damage: Small",
		debuff = this.DebuffType.STAT
	},
	WEAPON_DAMAGE_BOW = 91,
	[91] = {
		type = "short",
		name = "weapon_damage_box",
		prettyName = "Weapon Damage: Bow",
		debuff = this.DebuffType.STAT
	},
	WEAPON_DAMAGE_THROWN = 92,
	[92] = {
		type = "short",
		name = "weapon_damage_thrown",
		prettyName = "Weapon Damage: Thrown",
		debuff = this.DebuffType.STAT
	},
	WEAPON_DAMAGE_WAND = 93,
	[93] = {
		type = "short",
		name = "weapon_damage_wand",
		prettyName = "Weapon Damage: Wand",
		debuff = this.DebuffType.STAT
	},
	EXTRA_DAMAGE_FIRE = 94,
	[94] = {
		type = "short",
		name = "extra_damage_fire",
		prettyName = "Extra Damage: Fire",
		debuff = this.DebuffType.STAT
	},
	EXTRA_DAMAGE_FROST = 95,
	[95] = {
		type = "short",
		name = "extra_damage_frost",
		prettyName = "Extra Damage: Frost",
		debuff = this.DebuffType.STAT
	},
	EXTRA_DAMAGE_MYSTIC = 96,
	[96] = {
		type = "short",
		name = "extra_damage_mystic",
		prettyName = "Extra Damage: Mystic",
		debuff = this.DebuffType.STAT
	},
	EXTRA_DAMAGE_DEATH = 97,
	[97] = {
		type = "short",
		name = "extra_damage_death",
		prettyName = "Extra Damage: Death",
		debuff = this.DebuffType.STAT
	},
	APPEARANCE_OVERRIDE = 98,
	[98] = {
		type = "string",
		name = "appearance_override",
		prettyName = "Appearance Override"
	},
	INVISIBILITY_DISTANCE = 99,
	[99] = {
		type = "float",
		name = "invisibility_distance",
		prettyName = "Invisibility Distance"
	},
	TRANSLOCATE_DESTINATION = 100,
	[100] = {
		type = "string",
		name = "translocate_destination",
		prettyName = "Translocate Destination"
	},
	LOOTABLE_PLAYER_IDS = 101,
	[101] = {
		type = "string",
		name = "lootable_player_ids",
		prettyName = "Lootable Player ID\'s"
	},
	MASTER = 102,
	[102] = {
		type = "short",
		name = "master",
		prettyName = "Master"
	},
	REZ_PENDING = 103,
	[103] = {
		type = "short",
		name = "rez_pending",
		prettyName = "Pending Resurrection Offer"
	},
	LOOT_SEEABLE_PLAYER_IDS = 104,
	[104] = {
		type = "string",
		name = "loot_seeable_player_ids",
		prettyName = "Loot Seeable Player ID\'s"
	},
	LOOT = 105,
	[105] = {
		type = "string",
		name = "loot",
		prettyName = "Loot"
	},
	BASE_STATS = 107,
	[107] = {
		type = "string",
		name = "base_stats",
		prettyName = "Base Stats"
	},
	HEALTH_MOD = 108,
	[108] = {
		type = "short",
		name = "health_mod",
		prettyName = "Health Mod"
	},
	HEROISM = 110,
	[110] = {
		type = "short",
		name = "heroism",
		prettyName = "Heroism"
	},
	EXPERIENCE = 111,
	[111] = {
		type = "int",
		name = "experience",
		prettyName = "Experience Points",
		abbrev = "XP"
	},
	TOTAL_ABILITY_POINTS = 112,
	[112] = {
		type = "short",
		name = "total_ability_points",
		prettyName = "Total Ability Points",
		abbrev = "TAP"
	},
	CURRENT_ABILITY_POINTS = 113,
	[113] = {
		type = "short",
		name = "current_ability_points",
		prettyName = "Current Ability Points",
		abbrev = "CAP"
	},
	COPPER = 114,
	[114] = {
		type = "int",
		name = "copper",
		prettyName = "Copper"
	},
	CREDITS = 115,
	[115] = {
		type = "int",
		name = "credits",
		prettyName = "Credits"
	},
	AI_MODULE = 120,
	[120] = {
		type = "string",
		name = "ai_module",
		prettyName = "AI Module"
	},
	CREATURE_CATEGORY = 121,
	[121] = {
		type = "string",
		name = "creature_category",
		prettyName = "Creature Category"
	},
	RARITY = 122,
	[122] = {
		type = "short",
		name = "rarity",
		prettyName = "Rarity"
	},
	SPAWN_TABLE_POINTS = 123,
	[123] = {
		type = "short",
		name = "spawn_table_points",
		prettyName = "Spawn Table Points"
	},
	AI_PACKAGE = 124,
	[124] = {
		type = "string",
		name = "ai_package",
		prettyName = "AI Package"
	},
	BONUS_HEALTH = 125,
	[125] = {
		type = "short",
		name = "bonus_health",
		prettyName = "Bonus Health"
	},
	DR_MOD_MELEE = 126,
	[126] = {
		type = "int",
		name = "dr_mod_melee",
		prettyName = "Physical Damage Reduction"
	},
	DR_MOD_FIRE = 127,
	[127] = {
		type = "int",
		name = "dr_mod_fire",
		prettyName = "Fire Damage Reduction"
	},
	DR_MOD_FROST = 128,
	[128] = {
		type = "int",
		name = "dr_mod_frost",
		prettyName = "Frost Damage Reduction"
	},
	DR_MOD_MYSTIC = 129,
	[129] = {
		type = "int",
		name = "dr_mod_mystic",
		prettyName = "Mystic Damage Reduction"
	},
	DR_MOD_DEATH = 130,
	[130] = {
		type = "int",
		name = "dr_mod_death",
		prettyName = "Death Damage Reduction"
	},
	DMG_MOD_MELEE = 131,
	[131] = {
		type = "int",
		name = "dmg_mod_melee",
		prettyName = "Physical Damage Modifier"
	},
	DMG_MOD_FIRE = 132,
	[132] = {
		type = "int",
		name = "dmg_mod_fire",
		prettyName = "Fire Damage Modifier"
	},
	DMG_MOD_FROST = 133,
	[133] = {
		type = "int",
		name = "dmg_mod_frost",
		prettyName = "Frost Damage Modifier"
	},
	DMG_MOD_MYSTIC = 134,
	[134] = {
		type = "int",
		name = "dmg_mod_mystic",
		prettyName = "Mystic Damage Modifier"
	},
	DMG_MOD_DEATH = 135,
	[135] = {
		type = "int",
		name = "dmg_mod_death",
		prettyName = "Death Damage Modifier"
	},
	PVP_TEAM = 136,
	[136] = {
		type = "short",
		name = "pvp_team",
		prettyName = "PvP Team"
	},
	PVP_KILLS = 137,
	[137] = {
		type = "int",
		name = "pvp_score",
		prettyName = "PvP Score"
	},
	PVP_DEATHS = 138,
	[138] = {
		type = "int",
		name = "pvp_score",
		prettyName = "PvP Score"
	},
	PVP_STATE = 139,
	[139] = {
		type = "string",
		name = "pvp_state",
		prettyName = "PvP State"
	},
	PVP_FLAG_CAPTURES = 140,
	[140] = {
		type = "integer",
		name = "pvp_flag_captures",
		prettyName = "PvP Flag Captures"
	},
	MOD_MELEE_TO_HIT = 141,
	[141] = {
		type = "float",
		name = "mod_melee_to_hit",
		prettyName = "Mod Melee To Hit"
	},
	MOD_MELEE_TO_CRIT = 142,
	[142] = {
		type = "float",
		name = "mod_melee_to_crit",
		prettyName = "Mod Melee To Crit"
	},
	MOD_MAGIC_TO_HIT = 143,
	[143] = {
		type = "float",
		name = "mod_melee_to_hit",
		prettyName = "Mod Magic To Hit"
	},
	MOD_MAGIC_TO_CRIT = 144,
	[144] = {
		type = "float",
		name = "mod_melee_to_crit",
		prettyName = "Mod Magic To Crit"
	},
	MOD_PARRY = 145,
	[145] = {
		type = "float",
		name = "mod_parry",
		prettyName = "Mod Parry"
	},
	MOD_BLOCK = 146,
	[146] = {
		type = "float",
		name = "mod_block",
		prettyName = "Mod Block"
	},
	MOD_MOVEMENT = 147,
	[147] = {
		type = "int",
		name = "mod_movement",
		prettyName = "Mod Movement"
	},
	MOD_HEALTH_REGEN = 148,
	[148] = {
		type = "float",
		name = "mod_health_regen",
		prettyName = "Mod Health Regen"
	},
	MOD_ATTACK_SPEED = 149,
	[149] = {
		type = "float",
		name = "mod_attack_speed",
		prettyName = "Mod Attack Speed"
	},
	MOD_CASTING_SPEED = 150,
	[150] = {
		type = "float",
		name = "mod_casting_speed",
		prettyName = "Mod Casting Speed"
	},
	MOD_HEALING = 151,
	[151] = {
		type = "float",
		name = "mod_healing",
		prettyName = "Mod Healing"
	},
	TOTAL_SIZE = 152,
	[152] = {
		type = "float",
		name = "total_size",
		prettyName = "Total Size"
	},
	AGGRO_PLAYERS = 153,
	[153] = {
		type = "short",
		name = "aggro_players",
		prettyName = "Aggro Players"
	},
	MOD_LUCK = 154,
	[154] = {
		type = "float",
		name = "mod_luck",
		prettyName = "Mod Luck"
	},
	HEALTH_REGEN = 155,
	[155] = {
		type = "short",
		name = "health_regen",
		prettyName = "Health Regen"
	},
	BLEEDING = 156,
	[156] = {
		type = "short",
		name = "bleeding",
		prettyName = "Bleeding"
	},
	DAMAGE_SHIELD = 157,
	[157] = {
		type = "int",
		name = "damage_shield",
		prettyName = "Damage Shield"
	},
	HIDE_NAMEBOARD = 158,
	[158] = {
		type = "short",
		name = "hide_nameboard",
		prettyName = "Hide Nameboard"
	},
	HIDE_MINIMAP = 159,
	[159] = {
		type = "short",
		name = "hide_minimap",
		prettyName = "Hide Nameboard"
	},
	CREDIT_DROPS = 160,
	[160] = {
		type = "short",
		name = "credit_drops",
		prettyName = "Credit Drops"
	},
	HEROISM_GAIN_RATE = 161,
	[161] = {
		type = "short",
		name = "heroism_gain_rate",
		prettyName = "Heroism Gain Rate"
	},
	QUEST_EXP_GAIN_RATE = 162,
	[162] = {
		type = "short",
		name = "quest_exp_gain_rate",
		prettyName = "Quest Experience Gain Rate"
	},
	DROP_GAIN_RATE = 163,
	[163] = {
		type = "short",
		name = "drop_gain_rate",
		prettyName = "Treasure Gain Rate"
	}
};
this.StatusEffects <- {
	DEAD = 0,
	[0] = {
		prettyName = "Dead",
		icon = "",
		type = "Debuff"
	},
	SILENCE = 1,
	[1] = {
		prettyName = "Silenced",
		icon = "Icon-Ready2.png|Icon-32-BG-Aqua.png",
		type = "Debuff"
	},
	DISARM = 2,
	[2] = {
		prettyName = "Disarmed",
		icon = "",
		type = "Debuff"
	},
	STUN = 3,
	[3] = {
		prettyName = "Stunned",
		icon = "Icon-Ready5.png|Icon-32-BG-Red.png",
		type = "Debuff"
	},
	DAZE = 4,
	[4] = {
		prettyName = "Dazed",
		icon = "Icon-32-Ability-Trav_Tele_NewBadari.png|Icon-32-BG-Yellow.png",
		type = "Debuff"
	},
	CHARM = 5,
	[5] = {
		prettyName = "Charmed",
		icon = "",
		type = "Debuff"
	},
	FEAR = 6,
	[6] = {
		prettyName = "Feared",
		icon = "",
		type = "Debuff"
	},
	ROOT = 7,
	[7] = {
		prettyName = "Rooted",
		icon = "Icon-32-Ability-D_Thorns.png|Icon-64-BG-Black.png",
		type = "Debuff"
	},
	LOCKED = 8,
	[8] = {
		prettyName = "Locked",
		icon = "",
		type = "Debuff"
	},
	BROKEN = 9,
	[9] = {
		prettyName = "Broken",
		icon = "",
		type = "Debuff"
	},
	CAN_USE_WEAPON_2H = 10,
	[10] = {
		prettyName = "Proficient with 2-handed weapons",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_1H = 11,
	[11] = {
		prettyName = "Proficient with 1-handed weapons",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_SMALL = 12,
	[12] = {
		prettyName = "Proficient with small weapons",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_POLE = 13,
	[13] = {
		prettyName = "Proficient with pole weapons",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_BOW = 14,
	[14] = {
		prettyName = "Proficient with bows",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_THROWN = 15,
	[15] = {
		prettyName = "Proficient with thrown weapons",
		icon = "",
		type = "None"
	},
	CAN_USE_WEAPON_WAND = 16,
	[16] = {
		prettyName = "Proficient with wands",
		icon = "",
		type = "None"
	},
	CAN_USE_DUAL_WIELD = 17,
	[17] = {
		prettyName = "Able to dual wield",
		icon = "",
		type = "None"
	},
	CAN_USE_PARRY = 18,
	[18] = {
		prettyName = "Able to parry",
		icon = "",
		type = "None"
	},
	CAN_USE_BLOCK = 19,
	[19] = {
		prettyName = "Able to block",
		icon = "",
		type = "None"
	},
	CAN_USE_ARMOR_CLOTH = 20,
	[20] = {
		prettyName = "Able to wear cloth armor",
		icon = "",
		type = "None"
	},
	CAN_USE_ARMOR_LIGHT = 21,
	[21] = {
		prettyName = "Able to wear light armor",
		icon = "",
		type = "None"
	},
	CAN_USE_ARMOR_MEDIUM = 22,
	[22] = {
		prettyName = "Able to wear medium armor",
		icon = "",
		type = "None"
	},
	CAN_USE_ARMOR_HEAVY = 23,
	[23] = {
		prettyName = "Able to wear heavy armor",
		icon = "",
		type = "None"
	},
	INVISIBLE = 24,
	[24] = {
		prettyName = "Invisible",
		icon = "Icon-32-Ability-M_Invisibility.png|Icon-32-BG-Black.png",
		type = "Buff"
	},
	ALL_SEEING = 25,
	[25] = {
		prettyName = "Can see invisible creatures",
		icon = "",
		type = "None"
	},
	IN_COMBAT_STAND = 26,
	[26] = {
		prettyName = "In Combat",
		icon = "",
		type = "None"
	},
	INVINCIBLE = 27,
	[27] = {
		prettyName = "Invincible",
		icon = "",
		type = "None"
	},
	FLEE = 28,
	[28] = {
		prettyName = "Fleeing",
		icon = "",
		type = "None"
	},
	NO_AGGRO_GAINED = 29,
	[29] = {
		prettyName = "No Aggro Gained",
		icon = "",
		type = "None"
	},
	UNATTACKABLE = 30,
	[30] = {
		prettyName = "Unattackable",
		icon = "",
		type = "None"
	},
	IS_USABLE = 31,
	[31] = {
		prettyName = "Usable",
		icon = "",
		type = "None"
	},
	CLIENT_LOADING = 32,
	[32] = {
		prettyName = "Client is Loading",
		icon = "",
		type = "None"
	},
	IMMUNE_INTERRUPT = 33,
	[33] = {
		prettyName = "Cannot be interrupted",
		icon = "",
		type = "None"
	},
	IMMUNE_SILENCE = 34,
	[34] = {
		prettyName = "Cannot be silenced",
		icon = "",
		type = "None"
	},
	IMMUNE_DISARM = 35,
	[35] = {
		prettyName = "Cannot be disarmed",
		icon = "",
		type = "None"
	},
	IMMUNE_BLEED = 36,
	[36] = {
		prettyName = "Cannot bleed",
		icon = "",
		type = "None"
	},
	IMMUNE_DAMAGE_FIRE = 37,
	[37] = {
		prettyName = "Immune to fire damage",
		icon = "",
		type = "None"
	},
	IMMUNE_DAMAGE_FROST = 38,
	[38] = {
		prettyName = "Immune to frost damage",
		icon = "",
		type = "None"
	},
	IMMUNE_DAMAGE_MYSTIC = 39,
	[39] = {
		prettyName = "Immune to mystic damage",
		icon = "",
		type = "None"
	},
	IMMUNE_DAMAGE_DEATH = 40,
	[40] = {
		prettyName = "Immune to death damage",
		icon = "",
		type = "None"
	},
	IMMUNE_DAMAGE_MELEE = 41,
	[41] = {
		prettyName = "Immune to physical damage",
		icon = "",
		type = "None"
	},
	DISABLED = 42,
	[42] = {
		prettyName = "Disabled",
		icon = "",
		type = "None"
	},
	AUTO_ATTACK = 43,
	[43] = {
		prettyName = "Auto Attack",
		icon = "",
		type = "None"
	},
	AUTO_ATTACK_RANGED = 44,
	[44] = {
		prettyName = "Ranged Auto Attack",
		icon = "",
		type = "None"
	},
	CARRYING_RED_FLAG = 45,
	[45] = {
		prettyName = "Carrying red flag",
		icon = "",
		type = "None"
	},
	CARRYING_BLUE_FLAG = 46,
	[46] = {
		prettyName = "Carrying blue flag",
		icon = "",
		type = "None"
	},
	HENGE = 47,
	[47] = {
		prettyName = "Henge",
		icon = "",
		type = "None"
	},
	TRANSFORMER = 48,
	[48] = {
		prettyName = "Transformer",
		icon = "",
		type = "None"
	},
	PVPABLE = 49,
	[49] = {
		prettyName = "Allows a persona to be attacked",
		icon = "",
		type = "None"
	},
	IN_COMBAT = 50,
	[50] = {
		prettyName = "In combat mode",
		icon = "",
		type = "None"
	},
	WALK_IN_SHADOWS = 51,
	[51] = {
		prettyName = "Walk in Shadows",
		icon = "Icon-32-Ability-R_Walk_in_Shadow.png|Icon-32-BG-Black.png",
		type = "Buff"
	},
	EVADE = 52,
	[52] = {
		prettyName = "Evade",
		icon = "",
		type = "None"
	},
	TAUNTED = 53,
	[53] = {
		prettyName = "Taunted",
		icon = "Icon-32-Ability-K_Provoke.png|Icon-32-BG-Black.png",
		type = "Debuff"
	},
	XP_BOOST = 54,
	[54] = {
		prettyName = "Xp boost",
		icon = "Icon-32-Ability-D_Mystic_Specialization.png|Icon-32-BG-Black.png",
		type = "World"
	},
	REAGENT_GENERATOR = 55,
	[55] = {
		prettyName = "Reagents will be bought for you automatically as needed",
		icon = "",
		type = "World"
	},
	RES_PENALTY = 56,
	[56] = {
		prettyName = "Resurrection Penalty",
		icon = "Icon-32-Skull_and_Bones.png|Icon-64-BG-Black.png",
		type = "Debuff"
	},
	IMMUNE_STUN = 57,
	[57] = {
		prettyName = "Immune to Stun",
		icon = "",
		type = "None"
	},
	IMMUNE_DAZE = 58,
	[58] = {
		prettyName = "Immune to Daze",
		icon = "",
		type = "None"
	},
	GM_FROZEN = 59,
	[59] = {
		prettyName = "A GM has frozen you.  You are unable to move or perform abilities.",
		icon = "Icon-32-Ability-M_Frost_Specialization.png|Icon-32-Ability-D_Theft_of_Will.png",
		type = "Debuff"
	},
	GM_INVISIBLE = 60,
	[60] = {
		prettyName = "GM invisibility",
		icon = "Icon-32-Ability-R_Balance.png|Icon-32-Ability-D_Theft_of_Will.png",
		type = "Buff"
	},
	GM_SILENCED = 61,
	[61] = {
		prettyName = "A GM has silenced you.  You are unable to speak.",
		icon = "Icon-32-Ability-M_Jarnsaxas_Kiss.png|Icon-32-Ability-M_Cataclysm.png",
		type = "Debuff"
	},
	UNKILLABLE = 62,
	[62] = {
		prettyName = "Unkillable",
		icon = "",
		type = ""
	},
	TRANSFORMED = 63,
	[63] = {
		prettyName = "You are transformed into a feral form.",
		icon = "",
		type = "Buff"
	},
	INVISIBLE_EQUIPMENT = 64,
	[64] = {
		prettyName = "Your equipment is invisible.",
		icon = "",
		type = "Buff"
	},
	USABLE_BY_COMBATANT = 65,
	[65] = {
		prettyName = "Maybe interacted with by someone in combat.",
		icon = "",
		type = "Buff"
	}
};
this.AbilityUseType <- {
	CAST = 1,
	CHANNELED = 2,
	PASSIVE = 4
};
this.PetitionCategory <- {
	[0] = {
		category = "Character Stuck"
	},
	[1] = {
		category = "Report User"
	},
	[2] = {
		category = "Bug: Item Related"
	},
	[3] = {
		category = "Bug: World Related"
	},
	[4] = {
		category = "Bug: Creature Related"
	},
	[5] = {
		category = "Bug: Quest Related"
	},
	[6] = {
		category = "Bug: Dungeon Related"
	},
	[7] = {
		category = "Bug: Grouping Related"
	},
	[8] = {
		category = "Other"
	}
};
this.LootModes <- {
	FREE_FOR_ALL = 0,
	ROUND_ROBIN = 1,
	LOOT_MASTER = 2
};
this.LootFlags <- {
	NEED_B4_GREED = 1,
	MUNDANE = 2
};
this.VisibleWeaponSet <- {
	INVALID = -1,
	NONE = 0,
	MELEE = 1,
	RANGED = 2
};
this.AuthMethod <- {
	EXTERNAL = 0,
	DEV = 1,
	SERVICE = 2 
};
this.gMinCreatureSize <- 5.0;
this.gStopAutoAttackId <- 32759;
this.gMeleeAttackId <- 32766;
this.gRangedAttackId <- 32760;
this.gVendorMarkup <- 2.5;
