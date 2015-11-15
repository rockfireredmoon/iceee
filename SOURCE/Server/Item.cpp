#include <string.h>
#include "StringList.h"
#include "Config.h"   //For override globals
#include "ByteBuffer.h"
#include "DirectoryAccess.h"
#include <algorithm>
#include "Gamble.h"
#include "Util.h"
#include "VirtualItem.h"
#include "DebugProfiler.h"
#include "InstanceScale.h" //Drop rate profile
#include "ConfigString.h"
#include "Stats.h"

#include <stddef.h>

//vector<ItemDef> ItemList;

ItemManager g_ItemManager;

//Prebuilt override strings to use when loading items.
//To be used by loading functions only in this file.
char BindingTypeOverride[32] = {0};
char ArmorTypeOverride[32] = {0};
char WeaponTypeOverride[32] = {0};

const int maxItemLoadDef = 62;
ItemLoadTable ItemLoadDef[maxItemLoadDef] = {
	//0 to 4
	{Item_Integer, "mID", msizeof(ItemDef, mID), offsetof(ItemDef, mID) },
	{Item_Byte, "mType", msizeof(ItemDef, mType), offsetof(ItemDef, mType) },
	{Item_String, "mDisplayName", msizeof(ItemDef, mDisplayName), offsetof(ItemDef, mDisplayName) },
	{Item_String, "mAppearance", msizeof(ItemDef, mAppearance), offsetof(ItemDef, mAppearance) },
	{Item_String, "mIcon", msizeof(ItemDef, mIcon), offsetof(ItemDef, mIcon) },

	//5 to 9
	{Item_Byte, "mIvType1", msizeof(ItemDef, mIvType1), offsetof(ItemDef, mIvType1) },
	{Item_Short, "mIvMax1", msizeof(ItemDef, mIvMax1), offsetof(ItemDef, mIvMax1) },
	{Item_Byte, "mIvType2", msizeof(ItemDef, mIvType2), offsetof(ItemDef, mIvType2) },
	{Item_Short, "mIvMax2", msizeof(ItemDef, mIvMax2), offsetof(ItemDef, mIvMax2) },
	{Item_String, "mSv1", msizeof(ItemDef, mSv1), offsetof(ItemDef, mSv1) },

	//10
	{Item_Integer, "_mCopper", msizeof(ItemDef, _mCopper), offsetof(ItemDef, _mCopper) },

	//11 to 16
	{Item_Short, "mContainerSlots", msizeof(ItemDef, mContainerSlots), offsetof(ItemDef, mContainerSlots) },
	{Item_Byte, "mAutoTitleType", msizeof(ItemDef, mAutoTitleType), offsetof(ItemDef, mAutoTitleType) },
	{Item_Short, "mLevel", msizeof(ItemDef, mLevel), offsetof(ItemDef, mLevel) },
	{Item_Byte, "mBindingType", msizeof(ItemDef, mBindingType), offsetof(ItemDef, mBindingType) },
	{Item_Byte, "mEquipType", msizeof(ItemDef, mEquipType), offsetof(ItemDef, mEquipType) },
	{Item_Byte, "mWeaponType", msizeof(ItemDef, mWeaponType), offsetof(ItemDef, mWeaponType) },

	//16 to 21
	{Item_Integer, "mWeaponDamageMin", msizeof(ItemDef, mWeaponDamageMin), offsetof(ItemDef, mWeaponDamageMin) },
	{Item_Integer, "mWeaponDamageMax", msizeof(ItemDef, mWeaponDamageMax), offsetof(ItemDef, mWeaponDamageMax) },
	{Item_Byte, "_mSpeed", msizeof(ItemDef, _mSpeed), offsetof(ItemDef, _mSpeed) },
	{Item_Byte, "mWeaponExtraDamangeRating", msizeof(ItemDef, mWeaponExtraDamangeRating), offsetof(ItemDef, mWeaponExtraDamangeRating) },
	{Item_Byte, "mWeaponExtraDamageType", msizeof(ItemDef, mWeaponExtraDamageType), offsetof(ItemDef, mWeaponExtraDamageType) },

	//22 to 25
	{Item_Integer, "mEquipEffectId", msizeof(ItemDef, mEquipEffectId), offsetof(ItemDef, mEquipEffectId) },
	{Item_Integer, "mUseAbilityId", msizeof(ItemDef, mUseAbilityId), offsetof(ItemDef, mUseAbilityId) },
	{Item_Integer, "mActionAbilityId", msizeof(ItemDef, mActionAbilityId), offsetof(ItemDef, mActionAbilityId) },
	{Item_Byte, "mArmorType", msizeof(ItemDef, mArmorType), offsetof(ItemDef, mArmorType) },

	//26 to 30
	{Item_Integer, "mArmorResistMelee", msizeof(ItemDef, mArmorResistMelee), offsetof(ItemDef, mArmorResistMelee) },
	{Item_Integer, "mArmorResistFire", msizeof(ItemDef, mArmorResistFire), offsetof(ItemDef, mArmorResistFire) },
	{Item_Integer, "mArmorResistFrost", msizeof(ItemDef, mArmorResistFrost), offsetof(ItemDef, mArmorResistFrost) },
	{Item_Integer, "mArmorResistMystic", msizeof(ItemDef, mArmorResistMystic), offsetof(ItemDef, mArmorResistMystic) },
	{Item_Integer, "mArmorResistDeath", msizeof(ItemDef, mArmorResistDeath), offsetof(ItemDef, mArmorResistDeath) },

	//31 to 37
	{Item_Integer, "mBonusStrength", msizeof(ItemDef, mBonusStrength), offsetof(ItemDef, mBonusStrength) },
	{Item_Integer, "mBonusDexterity", msizeof(ItemDef, mBonusDexterity), offsetof(ItemDef, mBonusDexterity) },
	{Item_Integer, "mBonusConstitution", msizeof(ItemDef, mBonusConstitution), offsetof(ItemDef, mBonusConstitution) },
	{Item_Integer, "mBonusPsyche", msizeof(ItemDef, mBonusPsyche), offsetof(ItemDef, mBonusPsyche) },
	{Item_Integer, "mBonusSpirit", msizeof(ItemDef, mBonusSpirit), offsetof(ItemDef, mBonusSpirit) },
	{Item_Integer, "_mBonusHealth", msizeof(ItemDef, _mBonusHealth), offsetof(ItemDef, _mBonusHealth) },
	{Item_Integer, "mBonusWill", msizeof(ItemDef, mBonusWill), offsetof(ItemDef, mBonusWill) },

	//38 to 49
	{Item_Byte, "isCharm", msizeof(ItemDef, isCharm), offsetof(ItemDef, isCharm) },
	{Item_Float, "mMeleeHitMod", msizeof(ItemDef, mMeleeHitMod), offsetof(ItemDef, mMeleeHitMod) },
	{Item_Float, "mMeleeCritMod", msizeof(ItemDef, mMeleeCritMod), offsetof(ItemDef, mMeleeCritMod) },
	{Item_Float, "mMagicHitMod", msizeof(ItemDef, mMagicHitMod), offsetof(ItemDef, mMagicHitMod) },
	{Item_Float, "mMagicCritMod", msizeof(ItemDef, mMagicCritMod), offsetof(ItemDef, mMagicCritMod) },
	{Item_Float, "mParryMod", msizeof(ItemDef, mParryMod), offsetof(ItemDef, mParryMod) },
	{Item_Float, "mBlockMod", msizeof(ItemDef, mBlockMod), offsetof(ItemDef, mBlockMod) },
	{Item_Float, "mRunSpeedMod", msizeof(ItemDef, mRunSpeedMod), offsetof(ItemDef, mRunSpeedMod) },
	{Item_Float, "mRegenHealthMod", msizeof(ItemDef, mRegenHealthMod), offsetof(ItemDef, mRegenHealthMod) },
	{Item_Float, "mAttackSpeedMod", msizeof(ItemDef, mAttackSpeedMod), offsetof(ItemDef, mAttackSpeedMod) },
	{Item_Float, "mCastSpeedMod", msizeof(ItemDef, mCastSpeedMod), offsetof(ItemDef, mCastSpeedMod) },
	{Item_Float, "mHealingMod", msizeof(ItemDef, mHealingMod), offsetof(ItemDef, mHealingMod) },

	//50 to 51
	{Item_Integer, "mValue", msizeof(ItemDef, mValue), offsetof(ItemDef, mValue) },
	{Item_Byte, "mValueType", msizeof(ItemDef, mValueType), offsetof(ItemDef, mValueType) },

	//52 to 54
	{Item_Integer, "resultItemId", msizeof(ItemDef, resultItemId), offsetof(ItemDef, resultItemId) },
	{Item_Integer, "keyComponentId", msizeof(ItemDef, keyComponentId), offsetof(ItemDef, keyComponentId) },
	{Item_Integer, "numberOfItems", msizeof(ItemDef, numberOfItems), offsetof(ItemDef, numberOfItems) },

	//55, 56, 57
	{Item_IntVect, "craftItemDefId", msizeof(ItemDef, craftItemDefId), offsetof(ItemDef, craftItemDefId) },

	{Item_String, "mFlavorText", msizeof(ItemDef, mFlavorText), offsetof(ItemDef, mFlavorText) },

	{Item_Byte, "mSpecialItemType", msizeof(ItemDef, mSpecialItemType), offsetof(ItemDef, mSpecialItemType) },

	{Item_Byte, "mOwnershipRestriction", msizeof(ItemDef, mOwnershipRestriction), offsetof(ItemDef, mOwnershipRestriction) },
	{Item_Byte, "mQualityLevel", msizeof(ItemDef, mQualityLevel), offsetof(ItemDef, mQualityLevel) },
	{Item_Short, "mMinUseLevel", msizeof(ItemDef, mMinUseLevel), offsetof(ItemDef, mMinUseLevel) },

	//Extended stuff
	{Item_String, "Params", msizeof(ItemDef, Params), offsetof(ItemDef, Params) },
};


