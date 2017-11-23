// Implements the data sets and assignment properties for item set bonuses (additional bonuses
// when multiple pieces of an equipment set are worn by a player).

#ifndef ITEMSET_H
#define ITEMSET_H

#include <string>
#include <vector>
#include <map>

#include "Creature.h"
#include "ScriptCore.h"

class CreatureInstance;
class ReportBuffer;
class ItemSetManager;

class ItemSetData
{
public:
	std::string mSetName;        //The set name.  Also used as the script name to call.  Interpreted as a script label to call.  Must not contain spaces.
	int mMinItem;                //The player must equip at least this many items in order for any set bonuses to be examined and applied. 
	std::string mItemList;       //A comma separated list of ItemDef IDs that belong to this set.
	std::string mFlavorText;     //Optional flavor text to append to an item that belongs to a set.

	ItemSetData();
	void Clear(void);
	void CopyFrom(const ItemSetData &source);
};


namespace ItemSetScript
{
	
enum ItemSetScriptExtOpCodes
{
	OP_ADD = OP_MAXCOREOPCODE,
	OP_ADD_F,
};


class ItemSetScriptDef : public ScriptCore::ScriptDef
{
	friend class ItemSetManager;  //Need to expose the script to allow advanced debugging like label lookups and instruction scans.
private:
	virtual void GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize);
};

class ItemSetScriptPlayer : public ScriptCore::ScriptPlayer
{
	virtual void RunImplementationCommands(int opcode);

public:
	CreatureInstance *attachedCreature;
	void DebugGenerateReport(ReportBuffer &report);
};

} //namespace ItemSet

struct ItemSetTally
{
	std::map<int, int> mItemIDCount;           //Tallies how many times an item has been counted.  This is to prevent dual rings from counting as two equipped set items.
	std::map<std::string, int> mSetNameCount;  //Counts how many items are equipped that belong to a particular set.  Does not count multiples (such as rings).
	void TallyItem(int itemID, const std::string &setName);
};



// Stores which items are used in which sets.
class ItemSetManager
{
public:
	void LoadData(void);                                //Load all item set data.
	void CheckItem(int itemID, ItemSetTally &output);  //Check if the item belongs to a set.  If it does, increment the output tally.
	void UpdateCreatureBonuses(ItemSetTally &tally, CreatureInstance *actor);  //Scan the item set tally, and apply any bonuses if applicable.

private:
	std::map<int, std::string> mRegisteredItemID;        //Maps item IDs (int) to a script name (string).
	std::map<std::string, ItemSetData> mRegisteredSets;  //Maps set names to the information of a specific set.
	ItemSetScript::ItemSetScriptDef mScriptDef;
	
	void LoadFile(std::string filename);                 //Loads a table file defining sets to be registered.
	void UpdateFlavorText(void);
	void RegisterSet(const ItemSetData &data);
	void RegisterItem(int itemID, const std::string &setName);  //Register an item into a set.
	const ItemSetData* GetSetData(const std::string &setName);
};

extern ItemSetManager g_ItemSetManager;

#endif //#ifndef ITEMSET_H
