#include "Config.h"
#include "DropTable.h"
#include "FileReader.h"
#include "StringList.h"
#include "ByteBuffer.h"
#include "Util.h" //For randint()
#include "Item.h"


DropTableManager g_DropTableManager;

namespace LootSystem
{
	void BuildAppearanceOverride(std::string &output, const char *prop1, float scale1, const char *prop2)
	{
		output = "{[\"a\"]=\"";
		output.append(prop1);
		output.append("\",[\"size\"]=");
		
		char convBuf[32];
		sprintf(convBuf, "%g", scale1);
		output.append(convBuf);
		output.append(",[\"type\"]=\"Scenery\"}|{[\"a\"]=\"");
		output.append(prop2);
		output.append("\",[\"size\"]=1,[\"type\"]=\"Scenery\"}");
	}

	const LootProp lootModels[7] = {
		{"Prop-Loot-Bag1",          1.0F  }, // 0
		{"Prop-Loot-Sack1",         1.0F  }, // 1
		{"Prop-Loot-Crate1",        1.0F  }, // 2
		{"Prop-Loot-Wood_Chest1",   1.1F  }, // 3
		{"Prop-Loot-Metal_Chest1",  1.2F  }, // 4
		{"Prop-Loot-Silver_Chest1", 1.2F  }, // 5
		{"Prop-Loot-Gold_Chest1",   1.2F  }, // 6
	};

	const char *tombstone = "Prop-Corpse_Marker";
	const char DefaultTombstoneAppearanceOverride[] = "{[\"a\"]=\"Prop-Tombstone1\",[\"size\"]=3,[\"type\"]=\"Scenery\"}|{[\"a\"]=\"Prop-Corpse_Marker\",[\"size\"]=1,[\"type\"]=\"Scenery\"}";

	const LootProp* getLootBag(int mType, int mQualityLevel)
	{
		//Hack for tokens and essences (basic) and crafting plans (recipe) to always return bags.
		if(mType == ItemType::BASIC)
			return &lootModels[0];
		else if(mType == ItemType::RECIPE)
			return &lootModels[0];

		switch(mQualityLevel)
		{
		case 6:  //Artifact
		case 5:  //Legend
		case 4:  //Epic
			return &lootModels[6];
		case 3:  //Rare
			return &lootModels[5];
		case 2:  //Uncommon
			return &lootModels[3];
		default:
			return &lootModels[0];
		}
	}

	int ComputeSharesByFraction(int denominator)
	{
		//Compute shares as expressed by this fraction:   1 / denominator
		//Passing a value of 10000 means "1 in 10000" which is more intuitive than the
		//actual percent chance of 0.01%
		
		// Uh... definitely not the right way
		//double mod = (1.0 / static_cast<double>(denominator)) * static_cast<double>(DROP_SHARES);
		//return static_cast<int>(mod);

		if(denominator <= 0)
			denominator = DROP_MAX_SHARES;

		return DROP_MAX_SHARES / denominator;
	}

	int ComputeSharesByPercent(float percent)
	{
		return static_cast<int>(percent * static_cast<float>(DROP_ONE_PERCENT));
	}
	float ComputePercentByShares(int shares)
	{
		return (float)((float)shares / (float)DROP_ONE_PERCENT);
	}
}

ActiveLootContainer :: ActiveLootContainer()
{
	CreatureID = 0;
}

ActiveLootContainer :: ActiveLootContainer(int creatureID)
{
	CreatureID = creatureID;
}

ActiveLootContainer :: ~ActiveLootContainer()
{
	itemList.clear();
	greeded.clear();
	needed.clear();
	passed.clear();
}

void ActiveLootContainer :: AddItem(int itemID)
{
	//Sanity check.  Too many items can cause buffer overflows when writing packet data.
	if(itemList.size() > 16)
	{
		g_Log.AddMessageFormat("[ERROR] Too many items added to container.");
		return;
	}
	if(itemID > 0)
		itemList.push_back(itemID);
}