namespace WeaponTypeClassRestrictions
{
	static const int MAXWEAPON = 9;
	static const int MAXPROFESSION = 5;
	static const bool restrictData[MAXWEAPON][MAXPROFESSION] = {
		//  [0]   [1]    [2]     [3]   [4]
		//  none  knight rogue   mage  druid
		{  false, false, false, false, false  }, //0 = [WeaponType::NONE]
		{  false, false,  true,  true, false  }, //1 = [WeaponType::SMALL]
		{  false,  true,  true, false,  true  }, //2 = [WeaponType::ONE_HAND]
		{  false,  true, false, false, false  }, //3 = [WeaponType::TWO_HAND]
		{  false, false, false,  true,  true  }, //4 = [WeaponType::POLE]
		{  false, false, false,  true,  true  }, //5 = [WeaponType::WAND]
		{  false,  true,  true, false,  true  }, //6 = [WeaponType::BOW]
		{  false,  true,  true,  true, false  }, //7 = [WeaponType::THROWN]
		{  false,  true,  true,  true,  true  }, //8 = [WeaponType::ARCANE_TOTEM]
	};
	bool isValid(int weaponType, int profession)
	{
		if(weaponType < 0 || weaponType >= MAXWEAPON)
			return false;
		if(profession < 0 || profession >= MAXPROFESSION)
			return false;
		return restrictData[weaponType][profession];
	}
}

//Note: if entries are changed here, then StandardContainerEnum must also be updated.
const int InventoryMappingCount = 11;
InventoryMappingDef InventoryMapping[InventoryMappingCount] = {
	{"inv",       0 },
	{"inventory", 0 },  //When looking up the string from the ID, "inv" needs to take precedence
	{"eq",        1 },
	{"eq_head",   1 },
	{"bank",      2 },
	{"shop",      3 },
	{"buyback",   4 },
	{"trade",     5 },
	{"stamps",    7 },
	{"delivery",  8 },
	{"auction",   9 }
};


//These two functions are for searching values in the above table
int GetContainerIDFromName(const char *name)
{
	int a;
	for(a = 0; a < InventoryMappingCount; a++)
		if(strcmp(InventoryMapping[a].name, name) == 0)
			return InventoryMapping[a].ID;

	return -1;
}

const char *GetContainerNameFromID(int ID)
{
	int a;
	for(a = 0; a < InventoryMappingCount; a++)
		if(InventoryMapping[a].ID == ID)
			return InventoryMapping[a].name;

	return NULL;
}

const int EquipmentMappingCount = 29;
InventoryMappingDef EquipmentMapping[EquipmentMappingCount] = {
	{"eq_main_hand",           ItemEquipSlot::WEAPON_MAIN_HAND },
	{"eq_off_hand",            ItemEquipSlot::WEAPON_OFF_HAND },
	{"eq_ranged",              ItemEquipSlot::WEAPON_RANGED },
	{"eq_head",                ItemEquipSlot::ARMOR_HEAD },
	{"eq_neck",                ItemEquipSlot::ARMOR_NECK },
	{"eq_shoulders",           ItemEquipSlot::ARMOR_SHOULDER },
	{"eq_chest",               ItemEquipSlot::ARMOR_CHEST },
	{"eq_arms",                ItemEquipSlot::ARMOR_ARMS },
	{"eq_hands",               ItemEquipSlot::ARMOR_HANDS },
	{"eq_waist",               ItemEquipSlot::ARMOR_WAIST },
	{"eq_legs",                ItemEquipSlot::ARMOR_LEGS },
	{"eq_feet",                ItemEquipSlot::ARMOR_FEET },
	{"eq_ring_l",              ItemEquipSlot::ARMOR_RING_L },
	{"eq_ring_r",              ItemEquipSlot::ARMOR_RING_R },
	{"eq_amulet",              ItemEquipSlot::ARMOR_AMULET },
	{"eq_focus_fire",          ItemEquipSlot::FOCUS_FIRE },
	{"eq_focus_frost",         ItemEquipSlot::FOCUS_FROST },
	{"eq_focus_mystic",        ItemEquipSlot::FOCUS_MYSTIC },
	{"eq_focus_death",         ItemEquipSlot::FOCUS_DEATH },
	{"eq_cosmetic_shoulder_l", ItemEquipSlot::COSMETIC_SHOULDER_L },
	{"eq_cosmetic_hip_l",      ItemEquipSlot::COSMETIC_HIP_L },
	{"eq_cosmetic_shoulder_r", ItemEquipSlot::COSMETIC_SHOULDER_R },
	{"eq_cosmetic_hip_r",      ItemEquipSlot::COSMETIC_HIP_R },
	{"eq_red_charm",           ItemEquipSlot::RED_CHARM },
	{"eq_green_charm",         ItemEquipSlot::GREEN_CHARM },
	{"eq_blue_charm",          ItemEquipSlot::BLUE_CHARM },
	{"eq_orange_charm",        ItemEquipSlot::ORANGE_CHARM },
	{"eq_yellow_charm",        ItemEquipSlot::YELLOW_CHARM },
	{"eq_purple_charm",        ItemEquipSlot::PURPLE_CHARM }
};

