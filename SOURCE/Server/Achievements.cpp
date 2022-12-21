#include "Achievements.h"
#include "Util.h"
#include "Config.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "util/Log.h"


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

int GetIDByName(const string &name) {
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

AchievementObjectiveDef* AchievementDef::GetObjectiveDef(string name) {
	for(vector<AchievementObjectiveDef*>::iterator it = mObjectives.begin(); it != mObjectives.end(); ++it) {
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

AchievementDef * AchievementsManager::LoadDef(string name) {
	auto buf = GetPath(name);
	if (!fs::exists(buf)) {
		g_Logs.data->warn("No file for CS item [%v]", buf);
		return NULL;
	}

	AchievementDef *item = new AchievementDef();
	AchievementObjectiveDef *obj = NULL;

	FileReader lfr;
	if (lfr.OpenText(buf) != Err_OK) {
		g_Logs.data->warn("Could not open file [%s]", buf);
		return NULL;
	}

//		unsigned long mStartDate;
//		unsigned long mEndDate;

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
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

fs::path AchievementsManager::GetPath(string name) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "Achievements/%s.txt", name.c_str());
	Platform::FixPaths(buf);
	return buf;
}

int AchievementsManager::GetTotalAchievements() {
	return mDefs.size();
}

int AchievementsManager::GetTotalObjectives() {
	return mTotalObjectives;
}

int AchievementsManager::LoadItems(void) {
	mTotalObjectives = 0;
	for (map<string, AchievementDef*>::iterator it = mDefs.begin();
			it != mDefs.end(); ++it)
		delete it->second;
	mDefs.clear();

	auto path = g_Config.ResolveStaticDataPath() / fs::path("Achievements");
	for(const fs::directory_entry& entry : fs::directory_iterator(path)) {
		auto path = entry.path();
		if (path.extension() == ".txt") {
			LoadDef(path.stem());
		}
	}

	return 0;
}

AchievementDef * AchievementsManager::GetItem(string name) {
	map<string, AchievementDef*>::iterator it = mDefs.find(name);
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
	size_t c = 0;
	for(vector<AchievementObjectiveDef*>::iterator it = mDef->mObjectives.begin(); it != mDef->mObjectives.end(); ++it) {
		if(find(mCompletedObjectives.begin(), mCompletedObjectives.end(), (*it)) != mCompletedObjectives.end()) {
			c++;
		}
	}
	return c == mDef->mObjectives.size();
}

void Achievement::CompleteObjective(string name) {
	AchievementObjectiveDef *def = mDef->GetObjectiveDef(name);
	if(def == NULL)
		g_Logs.data->warn("Request to complete achievement object that does not exist. %v", name.c_str());
	else if(find(mCompletedObjectives.begin(), mCompletedObjectives.end(), def) == mCompletedObjectives.end())
		mCompletedObjectives.push_back(def);
	else
		g_Logs.data->warn("Request to complete achievement object is already complete. %v", name.c_str());
}

}
