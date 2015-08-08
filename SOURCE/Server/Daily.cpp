#include "Daily.h"
#include "FileReader3.h"
#include "DirectoryAccess.h"
#include "StringList.h"
#include "Util.h"
#include <stdlib.h>

const DailyProfile DailyProfileManager::mNullProfile;  //Need this to avoid linker error for static member

DailyProfileManager g_DailyProfileManager;

DailyProfile::DailyProfile()
{
	credits = 0;
	minItemRarity = 0;
	dayNumber = 0;
	dropRateProfileName = "";
	itemID = 0;
	hasData = false;
	equipTypes.clear();
}

void DailyProfile::CopyFrom(const DailyProfile &source)
{
	if(this == &source)
		return;
	credits = source.credits;
	minItemRarity = source.minItemRarity;
	dayNumber = source.dayNumber;
	dropRateProfileName = source.dropRateProfileName;
	itemID = source.itemID;
	hasData = source.hasData;
	equipTypes.assign(source.equipTypes.begin(), source.equipTypes.end());
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

	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "Daily.txt");
	LoadTable(filename.c_str());

	g_Log.AddMessageFormat("Loaded %d daily profiles", mProfiles.size());
}

void DailyProfileManager::LoadTable(const char *filename)
{
	FileReader3 fr;
	if(fr.OpenFile(filename) != FileReader3::SUCCESS)
	{
		g_Log.AddMessageFormat("[ERROR] Could not open file [%s]", filename);
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
			DailyProfile &entry = mProfiles[dayNumber];
			entry.itemID = fr.BlockToIntC(1);
			entry.credits = fr.BlockToIntC(2);
			entry.minItemRarity = fr.BlockToIntC(3);
			entry.dropRateProfileName = fr.BlockToStringC(4);

			std::string eq = fr.BlockToStringC(5);
			if(eq.compare("NONE") != 0) {
				std::vector<std::string> l;
				Util::Split(eq, ",", l);
				for(std::vector<std::string>::iterator it = l.begin(); it != l.end(); ++it) {
					entry.equipTypes.push_back(atoi(it->c_str()));
				}
			}

			entry.hasData = true;
		}
	}
	fr.CloseFile();
}

const DailyProfile& DailyProfileManager::GetProfileByDayNumber(int dayNumber)
{
	std::map<int, DailyProfile>::iterator it;
	it = mProfiles.find(dayNumber);
	if(it != mProfiles.end())
		return it->second;

	g_Log.AddMessageFormat("[ERROR] DailyProfile [%d] does not exist.", dayNumber);
	return  DailyProfileManager::mNullProfile;
}