const char * Map_EquipType[] = {
	"None",             //   0
	"One-Hand",			//   1
	"One-Hand Unique",	//   2
	"One-Hand Main",	//   3
	"One-Hand Offhand",	//   4
	"Two-Hand",			//   5
	"Ranged",			//   6
	"Shield",			//   7
	"Head",				//   8
	"Neck",				//   9
	"Shoulder",			//  10
	"Chest",			//  11
	"Arms",				//  12
	"Hands",			//  13
	"Waist",			//  14
	"Legs",				//  15
	"Feet",				//  16
	"Ring",				//  17
	"Ring Unique",		//  18
	"Amulet",			//  19
	"Focus Fire",		//  20
	"Focus Frost",		//  21
	"Focus Mystic",		//  22
	"Focus Death",		//  23
	"Container",		//  24
	"Cosmetic Shoulder",//  25
	"Cosmetic Hip",		//  26
	"Red Charm",		//  27
	"Green Charm",		//  28
	"Blue Charm",		//  29
	"Orange Charm",		//  30
	"Yellow Charm",		//  31
	"Purple Charm",		//  32
};
const char *GetEquipType(int Type)
{
	if(Type > 32)
		return Map_EquipType[0];
	return Map_EquipType[Type];
}

const char * Map_ArmorType[] = {
	"None",   // 0
	"Cloth",  // 1
	"Light",  // 2
	"Medium", // 3
	"Heavy",  // 4
	"Shield", // 5
};
const char *GetArmorType(int Type)
{
	if(Type > 5)
		return Map_ArmorType[0];
	return Map_ArmorType[Type];
}

const char * Map_WeaponType[] = {
	"None",         // 0
	"Small",        // 1
	"One Hand",     // 2
	"Two Hand",     // 3
	"Pole",         // 4
	"Wand",         // 5
	"Bow",          // 6
	"Thrown",       // 7
	"Arcane Totem", // 8
};
const char *GetWeaponType(int Type)
{
	if(Type > 8)
		return Map_WeaponType[0];
	return Map_WeaponType[Type];
}

//This function is for searching values in the above table
int GetEQSlotFromName(char *name)
{
	int a;
	for(a = 0; a < EquipmentMappingCount; a++)
		if(strcmp(EquipmentMapping[a].name, name) == 0)
			return EquipmentMapping[a].ID;

	return -1;
}

int SetItemProperty(ItemDef *item, const char *name, const char *value)
{
	//MessageBox(NULL, value, name, 0);
	int found = -1;
	int a;
	for(a = 0; a < maxItemLoadDef; a++)
		if(strcmp(ItemLoadDef[a].loadName, name) == 0)
		{
			found = a;
			break;
		}

	if(found == -1)
		return -1;

	char *base = (char*)item;
	switch(ItemLoadDef[found].type)
	{
	case Item_Byte:
		char cval;
		cval = (char)atoi(value);
		memcpy(&base[ItemLoadDef[found].offset], &cval, sizeof(cval));
		break;
	case Item_Short:
		short sval;
		sval = (short)atoi(value);
		memcpy(&base[ItemLoadDef[found].offset], &sval, sizeof(sval));
		break;
	case Item_Integer:
		int ival;
		ival = (int)atoi(value);
		memcpy(&base[ItemLoadDef[found].offset], &ival, sizeof(ival));
		break;
	case Item_Float:
		float fval;
		fval = (float)atof(value);
		memcpy(&base[ItemLoadDef[found].offset], &fval, sizeof(fval));
		break;
	case Item_String:
		string *str;
		str = (string*)&base[ItemLoadDef[found].offset];
		str->assign(value);
		break;
	case Item_IntVect:
		vector<int> *ivec;
		ivec = (vector<int>*)&base[ItemLoadDef[found].offset];
		ivec->push_back(atoi(value));
		break;
	}

	return found;
}

ItemDef :: ItemDef()
{
	Reset();
}

ItemDef :: ~ItemDef()
{
	mDisplayName.clear();
	mAppearance.clear();
	mIcon.clear();
	mSv1.clear();
	craftItemDefId.clear();
	mFlavorText.clear();
}

void ItemDef :: Reset(void)
{
	mID = 0;
	mType = 0;
	mDisplayName.clear();
	mAppearance.clear();
	mIcon.clear();

	mIvType1 = 0;
	mIvMax1 = 0;
	mIvType2 = 0;
	mIvMax2 = 0;
	mSv1.clear();

	_mCopper = 0;

	mContainerSlots = 0;
	mAutoTitleType = 0;
	mLevel = 0;
	mBindingType = 0;
	mEquipType = 0;
	mWeaponType = 0;

	mWeaponDamageMin = 0;
	mWeaponDamageMax = 0;
	_mSpeed = 0;
	mWeaponExtraDamangeRating = 0;
	mWeaponExtraDamageType = 0;

	mEquipEffectId = 0;
	mUseAbilityId = 0;
	mActionAbilityId = 0;
	mArmorType = 0;

	mArmorResistMelee = 0;
	mArmorResistFire = 0;
	mArmorResistFrost = 0;
	mArmorResistMystic = 0;
	mArmorResistDeath = 0;

	mBonusStrength = 0;
	mBonusDexterity = 0;
	mBonusConstitution = 0;
	mBonusPsyche = 0;
	mBonusSpirit = 0;
	_mBonusHealth = 0;
	mBonusWill = 0;

	isCharm = 0;
	mMeleeHitMod = 0.0F;
	mMeleeCritMod = 0.0F;
	mMagicHitMod = 0.0F;
	mMagicCritMod = 0.0F;
	mParryMod = 0.0F;
	mBlockMod = 0.0F;
	mRunSpeedMod = 0.0F;
	mRegenHealthMod = 0.0F;
	mAttackSpeedMod = 0.0F;
	mCastSpeedMod = 0.0F;
	mHealingMod = 0.0F;

	mValue = 0;
	mValueType = 0;

	resultItemId = 0;
	keyComponentId = 0;
	numberOfItems = 0;
	craftItemDefId.clear();

	mFlavorText.clear();

	mSpecialItemType = 0;

	mOwnershipRestriction = 0;
	mQualityLevel = 0;
	mMinUseLevel = 0;

	Params.clear();
}

