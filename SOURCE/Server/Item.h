#pragma once
#ifndef ITEM_H
#define ITEM_H

#ifndef msizeof
  #define msizeof(s,m)   sizeof(((s *)0)->m)
#endif

#include <vector>
#include <string>
#include <map>
#include "FileReader.h"
#include "Entities.h"
#include "Report.h"  //For debugging
#include "json/json.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

enum SpecialItemType
{
	NONE,
	REAGENT_GENERATOR,
	ITEM_GRINDER,
	XP_BOOST,
	PORTABLE_REFASHIONER,
	PORTABLE_CRAFTKIT
};

extern const char *FLAVOR_TEXT_HEADER;
extern const char *FLAVOR_TEXT_FOOTER;

struct ItemListQuery;
struct ValidTable;
struct EquipTemplate;
struct NameModEntry;
struct VirtualItemSpawnParams;

struct ItemDef
{
	ItemDef();
	~ItemDef();

	int mID;  //Used internally for ItemDef lookups

	char mType;
	string mDisplayName;
	string mAppearance;
	string mIcon;

	char mIvType1;
	short mIvMax1;
	char mIvType2;
	short mIvMax2;
	string mSv1;

	int _mCopper;  //OBSOLETE     if(g_ProtocolVersion < 5)

	short mContainerSlots;
	char mAutoTitleType;
	short mLevel;
	char mBindingType;
	char mEquipType;
	char mWeaponType;

	int mWeaponDamageMin;  //(BYTE if g_ProtocolVersion == 7)
	int mWeaponDamageMax;  //(BYTE if g_ProtocolVersion == 7)
	char _mSpeed;          //OBSOLETE   (g_ProtocolVersion == 7) 
	char mWeaponExtraDamangeRating;
	char mWeaponExtraDamageType;

	int mEquipEffectId;
	int mUseAbilityId;
	int mActionAbilityId;
	char mArmorType;

	//if(g_ProtocolVersion == 7) then these are saved as BYTE
	int mArmorResistMelee;
	int mArmorResistFire;
	int mArmorResistFrost;
	int mArmorResistMystic;
	int mArmorResistDeath;

	//if(g_ProtocolVersion == 7) then these are saved as BYTE
	int mBonusStrength;
	int mBonusDexterity;
	int mBonusConstitution;
	int mBonusPsyche;
	int mBonusSpirit;
	int _mBonusHealth;  //OBSOLETE  Phased out in modern protocols since items give Con instead of health
	int mBonusWill;

	char isCharm;
	float mMeleeHitMod;
	float mMeleeCritMod;
	float mMagicHitMod;
	float mMagicCritMod;
	float mParryMod;
	float mBlockMod;
	float mRunSpeedMod;
	float mRegenHealthMod;
	float mAttackSpeedMod;
	float mCastSpeedMod;
	float mHealingMod;

	int mValue;
	char mValueType;

	//if(g_ProtocolVersion >= 7)
	int resultItemId;
	int keyComponentId;
	unsigned int numberOfItems;
	vector<int> craftItemDefId;   //needs to be variable depending on <numberOfItems>

	//if(g_ProtocolVersion >= 9)
	string mFlavorText;

	//this.SpecialItemType <- {
	//	NONE = 0,
	//	REAGENT_GENERATOR = 1,
	//	ITEM_GRINDER = 2,
	//	XP_BOOST = 3
	//};
	//this.SpecialItemTypeNameMapping <- {
	//	[this.SpecialItemType.NONE] = {
	//		name = "NONE"
	//	},
	//	[this.SpecialItemType.REAGENT_GENERATOR] = {
	//		name = "REAGENT_GENERATOR"
	//	},
	//	[this.SpecialItemType.ITEM_GRINDER] = {
	//		name = "ITEM_GRINDER"
	//	},
	//	[this.SpecialItemType.XP_BOOST] = {
	//		name = "XP_BOOST"
	//	}
	//};
	char mSpecialItemType;

	//if(g_ProtocolVersion >= 30)
	char mOwnershipRestriction;

	//if(g_ProtocolVersion >= 31)
	char mQualityLevel;
	short mMinUseLevel;

