#pragma once
#ifndef CRAFTING_H
#define CRAFTING_H

#include <string>
#include <vector>
#include "ScriptCore.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

typedef vector<string> STRINGVECTOR;


class ItemDef;
class CraftManager;

class CraftInputSlot
{
public:
	unsigned int mCCSID;
	int mID;
	ItemDef *mItemDef;

	int mStackCount;
	CraftInputSlot(unsigned int CCSID, int id, int stackCount, ItemDef *itemDef);
	static bool SortComparator(const CraftInputSlot &lhs, const CraftInputSlot &rhs);
};

class CraftRecipe
{
	friend class CraftManager;  //Intended for internal use by this class.

private:
	string mDesc;
	size_t mInputCount;  //Exact number of distinct items, or stacks of items that recipe must have.
	size_t mOutputCount; //Exact number of output items.  Used to make sure the player has enough inventory space.
	STRINGVECTOR mConditions;
	STRINGVECTOR mActions;
	CraftRecipe()
	{
		mInputCount = 0;
		mOutputCount = 0;
	}

public:
	int GetOutputCount(void) const;  //The craft function needs to know externally whether there is enough space in the inventory.
	void GetRequiredItems(vector<int> &requiredItems) const;
	int GetRequiredItemCount(int itemID) const;
	const char *GetName(void) const;
};


class CraftManager
{
public:
	void LoadData(void);
	bool RunRecipe(const CraftRecipe* recipe, vector<CraftInputSlot> &inputItems, vector<CraftInputSlot> &outputItems);
	const CraftRecipe* GetRecipe(vector<CraftInputSlot> &inputItems);
	const CraftRecipe* GetFirstRecipeForResult(int resultItemID);
	
private:

	enum Comparator
	{
		CMP_NONE = 0,
		CMP_EQUAL,
		CMP_NOTEQUAL,
		CMP_LESS,
		CMP_LESSOREQUAL,
		CMP_GREATER,
		CMP_GREATEROREQUAL
	};
	static const int INVALID_PROPERTY = -1;

	vector<CraftRecipe> mRecipes;
	string mFileName;

	void SortInputs(vector<CraftInputSlot> &inputItems);
	void GenerateOutputs(const CraftRecipe *recipe, vector<CraftInputSlot> &inputItems, vector<CraftInputSlot> &outputItems);

	int GetIntParam(const STRINGVECTOR &strVector, size_t index);
	const char *GetStringParam(const STRINGVECTOR &strVector, size_t index);
	bool CheckCondition(const STRINGVECTOR &conditions, const vector<CraftInputSlot> &inputItems);
	int GetComparator(const char *symbol);
	int GetProperty(const CraftInputSlot &object, const char *propertyName);
	bool Compare(int leftValue, int compareOp, int rightValue);
	void LoadRecipeFile(const fs::path &filename);
};


extern CraftManager g_CraftManager;

#endif //CRAFTING_H
