#include "Item.h"
#include "ItemSet.h"
#include "DirectoryAccess.h"
#include "FileReader3.h"
#include "StringList.h"
#include "CommonTypes.h"
#include "Util.h"

ItemSetManager g_ItemSetManager;

void ItemSetTally :: TallyItem(int itemID, const std::string &setName)
{
	if(mItemIDCount[itemID]++ == 1)
	{
		mSetNameCount[setName]++;
	}
}


ItemSetData::ItemSetData()
{
	Clear();
}

void ItemSetData::Clear(void)
{
	mSetName.clear();
	mMinItem = 0;
	mItemList.clear();
	mFlavorText.clear();
}

void ItemSetData::CopyFrom(const ItemSetData &source)
{
	mSetName = source.mSetName;
	mMinItem = source.mMinItem;
	mItemList = source.mItemList;
	mFlavorText = source.mFlavorText;
}

//If the Item ID is registered to a set, then tally the result.
void ItemSetManager::CheckItem(int itemID, ItemSetTally &tally)
{
	std::map<int, std::string>::iterator it;
	it = mRegisteredItemID.find(itemID);
	if(it != mRegisteredItemID.end())
	{
		tally.TallyItem(itemID, it->second);
	}
}

void ItemSetManager::LoadData(void)
{
	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "ItemSet.txt");
	LoadFile(filename.c_str());

	Platform::GenerateFilePath(filename, "Data", "ItemSetScript.txt");
	mScriptDef.CompileFromSource(filename.c_str());
	
	UpdateFlavorText();
	
	g_Log.AddMessageFormat("Loaded %d Item Sets", mRegisteredSets.size());
}

void ItemSetManager::LoadFile(const char *filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename) != FileReader3::SUCCESS)
	{
		g_Log.AddMessageFormat("[ERROR] Could not open file [%s]", filename);
		return;
	}
	fr.SetCommentChar(';');
	fr.ReadLine();  //Assume the first line is a header
	ItemSetData entry;
	while(fr.Readable())
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r >= 3)
		{
			entry.mSetName = fr.BlockToStringC(0);
			entry.mMinItem = fr.BlockToIntC(1);
			entry.mItemList = fr.BlockToStringC(2);
			entry.mFlavorText = fr.BlockToStringC(3);

			RegisterSet(entry);
			entry.Clear();
		}
	}
	fr.CloseFile();
}

void ItemSetManager::RegisterSet(const ItemSetData &data)
{
	mRegisteredSets[data.mSetName].CopyFrom(data);
	STRINGLIST itemIDs;
	Util::Split(data.mItemList, ",", itemIDs);
	for(size_t i = 0; i < itemIDs.size(); i++)
	{
		int ID = atoi(itemIDs[i].c_str());
		RegisterItem(ID, data.mSetName);
	}
	std::map<int, std::string> mRegisteredItemID;        //Maps item IDs (int) to a script name (string).
	std::map<std::string, ItemSetData> mRegisteredSets;  //Maps set names to the information of a specific set.

}

void ItemSetManager::RegisterItem(int itemID, const std::string &setName)
{
	mRegisteredItemID[itemID] = setName;
}

void ItemSetManager::UpdateCreatureBonuses(ItemSetTally &tally, CreatureInstance *actor)
{
	if(actor == NULL)
		return;
	
	std::map<std::string, int>::const_iterator it;
	for(it = tally.mSetNameCount.begin(); it != tally.mSetNameCount.end(); ++it)
	{
		const ItemSetData *setData = GetSetData(it->first);
		if(setData == NULL)
			continue;

		int itemCount = it->second;
		if(itemCount < setData->mMinItem)
			continue;

		ItemSetScript::ItemSetScriptPlayer script;
		script.Initialize(&mScriptDef);
		script.attachedCreature = actor;

		std::string labelName = setData->mSetName;
		labelName.append(".");
		Util::StringAppendInt(labelName, itemCount);

		if(script.JumpToLabel(labelName.c_str()) == true)
		{
			script.RunScript();
		}
	}
}

void ItemSetManager::UpdateFlavorText(void)
{
	std::map<int, std::string>::iterator it;
	for(it = mRegisteredItemID.begin(); it != mRegisteredItemID.end(); ++it)
	{
		const ItemSetData *setData = GetSetData(it->second);
		if(setData == NULL)
			continue;

		if(setData->mFlavorText.size() == 0)
			continue;

		ItemDef *itemDef = g_ItemManager.GetPointerByID(it->first);
		if(itemDef == NULL)
		{
			g_Log.AddMessageFormat("[ERROR] Item ID [%d] does not exist", it->first);
			continue;
		}
		itemDef->mFlavorText.append(setData->mFlavorText);
	}
}

const ItemSetData* ItemSetManager::GetSetData(const std::string &setName)
{
	std::map<std::string, ItemSetData>::iterator it;
	it = mRegisteredSets.find(setName);
	if(it == mRegisteredSets.end())
		return NULL;
	return &it->second;
}

namespace ItemSetScript
{

	OpCodeInfo extCoreOpCode[] = {
	{ "add",                  OP_ADD,              2, {OPT_STR,  OPT_INT,  OPT_NONE }},
	{ "add_f",                OP_ADD_F,            2, {OPT_STR,  OPT_STR,  OPT_NONE }},
};

const int maxExtOpCode = COUNT_ARRAY_ELEMENTS(extCoreOpCode);

void ItemSetScriptDef::GetExtendedOpCodeTable(OpCodeInfo **arrayStart, size_t &arraySize)
{
	*arrayStart = ItemSetScript::extCoreOpCode;
	arraySize = ItemSetScript::maxExtOpCode;
}

void ItemSetScriptPlayer :: RunImplementationCommands(int opcode)
{
	ScriptCore::OpData *in = &def->instr[curInst];
	switch(opcode)
	{
	case OP_ADD:
		{
			const char *statName = GetStringTableEntry(in->param1);
			float amount = static_cast<float>(in->param2);
			StatDefinition *statDef = GetStatDefByName(statName);
			if(statDef != NULL)
			{
				attachedCreature->AddItemStatMod(0, statDef->ID, amount);
			}
		}
		break;
	case OP_ADD_F:
		{
			const char *statName = GetStringTableEntry(in->param1);
			const char *statAmount = GetStringTableEntry(in->param2);
			float amount = Util::GetFloat(statAmount);
			StatDefinition *statDef = GetStatDefByName(statName);
			if(statDef != NULL)
			{
				attachedCreature->AddItemStatMod(0, statDef->ID, amount);
			}
		}
		break;
	default:
		ScriptCore::PrintMessage("Unidentified op type: %d", in->opCode);
		break;
	}
}

} //namespace ItemSet