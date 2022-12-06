#include "EliteMob.h"
#include "DirectoryAccess.h"
#include "FileReader3.h"

#include "Creature.h"
#include "Config.h"
#include "Random.h"
#include "util/Log.h"

EliteManager g_EliteManager;

EliteType :: EliteType()
{
	Clear();
}

void EliteType :: Clear(void)
{
	mDisplayName.clear();
	mChance = 0;
	mMinLevel = -1;
	mMaxLevel = -1;
	mMinAffix = 0;
	mMaxAffix = 0;
	mAffixGroup.clear();
	mBonusExp = 0;
	mBonusDrop = 0;
}


AffixEntry :: AffixEntry()
{
	Clear();
}

void AffixEntry :: Clear(void)
{
	mInternalName.clear();
	mDisplayName.clear();
	mMinLevel = -1;
	mMaxLevel = -1;
	mStatName.clear();
	mOperation = 0;
	mAdjustValue = 0.0F;
	mNextAffix.clear();
	mBonusExp = 0;
	mBonusDrop = 0;
}


EliteManager :: EliteManager()
{
}

EliteManager :: ~EliteManager()
{
}

void EliteManager :: LoadData(void)
{
	//Clear existing entries just in case we have an external function to reload the tables.
	mEliteType.clear();
	mAffixEntry.clear();

	LoadTypeTable(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "EliteType.txt"));
	LoadAffixTable(Platform::JoinPath(Platform::JoinPath(g_Config.ResolveStaticDataPath(), "Data"), "EliteAffix.txt"));

	g_Logs.data->info("Loaded %v EliteType", mEliteType.size());
	g_Logs.data->info("Loaded %v EliteAffix", mAffixEntry.size());
}

void EliteManager :: LoadTypeTable(std::string filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename.c_str()) != FileReader3::SUCCESS)
	{
		g_Logs.data->error("Could not open file: %v", filename);
		return;
	}
	fr.SetCommentChar(';');
	fr.ReadLine(); //Assume the first line is a header.

	EliteType entry;

	while(fr.Readable())
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r > 1)
		{
			entry.mDisplayName = fr.BlockToStringC(0);
			entry.mChance = fr.BlockToIntC(1);
			entry.mMinLevel = fr.BlockToIntC(2);
			entry.mMaxLevel = fr.BlockToIntC(3);
			entry.mMinAffix = fr.BlockToIntC(4);
			entry.mMaxAffix = fr.BlockToIntC(5);
			entry.mAffixGroup = fr.BlockToStringC(6);
			entry.mBonusExp = fr.BlockToIntC(7);
			entry.mBonusDrop = fr.BlockToIntC(8);
			mEliteType.push_back(entry);
			entry.Clear();
		}
	}
	fr.CloseFile();
}

void EliteManager :: LoadAffixTable(std::string filename)
{

	FileReader3 fr;
	if(fr.OpenFile(filename.c_str()) != FileReader3::SUCCESS)
	{
		g_Logs.data->error("Could not open file: %v", filename);
		return;
	}
	fr.SetCommentChar(';');
	fr.ReadLine(); //Assume the first line is a header.

	AffixEntry entry;

	while(fr.Readable())
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r > 1)
		{
			entry.mInternalName = fr.BlockToStringC(0);
			entry.mDisplayName = fr.BlockToStringC(1);
			entry.mMinLevel = fr.BlockToIntC(2);
			entry.mMaxLevel = fr.BlockToIntC(3);
			entry.mStatName = fr.BlockToStringC(4);
			entry.mOperation = fr.BlockToStringC(5)[0];
			entry.mAdjustValue = fr.BlockToFloatC(6);
			entry.mNextAffix = fr.BlockToStringC(7);
			entry.mBonusExp = fr.BlockToIntC(8);
			entry.mBonusDrop = fr.BlockToIntC(9);
			mAffixEntry.push_back(entry);
			entry.Clear();
		}
	}
	fr.CloseFile();
}

void EliteManager :: QueryType(int level, std::vector<EliteType*> &outputResults)
{
	for(size_t i = 0; i < mEliteType.size(); i++)
	{
		if(level >= mEliteType[i].mMinLevel && level <= mEliteType[i].mMaxLevel)
			outputResults.push_back(&mEliteType[i]);
	}
}

void EliteManager :: QueryAffix(const std::string &name, int level, std::vector<AffixEntry*> &outputResults)
{
	if(name.size() == 0)
		return;

	//Group searches are a broad match
	bool group = (name[0] == '#');
		
	for(size_t i = 0; i < mAffixEntry.size(); i++)
	{
		if(name.compare(mAffixEntry[i].mInternalName) == 0)
		{
			//if((group == true) || (level >= mAffixEntry[i].mMinLevel && level <= mAffixEntry[i].mMinLevel))
			outputResults.push_back(&mAffixEntry[i]);
			if(group == false)
				return;
		}
	}
}

void EliteManager :: QueryAffixesByLevel(int level, std::vector<AffixEntry*> &outputResults)
{
	for(size_t i = 0; i < mAffixEntry.size(); i++)
	{
		if((level >= mAffixEntry[i].mMinLevel && level <= mAffixEntry[i].mMaxLevel))
			outputResults.push_back(&mAffixEntry[i]);
	}
}

