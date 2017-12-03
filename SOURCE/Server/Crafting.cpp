#include "Crafting.h"

#include "Item.h"
#include "StringList.h"
#include "Util.h"
#include "stdio.h"
#include "FileReader3.h"
#include <algorithm>
#include "DirectoryAccess.h"
#include "ScriptCore.h"

/*

idcount  int:itemID int:count                  At least one slot must contain <count> number of itemID.  Fails if itemID not found, and count does not match.
idmult   int:itemID int:mult                   At least one slot must contain a multiple of a certain number of itemID.  Fails if itemID not found, and modulo division does not result in zero remainder.
idmultof int:itemID int:itemID  int:mult       The slot with the first itemID must have a multiple count of the slot with the second ID.  Fails if either itemID is not found, or modulo division does not result in zero remainder.
getslotbyid int:itemID var:dest                Return the slot index 



requireid int:itemID int:itemCount                 Must have a specific count of a specific item.
requireidxmult                                     Must have a specific count of a specific item.

inputItemIndex requireid itemID itemCount
inputItemIndex requireidxmult itemID checkIndex mult
inputItemIndex requireidmult itemID itemCount
inputItemIndex itemtype typeVal
inputItemIndex quality quality
inputItemIndex cmp leftValue comparator rightValue


conditions
inputItemIndex requireid itemID itemCount
inputItemIndex requireidxmult itemID checkIndex mult
inputItemIndex requireidmult itemID itemCount
inputItemIndex itemtype typeVal
inputItemIndex quality quality
inputItemIndex cmp leftValue comparator rightValue

actions
giveid itemID itemCount
giveidxmult itemID checkIndex mult
giveidxdiv itemID checkIndex div

*/
/*
OpCodeInfo extCoreOpCode[] = {
{ "requireid",        OP_COND_REQUIREID,        2, {OPT_INT,  OPT_INT,  OPT_NONE }},
{ "requireidxmult",   OP_COND_REQUIREIDXMULT,   3, {OPT_INT,  OPT_INT,  OPT_INT  }},
{ "requireidmult",    OP_COND_REQUIREIDMULT,    2, {OPT_INT,  OPT_INT,  OPT_NONE }},
{ "itemtype",         OP_COND_ITEMTYPE,         1, {OPT_INT,  OPT_NONE, OPT_NONE }},
{ "quality",          OP_COND_QUALITY,          1, {OPT_INT,  OPT_NONE, OPT_NONE }},
{ "cmp",              OP_COND_CMP,              3, {OPT_INT,  OPT_INT,  OPT_INT  }},

{ "giveid",           OP_ACT_GIVEID,            2, {OPT_INT,  OPT_INT,  OPT_NONE }},
{ "giveidxmult",      OP_ACT_GIVEIDXMULT,       3, {OPT_INT,  OPT_INT,  OPT_INT  }},
{ "giveidxdiv",       OP_ACT_GIVEIDXDIVT,       3, {OPT_INT,  OPT_INT,  OPT_INT  }},
};
*/

CraftManager g_CraftManager;

int CraftRecipe::GetOutputCount(void) const
{
	return mOutputCount;
}
const char *CraftRecipe::GetName(void) const
{
	return mDesc.c_str();
}

int CraftRecipe::GetRequiredItemCount(int itemID) const
{
	for(size_t i = 0; i < mConditions.size(); i++)
	{
		STRINGVECTOR args;
		Util::Split(mConditions[i], ",", args);
		if(args.size() <= 2)
			continue;

		if(strcmp(args[1].c_str(), "requireid") == 0)
		{
			return atoi(args[3].c_str());
		}
	}
	return 0;
}

void CraftRecipe::GetRequiredItems(std::vector<int> &requiredItems) const
{
	for(size_t i = 0; i < mConditions.size(); i++)
	{
		STRINGVECTOR args;
		Util::Split(mConditions[i], ",", args);
		if(args.size() <= 2)
			continue;

		if(strcmp(args[1].c_str(), "requireid") == 0)
		{
			requiredItems.push_back(atoi(args[2].c_str()));
		}
	}
}

