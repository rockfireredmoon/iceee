/* VIRTUAL ITEMS - Dynamically generated items that do not exist in the
default static item database.
*/

#include "VirtualItem.h"
#include "Util.h"
#include "StringList.h"
#include "DropTable.h"
#include "InstanceScale.h"
#include <string.h>
#include <algorithm>
#include "Debug.h"
#include "Config.h"
#include "Stats.h"
#include "DirectoryAccess.h"

const char *FLAVOR_TEXT_HEADER = "</b></i><font size=\"16\"><b>";
const char *FLAVOR_TEXT_FOOTER = "</b></font>";

ModManager g_ModManager;
EquipTemplateManager g_ModTemplateManager;
NameTemplateManager g_NameTemplateManager;

EquipAppearance g_EquipAppearance;
EquipAppearance g_EquipIconAppearance;

EquipTable g_EquipTable;
NameModManager g_NameModManager;
NameWeightManager g_NameWeightManager;
VirtualItemModSystem g_VirtualItemModSystem;

//Maps string identifiers for common "mEquipType" values to their exact values
//used in the client.
FileSymbol MapItemEquipType[] = {
	{ "NONE", ItemEquipType::NONE },
	{ "WEAPON_1H",  ItemEquipType::WEAPON_1H },
	{ "WEAPON_1H_UNIQUE",  ItemEquipType::WEAPON_1H_UNIQUE },
	{ "WEAPON_1H_MAIN",  ItemEquipType::WEAPON_1H_MAIN },
	{ "WEAPON_1H_OFF",  ItemEquipType::WEAPON_1H_OFF },
	{ "WEAPON_2H",  ItemEquipType::WEAPON_2H },
	{ "WEAPON_RANGED",  ItemEquipType::WEAPON_RANGED },
	{ "ARMOR_SHIELD",  ItemEquipType::ARMOR_SHIELD },
	{ "ARMOR_HEAD",  ItemEquipType::ARMOR_HEAD },
	{ "ARMOR_NECK",  ItemEquipType::ARMOR_NECK },
	{ "ARMOR_SHOULDER",  ItemEquipType::ARMOR_SHOULDER },
	{ "ARMOR_CHEST",  ItemEquipType::ARMOR_CHEST },
	{ "ARMOR_ARMS",  ItemEquipType::ARMOR_ARMS },
	{ "ARMOR_HANDS",  ItemEquipType::ARMOR_HANDS },
	{ "ARMOR_WAIST",  ItemEquipType::ARMOR_WAIST },
	{ "ARMOR_LEGS",  ItemEquipType::ARMOR_LEGS },
	{ "ARMOR_FEET",  ItemEquipType::ARMOR_FEET },
	{ "ARMOR_RING",  ItemEquipType::ARMOR_RING },
	{ "ARMOR_RING_UNIQUE",  ItemEquipType::ARMOR_RING_UNIQUE },
	{ "ARMOR_AMULET",  ItemEquipType::ARMOR_AMULET },
	{ "FOCUS_FIRE",  ItemEquipType::FOCUS_FIRE },
	{ "FOCUS_FROST",  ItemEquipType::FOCUS_FROST },
	{ "FOCUS_MYSTIC",  ItemEquipType::FOCUS_MYSTIC },
	{ "FOCUS_DEATH",  ItemEquipType::FOCUS_DEATH },
	{ "CONTAINER",  ItemEquipType::CONTAINER },
	{ "COSEMETIC_SHOULDER",  ItemEquipType::COSEMETIC_SHOULDER },
	{ "COSEMETIC_HIP",  ItemEquipType::COSEMETIC_HIP },
	{ "RED_CHARM",  ItemEquipType::RED_CHARM },
	{ "GREEN_CHARM",  ItemEquipType::GREEN_CHARM },
	{ "BLUE_CHARM",  ItemEquipType::BLUE_CHARM },
	{ "ORANGE_CHARM",  ItemEquipType::ORANGE_CHARM },
	{ "YELLOW_CHARM",  ItemEquipType::YELLOW_CHARM },
	{ "PURPLE_CHARM",  ItemEquipType::PURPLE_CHARM },
};
const int NumMapItemEquipType = COUNT_ARRAY_ELEMENTS(MapItemEquipType);

//Maps string identifiers for common "mWeaponType" values to their exact values
//used in the client.
FileSymbol MapWeaponType[] = {
	{ "NONE",         WeaponType::NONE },
	{ "SMALL",        WeaponType::SMALL },
	{ "ONE_HAND",     WeaponType::ONE_HAND },
	{ "TWO_HAND",     WeaponType::TWO_HAND },
	{ "POLE",         WeaponType::POLE },
	{ "WAND",         WeaponType::WAND },
	{ "BOW",          WeaponType::BOW },
	{ "THROWN",       WeaponType::THROWN },
	{ "ARCANE_TOTEM", WeaponType::ARCANE_TOTEM },
};
const int NumMapWeaponType = COUNT_ARRAY_ELEMENTS(MapWeaponType);

int ResolveFileSymbol(const char *name, FileSymbol *set, int maxcount)
{
	if(name == NULL)
		return 0;

	bool isNum = true;
	for(size_t i = 0; i < strlen(name); i++)
	{
		if(name[i] < '0' || name[i] > '9')
		{
			isNum = false;
			break;
		}
	}
	if(isNum == true)
		return atoi(name);

	for(int i = 0; i < maxcount; i++)
		if(strcmp(set[i].name, name) == 0)
			return set[i].value;

	g_Log.AddMessageFormat("[WARNING] ResolveFileSymbol not found: %s", name);
	return set[0].value;
}

VirtualItemSpawnParams :: VirtualItemSpawnParams()
{
	level = 0;
	rarity = 0;
	namedMob = false;

	dropRateProfile = NULL;

	mEquipType = 0;
	mWeaponType = 0;
	minimumQuality = -1;
	SetAllDropMult(1.0F);
}
void VirtualItemSpawnParams :: SetAllDropMult(float value)
{
	for(int i = 0; i < VirtualItemModSystem::MAX_QUALITY_LEVEL + 1; i++)
		dropMult[i] = value;
}
void VirtualItemSpawnParams :: ClampLimits(void)
{
	bool special = false;
	if(namedMob == true && g_Config.LootNamedMobSpecial == true)
		special = true;
	if(rarity >= g_Config.LootMinimumMobRaritySpecial)
		special = true;

	int levelLimit = 0;
	if(g_Config.LootMaxRandomizedLevel > 0)
		levelLimit = g_Config.LootMaxRandomizedLevel;

	if(special == true && g_Config.LootMaxRandomizedSpecialLevel > 0)
		levelLimit = g_Config.LootMaxRandomizedSpecialLevel;
	
	if(levelLimit != 0)
		level = Util::ClipInt(level, 1, levelLimit);
}

VirtualItemPage :: VirtualItemPage()
{
	Clear();
}

VirtualItemPage :: VirtualItemPage(int page)
{
	Clear();
	pageIndex = page;
}

void VirtualItemPage :: Clear(void)
{
	pageIndex = 0;
	itemList.clear();
	bPendingSave = false;
}

void VirtualItemPage :: AddVirtualItemDef(VirtualItemDef& vid)
{
	itemList.push_back(vid);
	bPendingSave = true;
}

void VirtualItemPage :: SaveToStream(FILE *output)
{
	for(size_t i = 0; i < itemList.size(); i++)
	{
		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "mID=%d\r\n", itemList[i].mID);
		fprintf(output, "mLevel=%d\r\n", itemList[i].mLevel);
		fprintf(output, "mQualityLevel=%d\r\n", itemList[i].mQualityLevel);
		fprintf(output, "mValue=%d\r\n", itemList[i].mValue);
		fprintf(output, "mType=%d\r\n", itemList[i].mType);
		fprintf(output, "mEquipType=%d\r\n", itemList[i].mEquipType);
		fprintf(output, "mWeaponType=%d\r\n", itemList[i].mWeaponType);
		fprintf(output, "mDisplayName=%s\r\n", itemList[i].mDisplayName.c_str());
		fprintf(output, "mIcon=%s\r\n", itemList[i].mIcon.c_str());
		fprintf(output, "mAppearance=%s\r\n", itemList[i].mAppearance.c_str());
		fprintf(output, "mods=%s\r\n", itemList[i].mModString.c_str());
		fprintf(output, "\r\n");
		/*
		fprintf(output, "%d#%d#%d#%d#%d#%d#%s\r\n",
		itemList[i].itemID,
		itemList[i].mLevel,
		itemList[i].mType,
		itemList[i].mEquipType,
		itemList[i].mWeaponType,
		itemList[i].mValue,
		itemList[i].mModString.c_str());
		*/
	}
}

void VirtualItemPage :: DeleteID(int ID)
{
	for(size_t i = 0; i < itemList.size(); i++)
	{
		if(itemList[i].mID == ID)
		{
			bPendingSave = true;
			itemList.erase(itemList.begin() + i);
			return;
		}
	}
}

VirtualItemDef :: VirtualItemDef()
{
	Clear();
}

