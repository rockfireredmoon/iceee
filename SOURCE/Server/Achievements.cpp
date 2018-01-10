#include "Achievements.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "util/Log.h"
#include <string.h>
#include <algorithm>


Achievements::AchievementsManager g_AchievementsManager;

namespace Achievements {
namespace Category {

//UNDEFINED = -1,
//		COMBAT = 0,
//		QUESTING = 1,
//		SOCIAL = 2,
//		BOOKS = 3,
//		CRAFTING = 4,
//		PETS = 5,
//		MAX

const char *GetNameByID(int id) {
	switch (id) {
	case COMBAT:
		return "COMBAT";
	case QUESTING:
		return "QUESTING";
	case SOCIAL:
		return "SOCIAL";
	case BOOKS:
		return "BOOKS";
	case CRAFTING:
		return "CRAFTING";
	case PETS:
		return "PETS";
	}
	return "<undefined>";
}

int GetIDByName(const std::string &name) {
	if (name.compare("COMBAT") == 0)
		return COMBAT;
	if (name.compare("QUESTING") == 0)
		return QUESTING;
	if (name.compare("SOCIAL") == 0)
		return SOCIAL;
	if (name.compare("BOOKS") == 0)
		return BOOKS;
	if (name.compare("CRAFTING") == 0)
		return CRAFTING;
	if (name.compare("PETS") == 0)
		return PETS;
	return UNDEFINED;
}
}

//
AchievementDef::AchievementDef() {
	mCategory = Category::UNDEFINED;
	mName = "";
	mDescription = "";
	mIcon1 = "";
	mIcon2 = "";
	mTag = "";
}
AchievementDef::~AchievementDef() {
}

AchievementObjectiveDef* AchievementDef::GetObjectiveDef(std::string name) {
	for(std::vector<AchievementObjectiveDef*>::iterator it = mObjectives.begin(); it != mObjectives.end(); ++it) {
		if((*it)->mName.compare(name) == 0)
			return (*it);
	}
	return NULL;
}

//
AchievementObjectiveDef::AchievementObjectiveDef() {
	mName = "";
	mTitle = "";
	mDescription = "";
	mIcon1 = "";
	mIcon2 = "";
	mTag = "";
}
AchievementObjectiveDef::~AchievementObjectiveDef() {
}

//

AchievementsManager::AchievementsManager() {
	mTotalObjectives = 0;
}

AchievementsManager::~AchievementsManager() {
}

AchievementDef * AchievementsManager::LoadDef(std::string name) {
	std::string buf = GetPath(name);
	if (!Platform::FileExists(buf.c_str())) {
		g_Logs.data->warn("No file for CS item [%v]", buf.c_str());
		return NULL;
	}

	AchievementDef *item = new AchievementDef();
	AchievementObjectiveDef *obj = NULL;

	FileReader lfr;
	if (lfr.OpenText(buf.c_str()) != Err_OK) {
		g_Logs.data->warn("Could not open file [%s]", buf.c_str());
		return NULL;
	}

//		unsigned long mStartDate;
//		unsigned long mEndDate;

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	long amt = -1;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (item->mName.length() != 0) {
					g_Logs.data->warn(
							"%v contains multiple entries. CS items have one entry per file",
							buf.c_str());
					break;
				}
				item->mName = name;
			}
			else if (strcmp(lfr.SecBuffer, "[OBJECTIVE]") == 0) {
				obj = new AchievementObjectiveDef();
				item->mObjectives.push_back(obj);
			}
			else if (strcmp(lfr.SecBuffer, "DESCRIPTION") == 0) {
				if(obj == NULL)
					item->mDescription = lfr.BlockToStringC(1, 0);
				else
					obj->mDescription = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "TITLE") == 0) {
				if(obj == NULL)
					item->mTitle = lfr.BlockToStringC(1, 0);
				else
					obj->mTitle = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "NAME") == 0) {
				if(obj == NULL)
					g_Logs.data->warn("Name expected after each [OBJECTIVE]");
				else
					obj->mName = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "TAG") == 0) {
				if(obj == NULL)
					item->mTag = lfr.BlockToStringC(1, 0);
				else
					obj->mTag = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "ICON1") == 0) {
				if(obj == NULL)
					item->mIcon1 = lfr.BlockToStringC(1, 0);
				else
					obj->mIcon1 = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "ICON2") == 0) {
				if(obj == NULL)
					item->mIcon2 = lfr.BlockToStringC(1, 0);
				else
					obj->mIcon2 = lfr.BlockToStringC(1, 0);
			}
			else if (strcmp(lfr.SecBuffer, "CATEGORY") == 0)
				item->mCategory = Category::GetIDByName(
						lfr.BlockToStringC(1, 0));
			else
				g_Logs.data->warn("Unknown identifier [%v] in file [%v]",
						lfr.SecBuffer, buf.c_str());
		}
	}
	lfr.CloseCurrent();

	mDefs[name] = item;
	mTotalObjectives += item->mObjectives.size();

	return item;
}

std::string AchievementsManager::GetPath(std::string name) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Achievements/%s.txt", name.c_str());
	Platform::FixPaths(buf);
	return buf;
}

int AchievementsManager::GetTotalAchievements(void) {
	return mDefs.size();
}

int AchievementsManager::GetTotalObjectives(void) {
	return mTotalObjectives;
}

int AchievementsManager::LoadItems(void) {
	mTotalObjectives = 0;
	for (std::map<std::string, AchievementDef*>::iterator it = mDefs.begin();
			it != mDefs.end(); ++it)
		delete it->second;
	mDefs.clear();

	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory("Achievements");
	r.ReadFiles();
	r.SetDirectory(dir.c_str());

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			LoadDef(Platform::Basename(p.c_str()));
		}
	}

	return 0;
}

AchievementDef * AchievementsManager::GetItem(std::string name) {
	std::map<std::string, AchievementDef*>::iterator it = mDefs.find(name);
	return it == mDefs.end() ? NULL : it->second;
}

//
Achievement::Achievement() {
	mID = 0;
	mDef = NULL;
	mCompletedObjectives.clear();
}

Achievement::Achievement(const Achievement &other) {
	mID = other.mID;
	mDef = other.mDef;
	mCompletedObjectives = other.mCompletedObjectives;
}

Achievement::Achievement(AchievementDef *def) {
	mID = 0;
	mDef = def;
	mCompletedObjectives.clear();
}

Achievement::~Achievement() {
	mCompletedObjectives.clear();
}

bool Achievement::IsComplete() {
	int c = 0;
	for(std::vector<AchievementObjectiveDef*>::iterator it = mDef->mObjectives.begin(); it != mDef->mObjectives.end(); ++it) {
		if(std::find(mCompletedObjectives.begin(), mCompletedObjectives.end(), (*it)) != mCompletedObjectives.end()) {
			c++;
		}
	}
	return c == mDef->mObjectives.size();
}

void Achievement::CompleteObjective(std::string name) {
	AchievementObjectiveDef *def = mDef->GetObjectiveDef(name);
	if(def == NULL)
		g_Logs.data->warn("Request to complete achievement object that does not exist. %v", name.c_str());
	else if(std::find(mCompletedObjectives.begin(), mCompletedObjectives.end(), def) == mCompletedObjectives.end())
		mCompletedObjectives.push_back(def);
	else
		g_Logs.data->warn("Request to complete achievement object is already complete. %v", name.c_str());
}

}