CraftInputSlot::CraftInputSlot(unsigned int CCSID, int id, int stackCount, ItemDef *itemDef)
{
	mCCSID = CCSID;
	mID = id;
	mStackCount = stackCount;
	mItemDef = itemDef;
}
bool CraftInputSlot::SortComparator(const CraftInputSlot &lhs, const CraftInputSlot &rhs)
{
	if(lhs.mItemDef == NULL)
		return false;
	if(rhs.mItemDef == NULL)
		return false;

	if(lhs.mItemDef->mType < rhs.mItemDef->mType)
	{
		return true;
	}
	else if(lhs.mItemDef->mType == rhs.mItemDef->mType)
	{
		return lhs.mItemDef->mID < rhs.mItemDef->mID;
	}
	return false;
}

void CraftManager::LoadData(void)
{
	mRecipes.clear();  //Just in case we're reloading for debug purposes.

	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "CraftDef.txt");
	LoadRecipeFile(filename.c_str());

	g_Log.AddMessageFormat("Loaded %d crafting recipes.", mRecipes.size());
}

void CraftManager::LoadRecipeFile(const char *filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename) != FileReader3::SUCCESS)
	{
		g_Log.AddMessageFormat("Error opening crafting recipe file:%s", filename);
		return;
	}
	fr.SetCommentChar(';');
	fr.ReadLine(); //Assume first line is header.
	while(fr.Readable() == true)
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r >= 2)
		{
			CraftRecipe entry;
			entry.mDesc = fr.BlockToStringC(0);
			entry.mInputCount = fr.BlockToIntC(1);
			entry.mOutputCount = fr.BlockToIntC(2);

			std::string line;

			line = fr.BlockToStringC(3);
			Util::TrimWhitespace(line);
			Util::ToLowerCase(line);
			Util::Split(line, "|", entry.mConditions);

			line = fr.BlockToStringC(4);
			Util::TrimWhitespace(line);
			Util::ToLowerCase(line);
			Util::Split(line, "|", entry.mActions);

			mRecipes.push_back(entry);
		}
	}
	fr.CloseFile();
}

int CraftManager::GetIntParam(const STRINGVECTOR &strVector, size_t index)
{
	if(index >= strVector.size())
		return 0;
	
	return atoi(strVector[index].c_str());
}

const char *CraftManager::GetStringParam(const STRINGVECTOR &strVector, size_t index)
{
	if(index >= strVector.size())
		return NULL;
	
	return strVector[index].c_str();
}


const CraftRecipe* CraftManager::GetRecipe(std::vector<CraftInputSlot> &inputItems)
{
	SortInputs(inputItems);
	size_t inputCount = inputItems.size();
	for(size_t i = 0; i < mRecipes.size(); i++)
	{
		CraftRecipe *recipe = &mRecipes[i];
		if(recipe->mInputCount != inputCount)
			continue;
		if(CheckCondition(recipe->mConditions, inputItems) == true)
			return recipe;
	}
	return NULL;
}

const CraftRecipe* CraftManager::GetFirstRecipeForResult(int resultItemID)
{
	for(size_t i = 0; i < mRecipes.size(); i++)
	{
		CraftRecipe *recipe = &mRecipes[i];

		for(size_t i = 0; i < recipe->mActions.size(); i++)
		{
			STRINGVECTOR args;
			Util::Split(recipe->mActions[i], ",", args);
			if(args.size() == 0)
				continue;
			if(args[0].compare("giveid") == 0)
			{
				int itemID = GetIntParam(args, 1);
				int itemCount = GetIntParam(args, 2);
				if(itemID == resultItemID)
					return recipe;
			}
		}
	}
	return NULL;
}