void VirtualItemDef :: Clear(void)
{
	mID = 0;
	mType = 0;
	mEquipType = 0;
	mWeaponType = 0;
	mValue = 0;
	mLevel = 0;
	mQualityLevel = 0;
	mDisplayName.clear();
	mIcon.clear();
	mAppearance.clear();
	mModString.clear();
}


void VirtualItem :: UpdateArmorType(void)
{
	if(mStandardDef.mWeaponType == 0)
		mStandardDef.mArmorType = 1;
	
	//Hack, needed to get shields to display properly on a character's back.
	if(mStandardDef.mEquipType == ItemEquipType::ARMOR_SHIELD)
		mStandardDef.mArmorType = ArmorType::SHIELD;
}

void VirtualItem :: ApplyModString(void)
{
	STRINGLIST statpair;  //A single pair of [stat:amount]
	STRINGLIST statlist;  //A list of stat pairs broken from the main string.
	Util::Split(mModString, "&", statlist);
	for(size_t i = 0; i < statlist.size(); i++)
	{
		Util::Split(statlist[i], "=", statpair);
		if(statpair.size() >= 2)
		{
			if(statpair[0].compare("value") == 0)
				mStandardDef.mValue = atoi(statpair[1].c_str());
			else if(statpair[0].compare("wmin") == 0)
				mStandardDef.mWeaponDamageMin = atoi(statpair[1].c_str());
			else if(statpair[0].compare("wmax") == 0)
				mStandardDef.mWeaponDamageMax = atoi(statpair[1].c_str());
			ApplyStat(statpair[0], statpair[1]);
		}
	}
}

void VirtualItem :: GenerateFlavorText(void)
{
	char buffer[256];
	STRINGLIST modlist;
	STRINGLIST modpair;
	Util::Split(mModString, "&", modlist);
	for(size_t attrib = 0; attrib < modlist.size(); attrib++)
	{
		Util::Split(modlist[attrib], "=", modpair);
		if(modpair.size() < 2)
			continue;

		buffer[0] = 0;
		int value = atoi(modpair[1].c_str());
		if(modpair[0].compare("wmin") == 0) continue;
		if(modpair[0].compare("wmax") == 0) continue;
		if(modpair[0].compare("value") == 0) continue;
		if(modpair[0].compare("damage_resist_melee") == 0) continue;
		
		/*
		if(modpair[0].compare("strength") == 0) sprintf(buffer, "+%d Strength", value);
		else if(modpair[0].compare("dexterity") == 0) sprintf(buffer, "+%d Dexterity", value);
		else if(modpair[0].compare("constitution") == 0) sprintf(buffer, "+%d Constitution", value);
		else if(modpair[0].compare("psyche") == 0) sprintf(buffer, "+%d Psyche", value);
		else if(modpair[0].compare("spirit") == 0) sprintf(buffer, "+%d Spirit", value);
		*/
		
		if(modpair[0].compare("mod_melee_to_crit") == 0) sprintf(buffer, "+%g%% Physical Critical Chance", (float)value / 10.0F);
		else if(modpair[0].compare("mod_magic_to_crit") == 0) sprintf(buffer, "+%g%% Magic Critical Chance", (float)value / 10.0F);
		else if(modpair[0].compare("base_block") == 0) sprintf(buffer, "+%g%% Block Chance", (float)value / 10.0F);
		else if(modpair[0].compare("base_parry") == 0) sprintf(buffer, "+%g%% Parry Chance", (float)value / 10.0F);
		else if(modpair[0].compare("base_dodge") == 0) sprintf(buffer, "+%g%% Dodge Chance", (float)value / 10.0F);
		else if(modpair[0].compare("mod_movement") == 0) sprintf(buffer, "+%d%% Movement Speed", value);
		else if(modpair[0].compare("experience_gain_rate") == 0) sprintf(buffer, "+%d%% Experience Gain", value);
		else if(modpair[0].compare("melee_attack_speed") == 0) sprintf(buffer, "+%g%% Increased Attack Speed", (float)value / 10.0F);
		else if(modpair[0].compare("magic_attack_speed") == 0) sprintf(buffer, "+%g%% Increased Cast Rate", (float)value / 10.0F);
		else if(modpair[0].compare("dmg_mod_fire") == 0) sprintf(buffer, "+%g%% Fire Specialization", (float)value / 10.0F);
		else if(modpair[0].compare("dmg_mod_frost") == 0) sprintf(buffer, "+%g%% Frost Specialization", (float)value / 10.0F);
		else if(modpair[0].compare("dmg_mod_mystic") == 0) sprintf(buffer, "+%g%% Mystic Specialization", (float)value / 10.0F);
		else if(modpair[0].compare("dmg_mod_death") == 0) sprintf(buffer, "+%g%% Death Specialization", (float)value / 10.0F);
		else if(modpair[0].compare("base_healing") == 0) sprintf(buffer, "+%g%% Healing Specialization", (float)value / 10.0F);
		else if(modpair[0].compare("casting_setback_chance") == 0) sprintf(buffer, "%g%% Casting Setback Chance", (float)value / 10.0F);
		else if(modpair[0].compare("channeling_break_chance") == 0) sprintf(buffer, "%g%% Channel Break Chance", (float)value / 10.0F);
		else if(modpair[0].compare("mod_health_regen") == 0) sprintf(buffer, "+%d Hitpoint Regeneration", value);

		if(buffer[0] != 0)
		{
			if(mStandardDef.mFlavorText.size() > 0)
				mStandardDef.mFlavorText.append("<br>");
			mStandardDef.mFlavorText.append(buffer);
		}
	}
	mStandardDef.mFlavorText.insert(0, FLAVOR_TEXT_HEADER);
	mStandardDef.mFlavorText.append(FLAVOR_TEXT_FOOTER);
}

struct fieldCompare
{
	std::string name;
	float value;
	bool operator <(const fieldCompare& other) const
	{
		return value > other.value;
	}
};

void VirtualItem :: CheckNameMods(void)
{
	//Merge the instrinsic and bonus modifiers into one, to compare the stats.
	std::string output;
	MergeStats(mModString, output);

	std::vector<fieldCompare> values;

	STRINGLIST templist;
	STRINGLIST temppair;

	Util::Split(output, "&", templist);
	for(size_t i = 0; i < templist.size(); i++)
	{
		Util::Split(templist[i], "=", temppair);
		if(temppair.size() >= 2)
		{
			fieldCompare newField;
			newField.name = temppair[0];
			newField.value = (float)atof(temppair[1].c_str());
			values.push_back(newField);
		}
	}

	if(values.size() == 0)
		return;

	//Transform the values by applying the weight modifiers;
	for(size_t i = 0; i < values.size(); i++)
	{
		float mult = g_NameWeightManager.GetWeightForName(values[i].name.c_str());
		values[i].value *= mult;
	}

	//Sort highest to lowest
	std::sort(values.begin(), values.end());

	const char *first = NULL;
	const char *second = NULL;
	if(values.size() >= 1)
		first = values[0].name.c_str();
	if(values.size() >= 2)
		second = values[1].name.c_str();

	NameModEntry *firstMod = NULL;
	NameModEntry *secondMod = NULL;
	g_NameModManager.ResolveModifiers(first, second, &firstMod, &secondMod);
	ApplyNameMod(firstMod);
	ApplyNameMod(secondMod);
}

void VirtualItem :: ApplyNameMod(NameModEntry* mod)
{
	if(mod == NULL)
		return;
	std::string result;
	if(mod->mType == mod->TYPE_PREFIX)
	{
		result = mod->mName;
		result.append(" ");
		result.append(mStandardDef.mDisplayName);
	}
	else if(mod->mType == mod->TYPE_SUFFIX)
	{
		result = mStandardDef.mDisplayName;
		result.append(" ");
		result.append(mod->mName);
	}
	if(result.size() > 0)
		mStandardDef.mDisplayName = result;
}

const char* VirtualItem :: TransformStandardMappings(const char* search)
{
	struct Mapping
	{
		const char *modName;
		const char *itemDefStatName;
	};
	static Mapping mapping[10] =
	{
		{ "strength", "mBonusStrength" },
		{ "dexterity", "mBonusDexterity" },
		{ "constitution", "mBonusConstitution" },
		{ "psyche", "mBonusPsyche" },
		{ "spirit", "mBonusSpirit" },

		{ "damage_resist_melee", "mArmorResistMelee" },
		{ "damage_resist_fire", "mArmorResistFire" },
		{ "damage_resist_frost", "mArmorResistFrost" },
		{ "damage_resist_mystic", "mArmorResistMystic" },
		{ "damage_resist_death", "mArmorResistDeath" },
	};

	for(int i = 0; i < 10; i++)
		if(strcmp(mapping[i].modName, search) == 0)
			return mapping[i].itemDefStatName;

	return NULL;
}

void VirtualItem :: ApplyStat(std::string& key, std::string& value)
{
	const char *mappedStat = TransformStandardMappings(key.c_str());
	if(mappedStat != NULL)
	{
		SetItemProperty(&mStandardDef, mappedStat, value.c_str());
		return;
	}
}

