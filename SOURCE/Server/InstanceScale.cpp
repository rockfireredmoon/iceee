#include "InstanceScale.h"
#include "FileReader3.h"
#include "DirectoryAccess.h"
#include "StringList.h"
#include "DropTable.h"
#include "Stats.h"  //For CreatureRarityType.
#include "CommonTypes.h"

const char *DropRateProfileManager::STANDARD = "standard";
const char *DropRateProfileManager::INSTANCE = "instance";
const DropRateProfile DropRateProfileManager::mNullProfile;  //Need this to avoid linker error for static member

InstanceScaleManager g_InstanceScaleManager;
DropRateProfileManager g_DropRateProfileManager;



QualityArray::QualityArray()
{
	Clear();
}

void QualityArray::Clear(void)
{
	memset(QData, 0, sizeof(QData));
}

void QualityArray::CopyFrom(const QualityArray *source)
{
	if(source == this)
		return;
	memcpy(this, source, sizeof(QData));
}

void QualityArray::SetRawData(int q0, int q1, int q2, int q3, int q4, int q5, int q6)
{
	QData[0] = q0;
	QData[1] = q1;
	QData[2] = q2;
	QData[3] = q3;
	QData[4] = q4;
	QData[5] = q5;
	QData[6] = q6;
}

DropRateProfile::DropRateProfile(const DropRateProfile &source)
{
	CopyFrom(source);
}

DropRateProfile::DropRateProfile()
{
	SetAmountChance(0, 1);
	SetAmountChance(1, 1);
}

//We only want to copy the data here, not the name.
void DropRateProfile::CopyFrom(const DropRateProfile &source)
{
	if(this == &source)
		return;
	mName = source.mName;
	Chance.CopyFrom(&source.Chance);
	Flags.CopyFrom(&source.Flags);
	Level.CopyFrom(&source.Level);
	Amount.CopyFrom(&source.Amount);
}

int DropRateProfile::GetQualityChance(int qualityLevel)
{
	if(qualityLevel < 0 || qualityLevel >= QualityArray::ARRAY_SIZE)
		return 0;
	return Chance.QData[qualityLevel];
}

int DropRateProfile::GetAmountChance(int amount) const
{
	if(amount < 0 || amount >= QualityArray::ARRAY_SIZE)
		return 0;
	return Amount.QData[amount];
}

bool DropRateProfile::IsCreatureAllowed(int qualityLevel, int mobRarityLevel, bool isNamedMob) const
{
	if(qualityLevel < 0 || qualityLevel >= QualityArray::ARRAY_SIZE)
		return false;

	int checkFlag = 0;
	switch(mobRarityLevel)
	{
	case CreatureRarityType::NORMAL:    checkFlag = ((isNamedMob == false) ? FLAG_RARITY_NORMAL    : FLAG_RARITY_NORMAL_NAMED  ); break;
	case CreatureRarityType::HEROIC:    checkFlag = ((isNamedMob == false) ? FLAG_RARITY_HEROIC    : FLAG_RARITY_HEROIC_NAMED  ); break;
	case CreatureRarityType::EPIC:      checkFlag = ((isNamedMob == false) ? FLAG_RARITY_EPIC      : FLAG_RARITY_EPIC_NAMED    ); break;
	case CreatureRarityType::LEGEND:    checkFlag = ((isNamedMob == false) ? FLAG_RARITY_LEGEND    : FLAG_RARITY_LEGEND_NAMED  ); break;
	}
	return ((Flags.QData[qualityLevel] & checkFlag) != 0);
}


void DropRateProfile::SetQualityChance(int qualityLevel, int denominator)
{
	if(qualityLevel < 0 || qualityLevel >= QualityArray::ARRAY_SIZE)
		return;

	if(denominator > 0)
		Chance.QData[qualityLevel] = LootSystem::ComputeSharesByFraction(denominator);
	else
		Chance.QData[qualityLevel] = 0;
	
	//g_Log.AddMessageFormat("[%s] quality[%d]=%d (%d)", mName.c_str(), qualityLevel, denominator, Chance.QData[qualityLevel]);
}


void DropRateProfile::SetAmountChance(int amount, int denominator)
{
	if(amount < 0 || amount >= QualityArray::ARRAY_SIZE)
		return;

	if(denominator > 0)
		Amount.QData[amount] = LootSystem::ComputeSharesByFraction(denominator);
	else
		Amount.QData[amount] = 0;

	//g_Log.AddMessageFormat("[%s] quality[%d]=%d (%d)", mName.c_str(), qualityLevel, denominator, Chance.QData[qualityLevel]);
}