bool CraftManager::CheckCondition(const STRINGVECTOR &conditions, const std::vector<CraftInputSlot> &inputItems)
{
	size_t passed = 0;
	for(size_t i = 0; i < conditions.size(); i++)
	{
		STRINGVECTOR args;
		Util::Split(conditions[i], ",", args);
		if(args.size() <= 2)
			continue;
		int itemIndex = GetIntParam(args, 0);
		if(itemIndex < 0 || itemIndex >= (int)inputItems.size())
			return false;

		const char *op = GetStringParam(args, 1);
		if(op == NULL)
			return false;

		if(strcmp(op, "requireid") == 0)
		{
			int itemID = GetIntParam(args, 2);
			int itemCount = GetIntParam(args, 3);
			if(inputItems[itemIndex].mID != itemID)
				return false;
			if((itemCount >= 0) && (inputItems[itemIndex].mStackCount != itemCount))
				return false;

			passed++;
		}
		else if(strcmp(op, "requireidxmult") == 0)
		{
			int itemID = GetIntParam(args, 2);
			int checkIndex = GetIntParam(args, 3);
			int mult = GetIntParam(args, 4);
			if(mult < 1)
				mult = 1;
			
			if(inputItems[itemIndex].mID != itemID)
				return false;
			if(checkIndex < 0 || checkIndex >= (int)inputItems.size())
				return false;
			if(inputItems[itemIndex].mStackCount != inputItems[checkIndex].mStackCount * mult)
				return false;

			passed++;
		}
		else if(strcmp(op, "requireidmult") == 0)
		{
			int itemID = GetIntParam(args, 2);
			int itemCount = GetIntParam(args, 3);
			if(inputItems[itemIndex].mID != itemID)
				return false;
			if((inputItems[itemIndex].mStackCount % itemCount) != 0)
				return false;

			passed++;
		}
		else if(strcmp(op, "itemtype") == 0)
		{
			int typeVal = GetIntParam(args, 2);
			if(inputItems[itemIndex].mItemDef->mType != typeVal)
				return false;
			passed++;
		}
		else if(strcmp(op, "quality") == 0)
		{
			int quality = GetIntParam(args, 2);
			if(inputItems[itemIndex].mItemDef->mQualityLevel != quality)
				return false;
			passed++;
		}
		else if(strcmp(op, "cmp") == 0)
		{
			int leftValue = GetProperty(inputItems[itemIndex], GetStringParam(args, 2));
			if(leftValue == INVALID_PROPERTY)
				return false;
			int comparator = GetComparator(GetStringParam(args, 3));
			int rightValue = GetIntParam(args, 4);
			if(Compare(leftValue, comparator, rightValue) == false)
				return false;
			passed++;
		}
		else
		{
			g_Log.AddMessageFormat("[ERROR] CraftManager::CheckCondition unknown condition [%s]", op);
		}
	}
	if(passed == conditions.size())
		return true;

	return false;
}

//Return true if the recipe conditions were successful and an output list 
bool CraftManager::RunRecipe(const CraftRecipe* recipe, std::vector<CraftInputSlot> &inputItems, std::vector<CraftInputSlot> &outputItems)
{
	SortInputs(inputItems);

	if(recipe == NULL)
		recipe = GetRecipe(inputItems);

	if(recipe == NULL)
		return false;  //Still nothing? No match.

	GenerateOutputs(recipe, inputItems, outputItems);
	if(outputItems.size() > 0)
		return true;
	
	return false;
}

