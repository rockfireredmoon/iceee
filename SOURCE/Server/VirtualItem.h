#ifndef VIRTUALITEM_H
#define VIRTUALITEM_H

#include "Item.h"
#include "InstanceScale.h"
#include <string>
#include <map>

typedef std::vector<std::string> STRINGLIST;

class DropRateProfile;

struct ModRow
{
	static const int LOWEST_RARITY = 2;
	static const int NUMRARITY = 5;  //uncommon, rare, epic, legendary, artifact
	static const int NUMCOLUMN = 2;  //Number of data columns per rarity
	static const int TOTALCOLUMNS = 1 + (NUMRARITY * NUMCOLUMN);

	short level;
	short data[NUMRARITY][NUMCOLUMN];
	ModRow();
	void Clear(void);
	int FetchData(int rarity);
	int FetchFirst(int rarity);
	int FetchSecond(int rarity);
};

class ModTable
{
public:
	std::string name;
	std::vector<ModRow> modRow;
	ModTable();
	~ModTable();
	void Clear(void);
	void LoadFromFile(const char *filename);
	ModRow* GetRowByLevel(int level);
};

class ModManager
{
public:
	std::vector<ModTable> modTable;
	void LoadFromFile(const char *filename);
	ModTable* GetModTable(const char *name);
};

struct ValidTable
{
	char tableName[32];  //Name of the stat table to use.
	char statApp[32];    //Name(s) of the stats that this table will supply.
	int shares;          //Chance to roll this item in total with all other modifiers.
	bool core;           //This table distributes core stats.  Special behavior required.
	char maxApply;       //Maximum number of times this table may be rolled.
	ValidTable();
	void Clear(void);
	void FixDefaults(void);
};

struct EquipTemplate
{
	char mEquipType;     //Equivalent to ItemDef::mEquipType
	char mWeaponType;    //Equivalent to ItemDef::mWeaponType
	std::vector<ValidTable> intrinsicTable;  //A list of tables which are guaranteed to spawn on an item, and don't consume modifier slots.
	std::vector<ValidTable> alwaysTable;     //A list of tables which are guaranteed to spawn on an item, and consume slots.
	std::vector<ValidTable> randomTable;     //A list of tables which are randomly selected for application.
	EquipTemplate();
	void Clear(void);
};

class EquipTemplateManager
{
public:
	std::vector<EquipTemplate> equipTemplate;
	void LoadFromFile(const char *filename);
	EquipTemplate* GetTemplate(int equipType, int weaponType);
};

struct EquipAppearanceKey
{
	char mEquipType;
	char mWeaponType;
	char mQualityLevel;
	std::string asset;
	STRINGLIST dataList;

	EquipAppearanceKey();
	void Clear(void);
	bool isEqual(const EquipAppearanceKey& other) const;
	bool operator < (const EquipAppearanceKey& other) const
	{
		if(mEquipType < other.mEquipType)
			return true;
		else if(mEquipType == other.mEquipType)
		{
			if(mWeaponType < other.mWeaponType)
				return true;
			else if(mWeaponType == other.mWeaponType)
			{
				if(mQualityLevel < other.mQualityLevel)
					return true;
				else if(mQualityLevel == other.mQualityLevel)
				{
					if(asset.compare(other.asset) < 0)
						return true;
				}
			}
		}
		return false;
	}
};

class EquipAppearance
{
public:
	EquipAppearance();
	~EquipAppearance();
	typedef std::vector<EquipAppearanceKey> APPENTRY;
	typedef std::vector<EquipAppearanceKey*> SEARCHRESULT;
	APPENTRY dataEntry;
	void AddIfValid(EquipAppearanceKey& data);
	void LoadFromFile(const char *filename);
	void DebugSaveToFile(const char *filename);
	EquipAppearanceKey* GetOrCreate(EquipAppearanceKey& object);
	void SearchEntries(int equipType, int weaponType, SEARCHRESULT& results);
	void SearchEntriesWithAsset(int equipType, int weaponType, const char *asset, SEARCHRESULT& results);
	const char* GetRandomString(int preferredRarity, SEARCHRESULT& results);
	void Debug_CheckForNames(void);
	bool Debug_HasName(const char* asset, int mEquipType, int mWeaponType);
};

struct FileSymbol
{
	const char *name;
	int value;
};

int ResolveFileSymbol(const char *name, FileSymbol *set, int maxcount);

struct VirtualItemSpawnType
{
	int mEquipType;
	int mWeaponType;
	int shares;
	VirtualItemSpawnType();
	void Clear(void);
};

struct EquipTable
{
	std::vector<VirtualItemSpawnType> equipList;
	int maxShares;
	void LoadFromFile(const char *filename);
	void TallyMaxShares(void);
	VirtualItemSpawnType* GetRandomEntry(void);
	EquipTable();
};