int VirtualItem :: RollStats(std::string& output, int numPoints, int rarity)
{
	//Randomly distribute a number of points among the 5 core vital stats.

	std::vector<const char *> statID;
	statID.push_back("strength");
	statID.push_back("dexterity");
	statID.push_back("constitution");
	statID.push_back("psyche");
	statID.push_back("spirit");

	/*
	statID.push_back(STAT::STRENGTH);
	statID.push_back(STAT::DEXTERITY);
	statID.push_back(STAT::CONSTITUTION);
	statID.push_back(STAT::PSYCHE);
	statID.push_back(STAT::SPIRIT);
	*/

	int rollMin = g_VirtualItemModSystem.rarityConfig[rarity].GetMinRoll(numPoints);
	int rollMax = g_VirtualItemModSystem.rarityConfig[rarity].GetMaxRoll(numPoints);
	int maxIter = g_VirtualItemModSystem.rarityConfig[rarity].statMax;
	//g_Log.AddMessageFormat("Rarity: %d, Points: %d, min: %d, max: %d, iter: %d", rarity, numPoints, rollMin, rollMax, maxIter);

	int remain = numPoints;
	int iterCount = 0;
	while(remain > 0)
	{
		int allocPts = randint(rollMin, rollMax);
		if(allocPts > remain)
			allocPts = remain;

		iterCount++;
		if(iterCount >= maxIter)
			allocPts = remain;

		//g_Log.AddMessageFormat("  alloc: %d (remain: %d)", allocPts, remain);

		int allocSt = randint(0, statID.size() - 1);
		AppendStat(output, statID[allocSt], allocPts);
		statID.erase(statID.begin() + allocSt);
		remain -= allocPts;
	}
	return iterCount;
	/*
	for(int i = 0; i < iterations; i++)
	{
		int allocPts = numPoints;
		if(i < iterations - 1)
			allocPts = randint(1, numPoints);
		int allocSt = randint(0, statID.size() - 1);
		//int r = GetStatIndex(statID[allocSt]);
		AppendStat(output, statID[allocSt], allocPts);

		statID.erase(statID.begin() + allocSt);
		numPoints -= allocPts;
		if(numPoints == 0)
			break;
	}
	*/
}

void VirtualItem :: MergeStats(std::string& input, std::string& output)
{
	//Merge a list of new modifiers into the existing modifier list.
	//If a modifier already exists, add the new value into the tally.
	//Break a list of keys/values for easy search and manipulation,
	//then reconstruct the string after.
	STRINGLIST keys;
	STRINGLIST values;

	//Break up the output list so that new info can be merged into it.
	STRINGLIST templist;
	STRINGLIST temppair;
	Util::Split(output, "&", templist);
	for(size_t i = 0; i < templist.size(); i++)
	{
		Util::Split(templist[i], "=", temppair);
		if(temppair.size() >= 2)
		{
			keys.push_back(temppair[0]);
			values.push_back(temppair[1]);
		}
	}


	//Add or append the new modifiers
	char buffer[256];
	Util::Split(input, "&", templist);
	for(size_t i = 0; i < templist.size(); i++)
	{
		Util::Split(templist[i], "=", temppair);
		if(temppair.size() >= 2)
		{
			int fIndex = -1;
			for(size_t s = 0; s < keys.size(); s++)
			{
				if(keys[s].compare(temppair[0]) == 0)
				{
					fIndex = s;
					break;
				}
			}
			if(fIndex == -1)
			{
				keys.push_back(temppair[0]);
				values.push_back(temppair[1]);
			}
			else
			{
				int ivalue = atoi(values[fIndex].c_str());
				ivalue += atoi(temppair[1].c_str());
				sprintf(buffer, "%d", ivalue);
				values[fIndex] = buffer;
			}
		}
	}

	//Build the output string
	output.clear();
	for(size_t i = 0; i < keys.size(); i++)
	{
		if(i > 0)
			output.append("&");
		
		output.append(keys[i]);
		output.append("=");
		output.append(values[i]);
	}
}

bool VirtualItem :: isItemDefStat(int statID)
{
	//These are stats which are embedded within the ItemDef information and should not
	//be updated from the mod string.
	switch(statID)
	{
	case STAT::STRENGTH:
	case STAT::DEXTERITY:
	case STAT::CONSTITUTION:
	case STAT::PSYCHE:
	case STAT::SPIRIT:
	case STAT::DAMAGE_RESIST_MELEE:
	case STAT::DAMAGE_RESIST_FIRE:
	case STAT::DAMAGE_RESIST_FROST:
	case STAT::DAMAGE_RESIST_MYSTIC:
	case STAT::DAMAGE_RESIST_DEATH:
		return true;
	}
	return false;
}


std::string VirtualItem :: GetAppearanceAsset(std::string& mAppearance)
{
	std::string ret;
	size_t pos = mAppearance.find("[\"type\"]=\"");
	if(pos != string::npos)
	{
		pos += 10;
		size_t epos = mAppearance.find("\"", pos);
		ret = mAppearance.substr(pos, epos - pos);
	}
	return ret;
}

void VirtualItem :: InstantiateNew(int ID, int level, int rarity, int equipType, int weaponType)
{
	EquipTemplate* eqTemplate = g_ModTemplateManager.GetTemplate(equipType, weaponType);
	if(eqTemplate == NULL)
	{
		g_Log.AddMessageFormat("[WARNING] ItemMod template not found for equipType:%d, weaponType:%d", equipType, weaponType);
		return;
	}

	ApplyIntrinsicTables(eqTemplate, level, rarity);
	ApplyRandomTables(eqTemplate, level, rarity);

	if(weaponType > 0)
		mStandardDef.mType = ItemType::WEAPON;
	else
		mStandardDef.mType = ItemType::ARMOR;

	mStandardDef.mID = ID;
	mStandardDef.mLevel = level;
	mStandardDef.mMinUseLevel = level;
	mStandardDef.mQualityLevel = rarity;
	mStandardDef.mEquipType = equipType;

	if(weaponType != 0)
		mStandardDef.mWeaponType = weaponType;

	UpdateArmorType();
	
	EquipAppearance::SEARCHRESULT search;
	const char* resultStr = NULL;
	g_EquipAppearance.SearchEntries(equipType, weaponType, search);
	resultStr = g_EquipAppearance.GetRandomString(rarity, search);
	if(resultStr == NULL)
	{
		mStandardDef.mAppearance.clear();
		g_Log.AddMessageFormat("%d:NOT FOUND (eq:%d, weap:%d, rarity:%d)", ID, equipType, weaponType, rarity);
	}
	else
	{
		mStandardDef.mAppearance = resultStr;
		//g_Log.AddMessageFormat("%d:%s", ID, resultStr);
	}

	search.clear();
	std::string asset = GetAppearanceAsset(mStandardDef.mAppearance);
	//g_EquipIconAppearance.SearchEntries(equipType, weaponType, search);
	g_EquipIconAppearance.SearchEntriesWithAsset(equipType, weaponType, asset.c_str(), search);
	resultStr = g_EquipIconAppearance.GetRandomString(rarity, search);
	if(resultStr == NULL)
	{
		mStandardDef.mIcon = "Icon/QuestionMark|Icon-32-BG-Black.png";
		g_Log.AddMessageFormat("%d: ICON NOT FOUND (eq:%d, weap:%d, rarity:%d) (%s)", ID, equipType, weaponType, rarity, asset.c_str());
	}
	else
	{
		mStandardDef.mIcon = resultStr;
		if(mStandardDef.mIcon.size() > 0)
		{
			switch(rarity)
			{
			case 2: mStandardDef.mIcon.append("|Icon-32-BG-Green.png"); break;
			case 3: mStandardDef.mIcon.append("|Icon-32-BG-Aqua.png"); break;
			case 4: mStandardDef.mIcon.append("|Icon-32-BG-Purple.png"); break;
			case 5: mStandardDef.mIcon.append("|Icon-32-BG-Yellow.png"); break;
			case 6: mStandardDef.mIcon.append("|Icon-32-BG-Red.png"); break;
			default: mStandardDef.mIcon.append("|Icon-32-BG-Black.png"); break;
			}
		}
	}

	if(asset.size() == 0)
		g_Log.AddMessageFormat("ID:%d  Asset name not found: %s\n", ID, mStandardDef.mAppearance.c_str());
	mStandardDef.mDisplayName = g_NameTemplateManager.RetrieveName(equipType, weaponType, asset.c_str()); 

	ApplyModString();
	CheckNameMods();
	GenerateFlavorText();

	VirtualItemDef vid;
	vid.mID = ID;
	vid.mLevel = mStandardDef.mLevel;
	vid.mQualityLevel = mStandardDef.mQualityLevel;
	vid.mEquipType = mStandardDef.mEquipType;
	vid.mWeaponType = mStandardDef.mWeaponType;
	vid.mValue = mStandardDef.mValue;
	vid.mType = mStandardDef.mType;
	vid.mDisplayName = mStandardDef.mDisplayName;
	vid.mIcon = mStandardDef.mIcon;
	vid.mAppearance = mStandardDef.mAppearance;
	vid.mModString = mModString;
	g_ItemManager.AddVirtualItemDef(vid);

	/*
	//Called to fill out this VirtualItem with a completely new, randomized item with
	//the core parameters required to generate a new item.
	ModTable *modTable = g_ModManager.GetModTable("core");
	if(modTable == NULL)
	{
		g_Log.AddMessageFormat("Could not find 'core' mod table.");
		return;
	}

	ModRow *modRow = modTable->GetRowByLevel(level);
	if(modRow == NULL)
	{
		g_Log.AddMessageFormat("Could not find level:%d in core mod table", level);
		return;
	}

	int value = modRow->FetchData(rarity);
	if(value == 0)
	{
		g_Log.AddMessageFormat("Zero value returned in core mod table for level:%d, rarity:%d", level, rarity);
		return;
	}

	mStandardDef.mType = 3;
	mStandardDef.mID = ID;
	mStandardDef.mLevel = level;
	mStandardDef.mMinUseLevel = level;
	mStandardDef.mQualityLevel = rarity;
	mStandardDef.mEquipType = equipType;
	if(weaponType != 0)
		mStandardDef.mWeaponType = weaponType;
	else
		mStandardDef.mArmorType = 1;
	
	mStandardDef.mDisplayName = "TEST ITEM";
	mStandardDef.mValue = 123456;

	std::string rolledMods; 
	RollStats(rolledMods, value, 3);
	MergeStats(rolledMods);
	ApplyModString();
	g_Log.AddMessageFormat("Rolled item: %d", ID);
	*/
}

