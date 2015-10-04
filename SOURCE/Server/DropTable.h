/*  The drop system.

	When a drop is calculated, look up the drop table for a given creature.
	For that creature, roll for package.
	Within that package, roll for item.

*/

#ifndef DROPTABLE_H
#define DROPTABLE_H

#include <vector>
#include <map>
#include <set>
#include <string>

class CreatureInstance;

// An active container of loot.  Each lootable entity will
// have an attached loot container, which lists the items
// available for pickup.
class ActiveLootContainer
{
public:
	ActiveLootContainer();
	ActiveLootContainer(int creatureID);
	~ActiveLootContainer();
	int CreatureID;              //The owner of this container.  Ex: The ID of the dead creature.
	int robinID;				 //The ID of the current round robin for party loot
	std::map<int, bool> stage2Map; //Whether the looting is on it's second stage (i.e. robin offered/leader offered)
	std::vector<int> lootableID; //List of CreatureDefIDs that may loot this creature.
	std::vector<int> itemList;   //List of items left in this container.
	std::map<int, std::set<int> > greeded;   //Map of ItemIDs and CreatureIDs that have greeded this loot.
	std::map<int, std::set<int> > needed;    //Map of ItemIDs and CreatureIDs that have needed this loot.
	std::map<int, std::set<int> > passed;    //Map of ItemIDs and CreatureIDs that have passed on this loot.

	bool IsStage2(int itemId);
	void SetStage2(int itemId, bool stage2);
	void RemoveAllRolls();
	bool IsPassed(int itemId, int looterCreatureId);
	bool IsNeeded(int itemId, int looterCreatureId);
	bool IsGreeded(int itemId, int looterCreatureId);
	bool HasAnyDecided(int itemId, int looterCreatureId);
	void AddItem(int itemID);
	void RemoveCreatureRolls(int itemId, int looterCreatureId);
	void RemoveCreatureRollsFromMap(int itemId, int looterCreatureId, std::map<int, std::set<int> > * map);
	int CountDecisions(int itemId);
	void Greed(int itemId, int looterCreatureId);
	void Pass(int itemId, int looterCreatureId);
	void Need(int itemId, int looterCreatureId);
	int HasItem(int itemID);
	bool ContainsTag(int lootTag);
	void RemoveItem(int index);
	int WriteLootQueryToBuffer(char *buffer, char *convbuf, int queryIndex);
	void AddLootableID(int newLootableID);
	int HasLootableID(int creatureID);
	void DeleteAllLoot();
	int GetItemCount(void) const;
	void CopyLootContents(const ActiveLootContainer &source);
private:
	bool Decided(int itemId, int creatureId, std::map<int, std::set<int> > * map);
	int Count(int itemId, std::map<int, std::set<int> > * map);
};


// The loot manager.  Each map instance has its own loot manager.
// This central method performs all processing, such as creating
// new active loot, or looking up existing containers.
class WorldLootContainer
{
public:
	WorldLootContainer();
	~WorldLootContainer();

	std::map<int, ActiveLootContainer> creatureList;

	int AttachLootToCreature(const ActiveLootContainer &loot, int CreatureID);
	int GetCreature(int creatureID);
	void RemoveCreature(int creatureID);
	ActiveLootContainer * GetLootTag(int lootTag);
	void Clear(void);

	int WriteLootQueryToBuffer(int creatureID, char *buffer, char *convbuf, int queryIndex);
};


//Holds a collection of items grouped by a common theme.  Ideally a set should not mix items
//of different rarities or levels unless for very specific reasons, so that drops rates may
//be properly calculated.
struct DropSetDefinition
{
	std::string mName;            //Name of this set.  Required so that packages can reference this collection of items.
	int mRarity;                  //Corresponds to ItemDef mQualityLevel.  Note special cases Zero and -1.  Zero means that mChance is an explicit chance (100 = 100%).  -1 indicates the rate should be interpreted as a percentage.
	int mChance;                  //Custom drop chance, if applicable.
	std::vector<int> mItemList;
	static const int CHANCE_PERCENT = 0;    //If mRarity is this, mChance calculates drop chances based on integer percents (1-100, or higher for multiple drop chances)
	static const int CHANCE_SHARES  = -1;   //If mRarity is this, mChance is used as a denominator to calculate specific drop changes (ex: 1 / 20 = 5% chance)
	
	unsigned int mClassFlag;     //This is resolved at load time.  Should only have 1 flag set.

	DropSetDefinition();
	void Clear(void);
	void CopyFrom(const DropSetDefinition& other);
	void AssignList(const char *data);
	void ResolveClassFlag(void);
};

struct DropPackageDefinition
{
	std::string mName;            //Name of this package.
	int mAuto;                    //If above zero, this package will be automatically added into a level-specific drop table.
	unsigned int mMobFlags;       //Flags that determine the mob types that can drop from this package.
	std::vector<std::string> mSetList;  //A list of sets dropped by this package.

	unsigned int mCombinedClassFlags;     //This is resolved at load time.

