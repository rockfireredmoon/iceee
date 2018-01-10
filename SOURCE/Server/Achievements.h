#pragma once
#ifndef ACHIEVEMENTS_H
#define ACHIEVEMENTS_H

#include <map>
#include <list>
#include <string>
#include <stdarg.h>
#include "CommonTypes.h"


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
	int GetIDByName(const std::string &eventName);
}

class AchievementObjectiveDef
{
public:
	AchievementObjectiveDef();
	~AchievementObjectiveDef();

	std::string mName;
	std::string mTitle;
	std::string mDescription;
	std::string mIcon1;
	std::string mIcon2;
	std::string mTag;
};

class AchievementDef
{
public:
	AchievementDef();
	~AchievementDef();

	std::vector<AchievementObjectiveDef*> mObjectives;
	int mCategory;
	std::string mName;
	std::string mTitle;
	std::string mDescription;
	std::string mIcon1;
	std::string mIcon2;
	std::string mTag;

	AchievementObjectiveDef* GetObjectiveDef(std::string name);
};


class Achievement
{
public:
	Achievement();
	Achievement(const Achievement &other);
	Achievement(AchievementDef *def);
	~Achievement();

	AchievementDef *mDef;
	std::vector<AchievementObjectiveDef*> mCompletedObjectives;
	int mID;

	void CompleteObjective(std::string name);
	bool IsComplete();
};

class AchievementsManager
{
public:
	AchievementsManager();
	~AchievementsManager();
	std::map<std::string, AchievementDef*> mDefs;
	AchievementDef* LoadDef(std::string name);
	int LoadItems(void);
	int GetTotalObjectives();
	int GetTotalAchievements();
	AchievementDef* GetItem(std::string name);
private:
	std::string GetPath(std::string name);
	int mTotalObjectives;

};

} //namespace Achievements

extern Achievements::AchievementsManager g_AchievementsManager;

#endif //#ifndef ACHIEVEMENTS_H