void VirtualItem :: ApplyIntrinsicTables(EquipTemplate* eqTemplate, int level, int rarity)
{
	for(size_t i = 0; i < eqTemplate->intrinsicTable.size(); i++)
		ApplyModTable(&eqTemplate->intrinsicTable[i], level, rarity);
}

struct UsedTable
{
	ValidTable* table;
	int usedCount;
};

int IncrementTable(std::vector<UsedTable>& tableList, int index, int maxShares)
{
	if(index < 0 || index >= (int)tableList.size())
		return maxShares;

	tableList[index].usedCount++;
	if(tableList[index].usedCount >= tableList[index].table->maxApply)
	{
		maxShares -= tableList[index].table->shares;
		tableList.erase(tableList.begin() + index);
	}
	return maxShares;
}

int GetRandomTableIndex(std::vector<UsedTable>& tableList, int maxShares)
{
	//Search for any core tables
	for(size_t i = 0; i < tableList.size(); i++)
		if(tableList[i].table->core == true)
			return i;

	int base = 1;
	int rnd = randint(1, maxShares);
	for(size_t i = 0; i < tableList.size(); i++)
	{
		if(rnd >= base && rnd <= base + tableList[i].table->shares - 1)
			return i;
		base += tableList[i].table->shares;
	}
	return -1;
}

void VirtualItem :: ApplyRandomTables(EquipTemplate* eqTemplate, int level, int rarity)
{
	std::vector<UsedTable> tableList;

	UsedTable newTable;
	int maxShares = 0;
	for(size_t i = 0; i < eqTemplate->randomTable.size(); i++)
	{
		newTable.table = &eqTemplate->randomTable[i];
		newTable.usedCount = 0;
		tableList.push_back(newTable);

		maxShares += eqTemplate->randomTable[i].shares;
	}

	int maxMods = g_VirtualItemModSystem.rarityConfig[rarity].modMax;

	int bonus = (int)(g_VirtualItemModSystem.rarityConfig[rarity].levelBonus * level);
	while(bonus > 0)
	{
		int rnd = randint(1, 100);
		if(rnd <= bonus)
			maxMods++;
		bonus -= 100;
	}

	int nullMod = g_VirtualItemModSystem.rarityConfig[rarity].nullMod;

	int curMods = 0;
	
	//Apply modifiers that should always spawn.
	for(size_t i = 0; i < eqTemplate->alwaysTable.size(); i++)
		curMods += ApplyModTable(&eqTemplate->alwaysTable[i], level, rarity);

	while(curMods < maxMods && tableList.size() > 0)
	{
		if(nullMod > 0)
		{
			int rnd = randint(1, 100);
			if(rnd <= nullMod)
			{
				curMods++;
				continue;
			}
		}
		int index = GetRandomTableIndex(tableList, maxShares);
		if(index == -1)
			break;

		curMods += ApplyModTable(tableList[index].table, level, rarity);
		maxShares = IncrementTable(tableList, index, maxShares);
	}
}

int VirtualItem :: ApplyModTable(ValidTable* validTable, int level, int rarity)
{
	//Return the number of mods successfully added

	ModTable *modTable = g_ModManager.GetModTable(validTable->tableName);
	if(modTable == NULL)
	{
		g_Log.AddMessageFormat("Could not find mod table [%s]", validTable->tableName);
		return 0;
	}

	ModRow *modRow = modTable->GetRowByLevel(level);
	if(modRow == NULL)
	{
		g_Log.AddMessageFormat("Could not find level:%d in table [%s]", level, validTable->tableName);
		return 0;
	}

	int value = modRow->FetchData(rarity);
	if(value == 0)
	{
		g_Log.AddMessageFormat("Zero value returned in table [%s] for level:%d, rarity:%d", validTable->tableName, level, rarity);
		return 0;
	}

	int retval = 1;
	std::string rolledMods = mModString; 
	if(validTable->core == true)
	{
		//g_Log.AddMessageFormat("Distributing %d core stat points", value);
		value = g_VirtualItemModSystem.rarityConfig[rarity].GetAdjustedCorePoints(value);
		retval = RollStats(rolledMods, value, rarity);
	}
	else
	{
		//g_Log.AddMessageFormat("Applying table:%s [%s]", validTable->tableName, validTable->statApp);

		STRINGLIST results;
		std::string app = validTable->statApp;
		Util::Split(app, "|", results);
		if(results.size() == 1)
			AppendStat(rolledMods, results[0].c_str(), value);
		else if(results.size() == 2)
		{
			AppendStat(rolledMods, results[0].c_str(), modRow->FetchFirst(rarity));
			AppendStat(rolledMods, results[1].c_str(), modRow->FetchSecond(rarity));
		}
	}
	mModString = rolledMods;
	return retval;
}

void VirtualItem :: AppendStat(std::string& output, const char *name, int value)
{
	char buffer[32];
	if(output.size() > 0)
		output.append("&");

	output.append(name);
	output.append("=");
	sprintf(buffer, "%d", value);
	output.append(buffer);
}

int ItemManager :: RollVirtualItem(VirtualItemSpawnParams &viParams)
{
	//Determine if an item should drop.
	int rarity = g_VirtualItemModSystem.GetDropRarity(viParams);
	if(viParams.minimumQuality > 0 && rarity < viParams.minimumQuality)
		rarity = viParams.minimumQuality;

	/* DEBUG PURPOSES ONLY
	Sleep(1);
#ifndef WINDOWS_PLATFORM
#error WINDOWS DEBUG ONLY
#endif
	rarity = 4;
	Sleep(1);
	*/

	if(rarity < 0)
		return -1;

	//If the equipment type has not been explicitly chosen (eg: gambled item) then select
	//one from the equipment randomizer table.
	if(viParams.mEquipType == 0)
	{
		VirtualItemSpawnType *equipTable = g_EquipTable.GetRandomEntry();
		if(equipTable == NULL)
		{
			g_Log.AddMessageFormat("[ERROR] RollVirtualItem could not retrieve randomized item");
			return -1;
		}
		viParams.mEquipType = equipTable->mEquipType;
		viParams.mWeaponType = equipTable->mWeaponType;
	}

	return CreateNewVirtualItem(rarity, viParams);
}

void ItemManager :: NotifyDestroy(int itemID, const char *debugReason)
{
	//This function deletes a virtual item from the storage pages.
	if(itemID < BASE_VIRTUAL_ITEM_ID)
		return;

	VITEM_CONT::iterator it = VItemList.find(itemID);
	if(it == VItemList.end())
		return;

	int page = GetVirtualItemPage(itemID);
	VIRTUALITEMPAGE::iterator pageit;
	pageit = virtualItemPage.find(page);
	if(pageit != virtualItemPage.end())
		pageit->second.DeleteID(itemID);

	VItemList.erase(it);
	g_Log.AddMessageFormat("Destroyed virtual item: %d (%s)", itemID, debugReason);
	Debug::Log("Destroyed virtual item: %d (%s)", itemID, debugReason);
	SessionVarsChangeData.AddChange();
}

int ItemManager :: GetNewVirtualItemID(void)
{
	int ID = nextVirtualItemID++;

	//IMPORTANT: need to load an existing page BEFORE adding a new item.
	//Otherwise earlier entries will never load into the server.  When the
	//server next autosaves, it won't have that information to save back out,
	//effectively erasing the old data.

	int page = GetVirtualItemPage(ID);
	RequestVirtualItemPagePtr(page);

	return ID;
}

int ItemManager :: CreateNewVirtualItem(int rarity, VirtualItemSpawnParams &viParams)
{
	SessionVarsChangeData.AddChange();
	int ID = GetNewVirtualItemID();

	VirtualItem newItem;
	newItem.InstantiateNew(ID, viParams.level, rarity, viParams.mEquipType, viParams.mWeaponType);
	
	VItemList.insert(VItemList.end(), std::pair<int, VirtualItem>(ID, newItem));
	//g_Log.AddMessageFormat("Created: %d (new size: %d)", ID, VItemList.size());
	return ID;
}