int ActiveLootContainer :: HasItem(int itemID)
{
	for(size_t i = 0; i < itemList.size(); i++)
		if(itemList[i] == itemID)
			return i;
	return -1;
}

void ActiveLootContainer :: RemoveItem(int index)
{
	if(index < 0 || index >= (int)itemList.size())
		return;

	itemList.erase(itemList.begin() + index);
}

int ActiveLootContainer :: WriteLootQueryToBuffer(char *buffer, char *convbuf, int queryIndex)
{
	int wpos = 0;
	wpos += PutByte(&buffer[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&buffer[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&buffer[wpos], queryIndex);  //Query response index

	size_t lootCount = itemList.size();
	if(lootCount == 0)
	{
		wpos += PutShort(&buffer[wpos], 0);   //Row count
	}
	else
	{
		wpos += PutShort(&buffer[wpos], 1);             //Row count
		wpos += PutByte(&buffer[wpos], lootCount);      //String count
		for(size_t i = 0; i < lootCount; i++)
			wpos += PutStringUTF(&buffer[wpos], GetItemProto(convbuf, itemList[i], 0));
	}

	PutShort(&buffer[1], wpos - 3);                 //Message size
	return wpos;
}

void ActiveLootContainer :: AddLootableID(int newLootableID)
{
	for(size_t i = 0; i < lootableID.size(); i++)
		if(lootableID[i] == newLootableID)
			return;

	lootableID.push_back(newLootableID);
}

int ActiveLootContainer :: HasLootableID(int creatureID)
{
	for(size_t i = 0; i < lootableID.size(); i++)
		if(lootableID[i] == creatureID)
			return i;

	return -1;
}

void ActiveLootContainer :: DeleteAllLoot(void)
{
	for(size_t i = 0; i < itemList.size(); i++)
		g_ItemManager.NotifyDestroy(itemList[i], "DeleteAllLoot");
	itemList.clear();
}

int ActiveLootContainer :: GetItemCount(void) const
{
	return (int)itemList.size();
}

void ActiveLootContainer :: CopyLootContents(const ActiveLootContainer &source)
{
	CreatureID = source.CreatureID;
	itemList.assign(source.itemList.begin(), source.itemList.end());
	greeded.clear();
	greeded.insert(source.greeded.begin(), source.greeded.end());
	needed.clear();
	needed.insert(source.needed.begin(), source.needed.end());
	passed.clear();
	passed.insert(source.passed.begin(), source.passed.end());
}

void ActiveLootContainer :: Greed(int itemId, int creatureId)
{
	if(greeded.count(itemId) == 0) {
		set<int> a;
		greeded[itemId] = a;
	}
	greeded[itemId].insert(creatureId);
}

void ActiveLootContainer :: Need(int itemId, int creatureId)
{
	if(needed.count(itemId) == 0) {
		set<int> a;
		needed[itemId] = a;
	}
	needed[itemId].insert(creatureId);
}

void ActiveLootContainer :: Pass(int itemId, int creatureId)
{
	if(passed.count(itemId) == 0) {
		set<int> a;
		passed[itemId] = a;
	}
	passed[itemId].insert(creatureId);
}

bool ActiveLootContainer :: IsPassed(int itemId, int creatureId)
{
	return passed.count(itemId) >0 && passed[itemId].count(creatureId) > 0;
}

bool ActiveLootContainer :: IsNeeded(int itemId, int creatureId)
{
	return needed.count(itemId) >0 && needed[itemId].count(creatureId) > 0;
}

bool ActiveLootContainer :: IsGreeded(int itemId, int creatureId)
{
	return greeded.count(itemId) >0 && greeded[itemId].count(creatureId) > 0;
}

bool ActiveLootContainer :: HasAnyDecided(int creatureId)
{
	return Decided(creatureId, greeded) || Decided(creatureId, needed) || Decided(creatureId, passed);
}

bool ActiveLootContainer :: Decided(int creatureId, std::map<int, std::set<int> > map)
{
	return map.count(creatureId) > 0;
}

int ActiveLootContainer :: CountNeeds(int itemId)
{
	return needed.count(itemId) > 0 ? needed[itemId].size() : 0;
}

int ActiveLootContainer :: CountDecisions(int itemId)
{
	return ( greeded.count(itemId) > 0 ? greeded[itemId].size() : 0 ) +
			( needed.count(itemId) > 0 ? needed[itemId].size() : 0 ) +
			( passed.count(itemId) > 0 ? passed[itemId].size() : 0 );
}

int ActiveLootContainer :: Count(int itemId, std::map<int, int> map)
{
	int count = 0;
	typedef std::map<int, int>::iterator it_type;
	for(it_type iterator = map.begin(); iterator != map.end(); ++iterator) {
		if(iterator->first == itemId) {
			count++;
		}
	}
	return count;
}

WorldLootContainer :: WorldLootContainer()
{
}

WorldLootContainer :: ~WorldLootContainer()
{
}

void WorldLootContainer :: Clear(void)
{
	creatureList.clear();
}

int WorldLootContainer :: AttachLootToCreature(const ActiveLootContainer &loot, int CreatureID)
{
	if(loot.GetItemCount() == 0)
		return 0;

	creatureList[CreatureID].CopyLootContents(loot);
	return CreatureID;
}

int WorldLootContainer :: GetCreature(int creatureID)
{
	std::map<int, ActiveLootContainer>::iterator it;
	it = creatureList.find(creatureID);
	if(it == creatureList.end())
		return -1;

	return it->first;
}

void WorldLootContainer :: RemoveCreature(int creatureID)
{
	int r = GetCreature(creatureID);
	if(r >= 0)
	{
		creatureList[r].DeleteAllLoot();
		creatureList.erase(r);
	}
}

int WorldLootContainer :: WriteLootQueryToBuffer(int creatureID, char *buffer, char *convbuf, int queryIndex)
{
	return creatureList[creatureID].WriteLootQueryToBuffer(buffer, convbuf, queryIndex);
}


int randint_32bit(int min, int max)
{
	// Generate a 32 bit random number.

	/*
		Explanation:
		rand() doesn't work well for larger numbers.
		RAND_MAX is limited to 32767.
		There are other quirks, where powers of two seem to generate more even
		distributions of numbers.

		Since smaller numbers have better distribution, use a sequence
		of random numbers and use those to fill the bits of a larger number.
	*/

	// RAND_MAX (as defined with a value of 0x7fff) is only 15 bits wide.
	unsigned long rand_build = (rand() << 15) | rand();
	//unsigned long rand_build = ((rand() & 0xFF) << 24) | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 8) | ((rand() & 0xFF));
	return min + (rand_build % (max - min + 1));
}


DropSetDefinition :: DropSetDefinition()
{
	Clear();
}

void DropSetDefinition :: Clear(void)
{
	mName.clear();
	mRarity = CHANCE_PERCENT;
	mChance = 0;
	mItemList.clear();

	mClassFlag = 0;
}

void DropSetDefinition :: CopyFrom(const DropSetDefinition& other)
{
	mName = other.mName;
	mRarity = other.mRarity;
	mChance = other.mChance;
	mItemList.assign(other.mItemList.begin(), other.mItemList.end());
}

void DropSetDefinition :: AssignList(const char *data)
{
	std::string temp = data;
	STRINGLIST result;
	Util::Split(temp, ",", result);
	for(size_t i = 0; i < result.size(); i++)
		mItemList.push_back(atoi(result[i].c_str()));
}

void DropSetDefinition :: ResolveClassFlag(void)
{
	switch(mRarity)
	{
	case 2: mClassFlag = DropTableManager::CLASS_FLAG_UNCOMMON; break;
	case 3: mClassFlag = DropTableManager::CLASS_FLAG_RARE; break;
	case 4: mClassFlag = DropTableManager::CLASS_FLAG_EPIC; break;
	case 5: mClassFlag = DropTableManager::CLASS_FLAG_LEGENDARY; break;
	case 6: mClassFlag = DropTableManager::CLASS_FLAG_ARTIFACT; break;
	default: mClassFlag = DropTableManager::CLASS_FLAG_EXPLICIT; break;
	}
}

DropPackageDefinition :: DropPackageDefinition()
{
	Clear();
}

void DropPackageDefinition :: Clear(void)
{
	mName.clear();
	mAuto = 0;
	mMobFlags = FLAG_ALL;    //Default should include all mob types.
	mSetList.clear();

	mCombinedClassFlags = 0;
}

void DropPackageDefinition :: CopyFrom(const DropPackageDefinition& other)
{
	mName = other.mName;
	mAuto = other.mAuto;
	mMobFlags = other.mMobFlags;
	mSetList.assign(other.mSetList.begin(), other.mSetList.end());
}

void DropPackageDefinition :: AssignList(const char *data)
{
	std::string temp = data;
	STRINGLIST result;
	Util::Split(temp, ",", result);
	for(size_t i = 0; i < result.size(); i++)
		mSetList.push_back(result[i].c_str());
}

//Translate the string representation of flags to configure the bitfield.
void DropPackageDefinition :: TranslateFlags(const char *data)
{
	unsigned int mask = 0;
	bool combine = true;  //If false, negate instead of combine.

	size_t len = strlen(data);

	if(len == 0)
	{
		mMobFlags = FLAG_ALL;
		return;
	}

	for(size_t i = 0; i < len; i++)
	{
		switch(data[i])
		{
		case '*': combine = true; mask = FLAG_ALL; break;
		case '+': combine = true; break;
		case '-': combine = false; break;
		case 'a': mask |= FLAG_ALL_NORMAL; break;
		case 'A': mask |= FLAG_ALL_NAMED; break;
		case 'n': mask |= FLAG_NORMAL; break;
		case 'N': mask |= FLAG_NAMED_NORMAL; break;
		case 'h': mask |= FLAG_HEROIC; break;
		case 'H': mask |= FLAG_NAMED_HEROIC; break;
		case 'e': mask |= FLAG_EPIC; break;
		case 'E': mask |= FLAG_NAMED_EPIC; break;
		case 'l': mask |= FLAG_LEGENDARY; break;
		case 'L': mask |= FLAG_NAMED_LEGENDARY; break;
		}
	}
	if(combine == true)
		mMobFlags = mask;
	else
		mMobFlags = FLAG_ALL ^ mask;
}


DropCreatureDefinition :: DropCreatureDefinition()
{
	Clear();
}

void DropCreatureDefinition :: Clear(void)
{
	mCreatureDefID = 0;
	mExplicit = false;
	mSetList.clear();
}

//Merge the drop lists.  We do do this because definitions may span multiple row entries in the file
//and we want to aggregate that data instead of replace.
void DropCreatureDefinition :: ImportFrom(const DropCreatureDefinition& other)
{
	mCreatureDefID = other.mCreatureDefID;
	mExplicit = other.mExplicit;
	for(size_t i = 0; i < other.mSetList.size(); i++)
	{
		if(HasSet(other.mSetList[i]) == false)
			mSetList.push_back(other.mSetList[i]);
	}

	//Removed, used to be CopyFrom()
	//mSetList.assign(other.mSetList.begin(), other.mSetList.end());
}

bool DropCreatureDefinition :: HasSet(const std::string &search)
{
	for(size_t i = 0; i < mSetList.size(); i++)
		if(mSetList[i].compare(search) == 0)
			return true;
	return false;
}


void DropCreatureDefinition :: AssignList(const char *data)
{
	std::string temp = data;
	STRINGLIST result;
	Util::Split(temp, ",", result);
	for(size_t i = 0; i < result.size(); i++)
		mSetList.push_back(result[i].c_str());
}

DropLevelDefinition :: DropLevelDefinition()
{
	Clear();
}

void DropLevelDefinition :: Clear(void)
{
	mCombinedClassFlags = 0;
	mPackageList.clear();
}

DropTableManager :: DropTableManager()
{
}

DropTableManager :: ~DropTableManager()
{
}

void DropTableManager :: LoadSetFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[WARNING] Could not open file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	lfr.ReadLine();  //First line in the file is the table header

	DropSetDefinition entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("\t");
		if(r >= 4)
		{
			entry.mName = lfr.BlockToStringC(0, 0);
			entry.mRarity = lfr.BlockToIntC(1);
			entry.mChance = lfr.BlockToIntC(2);
			entry.AssignList(lfr.BlockToStringC(3, 0));
			mSet[entry.mName].CopyFrom(entry);
			entry.Clear();
		}
	}
	lfr.CloseCurrent();
}