	string Params;   //Extended arbitrary parameters as a string of delimited key/value pairs.  Used exclusively by this server for advanced functions.

	void Reset();
	void CopyFrom(ItemDef *source);
	void ProcessParams(void);
	int GetDynamicMax(int type);

	bool operator < (const ItemDef &other) const
	{
		return (mID < other.mID);
	}
	bool operator < (int ID) const
	{
		return (mID < ID);
	}
	void Debug_WriteToStream(FILE *output);
	void Debug_WriteReport(ReportBuffer &report);
	void WriteToJSON(Json::Value &value);
};

class VirtualItem
{
public:
	ItemDef mStandardDef;
	string mModString;
	void InstantiateNew(int ID, int level, int rarity, int equipType, int weaponType);
	static string GetAppearanceAsset(string& mAppearance);
	void GenerateFlavorText(void);
	void ApplyIntrinsicTables(EquipTemplate* eqTemplate, int level, int rarity);
	void ApplyRandomTables(EquipTemplate* eqTemplate, int level, int rarity);
	void AppendStat(string& output, const char *name, int value);
	int ApplyModTable(ValidTable* validTable, int level, int rarity);
	void UpdateArmorType(void);
	void ApplyModString(void);
	void CheckNameMods(void);
	void ApplyNameMod(NameModEntry* mod);
	const char* TransformStandardMappings(const char* search);
	void ApplyStat(string& key, string& value);
	int RollStats(string& output, int numPoints, int rarity);
	void MergeStats(string& input, string& output);
	bool isItemDefStat(int statID);
};

class VirtualItemDef: public AbstractEntity {
public:
	int mID;
	char mType;
	char mEquipType;
	char mWeaponType;
	int mValue;
	short mLevel;
	char mQualityLevel;
	string mDisplayName;
	string mIcon;
	string mAppearance;
	string mModString;
	VirtualItemDef();
	void Clear(void);

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
};

struct ItemLoadTable
{
	int type;
	const char *loadName;
	short size;
	short offset;
};

enum ItemVariableEnum
{
	Item_Byte    = 0,
	Item_Short   = 1,
	Item_Integer = 2,
	Item_Float   = 3,
	Item_String  = 4,
	Item_IntVect = 5
};

enum ItemUpdateDefEnum
{
	ITEM_DEF       = 1,
	ITEM_LOOK_DEF  = 2,
	ITEM_CONTAINER = 4,
	ITEM_IV1       = 8,
	ITEM_IV2       = 16,
	ITEM_ID_CHANGE = 32,
	ITEM_ALL       = 255
};

struct ItemType
{
	enum Enum
	{
		UNKNOWN     = 0,
		SYSTEM      = 1,
		WEAPON      = 2,
		ARMOR       = 3,
		CHARM       = 4,
		CONSUMABLE  = 5,
		CONTAINER   = 6,
		BASIC       = 7,
		SPECIAL     = 8,
		QUEST       = 9,
		RECIPE      = 10,
		PROP 		= 11
	};
};

struct ItemIntegerType
{
	enum Enum
	{
		NONE				=	0,
		STACKING			=	1,
		DURABILITY			=	2,
		CHARGES				=	3,
		CAPACITY			=	4,
		QUEST_ID			=	5,
		RESULT_ITEM			=	6,
		KEY_COMPONENT		=	7,
		REQUIRE_ROLLING		=	8,
		LIFETIME			=	9,
		BONUS_VALUE			=	10,
		BOOK_PAGE			=   11,
		BOOK				=   12,
		BOOK_MATERIAL		=	13
	};
};

