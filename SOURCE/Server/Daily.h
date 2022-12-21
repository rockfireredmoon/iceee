//Daily rewards configuration

#ifndef DAILY_H
#define DAILY_H

#include <string>
#include <map>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

namespace RewardType
{
	enum
	{
		UNDEFINED = -1,
		ITEM = 0,
		VIRTUAL = 1,
		CREDITS = 2,
		MAX
	};
	const char *GetNameByID(int eventID);
	int GetIDByName(const string &eventName);
}

class ItemReward
{
public:
	vector<int> itemIDs;
	ItemReward();
	void FromData(string data);
	void CopyFrom(const ItemReward &source);
};

class VirtualItemRewardComponent
{
public:
	int equipType;
	vector<int> weaponTypes;
	VirtualItemRewardComponent();
	void CopyFrom(const VirtualItemRewardComponent &source);
};

class VirtualItemReward
{
public:
	unsigned long minItemRarity;
	vector<VirtualItemRewardComponent> components;
	string dropRateProfileName;
	VirtualItemReward();
	void FromData(string data);
	void CopyFrom(const VirtualItemReward &source);
};

class CreditReward
{
public:
	unsigned long credits;
	CreditReward();
	void FromData(string data);
	void CopyFrom(const CreditReward &source);
};

class DailyProfile
{
public:

	int dayNumber;
	int minLevel;
	int maxLevel;
	int rewardType;
	int spawnCreatureDefID;

	ItemReward itemReward;
	CreditReward creditReward;
	VirtualItemReward virtualItemReward;

	DailyProfile();
	void CopyFrom(const DailyProfile &source);
};

class DailyProfileManager
{
public:
	DailyProfileManager();
	void LoadData(void);

	int GetMaxDayNumber();
	vector<DailyProfile> GetProfiles(int dayNumber, int tier);
	int GetNumberOfProfiles();

private:
	vector<DailyProfile> mProfiles;
	void LoadTable(const fs::path &filename);

	static const DailyProfile mNullProfile;
};

extern DailyProfileManager g_DailyProfileManager;

#endif  //DAILY_H