void DropTableManager :: LoadPackageFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[WARNING] Could not open file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	lfr.ReadLine();  //First line in the file is the table header

	DropPackageDefinition entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("\t");
		if(r >= 4)
		{
			entry.mName = lfr.BlockToStringC(0, 0);
			entry.mAuto = lfr.BlockToIntC(1);
			entry.TranslateFlags(lfr.BlockToStringC(2, 0));
			entry.AssignList(lfr.BlockToStringC(3, 0));
			mPackage[entry.mName].CopyFrom(entry);
			entry.Clear();
		}
	}
	lfr.CloseCurrent();
}

void DropTableManager :: LoadCreatureFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormat("[WARNING] Could not open file [%s]", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	lfr.ReadLine();  //First line in the file is the table header

	DropCreatureDefinition entry;

	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak("\t");
		if(r >= 3)
		{
			entry.mCreatureDefID = lfr.BlockToIntC(0);
			entry.mExplicit = lfr.BlockToBoolC(1);
			entry.AssignList(lfr.BlockToStringC(2, 0));
			mCreature[entry.mCreatureDefID].ImportFrom(entry);
			entry.Clear();
		}
	}
	lfr.CloseCurrent();
}

void DropTableManager :: LoadData(void)
{
	char buffer[256];
	LoadSetFile(Platform::GenerateFilePath(buffer, "Loot", "Sets.txt"));
	LoadPackageFile(Platform::GenerateFilePath(buffer, "Loot", "Packages.txt"));
	LoadCreatureFile(Platform::GenerateFilePath(buffer, "Loot", "Creatures.txt"));

	ResolveClassFlags();
	ResolveAutotable();

	g_Log.AddMessageFormat("Loaded %d Drop Sets.", mSet.size());
	g_Log.AddMessageFormat("Loaded %d Drop Packages.", mPackage.size());
	g_Log.AddMessageFormat("Loaded %d Drop Creatures.", mCreature.size());
	g_Log.AddMessageFormat("Resolved %d Drop Levels.", mLevel.size());
}