void ItemDef :: CopyFrom(ItemDef *source)
{
	if(source == NULL || source == this)
		return;

	mID = source->mID; 
	mType = source->mType;
	mDisplayName = source->mDisplayName;
	mAppearance = source->mAppearance;
	mIcon = source->mIcon;
	mIvType1 = source->mIvType1;
	mIvMax1 = source->mIvMax1;
	mIvType2 = source->mIvType2;
	mIvMax2 = source->mIvMax2;
	mSv1 = source->mSv1;
	_mCopper = source->_mCopper;
	mContainerSlots = source->mContainerSlots;
	mAutoTitleType = source->mAutoTitleType;
	mLevel = source->mLevel;
	mBindingType = source->mBindingType;
	mEquipType = source->mEquipType;
	mWeaponType = source->mWeaponType;
	mWeaponDamageMin = source->mWeaponDamageMin;
	mWeaponDamageMax = source->mWeaponDamageMax;
	_mSpeed = source->_mSpeed;
	mWeaponExtraDamangeRating = source->mWeaponExtraDamangeRating;
	mWeaponExtraDamageType = source->mWeaponExtraDamageType;
	mEquipEffectId = source->mEquipEffectId;
	mUseAbilityId = source->mUseAbilityId;
	mActionAbilityId = source->mActionAbilityId;
	mArmorType = source->mArmorType;
	mArmorResistMelee = source->mArmorResistMelee;
	mArmorResistFire = source->mArmorResistFire;
	mArmorResistFrost = source->mArmorResistFrost;
	mArmorResistMystic = source->mArmorResistMystic;
	mArmorResistDeath = source->mArmorResistDeath;
	mBonusStrength = source->mBonusStrength;
	mBonusDexterity = source->mBonusDexterity;
	mBonusConstitution = source->mBonusConstitution;
	mBonusPsyche = source->mBonusPsyche;
	mBonusSpirit = source->mBonusSpirit;
	_mBonusHealth = source->_mBonusHealth;
	mBonusWill = source->mBonusWill;
	isCharm = source->isCharm;
	mMeleeHitMod = source->mMeleeHitMod;
	mMeleeCritMod = source->mMeleeCritMod;
	mMagicHitMod = source->mMagicHitMod;
	mMagicCritMod = source->mMagicCritMod;
	mParryMod = source->mParryMod;
	mBlockMod = source->mBlockMod;
	mRunSpeedMod = source->mRunSpeedMod;
	mRegenHealthMod = source->mRegenHealthMod;
	mAttackSpeedMod = source->mAttackSpeedMod;
	mCastSpeedMod = source->mCastSpeedMod;
	mHealingMod = source->mHealingMod;
	mValue = source->mValue;
	mValueType = source->mValueType;
	resultItemId = source->resultItemId;
	keyComponentId = source->keyComponentId;
	numberOfItems = source->numberOfItems;
	craftItemDefId = source->craftItemDefId;
	mFlavorText = source->mFlavorText;
	mSpecialItemType = source->mSpecialItemType;
	mOwnershipRestriction = source->mOwnershipRestriction;
	mQualityLevel = source->mQualityLevel;
	mMinUseLevel = source->mMinUseLevel;
	Params = source->Params;
}

void ItemDef :: WriteToJSON(Json::Value &value) {
	value["id"] = mID;
	value["type"] = mType;
	value["displayName"] = mDisplayName;
	value["appearance"] = mAppearance;
	std::vector<std::string> l;
	Util::Split(mIcon, "|", l);
	switch (l.size()) {
	case 1:
		value["icon1"] = "Icon-32-BG-Blue.png";
		value["icon2"] = l[0];
		break;
	case 2:
		value["icon1"] = l[0];
		value["icon2"] = l[1];
		break;
	}
	value["ivType1"] = mIvType1;
	value["ivMax1"] = mIvMax1;
	value["ivType2"] = mIvType2;
	value["ivMax2"] = mIvMax2;
	value["sv1"] = mSv1;
	value["copper"] = _mCopper;
	value["containerSlots"] = mContainerSlots;
	value["autoTitleType"] = mAutoTitleType;
	value["level"] = mLevel;
	value["bindingType"] = mBindingType;
	value["equipType"] = mEquipType;
	value["weaponType"] = mWeaponType;
	value["weaponDamageMin"] = mWeaponDamageMin;
	value["weaponDamageMax"] = mWeaponDamageMax;
	value["speed"] = _mSpeed;
	value["weaponExtraDamageRating"] = mWeaponExtraDamangeRating;
	value["weaponExtraDamageType"] = mWeaponExtraDamageType;
	value["equipEffectId"] = mEquipEffectId;
	value["useAbilityId"] = mUseAbilityId;
	value["actionAbilityId"] = mActionAbilityId;
	value["armorType"] = mArmorType;
	value["armorResistMelee"] = mArmorResistMelee;
	value["armorResistFire"] = mArmorResistFire;
	value["armorResistFrost"] = mArmorResistFrost;
	value["armorResistMystic"] = mArmorResistMystic;
	value["armorResistDeath"] = mArmorResistDeath;
	value["bonusStrength"] = mBonusStrength;
	value["bonusDexterity"] = mBonusDexterity;
	value["bonusConstitution"] = mBonusConstitution;
	value["bonusPsyche"] = mBonusPsyche;
	value["bonusSpirit"] = mBonusSpirit;
	value["bonusHealth"] = _mBonusHealth;
	value["bonusWill"] = mBonusWill;
	value["charm"] = isCharm;
	value["meleeHitMod"] = mMeleeHitMod;
	value["meleeCritMod"] = mMeleeCritMod;
	value["magicHitMod"] = mMagicHitMod;
	value["parryMod"] = mParryMod;
	value["blockMod"] = mBlockMod;
	value["regenHealthMod"] = mRegenHealthMod;
	value["attackSpeedMod"] = mAttackSpeedMod;
	value["castSpeedMod"] = mCastSpeedMod;
	value["healingMod"] = mHealingMod;
	value["value"] = mValue;
	value["valueType"] = mValueType;
	value["resultItemId"] = resultItemId;
	value["keyComponentId"] = keyComponentId;
	value["numberOfItems"] = numberOfItems;
	value["flavorText"] = mFlavorText;
	value["specialItemType"] = mSpecialItemType;
	value["ownershipRestriction"] = mOwnershipRestriction;
	value["qualityLevel"] = mQualityLevel;
	value["minUseLevel"] = mMinUseLevel;
	value["params"] = Params;
	Json::Value c;
	for(std::vector<int>::iterator it = craftItemDefId.begin(); it != craftItemDefId.end(); ++it)
		c.append(*it);
	value["craft"] = c;
}