void DropRateProfile::SetQualityFlags(int qualityLevel, const char *flags)
{
	if(qualityLevel < 0 || qualityLevel >= QualityArray::ARRAY_SIZE)
		return;

	// Uppercase values mean that all kinds of mobs of that rarity are accepted (quality|quality_named)
	// Lowercase values mean that only named mobs of that rarity are accepted (quality_named)
	static const char FlagChar[11] = {
		'N', 'n',
		'H', 'h',
		'E', 'e',
		'L', 'l',
		'A', 'a',
		'*'
	};

	static const int FlagValue[11] = {
		FLAG_RARITY_NORMAL_ALL        , FLAG_RARITY_NORMAL_NAMED,
		FLAG_RARITY_HEROIC_ALL        , FLAG_RARITY_HEROIC_NAMED,
		FLAG_RARITY_EPIC_ALL          , FLAG_RARITY_EPIC_NAMED,
		FLAG_RARITY_LEGEND_ALL        , FLAG_RARITY_LEGEND_NAMED,
		FLAG_RARITY_ALL_ALL           , FLAG_RARITY_ALL_NAMED,
		FLAG_ALL,
	};
	int result = 0;
	
	size_t len = strlen(flags);
	for(size_t pos = 0; pos < len; pos++)
	{
		for(size_t fi = 0; fi < 11; fi++)
		{
			if(flags[pos] == FlagChar[fi])
			{
				result |= FlagValue[fi];
				break;
			}
		}
	}
	Flags.QData[qualityLevel] = result;

	//g_Log.AddMessageFormat("[%s] flags[%d]=%s (%d)", mName.c_str(), qualityLevel, flags, Flags.QData[qualityLevel]);
}

void DropRateProfile::SetMinimumLevel(int qualityLevel, int level)
{
	if(qualityLevel < 0 || qualityLevel >= QualityArray::ARRAY_SIZE)
		return;
	Level.QData[qualityLevel] = level;

	//g_Log.AddMessageFormat("[%s] level[%d]=%d (%d)", mName.c_str(), qualityLevel, level, Level.QData[qualityLevel]);
}

bool DropRateProfile::IsDefault() const
{
	return mName.length() == 0;
}

bool DropRateProfile::HasData(void)
{
	for(size_t i = 0; i < QualityArray::ARRAY_SIZE; i++)
	{
		if(Chance.QData[i] != 0)
			return true;
	}
	return false;
}

void DropRateProfile::DebugRunFlagTest(void)
{
	//Iterate through the combinations and produce a debug message for each result.
	for(int iq = 0; iq <= 6; iq++)
	{
		for(int mr = 0; mr <= 3; mr++)
		{
			for(int mn = 0; mn <= 1; mn++)
			{
				bool result = IsCreatureAllowed(iq, mr, (mn != 0));
				g_Log.AddMessageFormat("Testing[%s] ItemQual[%d] mobRarity[%d] mobNamed[%d] : %s", mName.c_str(), iq, mr, mn, ((result == true) ? "TRUE" : "false") );
			}
		}
	}
}

DropRateProfileManager::DropRateProfileManager()
{
}

void DropRateProfileManager::LoadData(void)
{
	mProfiles.clear();  //In case we're reloading.

	//Initialize the default tables
	QualityArray initLev;
	QualityArray standardRate;
	QualityArray instanceRate;

	initLev.SetRawData(-1, -1, 1, 15, 25, 45, -1);

	//This raw data holds the non-transformed drop rates which must be applied through the SetQualityChance
	//function to properly set them as drop rates.
	standardRate.SetRawData(0, 0, 25, 125, 625, 0, 0);
	instanceRate.SetRawData(0, 0, 25, 100, 500, 4000, 0);
	DropRateProfile &standard = mProfiles[STANDARD];
	DropRateProfile &instance = mProfiles[INSTANCE];
	for(int q = 0; q <= QualityArray::MAX_QUALITY; q++)
	{
		standard.SetQualityChance(q, standardRate.QData[q]);
		standard.SetQualityFlags(q, "*");
		standard.SetMinimumLevel(q, initLev.QData[q]);

		instance.SetQualityChance(q, instanceRate.QData[q]);
		instance.SetQualityFlags(q, "*");
		instance.SetMinimumLevel(q, initLev.QData[q]);
	}

	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "DropRateProfile.txt");
	LoadTable(filename.c_str());

	g_Log.AddMessageFormat("Loaded %d drop rate profiles", mProfiles.size());
}