//Resolves which item class (arbitrary drop rate or direct item mQualityLevel)
//is contained in each Set and Package.  Helps the drop system determine whether
//rolls for particular items or rarities are required.
void DropTableManager :: ResolveClassFlags(void)
{
	//Resolve the set flags first.  Packages reference multiple sets so iterate
	//through and combine them for the package flags.
	CSET::iterator sit;
	for(sit = mSet.begin(); sit != mSet.end(); ++sit)
		sit->second.ResolveClassFlag();

	CPACKAGE::iterator pit;
	for(pit = mPackage.begin(); pit != mPackage.end(); ++pit)
	{
		for(size_t i = 0; i < pit->second.mSetList.size(); i++)
		{
			sit = mSet.find(pit->second.mSetList[i]);
			if(sit != mSet.end())
				pit->second.mCombinedClassFlags |= sit->second.mClassFlag;
			else
				g_Log.AddMessageFormat("[WARNING] Drop Package [%s] contains an undefined set [%s]", pit->second.mName.c_str(), pit->second.mSetList[i].c_str());
		}
	}
}

//Scan for all packages assigned to a level and set up the necessary flags.
void DropTableManager :: ResolveAutotable(void)
{
	CPACKAGE::iterator it;
	for(it = mPackage.begin(); it != mPackage.end(); ++it)
	{
		if(it->second.mAuto <= 0)
			continue;
		mLevel[it->second.mAuto].mCombinedClassFlags |= it->second.mCombinedClassFlags;
		mLevel[it->second.mAuto].mPackageList.push_back(it->second.mName);
	}
}