void ItemDef :: ProcessParams(void)
{
	if(Params.size() == 0)
		return;
	
	std::string newFlavorText;
	char buffer[256];

	STRINGLIST datapairs;
	STRINGLIST keyvalue;
	Util::Split(Params, "&", datapairs);
	for(size_t i = 0; i < datapairs.size(); i++)
	{
		Util::Split(datapairs[i], "=", keyvalue);
		if(keyvalue.size() >= 2)
		{
			int index = GetStatIndexByName(keyvalue[0].c_str());
			if(index >= 0)
			{
				int statID = StatList[index].ID;
				float value = static_cast<float>(atof(keyvalue[1].c_str()));

				const char *formatStr = NULL;
				switch(statID)
				{
				case STAT::MOD_MELEE_TO_CRIT: formatStr = "+%g%% Physical Critical Chance"; value /= 10.0F; break;
				case STAT::MOD_MAGIC_TO_CRIT: formatStr = "+%g%% Magic Critical Chance"; value /= 10.0F; break;
				case STAT::BASE_BLOCK: formatStr = "+%g%% Block Chance", value /= 10.0F; break;
				case STAT::BASE_PARRY: formatStr = "+%g%% Parry Chance", value /= 10.0F; break;
				case STAT::BASE_DODGE: formatStr = "+%g%% Dodge Chance", value /= 10.0F; break;
				case STAT::MOD_MOVEMENT: formatStr = "+%g%% Movement Speed"; break;  //INT
				case STAT::EXPERIENCE_GAIN_RATE: formatStr = "+%g%% Experience Gain"; break;  //INT
				case STAT::MELEE_ATTACK_SPEED: formatStr = "+%g%% Increased Attack Speed", value /= 10.0F; break;
				case STAT::MAGIC_ATTACK_SPEED: formatStr = "+%g%% Increased Cast Rate", value /= 10.0F; break;
				case STAT::DMG_MOD_FIRE: formatStr = "+%g%% Fire Specialization", value /= 10.0F; break;
				case STAT::DMG_MOD_FROST: formatStr = "+%g%% Frost Specialization", value /= 10.0F; break;
				case STAT::DMG_MOD_MYSTIC: formatStr = "+%g%% Mystic Specialization", value /= 10.0F; break;
				case STAT::DMG_MOD_DEATH: formatStr = "+%g%% Death Specialization", value /= 10.0F; break;
				case STAT::BASE_HEALING: formatStr = "+%g%% Healing Specialization", value /= 10.0F; break;
				case STAT::CASTING_SETBACK_CHANCE: formatStr = "%g%% Casting Setback Chance", value /= 10.0F; break;
				case STAT::CHANNELING_BREAK_CHANCE: formatStr = "%g%% Channel Break Chance", value /= 10.0F; break;
				case STAT::MOD_HEALTH_REGEN: formatStr = "+%g Hitpoint Regeneration", value;  break;  //INT
				}
				if(formatStr != NULL)
				{
					Util::SafeFormat(buffer, sizeof(buffer), formatStr, value);
					if(newFlavorText.size() > 0)
						newFlavorText.append("<br>");
					newFlavorText.append(buffer);
				}
			}
		}
	}
	if(newFlavorText.size() > 0)
	{
		if(mFlavorText.size() > 0)
			mFlavorText.append("<br>");

		mFlavorText.append("</i>");
		mFlavorText.append(newFlavorText);
	}
}

//Nothing important, just used to write out arbitrary data to a file to help with whatever
//sorting or data analysis is needed at the time.
void ItemDef :: Debug_WriteToStream(FILE *output)
{
	fprintf(output, "%d;%d;%d;%d;", mID, mQualityLevel, mLevel, mMinUseLevel);
	fprintf(output, "%s;%s;%s;%s;", mDisplayName.c_str(), GetEquipType(mEquipType), GetArmorType(mArmorType), GetWeaponType(mWeaponType));
	if(mBonusStrength != 0) fprintf(output, "%d;", mBonusStrength); else fputc(';', output);
	if(mBonusDexterity != 0) fprintf(output, "%d;", mBonusDexterity); else fputc(';', output);
	if(mBonusConstitution != 0) fprintf(output, "%d;", mBonusConstitution); else fputc(';', output);
	if(mBonusPsyche != 0) fprintf(output, "%d;", mBonusPsyche); else fputc(';', output);
	if(mBonusSpirit != 0) fprintf(output, "%d;", mBonusSpirit); else fputc(';', output);
	fprintf(output, "%d;%d;", mArmorResistMelee, mValue);
	if(mWeaponDamageMax != 0) fprintf(output, "%d-%d;", mWeaponDamageMin, mWeaponDamageMax); else fputc(';', output);
	switch(mBindingType)
	{
	case BIND_ON_EQUIP: fputs("E;", output); break;
	case BIND_ON_PICKUP: fputs("P;", output); break;
	default: fputc(';', output); break;
	}
	fprintf(output, "%s;%s;", mSv1.c_str(), mFlavorText.c_str());
	fputs("\r\n", output);
}

void ItemDef :: Debug_WriteReport(ReportBuffer &report)
{
	report.AddLine("%s [%d] lev:%d, qlev:%d", mDisplayName.c_str(), mID, mLevel, mQualityLevel);
	report.AddLine("%s | %s | %s", GetEquipType(mEquipType), GetArmorType(mArmorType), GetWeaponType(mWeaponType));
	report.AddLine("str:%d, dex:%d, con:%d, psy:%d, spi:%d", mBonusStrength, mBonusDexterity, mBonusConstitution, mBonusPsyche, mBonusSpirit);
	report.AddLine("armor:%d, dmg:%d-%d", mArmorResistMelee, mWeaponDamageMin, mWeaponDamageMax);
	if(mFlavorText.size() > 0)
	{
		std::string tmp = mFlavorText;
		size_t pos;
		pos = tmp.find(FLAVOR_TEXT_HEADER);
		if(pos != string::npos)
			tmp.erase(pos, strlen(FLAVOR_TEXT_HEADER));
		pos = tmp.find(FLAVOR_TEXT_FOOTER);
		if(pos != string::npos)
			tmp.erase(pos, strlen(FLAVOR_TEXT_FOOTER));

		report.AddLine("flavor:%s", tmp.c_str());
	}
	report.AddLine(NULL);
}

int LoadItemFromStream(FileReader &fr, ItemDef *itemDef, char *debugFilename)
{
	//Return codes:
	//   1  Section end marker reached.
	//   0  End of file reached.
	//  -1  Another entry was encountered

	bool curEntry = false;
	int r;
	while(fr.FileOpen())
	{
		long CurPos = ftell(fr.FileHandle[0]);
		r = fr.ReadLine();
		if(r > 0)
			r = fr.SingleBreak(NULL);
		if(r > 0)
		{
			fr.BlockToStringC(0, Case_Upper);
			if(strcmp(fr.SecBuffer, "[ENTRY]") == 0)
			{
				if(curEntry == true)
				{
					//Reset the position so it doesn't interfere with reading the next
					//entry
					fr.FilePos = CurPos;
					fseek(fr.FileHandle[0], CurPos, SEEK_SET);
					return -1;
				}
				else
					curEntry = true;
			}
			else if(strcmp(fr.SecBuffer, "[END]") == 0)
			{
				return 1;
			}
			else
			{
				char *ValueBuf = fr.BlockToString(1);
				char *NameBuf = fr.BlockToString(0);
				if(strcmp(NameBuf, "mBindingType") == 0)
				{
					if(g_ItemBindingTypeOverride > 0)
					{
						if(ValueBuf[0] == '0' + g_ItemBindingTypeOverride)
							ValueBuf[0] = 0;
					}
					else if(g_ItemBindingTypeOverride == 0)
						ValueBuf = BindingTypeOverride;
				}
				else if(strcmp(NameBuf, "mArmorType") == 0)
				{
					if(g_ItemArmorTypeOverride >= 0)
						ValueBuf = ArmorTypeOverride;
				}
				else if(strcmp(NameBuf, "mWeaponType") == 0)
				{
					if(g_ItemWeaponTypeOverride >= 0)
						ValueBuf = WeaponTypeOverride;
				}
				/*
				if(strcmp(NameBuf, "craftItemDefId") == 0)
				{
					itemDef->craftItemDefId.push_back(fr.BlockToInt(1));
				}*/
				//int r = SetItemProperty(itemDef, fr.BlockToString(0), fr.BlockToString(1));
				int r = SetItemProperty(itemDef, NameBuf, ValueBuf);
				if(r == -1)
					g_Log.AddMessageFormat("Unknown property [%s] in item file [%s] on line [%d]", fr.BlockToString(0), debugFilename, fr.LineNumber);
			}
		}
	}
	fr.CloseCurrent();

	return 1;
}

