#ifndef ARENA_H
#define ARENA_H

#include <string>
#include <vector>

struct ArenaRule
{
	enum RuleTypes
	{
		RULE_NONE = 0,
		RULE_MOD_CLASS = 1,    //mOperator is a string of class abbreviations {'K', 'R', 'M', 'D'}
		RULE_MOD_PLAYERS = 2   //mOperator is a list of character names, separated by a commas.
	};
	enum StatApplyTypes
	{
		APPLY_NONE     = 0,
		APPLY_ADDITIVE = 1,    //The stat will be added to the player's total.
		APPLY_MULTIPLY = 2     //The stat amount to add will be computed based on a percentage of the current amount. (ex: 0.05 for 5%)
	};

	int mRuleType;
	std::string mOperator;
	
	std::string mStatName;
	float mStatChange;
	int mStatApplyType;

	ArenaRule();
	ArenaRule(const std::string &configStr);
	void Clear(void);
	int ResolveRuleType(const std::string &str);
	int ResolveStatApplyType(const std::string &str);
	void ApplyConfig(const std::string &configStr);
	bool IsMatchProfession(int statProfession) const;
	bool IsMatchDisplayName(const char *statDisplayName) const;
	bool GetStatApplyLimits(int inStatID, float &outMin, float &outMax) const;
};

struct ArenaStatLimit
{
	short statID;
	float min;
	float max;
};

class ArenaRuleset
{
public:
	bool mEnabled;        //A simple flag to disable processing of arena events for non-arenas.
	int mPVPStatus;      //Players will be given the PVPABLE status when they log in if set to one of the PVP GameMode constants.
	bool mTurboRunSpeed;

	std::vector<ArenaRule> mRuleList;

	ArenaRuleset();
	~ArenaRuleset();
	void DebugInit(void);
};


#endif //#ifndef ARENA_H
