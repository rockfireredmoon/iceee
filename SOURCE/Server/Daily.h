//Daily rewards configuration

#ifndef DAILY_H
#define DAILY_H

#include <string>
#include <map>
#include <vector>

class DailyProfile
{
public:

	int dayNumber;
	unsigned long credits;
	unsigned long minItemRarity;
	int itemID;
	std::vector<int> equipTypes;
	std::string dropRateProfileName;
	bool hasData;

	DailyProfile();
	void CopyFrom(const DailyProfile &source);
};

class DailyProfileManager
{
public:
	DailyProfileManager();
	void LoadData(void);

	static const char *STANDARD;
	static const char *INSTANCE;

	const DailyProfile& GetProfileByDayNumber(int dayNumber);
	int GetNumberOfProfiles();

private:
	std::map<int, DailyProfile> mProfiles;
	void LoadTable(const char *filename);

	static const DailyProfile mNullProfile;
};

extern DailyProfileManager g_DailyProfileManager;

#endif  //DAILY_H
