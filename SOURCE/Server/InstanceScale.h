//Zone Definitions, Map Definitions and World Instances

#ifndef INSTANCESCALE_H
#define INSTANCESCALE_H

#include <string>
#include <map>
#include "CommonTypes.h"

struct QualityArray
{
	static const int MAX_QUALITY = 6;
	static const int ARRAY_SIZE = MAX_QUALITY + 1;
	int QData[ARRAY_SIZE];

	QualityArray();
	void Clear(void);
	void CopyFrom(const QualityArray *source);
	void SetRawData(int q0, int q1, int q2, int q3, int q4, int q5, int q6);
};

class DropRateProfile
{
public:

	enum
	{
		QUALITY_NONE      = 0,
		QUALITY_NORMAL    = 1,
		QUALITY_UNCOMMON  = 2,
		QUALITY_RARE      = 3,
		QUALITY_EPIC      = 4,
		QUALITY_LEGENDARY = 5,
		QUALITY_ARTIFACT  = 6
	};
	enum : signed short
	{
		FLAG_RARITY_NONE                     = 0,
		FLAG_RARITY_NORMAL            = (1 << 0),    
		FLAG_RARITY_HEROIC            = (1 << 1),
		FLAG_RARITY_EPIC              = (1 << 2),
		FLAG_RARITY_LEGEND            = (1 << 3),

		FLAG_RARITY_NORMAL_NAMED      = (1 << 4),    
		FLAG_RARITY_HEROIC_NAMED      = (1 << 5),
		FLAG_RARITY_EPIC_NAMED        = (1 << 6),
		FLAG_RARITY_LEGEND_NAMED      = (1 << 7),

		FLAG_RARITY_NORMAL_ALL        = FLAG_RARITY_NORMAL | FLAG_RARITY_NORMAL_NAMED,
		FLAG_RARITY_HEROIC_ALL        = FLAG_RARITY_HEROIC | FLAG_RARITY_HEROIC_NAMED,
		FLAG_RARITY_EPIC_ALL          = FLAG_RARITY_EPIC | FLAG_RARITY_EPIC_NAMED,
		FLAG_RARITY_LEGEND_ALL        = FLAG_RARITY_LEGEND | FLAG_RARITY_LEGEND_NAMED,

		FLAG_RARITY_ALL_NAMED         = FLAG_RARITY_NORMAL_NAMED | FLAG_RARITY_HEROIC_NAMED | FLAG_RARITY_EPIC_NAMED | FLAG_RARITY_LEGEND_NAMED,
		FLAG_RARITY_ALL_ALL           = FLAG_RARITY_NORMAL_ALL | FLAG_RARITY_HEROIC_ALL | FLAG_RARITY_EPIC_ALL | FLAG_RARITY_LEGEND_ALL,

		FLAG_ALL                      = 0x0FFF,
	};

	//Reserve extra space so that index [MAX_QUALITY] is valid.
	//int Chance[ARRAY_SIZE]; //The chance, in shares {(1 / denominator)} to drop the associated quality.
	//int Flags[ARRAY_SIZE];  //The flags of creature rarity and named status that must be met to drop the associated quality.
	//int Level[ARRAY_SIZE];  //The minimum level a mob must be to drop the associated quality.

	std::string mName;    //Not really needed in this class, but helps with debugging to know what name a pointer is set to.

	QualityArray Chance;
	QualityArray Flags;
	QualityArray Level;
	QualityArray Amount; // Abused for drop amounts. Q0 is the global amount, if anything used if amounts are zero.
						 // If Q0 is -1, then Q1-Q6 amounts are multipled by the largest party size of all attackers.
						 // Q1-Q6 map to 1 items to 6 items. The value is the denominator of the chance you'll get that amount.


	DropRateProfile();
	DropRateProfile(const DropRateProfile &source);
	void CopyFrom(const DropRateProfile &source);
	int GetQualityChance(int qualityLevel);
	void SetQualityChance(int qualityLevel, int denominator);
	int GetAmountChance(int amount) const;
	void SetAmountChance(int qualityLevel, int denominator);
	void SetQualityFlags(int qualityLevel, const char *flags);
	void SetMinimumLevel(int qualityLevel, int level);
	bool HasData(void);
	bool IsDefault() const;
	void DebugRunFlagTest(void);

	//External classes not related to the loading phase of profiles should only be concerned about
	//accessing this function.
	bool IsCreatureAllowed(int qualityLevel, int mobRarityLevel, bool isNamedMob) const;
};

class DropRateProfileManager
{
public:
	DropRateProfileManager();
	void LoadData(void);

	static const char *STANDARD;
	static const char *INSTANCE;

	const DropRateProfile& GetProfileByName(const std::string &name);

private:
	std::map<std::string, DropRateProfile> mProfiles;
	void LoadTable(std::string filename);

	static const DropRateProfile mNullProfile;
};


/*
struct DropRateTable
{
	static const int MAX_QUALITY = 6;

	//Arrays need +1 to be inclusive to allow [MAX_QUALITY] to return a value.
	int mQualityOverworld[MAX_QUALITY+1];  //Drop chances (shares) for overworld zones.
	int mQualityInstance[MAX_QUALITY+1];   //Drop chances (shares) for instance zones.
	int GetDropRate(int rarity, bool instance);
};

int DropRateTable::GetDropRate(int rarity, bool instance)
{
	if(rarity < 0 || rarity > MAX_RARITY)
		return 0;
	if(instance == true)
		return mQualityInstance[rarity];
	
	return mQualityOverworld[rarity];
}*/


class InstanceScaleProfile
{
public:
	std::string mDifficultyName;    //Internal lookup name, and sent to the client for info.
	std::string mDropRateProfileName; //Name of the DropRateProfile to use.  If present it will override the instance's default instance profile, which is acquired from its ZoneDef information.
	int mLevelOffset;               //-1 to match player level.  0 or higher as a flat level bonus.
	float mCoreMultPerLev;          //Core stats (str/dex/con/psy/spi) are multiplied by this amount per level difference.
	float mDmgMultPerLev;           //Base weapon damage is multiplied by this amount per level difference.
	float mArmorMultPerLev;         //Armor is multiplied by this amount per level difference.
	float mStatMultBonus;           //Bonus multiplier for damage stats (str/dex/psy/spi) after level difference is applied.
	float mConMultBonus;            //Constitution multiplier after level difference is applied.
	float mDmgMultBonus;            //Damage multiplier after level difference is applied.
	float mArmorMultBonus;          //Armor multiplier after level difference is applied.
	float mDropMult;                //Multiplier to drop rate for all spawns in the instance.
	std::string mDescription;       //Description to give to the client.
	InstanceScaleProfile();
	void Clear();
};

class InstanceScaleManager
{
public:
	InstanceScaleManager();
	void LoadData(void);
	const InstanceScaleProfile* GetProfile(const std::string &name);
	const InstanceScaleProfile* GetDefaultProfile(void);
	void EnumProfileList(MULTISTRING &output);

private:
	//std::map<std::string, InstanceScaleProfile> mProfiles;
	std::vector<InstanceScaleProfile> mProfiles;
	void LoadTable(std::string filename);

	InstanceScaleProfile mNullScaleProfile;
};


extern InstanceScaleManager g_InstanceScaleManager;
extern DropRateProfileManager g_DropRateProfileManager;

#endif  //INSTANCESCALE_H