unsigned long GetIDSlot(unsigned long ID, unsigned long slot)
{
	return (ID << 16) | slot;
}

char *GetItemProto(char *convbuf, int ItemID, int count)
{
	sprintf(convbuf, "item%d:0:%d:0", ItemID, count);
	return convbuf;
}

ItemManager :: ItemManager()
{
	nextVirtualItemID = BASE_VIRTUAL_ITEM_ID;
	nextVirtualItemAutosave = 0;
}

ItemManager :: ~ItemManager()
{
	Free();
}

void ItemManager :: Free(void)
{
	ItemList.clear();
}

void ItemManager :: Sort(void)
{
	//TODO: OBSOLETE (maps are already sorted)
	//std::sort(ItemList.begin(), ItemList.end());
}

void ItemManager :: Finalize(void)
{
	// Need to have at least one entry in the list so that item lookups
	// can return a null entry if necessary.

	//TODO: OBSOLETE
	/*
	if(ItemList.size() == 0)
	{
		ItemDef newItem;
		newItem.mID = 0;
		newItem.mDisplayName = "Null Item";
		ItemList.push_back(newItem);
	}
	Sort();
	*/
	ItemList[0].mDisplayName = "Null Item";
}

void ItemManager :: LoadData(void)
{
	char buffer[256];
	LoadItemPackages(Platform::GenerateFilePath(buffer, "Packages", "ItemPack.txt"), false);
	LoadItemPackages(Platform::GenerateFilePath(buffer, "Packages", "ItemPackOverride.txt"), true);
	g_Log.AddMessageFormat("Loaded %d items.", g_ItemManager.GetStandardCount());
}

void ItemManager :: LoadItemList(char *filename, bool itemOverride)
{
	TimeObject to("ItemManager::LoadItemList");

	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open file [%s].", filename);
		return;
	}

	//Set the prebuilt overrides.  Should be helpful since there could potentially
	//be thousands of items in the list, and it will save from having convert these values
	//for each item that applies.
	sprintf(BindingTypeOverride, "%d", g_ItemBindingTypeOverride);
	sprintf(ArmorTypeOverride, "%d", g_ItemArmorTypeOverride);
	sprintf(WeaponTypeOverride, "%d", g_ItemWeaponTypeOverride);

	char Delimiters[] = {'=', 13, 10, 0 };
	lfr.Delimiter = Delimiters;
	lfr.CommentStyle = Comment_Semi;

	ItemDef newItem;

	int count = 0;
	while(lfr.FileOpen())
	{
		newItem.Reset();
		LoadItemFromStream(lfr, &newItem, filename);
		newItem.ProcessParams();
		if(itemOverride == true)
		{
			ItemDef *ptr = GetPointerByID(newItem.mID);
			if(ptr != NULL)
				ptr->CopyFrom(&newItem);
			else
				g_Log.AddMessageFormat("[WARNING] Item override specified for ID:%d was not found [%s]", newItem.mID, filename);
		}
		else
		{
			ItemDef *ptr = &ItemList[newItem.mID];
			if(ptr->mID == 0)
				ptr->CopyFrom(&newItem);
			else
				g_Log.AddMessageFormat("[WARNING] Not allowed to override item %d [%s] in file [%s]", ptr->mID, ptr->mDisplayName.c_str(), filename);
				//TODO: Obsolete
			//ItemList.push_back(newItem);
		}
		count++;
	}
	lfr.CloseCurrent();
}

void ItemManager :: LoadItemPackages(char *listFile, bool itemOverride)
{
	FileReader lfr;
	if(lfr.OpenText(listFile) != Err_OK)
	{
		g_Log.AddMessageFormat("Could not open Item list file [%s]", listFile);
		Finalize();
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			Platform::FixPaths(lfr.DataBuffer);
			LoadItemList(lfr.DataBuffer, itemOverride);
		}
	}
	lfr.CloseCurrent();

	//Run any post-processing, like sorting.
	Finalize();
}

ItemDef * ItemManager :: GetDefaultItemPtr(void)
{
	/*TODO: OBSOLETE
	return &*ItemList.begin();
	*/
	return &ItemList[0];
}

ItemDef * ItemManager :: GetPointerByID(int ID)
{
	if(ID < BASE_VIRTUAL_ITEM_ID)
	{
		/* TODO: OBSOLETE
		ITEM_CONT::iterator it;
		ItemDef searchObj;
		searchObj.mID = ID;
		it = std::lower_bound(ItemList.begin(), ItemList.end(), searchObj);
		if(it == ItemList.end())
			return NULL;

		if(it->mID != ID)
			return NULL;
		return &*it;
		*/
		ITEM_CONT::iterator it;
		it = ItemList.find(ID);
		if(it == ItemList.end())
			return NULL;
		return &it->second;
	}
	else
	{
		VITEM_CONT::iterator it;
		it = VItemList.find(ID);
		if(it != VItemList.end())
			return &it->second.mStandardDef;

		//Not found, load the page and try again.
		LoadVirtualItemWithID(ID);
		it = VItemList.find(ID);
		if(it != VItemList.end())
			return &it->second.mStandardDef;
		else
			g_Log.AddMessageFormat("Virtual item not found: %d", ID);
	}
	return NULL;
}

VirtualItem * ItemManager :: GetVirtualItem(int ID)
{
	if(ID < BASE_VIRTUAL_ITEM_ID)
		return NULL;
	
	VITEM_CONT::iterator it;
	it = VItemList.find(ID);
	if(it != VItemList.end())
		return &it->second;

	//Not found, load the page and try again.
	LoadVirtualItemWithID(ID);
	it = VItemList.find(ID);
	if(it != VItemList.end())
		return &it->second;
	else
		g_Log.AddMessageFormat("Virtual Item not found: %d", ID);
	return NULL;
}