int ItemManager :: GetVirtualItemPage(int itemID)
{
	return (itemID - BASE_VIRTUAL_ITEM_ID) / VirtualItemPage::ITEMS_PER_PAGE;
}

void ItemManager :: AddVirtualItemDef(VirtualItemDef& vid)
{
	int page = GetVirtualItemPage(vid.mID);

	VIRTUALITEMPAGE::iterator it;
	it = virtualItemPage.find(page);
	if(it == virtualItemPage.end())
		it = virtualItemPage.insert(virtualItemPage.end(), VIRTUALITEMPAGEPAIR(page, VirtualItemPage(page)));
	it->second.AddVirtualItemDef(vid);
}

void ItemManager :: CheckVirtualItemAutosave(bool force)
{
	if(g_ServerTime < nextVirtualItemAutosave && force == false)
		return;

	char filename[256];
	VIRTUALITEMPAGE::iterator it;
	it = virtualItemPage.begin();
	while(it != virtualItemPage.end())
	{
		bool del = false;
		if(it->second.bPendingSave == true)
		{
			Util::SafeFormat(filename, sizeof(filename), "VirtualItems\\%08d.txt", it->second.pageIndex);
			Platform::FixPaths(filename);
			if(it->second.itemList.size() > 0)
			{
				FILE *output = fopen(filename, "wb");
				if(output != NULL)
				{
					it->second.SaveToStream(output);
					fclose(output);
					it->second.bPendingSave = false;
				}
				else
					g_Log.AddMessageFormat("[ERROR] Failed to open VirtualItem page: %s", filename);
			}
			else
			{
				remove(filename);
				g_Log.AddMessageFormat("Removing item page [%s]", filename);
				del = true;
			}
		}
		if(del == true)
			virtualItemPage.erase(it++);
		else
			++it;
	}
	nextVirtualItemAutosave = g_ServerTime + VirtualItemPage::AUTOSAVE_TIMER;
}

VirtualItemPage* ItemManager :: RequestVirtualItemPagePtr(int page)
{
	//Load the page if it doesn't exist.
	//Return a pointer to the page.
	VIRTUALITEMPAGE::iterator it = virtualItemPage.find(page);
	if(it == virtualItemPage.end())
	{
		it = virtualItemPage.insert(virtualItemPage.end(), VIRTUALITEMPAGEPAIR(page, VirtualItemPage(page)));
		if(it != virtualItemPage.end())
		{
			LoadVirtualItemPage(&it->second);
			return &it->second;
		}
		g_Log.AddMessageFormat("[CRITICAL] ItemManager::GetVirtualItemPagePtr could not insert page.");
		return NULL;
	}
	return &it->second;
}

void ItemManager :: LoadVirtualItemWithID(int itemID)
{
	int page = GetVirtualItemPage(itemID);
	VirtualItemPage *pagePtr = RequestVirtualItemPagePtr(page);
	if(pagePtr == NULL)
		return;

	for(size_t i = 0; i < pagePtr->itemList.size(); i++)
	{
		if(pagePtr->itemList[i].mID == itemID)
		{
			ExpandVirtualItem(pagePtr->itemList[i]);
			return;
		}
	}
}

void ItemManager :: LoadVirtualItemPage(VirtualItemPage* targetPage)
{
	char filename[256];
	Util::SafeFormat(filename, sizeof(filename), "VirtualItems\\%08d.txt", targetPage->pageIndex);
	Platform::FixPaths(filename);
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open VirtualItem page [%s]", filename);
		return;
	}

	lfr.CommentStyle = Comment_Semi;

	VirtualItemDef newItem;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.mID != 0)
					targetPage->itemList.push_back(newItem);
				newItem.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "mID") == 0)
				newItem.mID = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mLevel") == 0)
				newItem.mLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mQualityLevel") == 0)
				newItem.mQualityLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mValue") == 0)
				newItem.mValue = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mType") == 0)
				newItem.mType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mEquipType") == 0)
				newItem.mEquipType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mWeaponType") == 0)
				newItem.mWeaponType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mDisplayName") == 0)
				newItem.mDisplayName = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "mIcon") == 0)
				newItem.mIcon = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "mAppearance") == 0)
				newItem.mAppearance = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "mods") == 0)
				newItem.mModString = lfr.BlockToStringC(1, 0);
		}
	}
	if(newItem.mID != 0)
		targetPage->itemList.push_back(newItem);

	lfr.CloseCurrent();
}

void ItemManager :: ExpandVirtualItem(const VirtualItemDef& item)
{
	VirtualItem newItem;
	newItem.mStandardDef.Reset();
	newItem.mStandardDef.mID = item.mID;
	newItem.mStandardDef.mType = item.mType;
	newItem.mStandardDef.mLevel = item.mLevel;
	newItem.mStandardDef.mMinUseLevel = item.mLevel;
	newItem.mStandardDef.mQualityLevel = item.mQualityLevel;
	newItem.mStandardDef.mMinUseLevel = item.mLevel;
	newItem.mStandardDef.mEquipType = item.mEquipType;
	newItem.mStandardDef.mWeaponType = item.mWeaponType;
	newItem.mStandardDef.mDisplayName = item.mDisplayName;
	newItem.mStandardDef.mIcon = item.mIcon;
	newItem.mStandardDef.mAppearance = item.mAppearance;

	newItem.mModString = item.mModString;
	newItem.UpdateArmorType();
	newItem.ApplyModString();
	newItem.GenerateFlavorText();
	InsertVirtualItem(newItem);
}

void ItemManager :: InsertVirtualItem(const VirtualItem& item)
{
	VItemList.insert(VItemList.end(), VITEM_PAIR(item.mStandardDef.mID, item));
}

ModRow :: ModRow()
{
	Clear();
}

void ModRow :: Clear(void)
{
	memset(this, 0, sizeof(ModRow));
}

int ModRow :: FetchData(int rarity)
{
	//Rarity begins at uncommon (2)
	if(rarity < LOWEST_RARITY)
		rarity = LOWEST_RARITY;
	rarity -= LOWEST_RARITY;
	if(rarity < 0)
		return 0;
	if(rarity >= NUMRARITY)
		return 0;

	//Adding this because some stats like break chance use negative values.
	//Signs in the table were swapped, but this meant that ranges like (-10 to -25)
	//May not be calculated correctly since the "max" is technically lower than the min.
	int min = data[rarity][0];
	int max = data[rarity][1];
	if(max < min)
	{
		int temp = min;
		min = max;
		max = temp;
	}
	return randint(min, max);
}

int ModRow :: FetchFirst(int rarity)
{
	if(rarity < LOWEST_RARITY)
		rarity = LOWEST_RARITY;
	rarity -= LOWEST_RARITY;
	if(rarity < 0)
		return 0;
	if(rarity >= NUMRARITY)
		return 0;
	return data[rarity][0];
}

int ModRow :: FetchSecond(int rarity)
{
	if(rarity < LOWEST_RARITY)
		rarity = LOWEST_RARITY;
	rarity -= LOWEST_RARITY;
	if(rarity < 0)
		return 0;
	if(rarity >= NUMRARITY)
		return 0;
	return data[rarity][1];
}

ModTable :: ModTable()
{
}

ModTable :: ~ModTable()
{
	modRow.clear();
}

void ModTable :: Clear(void)
{
	name.clear();
	modRow.clear();
}

void ModTable :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not load ModTable file: %s", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	ModRow rowData;
	bool header = false;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		if(header == true)
		{
			int r = lfr.MultiBreak(",");
			if(r > 0)
			{
				rowData.level = lfr.BlockToIntC(0);
				int fcol = 0;
				for(int rarity = 0; rarity < ModRow::NUMRARITY; rarity++)
					for(int col = 0; col < ModRow::NUMCOLUMN; col++)
						rowData.data[rarity][col] = lfr.BlockToIntC(1 + fcol++);
				modRow.push_back(rowData);
			}
		}
		else
			header = true;
	}
	lfr.CloseCurrent();
}

ModRow* ModTable :: GetRowByLevel(int level)
{
	for(size_t i = 0; i < modRow.size(); i++)
		if(modRow[i].level == level)
			return &modRow[i];
	return NULL;
}

void ModManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not load ModTable list file: %s", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	ModTable newItem;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r >= 2)
		{
			newItem.name = lfr.BlockToStringC(0, 0);
			lfr.BlockToStringC(1, 0);
			Platform::FixPaths(lfr.SecBuffer);
			newItem.LoadFromFile(lfr.SecBuffer);
			modTable.push_back(newItem);
			newItem.Clear();
		}
	}
	lfr.CloseCurrent();
}

ModTable* ModManager :: GetModTable(const char *name)
{
	for(size_t i = 0; i < modTable.size(); i++)
		if(modTable[i].name.compare(name) == 0)
			return &modTable[i];
	return NULL;
}





ValidTable :: ValidTable()
{
	Clear();
}

void ValidTable :: Clear(void)
{
	memset(tableName, 0, sizeof(tableName));
	memset(statApp, 0, sizeof(statApp));
	shares = 0;
	core = false;
	maxApply = 0;
}

