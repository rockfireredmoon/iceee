//Daily rewards configuration

#ifndef DAILY_H
#define DAILY_H

#include <string>
#include <map>
#include <vector>

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
	int GetIDByName(const std::string &eventName);
}

class ItemReward
{
public:
	std::vector<int> itemIDs;
	ItemReward();
	void FromData(std::string data);
	void CopyFrom(const ItemReward &source);
};

class VirtualItemRewardComponent
{
public:
	int equipType;
	std::vector<int> weaponTypes;
	VirtualItemRewardComponent();
	void CopyFrom(const VirtualItemRewardComponent &source);
};

class VirtualItemReward
{
public:
	unsigned long minItemRarity;
	std::vector<VirtualItemRewardComponent> components;
	std::string dropRateProfileName;
	VirtualItemReward();
	void FromData(std::string data);
	void CopyFrom(const VirtualItemReward &source);
};

class CreditReward
{
public:
	unsigned long credits;
	CreditReward();
	void FromData(std::string data);
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
	std::vector<DailyProfile> GetProfiles(int dayNumber, int tier);
	int GetNumberOfProfiles();

private:
	std::vector<DailyProfile> mProfiles;
	void LoadTable(const char *filename);

	static const DailyProfile mNullProfile;
};

extern DailyProfileManager g_DailyProfileManager;

#endif  //DAILY_H