ItemDef * ItemManager :: GetSafePointerByID(int ID)
{
	if(ID < BASE_VIRTUAL_ITEM_ID)
	{
		/* TODO: OBSOLETE
		ITEM_CONT::iterator it;
		ItemDef searchObj;
		searchObj.mID = ID;
		it = std::lower_bound(ItemList.begin(), ItemList.end(), searchObj);
		if(it == ItemList.end())
			return &*ItemList.begin();

		if(it->mID != ID)
			return &*ItemList.begin();

		return &*it;
		*/
		ITEM_CONT::iterator it;
		it = ItemList.find(ID);
		if(it != ItemList.end())
			return &it->second;
		else
			g_Log.AddMessageFormat("[ERROR] Item not found: %d", ID);
	}
	else
	{
		VITEM_CONT::iterator it;
		it = VItemList.find(ID);
		if(it != VItemList.end())
			return &it->second.mStandardDef;

		//Not found, load the page and try again.
		LoadVirtualItemWithID(ID);
		it = VItemList.find(ID);
		if(it != VItemList.end())
			return &it->second.mStandardDef;
		else
			g_Log.AddMessageFormat("[ERROR] Virtual item not found: %d", ID);
	}
	return GetDefaultItemPtr();
}

ItemDef * ItemManager :: GetSafePointerByExactName(const char *name)
{
	ITEM_CONT::iterator it;
	for(it = ItemList.begin(); it != ItemList.end(); ++it)
		if(it->second.mDisplayName.compare(name) == 0)
			return &it->second;

	return GetDefaultItemPtr();
}

ItemDef * ItemManager :: GetSafePointerByPartialName(const char *name)
{
	ITEM_CONT::iterator it;
	for(it = ItemList.begin(); it != ItemList.end(); ++it)
		if(it->second.mDisplayName.find(name) != string::npos)
			return &it->second;

	return GetDefaultItemPtr();
}

// Return the number of items in the standard list.
int ItemManager :: GetStandardCount(void)
{
	return ItemList.size();
}

// Scan all items in the standard list for a partial name match, building a
// list of pointers.  Abort if maxCount is reached.
int ItemManager :: EnumPointersByPartialName(const char *search, ITEMDEFPTR_ARRAY &resultList, int maxCount)
{
	int addCount = 0;
	ITEM_CONT::iterator it;
	for(it = ItemList.begin(); it != ItemList.end(); ++it)
	{
		if(it->second.mDisplayName.find(search) != string::npos)
		{
			resultList.push_back(&it->second);
			addCount++;
			if(addCount >= maxCount)
				return addCount;
		}
	}
	return addCount;
}

int ItemManager :: EnumPointersByPartialAppearance(const char *search, ITEMDEFPTR_ARRAY &resultList, int maxCount)
{
	int addCount = 0;
	ITEM_CONT::iterator it;
	for(it = ItemList.begin(); it != ItemList.end(); ++it)
	{
		if(it->second.mAppearance.find(search) != string::npos)
		{
			resultList.push_back(&it->second);
			addCount++;
			if(addCount >= maxCount)
				return addCount;
		}
	}
	return addCount;
}

int ItemManager :: EnumPointersByIDRange(ITEMDEFPTR_ARRAY &resultList, int minID, int maxID)
{
	ITEM_CONT::iterator start;
	ITEM_CONT::iterator end;
	start = ItemList.find(minID);
	if(start == ItemList.end())
		return 0;

	end = ItemList.find(maxID);
	if(end == ItemList.end())
		return 0;

	ITEM_CONT::iterator search = start;
	while(start != end)
	{
		resultList.push_back(&search->second);
		++search;
	}
	return (int)resultList.size();
} 

int ItemManager :: EnumPointersByRangeType(ITEMDEFPTR_ARRAY &resultList, int minID, int maxID, int eqType)
{
	ITEM_CONT::iterator start;
	ITEM_CONT::iterator end;
	start = ItemList.find(minID);
	if(start == ItemList.end())
		return 0;

	end = ItemList.find(maxID);
	if(end == ItemList.end())
		return 0;

	ITEM_CONT::iterator search = start;
	while(start != end)
	{
		if(start->second.mEquipType == eqType)
			resultList.push_back(&search->second);
		++search;
	}
	return (int)resultList.size();
}

int ItemManager :: EnumPointersByQuery(ITEMDEFPTR_ARRAY &resultList, ItemListQuery *queryData)
{
	ITEM_CONT::iterator istart = ItemList.begin();
	ITEM_CONT::iterator iend = ItemList.end();
	if(queryData->minID != ItemListQuery::FIELD_UNUSED)
	{
		istart = ItemList.find(queryData->minID);
		if(istart == ItemList.end())
			return 0;
	}
	if(queryData->maxID != ItemListQuery::FIELD_UNUSED)
	{
		iend = ItemList.find(queryData->maxID);
		if(iend == ItemList.end())
			return 0;
	}
	ITEM_CONT::iterator it;
	for(it = istart; it != iend; ++it)
	{
		ItemDef *item = &it->second;
		if(queryData->minLevel != ItemListQuery::FIELD_UNUSED)
			if(item->mMinUseLevel < queryData->minLevel)
				continue;
		if(queryData->maxLevel != ItemListQuery::FIELD_UNUSED)
			if(item->mMinUseLevel > queryData->maxLevel)
				continue;
		if(queryData->equipType != ItemListQuery::FIELD_UNUSED)
		{
			if(queryData->equipType != 0)
			{
				if(item->mEquipType != queryData->equipType) //Allow only specific type
					continue;
			}
			else
			{
				if(item->mWeaponType != 0)  //Exclude all weapons
					continue;
			}
		}
		if(queryData->weaponType != ItemListQuery::FIELD_UNUSED)
		{
			if(queryData->weaponType != 0)
			{
				if(item->mWeaponType != queryData->weaponType) //Allow only specific type
					continue;
			}
			else
			{
				if(item->mWeaponType == 0)  //Exclude all nonweapons
					continue;
			}
		}

		resultList.push_back(item);
	}
	return resultList.size();
}

int ItemManager :: RunPurchaseModifier(int itemID)
{
	//Find out 
	GambleDefinition *gDef = g_GambleManager.GetGambleDefByTriggerItemID(itemID);
	if(gDef == NULL)
		return itemID;

	vector<ItemDef*> searchList;
	switch(gDef->searchType)
	{
	case GambleManager::SEARCH_LINEARRANGE:
		g_ItemManager.EnumPointersByIDRange(searchList, gDef->params.minID, gDef->params.maxID);
		break;
	case GambleManager::SEARCH_EQUIP:
		g_ItemManager.EnumPointersByRangeType(searchList, gDef->params.minID, gDef->params.maxID, gDef->params.equipType);
		break;
	case GambleManager::SEARCH_QUERY:
		g_ItemManager.EnumPointersByQuery(searchList, &gDef->params);
		break;
	case GambleManager::SEARCH_LIST:
		return gDef->GetRandomSelection();
		break;
	}

	if(searchList.size() == 0)
	{
		g_Log.AddMessageFormat("RunPurchaseModifier() search list returned empty");
		return itemID;
	}

	int randIndex = randint(0, (int)searchList.size() - 1);
	if(randIndex < 0 || randIndex >= (int)searchList.size())
	{
		g_Log.AddMessageFormat("RunPurchaseModifier() index error.");
		return itemID;
	}
	return searchList[randIndex]->mID;
}