void EliteManager :: ApplyTransformation(CreatureInstance *creature)
{
	int level = creature->css.level;

	std::vector<EliteType*> types;
	QueryType(level, types);

	if(types.size() == 0)
		return;

	//Determine the kind of elite this this, and load the intrinsic affixes that belong to that type.
	const EliteType *typeSel = types[g_RandomManager.RandInt(0, types.size() - 1)];
	if(typeSel->mChance == 0)
		return;
	int chance = g_RandomManager.RandInt(1, typeSel->mChance);
	if(chance > 1)  //No spawn.
		return;

	const std::string &affixGroup = typeSel->mAffixGroup;
	ProcessAffixes(level, creature, affixGroup, 0);
	int bonusExp = typeSel->mBonusExp;
	int bonusDrop = typeSel->mBonusDrop;

	std::vector<AffixEntry*> randomAffixes;
	QueryAffixesByLevel(level, randomAffixes);
	/* DEBUG, NO LONGER NEEDED
	for(size_t i = 0; i < randomAffixes.size(); i++)
		g_Log.AddMessageFormat("Query [%d]=%s", i, randomAffixes[i]->mInternalName.c_str());
	*/

	int affixCount = 0;
	int maxAffix = g_RandomManager.RandInt(typeSel->mMinAffix, typeSel->mMaxAffix);
	STRINGLIST appliedAffixNames;
	while(affixCount < maxAffix && randomAffixes.size() > 0)
	{
		size_t affixSel = g_RandomManager.RandInt(0, randomAffixes.size() - 1);
		AffixEntry *affix = randomAffixes[affixSel];

		if(affix->mDisplayName.size() > 0)
			AppendInfoNameTo(affix->mDisplayName, appliedAffixNames);

		bonusExp += affix->mBonusExp;
		bonusDrop += affix->mBonusDrop;

		ApplyAffixStat(affix, creature);
		
		if(affix->mNextAffix.size() > 0)
		{
			ProcessAffixes(level, creature, affix->mNextAffix, 0);
			g_Logs.server->info("Applied next: [%v]", affix->mNextAffix.c_str());
		}

		affixCount++;
		randomAffixes.erase(randomAffixes.begin() + affixSel);
	}

	std::string nameStr = typeSel->mDisplayName;
	nameStr.append(" ");
	nameStr.append(creature->css.display_name);
	creature->css.SetDisplayName(nameStr.c_str());
	creature->css.aggro_players = 0;

	nameStr.clear();
	for(size_t i = 0; i < appliedAffixNames.size(); i++)
	{
		if(i > 0)
			nameStr.append(", ");
		nameStr.append(appliedAffixNames[i]);
	}
	if(nameStr.size() > 0)
		creature->css.SetSubName(nameStr.c_str());

	creature->css.experience_gain_rate = bonusExp;
	creature->css.magic_loot_drop_rate = bonusDrop;
	creature->LimitValueOverflows();
}

/*
This function processes a named affix entry.  It can operate on either a single entry, list, or a
chained affix or group.  In the case of a group, it will call itself recursively until all group
references have been resolved.
*/
void EliteManager :: ProcessAffixes(int level, CreatureInstance *creature, const std::string &affixName, int nestlevel)
{
	//The nest level is a generic debug test to make sure that the recursion isn't getting too deep.
	//Could be caused by incorrect table data, or memory corruption.
	//Generic error test.  There is no reason for any chain to get this deep.
	if(nestlevel > RECURSION_NEST_LEVEL_MAX)
	{
		g_Logs.server->error("EliteManager::ProcessAffixes potential loop [%v] nestlevel [%v]", affixName.c_str(), nestlevel);
		return;
	}

	if(affixName.size() == 0)
		return;

	std::vector<AffixEntry*> affixes;

	QueryAffix(affixName, level, affixes);
	if(affixes.size() == 0)
		return;

	for(size_t i = 0; i < affixes.size(); i++)
	{
		ApplyAffixStat(affixes[i], creature);
		if(affixes[i]->mNextAffix.size() > 0)
		{
			if(affixes[i]->mNextAffix.compare(affixName) == 0)
			{
				g_Logs.server->error("EliteManager::ProcessAffixes next affix references self [%v->%v]", affixName.c_str(), affixes[i]->mNextAffix.c_str());
			}
			else
			{
				ProcessAffixes(level, creature, affixes[i]->mNextAffix, nestlevel + 1);
			}
		}
	}
}

void EliteManager :: ApplyAffixStat(const AffixEntry *affix, CreatureInstance *creature)
{
	if(affix == NULL || creature == NULL)
		return;

	int statIndex = GetStatIndexByName(affix->mStatName);
	if(statIndex >= 0)
	{
		int statID = StatList[statIndex].ID;
		float oldValue = GetStatValueByID(statID, &creature->css);
		float newValue = oldValue;
		switch(affix->mOperation)
		{
		case '+': newValue += affix->mAdjustValue; break;
		case '-': newValue -= affix->mAdjustValue; break;
		case '*': newValue *= affix->mAdjustValue; break;
		case '/': newValue /= affix->mAdjustValue; break;
		}
		if(newValue != oldValue)
		{
			WriteValueToStat(statID, newValue, &creature->css);
		}
	}
}

void EliteManager :: AppendInfoNameTo(const std::string &infoName, STRINGLIST &infoList)
{
	for(size_t i = 0; i < infoList.size(); i++)
	{
		if(infoList[i].compare(infoName) == 0)
			return;
	}
	infoList.push_back(infoName);
}