struct NameEntry
{
	char name[24];
	unsigned char shares;
	NameEntry();
	void Clear(void);
};

struct NameTemplate
{
	static const char DEFAULT_NAME[];
	std::string assetName;
	short mEquipType;
	short mWeaponType;
	std::vector<NameEntry> nameList;
	short maxShares;
	NameTemplate();
	void Clear(void);
	void CountShareTotal(void);
	const char* GetRandomName(void);
	bool operator < (const NameTemplate& other) const
	{
		if(mEquipType < other.mEquipType)
			return true;
		else if(mEquipType == other.mEquipType)
		{
			if(mWeaponType < other.mWeaponType)
				return true;
			else if(mWeaponType == other.mWeaponType)
			{
				if(assetName.compare(other.assetName) < 0)
					return true;
			}
		}
		return false;
	}
};

class NameTemplateManager
{
public:
	std::vector<NameTemplate> nameTemplate;
	NameTemplateManager();
	~NameTemplateManager();
	void AddIfValid(NameTemplate &newItem);
	void LoadFromFile(const char *filename);
	void DebugSaveToFile(const char *filename);
	const char* RetrieveName(int mEquipType, int mWeaponType, const char* asset);
	const char* GetDefaultName(int mEquipType, int mWeaponType, const char *asset);
	bool Debug_HasName(const char* asset, int mEquipType, int mWeaponType);
};

struct NameModEntry
{
	int mType;
	std::string mStatBase;
	std::string mName;

	static const int TYPE_NONE = 0;
	static const int TYPE_PREFIX = 1;
	static const int TYPE_SUFFIX = 2;

	NameModEntry();
	void Clear(void);
};

class NameModManager
{
public:
	NameModManager();
	~NameModManager();

	std::vector<NameModEntry> mModList;
	void LoadFromFile(const char *filename);
	void ResolveModifiers(const char *firstStat, const char *secondStat, NameModEntry** firstName, NameModEntry** secondName);
};

struct NameWeight
{
	char mStat[32];
	float mWeight;
	NameWeight();
	void Clear(void);
};

struct NameWeightManager
{
	std::vector<NameWeight> mWeightList;
	void LoadFromFile(const char *filename);
	float GetWeightForName(const char *statName);
};

struct RarityConfig
{
	short mQualityLevel;
	short level;
	int chance;    //In the file these are loaded as a denominator for drop chances (1/X) but after the load stage are converted to shares (see DROP_SHARES).
	short statMax; //Maximum number of core stats out of the 5 possibilities (str/dex/con/psy/spi) that may receive points.
	short modMax;  //The total number of stats and modifiers that may be applied to an item.
	float levelBonus;
	float rollMin;
	float rollMax;
	short nullMod;
	float statPointMult;
	RarityConfig();
	void Clear(void);
	void CopyFrom(RarityConfig& source);
	int GetMinRoll(int numPoints);
	int GetMaxRoll(int numPoints);
	int GetAdjustedCorePoints(int numPoints);
};

class VirtualItemModSystem
{
public:
	static const int DROP_SHARES = 1000000;
	static const int DROP_PERCENT = DROP_SHARES / 100;

	static const int MIN_QUALITY_LEVEL = 2;
	static const int MAX_QUALITY_LEVEL = 6;
	RarityConfig rarityConfig[MAX_QUALITY_LEVEL + 1];
	void LoadFromFile(const char *filename);
	void UpdateRarityConfig(RarityConfig& entry);
	void LoadSettings(void);
	int GetDropRarity(const VirtualItemSpawnParams &params);
	void Debug_RunDropDiagnostic(int level);
};

struct VirtualItemSpawnParams
{
	int level;   //Required field for all spawns.  The target level that the item should be.
	int rarity;  //The creature rarity type of the mob that was killed.
	bool namedMob; //Set if the killed creature is a named mob.

	int mEquipType;      //Should only be set if the equipment type is predetermined.
	int mWeaponType;     //Should only be set if the equipment type is predetermined.
	int minimumQuality;  //Should only be set if an item MUST be rolled.  This quality will be chosen. even if all tier drop checks fail.

	DropRateProfile dropRateProfile; //How to determine drop rates from particular creatures.

	float dropMult[VirtualItemModSystem::MAX_QUALITY_LEVEL + 1];  //Optional drop rate multipliers to each quality level

	VirtualItemSpawnParams();
	void SetAllDropMult(float value);
	void ClampLimits(void);
};

extern ModManager g_ModManager;
extern EquipTemplateManager g_ModTemplateManager;
extern EquipAppearance g_EquipAppearance;
extern EquipAppearance g_EquipIconAppearance;
extern EquipTable g_EquipTable;
extern NameTemplateManager g_NameTemplateManager;
extern NameModManager g_NameModManager;
extern NameWeightManager g_NameWeightManager;
extern VirtualItemModSystem g_VirtualItemModSystem;

#endif  //#define VIRTUALITEM_H