bool ItemManager :: CheckVirtualItemGamble(ItemDef *in_itemDef, int in_charLevel, VirtualItemSpawnParams *out_spawnParams, int &out_cost)
{
	if(in_itemDef == NULL)
		return false;

	ConfigString cfg(in_itemDef->Params);
	if(cfg.GetValueInt("gamble") == 0)
		return false;

	std::string dropRateProfile;
	int costperlevel = cfg.GetValueInt("cost_per_level");
	float dropratebonus = cfg.GetValueFloat("drop_rate_bonus");
	cfg.GetValueString("drop_rate_profile", dropRateProfile);

	if(in_charLevel < in_itemDef->mMinUseLevel)
		in_charLevel = in_itemDef->mMinUseLevel;

	int cost = in_charLevel * costperlevel;
	if(cost < in_itemDef->mValue)
		cost = in_itemDef->mValue;

	out_cost = cost;
	out_spawnParams->level = in_charLevel;
	out_spawnParams->minimumQuality = in_itemDef->mQualityLevel;
	out_spawnParams->mEquipType = in_itemDef->mEquipType;
	out_spawnParams->mWeaponType = in_itemDef->mWeaponType;
	out_spawnParams->SetAllDropMult(dropratebonus);

	out_spawnParams->dropRateProfile = &g_DropRateProfileManager.GetProfileByName(dropRateProfile);
	return true;
}

bool ItemManager :: IsWeaponTwoHanded(int mEquipType, int mWeaponType)
{
	if(mEquipType == ItemEquipType::WEAPON_2H)
		return true;

	if(mWeaponType == WeaponType::POLE)
		return true;

	if(mWeaponType == WeaponType::TWO_HAND)
		return true;

	return false;
}


int PrepExt_ItemDef(char *SendBuf, ItemDef *item, int ProtocolState)
{
	int WritePos = 0;

	//_handleItemDefUpdateMsg is [4] for lobby, [71] for play (most common) protocol
	char message = 71;
	if(ProtocolState == 0)
		message = 4;

	WritePos += PutByte(&SendBuf[WritePos], message);
	WritePos += PutShort(&SendBuf[WritePos], 0);      //Message size

	WritePos += PutInteger(&SendBuf[WritePos], item->mID);

	//Fill the item properties
	WritePos += PutByte(&SendBuf[WritePos], item->mType);
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mDisplayName.c_str());
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mAppearance.c_str());
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mIcon.c_str());

	WritePos += PutByte(&SendBuf[WritePos], item->mIvType1);
	WritePos += PutShort(&SendBuf[WritePos], item->mIvMax1);
	WritePos += PutByte(&SendBuf[WritePos], item->mIvType2);
	WritePos += PutShort(&SendBuf[WritePos], item->mIvMax2);
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mSv1.c_str());

	if(g_ProtocolVersion < 5)
		WritePos += PutInteger(&SendBuf[WritePos], item->_mCopper);

	WritePos += PutShort(&SendBuf[WritePos], item->mContainerSlots);
	WritePos += PutByte(&SendBuf[WritePos], item->mAutoTitleType);
	WritePos += PutShort(&SendBuf[WritePos], item->mLevel);
	WritePos += PutByte(&SendBuf[WritePos], item->mBindingType);
	WritePos += PutByte(&SendBuf[WritePos], item->mEquipType);

	WritePos += PutByte(&SendBuf[WritePos], item->mWeaponType);
	if(item->mWeaponType != 0)
	{
		if(g_ProtocolVersion == 7)
		{
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponDamageMin);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponDamageMax);
			WritePos += PutByte(&SendBuf[WritePos], item->_mSpeed);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamangeRating);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamageType);
		}
		else
		{
			WritePos += PutInteger(&SendBuf[WritePos], item->mWeaponDamageMin);
			WritePos += PutInteger(&SendBuf[WritePos], item->mWeaponDamageMax);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamangeRating);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamageType);
		}
	}

	WritePos += PutInteger(&SendBuf[WritePos], item->mEquipEffectId);
	WritePos += PutInteger(&SendBuf[WritePos], item->mUseAbilityId);
	WritePos += PutInteger(&SendBuf[WritePos], item->mActionAbilityId);
	WritePos += PutByte(&SendBuf[WritePos], item->mArmorType);
	if(item->mArmorType != 0)
	{
		if(g_ProtocolVersion == 7)
		{
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistMelee);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistFire);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistFrost);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistMystic);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistDeath);
		}
		else
		{
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistMelee);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistFire);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistFrost);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistMystic);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistDeath);
		}
	}
	if(g_ProtocolVersion == 7)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusStrength);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusDexterity);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusConstitution);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusPsyche);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusSpirit);
		WritePos += PutByte(&SendBuf[WritePos], item->_mBonusHealth);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusWill);
	}
	else
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusStrength);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusDexterity);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusConstitution);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusPsyche);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusSpirit);

		if(g_ProtocolVersion < 32)
			WritePos += PutInteger(&SendBuf[WritePos], item->_mBonusHealth);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusWill);
	}

	if(g_ProtocolVersion >= 4)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->isCharm);
		if(item->isCharm != 0)
		{
			WritePos += PutFloat(&SendBuf[WritePos], item->mMeleeHitMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMeleeCritMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMagicHitMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMagicCritMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mParryMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mBlockMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mRunSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mRegenHealthMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mAttackSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mCastSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mHealingMod);
		}
	}

	if(g_ProtocolVersion >= 5)
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->mValue);
		WritePos += PutByte(&SendBuf[WritePos], item->mValueType);
	}

	bool ItemUpdateDefMsgCraft = false;
	if(g_ProtocolVersion >= 7)
		ItemUpdateDefMsgCraft = true;
	if(ItemUpdateDefMsgCraft == true)
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->resultItemId);
		WritePos += PutInteger(&SendBuf[WritePos], item->keyComponentId);
		WritePos += PutInteger(&SendBuf[WritePos], item->numberOfItems);
		for(size_t i = 0; i < item->craftItemDefId.size(); i++)
			WritePos += PutInteger(&SendBuf[WritePos], item->craftItemDefId[i]);

		if(item->numberOfItems != item->craftItemDefId.size())
			g_Log.AddMessageFormatW(MSG_WARN, "[ERROR] Crafting material item count mismatch for ID: %d", item->mID);
	}

	if(g_ProtocolVersion >= 9)
		WritePos += PutStringUTF(&SendBuf[WritePos], item->mFlavorText.c_str());

	if(g_ProtocolVersion >= 18)
		WritePos += PutByte(&SendBuf[WritePos], item->mSpecialItemType);

	if(g_ProtocolVersion >= 30)
		WritePos += PutByte(&SendBuf[WritePos], item->mOwnershipRestriction);

	if(g_ProtocolVersion >= 31)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->mQualityLevel);
		WritePos += PutShort(&SendBuf[WritePos], item->mMinUseLevel);
	}

	PutShort(&SendBuf[1], WritePos - 3);
	return WritePos;
}
