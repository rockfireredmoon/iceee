#pragma once
#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <map>
#include <list>
#include <string>
#include <stdarg.h>
#include "CommonTypes.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;


namespace Achievements
{

namespace Category
{
	enum
	{
		UNDEFINED = -1,
		COMBAT = 0,
		QUESTING = 1,
		SOCIAL = 2,
		BOOKS = 3,
		CRAFTING = 4,
		PETS = 5,
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const string &eventName);
}

class AchievementObjectiveDef
{
public:
	AchievementObjectiveDef();
	~AchievementObjectiveDef();

	string mName;
	string mTitle;
	string mDescription;
	string mIcon1;
	string mIcon2;
	string mTag;
};

class AchievementDef
{
public:
	AchievementDef();
	~AchievementDef();

	vector<AchievementObjectiveDef*> mObjectives;
	int mCategory;
	string mName;
	string mTitle;
	string mDescription;
	string mIcon1;
	string mIcon2;
	string mTag;

	AchievementObjectiveDef* GetObjectiveDef(string name);
};


class Achievement
{
public:
	Achievement();
	Achievement(const Achievement &other);
	Achievement(AchievementDef *def);
	~Achievement();

	AchievementDef *mDef;
	vector<AchievementObjectiveDef*> mCompletedObjectives;
	int mID;

	void CompleteObjective(string name);
	bool IsComplete();
};

class AchievementsManager
{
public:
	AchievementsManager();
	~AchievementsManager();
	map<string, AchievementDef*> mDefs;
	AchievementDef* LoadDef(string name);
	int LoadItems(void);
	int GetTotalObjectives();
	int GetTotalAchievements();
	AchievementDef* GetItem(string name);
private:
	fs::path GetPath(string name);
	int mTotalObjectives;

};

} //namespace Achievements

extern Achievements::AchievementsManager g_AchievementsManager;

#endif //#ifndef ACHIEVEMENTS_H
