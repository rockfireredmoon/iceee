#include <algorithm>
#include <string.h>
#include "NPC.h"
#include "StringList.h" //For debugging
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "Util.h"

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

void HateProfile :: Add(int CDefID, short level, int damage, int hate)
{
	int r = GetIndexByCreature(CDefID);
	if(r == -1)
	{
		HateCreatureData newItem;
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
	g_Log.AddMessageFormat("[DEBUG] Hate profile pointer not found: %p", profile);
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
		g_Log.AddMessageFormat("Error opening file: %s", filename);
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
	mInterval = 0;
	mTrigger = 0;
}

NPCDialogItem::~NPCDialogItem() {}

NPCDialogManager::NPCDialogManager() {
}

NPCDialogManager g_NPCDialogManager;

NPCDialogManager::~NPCDialogManager() {
}

bool NPCDialogManager::SaveItem(NPCDialogItem * item) {
	std::string path = GetPath(item->mName);
	g_Log.AddMessageFormat("Saving dialog item to %s.", path.c_str());
	FILE *output = fopen(path.c_str(), "wb");
	if (output == NULL) {
		g_Log.AddMessageFormat("[ERROR] Saving petition could not open: %s",
				path.c_str());
		return false;
	}

	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "Trigger=%d\r\n", item->mTrigger);
	fprintf(output, "Interval=%d\r\n", item->mInterval);

	std::vector<NPCDialogParagraph>::iterator it;
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

NPCDialogItem * NPCDialogManager::LoadItem(std::string name) {
	std::string buf = GetPath(name);
	if (!Platform::FileExists(buf.c_str())) {
		g_Log.AddMessageFormat("No file for dialog item [%s]", buf.c_str());
		return NULL;
	}

	NPCDialogItem *item = new NPCDialogItem();
	item->mName = name;

	FileReader lfr;
	if (lfr.OpenText(buf.c_str()) != Err_OK) {
		g_Log.AddMessageFormat("Could not open file [%s]", buf.c_str());
		return NULL;
	}

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	long amt = -1;
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
			else if (strcmp(lfr.SecBuffer, "TRIGGER") == 0)
				item->mTrigger = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "INTERVAL") == 0)
				item->mInterval = lfr.BlockToIntC(1);
			else
				g_Log.AddMessageFormat("Unknown identifier [%s] in file [%s]",
						lfr.SecBuffer, buf.c_str());
		}
	}
	lfr.CloseCurrent();

	mItems[name] = item;

	return item;
}


bool NPCDialogManager::RemoveItem(std::string name) {
	const char * path = GetPath(name).c_str();
	if (!Platform::FileExists(path)) {
		g_Log.AddMessageFormat("No file for NPC dialog item [%s] to remove", path);
		return false;
	}
	std::map<std::string, NPCDialogItem*>::iterator it = mItems.find(name);
	if(it != mItems.end()) {
		delete it->second;
		mItems.erase(it);
	}
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Dialog/%s.del", name.c_str());
	Platform::FixPaths(buf);
	if(!Platform::FileExists(buf) || remove(buf) == 0) {
		if(!rename(path, buf) == 0) {
			g_Log.AddMessageFormat("Failed to remove NPC dialog item %s", name.c_str());
			return false;
		}
	}
	return true;
}

std::string NPCDialogManager::GetPath(std::string name) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Dialog/%s.txt", name.c_str());
	Platform::FixPaths(buf);
	return buf;
}

int NPCDialogManager::LoadItems(void) {
	for(std::map<std::string, NPCDialogItem*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
		delete it->second;
	mItems.clear();

	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory("Dialog");
	r.ReadFiles();
	r.SetDirectory(dir.c_str());

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			LoadItem(Platform::Basename(p.c_str()).c_str());
		}
	}

	return 0;
}

NPCDialogItem * NPCDialogManager::GetItem(std::string name) {
	std::map<std::string, NPCDialogItem*>::iterator it = mItems.find(name);
	return it == mItems.end() ? NULL : it->second;
}

