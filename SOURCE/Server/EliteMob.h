#include <string>
#include <vector>
#include "Util.h"
#include "util/Log.h"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

class CreatureInstance;   //Forward declaration for a pointer argument.

struct EliteType
{
	string mDisplayName;    //Display name for the player to see.
	int mChance;                 //Chance to spawn.  This is the denominator.  (100 = 1 in 100 chance)
	int mMinLevel;               //Minimum creature level to select this type.
	int mMaxLevel;               //Maximum creature level to select this type.
	int mMinAffix;               //Minimum number of randomized affixes to select and apply.
	int mMaxAffix;               //Maximum number of randomized affixes to select and apply.
	string mAffixGroup;     //All 'AffixEntry' matching this name will be intrinsically applied to this creature.
	int mBonusExp;               //Percentage bonus to apply (100 = additional 100%)
	int mBonusDrop;              //Percentage bonus to apply to drop rate (100 = additional 100%)
	EliteType();
	void Clear(void);
};

struct AffixEntry
{
	string mInternalName;   //Internal name of this entry, used for identification and lookups.
	string mDisplayName;    //Display name for the player to see.
	int mMinLevel;               //Minimum level necessary to select this affix for consideration.
	int mMaxLevel;               //Maximum level necessary to select this affix for consideration.
	string mStatName;       //Name of the CSS stat to adjust.
	char mOperation;             //How to mathematically adjust the stat value.  { '+', '-', '*', '/' }
	float mAdjustValue;          //Amount to adjust the stat ( on the operator).
	string mNextAffix;      //Next affix to explicitly select, or a group.  Groups are defined by prefixing '#'.
	int mBonusExp;               //Percentage bonus to apply (100 = additional 100%)
	int mBonusDrop;              //Percentage bonus to apply to drop rate (100 = additional 100%)

	AffixEntry();
	void Clear(void);
};

class EliteManager
{
public:
	EliteManager();
	~EliteManager();
	void LoadData();
	void ApplyTransformation(CreatureInstance *creature);

private:
	static const int RECURSION_NEST_LEVEL_MAX = 5;

	vector<EliteType> mEliteType;
	vector<AffixEntry> mAffixEntry;

	void LoadTypeTable(const fs::path &filename);
	void LoadAffixTable(const fs::path &filename);
	
	void QueryType(int level, vector<EliteType*> &outputResults);
	void QueryAffix(const string &name, int level, vector<AffixEntry*> &outputResults);
	void QueryAffixesByLevel(int level, vector<AffixEntry*> &outputResults);
	void ProcessAffixes(int level, CreatureInstance *creature, const string &affixName, int nestlevel);
	void ApplyAffixStat(const AffixEntry *affix, CreatureInstance *creature);

	void AppendInfoNameTo(const string &infoName, STRINGLIST &infoList);
};

extern EliteManager g_EliteManager;