void DropRateProfileManager::LoadTable(const char *filename)
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
			std::string name = fr.BlockToStringC(0);
			DropRateProfile &entry = mProfiles[name];
			entry.mName = name;

			//7 quality levels  0,1,2,3,4,5,6
			//7 columns         1,2,3,4,5,6,7
			for(int qi = 0; qi <= 6; qi++)
				entry.SetQualityChance(qi, fr.BlockToIntC(1 + qi));

			//7 columns         8,9,10,11,12,13,14
			for(int qi = 0; qi <= 6; qi++)
				entry.SetMinimumLevel(qi, fr.BlockToIntC(8 + qi));

			//7 columns         15,16,17,18,19,20,21
			for(int qi = 0; qi <= 6; qi++)
				entry.SetQualityFlags(qi, fr.BlockToStringC(15 + qi));

			//7 columns         22,23,24,25,26,27,28
			for(int qi = 0; qi <= 6; qi++)
				entry.SetAmountChance(qi, fr.BlockToIntC(22 + qi));
		}
	}
	fr.CloseFile();
}

const DropRateProfile& DropRateProfileManager::GetProfileByName(const std::string &name)
{
	std::map<std::string, DropRateProfile>::iterator it;
	it = mProfiles.find(name);
	if(it != mProfiles.end())
		return it->second;

	g_Log.AddMessageFormat("[ERROR] DropRateProfile [%s] does not exist.", name.c_str());
	return mNullProfile;
}

InstanceScaleProfile::InstanceScaleProfile()
{
	Clear();
}

void InstanceScaleProfile::Clear(void)
{
	mDifficultyName.clear();
	mDropRateProfileName.clear();
	mLevelOffset = 0;
	mCoreMultPerLev = 0.0F;
	mDmgMultPerLev = 0.0F;
	mArmorMultPerLev = 0.0F;
	mStatMultBonus = 0.0F;
	mConMultBonus = 0.0F;
	mDmgMultBonus = 0.0F;
	mArmorMultBonus = 0.0F;
	mDropMult = 0.0F;
	mDescription.clear();
}

InstanceScaleManager::InstanceScaleManager()
{
}

void InstanceScaleManager::LoadData(void)
{
	mProfiles.clear();  //In case we're reloading.

	std::string filename;
	Platform::GenerateFilePath(filename, "Data", "ScaledDifficulties.txt");

	LoadTable(filename.c_str());
	g_Log.AddMessageFormat("Loaded %d instance scaling profiles", mProfiles.size());
}

void InstanceScaleManager::LoadTable(const char *filename)
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
			std::string name = fr.BlockToStringC(0);

			//InstanceScaleProfile &row = mProfiles[name];
			InstanceScaleProfile row;

			row.mDifficultyName = name;
			row.mDropRateProfileName = fr.BlockToStringC(1);
			row.mLevelOffset = fr.BlockToIntC(2);
			row.mCoreMultPerLev = fr.BlockToFloatC(3);
			row.mDmgMultPerLev = fr.BlockToFloatC(4);
			row.mArmorMultPerLev = fr.BlockToFloatC(5);
			row.mStatMultBonus = fr.BlockToFloatC(6);
			row.mConMultBonus = fr.BlockToFloatC(7);
			row.mDmgMultBonus = fr.BlockToFloatC(8);
			row.mArmorMultBonus = fr.BlockToFloatC(9);
			row.mDropMult = fr.BlockToFloatC(10);
			row.mDescription = fr.BlockToStringC(11);

			mProfiles.push_back(row);
		}
	}
	fr.CloseFile();
}

const InstanceScaleProfile* InstanceScaleManager::GetProfile(const std::string &name)
{
	for(size_t i = 0; i < mProfiles.size(); i++)
	{
		if(mProfiles[i].mDifficultyName.compare(name) == 0)
			return &mProfiles[i];
	}
	return NULL;

	/* OLD, WAS FOR MAP CONTAINER
	std::map<std::string, InstanceScaleProfile>::iterator it;
	it = mProfiles.find(name);
	if(it != mProfiles.end())
		return &it->second;
	
	return NULL;
	*/
}

const InstanceScaleProfile* InstanceScaleManager::GetDefaultProfile(void)
{
	return &mNullScaleProfile;
}

void InstanceScaleManager :: EnumProfileList(MULTISTRING &output)
{
	for(size_t i = 0; i < mProfiles.size(); i++)
	{
		STRINGLIST row;
		row.push_back(mProfiles[i].mDifficultyName);
		row.push_back(mProfiles[i].mDescription);
		output.push_back(row);
	}
	if(output.size() == 0)
	{
		STRINGLIST row;
		row.push_back("<none>");
		row.push_back("The server does not have any profiles.");
		output.push_back(row);
	}
	/* OLD, WAS FOR MAP
	std::map<std::string, InstanceScaleProfile>::iterator it;
	for(it = mProfiles.begin(); it != mProfiles.end(); ++it)
	{
		STRINGLIST row;
		row.push_back(it->second.mDifficultyName);
		row.push_back(it->second.mDescription);
		output.push_back(row);
	}
	if(output.size() == 0)
	{
		STRINGLIST row;
		row.push_back("<none>");
		row.push_back("The server does not have any profiles.");
		output.push_back(row);
	}
	*/
}