	enum Flags
	{
		FLAG_NORMAL          = (1 << 0),
		FLAG_HEROIC          = (1 << 1),
		FLAG_EPIC            = (1 << 2),
		FLAG_LEGENDARY       = (1 << 3),
		FLAG_NAMED_NORMAL    = (1 << 4),
		FLAG_NAMED_HEROIC    = (1 << 5),
		FLAG_NAMED_EPIC      = (1 << 6),
		FLAG_NAMED_LEGENDARY = (1 << 7),

		FLAG_ALL_NORMAL = FLAG_NORMAL | FLAG_HEROIC | FLAG_EPIC | FLAG_LEGENDARY,
		FLAG_ALL_NAMED = FLAG_NAMED_NORMAL | FLAG_NAMED_HEROIC | FLAG_NAMED_EPIC | FLAG_NAMED_LEGENDARY,

		FLAG_ALL = FLAG_ALL_NORMAL | FLAG_ALL_NAMED
	};

	DropPackageDefinition();
	void Clear(void);
	void CopyFrom(const DropPackageDefinition& other);
	void AssignList(const char *data);
	void TranslateFlags(const char *data);

};

struct DropCreatureDefinition
{
	int mCreatureDefID;                  //Creature to apply to.
	bool mExplicit;                      //If true, automatic level-based packages will not be rolled, instead only using sets defined here.  If false, these sets will drop in addition to automatic level packages.
	std::vector<std::string> mSetList;   //A list of sets (not packages!) that this creature may drop.

	DropCreatureDefinition();
	void Clear(void);
	void ImportFrom(const DropCreatureDefinition& other);
	void AssignList(const char *data);
	bool HasSet(const std::string &search);
};

struct DropLevelDefinition
{
	unsigned int mCombinedClassFlags;
	std::vector<std::string> mPackageList;

	DropLevelDefinition();
	void Clear(void);
};

struct DropRollParameters
{
	int mCreatureLevel;
	int mCreatureDefID;
	int mPlayerLevel;
};

//Holds the information for a package/set lookup.  Packages will be resolved into a list
//of sets.  This holds the cumulative bit flag information from the search results so
//that when sets must be selected, it already knows what it can and can't roll.
struct SetQueryResult
{
	typedef std::vector<DropSetDefinition*> RESULT_LIST;
	unsigned int mClassFlags;
	RESULT_LIST mSetListPtr;
	SetQueryResult();
	void AddSet(unsigned int classFlag, DropSetDefinition* ptrToSet);
	bool isEmpty(void);
	int Filter(unsigned int classFlag, RESULT_LIST& output);
};

class DropTableManager
{
public:
	//Containers for each type.
	typedef std::map<std::string, DropSetDefinition> CSET;
	typedef std::map<std::string, DropPackageDefinition> CPACKAGE;
	typedef std::map<int, DropCreatureDefinition> CCREATURE;
	typedef std::map<int, DropLevelDefinition> CLEVEL;

	CSET mSet;
	CPACKAGE mPackage;
	CCREATURE mCreature;
	CLEVEL mLevel;

	static const int CLASS_FLAG_EXPLICIT = 1;    //Contains a set with an explicit drop rate.
	static const int CLASS_FLAG_UNCOMMON = 2;    //Contains a set of item mQualityLevel=2
	static const int CLASS_FLAG_RARE = 4;        //Contains a set of item mQualityLevel=3
	static const int CLASS_FLAG_EPIC = 8;        //Contains a set of item mQualityLevel=4
	static const int CLASS_FLAG_LEGENDARY = 16;  //Contains a set of item mQualityLevel=5
	static const int CLASS_FLAG_ARTIFACT = 32;   //Contains a set of item mQualityLevel=6

	DropTableManager();
	~DropTableManager();

	void LoadData(void);
	void RollDrops(const DropRollParameters& params, std::vector<int>& output);

private:
	//These load functions are called separately by the LoadData() function and don't need
	//to be accessed anywhere else.
	void LoadSetFile(const char *filename);
	void LoadPackageFile(const char *filename);
	void LoadCreatureFile(const char *filename);

	void ResolveClassFlags(void);
	void ResolveAutotable(void);
	void AddSetQueryResult(const std::string &setOrPkgName, SetQueryResult &output);
};


namespace LootSystem
{
	struct LootProp
	{
		const char *propName;
		float size;
	};
	void BuildAppearanceOverride(std::string &output, const char *prop1, float scale1, const char *prop2);
	extern const LootProp lootModels[7];
	extern const char *tombstone;
	extern const char DefaultTombstoneAppearanceOverride[];

	const LootProp* getLootBag(int mType, int mQualityLevel);

	//Drops use integral values to simulate percentage chances.  A high number of drop shares
	//increases the precision to allow fine-tuned drop rates and bonuses.
	//NOTE: 2^24 (=16777216) is the maximum INTEGER that can be safely converted to FLOAT and back
	//without losing precision.  The DropProfile class uses such conversions.
	static const int DROP_MAX_SHARES = 1000000;   //Maximum shares.  Equivalent to 100% chance.
	static const int DROP_ONE_PERCENT = DROP_MAX_SHARES / 100;

	int ComputeSharesByFraction(int denominator);
	int ComputeSharesByPercent(float percent);
	float ComputePercentByShares(int shares);
}

extern DropTableManager g_DropTableManager;

int randint_32bit(int min, int max);

#endif //DROPTABLE_H