//Roll all potential drops from the supplied parameters.  If a drop is rolled, add the
//item ID to the output vector.
void DropTableManager :: RollDrops(const DropRollParameters& params, std::vector<int>& output)
{
	//The creature table has highest priority and can override level drops.
	CCREATURE::iterator crit = mCreature.find(params.mCreatureDefID);

	SetQueryResult queryResults;

	bool useLevelPackage = true;
	if(crit != mCreature.end())
	{
		for(size_t i = 0; i < crit->second.mSetList.size(); i++)
			AddSetQueryResult(crit->second.mSetList[i], queryResults);

		if(crit->second.mExplicit == true)
			useLevelPackage = false;
	}
	if(useLevelPackage == true)
	{
		CLEVEL::iterator lvit;
		lvit = mLevel.find(params.mCreatureLevel);
		if(lvit != mLevel.end())
		{
			for(size_t i = 0; i < lvit->second.mPackageList.size(); i++)
				AddSetQueryResult(lvit->second.mPackageList[i], queryResults);
		}
	}
	
	if(queryResults.isEmpty() == true)
		return;

	int itemIndex = 0;
	
	int dropRoll;

	SetQueryResult::RESULT_LIST filter;
	if(queryResults.mClassFlags & DropTableManager::CLASS_FLAG_EXPLICIT)
	{
		queryResults.Filter(DropTableManager::CLASS_FLAG_EXPLICIT, filter);
		for(size_t i = 0; i < filter.size(); i++)
		{
			int max = 0, needed = 0;
			if(filter[i]->mRarity == DropSetDefinition::CHANCE_PERCENT)
			{
				needed = filter[i]->mChance;
				max = 100;
			}
			else if(filter[i]->mRarity == DropSetDefinition::CHANCE_SHARES)
			{
				needed = LootSystem::ComputeSharesByFraction(filter[i]->mChance);
				max = LootSystem::DROP_MAX_SHARES;
			}
			//If the chance uses a percent above 100, it can roll multiple times.
			//For for any other percent, or for drops based on shares, it will only roll one.
			while(needed > 0)
			{
				dropRoll = randint_32bit(1, max);
				if(dropRoll <= needed)
				{
					itemIndex = randint(0, filter[i]->mItemList.size() - 1);
					output.push_back(filter[i]->mItemList[itemIndex]);
				}

				/*  DEBUGGING
				else
				{
					g_Log.AddMessageFormat("Failed roll (%d,%d) [%s]: %d / %d (need: %d)", filter[i]->mRarity, filter[i]->mChance, filter[i]->mName.c_str(), dropRoll, max, needed);
				}
				*/
				needed -= max;
			}
		}
	}



	static const int classOrder[5] = {
		DropTableManager::CLASS_FLAG_ARTIFACT,
		DropTableManager::CLASS_FLAG_LEGENDARY,
		DropTableManager::CLASS_FLAG_EPIC,
		DropTableManager::CLASS_FLAG_RARE,
		DropTableManager::CLASS_FLAG_UNCOMMON,
	};

	static const int classShares[5] = {
		LootSystem::ComputeSharesByFraction(250000),  //artifact
		LootSystem::ComputeSharesByFraction(25000),   //legendary
		LootSystem::ComputeSharesByFraction(1200),    //1 in 1200 for epic
		LootSystem::ComputeSharesByFraction(300),     //1 in 300 for rare
		LootSystem::ComputeSharesByFraction(75),      //1 in 75 for uncommon
	};



	/*
	//BEGIN DEBUGGING DROP RATES
	int count[6];
	memset(count, 0, sizeof(count));
	int tests = 100000000;
	for(size_t s = 0; s < tests; s++)
	{
		dropRoll = randint_32bit(1, LootSystem::DROP_MAX_SHARES);
		for(size_t i = 0; i < 5; i++)
		{
			if(dropRoll > classShares[i])
				continue;
			
			count[i]++;
			break;
		}
	}
	for(int i = 0; i < 5; i++)
	{
		double pc = ((double)count[i] / (double)tests) * 100.0;
		double rpc = tests / classShares[i];
		g_Log.AddMessageFormat("[%d] QL=%d (needs:%d) : %d (%g) (%g)", i, classOrder[i], classShares[i], count[i], pc, rpc);
	}
	// END DEBUGGING
	*/


	//Find out how much to adjust the drop chance based on the level of the attacker
	int highestLevel = params.mPlayerLevel;
	if(params.mCreatureLevel > highestLevel)
		highestLevel = params.mCreatureLevel;

	int dcAdjust = LootSystem::DROP_MAX_SHARES;
	int n = highestLevel - 3;  //Offset to tip the chances higher for low levels.
	if(n < 1)
		n = 1;
	if(n < 35)
		dcAdjust = int((float)LootSystem::DROP_MAX_SHARES * ((float)n / 35));

	dropRoll = randint_32bit(1, dcAdjust);     // LootSystem::DROP_MAX_SHARES);
	
	for(size_t i = 0; i < 5; i++)   //For each rarity class (uncommon, rare, epic, legendary, artifact)
	{
		if(!(queryResults.mClassFlags & classOrder[i]))
			continue;

		if(dropRoll > classShares[i])    //Failed roll.
		{
			//g_Log.AddMessageFormat("[DROP] Failed roll (class:%d) (%d / %d)", i, dropRoll, classShares[i]);
			continue;
		}

		//Roll was a success, query the available drops for that rarity class.
		queryResults.Filter(classOrder[i], filter);
		if(filter.size() == 0)
		{
			g_Log.AddMessageFormat("[DROP] No sets found for class flag %d", classOrder[i]);
			continue;
		}

		//We have a list of sets.  Roll a set, then roll a specific item from that set.
		int setIndex = randint(0, filter.size() - 1);
		itemIndex = randint(0, filter[setIndex]->mItemList.size() - 1);
		output.push_back(filter[setIndex]->mItemList[itemIndex]);

		//We're checking in order from rarest to most common, if we don't break then it will
		//generate items from ALL pending rarity types.
		break;
	}
}