void ValidTable :: FixDefaults(void)
{
	if(shares < 1)
		shares = 1;
	if(maxApply < 1)
		maxApply = 1;
}

EquipTemplate :: EquipTemplate()
{
	Clear();
}

void EquipTemplate :: Clear(void)
{
	mEquipType = 0;
	mWeaponType = 0;
	intrinsicTable.clear();
	alwaysTable.clear();
	randomTable.clear();
}

void EquipTemplateManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Error opening Mod Template file: %s", filename);
		return;
	}

	EquipTemplate newItem;
	lfr.CommentStyle = Comment_Semi;
	int mod = 0;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				if(newItem.mEquipType != 0)
					equipTemplate.push_back(newItem);
				newItem.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "mEquipType") == 0)
				newItem.mEquipType = ResolveFileSymbol(lfr.BlockToStringC(1, Case_Upper), MapItemEquipType, NumMapItemEquipType);
			else if(strcmp(lfr.SecBuffer, "mWeaponType") == 0)
				newItem.mWeaponType = ResolveFileSymbol(lfr.BlockToStringC(1, Case_Upper), MapWeaponType, NumMapWeaponType);
			else if(strcmp(lfr.SecBuffer, "intrinsic") == 0)
				mod = 1;
			else if(strcmp(lfr.SecBuffer, "always") == 0)
				mod = 2;
			else if(strcmp(lfr.SecBuffer, "mod") == 0)
				mod = 3;
			if(mod > 0)
			{
				ValidTable newTable;
				Util::SafeCopy(newTable.statApp, lfr.BlockToStringC(1, 0), sizeof(newTable.statApp));
				Util::SafeCopy(newTable.tableName, lfr.BlockToStringC(2, 0), sizeof(newTable.tableName));
				newTable.shares = lfr.BlockToIntC(3);
				newTable.core = lfr.BlockToBoolC(4);
				newTable.maxApply = lfr.BlockToIntC(5);
				newTable.FixDefaults();
				if(mod == 1) newItem.intrinsicTable.push_back(newTable);
				else if(mod == 2) newItem.alwaysTable.push_back(newTable);
				else if(mod == 3) newItem.randomTable.push_back(newTable);
				mod = 0;
			}
		}
	}
	if(newItem.mEquipType != 0)
		equipTemplate.push_back(newItem);

	lfr.CloseCurrent();
};

EquipTemplate* EquipTemplateManager :: GetTemplate(int equipType, int weaponType)
{
	for(size_t i = 0; i < equipTemplate.size(); i++)
		if(equipType == equipTemplate[i].mEquipType && weaponType == equipTemplate[i].mWeaponType)
			return &equipTemplate[i];

	return NULL;
}



EquipAppearanceKey :: EquipAppearanceKey()
{
	Clear();
}

void EquipAppearanceKey :: Clear(void)
{
	mEquipType = 0;
	mWeaponType = 0;
	mQualityLevel = 0;
	asset.clear();
	dataList.clear();
}

bool EquipAppearanceKey :: isEqual(const EquipAppearanceKey& other) const
{
	if(mEquipType != other.mEquipType)
		return false;
	if(mWeaponType != other.mWeaponType)
		return false;
	if(mQualityLevel != other.mQualityLevel)
		return false;
	if(asset.compare(other.asset) != 0)
		return false;
	return true;
}

EquipAppearance :: EquipAppearance()
{
}

EquipAppearance :: ~EquipAppearance()
{
}

void EquipAppearance :: AddIfValid(EquipAppearanceKey& data)
{
	if(data.mEquipType == 0)
		return;
	if(data.dataList.size() == 0)
		return;
	dataEntry.push_back(data);
}

void EquipAppearance :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open EquipAppearance file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	EquipAppearanceKey entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				AddIfValid(entry);
				entry.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "app") == 0)
				entry.dataList.push_back(lfr.BlockToStringC(1, 0));
			else if(strcmp(lfr.SecBuffer, "mEquipType") == 0)
				entry.mEquipType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mWeaponType") == 0)
				entry.mWeaponType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mQualityLevel") == 0)
				entry.mQualityLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "asset") == 0)
				entry.asset = lfr.BlockToStringC(1, 0);
			else
				g_Log.AddMessageFormat("[WARNING] Unknown identifier [%s] in file [%s] on line [%d]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	AddIfValid(entry);
	lfr.CloseCurrent();
}

void EquipAppearance :: DebugSaveToFile(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
		return;

	std::sort(dataEntry.begin(), dataEntry.end());
	for(size_t i = 0; i < dataEntry.size(); i++)
	{
		fprintf(output, "[ENTRY]\r\n");
		if(dataEntry[i].asset.size() > 0)
			fprintf(output, "asset=%s\r\n", dataEntry[i].asset.c_str());

		fprintf(output, "mEquipType=%d\r\n", dataEntry[i].mEquipType);
		fprintf(output, "mWeaponType=%d\r\n", dataEntry[i].mWeaponType);
		if(dataEntry[i].mQualityLevel != 0)
			fprintf(output, "mQualityLevel=%d\r\n", dataEntry[i].mQualityLevel);
		std::sort(dataEntry[i].dataList.begin(), dataEntry[i].dataList.end());
		for(size_t c = 0; c < dataEntry[i].dataList.size(); c++)
			fprintf(output, "app=%s\r\n", dataEntry[i].dataList[c].c_str());
		fprintf(output, "\r\n");
	}
	fclose(output);
}

EquipAppearanceKey* EquipAppearance :: GetOrCreate(EquipAppearanceKey& object)
{
	for(size_t i = 0; i < dataEntry.size(); i++)
		if(dataEntry[i].isEqual(object) == true)
			return &dataEntry[i];

	dataEntry.push_back(object);
	return &dataEntry.back();
}

void EquipAppearance :: SearchEntries(int equipType, int weaponType, SEARCHRESULT& results)
{
	for(size_t i = 0; i < dataEntry.size(); i++)
	{
		if(dataEntry[i].mEquipType != equipType)
			continue;
		if(dataEntry[i].mWeaponType != weaponType)
			continue;
		results.push_back(&dataEntry[i]);
	}
}

void EquipAppearance :: SearchEntriesWithAsset(int equipType, int weaponType, const char *asset, SEARCHRESULT& results)
{
	for(size_t i = 0; i < dataEntry.size(); i++)
	{
		if(dataEntry[i].mEquipType != equipType)
			continue;
		if(dataEntry[i].mWeaponType != weaponType)
			continue;
		if(dataEntry[i].asset.compare(asset) != 0)
			continue;
		results.push_back(&dataEntry[i]);
	}
}

const char* EquipAppearance :: GetRandomString(int preferredRarity, SEARCHRESULT& results)
{
	int rarity = preferredRarity;
	while(rarity >= 0)
	{
		for(size_t i = 0; i < results.size(); i++)
		{
			if(results[i]->mQualityLevel != rarity)
				continue;
			if(results[i]->dataList.size() == 0)
				continue;
			int r = randint(0, results[i]->dataList.size() - 1);
			return results[i]->dataList[r].c_str();
		}
		rarity--;
	}
	return NULL;
}

void EquipAppearance :: Debug_CheckForNames(void)
{
	for(size_t i = 0; i < dataEntry.size(); i++)
	{
		for(size_t a = 0; a < dataEntry[i].dataList.size(); a++)
		{
			std::string app = dataEntry[i].dataList[a];
			app = VirtualItem::GetAppearanceAsset(app);
			if(g_NameTemplateManager.Debug_HasName(app.c_str(), dataEntry[i].mEquipType, dataEntry[i].mWeaponType) == false)
				g_Log.AddMessageFormat("NAME Not found: %s (mEquipType=%d, mWeaponType=%d)", app.c_str(), dataEntry[i].mEquipType, dataEntry[i].mWeaponType);

			if(g_EquipIconAppearance.Debug_HasName(app.c_str(), dataEntry[i].mEquipType, dataEntry[i].mWeaponType) == false)
				g_Log.AddMessageFormat("ICON Not found: %s (mEquipType=%d, mWeaponType=%d)", app.c_str(), dataEntry[i].mEquipType, dataEntry[i].mWeaponType);
		}
	}
}

bool EquipAppearance :: Debug_HasName(const char* asset, int mEquipType, int mWeaponType)
{
	for(size_t i = 0; i < dataEntry.size(); i++)
	{
		if(dataEntry[i].mEquipType != mEquipType)
			continue;
		if(dataEntry[i].mWeaponType != mWeaponType)
			continue;
		if(dataEntry[i].asset.compare(asset) == 0)
			return true;
	}
	return false;
}

VirtualItemSpawnType :: VirtualItemSpawnType()
{
	Clear();
}

void VirtualItemSpawnType :: Clear(void)
{
	mEquipType = 0;
	mWeaponType = 0;
	shares = 0;
}

EquipTable :: EquipTable()
{
	maxShares = 0;
}