void CraftManager::GenerateOutputs(const CraftRecipe *recipe, std::vector<CraftInputSlot> &inputItems, std::vector<CraftInputSlot> &outputItems)
{
	for(size_t i = 0; i < recipe->mActions.size(); i++)
	{
		STRINGVECTOR args;
		Util::Split(recipe->mActions[i], ",", args);
		if(args.size() == 0)
			continue;
		if(args[0].compare("giveid") == 0)
		{
			int itemID = GetIntParam(args, 1);
			int itemCount = GetIntParam(args, 2);
			if(itemCount <= 0)
				itemCount = 1;
			outputItems.push_back(CraftInputSlot(0, itemID, itemCount, NULL));
		}
		else if(args[0].compare("giveidxmult") == 0)
		{
			int itemID = GetIntParam(args, 1);
			int checkIndex = GetIntParam(args, 2);
			int mult = GetIntParam(args, 3);
			if(mult < 1)
				mult = 1;
			if(checkIndex < 0 || checkIndex >= inputItems.size())
				continue;
			int count = inputItems[checkIndex].mStackCount * mult;
			outputItems.push_back(CraftInputSlot(0, itemID, count, NULL));
		}
		else if(args[0].compare("giveidxdiv") == 0)
		{
			int itemID = GetIntParam(args, 1);
			int checkIndex = GetIntParam(args, 2);
			int div = GetIntParam(args, 3);
			if(div < 1)
				div = 1;
			if(checkIndex < 0 || checkIndex >= inputItems.size())
				continue;
			int count = inputItems[checkIndex].mStackCount / div;
			if(count > 0)
				outputItems.push_back(CraftInputSlot(0, itemID, count, NULL));
		}
		else
		{
			g_Log.AddMessageFormat("[ERROR] CraftManager::CheckCondition unknown action [%s]", args[0].c_str());
		}
	}
}

void CraftManager::SortInputs(std::vector<CraftInputSlot> &inputItems)
{
	std::sort(inputItems.begin(), inputItems.end(), CraftInputSlot::SortComparator);
}

int CraftManager::GetComparator(const char *symbol)
{
	static const char *symbols[6] = { "=",       "!=",         "<",      "<=",            ">",         ">="               };
	static const int   values[6]  = { CMP_EQUAL, CMP_NOTEQUAL, CMP_LESS, CMP_LESSOREQUAL, CMP_GREATER, CMP_GREATEROREQUAL };
	for(int i = 0; i < 6; i++)
	{
		if(strcmp(symbol, symbols[i]) == 0)
			return values[i];
	}
	return CMP_NONE;
}

int CraftManager::GetProperty(const CraftInputSlot &object, const char *propertyName)
{
	if(strcmp(propertyName, "level") == 0) return object.mItemDef->mLevel;
	if(strcmp(propertyName, "quality") == 0) return object.mItemDef->mQualityLevel;
	if(strcmp(propertyName, "type") == 0) return object.mItemDef->mType;
	if(strcmp(propertyName, "weapontype") == 0) return object.mItemDef->mWeaponType;
	if(strcmp(propertyName, "armortype") == 0) return object.mItemDef->mArmorType;
	if(strcmp(propertyName, "equiptype") == 0) return object.mItemDef->mEquipType;
	
	g_Log.AddMessageFormat("[ERROR] CraftManager::GetProperty unknown propert name [%s]", propertyName);
	return INVALID_PROPERTY;
}

bool CraftManager::Compare(int leftValue, int compareOp, int rightValue)
{
	switch(compareOp)
	{
	case CMP_EQUAL:          return leftValue == rightValue;
	case CMP_NOTEQUAL:       return leftValue != rightValue;
	case CMP_LESS:           return leftValue < rightValue;
	case CMP_LESSOREQUAL:    return leftValue <= rightValue;
	case CMP_GREATER:        return leftValue > rightValue;
	case CMP_GREATEROREQUAL: return leftValue >= rightValue;
	}
	return false;
}


/*
conditions
requireid int:slot int:itemID int:itemCount
requireidxmult int:slot int:itemID int:checkIndex int:mult
requireidmult int:slot int:itemID int:itemCount
itemtype int:slot int:typeVal
quality int:slot int:quality
cmp int:slot val:leftValue cmp:comparator val:rightValue

actions
giveid int:itemID int:itemCount
giveidxmult int:itemID int:checkIndex int:mult
giveidxdiv int:itemID int:checkIndex int:div
*/
