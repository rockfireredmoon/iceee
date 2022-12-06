#pragma once

#ifndef GAMBLE_H
#define GAMBLE_H

#include <vector>
#include <string>
#include <random>

struct ItemListQuery
{
	int minID;
	int maxID;
	char equipType;
	char weaponType;
	short minLevel;
	short maxLevel;
	static const int FIELD_UNUSED = -1;
	ItemListQuery()
	{
		Reset();
	}
	void Reset(void)
	{
		minID = FIELD_UNUSED;
		maxID = FIELD_UNUSED;
		equipType = FIELD_UNUSED;
		weaponType = FIELD_UNUSED;
		minLevel = FIELD_UNUSED;
		maxLevel = FIELD_UNUSED;
	}
	void CopyFrom(ItemListQuery *other)
	{
		minID = other->minID;
		maxID = other->maxID;
		equipType = other->equipType;
		weaponType = other->weaponType;
		minLevel = other->minLevel;
		maxLevel = other->maxLevel;
	}
};

struct GambleItemSelection
{
	int ItemID;
	int Shares;
	GambleItemSelection();
};

struct GambleDefinition
{
	char defName[20];
	int triggerItemID;
	int searchType;
	ItemListQuery params;
	std::vector<GambleItemSelection> itemSelection;

	GambleDefinition() { Clear(); }
	~GambleDefinition() { }
	void AddItemSelections(const char *itemList);
	void Clear(void);
	int GetRandomSelection(void);
};

class GambleManager
{
public:
	GambleManager();
	~GambleManager();

	enum SearchType
	{
		SEARCH_NONE        = 0,
		SEARCH_LINEARRANGE    ,  //Search an unweighted linear range.  (param1 = min, param2 = max)
		SEARCH_EQUIP          ,  //Search for a matching equipment type
		SEARCH_QUERY          ,  //Search using a set of query parameters.
		SEARCH_LIST              //Use a specific list of items, 
	};
	std::vector<GambleDefinition> defList;
	std::default_random_engine randomEngine;

	void LoadFile(std::string filename);
	int GetSearchType(const char *name);
	int GetStandardCount(void);
	GambleDefinition *GetGambleDefByTriggerItemID(int itemID);

	int Randint_32bit(int min, int max);
	int RandInt(int min, int max);
	int RandMod(int max);
	int RandI(int max);
	int RandModRng(int min, int max);
	double RandDbl(double min, double max);
};

extern GambleManager g_GambleManager;

#endif // GAMBLE_H