void EquipTable :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open EquipTable file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	VirtualItemSpawnType newItem;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil(",=", '=');
		if(r >= 3)
		{
			int res;
			lfr.BlockToStringC(0, Case_Upper);
			res = ResolveFileSymbol(lfr.SecBuffer, MapItemEquipType, NumMapItemEquipType);
			newItem.mEquipType = res;

			lfr.BlockToStringC(1, Case_Upper);
			res = ResolveFileSymbol(lfr.SecBuffer, MapWeaponType, NumMapWeaponType);
			newItem.mWeaponType = res;

			newItem.shares = lfr.BlockToIntC(2);

			if(newItem.mEquipType > 0)
				equipList.push_back(newItem);
			newItem.Clear();
		}
	}
	lfr.CloseCurrent();

	TallyMaxShares();
}

void EquipTable :: TallyMaxShares(void)
{
	maxShares = 0;
	for(size_t i = 0; i < equipList.size(); i++)
		maxShares += equipList[i].shares;
}

VirtualItemSpawnType* EquipTable :: GetRandomEntry(void)
{
	int base = 1;
	int rnd = randint(1, maxShares);
	for(size_t i = 0; i < equipList.size(); i++)
	{
		if(rnd >= base && rnd <= base + equipList[i].shares - 1)
			return &equipList[i];
		base += equipList[i].shares;
	}
	return NULL;
}


NameEntry :: NameEntry()
{
	Clear();
}

void NameEntry :: Clear(void)
{
	memset(name, 0, sizeof(name));
	shares = 0;
}

NameTemplate :: NameTemplate()
{
	Clear();
}

const char NameTemplate::DEFAULT_NAME[] = "UNKNOWN NAME";

void NameTemplate :: Clear(void)
{
	assetName.clear();
	mEquipType = 0;
	mWeaponType = 0;
	nameList.clear();
	maxShares = 0;
}

void NameTemplate :: CountShareTotal(void)
{
	maxShares = 0;
	for(size_t i = 0; i < nameList.size(); i++)
		maxShares += nameList[i].shares;
}

const char* NameTemplate :: GetRandomName(void)
{
	int base = 1;
	int r = randint(1, maxShares);
	for(size_t i = 0; i < nameList.size(); i++)
	{
		if(r >= base && r <= base + nameList[i].shares - 1)
			return nameList[i].name;
		base += nameList[i].shares;
	}
	g_Log.AddMessageFormat("[DEBUG] GetRandomName() unable to resolve name for template: mEquipType:%d, mWeaponType:%d, asset:%s", mEquipType, mWeaponType, assetName.c_str());
	return NULL;
}

NameTemplateManager :: NameTemplateManager()
{
}

NameTemplateManager :: ~NameTemplateManager()
{
}

void NameTemplateManager :: AddIfValid(NameTemplate &newItem)
{
	if(newItem.mEquipType == 0)
		return;
	newItem.CountShareTotal();
	nameTemplate.push_back(newItem);
}

void NameTemplateManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open Item Name file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	NameEntry nameEntry;
	NameTemplate newItem;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				AddIfValid(newItem);
				newItem.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "mEquipType") == 0)
				newItem.mEquipType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "mWeaponType") == 0)
				newItem.mWeaponType = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "asset") == 0)
				newItem.assetName = lfr.BlockToStringC(1, 0);
			else if(strcmp(lfr.SecBuffer, "name") == 0)
			{
				Util::SafeCopy(nameEntry.name, lfr.BlockToStringC(1, 0), sizeof(nameEntry.name));
				nameEntry.shares = lfr.BlockToIntC(2);
				if(nameEntry.shares <= 0)
				{
					g_Log.AddMessageFormat("[WARNING] Name share chance is zero in file [%s] on line [%d]", filename, lfr.LineNumber);
					nameEntry.shares = 1;
				}
				newItem.nameList.push_back(nameEntry);
				nameEntry.Clear();
			}
			else
				g_Log.AddMessageFormat("[WARNING] Unknown identifier [%s] in file [%s] on line [%d]", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	AddIfValid(newItem);
	lfr.CloseCurrent();
}

void NameTemplateManager :: DebugSaveToFile(const char *filename)
{
	FILE *output = fopen(filename, "wb");
	if(output == NULL)
		return;

	std::sort(nameTemplate.begin(), nameTemplate.end());
	for(size_t i = 0; i < nameTemplate.size(); i++)
	{
		fprintf(output, "[ENTRY]\r\n");
		if(nameTemplate[i].assetName.size() > 0)
			fprintf(output, "asset=%s\r\n", nameTemplate[i].assetName.c_str());

		fprintf(output, "mEquipType=%d\r\n", nameTemplate[i].mEquipType);
		fprintf(output, "mWeaponType=%d\r\n", nameTemplate[i].mWeaponType);
		//std::sort(nameTemplate[i].nameList.begin(), nameTemplate[i].nameList.end());
		for(size_t c = 0; c < nameTemplate[i].nameList.size(); c++)
			fprintf(output, "app=%s,%d\r\n", nameTemplate[i].nameList[c].name, nameTemplate[i].nameList[c].shares);
		fprintf(output, "\r\n");
	}
	fclose(output);
}

const char* NameTemplateManager :: RetrieveName(int mEquipType, int mWeaponType, const char* asset)
{
	for(size_t i = 0; i < nameTemplate.size(); i++)
	{
		if(nameTemplate[i].mEquipType != mEquipType)
			continue;
		if(nameTemplate[i].mWeaponType != mWeaponType)
			continue;
		if(nameTemplate[i].assetName.compare(asset) != 0)
			continue;

		const char *result = nameTemplate[i].GetRandomName();
		if(result == NULL)
			result = GetDefaultName(mEquipType, mWeaponType, asset);
		return result;
	}
	g_Log.AddMessageFormat("[DEBUG] RetrieveName() unable to find name for mEquipType:%d, mWeaponType:%d, asset:%s", mEquipType, mWeaponType, asset);
	return GetDefaultName(mEquipType, mWeaponType, asset);
}

const char* NameTemplateManager :: GetDefaultName(int mEquipType, int mWeaponType, const char *asset)
{
	switch(mEquipType)
	{
	case ItemEquipType::ARMOR_SHIELD: return "Shield"; break;
	case ItemEquipType::ARMOR_HEAD: return "Helmet"; break;
	case ItemEquipType::ARMOR_NECK: return "Collar"; break;
	case ItemEquipType::ARMOR_SHOULDER: return "Shoulderpads"; break;
	case ItemEquipType::ARMOR_CHEST: return "Vest"; break;
	case ItemEquipType::ARMOR_ARMS: return "Sleeves"; break;
	case ItemEquipType::ARMOR_HANDS: return "Gloves"; break;
	case ItemEquipType::ARMOR_WAIST: return "Belt"; break;
	case ItemEquipType::ARMOR_LEGS: return "Leggings"; break;
	case ItemEquipType::ARMOR_FEET: return "Boots"; break;
	case ItemEquipType::ARMOR_RING: return "Ring"; break;
	case ItemEquipType::ARMOR_AMULET: return "Amulet"; break;
	}

	if(mWeaponType > 0 && asset != NULL)
	{
		if(strstr(asset, "Wand") != NULL) return "Wand";
		else if(strstr(asset, "Talisman") != NULL) return "Talisman";
		else if(strstr(asset, "Staff") != NULL) return "Staff";
		else if(strstr(asset, "Spear") != NULL) return "Spear";
		else if(strstr(asset, "Katar") != NULL) return "Katar";
		else if(strstr(asset, "Dagger") != NULL) return "Dagger";
		else if(strstr(asset, "Claw") != NULL) return "Claw";
		else if(strstr(asset, "Dirk") != NULL) return "Dirk";
		else if(strstr(asset, "Bow") != NULL) return "Bow";

		else if(strstr(asset, "2hSword") != NULL) return "Sword";
		else if(strstr(asset, "2hMace") != NULL) return "Mace";
		else if(strstr(asset, "2hAxe") != NULL) return "2hAxe";
		else if(strstr(asset, "1hSword") != NULL) return "Sword";
		else if(strstr(asset, "1hMace") != NULL) return "Mace";
		else if(strstr(asset, "1hHammer") != NULL) return "Hammer";
		else if(strstr(asset, "1hAxe") != NULL) return "Axe";
		
		//Throwing weapons
		else if(strstr(asset, "Shuriken") != NULL) return "Shuriken";
		else if(strstr(asset, "Knife") != NULL) return "Knife";
		else if(strstr(asset, "Axe") != NULL) return "Axe";
		else if(strstr(asset, "Dart") != NULL) return "Dart";
	}

	g_Log.AddMessageFormat("[WARNING] GetDefaultName() failed to resolve default name form mEquipType=%d, mWeaponType=%d, asset=%s", mEquipType, mWeaponType, asset);
	return NameTemplate::DEFAULT_NAME;
}



bool NameTemplateManager :: Debug_HasName(const char* asset, int mEquipType, int mWeaponType)
{
	for(size_t i = 0; i < nameTemplate.size(); i++)
	{
		if(nameTemplate[i].mEquipType != mEquipType)
			continue;
		if(nameTemplate[i].mWeaponType != mWeaponType)
			continue;
		if(nameTemplate[i].assetName.compare(asset) == 0)
			return true;
	}
	return false;
}



