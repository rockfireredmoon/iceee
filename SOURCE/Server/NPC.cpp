#include <algorithm>
#include "NPC.h"
 //For debugging
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "Util.h"
#include "util/Log.h"
#include "Components.h"
#include "Config.h"
#include <boost/format.hpp>

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

	sort(hateList.begin(), hateList.end(), HateCreatureData::SortByHate);

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
	list<HateProfile>::iterator it;
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
	list<HateProfile>::iterator it;
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
	mItemDefID = 0;
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

void PetDefManager :: LoadFile(const fs::path &filename)
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


//
NPCDialogParagraph::NPCDialogParagraph() {
	mType = 0;
	mValue = "";
}
NPCDialogParagraph::~NPCDialogParagraph() {}

NPCDialogParagraph::NPCDialogParagraph(const NPCDialogParagraph &other) {
	mType = other.mType;
	mValue  = other.mValue;
}


//
NPCDialogItem::NPCDialogItem() {
	mMinInterval = 0;
	mMaxInterval = 0;
	mSequence = 0;
}

NPCDialogItem::~NPCDialogItem() {}

NPCDialogManager::NPCDialogManager() {
}

NPCDialogManager g_NPCDialogManager;

NPCDialogManager::~NPCDialogManager() {
}

bool NPCDialogManager::SaveItem(NPCDialogItem * item) {
	string path = GetPath(item->mName);
	g_Logs.data->info("Saving dialog item to %v.", path.c_str());
	FILE *output = fopen(path.c_str(), "wb");
	if (output == NULL) {
		g_Logs.data->error("[ERROR] Saving petition could not open: %v",
				path.c_str());
		return false;
	}

	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "Sequence=%d\r\n", item->mSequence);
	fprintf(output, "MinInterval=%d\r\n", item->mMinInterval);
	fprintf(output, "MaxInterval=%d\r\n", item->mMaxInterval);

	vector<NPCDialogParagraph>::iterator it;
	for (it = item->mParagraphs.begin(); it != item->mParagraphs.end(); ++it) {
		NPCDialogParagraph p = *it;
		fprintf(output, "\r\n[PARAGRAPH]\r\n");
		fprintf(output, "Type=%d\r\n", p.mType);
		fprintf(output, "Value=%s\r\n", p.mValue.c_str());
	}

	fflush(output);
	fclose(output);

	return true;
}

NPCDialogItem * NPCDialogManager::LoadItem(const fs::path &name) {
//	string buf = GetPath(name);
//	if (!Platform::FileExists(buf.c_str())) {
//		g_Logs.data->error("No file for dialog item [%v]", buf.c_str());
//		return NULL;
//	}

	NPCDialogItem *item = new NPCDialogItem();
	item->mName = name;

	FileReader lfr;
	if (lfr.OpenText(name) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", name);
		return NULL;
	}

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
			}
			else if (strcmp(lfr.SecBuffer, "[PARAGRAPH]") == 0) {
				NPCDialogParagraph para;
				item->mParagraphs.push_back(para);
			}
			else if (strcmp(lfr.SecBuffer, "VALUE") == 0)
				item->mParagraphs.back().mValue = lfr.BlockToStringC(1, 0);
			else if (strcmp(lfr.SecBuffer, "TYPE") == 0)
				item->mParagraphs.back().mType = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "SEQUENCE") == 0)
				item->mSequence = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "MININTERVAL") == 0)
				item->mMinInterval = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "MAXINTERVAL") == 0)
				item->mMaxInterval = lfr.BlockToIntC(1);
			else
				g_Logs.data->warn("Unknown identifier [%v] in file [%v]",
						lfr.SecBuffer, name);
		}
	}
	lfr.CloseCurrent();

	mItems[name] = item;

	return item;
}


bool NPCDialogManager::RemoveItem(const string &name) {
	auto path = GetPath(name);
	if (!fs::exists(path)) {
		g_Logs.data->error("No file for NPC dialog item [%v] to remove", path);
		return false;
	}
	map<string, NPCDialogItem*>::iterator it = mItems.find(name);
	if(it != mItems.end()) {
		delete it->second;
		mItems.erase(it);
	}
	auto filename = g_Config.ResolveStaticDataPath() / "Dialog" / str(boost::format("%s.del") % name);
	if(!fs::exists(filename) || fs::remove(filename)) {
		fs::rename(path, filename);
	}
	return true;
}

fs::path NPCDialogManager::GetPath(const string &name) {
	return g_Config.ResolveStaticDataPath()/ "Dialog" / str(boost::format("%s.txt") % name);
}

int NPCDialogManager::LoadItems(void) {
	for(auto it = mItems.begin(); it != mItems.end(); ++it)
		delete it->second;
	mItems.clear();

	auto dirpath = g_Config.ResolveStaticDataPath()/ "Dialog";

	for(const fs::directory_entry& entry : fs::directory_iterator(dirpath)) {
		auto path = entry.path();
		if (path.extension() == ".txt") {
			LoadItem(path);
		}
	}

	return 0;
}

NPCDialogItem * NPCDialogManager::GetItem(const string &name) {
	map<string, NPCDialogItem*>::iterator it = mItems.find(name);
	return it == mItems.end() ? NULL : it->second;
}

