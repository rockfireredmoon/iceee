#include "Arena.h"
#include "Util.h"
#include "Stats.h"

ArenaRule :: ArenaRule()
{
	Clear();
}

ArenaRule :: ArenaRule(const std::string &configStr)
{
	ApplyConfig(configStr);
}

void ArenaRule :: Clear(void)
{
	mRuleType = 0;
	mOperator.clear();

	mStatName.clear();
	mStatChange = 0.0F;
	mStatApplyType = 0;
}

int ArenaRule :: ResolveRuleType(const std::string &str)
{
	const char *nameList[] = { "RULE_NONE", "RULE_MOD_CLASS", "RULE_MOD_PLAYERS" };
	const int resultList[] = {  RULE_NONE,   RULE_MOD_CLASS,   RULE_MOD_PLAYERS  };
	const int nameCount = sizeof(nameList) / sizeof(nameList[0]);
	
	for(int i = 0; i < nameCount; i++)
	{
		if(str.compare(nameList[i]) == 0)
			return resultList[i];
	}
	return RULE_NONE;
}

int ArenaRule :: ResolveStatApplyType(const std::string &str)
{
	const char *nameList[] = { "APPLY_NONE", "APPLY_ADDITIVE", "APPLY_MULTIPLY" };
	const int resultList[] = {  APPLY_NONE,   APPLY_ADDITIVE,   APPLY_MULTIPLY  };
	const int nameCount = sizeof(nameList) / sizeof(nameList[0]);
	
	for(int i = 0; i < nameCount; i++)
	{
		if(str.compare(nameList[i]) == 0)
			return resultList[i];
	}
	return APPLY_NONE;
}

void ArenaRule :: ApplyConfig(const std::string &configStr)
{
	Clear();
	STRINGLIST output;
	Util::Split(configStr, "|", output);
	if(output.size() < 5)
		return;

	mRuleType = ResolveRuleType(output[0]);
	mOperator = output[1];
	mStatName = output[2];
	mStatChange = Util::StringToFloat(output[3]);
	mStatApplyType = ResolveStatApplyType(output[4]);
}

bool ArenaRule :: IsMatchProfession(int statProfession) const
{
	const char *search = NULL;
	switch(statProfession)
	{
	case Professions::KNIGHT: search = "K"; break;
	case Professions::ROGUE: search = "R"; break;
	case Professions::MAGE: search = "M"; break;
	case Professions::DRUID: search = "D"; break;
	}
	if(search == NULL)
		return false;
	if(mOperator.find(search) != std::string::npos)
		return true;
	return false;
}

bool ArenaRule :: IsMatchDisplayName(const char *statDisplayName) const
{
	std::string name = statDisplayName;
	Util::ToLowerCase(name);

	STRINGLIST output;
	Util::Split(mOperator, ",", output);
	for(size_t i = 0; i < output.size(); i++)
	{
		Util::ToLowerCase(output[i]);
		if(output[i].compare(name) == 0)
			return true;
	}
	return false;
}

bool ArenaRule :: GetStatApplyLimits(int inStatID, float &outMin, float &outMax) const
{
	return false;
}




ArenaRuleset :: ArenaRuleset()
{
	mEnabled = false;
	mPVPStatus = false;
	mTurboRunSpeed = false;
}

ArenaRuleset :: ~ArenaRuleset()
{
}

void ArenaRuleset :: DebugInit(void)
{
	mEnabled = true;
	mPVPStatus = true;
	mTurboRunSpeed = true;

	//mRuleList.push_back(ArenaRule("RULE_MOD_CLASS|KR|psyche|500|APPLY_ADDITIVE"));
	//mRuleList.push_back(ArenaRule("RULE_MOD_PLAYERS|Eld Khran|spirit|4|APPLY_MULTIPLY"));
}