NameModEntry :: NameModEntry()
{
	Clear();
}

void NameModEntry :: Clear(void)
{
	mType = TYPE_NONE;
	mStatBase.clear();
	mName.clear();
}

NameModManager :: NameModManager()
{
}

NameModManager :: ~NameModManager()
{
}

void NameModManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open NameMod file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	NameModEntry entry;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("=,");
		if(r >= 3)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "prefix") == 0)
				entry.mType = NameModEntry::TYPE_PREFIX;
			else if(strcmp(lfr.SecBuffer, "suffix") == 0)
				entry.mType = NameModEntry::TYPE_SUFFIX;

			entry.mStatBase = lfr.BlockToStringC(1, 0);
			entry.mName = lfr.BlockToStringC(2, 0);

			if(entry.mType != NameModEntry::TYPE_NONE)
				mModList.push_back(entry);
			entry.Clear();
		}
	}
	lfr.CloseCurrent();
}

void NameModManager :: ResolveModifiers(const char *firstStat, const char *secondStat, NameModEntry** firstName, NameModEntry** secondName)
{
	if(firstStat == NULL)
		return;

	NameModEntry *first = NULL;
	NameModEntry *second = NULL;
	STRINGLIST strpair;
	for(size_t i = 0; i < mModList.size(); i++)
	{
		Util::Split(mModList[i].mStatBase, "|", strpair);
		if(strpair.size() == 1)
		{
			if(strpair[0].compare(firstStat) == 0 && mModList[i].mType == NameModEntry::TYPE_PREFIX)
				first = &mModList[i];
			if(secondStat != NULL && mModList[i].mType == NameModEntry::TYPE_SUFFIX)
				if(strpair[0].compare(secondStat) == 0)
					second = &mModList[i];
		}
		else
		{
			if(secondStat != NULL)
			{
				bool firstMatch = (strpair[0].compare(firstStat) == 0) || (strpair[0].compare(secondStat) == 0);
				bool secondMatch = (strpair[1].compare(firstStat) == 0) || (strpair[1].compare(secondStat) == 0);
				if(firstMatch == true && secondMatch == true)
				{
					first = &mModList[i];
					second = NULL;
					break;
				}
			}
		}
	}
	*firstName = first;
	*secondName = second;
}

NameWeight :: NameWeight()
{
	Clear();
}

void NameWeight :: Clear(void)
{
	memset(mStat, 0, sizeof(mStat));
	mWeight = 0.0F;
}

void NameWeightManager :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open NameWeight file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;

	NameWeight entry;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r >= 2)
		{
			Util::SafeCopy(entry.mStat, lfr.BlockToStringC(0, 0), sizeof(entry.mStat));
			entry.mWeight = (float)lfr.BlockToDblC(1);
			if(entry.mStat[0] != 0)
				mWeightList.push_back(entry);
			entry.Clear();
		}
	}
	lfr.CloseCurrent();
}

float NameWeightManager :: GetWeightForName(const char *statName)
{
	for(size_t i = 0; i < mWeightList.size(); i++)
		if(strcmp(mWeightList[i].mStat, statName) == 0)
			return mWeightList[i].mWeight;

	g_Log.AddMessageFormat("[WARNING] GetWeightForName() could not find stat: %s", statName);
	return 0.0F;
}



RarityConfig :: RarityConfig()
{
	Clear();
}

void RarityConfig :: Clear(void)
{
	mQualityLevel = 0;
	level = 0;
	chance = 0;
	statMax = 0;
	modMax = 0;
	levelBonus = 0.0F;
	rollMin = 0.0F;
	rollMax = 0.0F;
	nullMod = 0;
	statPointMult = 0.0F;
};

void RarityConfig :: CopyFrom(RarityConfig& source)
{
	memcpy(this, &source, sizeof(RarityConfig));
}

int RarityConfig :: GetMinRoll(int numPoints)
{
	if(rollMin <= 0.001F)
		return 1;

	int retAmount = (int)((float)numPoints * rollMin);
	return Util::ClipInt(retAmount, 1, numPoints);
}

int RarityConfig :: GetMaxRoll(int numPoints)
{
	if(rollMax <= 0.001F)
		return numPoints;

	int retAmount = (int)((float)numPoints * rollMax);
	return Util::ClipInt(retAmount, 1, numPoints);
}

int RarityConfig :: GetAdjustedCorePoints(int numPoints)
{
	if(statPointMult <= 0.001F)
		return numPoints;
	return (int)((float)numPoints * statPointMult);
}

void VirtualItemModSystem :: LoadFromFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[ERROR] Unable to open ModConfig file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	RarityConfig newItem;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.BreakUntil("=", '=');
		if(r > 0)
		{
			lfr.BlockToStringC(0, 0);
			if(strcmp(lfr.SecBuffer, "[ENTRY]") == 0)
			{
				UpdateRarityConfig(newItem);
				newItem.Clear();
			}
			else if(strcmp(lfr.SecBuffer, "mQualityLevel") == 0)
				newItem.mQualityLevel = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Level") == 0)
				newItem.level = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "Chance") == 0)
				newItem.chance = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "StatMax") == 0)
				newItem.statMax = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "ModMax") == 0)
				newItem.modMax = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "LevelBonus") == 0)
				newItem.levelBonus = (float)lfr.BlockToDblC(1);
			else if(strcmp(lfr.SecBuffer, "RollMin") == 0)
				newItem.rollMin = (float)lfr.BlockToDblC(1);
			else if(strcmp(lfr.SecBuffer, "RollMax") == 0)
				newItem.rollMax = (float)lfr.BlockToDblC(1);
			else if(strcmp(lfr.SecBuffer, "NullMod") == 0)
				newItem.nullMod = lfr.BlockToIntC(1);
			else if(strcmp(lfr.SecBuffer, "StatPointMult") == 0)
				newItem.statPointMult = (float)lfr.BlockToDblC(1);
			else
				g_Log.AddMessageFormat("[WARNING] Unknown identifier [%s] in file [%s] on line [%d", lfr.SecBuffer, filename, lfr.LineNumber);
		}
	}
	UpdateRarityConfig(newItem);
	lfr.CloseCurrent();
}

void VirtualItemModSystem :: UpdateRarityConfig(RarityConfig& entry)
{
	int index = Util::ClipInt(entry.mQualityLevel, 0, MAX_QUALITY_LEVEL);
	if(index == 0)
		return;
	rarityConfig[index].CopyFrom(entry);
}

void VirtualItemModSystem :: LoadSettings(void)
{
	char buffer[256];
	LoadFromFile(Platform::GenerateFilePath(buffer, "ItemMod", "RarityConfig.txt"));
	for(int i = 0; i <= MAX_QUALITY_LEVEL; i++)
	{
		if(rarityConfig[i].chance <= 0)
		{
			rarityConfig[i].chance = 0;
			continue;
		}

		double mod = (1.0 / (double)rarityConfig[i].chance) * (double)DROP_SHARES;
		rarityConfig[i].chance = (int)mod;
		//g_Log.AddMessageFormat("Applying rarity: %d = %d (%g)", rarityConfig[i].mQualityLevel, rarityConfig[i].chance, (rarityConfig[i].chance/ (double)DROP_SHARES) * 100.0);
	}
}

int VirtualItemModSystem :: GetDropRarity(const VirtualItemSpawnParams &params)
{
	//Roll a rarity type for a new drop.  Go through the list backwards.
	int baseRoll = randint_32bit(1, DROP_SHARES);
	for(int q = MAX_QUALITY_LEVEL; q >= MIN_QUALITY_LEVEL; q--)
	{
		if(params.level >= rarityConfig[q].level)
		{
			int compare = rarityConfig[q].chance;
			
			if(params.dropRateProfile != NULL)
			{
				if(params.dropRateProfile->IsCreatureAllowed(q, params.rarity, params.namedMob) == false)
					break;

				compare = params.dropRateProfile->Chance.QData[q];
			}

			if(Util::FloatEquivalent(params.dropMult[q], 0.0F) == false)
				compare = (int)(compare * params.dropMult[q]);

			//g_Log.AddMessageFormat("[%d] from %d to %d (%g) {%d}", q, rarityConfig[q].chance, compare, params.dropMult[q], baseRoll);

			if(baseRoll <= compare)
				return q;

			/*
			int adjusted = (int)((float)rarityConfig[i].chance * params.dropMult[i]);
			if(rnd <= adjusted)
				return i;
			*/
		}
	}
	return -1;
}


void VirtualItemModSystem :: Debug_RunDropDiagnostic(int level)
{
	int non = 0;
	int rarity[7] = {0};
	int iter = 100000000;
	VirtualItemSpawnParams params;
	params.level = level;
	for(int i = 0; i < iter; i++)
	{
		int index = g_VirtualItemModSystem.GetDropRarity(params);
		if(index >= 0)
			rarity[index]++;
		else
			non++;
	}
	g_Log.AddMessageFormat("No drops: %d (%g)", non, (non / (double)iter) * 100.0);
	for(int i = 0; i <= 6; i++)
		g_Log.AddMessageFormat("Rarity[%d]: %d (%g)", i, rarity[i], (rarity[i] / (double)iter) * 100.0);

}