//Taken from the internal table.  These are the slots in the equipment screen that
//can be filled with equipped items.
struct ItemEquipSlot
{
	enum
	{
		NONE                = -1,
		WEAPON_MAIN_HAND    =  0,
		WEAPON_OFF_HAND     =  1,
		WEAPON_RANGED       =  2,
		ARMOR_HEAD          =  3,
		ARMOR_NECK          =  4,
		ARMOR_SHOULDER      =  5,
		ARMOR_CHEST         =  6,
		ARMOR_ARMS          =  7,
		ARMOR_HANDS         =  8,
		ARMOR_WAIST         =  9,
		ARMOR_LEGS          =  10,
		ARMOR_FEET          =  11,
		ARMOR_RING_L        =  12,
		ARMOR_RING_R        =  13,
		ARMOR_AMULET        =  14,
		FOCUS_FIRE          =  15,
		FOCUS_FROST         =  16,
		FOCUS_MYSTIC        =  17,
		FOCUS_DEATH         =  18,
		CONTAINER_0         =  19,
		CONTAINER_1         =  20,
		CONTAINER_2         =  21,
		CONTAINER_3         =  22,
		COSMETIC_SHOULDER_L =  23,
		COSMETIC_HIP_L      =  24,
		COSMETIC_SHOULDER_R =  25,
		COSMETIC_HIP_R      =  26,
		RED_CHARM           =  27,
		GREEN_CHARM         =  28,
		BLUE_CHARM          =  29,
		ORANGE_CHARM        =  30,
		YELLOW_CHARM        =  31,
		PURPLE_CHARM        =  32,
		MAX_SLOT
	};
};

struct ItemEquipType
{
	enum 
	{
		NONE				=	0,
		WEAPON_1H			=	1,
		WEAPON_1H_UNIQUE	=	2,
		WEAPON_1H_MAIN		=	3,
		WEAPON_1H_OFF		=	4,
		WEAPON_2H			=	5,
		WEAPON_RANGED		=	6,
		ARMOR_SHIELD		=	7,
		ARMOR_HEAD			=	8,
		ARMOR_NECK			=	9,
		ARMOR_SHOULDER		=	10,
		ARMOR_CHEST			= 	11,
		ARMOR_ARMS			=	12,
		ARMOR_HANDS			=	13,
		ARMOR_WAIST			=	14,
		ARMOR_LEGS			=	15,
		ARMOR_FEET			=	16,
		ARMOR_RING			=	17,
		ARMOR_RING_UNIQUE	=	18,
		ARMOR_AMULET		=	19,
		FOCUS_FIRE			=	20,
		FOCUS_FROST			=	21,
		FOCUS_MYSTIC		=	22,
		FOCUS_DEATH			=	23,
		CONTAINER			=	24,
		COSEMETIC_SHOULDER	=	25,
		COSEMETIC_HIP		=	26,
		RED_CHARM			=	27,
		GREEN_CHARM			=	28,
		BLUE_CHARM			=	29,
		ORANGE_CHARM		=	30,
		YELLOW_CHARM		=	31,
		PURPLE_CHARM		=	32
	};
};

struct WeaponType
{
	enum
	{
		NONE			=	0,
		SMALL			=	1,
		ONE_HAND		=	2,
		TWO_HAND		=	3,
		POLE			=	4,
		WAND			=	5,
		BOW				=	6,
		THROWN			=	7,
		ARCANE_TOTEM	=	8
	};
};

//Corresponds to the mArmorType property.  Values taken from the client entries.  This is mostly
//defunct since armor weight classes are no longer used.  However, it does have one important
//property, SHIELD, which is required for shield type items to properly display on the character's
//back.
struct ArmorType
{
	enum
	{
		NONE    = 0,
		CLOTH   = 1,
		LIGHT   = 2,
		MEDIUM  = 3,
		HEAVY   = 4,
		SHIELD  = 5
	};
};


namespace WeaponTypeClassRestrictions
{
	bool isValid(int weaponType, int profession);
}

enum ItemUpdateEnum
{
	FLAG_ITEM_BOUND          = 1,
	FLAG_ITEM_TIME_REMAINING = 2,
	FLAG_ALL                 = 255
};

struct InventoryMappingDef
{
	const char *name;
	int ID;
};

enum ItemBindingType
{
	BIND_NEVER			=	0,
	BIND_ON_PICKUP		=	1,
	BIND_ON_EQUIP		=	2
};

//Note: should keep these mapped to InventoryMapping.
//This is the index into the player container array.  Each of those container
//arrays holds an array of items.
enum StandardContainerEnum
{
	INV_CONTAINER = 0,
	EQ_CONTAINER = 1,
	BANK_CONTAINER = 2,
	BUYBACK_CONTAINER = 4,
	STAMPS_CONTAINER = 7,
	DELIVERY_CONTAINER = 8,
	AUCTION_CONTAINER = 9,
	BOOKSHELF_CONTAINER = 10
};

