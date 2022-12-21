/*
 * Manages the 'Daily Reward' configuration, allowing a list of different rewards to be
 * given to players upon logging in for consecutive days. The configuration file supports
 * multiple reward types, including credits, items and virtual items.
 *
 * The rewards may be split up by tier (allowing different tiered potions to be given
 * out for example). If tier 0 is used, that reward will be available to all tiers.
 *
 * When the player reaches the maximum number of days configured in this file, the
 * rewards will reset back to day 1.
 */

#include "Daily.h"
#include "Config.h"
#include "FileReader3.h"
#include "DirectoryAccess.h"

#include "Util.h"
#include <stdlib.h>
#include "util/Log.h"

const DailyProfile DailyProfileManager::mNullProfile;  //Need this to avoid linker error for static member

DailyProfileManager g_DailyProfileManager;


namespace RewardType {

const char *GetNameByID(int id) {
	switch (id) {
	case ITEM:
		return "ITEM";
	case VIRTUAL:
		return "VIRTUAL";
	case CREDITS:
		return "CREDITS";
	}
	return "<undefined>";
}

int GetIDByName(const string &name) {
	if (name.compare("ITEM") == 0)
		return ITEM;
	if (name.compare("VIRTUAL") == 0)
		return VIRTUAL;
	if (name.compare("CREDITS") == 0)
		return CREDITS;
	return UNDEFINED;
}

}

//

ItemReward::ItemReward() {
}

void ItemReward::CopyFrom(const ItemReward &source) {
	itemIDs.assign(source.itemIDs.begin(), source.itemIDs.end());
}

void ItemReward::FromData(string data) {
	vector<string> a;
	Util::Split(data.c_str(), ",", a);
	for(auto s : a) {
		itemIDs.push_back(stoi(s));
	}
}

//

CreditReward::CreditReward() {
	credits = 0;
}

void CreditReward::CopyFrom(const CreditReward &source) {
	credits = source.credits;
}

void CreditReward::FromData(string data) {
	credits = atoi(data.c_str());
}

//

VirtualItemRewardComponent::VirtualItemRewardComponent() {
	equipType = 0;
	weaponTypes.clear();
}

void VirtualItemRewardComponent::CopyFrom(const VirtualItemRewardComponent &source) {
	equipType = source.equipType;
	weaponTypes.assign(source.weaponTypes.begin(), source.weaponTypes.end());
}

//

VirtualItemReward::VirtualItemReward() {
	dropRateProfileName = "";
	minItemRarity = 0;
	components.clear();
}

void VirtualItemReward::FromData(string data) {
	vector<string> a;
	Util::Split(data.c_str(), ":", a);
	if(a.size() < 3) {
		g_Logs.data->warn("Daily configuration contains incomplete VIRTUAL item");
	}
	else {
		minItemRarity = atoi(a[0].c_str());
		dropRateProfileName = a[1];
		if(a[2].compare("NONE") != 0 && a[2].compare("ANY") != 0) {
			vector<string> l;
			Util::Split(a[2], ",", l);
			for(auto it = l.begin(); it != l.end(); ++it) {
				vector<string> a;
				Util::Split(it->c_str(), "/", a);
				VirtualItemRewardComponent ei;
				ei.equipType = stoi(a[0]);
				if(a.size() > 1) {
					vector<string> z;
					Util::Split(a[1], "|", z);
					for(auto s : z) {
						ei.weaponTypes.push_back(stoi(s));
					}
				}
			}
		}
	}
}

void VirtualItemReward::CopyFrom(const VirtualItemReward &source) {
	dropRateProfileName = source.dropRateProfileName;
	minItemRarity = source.minItemRarity;
	vector<VirtualItemRewardComponent> sourceComponents = source.components;
	components.clear();
	for(auto v : sourceComponents) {
		VirtualItemRewardComponent ei;
		ei.CopyFrom(v);
		components.push_back(ei);
	}
}

DailyProfile::DailyProfile()
{
	dayNumber = 0;
	spawnCreatureDefID = 0;
	rewardType = RewardType::UNDEFINED;
	minLevel = 0;
	maxLevel = 999;
}

void DailyProfile::CopyFrom(const DailyProfile &source)
{
	if(this == &source)
		return;

	rewardType = source.rewardType;
	dayNumber = source.dayNumber;
	minLevel = source.minLevel;
	maxLevel = source.maxLevel;
	spawnCreatureDefID = source.spawnCreatureDefID;

	switch(source.rewardType) {
	case RewardType::CREDITS:
		creditReward.CopyFrom(source.creditReward);
		break;
	}
}

DailyProfileManager::DailyProfileManager()
{
}

int DailyProfileManager::GetNumberOfProfiles() {
	return mProfiles.size();
}

void DailyProfileManager::LoadData(void)
{
	mProfiles.clear();  //In case we're reloading.

	LoadTable(g_Config.ResolveStaticDataPath() / "Data" /  "Daily.txt");

	g_Logs.data->info("Loaded %v daily profiles", mProfiles.size());
}

void DailyProfileManager::LoadTable(const fs::path &filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename) != FileReader3::SUCCESS)
	{
		g_Logs.data->error("Could not open file [%v]", filename);
		return;
	}
	fr.SetCommentChar(';');
	fr.ReadLine(); //First row is header.
	while(fr.Readable() == true)
	{
		fr.ReadLine();
		int r = fr.MultiBreak("\t");
		if(r > 1)
		{
			int dayNumber = fr.BlockToIntC(0);
			int minLevel = fr.BlockToIntC(1);
			int maxLevel = fr.BlockToIntC(2);
			int spawnCreatureDefID = fr.BlockToIntC(3);
			string type = fr.BlockToStringC(4);
			string data = fr.BlockToStringC(5);

			DailyProfile entry;
			entry.dayNumber = dayNumber;
			entry.minLevel = minLevel;
			entry.maxLevel = maxLevel;
			entry.spawnCreatureDefID = spawnCreatureDefID;
			entry.rewardType = RewardType::GetIDByName(type);

			switch(entry.rewardType) {
			case RewardType::CREDITS:
				entry.creditReward.FromData(data);
				break;
			case RewardType::ITEM:
				entry.itemReward.FromData(data);
				break;
			case RewardType::VIRTUAL:
				entry.virtualItemReward.FromData(data);
				break;
			}

			mProfiles.push_back(entry);
		}
	}
	fr.CloseFile();
}

int DailyProfileManager::GetMaxDayNumber()
{
	int dn = 0;
	for(auto p : mProfiles) {
		if(p.dayNumber > dn)
			dn = p.dayNumber;
	}
	return dn;
}

vector<DailyProfile> DailyProfileManager::GetProfiles(int dayNumber, int level)
{
	vector<DailyProfile> l;
	for(auto p :  mProfiles) {
		if(p.dayNumber == dayNumber && level >= p.minLevel && level <= p.maxLevel) {
			l.push_back(p);
		}
	}
	return l;
}


