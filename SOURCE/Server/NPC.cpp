#include <algorithm>
#include "NPC.h"
 //For debugging
#include "FileReader.h"
#include "util/Log.h"
#include "Components.h"

PetDefManager g_PetDefManager;

/*****************************************************************
 *  HateProfile
 *****************************************************************/

HateProfile :: HateProfile()
{
	tauntReleaseTime = 0;
	nextRefreshTime = 0;
}

HateProfile :: ~HateProfile()
{
	hateList.clear();
}

int HateProfile :: GetIndexByCreature(int CDefID)
{
	for(size_t i = 0; i < hateList.size(); i++)
		if(hateList[i].CDefID == CDefID)
			return i;
	return -1;
}

void HateProfile :: Add(int CID, int CDefID, short level, int damage, int hate)
{
	int r = GetIndexByCreature(CDefID);
	if(r == -1)
	{
		HateCreatureData newItem;
		newItem.CID = CID;
		newItem.CDefID = CDefID;
		newItem.level = level;
		if(hate < 0)
			hate = 0;
		newItem.hate = hate;
		newItem.damage = damage;
		hateList.push_back(newItem);
	}
	else if(r >= 0)
	{
		hateList[r].damage += damage;
		hateList[r].hate += hate;
		if(hateList[r].hate < 0)
			hateList[r].hate = 0;
	}
}

void HateProfile :: SetImmediateRefresh(void)
{
	nextRefreshTime = 0;
}

void HateProfile :: ExtendTauntRelease(int seconds)
{
	tauntReleaseTime = g_ServerTime + (seconds * 1000);
}

int HateProfile :: GetHighestLevel(void)
{
	int highestLev = 0;
	for(size_t i = 0; i < hateList.size(); i++)
		if(hateList[i].level > highestLev)
			highestLev = hateList[i].level;
	return highestLev;
}

void HateProfile :: UnHate(int CreatureDefID)
{
	for(size_t i = 0; i < hateList.size(); i++)
	{
		if(hateList[i].CDefID == CreatureDefID)
		{
			hateList.erase(hateList.begin() + i);
			//hateList[i].hate = 0;
			tauntReleaseTime = 0;
			return;
		}
	}
}

bool HateProfile :: CheckRefreshAndUpdate(void)
{
	if(g_ServerTime < nextRefreshTime)
		return false;

	nextRefreshTime = g_ServerTime + REFRESH_DELAY;

	std::sort(hateList.begin(), hateList.end(), HateCreatureData::SortByHate);

	return true;
}

/*****************************************************************
 *  HateProfileContainer
 *****************************************************************/

HateProfileContainer :: HateProfileContainer()
{
}

HateProfileContainer :: ~HateProfileContainer()
{
	profileList.clear();
}

HateProfile * HateProfileContainer :: GetProfile(void)
{
	HateProfile newItem;
	profileList.push_back(newItem);
	return &profileList.back();
}

void HateProfileContainer :: RemoveProfile(HateProfile* profile)
{
	std::list<HateProfile>::iterator it;
	for(it = profileList.begin(); it != profileList.end(); ++it)
	{
		if(&*it == profile)
		{
			profileList.erase(it);
			return;
		}
	}
	g_Logs.server->debug("Hate profile pointer not found: %v", profile);
}

void HateProfileContainer :: UnHate(int CreatureDefID)
{
	std::list<HateProfile>::iterator it;
	for(it = profileList.begin(); it != profileList.end(); ++it)
	{
		it->UnHate(CreatureDefID);
	}
}





/*****************************************************************
 *  PetDef
 *****************************************************************/


PetDef :: PetDef()
{
	mCreatureDefID = 0;
	mLevel = 0;
	mCost = 0;
}

void PetDef	:: Clear(void)
{
	mCreatureDefID = 0;
	mDisplayName.clear();
	mLevel = 0;
	mCost = 0;
	mDesc.clear();
	mItemDefID = 0;
}

void PetDef :: CopyFrom(const PetDef& source)
{
	mCreatureDefID = source.mCreatureDefID;
	mDisplayName = source.mDisplayName;
	mLevel = source.mLevel;
	mCost = source.mCost;
	mDesc = source.mDesc;
	mItemDefID = source.mItemDefID;
}



/*****************************************************************
 *  PetDefManager
 *****************************************************************/

PetDefManager :: PetDefManager()
{
}

PetDefManager :: ~PetDefManager()
{
}

void PetDefManager :: AddEntry(PetDef& data)
{
	mDefs[data.mCreatureDefID].CopyFrom(data);
}

PetDef* PetDefManager :: GetEntry(int CreatureDefID)
{
	PETDEF_MAP::iterator it = mDefs.find(CreatureDefID);
	if(it == mDefs.end())
		return NULL;
	return &it->second;
}

void PetDefManager :: LoadFile(const char *filename)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Logs.data->error("Error opening file: %v", filename);
		return;
	}
	lfr.CommentStyle = Comment_Semi;
	PetDef newEntry;
	while(lfr.FileOpen() == true)
	{
		lfr.ReadLine();
		int r = lfr.MultiBreak(",");
		if(r >= 6)
		{
			newEntry.mCreatureDefID = lfr.BlockToIntC(0);
			newEntry.mDisplayName = lfr.BlockToStringC(1, 0);
			newEntry.mLevel = lfr.BlockToIntC(2);
			newEntry.mCost = lfr.BlockToIntC(3);
			newEntry.mDesc = lfr.BlockToStringC(4, 0);
			newEntry.mItemDefID = lfr.BlockToIntC(5);
			AddEntry(newEntry);
			newEntry.Clear();
		}
	}
	lfr.CloseCurrent();
}

int PetDefManager :: GetStandardCount(void)
{
	return (int)mDefs.size();
}

void PetDefManager :: FillQueryResponse(MULTISTRING& output)
{
	STRINGLIST row;
	char convert[32];

	PETDEF_MAP::iterator it;
	for(it = mDefs.begin(); it != mDefs.end(); ++it)
	{
		sprintf(convert, "%d", it->second.mCreatureDefID);
		row.push_back(convert);

		row.push_back(it->second.mDisplayName);

		sprintf(convert, "%d", it->second.mLevel);
		row.push_back(convert);

		sprintf(convert, "%d", it->second.mCost);
		row.push_back(convert);

		row.push_back(it->second.mDesc);

		output.push_back(row);
		row.clear();
	}
}