void DropTableManager :: AddSetQueryResult(const std::string &setOrPkgName, SetQueryResult &output)
{
	CSET::iterator setit;
	setit = mSet.find(setOrPkgName);
	if(setit != mSet.end())
	{
		output.AddSet(setit->second.mClassFlag, &setit->second);
		return;
	}

	CPACKAGE::iterator pkgit;
	pkgit = mPackage.find(setOrPkgName);
	if(pkgit != mPackage.end())
	{
		for(size_t i = 0; i < pkgit->second.mSetList.size(); i++)
		{
			setit = mSet.find(pkgit->second.mSetList[i]);
			if(setit != mSet.end())
			{
				output.AddSet(setit->second.mClassFlag, &setit->second);
			}
			else
				g_Log.AddMessageFormat("[WARNING] Unknown set [%s] referenced by package [%s]", pkgit->second.mSetList[i].c_str(), pkgit->second.mName.c_str());
		}
	}
}

SetQueryResult :: SetQueryResult()
{
	mClassFlags = 0;
}

void SetQueryResult :: AddSet(unsigned int classFlag, DropSetDefinition* ptrToSet)
{
	for(size_t i = 0; i < mSetListPtr.size(); i++)
		if(mSetListPtr[i] == ptrToSet)
			return;
	mSetListPtr.push_back(ptrToSet);
	mClassFlags |= classFlag;
	//g_Log.AddMessageFormat("[DEBUG] Drop set consideration [%s] [%d]", ptrToSet->mName.c_str(), classFlag);
}

bool SetQueryResult :: isEmpty(void)
{
	return (mSetListPtr.size() == 0);
}

int SetQueryResult :: Filter(unsigned int classFlag, RESULT_LIST& output)
{
	output.clear();
	for(size_t i = 0; i < mSetListPtr.size(); i++)
		if(mSetListPtr[i]->mClassFlag == classFlag)
			output.push_back(mSetListPtr[i]);
	return output.size();
}