#define INV_BASESLOTS   24    //All characters start with 24 slots.

extern const int InventoryMappingCount;
extern const int EquipmentMappingCount;
//extern char * InventoryMapping[];
extern InventoryMappingDef InventoryMapping[];
extern InventoryMappingDef EquipmentMapping[];

const unsigned long CONTAINER_ID = 0xFFFF0000;
const unsigned long CONTAINER_SLOT = 0x0000FFFF;

extern const int maxItemLoadDef;
extern ItemLoadTable ItemLoadDef[];

//extern vector<ItemDef> ItemList;

int GetContainerIDFromName(const char *name);
string GetContainerNameFromID(int ID);
bool IsContainerIDValid(int ID);

int GetEQSlotFromName(char *name);

int SetItemProperty(ItemDef *item, const char *name, const char *value);
int LoadItemFromStream(FileReader &fr, ItemDef *itemDef, string debugFilename);
unsigned long GetIDSlot(unsigned long ID, unsigned long slot);
char *GetItemProto(char *convbuf, int ItemID, int count);

extern const char * Map_EquipType[];
extern const char * Map_ArmorType[];
extern const char * Map_WeaponType[];
const char *GetEquipType(int Type);
const char *GetArmorType(int Type);
const char *GetWeaponType(int Type);

class ItemManager
{
public:
	typedef map<int, ItemDef> ITEM_CONT;
	typedef vector<ItemDef*> ITEMDEFPTR_ARRAY;
	typedef map<int, VirtualItem> VITEM_CONT;
	typedef pair<int, VirtualItem> VITEM_PAIR;

	ItemManager();
	~ItemManager();
	void Free();
	
	static const int BASE_VIRTUAL_ITEM_ID = 5000000;

	ITEM_CONT ItemList;
	VITEM_CONT VItemList;

	void LoadData(void);

	ItemDef *GetDefaultItemPtr(void);
	ItemDef *GetPointerByID(int ID);
	ItemDef *GetSafePointerByID(int ID);
	ItemDef *GetSafePointerByExactName(const char *name);
	ItemDef *GetSafePointerByPartialName(const char *name);
	VirtualItem * GetVirtualItem(int ID);

	int RunPurchaseModifier(int itemID);
	bool CheckVirtualItemGamble(ItemDef *in_itemDef, int in_charLevel, VirtualItemSpawnParams *out_spawnParams, int &out_cost);

	int EnumPointersByPartialName(const char *search, ITEMDEFPTR_ARRAY &resultList, int maxCount);
	int EnumPointersByPartialAppearance(const char *search, ITEMDEFPTR_ARRAY &resultList, int maxCount);
	int EnumPointersByIDRange(ITEMDEFPTR_ARRAY &resultList, int minID, int maxID);
	int EnumPointersByRangeType(ITEMDEFPTR_ARRAY &resultList, int minID, int maxID, int eqType);
	int EnumPointersByQuery(ITEMDEFPTR_ARRAY &resultList, ItemListQuery *queryData);

	int GetStandardCount(void);
	int CreateNewVirtualItem(int rarity, VirtualItemSpawnParams &viParams);
	int GetNewVirtualItemID(void);
	void AddVirtualItemDef(VirtualItemDef& vid);
	void LoadVirtualItemWithID(int itemID);
	void ExpandVirtualItem(const VirtualItemDef& item);
	void InsertVirtualItem(const VirtualItem& item);

	int RollVirtualItem(VirtualItemSpawnParams &viParams);
	void NotifyDestroy(int itemID, const char *debugReason);

	static bool IsWeaponTwoHanded(int mEquipType, int mWeaponType);

private:
	void Sort(void);
	void Finalize(void);

	void LoadItemList(const fs::path &filename, bool itemOverride);
	void LoadItemPackages(const fs::path &listFile, bool itemOverride);
};

extern ItemManager g_ItemManager;

int PrepExt_ItemDef(char *SendBuf, ItemDef *item, int ProtocolState);

#endif //ITEM_H
