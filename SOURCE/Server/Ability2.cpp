//#pragma warning(disable: 4996)
#include "Config.h"  //For protocol.
#include "Ability2.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "Creature.h"
#include "Formula.h"
#include "StringList.h"
#include "Util.h"
#include "Instance.h"
#include "Stats.h"
#include "Item.h"  //For item ID verification
#include "Combat.h"

extern double g_FacingTarget;
extern char GSendBuf[];

Ability2::AbilityManager2 g_AbilityManager;



/* Criteria Functions

CheckBuffLimits(1,-/DBMove/Debuff:Daze)     Check for an active buff/statis affect. (Comparator, Math/BuffCategery/BuffName)
Facing                                      Must be facing the target
InRange(10.0)                               Range check (distance)  [distance / 10 = meters]
has2HorPoleWeapon                           Requires a Two Hand or Pole Weapon
hasBow                                      Requires a Bow
hasMainHandWeapon                           Requires a Main Weapon
Behind                                      Must be within a 180 degree arc behind the target
hasMeleeWeapon                              Special case?  Only used for [Parry]
hasOffHandWeapon                            Special case?  Only used for [Dual Wield]
hasRangedWeapon                             Requies a ranged weapon.  Only used for [Impact Shot] 
HasResurrectMoney(0)                        Only used for 2nd res option [Resurrect]
HasResurrectMoney(1)                        Only used for 3rd res option [Rebirth]
hasShield                                   Shield must be equipped.  Only used for [Block]
HasStatus(IN_COMBAT)                        Requires a status effect in an enabled state
HasStatus(INVISIBLE)
HasStatus(WALK_IN_SHADOWS)
hasWand                                     Requires a wand.
Might(1)                                    Might cost (amount)
MightCharge(1,3)                            Active might charges required (min, max)
NearbySanctuary                             Must be near a sanctuary.  Only used for [Bind]
NotSilenced()                               Must not be silenced.
NotStatus(IN_COMBAT)                        Requires a status effect in a disabled state
PercentMaxHealth(0.251)                     Must have enough health (ratio)  Only used for [Blood Ritual]
Reagent(20522,2)                            Uses a reagent (itemID, amount)
StatValueLessThan(MOD_MOVEMENT, 20)         Stat amount within limit (statName, amount)
Will(1)                                     Will cost (amount)
WillCharge(1,2)                             Active will charges required (min, max)

*/




/* ACTION FUNCTIONS

 FireDamage(A_STRENGTH*0.60+5)
 Status(ROOT,7)
~10%{Status(StatusEffects::STUN,5)}                         For Implicit actions.  (~chance%{functions...})
A_AddMightCharge(1)                          Add a might charge to avatar (amount)
A_AddWillCharge(1)                           Add a will charge to avatar (amount)
A_Heal(((A_SPIRIT*1.792+131)*1)/5)           Heal avatar for (formula...)
Add(BASE_BLOCK,100,-1)                       Add status effect (statName, amount, duration)      [duration is in sections, -1 for no limit?]
AddHate(-100)                                Adjust hate (amount)
AddWDesc(BLEEDING,1,8,"Shadow Strike")       Display info? (stat,iteration,duration,name)
Amp(BASE_LUCK,0.05,3600)                     Amplify a stat by a percentage (stat, percent, duration)
AmpCore(-0.06,60)                            Reduce main stats (percentage, duration)
AttackMelee(0)                               Begin melee autoattack?  Only used for [melee]
AttackRanged(0)                              Begin ranged autoattack?  Only used for [ranged_melee]
BindTranslocate()                            Bind to a a sanctuary.  Only used for [Bind]
DeathDamage(((A_SPIRIT*1.12)*1)/5)           Apply death damage (formula)
DoNothing()                                  Seems to be a null action used for projectile effects, arrows, catapults, etc.
Duration(mightCharges*5)                     Assigns a duration for iteration effect skills (formula)
FireDamage(((A_PSYCHE*0.9+31)*1.2)/3.5)      Apply fire damage (formula)
FrostDamage(((A_PSYCHE*0.9+31)*1)/8)         Apply frost damage (formula)
FullMight()                                  Instantly restore might?  Only used for [Blood Ritual]
Go("LOC5")                                   Warp to a named location?  Only used for unused portal skills.
GoSanctuary()                                Warp to the last bound sanctuary?  Used for unstick, res skills, etc.
Harm(MAX_HEALTH*0.25)                        Apply arbitrary damage amount (formula).  Only used for [Blood Ritual]
Heal((A_DEXTERITY*0.448)*1)                  Apply arbitrary healing (formula)
HealthSacrifice((100 * willCharges) / 8)     Transfer HP from self to target (formula).  Only used for [Sacrifice]
Interrupt()                                  Interrupt spell casting.  Only used for [Kick]
Invisible(120,A_PSYCHE*1,0)                  Set invisibility (duration (sec), meter start(?), meter end(?))  Only used for [Invisibility]
Jump()                                       Perform a jump.  Only used for [Jump]
LeaveCombat()                                Removes in-combat flag?  Only used for [Fade]
MeleeDamage((A_DEXTERITY*0.168+12)*1.1)      Apply melee damage (formula)
MysticDamage(((A_SPIRIT*1.4336+104)*1)/8)    Apply mystic damage (formula)
Nullify(WILL_REGEN,(2*willCharges))          Set stat to zero?  (stat, duration(sec))
PortalRequest("Transport_GreatForest", "Great Forest")   Attempt portal to a location (internal name(?), visual name(?) )
Regenerate(12)                               Iterative heal (amount).  Used for health regen potions.
RemoveAltBuff()                              Remove a cast/attack/movement speed buff.  Only used for [Suppress]
RemoveAltDebuff()                            Removes debuffs (speeds, root, etc).  Only used for [Free]
RemoveEtcBuff()                              Removes other buffs.  Only used for [Abolish]
RemoveHealthBuff()                           Removes health buffs.  Only used for [Shield Warmer]
RemoveEtcDebuff()                            Removes other debuffs.  Only used for [Purge]
RemoveStatBuff()                             Removes stat buff.  Only used for [Corrupt]
RemoveStatDebuff()                           Removes stat debuff.  Only used for [Cleanse]
Resurrect(0.01, 1)                           Resurrect (health ratio, luck ratio)
Resurrect(0.01)
Resurrect(target_REZ_PENDING)                Unknown.  Only used for [Accept Rez]
Spin()                                       Spins the target 180 degrees around.
Status(CAN_USE_BLOCK,-1)                     Set a status effect (statname, duration)   Duration is seconds, -1 for unlimited.
SummonPet(53)                                Unknown effect.  (unknown parameter).  Only used for [Summon Pet]
T_AddMightCharge(-1*mightCharges)            Adjust might charges on target (amount).  Negative amount for loss.
T_AddWillCharge(-1*willCharges)              Adjust will charges on target (amount).  Negative amount for loss.
Taunt(3)                                     Taunt a target, holding its attention.  (duration_sec)
TopHated(10)                                 Unknown effect (unknown parameter).  Only used for [Frost Mire]
Translocate()                                Transports to bind location.  Only used for [Portal: Bind]
UnHate()                                     Remove all hate generated against you.
WalkInShadows(120,A_DEXTERITY*1.0)           Limited unseen movement (duration_sec, movementCounter?)
Nudify()							 	 	 Removes all clothes
Transform(CDefID)							 Transform into a creature
Untransform()								 Revert to natural appearance

*/

namespace EventType
{
	const char *GetNameByEventID(int eventID)
	{
		switch(eventID)
		{
		case onRequest: return "onRequest";
		case onActivate: return "onActivate";
		case onDeactivate: return "onDeactivate";
		case onIterate: return "onIterate";
		case onParry: return "onParry";
		}
		return "<undefined>";
	}
	int GetEventIDByName(const std::string &eventName)
	{
		if(eventName.compare("onRequest") == 0) return onRequest;
		if(eventName.compare("onActivate") == 0) return onActivate;
		if(eventName.compare("onDeactivate") == 0) return onDeactivate;
		if(eventName.compare("onIterate") == 0) return onIterate;
		if(eventName.compare("onParry") == 0) return onParry;
		return UNDEFINED;
	}
}


namespace Ability2
{

const int PLAYER_LIGHT_CHANCE_PER_LEVEL = 50;  //10 = 1% chance.
const double PLAYER_LIGHT_MULTIPLIER_MIN = 0.01;
const double PLAYER_LIGHT_MULTIPLIER_MAX = 0.05;

const int CREATURE_LIGHT_CHANCE_PER_LEVEL = 10;  //10 = 1% chance.
const double CREATURE_LIGHT_MULTIPLIER_MIN = 0.01;
const double CREATURE_LIGHT_MULTIPLIER_MAX = 0.02;

const double PLAYER_DAMAGE_REDUCTION_PER_LEVEL = 5.0;    //Players apply reduced damage for every level below the target.
const double CREATURE_DAMAGE_REDUCTION_PER_LEVEL = 1.0;  //Creatures apply reduced damage for every level below the target.

void AbilityImplicitAction :: Clear(void)
{
	memset(this, 0, sizeof(AbilityImplicitAction));
}

void VerifyExpression(const std::string &exp, AbilityVerify &verifyInfo)
{
	int nestLevel = 0;
	for(size_t i = 0; i < exp.size(); i++)
	{
		const char c = exp[i];
		switch(c)
		{
		case '(':
			nestLevel++;
			break;
		case ')':
			nestLevel--;
			if(nestLevel < 0)
				verifyInfo.AddError("Unexpected closing parenthesis in expression [%s]", exp.c_str());
			break;
		}
	}
	if(nestLevel != 0)
		verifyInfo.AddError("Mismatched parenthesis in expression [%s]", exp.c_str());
}

AbilityVerify :: AbilityVerify()
{
	Clear();
}

void AbilityVerify :: Clear(void)
{
	mError = 0;
	mErrorMsg.clear();
	memset(mWriteBuffer, 0, sizeof(mWriteBuffer));
}

void AbilityVerify :: AddError(const char *format, ...)
{
	mError++;
	va_list args;
	va_start (args, format);
	vsnprintf(mWriteBuffer, sizeof(mWriteBuffer) - 1, format, args);
	va_end (args);
	mErrorMsg.push_back(mWriteBuffer);
}

void AbilityVerify :: AddError(const char *format, va_list args)
{
	mError++;
	vsnprintf(mWriteBuffer, sizeof(mWriteBuffer) - 1, format, args);
	mErrorMsg.push_back(mWriteBuffer);
}

namespace TargetType
{
	const char *GetNameByTargetType(int targetType)
	{
		switch(targetType)
		{
		case TargetType::None: return "None";
		case TargetType::Self: return "Self";
		case TargetType::ST: return "ST";
		case TargetType::Implicit: return "Implicit";
		case TargetType::Party: return "Party";
		case TargetType::Cone180: return "Cone180";
		case TargetType::Cone90: return "Cone90";
		case TargetType::GTAE: return "GTAE";
		case TargetType::PBAE: return "PBAE";
		case TargetType::STAE: return "STAE";
		case TargetType::STXAE: return "STXAE";
		case TargetType::STP: return "STP";
		}
		return "<undefined>";
	}
	int GetTargetTypeByName(const std::string &targetTypeName)
	{
		//Self and ST are most common, just start with those.
		if(targetTypeName.compare("Self") == 0) return TargetType::Self;
		else if(targetTypeName.compare("ST") == 0) return TargetType::ST;
		else if(targetTypeName.compare("Implicit") == 0) return TargetType::Implicit;
		else if(targetTypeName.compare("Party") == 0) return TargetType::Party;
		else if(targetTypeName.compare("Cone180") == 0) return TargetType::Cone180;
		else if(targetTypeName.compare("Cone90") == 0) return TargetType::Cone90;
		else if(targetTypeName.compare("GTAE") == 0) return TargetType::GTAE;
		else if(targetTypeName.compare("PBAE") == 0) return TargetType::PBAE;
		else if(targetTypeName.compare("STAE") == 0) return TargetType::STAE;
		else if(targetTypeName.compare("STXAE") == 0) return TargetType::STXAE;
		else if(targetTypeName.compare("STP") == 0) return TargetType::STP;
		else if(targetTypeName.size() > 0)
			g_Log.AddMessageFormat("TargetType: Unknown name [%s]", targetTypeName.c_str());
		
		return None;
	}
};

namespace TargetFilter
{
	//Intended as a private use struct for defining a lookup table here.
	struct _KeyValue
	{
		const char *name;
		int flags;
	};
	int GetTargetFlagsByName(const std::string name)
	{
		static const _KeyValue pairs[] = {
			{ "Dead",         TargetFilter::Dead         },
			{ "Enemy",        TargetFilter::Enemy        },
			{ "Enemy Alive",  TargetFilter::Enemy_Alive  },
			{ "Friend",       TargetFilter::Friend       },
			{ "Friend Alive", TargetFilter::Friend_Alive },
			{ "Friend Dead",  TargetFilter::Friend_Dead  },
		};
		static const int pairNum = sizeof(pairs) / sizeof(pairs[0]);
		for(size_t i = 0; i < pairNum; i++)
		{
			if(name.compare(pairs[i].name) == 0)
				return pairs[i].flags;
		}
		//Default to self infliction only.
		return TargetFilter::Alive;
	}
}

//Tokenize a string, breaking at the given delimiter(s).
void Split(const std::string &source, const char *delim, STRINGLIST &dest)
{
	if(dest.size() > 0)
		dest.clear();
	size_t pos = 0;
	size_t fpos = 0;
	std::string extract;
	do
	{
		fpos = source.find(delim, pos);
		if(fpos != std::string::npos)
		{
			extract = source.substr(pos, fpos - pos);
			dest.push_back(extract);
			pos = fpos + 1;
		}
	} while(fpos != std::string::npos);
	extract = source.substr(pos, std::string::npos);
	dest.push_back(extract);
}

//Tokenize a string, breaking at any comma that is not enclosed within parenthesis.
//Input: a(1), b(2) ,c(3)
//Output:
//  [0]=[a(1)]
//  [1]=[ b(2) ]
//  [2]=[c(3)]
void SplitFunctionList(const std::string &input, STRINGLIST &output) 
{
	size_t len = input.length();
	int nestLevel = 0;
	size_t first = 0;
	for(size_t i = 0; i < len; i++)
	{
		switch(input[i])
		{
		case '(': nestLevel++; break;
		case ')': nestLevel--; break;
		case '{': nestLevel++; break;
		case '}': nestLevel--; break;
		case ',':
			if(nestLevel == 0)
			{
				output.push_back(input.substr(first, i - first));
				first = i + 1;
			}
			break;
		}
	}
	if(first < len)
		output.push_back(input.substr(first, len));
}


//Take a function, such as
// SomeFunction(identifier, (value * 20) + 1 , 1000)
// And split, so the function and each argument each have one tokens, whitespace included:
// [0] = [SomeFunction]
// [1] = [identifier]
// [2] = [ (value * 20) + 1 ]
// [3] = [1000]
bool SplitFunction(const std::string &input, STRINGLIST &output)
{
	size_t first = input.find("(");
	if(first == std::string::npos)
	{
		output.push_back(input);
		return true;
	}

	//Push function name
	output.push_back(input.substr(0, first));
	
	//Advance position inside the parenthesis and start searching
	first++;
	int nestLevel = 1;  //Already got the opening parenthesis

	size_t last = std::string::npos;
	size_t len = input.length();
	bool quote = false;
	for(size_t i = first; i < len; i++)
	{
		const char c = input[i];
		switch(c)
		{
		case '(':
			nestLevel++;
			break;
		case ')':
			nestLevel--;
			if(nestLevel < 0)
				i = input.length();  //Quit
			last = i;
			break;
		case ',':
			if(quote == false)
			{
				if(nestLevel != 1)
				{
					g_Log.AddMessageFormat("Unexpected embedded comma: [%s]", input.c_str());
					break;
				}
				output.push_back(input.substr(first, i - first));
				first = i + 1;
			}
			break;
		case '"':
			quote = !quote;
			break;
		default:
			break;
		}
	}
	if(nestLevel != 0)
	{
		g_Log.AddMessageFormat("Malformed parenthesis: [%s]", input.c_str());
		return false;
	}
	if(last != std::string::npos && first != last)
	{
		output.push_back(input.substr(first, last - first));
		return true;
	}
	return false;
}

//Modify a string that contains quote-wrapped text to remove anything before
//and after the quotes, including the quotes themselves.
//Example: [  "example"  ]    --->   [example]
void TrimQuote(std::string &modify)
{
	size_t pos = modify.find_first_of("\"");
	if(pos != std::string::npos)
		modify.erase(0, pos + 1);

	pos = modify.find_last_of("\"");
	if(pos != std::string::npos)
		modify.erase(pos, modify.length());
}

//Remove the outer parenthesis from a string, if they exist.
void TrimParenthesis(std::string &modify)
{
	size_t pos = modify.find_first_of("(");
	if(pos != std::string::npos)
		modify.erase(0, pos + 1);

	pos = modify.find_last_of(")");
	if(pos != std::string::npos)
		modify.erase(pos, modify.length());
}

void RemoveTrailingNewlines(char *str)
{
	size_t len = strlen(str);
	for(size_t i = len - 1; i >= 0; i--)
	{
		if(str[i] == '\r' || str[i] == '\n')
			str[i] = 0;
		else
			return;
	}
}

/*
void SplitDelimQuote(const std::string &input, STRINGLIST &output)
{
    //Note: Some special case functions like "PortalRequest" have string arguments.
	//Skip over any quotes that might be within parenthesis. 
	size_t len = input.length();
	int nestLevel = 0;
	bool quote = false;
	size_t first = 0;
	for(size_t i = 0; i < len; i++)
	{
		const char c = input[i];
		switch(c)
		{
		case '"':
			if(nestLevel == 0)
				quote = !quote;
			break;
		case ',':
			if(quote == false)
			{
				output.push_back(input.substr(first, i - first));
				first = i + 1;
			}
			break;
		case '(':
			nestLevel++;
			break;
		case ')':
			nestLevel--;
			break;
		}
	}
	if(first < len)
		output.push_back(input.substr(first, len - first));
}
*/

AbilityFunction2 :: AbilityFunction2()
{
	Clear();
}

void AbilityFunction2 :: Clear(void)
{
	mChance = 0;
	mChanceID = 0;
	mCached = false;
	mFunctionName.clear();
	mArguments.clear();
	mArgumentCache.clear();
}

void AbilityFunction2 :: Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const
{
	if(parent->VerifyFunctionName(mFunctionName) == false)
		verifyInfo.AddError("Undefined function [%s]", mFunctionName.c_str());

	for(size_t args = 0; args < mArguments.size(); args++)
		parent->VerifyFunctionArgument(mFunctionName, args, mArguments[args], verifyInfo);
}

// Expects a generic formula string like: test(a, b, c)
// Or in special cases string arguments like: test("a", "b")
void AbilityFunction2 :: AssignFormula(const std::string &formula)
{
	STRINGLIST result;
	SplitFunction(formula, result);
	if(result.size() > 0)
		mFunctionName = result[0];
	for(size_t i = 1; i < result.size(); i++)
	{
		Util::TrimWhitespace(result[i]);
		if(result[i].find('"') != std::string::npos)
			TrimQuote(result[i]);
		mArguments.push_back(result[i]);
	}
}

const char* AbilityFunction2 :: GetString(size_t argIndex) const
{
	if(argIndex >= mArguments.size())
		return NULL;
	return mArguments[argIndex].c_str();
}

int AbilityFunction2 :: GetInteger(size_t argIndex) const
{
	if(argIndex >= mArguments.size())
		return 0;
	return atoi(mArguments[argIndex].c_str());
}

float AbilityFunction2 :: GetFloat(size_t argIndex) const
{
	if(argIndex >= mArguments.size())
		return 0.0F;
	return static_cast<float>(atof(mArguments[argIndex].c_str()));
}

float AbilityFunction2 :: GetEvaluation(size_t argIndex, AbilityManager2 *symbolResolver)
{
	if(argIndex >= mArguments.size())
	{
		g_Log.AddMessageFormat("GetEvaluation() not enough arguments");
		return 0.0F;
	}
	if(mCached == false)
	{
		//Expand the cache to contain at least as many objects as arguments.
		if(mArgumentCache.size() < mArguments.size())
		{
			STRINGLIST newArg;
			mArgumentCache.reserve(mArguments.size());
			for(size_t i = 0; i < mArguments.size(); i++)
				mArgumentCache.push_back(newArg);
		}
		STRINGLIST tokens;
		for(size_t i = 0; i < mArguments.size(); i++)
		{
			Formula::TokenizeExpression(mArguments[i], tokens);
			Formula::ExpandAssociativeTokens(tokens, NULL);
			//Formula::VerifyVariableNames(tokens, symbolResolver, NULL);
			Formula::PreparePostfix(tokens, mArgumentCache[i]);
			tokens.clear();
		}
		mCached = true;
	}
	return static_cast<float>(Formula::Evaluate(mArgumentCache[argIndex], symbolResolver, NULL));
}


AbilityEvent2 :: AbilityEvent2()
{
	mTargetType = TargetType::None;
	mTargetRange = 0;
	mChanceID = 0;
}

AbilityEvent2 :: ~AbilityEvent2()
{
}

void AbilityEvent2 :: SetFunctionEvent(const std::string &eventFunctionList)
{
	STRINGLIST extract;
	SplitFunctionList(eventFunctionList, extract);

	AbilityFunction2 newFunction;

	for(size_t i = 0; i < extract.size(); i++)
	{
		Util::TrimWhitespace(extract[i]);

		if(extract[i].find("~") == 0)
			AddChanceFunctionList(extract[i]);
		else
		{
			newFunction.AssignFormula(extract[i]);
			mFunctionList.push_back(newFunction);
			newFunction.Clear();
		}
	}
}

//This is a special case function, only called if a list of function calls is supposed
//to be triggered on "chance".
//Example string: ~25%{Interrupt()}
void AbilityEvent2 :: AddChanceFunctionList(const std::string &eventFunctionList)
{
	AbilityFunction2 newFunction;

	size_t first = eventFunctionList.find("~");
	size_t second = eventFunctionList.find("%");
	if(first == std::string::npos || second == std::string::npos)
	{
		g_Log.AddMessageFormat("Missing percentage chance in chance function list [%s]", eventFunctionList.c_str());
		return;
	}
	first++; //Advance beyond the tilde
	std::string ext = eventFunctionList.substr(first, second - first);
	unsigned char chance = static_cast<unsigned char>(atoi(ext.c_str()));
	if(chance > 100)
		chance = 100;

	first = eventFunctionList.find("{", second);
	second = eventFunctionList.find("}", second);
	if(first == std::string::npos || second == std::string::npos)
	{
		g_Log.AddMessageFormat("Missing curly braces in chance function list: [%s]", eventFunctionList.c_str());
		return;
	}
	first++; //Advance beyond the opening brace
	ext = eventFunctionList.substr(first, second - first);

	STRINGLIST functionList;
	SplitFunctionList(ext, functionList);
	for(size_t i = 0; i < functionList.size(); i++)
	{
		Util::TrimWhitespace(functionList[i]);

		newFunction.mChance = chance;
		newFunction.mChanceID = mChanceID;
		newFunction.AssignFormula(functionList[i]);
		mFunctionList.push_back(newFunction);
		newFunction.Clear();
	}
	mChanceID++;
}

void AbilityEvent2 :: SetFullEvent(const STRINGLIST &eventParams)
{
	if(eventParams.size() != 3)
		g_Log.AddMessageFormat("SetFullEvent failed (not enough parameters)");
	mActionType = eventParams[0];
	mTargetTypeStr.AssignFormula(eventParams[1]);
	SetFunctionEvent(eventParams[2]);

	ResolveTargetingInfo();
}

void AbilityEvent2 :: ResolveTargetingInfo(void)
{
	mTargetType = TargetType::GetTargetTypeByName(mTargetTypeStr.mFunctionName);
	if(mTargetTypeStr.mArguments.size() > 0)
		mTargetRange = atoi(mTargetTypeStr.mArguments[0].c_str());
}

void AbilityEvent2 :: DebugPrint(void)
{
	g_Log.AddMessageFormat("ActionType: [%s]", mActionType.c_str());
	g_Log.AddMessageFormat("TargetTypeStr: [%s]", mTargetTypeStr.mFunctionName.c_str());
	for(size_t i = 0; i < mTargetTypeStr.mArguments.size(); i++)
		g_Log.AddMessageFormat("  [%d]=[%s]", i, mTargetTypeStr.mArguments[i].c_str());
	g_Log.AddMessageFormat("Functions:%d", mFunctionList.size());
	for(size_t i = 0; i < mFunctionList.size(); i++)
	{
		g_Log.AddMessageFormat("  [%d]=[%s]", i, mFunctionList[i].mFunctionName.c_str());
		for(size_t a = 0; a < mFunctionList[i].mArguments.size(); a++)
			g_Log.AddMessageFormat("    [%d]=[%s]", a, mFunctionList[i].mArguments[a].c_str());
	}
}

void AbilityEvent2 :: Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const
{
	for(size_t i = 0; i < mFunctionList.size(); i++)
		mFunctionList[i].Verify(parent, verifyInfo);
}

//A helper function to determine if certain functions are present when resolving ability
//flag information.
const char* AbilityEvent2 :: GetFunctionArgument(const char *functionName, size_t argIndex)
{
	for(size_t i = 0; i < mFunctionList.size(); i++)
	{
		if(mFunctionList[i].mFunctionName.compare(functionName) == 0)
		{
			if(argIndex < mFunctionList[i].mArguments.size())
				return mFunctionList[i].mArguments[argIndex].c_str();

			return NULL;
		}
	}
	return NULL;
}

bool AbilityEvent2 :: HasTargetType(void)
{
	if(mTargetTypeStr.mFunctionName.size() > 0)
		return true;
	return false;
}

bool AbilityEvent2 :: HasDifferentTargetType(AbilityEvent2 *other)
{
	if(other == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] HasDifferentTargetType other is NULL");
		return true;
	}
	if((mTargetType != other->mTargetType) || (mTargetRange != other->mTargetRange))
		return true;

	return false;
}

void AbilityEvent2 :: CopyTargetType(const AbilityEvent2 &other)
{
	mTargetTypeStr.mFunctionName = other.mTargetTypeStr.mFunctionName;
	mTargetTypeStr.mArguments = other.mTargetTypeStr.mArguments;

	mTargetType = other.mTargetType;
	mTargetRange = other.mTargetRange;
}

AbilityEntry2 :: AbilityEntry2()
{
	mAbilityID = 0;
	mAbilityGroupID = 0;
	mTargetType = 0;
	mTargetTypeRange = 0;
	mTargetFilter = 0;
	mAbilityType = 0;
	mAbilityFlags = 0;
	mTier = 0;
	mReqLevel = 0;
	mReqCostOtherClass = 0;
	mReqCostInClass = 0;
	//mReqAbilityID.clear();

	mWarmupTime = 0;
	mChannelDuration = 0;
	mChannelIteration = 0;
	mCooldownTime = 0;
}

AbilityEntry2 :: ~AbilityEntry2()
{
}

void AbilityEntry2 :: ImportRow(const STRINGLIST &newRowData)
{
	mRowData = newRowData;
	InitializeData();
	FinalizeData();
}

void AbilityEntry2 :: DebugPrint(void)
{
	for(size_t i = 0; i < EventType::MAX_EVENT; ++i)
	{
		if(mEvents[i].mFunctionList.size() == 0)
			continue;
		g_Log.AddMessageFormat("Event:%d", i);
		mEvents[i].DebugPrint();
	}
}

void AbilityEntry2 :: Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const
{
	for(size_t i = 0; i < EventType::MAX_EVENT; ++i)
		mEvents[i].Verify(parent, verifyInfo);
}

AbilityEvent2* AbilityEntry2 :: GetEvent(int eventID)
{
	if(eventID < 0 || eventID >= EventType::MAX_EVENT)
		return NULL;
	return &mEvents[eventID];
}


void AbilityEntry2 :: InitializeData(void)
{
	//Example: Healing Breeze uses this function setup:
	// CONDITIONS
	//   NotSilenced(),Facing,InRange(100.0),Will(4),Might(4),Reagent(20524,3)
	//
	// ACTIONS
	//   onIterate:ST:Heal((A_SPIRIT*2.1448+20)/8);onActivate:ST:Heal(A_SPIRIT*4.2896+40)

	//Set up the conditions first.
	AbilityEvent2 *eventPtr = &mEvents[EventType::onRequest];
	eventPtr->mActionType = "onRequest";
	eventPtr->SetFunctionEvent(mRowData[9]);

	int abilityID = atoi(mRowData[0].c_str());

	eventPtr = NULL;
	STRINGLIST eventList;
	Split(mRowData[10], ";", eventList);
	for(size_t i = 0; i < eventList.size(); i++)
	{
		STRINGLIST eventParams;
		Split(eventList[i], ":", eventParams);
		if(eventParams.size() == 3)  //Expects: 0=[onIterate] 1=[ST] 2=[Function(),...]
		{
			int index = EventType::GetEventIDByName(eventParams[0]);
			if(index == EventType::UNDEFINED)
				g_Log.AddMessageFormat("[WARNING] Ability:%d unknown event [%s]", abilityID, eventParams[0].c_str());
			else
			{
				eventPtr = &mEvents[index];
				eventPtr->SetFullEvent(eventParams);
			}
		}	
		else if(eventParams.size() != 0)
		{
			g_Log.AddMessageFormat("[WARNING] Ability:%d malformed event [%s]", abilityID, eventList[i].c_str());
		}
	}
}

void AbilityEntry2 :: FinalizeData(void)
{
	//Perform some basic optimizations by converting TargetType and TargetFilter flags from strings.
	//Also perform some other lookups and checks for ability types.

	//If the condition string doesn't have a target type, acquire one from the action or iteration effects.
	//It needs to have a proper target type to enumerate a list of potential targets.
	AbilityEvent2 *onReq = &mEvents[EventType::onRequest];
	if(onReq->HasTargetType() == false)
	{
		if(mEvents[EventType::onActivate].HasTargetType() == true)
			onReq->CopyTargetType(mEvents[EventType::onActivate]);
		else if(mEvents[EventType::onIterate].HasTargetType() == true)
			onReq->CopyTargetType(mEvents[EventType::onIterate]);
	}

	mAbilityID = GetRowAsInteger(ABROW::ID);
	mAbilityGroupID = GetRowAsInteger(ABROW::GROUP_ID);

	//onRequest contains function information for selecting a target.  First get a pointer to make the
	//access code easier to read.  Then resolve the ability TargetType integer from the name.
	//If it has a function argument, it's probably a numerical value indicating range or radius.
	//Convert that to an integer for easier use.
	AbilityFunction2 *initFunction = &mEvents[EventType::onRequest].mTargetTypeStr;
	mTargetType = TargetType::GetTargetTypeByName(initFunction->mFunctionName);
	if(initFunction->mArguments.size() > 0)
		mTargetTypeRange = atoi(initFunction->mArguments[0].c_str());

	mTargetFilter = TargetFilter::GetTargetFlagsByName(mRowData[ABROW::TARGET_STATUS]);
	mWarmupTime = GetRowAsInteger(ABROW::WARMUP_TIME);
	mChannelDuration = GetRowAsInteger(ABROW::DURATION);
	mChannelIteration = GetRowAsInteger(ABROW::INTERVAL);
	mCooldownTime = GetRowAsInteger(ABROW::COOLDOWN_TIME);

	const char *param;
	param = GetFunctionArgument(EventType::onRequest, "InRange", 0);
	if(param != NULL)
	{
		int range = atoi(param);
		if(range > 30) //30 units = 3 meters
			mAbilityFlags |= AbilityFlags::Ranged;
		else 
			mAbilityFlags |= AbilityFlags::Melee;
	}

	if(GetFunctionArgument(EventType::UNDEFINED, "MeleeDamage", 0) != NULL)
		mAbilityFlags |= AbilityFlags::PhysicalDamage;

	if(GetFunctionArgument(EventType::UNDEFINED, "FireDamage", 0) != NULL)
		mAbilityFlags |= AbilityFlags::MagicDamage;

	if(GetFunctionArgument(EventType::UNDEFINED, "FrostDamage", 0) != NULL)
		mAbilityFlags |= AbilityFlags::MagicDamage;

	if(GetFunctionArgument(EventType::UNDEFINED, "MysticDamage", 0) != NULL)
		mAbilityFlags |= AbilityFlags::MagicDamage;

	if(GetFunctionArgument(EventType::UNDEFINED, "DeathDamage", 0) != NULL)
		mAbilityFlags |= AbilityFlags::MagicDamage;

	//Default to instant.
	mAbilityType = AbilityType::Instant;
	if(mWarmupTime > 0)
		mAbilityType = AbilityType::Cast;

	if((mRowData[ABROW::ABILITY_CLASS].compare("Passive") == 0) || (GetRowAsInteger(ABROW::USE_TYPE) == AbilityUseType::PASSIVE))
	{
		mAbilityType = AbilityType::Passive;
		mAbilityFlags |= AbilityFlags::Passive;

		//Some passives have implicit actions that need to be registered, but no conditions or actions
		//so there isn't a target type to derive from.  Assume self instead.
		if(mTargetType == TargetType::None)
			mTargetType = TargetType::Self;
	}
	else if(mRowData[ABROW::ABILITY_CLASS].compare("Charge") == 0)
		mAbilityFlags |= AbilityFlags::Charge;
	else if(mRowData[ABROW::ABILITY_CLASS].compare("Execute") == 0)
		mAbilityFlags |= AbilityFlags::Execute;

	if(mChannelDuration > 0)
	{
		mAbilityFlags |= AbilityFlags::Channel;
		if(GetFunctionArgument(EventType::UNDEFINED, "AddWDesc", 0) != NULL)
			mAbilityFlags |= AbilityFlags::SecondaryChannel;

		if(mEvents[EventType::onActivate].mTargetType != mEvents[EventType::onIterate].mTargetType)
			mAbilityFlags |= AbilityFlags::SecondaryTarget;

		//if((mAbilityFlags & AbilityFlags::Execute) || (mAbilityFlags & AbilityFlags::Charge))
		//	mAbilityFlags |= AbilityFlags::UnbreakableChannel;
	}

	if(mWarmupTime > 0 && mChannelDuration > 0)
		mAbilityFlags |= AbilityFlags::SecondaryChannel;

	if(mRowData.size() > AbilityManager2::REQUIRED_ROW_ENTRIES)
	{
		STRINGLIST flagList;
		Util::Split(GetRowAsString(ABROW::SERVER_FLAGS), "|", flagList);
		for(size_t i = 0; i < flagList.size(); i++)
		{
			Util::ToLowerCase(flagList[i]);
			if(flagList[i].compare("secondarychannel") == 0)
				mAbilityFlags |= AbilityFlags::SecondaryChannel;
			else if(flagList[i].compare("unbreakablechannel") == 0)
				mAbilityFlags |= AbilityFlags::UnbreakableChannel;
			else if(flagList[i].compare("allowdeadstate") == 0)
				mAbilityFlags |= AbilityFlags::AllowDeadState;
			else if(flagList[i].size() > 0)
				g_Log.AddMessageFormat("Unknown server flag [%s] for ability [%d]", flagList[i].c_str(), mAbilityID);
		}
	}
	
	if(mEvents[EventType::onParry].mFunctionList.size() > 0)
		mAbilityFlags |= AbilityFlags::ImplicitParry;

	mTier = GetRowAsInteger(ABROW::TIER);
	//The ability purchase prereq string looks like this:
	//Example (Mystic Specialization):  40,4,4,(53|502),KRMD
	/*
	[0] integer: level
	[1] integer: purchase point cost (cross class)
	[2] integer: purchase point cost (in class)
	[3] string: ability IDs of a required prereq ability, separated by "|".
	[4] string: Contains a letter for each class that may purchase the ability 'K', 'R', 'M', 'D'
	*/
	STRINGLIST preReq;
	Split(mRowData[ABROW::PREREQ], ",", preReq);
	if(preReq.size() > 0)
		mReqLevel = atoi(preReq[0].c_str());
	if(preReq.size() > 1)
		mReqCostOtherClass = atoi(preReq[1].c_str());
	if(preReq.size() > 2)
		mReqCostInClass = atoi(preReq[2].c_str());
	if(preReq.size() > 3)
	{
		STRINGLIST IDList;
		TrimParenthesis(preReq[3]);
		Split(preReq[3], "|", IDList);
		for(size_t i = 0; i < IDList.size(); i++)
		{
			if(IDList[i].size() > 0)   //Was attempting to add empty strings.
				mReqAbilityID.push_back(atoi(IDList[i].c_str()));
		}
	}
	if(preReq.size() > 4)
	{
		for(size_t i = 0; i < preReq[4].size(); i++)
		{
			switch(preReq[4][i])
			{
			case 'K': mAbilityFlags |= AbilityFlags::Knight; break;
			case 'R': mAbilityFlags |= AbilityFlags::Rogue; break;
			case 'M': mAbilityFlags |= AbilityFlags::Mage; break;
			case 'D': mAbilityFlags |= AbilityFlags::Druid; break;
			default:
				g_Log.AddMessageFormat("Unknown class specifier [%c] for ability [%d]", preReq[4][i], mAbilityID);
			}
		}
	}
}

//Search a specific event type, (or all events of EventType::UNDEFINED) to see if the action list
//contains a particular function.  If it does, return the string of the specified argument index.
const char* AbilityEntry2 :: GetFunctionArgument(int eventType, const char *functionName, size_t argIndex)
{
	const char *result = NULL;
	if(eventType == EventType::UNDEFINED)
	{
		for(int i = 0; i < EventType::MAX_EVENT; i++)
		{
			result = mEvents[i].GetFunctionArgument(functionName, argIndex);
			if(result != NULL)
				break;
		}
	}
	else
	{
		if(eventType >= 0 && eventType < EventType::MAX_EVENT)
			result = mEvents[eventType].GetFunctionArgument(functionName, argIndex);
	}
	return result;
}

const char* AbilityEntry2 :: GetRowAsCString(size_t index) const
{
	if(index >= mRowData.size())
		return NULL;
	return mRowData[index].c_str();
}

const std::string& AbilityEntry2 :: GetRowAsString(size_t index) const
{
	static const std::string nullString;
	if(index >= mRowData.size())
		return nullString;
	return mRowData[index];
}

int AbilityEntry2 :: GetRowAsInteger(size_t index) const
{
	if(index >= mRowData.size())
		return 0;
	return atoi(mRowData[index].c_str());
}

bool AbilityEntry2 :: IsPhysicalDamageOnly(void) const
{
	if(!(mAbilityFlags & AbilityFlags::PhysicalDamage))
		return false;
	if(mAbilityFlags & AbilityFlags::MagicDamage)
		return false;

	return true;
}

bool AbilityEntry2 :: IsPassive(void) const
{
	return ((mAbilityFlags & AbilityFlags::Passive) != 0);
}

bool AbilityEntry2 :: IsPurchaseableBy(int profession) const
{
	int searchFlag = 0; 
	switch(profession)
	{
	case Professions::KNIGHT: searchFlag = AbilityFlags::Knight; break;
	case Professions::ROGUE: searchFlag = AbilityFlags::Rogue; break;
	case Professions::MAGE: searchFlag = AbilityFlags::Mage; break;
	case Professions::DRUID: searchFlag = AbilityFlags::Druid; break;
	}

	if(mAbilityFlags & searchFlag)
		return true;

	return false;
}

int AbilityEntry2 :: GetPurchaseCost(void) const
{
	if(mReqCostInClass < MINIMUM_ABILITY_COST)
		return MINIMUM_ABILITY_COST;
	return mReqCostInClass;
}

int AbilityEntry2 :: GetPurchaseLevel(void) const
{
	return mReqLevel;
}

void AbilityEntry2 :: GetPurchasePrereqList(std::vector<short> &output) const
{
	output.assign(mReqAbilityID.begin(), mReqAbilityID.end());
}

bool AbilityEntry2 :: CanUseWhileDead(void) const
{
	if(AbilityManager2::IsGlobalIntrinsicAbility(mAbilityID) == true)
		return true;
	if(mAbilityFlags & AbilityFlags::Passive)
		return true;
	if(mAbilityFlags & AbilityFlags::AllowDeadState)
		return true;
	
	return false;
}

AbilityManager2 :: AbilityManager2()
{
	mFunctionTablesLoaded = false;
}

AbilityManager2 :: ~AbilityManager2()
{
}

void AbilityManager2 :: InitFunctionTables(void)
{
	if(mFunctionTablesLoaded == true)
		return;

	InsertFunction("Facing", &Ability2::AbilityCalculator::Facing);
	InsertFunction("InRange", &AbilityCalculator::InRange);
	InsertFunction("NotSilenced", &AbilityCalculator::NotSilenced);
	InsertFunction("HasStatus", &AbilityCalculator::HasStatus);
	InsertFunction("NotStatus", &AbilityCalculator::NotStatus);
	InsertFunction("Interrupt", &AbilityCalculator::Interrupt);

	InsertFunction("Might", &AbilityCalculator::Might);
	InsertFunction("Will", &AbilityCalculator::Will);
	InsertFunction("MightCharge", &AbilityCalculator::MightCharge);
	InsertFunction("WillCharge", &AbilityCalculator::WillCharge);
	InsertFunction("A_AddMightCharge", &AbilityCalculator::A_AddMightCharge);
	InsertFunction("A_AddWillCharge", &AbilityCalculator::A_AddWillCharge);
	InsertFunction("T_AddMightCharge", &AbilityCalculator::T_AddMightCharge);
	InsertFunction("T_AddWillCharge", &AbilityCalculator::T_AddWillCharge);
	InsertFunction("StealWillCharge", &AbilityCalculator::StealWillCharge);
	InsertFunction("StealMightCharge", &AbilityCalculator::StealMightCharge);
	InsertFunction("FullMight", &AbilityCalculator::FullMight);
	InsertFunction("FullWill", &AbilityCalculator::FullWill);

	InsertFunction("hasMainHandWeapon", &AbilityCalculator::hasMainHandWeapon);
	InsertFunction("hasOffHandWeapon", &AbilityCalculator::hasOffHandWeapon);
	InsertFunction("hasMeleeWeapon", &AbilityCalculator::hasMeleeWeapon);
	InsertFunction("hasRangedWeapon", &AbilityCalculator::hasRangedWeapon);
	InsertFunction("hasShield", &AbilityCalculator::hasShield);
	InsertFunction("hasBow", &AbilityCalculator::hasBow);
	InsertFunction("has2HorPoleWeapon", &AbilityCalculator::has2HorPoleWeapon);
	InsertFunction("hasWand", &AbilityCalculator::hasWand);

	InsertFunction("Status", &AbilityCalculator::Status);
	InsertFunction("Amp", &AbilityCalculator::Amp);
	InsertFunction("Add", &AbilityCalculator::Add);
	InsertFunction("AmpCore", &AbilityCalculator::AmpCore);
	InsertFunction("Nullify", &AbilityCalculator::Nullify);
	InsertFunction("CheckBuffLimits", &AbilityCalculator::CheckBuffLimits);

	InsertFunction("MeleeDamage", &AbilityCalculator::MeleeDamage);
	InsertFunction("FireDamage", &AbilityCalculator::FireDamage);
	InsertFunction("FrostDamage", &AbilityCalculator::FrostDamage);
	InsertFunction("MysticDamage", &AbilityCalculator::MysticDamage);
	InsertFunction("DeathDamage", &AbilityCalculator::DeathDamage);
	InsertFunction("Heal", &AbilityCalculator::Heal);
	InsertFunction("A_Heal", &AbilityCalculator::A_Heal);
	InsertFunction("Harm", &AbilityCalculator::Harm);
	InsertFunction("PercentMaxHealth", &AbilityCalculator::PercentMaxHealth);
	InsertFunction("Regenerate", &AbilityCalculator::Regenerate);

	InsertFunction("AttackMelee", &AbilityCalculator::AttackMelee);
	InsertFunction("AttackRanged", &AbilityCalculator::AttackRanged);

	InsertFunction("Taunt", &AbilityCalculator::Taunt);
	InsertFunction("AddHate", &AbilityCalculator::AddHate);
	InsertFunction("TopHated", &AbilityCalculator::TopHated);
	InsertFunction("UnHate", &AbilityCalculator::UnHate);
	InsertFunction("LeaveCombat", &AbilityCalculator::LeaveCombat);

	InsertFunction("Behind", &AbilityCalculator::Behind);
	InsertFunction("Spin", &AbilityCalculator::Spin);
	InsertFunction("Duration", &AbilityCalculator::Duration);

	InsertFunction("Resurrect", &AbilityCalculator::Resurrect);
	InsertFunction("NearbySanctuary", &AbilityCalculator::NearbySanctuary);
	InsertFunction("BindTranslocate", &AbilityCalculator::BindTranslocate);
	InsertFunction("Translocate", &AbilityCalculator::Translocate);
	InsertFunction("PortalRequest", &AbilityCalculator::PortalRequest);
	InsertFunction("Go", &AbilityCalculator::Go);
	InsertFunction("GoSanctuary", &AbilityCalculator::GoSanctuary);
	
	InsertFunction("RemoveAltDebuff", &AbilityCalculator::RemoveAltDebuff);
	InsertFunction("RemoveStatDebuff", &AbilityCalculator::RemoveStatDebuff);
	InsertFunction("RemoveEtcDebuff", &AbilityCalculator::RemoveEtcDebuff);
	InsertFunction("RemoveAltBuff", &AbilityCalculator::RemoveAltBuff);
	InsertFunction("RemoveStatBuff", &AbilityCalculator::RemoveStatBuff);
	InsertFunction("RemoveEtcBuff", &AbilityCalculator::RemoveEtcBuff);
	InsertFunction("RemoveHealthBuff", &AbilityCalculator::RemoveHealthBuff);

	InsertFunction("Invisible", &AbilityCalculator::Invisible);
	InsertFunction("WalkInShadows", &AbilityCalculator::WalkInShadows);
	InsertFunction("AddWDesc", &AbilityCalculator::AddWDesc);
	InsertFunction("HealthSacrifice", &AbilityCalculator::HealthSacrifice);
	InsertFunction("DoNothing", &AbilityCalculator::DoNothing);
	InsertFunction("Reagent", &AbilityCalculator::Reagent);

	//Non-official functions:
	InsertFunction("SetAutoAttack", &AbilityCalculator::SetAutoAttack);
	InsertFunction("DisplayEffect", &AbilityCalculator::DisplayEffect);
	InsertFunction("InterruptChance", &AbilityCalculator::InterruptChance);
	InsertFunction("Transform", &AbilityCalculator::Transform);
	InsertFunction("Nudify", &AbilityCalculator::Nudify);
	InsertFunction("Untransform", &AbilityCalculator::Untransform);
	InsertFunction("NotTransformed", &AbilityCalculator::NotTransformed);
	
	//The verifier indicates which argument indexes should be flagged for examination
	//as valid expressions.
	InsertVerifier("Status",  ABVerifier(ABVerifier::EFFECT, ABVerifier::TIME));  //Status(statusEffect, time)
	InsertVerifier("NotSilenced", ABVerifier());                          //NotSilenced()
	InsertVerifier("NotTransformed", ABVerifier());                          //NotTransformed()
	InsertVerifier("HasStatus", ABVerifier(ABVerifier::EFFECT));          //HasStatus(effectName)
	InsertVerifier("NotStatus", ABVerifier(ABVerifier::EFFECT));          //NotStatus(effectName)
	InsertVerifier("Interrupt", ABVerifier());                            //Interrupt()

	InsertVerifier("Add",     ABVerifier(ABVerifier::STATID, ABVerifier::AMOUNT, ABVerifier::TIME));  //Add(statID, amount, time)
	InsertVerifier("Amp",     ABVerifier(ABVerifier::STATID, ABVerifier::AMOUNT, ABVerifier::TIME));  //Amp(statID, amount, time)
	InsertVerifier("AmpCore", ABVerifier(ABVerifier::AMOUNT, ABVerifier::TIME));    //AmpCore(amount, time)
	InsertVerifier("Nullify", ABVerifier(ABVerifier::STATID, ABVerifier::AMOUNT));  //Nullify(amount, time)

	InsertVerifier("Heal", ABVerifier(ABVerifier::AMOUNT));  //Heal(amount)
	InsertVerifier("A_Heal", ABVerifier(ABVerifier::AMOUNT));  //A_Heal(amount)
	InsertVerifier("Harm", ABVerifier(ABVerifier::AMOUNT));  //Harm(damage)
	InsertVerifier("PercentMaxHealth", ABVerifier(ABVerifier::AMOUNT));   //PercentMaxHealth(floatPercent)
	InsertVerifier("Regenerate", ABVerifier(ABVerifier::AMOUNT));  //Regenerate(amount);

	InsertVerifier("MeleeDamage", ABVerifier(ABVerifier::AMOUNT));  //MeleeDamage(damage)
	InsertVerifier("MysticDamage", ABVerifier(ABVerifier::AMOUNT));  //MysticDamage(damage)
	InsertVerifier("DeathDamage", ABVerifier(ABVerifier::AMOUNT));  //DeathDamage(damage)
	InsertVerifier("FireDamage", ABVerifier(ABVerifier::AMOUNT));  //FireDamage(damage)
	InsertVerifier("FrostDamage", ABVerifier(ABVerifier::AMOUNT));  //FrostDamage(damage)

	InsertVerifier("A_AddMightCharge", ABVerifier(ABVerifier::AMOUNT));  //A_AddMightCharge(amount)
	InsertVerifier("A_AddWillCharge", ABVerifier(ABVerifier::AMOUNT));  //A_AddWillCharge(amount)
	InsertVerifier("T_AddMightCharge", ABVerifier(ABVerifier::AMOUNT));  //T_AddMightCharge(amount)
	InsertVerifier("T_AddWillCharge", ABVerifier(ABVerifier::AMOUNT));  //T_AddWillCharge(amount)
	InsertVerifier("StealWillCharge", ABVerifier(ABVerifier::AMOUNT));  //StealWillCharge(amount)
	InsertVerifier("StealMightCharge", ABVerifier(ABVerifier::AMOUNT));  //StealMightCharge(amount)
	
	InsertVerifier("Taunt", ABVerifier(ABVerifier::TIME));  //Taunt(time)
	InsertVerifier("AddHate", ABVerifier(ABVerifier::AMOUNT));  //AddHate(hateAmount)
	InsertVerifier("TopHated", ABVerifier(ABVerifier::AMOUNT));  //TopHated(unknownAmount)
	//InsertVerifier("UnHate", ABVerifier());  //UnHate()
	//InsertVerifier("LeaveCombat", ABVerifier());  //LeaveCombat()

	//InsertVerifier("Behind", ABVerifier());  //Behind()
	//InsertVerifier("Spin", ABVerifier());  //Spin()
	InsertVerifier("Duration", ABVerifier(ABVerifier::AMOUNT));  //Duration(timeSec)

	InsertVerifier("Resurrect", ABVerifier(ABVerifier::AMOUNT, ABVerifier::AMOUNT));  //Resurrect(healthRatio,luckRatio)
//	InsertVerifier("NearbySanctuary", ABVerifier());  //NearbySanctuary()
//	InsertVerifier("BindTranslocate", ABVerifier());  //BindTranslocate()
//	InsertVerifier("Translocate", ABVerifier());  //Translocate()
	InsertVerifier("PortalRequest", ABVerifier(ABVerifier::STRING, ABVerifier::STRING));  //PortalRequest(internalName, prettyName)
	InsertVerifier("Go", ABVerifier(ABVerifier::STRING));  //Go(destName)
	//InsertVerifier("GoSanctuary", ABVerifier());  //GoSanctuary()

	InsertVerifier("SummonPet", ABVerifier(ABVerifier::AMOUNT));  //SummonPet(unknown)

	InsertVerifier("GTAE", ABVerifier(ABVerifier::DIST));  //GTAE(distance)
	InsertVerifier("PBAE", ABVerifier(ABVerifier::DIST));  //PBAE(distance)
	InsertVerifier("Party", ABVerifier(ABVerifier::DIST));  //Party(distance)
	InsertVerifier("Cone90", ABVerifier(ABVerifier::DIST));  //Cone90(distance)
	InsertVerifier("Cone180", ABVerifier(ABVerifier::DIST));  //Cone180(distance)
	InsertVerifier("STAE", ABVerifier(ABVerifier::DIST));  //STXAE(distance)
	InsertVerifier("STXAE", ABVerifier(ABVerifier::DIST));  //STXAE(distance)

	InsertVerifier("InRange", ABVerifier(ABVerifier::DIST));  //InRange(distance)
	InsertVerifier("CheckBuffLimits", ABVerifier(ABVerifier::TIER, ABVerifier::BUFF));  //CheckBuffLimits(amount,buffName)
	InsertVerifier("Will", ABVerifier(ABVerifier::AMOUNT));  //Will(amount)
	InsertVerifier("Might", ABVerifier(ABVerifier::AMOUNT));  //Might(amount)
	InsertVerifier("WillCharge", ABVerifier(ABVerifier::AMOUNT, ABVerifier::AMOUNT));  //WillCharge(min,max)
	InsertVerifier("MightCharge", ABVerifier(ABVerifier::AMOUNT, ABVerifier::AMOUNT));  //MightCharge(min,max)

	InsertVerifier("Invisible", ABVerifier(ABVerifier::AMOUNT, ABVerifier::AMOUNT));  //Invisible(durationSec,distMeters)
	InsertVerifier("WalkInShadows", ABVerifier(ABVerifier::AMOUNT, ABVerifier::AMOUNT));  //WalkInShadows(durationSec,distMeters)
	InsertVerifier("AddWDesc", ABVerifier(ABVerifier::STATID, ABVerifier::AMOUNT, ABVerifier::AMOUNT, ABVerifier::STRING));  //AddWDesc(statID,intervalSec,durationSec,stringDesc)
	InsertVerifier("HealthSacrifice", ABVerifier(ABVerifier::AMOUNT));  //HealthSacrifice(hitPoints)
	InsertVerifier("Reagent", ABVerifier(ABVerifier::ITEMID, ABVerifier::AMOUNT));  //Reagent(itemID,count)

	InsertVerifier("AttackMelee", ABVerifier(ABVerifier::AMOUNT));  //AttackMelee(unknown)
	InsertVerifier("AttackRanged", ABVerifier(ABVerifier::AMOUNT));  //AttackRanged(unknown)

	//Unofficial functions:
	InsertVerifier("SetAutoAttack", ABVerifier(ABVerifier::EFFECT));  //SetAutoAttack(statID)
	InsertVerifier("DisplayEffect", ABVerifier(ABVerifier::STRING));  //DisplayEffect(effectName)
	InsertVerifier("InterruptChance", ABVerifier(ABVerifier::AMOUNT)); //InterruptChance(amount);

	mFunctionTablesLoaded = true;
}

void AbilityManager2 :: InsertFunction(const char *name, FunctionPtr function)
{
	if(mFunctionMap.find(name) != mFunctionMap.end())
	{
		g_Log.AddMessageFormat("[ERROR] AbilityManager function [%s] already registered", name);
		return;
	}
	mFunctionMap[name] = function;
}

void AbilityManager2 :: InsertVerifier(const char *functionName, const ABVerifier &argInfo)
{
	if(mVerifierMap.find(functionName) != mVerifierMap.end())
	{
		g_Log.AddMessageFormat("[ERROR] AbilityManager verifier [%s] already registered", functionName);
		return;
	}
	mVerifierMap[functionName] = argInfo;
}

//Load all data files and overrides.
void AbilityManager2 :: LoadData(void)
{
	InitFunctionTables();

	//Clear it in case we got a request to reload the ability table during run-time.
	mAbilityIndex.clear();

	std::string path;
	Platform::GenerateFilePath(path, "Data", "AbilityTable.txt");
	LoadAbilityTable(path.c_str());

	Platform::GenerateFilePath(path, "Data", "AbilityTableAdmin.txt");
	LoadAbilityTable(path.c_str());

	//Need to override special case functions.
	AbilityEvent2 *evt = NULL;
	
	evt = mAbilityIndex[ABILITYID_AUTO_ATTACK].GetEvent(EventType::onRequest);
	evt->SetFunctionEvent("SetAutoAttack(AUTO_ATTACK)");

	evt = mAbilityIndex[ABILITYID_AUTO_ATTACK_RANGED].GetEvent(EventType::onRequest);
	evt->SetFunctionEvent("SetAutoAttack(AUTO_ATTACK_RANGED)");

	InitializeCategories();
	StatNameLowercase();

	Verify();
}

//Catalog all the buff and cooldown categories names in the ability table.  Ability
//processing can then resolve them into a numerical ID to pass to functions.
void AbilityManager2 :: InitializeCategories(void)
{
	ABILITY_ITERATOR abit;
	for(abit = mAbilityIndex.begin(); abit != mAbilityIndex.end(); ++abit)
	{
		if(abit->second.mRowData.size() < REQUIRED_ROW_ENTRIES)
			continue;
		mBuffCategories[abit->second.GetRowAsCString(ABROW::BUFF_CATEGORY)]++;
		mCooldownCategories[abit->second.GetRowAsCString(ABROW::COOLDOWN_CATEGORY)]++;
	}

	CONSTANT_MAP::iterator it;

	int currentID = 0;
	for(it = mBuffCategories.begin(); it != mBuffCategories.end(); ++it)
		it->second = currentID++;

	currentID = 0;
	for(it = mCooldownCategories.begin(); it != mCooldownCategories.end(); ++it)
		it->second = currentID++;

	g_Log.AddMessageFormat("Registered %d Buff Categories", mBuffCategories.size());
	g_Log.AddMessageFormat("Registered %d Cooldown Categories", mCooldownCategories.size());
}

//Required in the loading stage. Stat ID names need to be lowercase for the lookup
//to work.  Iterate through all abilities, their functions, and compare the verifier
//type, and convert if necessary.
void AbilityManager2 :: StatNameLowercase(void)
{
	ABILITY_ITERATOR abit;
	VERIFIER_ITERATOR vit;
	for(abit = mAbilityIndex.begin(); abit != mAbilityIndex.end(); ++abit)
	{
		for(int e = 0; e < EventType::MAX_EVENT; e++)
		{
			AbilityEvent2 *evt = abit->second.GetEvent(e);
			if(evt == NULL)
				continue;
			for(size_t f = 0; f < evt->mFunctionList.size(); f++)
			{
				vit = mVerifierMap.find(evt->mFunctionList[f].mFunctionName);
				if(vit == mVerifierMap.end())
					continue;
				for(size_t varg = 0; varg < ABVerifier::MAX_ARG; varg++)
				{
					if(varg >= evt->mFunctionList[f].mArguments.size())
						continue;
 
					switch(vit->second.argIndex[varg])
					{
						//The stat lookup table is all lower-case
					case ABVerifier::STATID:
						Util::ToLowerCase(evt->mFunctionList[f].mArguments[varg]);
						break;
					
					/*
					case ABVerifier::AMOUNT:
						ToLowerCase(evt->mFunctionList[f].mArguments[varg]);
						break;
					*/
					}
				}
			}
		}
	}
}

void AbilityManager2 :: LoadAbilityTable(const char *filename)
{
	FILE *input = fopen(filename, "rb");
	if(input == NULL)
	{
		g_Log.AddMessageFormat("Cannot open ability table file [%s]", filename);
		return;
	}
	char buffer[4096];
	std::string lineEntry;
	int lineNumber = 0;
	int loadCount = 0;
	while(!feof(input))
	{
		buffer[0] = 0;  //Always reset, otherwise the last line will be processed twice
		fgets(buffer, sizeof(buffer), input);
		lineNumber++;
		RemoveTrailingNewlines(buffer);

		lineEntry = buffer;

		//TODO: Debugging hack to locate bad quotation marks.  Improve this.
		int count = 0;
		for(size_t i = 0; i < strlen(buffer); i++)
		{
			if(buffer[i] == '"')
				count++;
			if((buffer[i] != '\t' && buffer[i] < ' ') || buffer[i] >= 127)
				g_Log.AddMessageFormat("Unidentified character [%c] on line [%d] of [%s]", buffer[i], lineNumber, filename);
		}
		if(count % 2 != 0)
			g_Log.AddMessageFormat("Mismatched quotation marks on line [%d] of [%s]", lineNumber, filename);
		//End hack

		//Don't process empty lines.  Prevents reporting errors for lacking the required row entries.
		if(buffer[0] == 0)
			continue;

		//Prepare the row data.
		STRINGLIST rowData;
		Util::Split(lineEntry, "\t", rowData); 
		//SplitDelimQuote(lineEntry, rowData);

		if(rowData.size() > 0)
		{
			if(rowData.size() < REQUIRED_ROW_ENTRIES)
			{
				g_Log.AddMessageFormat("[WARNING] Malformed row, %d objects (needs %d) (file:%s, line: %d)", rowData.size(), REQUIRED_ROW_ENTRIES, filename, lineNumber);
			}
			else
			{
				//Fix up the row data before inserting it into the ability index.
				for(size_t i = 0; i < rowData.size(); i++)
				{
					TrimQuote(rowData[i]);
					Util::TrimWhitespace(rowData[i]);
				}

				int abID = atoi(rowData[0].c_str());
				InsertAbility(abID, rowData);
				loadCount++;
			}
		}
	}
	fclose(input);
	g_Log.AddMessageFormat("Loaded ability file [%s], %d abilities loaded.", filename, loadCount);
}

void AbilityManager2 :: InsertAbility(int abilityID, const STRINGLIST &rowData)
{
	mAbilityIndex[abilityID].ImportRow(rowData);
}

void AbilityManager2 :: DebugPrint(void)
{
	ABILITY_ITERATOR it;
	for(it = mAbilityIndex.begin(); it != mAbilityIndex.end(); ++it)
	{
		g_Log.AddMessageFormat("Ability:%d", it->first);
		it->second.DebugPrint();
	}
}

void AbilityManager2 :: Verify(void)
{
	AbilityVerify verifyInfo;
	ABILITY_ITERATOR it;
	for(it = mAbilityIndex.begin(); it != mAbilityIndex.end(); ++it)
	{
		it->second.Verify(this, verifyInfo);
		if(verifyInfo.mError != 0)
		{
			g_Log.AddMessageFormat("Ability [%d] verification failed (%d errors)", it->first, verifyInfo.mError);
			for(size_t i = 0; i < verifyInfo.mErrorMsg.size(); i++)
				g_Log.AddMessageFormat("  %s", verifyInfo.mErrorMsg[i].c_str());
			verifyInfo.Clear();
		}
	}
}

bool AbilityManager2 :: VerifyFunctionName(const std::string &functionName)
{
	FUNCTION_ITERATOR it = mFunctionMap.find(functionName);
	if(it != mFunctionMap.end())
		return true;
	return false;
}

void AbilityManager2 :: VerifyFunctionArgument(const std::string &functionName, size_t argIndex, const std::string &argumentString, AbilityVerify &verifyInfo)
{
	//First check basic stuff in the expression.
	VerifyExpression(argumentString, verifyInfo);

	//Perform additional argument verification of arguments that could have possible
	//evaluations.
	VERIFIER_ITERATOR it = mVerifierMap.find(functionName);
	if(it == mVerifierMap.end())
		return;

	if(argIndex >= ABVerifier::MAX_ARG)
	{
		verifyInfo.AddError("Bad argument index [%d] to internal verifier", argIndex);
		return;
	}
	
	//No verification needed for this argument.
	int resolveResult = 0;
	bool resolvePostfix = false;
	switch(it->second.argIndex[argIndex])
	{
	case ABVerifier::NONE:
		return;
	case ABVerifier::AMOUNT:
		resolvePostfix = true;
		break;
	case ABVerifier::STATID:
		resolveResult = ResolveStatID(argumentString.c_str());
		if(resolveResult == -1)
			verifyInfo.AddError("Could not resolve Stat Name [%s] for function [%s]", argumentString.c_str(), functionName.c_str());
		return;
	case ABVerifier::EFFECT:
		resolveResult = ResolveStatusEffectID(argumentString.c_str());
		if(resolveResult == -1)
			verifyInfo.AddError("Could not resolve Status Effect [%s] for function [%s]", argumentString.c_str(), functionName.c_str());
		return;
	case ABVerifier::TIER:
		return;
	case ABVerifier::TIME:
		resolvePostfix = true;
		break;
	case ABVerifier::DIST:
		resolvePostfix = true;
		break;
	case ABVerifier::BUFF:
		resolveResult = ResolveBuffCategoryID(argumentString.c_str());
		if(resolveResult <= 0)
			verifyInfo.AddError("Could not resolve buff name [%s] for function [%s]", argumentString.c_str(), functionName.c_str());
		return;
	case ABVerifier::ITEMID:
		resolveResult = ResolveItemID(argumentString.c_str());
		if(resolveResult == -1)
			verifyInfo.AddError("Could not resolve Item ID [%s] for function [%s]", argumentString.c_str(), functionName.c_str());
		return;
	case ABVerifier::STRING:
		return;
	default:
		verifyInfo.AddError("Unknown argument verification type [%d] for function [%s]" , it->second.argIndex[argIndex], functionName.c_str());
		return;
	}

	if(resolvePostfix == true)
	{
		STRINGLIST tokens;
		STRINGLIST postfix;
		Formula::TokenizeExpression(argumentString, tokens);
		Formula::ExpandAssociativeTokens(tokens,  &verifyInfo);
		Formula::VerifyVariableNames(tokens, this,  &verifyInfo);
		Formula::PreparePostfix(tokens, postfix);
		Formula::TestEvaluate(postfix, &verifyInfo);
	}
}

int AbilityManager2 :: EnumerateTargets(CreatureInstance *actor, int targetType, int targetFilter, int distance)
{
	CreatureInstance::CREATURE_PTR_SEARCH searchResults;

	switch(targetType)
	{
	case TargetType::Self:
		searchResults.push_back(actor);
		break;
	case TargetType::ST:
		actor->AddTargetsST(searchResults, targetFilter);
		break;
	case TargetType::Party:
		actor->AddTargetsParty(searchResults, targetFilter, distance);
		break;
	case TargetType::Cone180:
		actor->AddTargetsConeAngle(searchResults, targetFilter, distance, Cone_180);
		break;
	case TargetType::Cone90:
		actor->AddTargetsConeAngle(searchResults, targetFilter, distance, Cone_90);
		break;
	case TargetType::GTAE:
		actor->AddTargetsGTAE(searchResults, targetFilter, distance);
		break;
	case TargetType::PBAE:
		actor->AddTargetsPBAE(searchResults, targetFilter, distance);
		break;
	case TargetType::STAE:
		actor->AddTargetsSTAE(searchResults, targetFilter, distance);
		break;
	case TargetType::STXAE:
		actor->AddTargetsSTXAE(searchResults, targetFilter, distance);
		break;
	case TargetType::STP:
		actor->AddTargetsSTP(searchResults, targetFilter);
		break;
	case TargetType::Implicit:
		break;
	}

	if(searchResults.size() == 0)
	{
		actor->ab[0].ClearTargetList();
		return 0;
	}

	actor->TransferTargets(searchResults, actor->ab[0]);
	return actor->ab[0].TargetCount;
}

int AbilityManager2 :: ActivateAbility(CreatureInstance *cInst, short abilityID, int eventType, ActiveAbilityInfo *abInfo)
{
//	g_Log.AddMessageFormat("ActivateAbility [%s] AbID:%d, Evt:%d", cInst->css.display_name, abilityID, eventType);

	//Hack for autoattacks and quest interaction triggers
	int result = CheckActivateSpecialAbility(cInst, abilityID, eventType);
	if(result != ABILITY_NOT_SPECIAL)
		return result;


	ABILITY_ITERATOR it;
	it = mAbilityIndex.find(abilityID);
	if(it == mAbilityIndex.end())
	{
		g_Log.AddMessageFormat("Ability not found in table: %d", abilityID);
		return ABILITY_NOT_FOUND;
	}
	AbilityEvent2 *abEvent = it->second.GetEvent(eventType);
	if(abEvent == NULL)
	{
		g_Log.AddMessageFormat("Ability ID [%d] does not have an event [%d]");
		return ABILITY_BAD_EVENT;
	}

	if(eventType == EventType::onRequest || eventType == EventType::onDeactivate)
	{
		abProcessing.mIsRequestGTAE = (it->second.mTargetType == TargetType::GTAE);

		cInst->OverrideCurrentAbility(abilityID);
		EnumerateTargets(cInst, it->second.mTargetType, it->second.mTargetFilter, it->second.mTargetTypeRange);

		if(g_Config.CustomAbilityMechanics == true)
		{
			if(cInst->HasStatus(StatusEffects::DISARM) == true)
			{
				if(it->second.IsPhysicalDamageOnly() == true)
					return ABILITY_DISARM;
			}
		}
		if(cInst->HasStatus(StatusEffects::DEAD) && it->second.CanUseWhileDead() == false)
			return ABILITY_DEAD;
	}

	ActiveAbilityInfo *ab = abInfo;
	if(ab == NULL)
		ab = &cInst->ab[0];
	if(ab->TargetCount == 0)
		return ABILITY_NO_TARGET;

	FUNCTION_ITERATOR fit;

	abProcessing.ciSource = cInst;
	abProcessing.ciSourceAb = ab;
	abProcessing.mAbilityEntry = &it->second;

	if(eventType == EventType::onRequest)
	{
		if(ab->bPending == true && ab->abilityID == abilityID)
			return ABILITY_PENDING;

		int cooldownCategory = ResolveCooldownCategoryID(it->second.GetRowAsCString(ABROW::COOLDOWN_CATEGORY));
		if(cInst->HasCooldown(cooldownCategory) == true)
			return ABILITY_COOLDOWN;
	}

	/*  DEBUG ONLY */
//	g_Log.AddMessageFormat("%s %s (%d) %s", cInst->css.display_name, it->second.GetRowAsCString(ABROW::NAME), abilityID, EventType::GetNameByEventID(eventType));
//	for(int i = 0; i < ab->TargetCount; i++)
//		g_Log.AddMessageFormat("  %d: %s", i, ab->TargetList[i]->css.display_name);
	/**/

	for(int targIndex = 0; targIndex < ab->TargetCount; targIndex++)
	{
		abProcessing.ResetState();
		abProcessing.ciTarget = ab->TargetList[targIndex];
		if(abProcessing.ciTarget == NULL)
		{
			g_Log.AddMessageFormat("[CRITICAL] Target index [%d] is NULL", targIndex);
			return 0;
		}
		for(size_t f = 0; f < abEvent->mFunctionList.size(); f++)
		{
			fit = mFunctionMap.find(abEvent->mFunctionList[f].mFunctionName);
			if(fit == mFunctionMap.end())
			{
				g_Log.AddMessageFormat("[ERROR] Ability ID [%d] event [%d] references an unrecognized function [%s]", abilityID, eventType, abEvent->mFunctionList[f].mFunctionName.c_str());
				//return ABILITY_NOT_FOUND;
			}
			else
			{
//				g_Log.AddMessageFormat("Attempting call [%s]", abEvent->mFunctionList[f].mFunctionName.c_str());

				bool pass = abProcessing.CheckActivationChance(abEvent->mFunctionList[f].mChance, abEvent->mFunctionList[f].mChanceID);


				// This here calls the actual ability functions via a member function pointer.
				int result = ABILITY_SUCCESS;
				if(pass == true)
					result = (this->abProcessing.*fit->second)(abEvent->mFunctionList[f]);

				//If we're requesting an ability activation, abort at the first failure.
				//If processing actions, ignore and keep going.
				if(result != ABILITY_SUCCESS)
				{
					//if(eventType == EventType::onRequest)
					return result;
				}
			}
		}
		
		//Send damage before finishing or moving to another target.
		if(abProcessing.mTotalDamage > 0)
			abProcessing.SendDamageString(it->second.GetRowAsCString(1));
	}

	if(eventType == EventType::onRequest)
	{
		if(it->second.mWarmupTime > 0)
			cInst->RegisterCast(it->second.mAbilityID, it->second.mWarmupTime);
		else
			cInst->RegisterInstant(it->second.mAbilityID);

	}
	else if(eventType == EventType::onActivate)
	{
		abProcessing.ciSourceAb->bResourcesSpent = false;

		// TODO - unused - check if it is!
		// const char *debugStr = it->second.GetRowAsCString(ABROW::COOLDOWN_CATEGORY);

		int cooldownCategory = ResolveCooldownCategoryID(it->second.GetRowAsCString(ABROW::COOLDOWN_CATEGORY));
		cInst->RegisterCooldown(cooldownCategory, it->second.mCooldownTime);

		//TODO: Process any reagents that may have been used.
		abProcessing.ConsumeReagent();

		//If there's an implicit action attached to this skill, register it with the creature.
		if(it->second.mAbilityFlags & AbilityFlags::ImplicitParry)
			cInst->RegisterImplicit(EventType::onParry, abilityID, it->second.mAbilityGroupID);

		if(it->second.mAbilityFlags & AbilityFlags::Channel)
		{
			bool secSlot = false;
			if(it->second.mAbilityFlags & AbilityFlags::SecondaryChannel)
			{
				//If the iteration uses a different target selection (sometimes used for alternately
				//calculated effects on abilities, like area-of-effect splash on abilities) then
				//we need to repopulate the target list.
				AbilityEvent2 *iterateEvent = it->second.GetEvent(EventType::onIterate);
				if(abEvent->HasDifferentTargetType(iterateEvent) == true)
					EnumerateTargets(cInst, iterateEvent->mTargetType, it->second.mTargetFilter, iterateEvent->mTargetRange);

				secSlot = true;
			}
			if(it->second.mAbilityFlags & AbilityFlags::UnbreakableChannel)
			{
				ab->bUnbreakableChannel = true;
			}
			int durationMS = abProcessing.GetAdjustedChannelDuration(it->second.mChannelDuration);
			int interruptResist = abProcessing.GetAdjustedInterruptChance();
			//if(it->second.mAbilityFlags & AbilityFlags::SecondaryTarget)
			//	ab->bSecondary = true;
			cInst->RegisterChannel(it->second.mAbilityID, durationMS, it->second.mChannelIteration, secSlot, interruptResist);
		}
	}

	if(eventType != EventType::onRequest)
	{
		abProcessing.ModifyOrbs();
		abProcessing.ResetOrbs();
	}
	return 0;
}

int AbilityManager2 :: ActivateImplicit(CreatureInstance *cInst, CreatureInstance *target, short abilityID, int eventType)
{
	ABILITY_ITERATOR it;
	it = mAbilityIndex.find(abilityID);
	if(it == mAbilityIndex.end())
	{
		g_Log.AddMessageFormat("[ERROR] Ability not found in table: %d", abilityID);
		return ABILITY_NOT_FOUND;
	}
	AbilityEvent2 *abEvent = it->second.GetEvent(eventType);
	if(abEvent == NULL)
	{
		g_Log.AddMessageFormat("[ERROR] Ability ID [%d] does not have an event [%d]");
		return ABILITY_BAD_EVENT;
	}

	FUNCTION_ITERATOR fit;
	abProcessing.ciSource = cInst;
	abProcessing.ciSourceAb = &cInst->ab[0];
	abProcessing.mAbilityEntry = &it->second;

	//Unlike normal abilities, we don't acquire or traverse a target list, we just operate
	//on the information that's been given.
	abProcessing.ResetState();
	abProcessing.ciTarget = target;
	if(abProcessing.ciTarget == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] Implicit action target is NULL");
		return 0;
	}
	for(size_t f = 0; f < abEvent->mFunctionList.size(); f++)
	{
		fit = mFunctionMap.find(abEvent->mFunctionList[f].mFunctionName);
		if(fit == mFunctionMap.end())
		{
			g_Log.AddMessageFormat("Ability ID [%d] event [%d] references an invalid function [%s]", abilityID, eventType, abEvent->mFunctionList[f].mFunctionName.c_str());
		}
		else
		{
			bool pass = abProcessing.CheckActivationChance(abEvent->mFunctionList[f].mChance, abEvent->mFunctionList[f].mChanceID);

			int result = ABILITY_SUCCESS;
			if(pass == true)
				result = (this->abProcessing.*fit->second)(abEvent->mFunctionList[f]);

			//If we're requesting an ability activation, abort at the first failure.
			//If processing actions, ignore and keep going.
			if(result != ABILITY_SUCCESS)
			{
				//if(eventType == EventType::onRequest)
				return result;
			}
		}
	}
	
	//Send damage before finishing or moving to another target.
	if(abProcessing.mTotalDamage > 0)
		abProcessing.SendDamageString(it->second.GetRowAsCString(1));
	
	abProcessing.ModifyOrbs();
	abProcessing.ResetOrbs();
	return 0;
}

//These functions are basically set up to emulate special case internal ability overrides or hacks
//for custom abilities that don't appear in the ability table.  This code is basically equivalent
//to old style activation method where everything was hardcoded.
int AbilityManager2 :: CheckActivateSpecialAbility(CreatureInstance *cInst, short abilityID, int eventType)
{
	abProcessing.ciSource = cInst;
	switch(abilityID)
	{
	case ABILITYID_QUEST_INTERACT_OBJECT:
		if(eventType == EventType::onActivate)
			cInst->RunQuestObjectInteraction(cInst->ab[0].TargetList[0], false);
		break;
	case ABILITYID_QUEST_GATHER_OBJECT:
		if(eventType == EventType::onActivate)
			cInst->RunQuestObjectInteraction(cInst->ab[0].TargetList[0], true);
		break;
	case ABILITYID_INTERACT_OBJECT:
		if(eventType == EventType::onActivate)
			cInst->RunObjectInteraction(cInst->ab[0].TargetList[0]);
		break;
	case 32759:		//stop_melee : 1
		if(eventType == EventType::onRequest)
			cInst->SetAutoAttack(NULL, -1);
		break;
	case 32760:		//ranged_melee : 1
		if(eventType == EventType::onRequest)
		{
			cInst->SetAutoAttack(cInst->CurrentTarget.targ, StatusEffects::AUTO_ATTACK_RANGED);
			//if(cInst->RegisterTarget(32760, TargetType::ST, 0, TargetStatus::Enemy) == 0) return ABILITY_NO_TARGET;
			//if(cInst->InRange(350.0) == false) return ((350 << 8) | AB_RANGE);
			//if(cInst->Facing() == false) return AB_FACING;
			//cInst->RegisterInstant(32760);
		}
		else if(eventType == EventType::onActivate)
		{
			//Disallow autoattacks if casting an ability
			if(cInst->ab[0].bPending == true)
				if(cInst->ab[0].type != AbilityType::Instant)
					return ABILITY_PENDING;

			if(cInst->CurrentTarget.targ == NULL)
			{
				cInst->SetAutoAttack(NULL, -1);
				return ABILITY_NO_TARGET;
			}
			if(cInst->_ValidTargetFlag(cInst->CurrentTarget.targ, TargetStatus::Enemy_Alive) == false)
			{
				cInst->SetAutoAttack(NULL, -1);
				return ABILITY_NO_TARGET;
			}
			if(cInst->InRange_Target(350.0) == false) return CreateAbilityError(ABILITY_INRANGE, 350);
			if(cInst->isTargetInCone(cInst->CurrentTarget.targ, g_FacingTarget) == false) return ABILITY_FACING;

			abProcessing.ciTarget = cInst->CurrentTarget.targ;
			abProcessing.ResetState();
			abProcessing._AttackRanged(0);
			abProcessing.SendDamageString("ranged_melee");
			cInst->RegisterCooldown(ResolveCooldownCategoryID("autoMelee"), 0);
			cInst->SendAutoAttack(32760, cInst->CurrentTarget.targ->CreatureID);
		}
		break;
	case 32766:		//melee : 1
		if(eventType == EventType::onRequest)
		{
			cInst->SetAutoAttack(cInst->CurrentTarget.targ, StatusEffects::AUTO_ATTACK);

			//if(cInst->HasCooldown(CooldownCategory::autoMelee) == true) return AB_COOLDOWN;
			//if(EnumerateTargets(cInst, TargetType::ST, TargetFilter::Enemy, 0) == 0) return ABILITY_NO_TARGET;
			//if(cInst->RegisterTarget(32766, TargetType::ST, 0, TargetStatus::Enemy) == 0) return ABILITY_NO_TARGET;
			//if(cInst->InRange(30.0) == false) return ((30 << 8) | AB_RANGE);
			//if(cInst->Facing() == false) return AB_FACING;
			//cInst->RegisterInstant(32766);
		}
		else if(eventType == EventType::onActivate)
		{
			//Disallow autoattacks if casting an ability
			if(cInst->ab[0].bPending == true)
				if(cInst->ab[0].type != AbilityType::Instant)
					return ABILITY_PENDING;

			if(cInst->CurrentTarget.targ == NULL)
			{
				cInst->SetAutoAttack(NULL, -1);
				return ABILITY_NO_TARGET;
			}
			if(cInst->_ValidTargetFlag(cInst->CurrentTarget.targ, TargetStatus::Enemy_Alive) == false)
			{
				cInst->SetAutoAttack(NULL, -1);
				return ABILITY_NO_TARGET;
			}
			if(cInst->InRange_Target(30.0) == false) return CreateAbilityError(ABILITY_INRANGE, 30);
			if(cInst->isTargetInCone(cInst->CurrentTarget.targ, g_FacingTarget) == false) return ABILITY_FACING;

			abProcessing.ciTarget = cInst->CurrentTarget.targ;
			abProcessing.ResetState();
			abProcessing._AttackMelee(0);
			abProcessing.SendDamageString("melee");
			cInst->RegisterCooldown(ResolveCooldownCategoryID("autoMelee"), 0);
			cInst->SendAutoAttack(32766, cInst->CurrentTarget.targ->CreatureID);
		}
		break;
	default:
		return ABILITY_NOT_SPECIAL;
	}
	return ABILITY_SUCCESS;
}

//Check for all valid variable names.  Should have a matching conditional return value in ResolveSymbol()
bool AbilityManager2 :: CheckValidVariableName(const std::string &token)
{
	const char *validTokens[] = {
		"A_STRENGTH", "A_DEXTERITY", "A_SPIRIT", "A_PSYCHE", "A_CONSTITUTION",
		"A_LEVEL",
		"mightCharges", "willCharges", "MWD", "RWD", "WMD",
		"MAX_HEALTH", "damage", "T_mightCharges", "T_willCharges",
		"mightChargesCurrent", "willChargesCurrent"
	};
	const int numValidTokens = sizeof(validTokens) / sizeof(validTokens[0]);

	for(int i = 0; i < numValidTokens; i++)
	{
		if(token.compare(validTokens[i]) == 0)
			return true;
	}
	return false;
}

double AbilityManager2 :: ResolveSymbol(const std::string &symbol)
{
	//Note: for mightCharges and willCharges, we want to get the amount of charges
	//registered to the source's own ability data.  We don't want to amounts in
	//abProcessing because that state information is not preserved between onRequest
	//and onActivate, which means that the amounts will be reset to zero by the time
	//onActivate or onIterate try to access them.
	if(symbol.compare("A_STRENGTH") == 0)
		return abProcessing.ciSource->css.strength;
	else if(symbol.compare("A_DEXTERITY") == 0)
		return abProcessing.ciSource->css.dexterity;
	else if(symbol.compare("A_SPIRIT") == 0)
		return abProcessing.ciSource->css.spirit;
	else if(symbol.compare("A_PSYCHE") == 0)
		return abProcessing.ciSource->css.psyche;
	else if(symbol.compare("A_CONSTITUTION") == 0)
		return abProcessing.ciSource->css.constitution;
	else if(symbol.compare("A_LEVEL") == 0)
		return abProcessing.ciSource->css.level;
	else if(symbol.compare("mightCharges") == 0)
		return abProcessing.ciSourceAb->mightChargesSpent;
	else if(symbol.compare("willCharges") == 0)
		return abProcessing.ciSourceAb->willChargesSpent;
	else if(symbol.compare("MWD") == 0)
		return abProcessing.ciSource->MWD();
	else if(symbol.compare("RWD") == 0)
		return abProcessing.ciSource->RWD();
	else if(symbol.compare("WMD") == 0)
		return abProcessing.ciSource->WMD();
	else if(symbol.compare("MAX_HEALTH") == 0)
		return abProcessing.ciSource->GetMaxHealth(true);
	else if(symbol.compare("damage") == 0)
		return abProcessing.GetImplicitDamage();
	else if(symbol.compare("T_mightCharges") == 0)    //Expanded attribute, not used in the official gameplay.
		return abProcessing.ciTarget->css.might_charges;
	else if(symbol.compare("T_willCharges") == 0)     //Expanded attribute, not used in the official gameplay.
		return abProcessing.ciTarget->css.will_charges;
	else if(symbol.compare("mightChargesCurrent") == 0)    //Expanded attribute, not used in the official gameplay.
		return abProcessing.ciSource->css.might_charges;
	else if(symbol.compare("willChargesCurrent") == 0)     //Expanded attribute, not used in the official gameplay.
		return abProcessing.ciSource->css.will_charges;
	else
		g_Log.AddMessageFormat("[WARNING] UNRESOLVED FUNCTION SYMBOL [%s]", symbol.c_str());
	return 0.0;
}

int AbilityManager2 :: ResolveBuffCategoryID(const char *buffName)
{
	CONSTANT_MAP::iterator it;
	it = mBuffCategories.find(buffName);
	if(it == mBuffCategories.end())
	{
		g_Log.AddMessageFormat("[WARNING] Could not buff category name [%s]", buffName);
		return 0;
	}
	return it->second;
}

const char* AbilityManager2 :: ResolveBuffCategoryName(int buffCategoryID)
{
	CONSTANT_MAP::iterator it;
	for(it = mBuffCategories.begin(); it != mBuffCategories.end(); ++it)
	{
		if(it->second == buffCategoryID)
			return it->first.c_str();
	}
	return NULL;
}

int AbilityManager2 :: ResolveCooldownCategoryID(const char *cooldownName)
{
	CONSTANT_MAP::iterator it;
	it = mCooldownCategories.find(cooldownName);
	if(it == mCooldownCategories.end())
	{
		g_Log.AddMessageFormat("[WARNING] Could not resolve cooldown category name [%s]", cooldownName);
		return 0;
	}
	return it->second;
}

const char* AbilityManager2 :: ResolveCooldownCategoryName(int cooldownCategoryID)
{
	CONSTANT_MAP::iterator it;
	for(it = mCooldownCategories.begin(); it != mCooldownCategories.end(); ++it)
	{
		if(it->second == cooldownCategoryID)
			return it->first.c_str();
	}
	return NULL;
}

int AbilityManager2 :: ResolveStatID(const char *statName)
{
	int index = GetStatIndexByName(statName);
	if(index >= 0)
		return StatList[index].ID;

	g_Log.AddMessageFormat("ResolveStatID failed to resolve [%s]", statName);
	return -1;
}
int AbilityManager2 :: ResolveStatusEffectID(const char *statusEffectName)
{
	int ID = GetStatusIDByName(statusEffectName);
	if(ID >= 0)
		return ID;

	g_Log.AddMessageFormat("ResolveStatusEffectID failed to resolve [%s]", statusEffectName);
	return -1;
}

int AbilityManager2 :: ResolveItemID(const char *itemID)
{
	int ID = atoi(itemID);
	ItemDef *res = g_ItemManager.GetPointerByID(ID);
	if(res != NULL)
		return res->mID;
	return -1;
}


// Extract the error code from a CreateAbilityError() return value. 
int AbilityManager2 :: GetAbilityErrorCode(int value)
{
	return (value & ABILITY_CODE_BITMASK);
}

// Extract the parameter from a CreateAbilityError() return value.
int AbilityManager2 :: GetAbilityErrorParameter(int value)
{
	return (value >> ABILITY_CODE_BITCOUNT);
}

// Generate an ability error code.
int AbilityManager2 :: CreateAbilityError(int errorCode, int parameter)
{
	return (parameter << ABILITY_CODE_BITCOUNT) | (errorCode & ABILITY_CODE_BITMASK);
}

//Return true if a player always has permission to activate an ability, even if they are not purchased.
bool AbilityManager2 :: IsGlobalIntrinsicAbility(int abilityID)
{
	switch(abilityID)
	{
	case 10000:  //Revive
	case 10001:  //Resurrect
	case 10002:  //Rebirth
	case 10006:  //Unstick
	case 32759:  //stop autoattack
	case 32760:  //ranged attack
	case 32766:  //melee autoattack
	case 32767:  //jump
		return true;
	}
	return false;
}

void AbilityManager2 :: RunImplicitActions(void)
{
	abProcessing.RunImplicitActions();
}

const AbilityEntry2* AbilityManager2 :: GetAbilityPtrByID(int abilityID)
{
	ABILITY_ITERATOR it;
	it = mAbilityIndex.find(abilityID);
	if(it != mAbilityIndex.end())
		return &it->second;
	return NULL;
}

//An external debug command uses this to retrieve a list of cooldown category names
//so that all cooldowns may be reset in the client.
void AbilityManager2 :: GetCooldownCategoryStrings(STRINGLIST &output)
{
	output.clear();
	CONSTANT_MAP::iterator it;
	for(it = mCooldownCategories.begin(); it != mCooldownCategories.end(); ++it)
		output.push_back(it->first);
}

//Return the ability name.  Intended for use by any required debug reporting.
const char* AbilityManager2 :: GetAbilityNameByID(int abilityID)
{
	ABILITY_ITERATOR it;
	it = mAbilityIndex.find(abilityID);
	if(it != mAbilityIndex.end())
		return it->second.GetRowAsCString(ABROW::NAME);
	return "<invalid ability>";
}





AbilityCalculator :: AbilityCalculator()
{
	Clear();
	ResetState();
}

AbilityCalculator :: ~AbilityCalculator()
{
}

void AbilityCalculator :: Clear(void)
{
	ciSource = NULL;
	ciTarget = NULL;
	mAbilityEntry = NULL;
	mIsRequestGTAE = false;
	
	mReagentItemID = 0;     //The processing functions will handle clearing during run-time.
	mReagentItemCount = 0;  //The processing functions will handle clearing during run-time.

	mImplicitInProgress = false;
	mImplicitMeleeDamage = 0;

	mChannelExtendedDurationSec = 0;
	mChannelInterruptChanceMod = 0;

	memset(mDamageStringBuf, 0, sizeof(mDamageStringBuf));
	ResetState();
	ResetOrbs();
}

void AbilityCalculator :: ResetState(void)
{
	//Clear all damage counters and combat states.  Necessary when processing a new ability or target.
	mTotalDamage = 0;
	mTotalDamageMelee = 0;
	mTotalDamageFire = 0;
	mTotalDamageFrost = 0;
	mTotalDamageMystic = 0;
	mTotalDamageDeath = 0;
	mAbsorbedDamage = 0;

	mIsBlocked = false;
	mIsParried = false;
	mIsDodged = false;
	mIsLightHit = false;
	mIsMissed = false;
	mCriticalHitState = 0;

	mChanceRoll = 0;
	mChanceRollID = 0;

	mDamageStringBuf[0] = 0;
	mDamageStringPos = 0;
	//mImplicitInProgress = false;  //NOTE: This state will be cleared directly when implicit processing is finished.
}

void AbilityCalculator :: ResetOrbs(void)
{
	mMightAdjust = 0;
	mMightChargeAdjust = 0;
	mWillAdjust = 0;
	mWillChargeAdjust = 0;
}

int AbilityCalculator :: Facing(ARGUMENT_LIST args)
{
	if(ciSource->Facing(mIsRequestGTAE) == true)
		return ABILITY_SUCCESS;
	return ABILITY_FACING;
}

int AbilityCalculator :: InRange(ARGUMENT_LIST args)
{
	float dist = args.GetFloat(0);
	if(ciSource->InRange(dist, mIsRequestGTAE) == true)
		return ABILITY_SUCCESS;
	return AbilityManager2::CreateAbilityError(ABILITY_INRANGE, static_cast<int>(dist));
}

int AbilityCalculator :: NotSilenced(ARGUMENT_LIST args)
{
	if(ciSource->NotSilenced() == true)
		return ABILITY_SUCCESS;
	return ABILITY_NOTSILENCED;
}

int AbilityCalculator :: HasStatus(ARGUMENT_LIST args)
{
	int EffectID = g_AbilityManager.ResolveStatusEffectID(args.GetString(0));
	if(EffectID == -1)
		return ABILITY_GENERIC;
	if(ciSource->HasStatus(EffectID) == true)
		return ABILITY_SUCCESS;
	
	return ABILITY_STATUS;
}

int AbilityCalculator :: NotStatus(ARGUMENT_LIST args)
{
	int EffectID = g_AbilityManager.ResolveStatusEffectID(args.GetString(0));
	if(EffectID == -1)
		return ABILITY_GENERIC;
	if(ciSource->NotStatus(EffectID) == true)
		return ABILITY_SUCCESS;
	
	return ABILITY_STATUS;
}

//Action.  Interrupt the target's cast if it's casting a warmup or channel.
int AbilityCalculator :: Interrupt(ARGUMENT_LIST args)
{
	ciTarget->Interrupt();
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: Might(ARGUMENT_LIST args)
{
	int amount = args.GetInteger(0);
	if(ciSource->Might(amount) == true)
	{
		mMightAdjust = -amount;
		ciSourceAb->mightAdjust = -amount;
		return ABILITY_SUCCESS;
	}
	return ABILITY_MIGHT;
}

int AbilityCalculator :: Will(ARGUMENT_LIST args)
{
	int amount = args.GetInteger(0);
	if(ciSource->Will(amount) == true)
	{
		mWillAdjust = -amount;
		ciSourceAb->willAdjust = -amount;
		return ABILITY_SUCCESS;
	}
	return ABILITY_WILL;
}

int AbilityCalculator :: MightCharge(ARGUMENT_LIST args)
{
	int min = args.GetInteger(0);
	int max = args.GetInteger(1);
	int used = ciSource->MightCharge(min, max);
	if(used > 0)
	{
		mMightChargeAdjust = -used;
		ciSourceAb->mightChargesSpent = used;
		ciSourceAb->mightChargeAdjust = -used;
		return ABILITY_SUCCESS;
	}
	return ABILITY_MIGHT_CHARGE;
}

int AbilityCalculator :: WillCharge(ARGUMENT_LIST args)
{
	int min = args.GetInteger(0);
	int max = args.GetInteger(1);
	int used = ciSource->WillCharge(min, max);
	if(used > 0)
	{
		mWillChargeAdjust = -used;
		ciSourceAb->willChargesSpent = used;
		ciSourceAb->willChargeAdjust = -used;
		return ABILITY_SUCCESS;
	}
	return ABILITY_WILL_CHARGE;
}

int AbilityCalculator :: A_AddMightCharge(ARGUMENT_LIST args)
{
	mMightChargeAdjust = args.GetInteger(0);
	ciSourceAb->mightChargeAdjust = args.GetInteger(0);
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: A_AddWillCharge(ARGUMENT_LIST args)
{
	mWillChargeAdjust = args.GetInteger(0);
	ciSourceAb->willChargeAdjust = args.GetInteger(0);
	return ABILITY_SUCCESS;
}

//Action.  Adjust the target's might charges (positive or negative change).
int AbilityCalculator :: T_AddMightCharge(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	ciTarget->AddMightCharge(amount);
	return ABILITY_SUCCESS;
}

//Action.  Adjust the target's will charges (positive or negative change).
int AbilityCalculator :: T_AddWillCharge(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	ciTarget->AddWillCharge(amount);
	return ABILITY_SUCCESS;
}

//Action.  Unique feature of this server.  Remove charges from the target and grant to the caster.
int AbilityCalculator :: StealWillCharge(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	amount = Util::ClipInt(ciTarget->css.will_charges, 0, amount);
	ciSource->AddWillCharge(amount);
	ciTarget->AddWillCharge(-amount);
	return ABILITY_SUCCESS;
}

//Action.  Unique feature of this server.  Remove charges from the target and grant to the caster.
int AbilityCalculator :: StealMightCharge(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	amount = Util::ClipInt(ciTarget->css.might_charges, 0, amount);
	ciSource->AddMightCharge(amount);
	ciTarget->AddMightCharge(-amount);
	return ABILITY_SUCCESS;
}

//Action.  Refill the targets's might bar (usually operated on Self).
int AbilityCalculator :: FullMight(ARGUMENT_LIST args)
{
	mMightAdjust = 0;
	ciSourceAb->mightAdjust = 0;  //Override the amount spent by the ability.
	ciTarget->FullMight();
	return ABILITY_SUCCESS;
}

//Action.  Refill the targets's will bar.
//NOTE: This function does not appear in the official table, but is added here for completion.
int AbilityCalculator :: FullWill(ARGUMENT_LIST args)
{
	mWillAdjust = 0;
	ciSourceAb->willAdjust = 0;  //Override the amount spent by the ability.
	ciTarget->FullWill();
	return ABILITY_SUCCESS;
}

//Condition. Check if the caster currently has a weapon equipped in the main hand.
int AbilityCalculator :: hasMainHandWeapon(ARGUMENT_LIST args)
{
	if(ciSource->hasMainHandWeapon() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASMAINHAND;
}

int AbilityCalculator :: hasOffHandWeapon(ARGUMENT_LIST args)
{
	if(ciSource->hasOffHandWeapon() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASOFFHAND;
}

int AbilityCalculator :: hasMeleeWeapon(ARGUMENT_LIST args)
{
	if(ciSource->hasMeleeWeapon() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASMELEE;
}

int AbilityCalculator :: hasRangedWeapon(ARGUMENT_LIST args)
{
	if(ciSource->hasRangedWeapon() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASMELEE;
}

int AbilityCalculator :: hasShield(ARGUMENT_LIST args)
{
	if(ciSource->hasShield() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASSHIELD;
}

int AbilityCalculator :: hasBow(ARGUMENT_LIST args)
{
	if(ciSource->hasBow() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASBOW;
}

int AbilityCalculator :: has2HorPoleWeapon(ARGUMENT_LIST args)
{
	if(ciSource->has2HorPoleWeapon() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HAS2HORPOLE;
}

//Condition. Check if the caster currently has a wand equipped.
int AbilityCalculator :: hasWand(ARGUMENT_LIST args)
{
	if(ciSource->hasWand() == true)
		return ABILITY_SUCCESS;
	return ABILITY_HASWAND;
}

int AbilityCalculator :: Status(ARGUMENT_LIST args)
{
	int StatusID = g_AbilityManager.ResolveStatusEffectID(args.GetString(0));
	if(StatusID == -1)
		return ABILITY_GENERIC;
	float durationSec = args.GetEvaluation(1, &g_AbilityManager);
	ciTarget->Status(StatusID, durationSec);

	return ABILITY_SUCCESS;
}

int AbilityCalculator :: Amp(ARGUMENT_LIST args)
{
	int StatID = g_AbilityManager.ResolveStatID(args.GetString(0));
	if(StatID == -1)
		return ABILITY_GENERIC;

	float percent = args.GetFloat(1);
	int timeSec = static_cast<int>(args.GetEvaluation(2, &g_AbilityManager));
	
	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));

	ciTarget->Amp(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, StatID, percent, timeSec);
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: Add(ARGUMENT_LIST args)
{
	int StatID = g_AbilityManager.ResolveStatID(args.GetString(0));
	if(StatID == -1)
		return ABILITY_GENERIC;

	float amount = args.GetEvaluation(1, &g_AbilityManager);
	int timeSec = static_cast<int>(args.GetEvaluation(2, &g_AbilityManager));

	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));

	ciTarget->Add(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, StatID, amount, amount, timeSec);
	return NO_RETURN_VALUE;
}

int AbilityCalculator :: AmpCore(ARGUMENT_LIST args)
{
	float percent = args.GetFloat(0);
	int timeSec = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));

	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));
	ciTarget->AmpCore(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, percent, timeSec);
	return NO_RETURN_VALUE;
}

//Action.  Set target's stat to zero by adding a debuff to nullify the current amount of a stat.
int AbilityCalculator :: Nullify(ARGUMENT_LIST args)
{
	int StatID = g_AbilityManager.ResolveStatID(args.GetString(0));
	if(StatID == -1)
		return ABILITY_GENERIC;
	int durationSec = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));

	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));

	ciTarget->Nullify(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, StatID, durationSec);
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: CheckBuffLimits(ARGUMENT_LIST args)
{
	//int level = args.GetInteger(0);
	//int buffID = ResolveBuffCategoryID(args.GetString(1));

	return ABILITY_SUCCESS;
}

//Resolve a buff category name into its integer representation, unique from all other buff names.
int AbilityCalculator :: ResolveBuffCategoryID(const char *buffName)
{
	return g_AbilityManager.ResolveBuffCategoryID(buffName);
}

int AbilityCalculator :: MeleeDamage(ARGUMENT_LIST args)
{
	//Evaluate the amount in the damage formula.  The attacker performed an action, so cancel
	//invisibility.  Apply damage mitigation factors for physical damage.
	if(g_Config.CustomAbilityMechanics == true)
	{
		if(ciSource->HasStatus(StatusEffects::DISARM))
			return ABILITY_DISARM;
	}

	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	_DoMeleeDamage(amount);
	return NO_RETURN_VALUE;
}

int AbilityCalculator :: SetAutoAttack(ARGUMENT_LIST args)
{
	int StatusID = g_AbilityManager.ResolveStatusEffectID(args.GetString(0));
	if(StatusID == -1)
		return ABILITY_GENERIC;

	ciSource->SetAutoAttack(ciTarget, StatusID);
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: AttackRanged(ARGUMENT_LIST args)
{
	//g_Log.AddMessageFormat("AttackRanged");
	//Disallow autoattacks if casting an ability
	if(ciSource->ab[0].bPending == true)
		if(ciSource->ab[0].type != AbilityType::Instant)
			return ABILITY_PENDING;

	if(ciSource->CurrentTarget.targ == NULL)
	{
		ciSource->SetAutoAttack(NULL, -1);
		return ABILITY_NO_TARGET;
	}
	if(ciSource->_ValidTargetFlag(ciSource->CurrentTarget.targ, TargetStatus::Enemy_Alive) == false)
	{
		ciSource->SetAutoAttack(NULL, -1);
		return ABILITY_NO_TARGET;
	}
	if(ciSource->InRange_Target(350.0) == false) return AbilityManager2::CreateAbilityError(ABILITY_INRANGE, 350);
	if(ciSource->isTargetInCone(ciSource->CurrentTarget.targ, g_FacingTarget) == false) return ABILITY_FACING;

	int amount = ciSource->RWD();
	_DoMeleeDamage(amount);
	_DoElementalAutoDamage();
	return NO_RETURN_VALUE;
}

int AbilityCalculator :: AttackMelee(ARGUMENT_LIST args)
{
	//g_Log.AddMessageFormat("AttackMelee");

	//Disallow autoattacks if casting an ability
	if(ciSource->ab[0].bPending == true)
		if(ciSource->ab[0].type != AbilityType::Instant)
			return ABILITY_PENDING;

	if(ciSource->CurrentTarget.targ == NULL)
	{
		ciSource->SetAutoAttack(NULL, -1);
		return ABILITY_NO_TARGET;
	}
	if(ciSource->_ValidTargetFlag(ciSource->CurrentTarget.targ, TargetStatus::Enemy_Alive) == false)
	{
		ciSource->SetAutoAttack(NULL, -1);
		return ABILITY_NO_TARGET;
	}
	if(ciSource->InRange_Target(30.0) == false) return AbilityManager2::CreateAbilityError(ABILITY_INRANGE, 30);
	if(ciSource->isTargetInCone(ciSource->CurrentTarget.targ, g_FacingTarget) == false) return ABILITY_FACING;

	//int amount = ciSource->MWD() + ciSource->GetOffhandDamage();   OBSOLETE
	int amount = ciSource->MWD();  //This automatically includes offhand now
	_DoMeleeDamage(amount);
	_DoElementalAutoDamage();
	return NO_RETURN_VALUE;
}

int AbilityCalculator :: _AttackRanged(int unused)
{
	//The official ability list only uses this function once for the ranged autoattack.
	//It has one argument: AttackRanged(0)
	//int unused = args.GetInteger(0);
	int amount = ciSource->RWD();
	_DoMeleeDamage(amount);
	_DoElementalAutoDamage();
	return NO_RETURN_VALUE;
}

int AbilityCalculator :: _AttackMelee(int unused)
{
	//The official ability list only uses this function once for the ranged autoattack.
	//It has one argument: AttackRanged(0)
	//int unused = args.GetInteger(0);
	//int amount = ciSource->MWD() + ciSource->GetOffhandDamage();  OBSOLETE
	int amount = ciSource->MWD();  //Factors offhand damage.
	_DoMeleeDamage(amount);
	_DoElementalAutoDamage();
	return NO_RETURN_VALUE;
}

//A helper function for autoattacks to apply elemental damage buffs to outgoing attack damage.
void AbilityCalculator :: _DoElementalAutoDamage(void)
{
	if(ciSource->css.base_damage_fire != 0) _DoFireDamage(ciSource->css.base_damage_fire);
	if(ciSource->css.base_damage_frost != 0) _DoFrostDamage(ciSource->css.base_damage_frost);
	if(ciSource->css.base_damage_mystic != 0) _DoMysticDamage(ciSource->css.base_damage_mystic);
	if(ciSource->css.base_damage_death != 0) _DoDeathDamage(ciSource->css.base_damage_death);
}

void AbilityCalculator :: _DoMeleeDamage(int amount)
{
	ciSource->CancelInvisibility();
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.dmg_mod_melee);
	ApplyHitChances(amount, DamageType::MELEE);
	amount = ciTarget->GetAdjustedDamage(ciSource, DamageType::MELEE, amount);
	ApplyBlockChance(amount);
	ApplyParryChance(amount);
	ApplyDodgeChance(amount);
	amount = ciTarget->ApplyPostDamageModifiers(ciSource, amount, mAbsorbedDamage);
	ciTarget->OnApplyDamage(ciSource, amount);
	mTotalDamageMelee += amount;
	mTotalDamage += amount;
	//ciTarget->RegisterHostility(ciSource, 1);
}

int AbilityCalculator :: FireDamage(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	_DoFireDamage(amount);
	return NO_RETURN_VALUE;
}

//Helper function to apply fire damage to a target.
void AbilityCalculator :: _DoFireDamage(int amount)
{
	ciSource->CancelInvisibility();
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.dmg_mod_fire);
	ApplyHitChances(amount, DamageType::FIRE);
	amount = ciTarget->GetAdjustedDamage(ciSource, DamageType::FIRE, amount);
	amount = ciTarget->ApplyPostDamageModifiers(ciSource, amount, mAbsorbedDamage);
	ciTarget->OnApplyDamage(ciSource, amount);
	mTotalDamageFire += amount;
	mTotalDamage += amount;
	//ciTarget->RegisterHostility(ciSource, 1);
}

int AbilityCalculator :: FrostDamage(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	_DoFrostDamage(amount);
	return NO_RETURN_VALUE;
}

void AbilityCalculator :: _DoFrostDamage(int amount)
{
	ciSource->CancelInvisibility();
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.dmg_mod_frost);
	ApplyHitChances(amount, DamageType::FROST);
	amount = ciTarget->GetAdjustedDamage(ciSource, DamageType::FROST, amount);
	amount = ciTarget->ApplyPostDamageModifiers(ciSource, amount, mAbsorbedDamage);
	ciTarget->OnApplyDamage(ciSource, amount);
	mTotalDamageFrost += amount;
	mTotalDamage += amount;
	//ciTarget->RegisterHostility(ciSource, 1);
}

int AbilityCalculator :: MysticDamage(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	_DoMysticDamage(amount);
	return NO_RETURN_VALUE;
}

void AbilityCalculator :: _DoMysticDamage(int amount)
{
	ciSource->CancelInvisibility();
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.dmg_mod_mystic);
	ApplyHitChances(amount, DamageType::MYSTIC);
	amount = ciTarget->GetAdjustedDamage(ciSource, DamageType::MYSTIC, amount);
	amount = ciTarget->ApplyPostDamageModifiers(ciSource, amount, mAbsorbedDamage);
	ciTarget->OnApplyDamage(ciSource, amount);
	mTotalDamageMystic += amount;
	mTotalDamage += amount;
	//ciTarget->RegisterHostility(ciSource, 1);
}

int AbilityCalculator :: DeathDamage(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	_DoDeathDamage(amount);
	return NO_RETURN_VALUE;
}

void AbilityCalculator :: _DoDeathDamage(int amount)
{
	ciSource->CancelInvisibility();
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.dmg_mod_death);
	ApplyHitChances(amount, DamageType::DEATH);
	amount = ciTarget->GetAdjustedDamage(ciSource, DamageType::DEATH, amount);
	amount = ciTarget->ApplyPostDamageModifiers(ciSource, amount, mAbsorbedDamage);
	ciTarget->OnApplyDamage(ciSource, amount);
	mTotalDamageDeath += amount;
	mTotalDamage += amount;
	//ciTarget->RegisterHostility(ciSource, 1);
}

//Heal the target.
int AbilityCalculator :: Heal(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.base_healing);
	ciTarget->Heal(amount);
	return ABILITY_SUCCESS;
}

//Heal the caster.
int AbilityCalculator :: A_Heal(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	amount += Util::GetAdditiveFromIntegralPercent1000(amount, ciSource->css.base_healing);
	ciSource->Heal(amount);
	return ABILITY_SUCCESS;
}

//Action.  Direct damage to the target.
//Note: official abilities that use this function tend to be Self-cast only.
int AbilityCalculator :: Harm(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	ciTarget->Harm(amount);
	return ABILITY_SUCCESS;
}

//Condition. Successful if the caster's health is at least the provided ratio.
int AbilityCalculator :: PercentMaxHealth(ARGUMENT_LIST args)
{
	float percent = args.GetFloat(0);
	if(ciSource->PercentMaxHealth(percent) == true)
		return ABILITY_SUCCESS;
	return ABILITY_HEALTH_TOO_LOW;
}

//Action.  This only seems to be used for some potions in the official ability table.  According to the
//description, this property seems to set an iterative regeneration amount every 6 seconds.
int AbilityCalculator :: Regenerate(ARGUMENT_LIST args)
{
	int amount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	ciTarget->Regenerate(amount);
	return ABILITY_HEALTH_TOO_LOW;
}


int AbilityCalculator :: Taunt(ARGUMENT_LIST args)
{
	int durationSec = args.GetInteger(0);
	ciTarget->Taunt(ciSource, durationSec);
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: AddHate(ARGUMENT_LIST args)
{
	int amount = args.GetInteger(0);
	ciTarget->AddHate(ciSource, amount);
	return ABILITY_SUCCESS;
}

//The purpose of this function is unknown.
//Officially it only appears on the Frost Mire skill.  Nothing is mentioned in the skill description
//about modifying hate.
int AbilityCalculator :: TopHated(ARGUMENT_LIST args)
{
	int amount = args.GetInteger(0);
	ciTarget->TopHated(amount);
	return ABILITY_SUCCESS;
}

//All hate that the caster generated for all creatures should be removved.
int AbilityCalculator :: UnHate(ARGUMENT_LIST args)
{
	ciSource->UnHate();
	return ABILITY_SUCCESS;
}

int AbilityCalculator :: LeaveCombat(ARGUMENT_LIST args)
{
	ciSource->LeaveCombat();
	return ABILITY_SUCCESS;
}

//Condition.  Successful if the caster is behind the target.
int AbilityCalculator :: Behind(ARGUMENT_LIST args)
{
	if(ciSource->Behind() == true)
		return ABILITY_SUCCESS;
	return ABILITY_BEHIND;
}

//Action.  Spin the target 180 degrees.
int AbilityCalculator :: Spin(ARGUMENT_LIST args)
{
	ciTarget->Spin();
	return ABILITY_SUCCESS;
}

//Action.  Adjust the duration of a channel in progress.
//TODO: This probably needs to be changed.
int AbilityCalculator :: Duration(ARGUMENT_LIST args)
{
	int timeSec = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	mChannelExtendedDurationSec = timeSec;
	return ABILITY_SUCCESS;
}

//Action.  Unique to this server.  Provide additional resistance for interrupt for a channel.
int AbilityCalculator :: InterruptChance(ARGUMENT_LIST args)
{
	//This stat internally works as a bonus to CHANNELING_BREAK_CHANCE.  10 = 1%.
	int channelBreakMod = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	mChannelInterruptChanceMod = channelBreakMod;
	return ABILITY_SUCCESS;
}


//Action.  Resurrect the caster.
int AbilityCalculator :: Resurrect(ARGUMENT_LIST args)
{
	float healthRatio = args.GetFloat(0);
	float luckRatio = 1.0;

	if(args.mArguments.size() > 1)   //The second argument is not always used.
		luckRatio = args.GetFloat(1);

	ciTarget->Resurrect(healthRatio, luckRatio, mAbilityEntry->mAbilityID);
	return ABILITY_SUCCESS;
}

//Condition.  Check if the caster is near a sanctuary.
int AbilityCalculator :: NearbySanctuary(ARGUMENT_LIST args)
{
	if(ciSource->NearbySanctuary() == true)
		return ABILITY_SUCCESS;
	return ABILITY_NEARBY_SANCTUARY;
}

//Action.  Bind the caster to the nearest sanctuary.
int AbilityCalculator :: BindTranslocate(ARGUMENT_LIST args)
{
	ciSource->BindTranslocate();
	return ABILITY_SUCCESS;
}

//Action.  Warp the caster to their bound sanctuary.
int AbilityCalculator :: Translocate(ARGUMENT_LIST args)
{
	ciSource->Translocate(false);
	return ABILITY_SUCCESS;
}

//Action.  A henge teleport skill has been sent to the target, ask them for confirmation to accept.
int AbilityCalculator :: PortalRequest(ARGUMENT_LIST args)
{
	const char *internalName = args.GetString(0);
	const char *prettyName = args.GetString(1);
	ciTarget->PortalRequest(ciSource, internalName, prettyName);
	return ABILITY_SUCCESS;
}

//Action.  Teleport the caster to a named location.
//The official use of this function only seems to be unnamed henge targets.
int AbilityCalculator :: Go(ARGUMENT_LIST args)
{
	//Currently do nothing.
	//const char *destName = args.GetString(0);
	return ABILITY_SUCCESS;
}

//Action.  Teleport the target to the nearest sanctuary.  Often called on self, but sometimes party
//members too, like the Exodus skill.
int AbilityCalculator :: GoSanctuary(ARGUMENT_LIST args)
{
	ciTarget->GoSanctuary();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveAltDebuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveAltDebuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveStatDebuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveStatDebuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveEtcDebuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveEtcDebuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveAltBuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveAltBuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveStatBuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveStatBuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveEtcBuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveEtcBuff();
	return ABILITY_SUCCESS;
}

//Action.  Check skill table for usage.
int AbilityCalculator :: RemoveHealthBuff(ARGUMENT_LIST args)
{
	//No arguments.
	ciTarget->RemoveHealthBuff();
	return ABILITY_SUCCESS;
}

//Action.  Enable mage invisibility.
int AbilityCalculator :: Invisible(ARGUMENT_LIST args)
{
	int timeSec = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	int dist = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));
	//int unknown = static_cast<int>(args.GetEvaluation(2, &g_AbilityManager));
	ciTarget->Invisible(timeSec, dist, 0);
	return ABILITY_SUCCESS;
}

//Action.  Enable rogue invisibility.
int AbilityCalculator :: WalkInShadows(ARGUMENT_LIST args)
{
	int timeSec = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	int dist = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));
	ciTarget->WalkInShadows(timeSec, dist);
	return ABILITY_SUCCESS;
}

//Action.  Set a description.  Official use is unknown, but is probably to customize the descriptive
//text displayed when certain channels are running.  Not sure if this has any server use.
//TODO: Implement this?
int AbilityCalculator :: AddWDesc(ARGUMENT_LIST args)
{
	int StatID = g_AbilityManager.ResolveStatID(args.GetString(0));
	if(StatID == -1)
		return ABILITY_GENERIC;

//	int iterationTimeSec = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));
//	int durationTimeSec = static_cast<int>(args.GetEvaluation(2, &g_AbilityManager));
//	const char *descText = args.GetString(3);
	return ABILITY_SUCCESS;
}

//Action.  Transfer health from the caster to the target.
int AbilityCalculator :: HealthSacrifice(ARGUMENT_LIST args)
{
	if(ciTarget == ciSource)
		return ABILITY_SUCCESS;

	int healAmount = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));

	//Clip the amount of granted health so it doesn't exceed maximum.
	//This will save the caster from unnecessary losses in health.
	int neededHealth = ciTarget->GetMaxHealth(true) - ciTarget->css.health;
	neededHealth = Util::ClipInt(neededHealth, 0, healAmount);
	if(neededHealth > 0)
	{
		ciSource->ApplyRawDamage(neededHealth);
		ciTarget->Heal(neededHealth);
	}
	return ABILITY_SUCCESS;
}

//Action.  Does exactly what the name implies.
int AbilityCalculator :: DoNothing(ARGUMENT_LIST args)
{
	return ABILITY_SUCCESS;
}

//Condition.  Check if the player has the necessary reagents.
int AbilityCalculator :: Reagent(ARGUMENT_LIST args)
{
	mReagentItemID = args.GetInteger(0);
	mReagentItemCount = args.GetInteger(1);

	//TODO: Run an inventory check.
	return ABILITY_SUCCESS;
}

//Internal helper function to consume any reagents.
void AbilityCalculator :: ConsumeReagent(void)
{
	if(mReagentItemID != 0)
	{
		//TODO: Remove items from inventory.
		mReagentItemID = 0;
		mReagentItemCount = 0;
	}
}

//Action.  New to this server.  Send a notification for an ability effect.  Visual purposes only.
int AbilityCalculator :: DisplayEffect(ARGUMENT_LIST args)
{
	const char *effect = args.GetString(0);
	g_Log.AddMessageFormat("Display Effect: %s", effect);
	ciSource->SimulateEffect(effect, ciTarget);
	return ABILITY_SUCCESS;
}

//Action.  Condition.  True when no transform is in place.
int AbilityCalculator :: NotTransformed(ARGUMENT_LIST args)
{
	if(ciSource->transformModifier == NULL)
		return ABILITY_SUCCESS;
	return ABILITY_GENERIC;
}

//Action.  New to this server.  Removes all clothes (but keeps stats)
int AbilityCalculator :: Nudify(ARGUMENT_LIST args)
{
	g_Log.AddMessageFormat("Nudify");
	int timeSec = static_cast<int>(args.GetEvaluation(0, &g_AbilityManager));
	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));
	ActiveBuff * buff = ciTarget->AddMod(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, timeSec);
	ciSource->CAF_Nudify(buff->durationS);
	return ABILITY_SUCCESS;
}

//Action.  New to this server.  Transform into another creature
int AbilityCalculator :: Transform(ARGUMENT_LIST args)
{
	int creatureDefID = args.GetInteger(0);
	g_Log.AddMessageFormat("Transform: %d", creatureDefID);
	int timeSec = static_cast<int>(args.GetEvaluation(1, &g_AbilityManager));
	int buffType = ResolveBuffCategoryID(mAbilityEntry->GetRowAsCString(ABROW::BUFF_CATEGORY));
	ActiveBuff * buff = ciTarget->AddMod(mAbilityEntry->mTier, buffType, mAbilityEntry->mAbilityID, mAbilityEntry->mAbilityGroupID, timeSec);
	ciSource->CAF_Transform(creatureDefID, mAbilityEntry->mAbilityID, buff->durationS);
	return ABILITY_SUCCESS;
}

//Action.  New to this server.  Transform into another creature
int AbilityCalculator :: Untransform(ARGUMENT_LIST args)
{
	g_Log.AddMessageFormat("Untransform");
	ciSource->CAF_Untransform();
	return ABILITY_SUCCESS;
}

void AbilityCalculator :: ApplyBlockChance(int &amount)
{
	if(amount == 0)
		return;
	if(ciTarget->IsCombatReactive() == false)
		return;

	//Official formula:
	//float blockRate = (ciTarget->css.base_block + (ciTarget->css.dexterity * BlockDexterityMod) + (ciTarget->css.strength * BlockStrengthMod)) / BlockMod;
	//float blockRate = ((ciTarget->css.dexterity * BlockDexterityMod) + (ciTarget->css.strength * BlockStrengthMod)) / BlockMod;

	//int blockChance = ciTarget->css.base_block + (int)(blockRate * 10.0F);
	int blockChance = ciTarget->css.base_block;
	if(ciTarget->serverFlags & ServerFlags::IsPlayer)
	{
		blockChance += INNATE_BLOCK_CHANCE;
		blockChance += Combat::GetBlockStatBonus(ciTarget->css.dexterity);
		if(ciTarget->HasStatus(StatusEffects::CAN_USE_BLOCK) == false)
			blockChance = 0;
		if(!(ciTarget->serverFlags & ServerFlags::HasShield))
			blockChance = 0;
	}

	if(blockChance > 0)
	{
		int rnd = randint(1, INTEGRAL_FRACTION_TOTAL);
		if(rnd <= blockChance)
		{
			mIsBlocked = true;
			amount = 0;
		}
	}
}

void AbilityCalculator :: ApplyParryChance(int &amount)
{
	if(amount == 0)
		return;
	if(ciTarget->IsCombatReactive() == false)
		return;

	int parryChance = ciTarget->css.base_parry;
	if(ciTarget->serverFlags & ServerFlags::IsPlayer)
	{
		parryChance += INNATE_PARRY_CHANCE;
		parryChance += Combat::GetParryStatBonus(ciTarget->css.dexterity);
		if(ciTarget->HasStatus(StatusEffects::CAN_USE_PARRY) == false)
			parryChance = 0;
		if(!(ciTarget->serverFlags & ServerFlags::HasMeleeWeapon))
			parryChance = 0;
	}

	if(parryChance > 0)
	{
		int rnd = randint(1, INTEGRAL_FRACTION_TOTAL);
		if(rnd <= parryChance)
		{
			RegisterTargetImplicitActions(EventType::onParry, amount);
			mIsParried = true;
			amount = 0;
		}
	}
}

void AbilityCalculator :: ApplyDodgeChance(int &amount)
{
	if(amount == 0)
		return;
	if(ciTarget->IsCombatReactive() == false)
		return;

	int dodgeChance = ciTarget->css.base_dodge;

	if(ciTarget->serverFlags & ServerFlags::IsPlayer)
		dodgeChance += Combat::GetDodgeStatBonus(ciTarget->css.dexterity);

	if(dodgeChance > 0)
	{
		int rnd = randint(1, INTEGRAL_FRACTION_TOTAL);
		if(rnd <= dodgeChance)
		{
			mIsDodged = true;
			amount = 0;
		}
	}
}

void AbilityCalculator :: ApplyHitChances(int &amount, int damageType)
{
	if(amount <= 0)
		return;

	int levelDiff = ciSource->css.level - ciTarget->css.level;
	if(levelDiff < 0)
	{
		int missChance = abs(levelDiff);
		if(ciSource->serverFlags & ServerFlags::IsPlayer)
		{
			if(ciTarget->serverFlags & ServerFlags::IsPlayer)
				missChance = 0;  //Don't miss versus players in PvP.
			else
				missChance *= PLAYER_LIGHT_CHANCE_PER_LEVEL;  //A creature is the most likely target then.
		}
		else
			missChance *= CREATURE_LIGHT_CHANCE_PER_LEVEL;

		int rnd = randint(1, INTEGRAL_FRACTION_TOTAL);
		if(rnd <= missChance)
		{
			double mult = 0.0;
			if(ciSource->serverFlags & ServerFlags::IsPlayer)
				mult = randdbl(PLAYER_LIGHT_MULTIPLIER_MIN, PLAYER_LIGHT_MULTIPLIER_MAX);
			else
				mult = randdbl(CREATURE_LIGHT_MULTIPLIER_MIN, CREATURE_LIGHT_MULTIPLIER_MAX);

			amount = (int)((double)amount * mult);
			mIsLightHit = true;
		}
		else
		{
			double reduction = 0.0;
			if(ciSource->serverFlags & ServerFlags::IsPlayer)
			{
				if(ciTarget->serverFlags & ServerFlags::IsPlayer)
					reduction = 0.0;   //No reduction versus players in PvP.
				else
					reduction = PLAYER_DAMAGE_REDUCTION_PER_LEVEL * abs(levelDiff);
			}
			else
				reduction = CREATURE_DAMAGE_REDUCTION_PER_LEVEL * abs(levelDiff);

			if(reduction > 100.0)
				reduction = 100.0;
			reduction = (100.0 - reduction) / 100.0;  //Convert reduction percentage into a multipler to achieve the desired result.
			amount = (int)((double)amount * reduction);
		}
	}
	if(amount > 0 && mIsLightHit == false)
		CheckCritical(amount, damageType);
}

void AbilityCalculator :: CheckCritical(int &amount, int damageType)
{
	//Roll critical check.  Return the adjusted critical damage.
	//Critical percent will be based on an adjusted integer that
	//can compensate for fine-tuned adjustments.
	//A value of 1000 will indicate a 100% chance to critical.

	//Alternate formula for Basic Crit:
	//  ((luck / 480) * (luck / 10)) + (luck / 160)
	//Alternate formula for Super Crit
	//  (luck / 75) * (luck / 10)

	//Base critical chance.
	int CritChance = 0;

	//Luck will increase critical chance.  It can be modified by skills
	//like Lady Luck which give a percentage boost.  
	//Calculated here, 500 luck = 50 points = 5% bonus
	int luckTotal = ciSource->css.base_luck;
	luckTotal += Util::GetAdditiveFromIntegralPercent100(luckTotal, ciSource->css.mod_luck);
	//g_Log.AddMessageFormat("Luck: %d to %d", ciSource->css.base_luck, luckTotal);

	//The formula = (luckTotal / 250) ^ 2
	//produces the following chances:
	/*
	100	0.16
	200	0.64
	300	1.44
	400	2.56
	450	3.24
	500	4.00
	550	4.84*/
	float luckMod = (float)luckTotal / 250.0F;
	CritChance += (int)((1.0 + (luckMod * luckMod)) * 10);

	//Check level difference.  If the attacker is higher level,
	//give a bonus chance.  If the attacker is lower level, it will
	//reduce their critical rate by a more significant factor.
	int levelDiff = ciSource->css.level - ciTarget->css.level;
	float levelMult = 0.1F;

	//Players and Sidekicks don't have this aggro flag, but mobs do.
	if(ciSource->css.aggro_players == 0)
	{
		if(levelDiff > 5)
			levelDiff = 5;
	}
	else
	{
		CritChance += 50;  //Base mob bonus since they don't have luck.
		levelMult = 1.0F;  //Make higher level creatures more of a threat.
	}

	//Sidekicks tend to be lower level.  Remove the level bonus against them.
	if(ciTarget->AnchorObject != NULL)
		levelDiff = 0;

	if(levelDiff > 0)
		CritChance += (int)((float)levelDiff * levelMult * 10.0F);
	else
		CritChance += (int)((float)levelDiff * 10.0F);

	switch(damageType)
	{
	case DamageType::MELEE:
		CritChance += (int)ciSource->css.mod_melee_to_crit;
		break;
	case DamageType::FIRE:
	case DamageType::FROST:
	case DamageType::MYSTIC:
	case DamageType::DEATH:
		CritChance += (int)ciSource->css.mod_magic_to_crit;
		break;
	}
	//g_Log.AddMessageFormat("Final crit chance: %d (%d)", CritChance, (int)ciSource->css.mod_melee_to_crit);

	//Roll critical. If CritChance is zero or less, it will never trigger.
	int rnd = randint(1, 1000);
	if(rnd <= CritChance)
	{
		//Success!  Set critical flag and modify the damage.
		mCriticalHitState = CRITICAL_NORMAL;
		amount *= 2;

		if(ciSource->serverFlags & ServerFlags::IsPlayer)
		{
			/*
			//Check for super crit now
			if(luckTotal >= 501)
				luckMult = 0.6F;
			else if(luckTotal >= 401)
				luckMult = 0.5F;
			else if(luckTotal >= 301)
				luckMult = 0.4F;
			else if(luckTotal >= 201)
				luckMult = 0.3F;
			else if(luckTotal >= 101)
				luckMult = 0.2F;
			else
				luckMult = 0.1F;
			int SuperChance = (int)((float)luckTotal * luckMult);
			*/
			//100~10, 200~40, 300~110, 400~190, 500~300, 550~370
			float luckMod = (float)luckTotal / 90.0F;
			int SuperChance = (int)(luckMod * luckMod) * 10;

			rnd = randint(1, 1000);
			if(rnd <= SuperChance)
			{
				if(g_ProtocolVersion >= 37)
					mCriticalHitState = CRITICAL_SUPER;
				else
					ciSource->NotifySuperCrit(ciTarget->CreatureID);
				amount *= 2;
				//g_Log.AddMessageFormatW(MSG_DIAGV, "Supercrit (chance: %d)", SuperChance);
			}
		}
	}
	//g_Log.AddMessageFormatW(MSG_DIAGV, "%s (%d) attacking %s (%d) for %d rate (lvDff: %d, mult: %g)", ciSource->css.display_name, ciSource->css.level, ciTarget->css.display_name, ciTarget->css.level, CritChance, levelDiff, levelMult);
}


void AbilityCalculator :: SendDamageString(const char *abilityName)
{
	int totalDamage = mTotalDamageMelee + mTotalDamageFire + mTotalDamageFrost + mTotalDamageMystic + mTotalDamageDeath;
	
	//Prepare the header
	int size = 0;
	size += PutByte(&GSendBuf[size], 4);
	size += PutShort(&GSendBuf[size], 0);
	size += PutInteger(&GSendBuf[size], ciTarget->CreatureID);

	if(mIsBlocked == true)
	{
		size += PutByte(&GSendBuf[size], 19);  //Event for block
		mIsBlocked = false;
	}
	else if(mIsParried == true)
	{
		size += PutByte(&GSendBuf[size], 9);  //Event for parry
		mIsParried = false;
	}
	else if(mIsDodged == true)
	{
		size += PutByte(&GSendBuf[size], 18);  //Event for dodge
		size += PutInteger(&GSendBuf[size], ciSource->CreatureID);
		mIsDodged = false;
	}
	else if(mIsMissed == true || totalDamage == 0)
	{
		size += PutByte(&GSendBuf[size], 8);  //Event for miss
		size += PutInteger(&GSendBuf[size], ciSource->CreatureID);
		mIsMissed = false;
	}
	else
	{
		//Regular hit.
		//Build the damage string based on data.
		if(mTotalDamageMelee != 0)
			AddDamageString(DamageType::MELEE, mTotalDamageMelee);
		if(mTotalDamageFire != 0)
			AddDamageString(DamageType::FIRE, mTotalDamageFire);
		if(mTotalDamageFrost != 0)
			AddDamageString(DamageType::FROST, mTotalDamageFrost);
		if(mTotalDamageMystic != 0)
			AddDamageString(DamageType::MYSTIC, mTotalDamageMystic);
		if(mTotalDamageDeath != 0)
			AddDamageString(DamageType::DEATH, mTotalDamageDeath);

		size += PutByte(&GSendBuf[size], 7);
		size += PutInteger(&GSendBuf[size], ciSource->CreatureID);
		size += PutStringUTF(&GSendBuf[size], mDamageStringBuf);
		size += PutStringUTF(&GSendBuf[size], abilityName);

		if(g_ProtocolVersion >= 38)
			size += PutStringUTF(&GSendBuf[size], abilityName);  //Translated ability name.
		
		//Not sure when this was added for expanded criticals
		//0.8.7 is version 35, and uses a byte for criticals
		if(g_ProtocolVersion >= 36)
			size += PutInteger(&GSendBuf[size], static_cast<int>(mCriticalHitState));
		else
			size += PutByte(&GSendBuf[size], mCriticalHitState);

		if(g_ProtocolVersion >= 21)
			size += PutInteger(&GSendBuf[size], mAbsorbedDamage);

		if(g_ProtocolVersion >= 36)
			size += PutByte(&GSendBuf[size], mIsLightHit);
	}
			
	PutShort(&GSendBuf[1], size - 3);
	ciTarget->actInst->LSendToLocalSimulator(GSendBuf, size, ciTarget->CurrentX, ciTarget->CurrentZ);

	if(totalDamage > 0)
	{
		ciTarget->CheckStatusInterruptOnHit();
		ciTarget->CancelInvisibility();
		ciTarget->ApplyRawDamage(totalDamage);
		ciTarget->CheckInterrupts();

		ciSource->SetCombatStatus();
		ciTarget->SetCombatStatus();

		ciTarget->RegisterHostility(ciSource, 1);
	}
	mDamageStringPos = 0;
}

void AbilityCalculator :: AddDamageString(int damageType, int damageAmount)
{
	if(mDamageStringPos != 0)
		mDamageStringBuf[mDamageStringPos++] = '|';
	mDamageStringPos += sprintf(&mDamageStringBuf[mDamageStringPos], "%d:%d", damageType, damageAmount);
}

void AbilityCalculator :: ModifyOrbs(void)
{
	if(ciSourceAb->bResourcesSpent == true)
		return;

	ciSourceAb->bResourcesSpent = true;

	mMightAdjust = ciSourceAb->mightAdjust;
	mWillAdjust = ciSourceAb->willAdjust;
	mMightChargeAdjust = ciSourceAb->mightChargeAdjust;
	mWillChargeAdjust = ciSourceAb->willChargeAdjust;
	ciSourceAb->bResourcesSpent = true;
	
	if(mMightAdjust != 0)
	{
		ciSource->css.might = Util::ClipInt(ciSource->css.might + mMightAdjust, 0, MAX_MIGHT_WILL);
		mStatUpdate.push_back(STAT::MIGHT);
	}

	if(mWillAdjust != 0)
	{
		ciSource->css.will = Util::ClipInt(ciSource->css.will + mWillAdjust, 0, MAX_MIGHT_WILL);
		mStatUpdate.push_back(STAT::WILL);
	}

	if(mMightChargeAdjust != 0)
	{
		ciSource->css.might_charges = Util::ClipInt(ciSource->css.might_charges + mMightChargeAdjust, 0, MAX_CHARGES);
		mStatUpdate.push_back(STAT::MIGHT_CHARGES);
	}

	if(mWillChargeAdjust != 0)
	{
		ciSource->css.will_charges = Util::ClipInt(ciSource->css.will_charges + mWillChargeAdjust, 0, MAX_CHARGES);
		mStatUpdate.push_back(STAT::WILL_CHARGES);
	}
	if(mStatUpdate.size() > 0)
	{
		for(size_t i = 0; i < mStatUpdate.size(); i++)
			ciSource->SendStatUpdate(mStatUpdate[i]);
		mStatUpdate.clear();
	}
}

int AbilityCalculator :: GetImplicitDamage(void)
{
	return mImplicitMeleeDamage;
}

void AbilityCalculator :: RegisterTargetImplicitActions(int eventType, int damage)
{
	//Block implicit actions from triggering other implicit actions.  For example, if two players
	//have parry, it could theoretically trigger an infinite loop if they continously trigger against
	//each other.
	if(mImplicitInProgress == true)
		return;

	AbilityImplicitAction newAction;

	for(size_t i = 0; i < ciTarget->implicitActions.size(); i++)
	{
		newAction.Clear();

		ImplicitAction *implicit = &ciTarget->implicitActions[i];
		if(implicit->eventType != eventType)
			continue;

		newAction.source = ciSource;
		newAction.target = ciTarget;
		newAction.eventType = eventType;
		newAction.abilityID = implicit->abilityID;  //Certain abilities register themselves with actions that must be performed.
		if(eventType == EventType::onParry)
			newAction.damage = damage;
		mImplicitActions.push_back(newAction);
	}
}

void AbilityCalculator :: RunImplicitActions(void)
{
	size_t count = mImplicitActions.size();
	if(count == 0)
		return;

	//Temporarily mark set this so that implicit actions may not be processed.
	mImplicitInProgress = true;
	for(size_t i = 0; i < count; i++)
	{
		AbilityImplicitAction *action = &mImplicitActions[i];
		switch(action->eventType)
		{
		//For parries, the original target was the victim who was hit by damage and successfully parried
		//the attack.  The caster (source) is the one that should now receive retaliatory damages
		//or effects.  So source/target are flipped from the original event.
		case EventType::onParry:
			mImplicitMeleeDamage = action->damage;  //Need to set the damage.
			ciTarget = action->source;              //Set the target, since we need to override targeting.
			g_AbilityManager.ActivateImplicit(action->target, action->source, action->abilityID, EventType::onParry);
			break;
		}
	}
	mImplicitActions.clear();
	mImplicitInProgress = false;
}

int AbilityCalculator :: GetAdjustedChannelDuration(int initialDuration)
{
	if(mChannelExtendedDurationSec == 0)
		return initialDuration;

	//If a duration has been explicitly chosen with the Duration() ability function, calculate it.
	//Channel durations are in milliseconds, so convert appropriately.  Perform bounds checking to
	//make sure the calculated amount can fit within the 'short' integer range.
	int channelTime = mChannelExtendedDurationSec;
	if(channelTime < 1 || channelTime > 32)
	{
		g_Log.AddMessageFormat("[DEBUG] Ability Duration is invalid: %d", channelTime);
		channelTime = Util::ClipInt(channelTime, 1, 32);
	}

	//Now that the duration is utilized, clear it.
	mChannelExtendedDurationSec = 0;

	return channelTime * 1000;
}

int AbilityCalculator :: GetAdjustedInterruptChance(void)
{
	int amount = mChannelInterruptChanceMod;
	if(amount != 0)
		mChannelInterruptChanceMod = 0;

	return amount;
}

bool AbilityCalculator :: CheckActivationChance(unsigned char requiredChance, unsigned char chanceGroupID)
{
	//No chance to compare.
	if(requiredChance == 0)
		return true;

	
	if(mChanceRoll == 0 || (chanceGroupID || mChanceRollID))
	{
		mChanceRoll = randint(1, 100);
		mChanceRollID = chanceGroupID; 
	}
	return (mChanceRoll <= requiredChance);
}



UniqueAbility :: UniqueAbility(int ID, int groupID, int level)
{
	mID = ID;
	mGroupID = groupID;
	mLevel = level;
}

void UniqueAbilityList :: AddAbilityToList(const AbilityEntry2* abilityPtr)
{
	int id = abilityPtr->mAbilityID;
	int gid = abilityPtr->mAbilityGroupID;
	int lev = abilityPtr->mReqLevel;
	for(size_t i = 0; i < mAbilityList.size(); i++)
	{
		if(gid != mAbilityList[i].mGroupID)
			continue;

		//If lower level, exit so it doesn't add a lower level buff as a new entry at the
		//end of the function.
		if(lev < mAbilityList[i].mLevel)
			return;

		mAbilityList[i].mID = id;
		mAbilityList[i].mGroupID = gid;
		mAbilityList[i].mLevel = lev;
		return;
	}
	mAbilityList.push_back(UniqueAbility(id, gid, lev));
}


void AbilityManager2 :: DamageTest(CreatureInstance *playerData)
{
	CreatureInstance *targ = playerData->CurrentTarget.targ;
	if(targ == NULL)
		return;

	ABILITY_ITERATOR it;
	FUNCTION_ITERATOR fit;
	FILE *mDebugDumpFile = fopen("debug_ability_dump.txt", "wb");
	static const char * functList[] = { "MeleeDamage", "FrostDamage", "FireDamage", "MysticDamage", "DeathDamage", "Heal", "A_Heal"};
	static const int count = sizeof(functList) / sizeof(functList[0]);

	static const char *replaceSrc[] = { "A_STRENGTH", "A_DEXTERITY", "A_CONSTITUTION", "A_PSYCHE", "a_psyche", "A_SPIRIT", "a_spirit", "MWD",  "mwd",  "RWD",   "rwd",   "mightCharges", "willCharges" };
	static const char *replaceDst[] = { "$B$2",       "$B$3",        "$B$4",           "$B$5",     "$B$5",     "$B$6",     "$B$6",     "$B$17", "$B$17", "$B$18", "$B$18", "$B$19",        "$B$20" };
	static const int repCount = sizeof(replaceSrc) / sizeof(replaceSrc[0]);


	std::string outstr;
	for(it = mAbilityIndex.begin(); it != mAbilityIndex.end(); ++it)
	{
		bool header = false;
		for(int e = 0; e < EventType::MAX_EVENT; e++)
		{
			outstr.clear();
			AbilityEvent2 *evt = it->second.GetEvent(e);
			for(size_t f = 0; f < evt->mFunctionList.size(); f++)
			{
				bool pass = false;
				for(size_t c = 0; c < count; c++)
				{
					if(evt->mFunctionList[f].mFunctionName.compare(functList[c]) == 0)
					{
						pass = true;
						break;
					}
				}
				if(pass == true)
				{
					if(header == false)
					{
						fprintf(mDebugDumpFile, "%d\t%d\t%s\t%d\t", it->second.mAbilityID, it->second.mAbilityGroupID, it->second.GetRowAsString(ABROW::NAME).c_str(), it->second.mReqLevel);
						header = true;
					}

					if(outstr.size() == 0)
						outstr = "=";
					else
						outstr.append("+");
					outstr.append("(");
					for(size_t a = 0; a < evt->mFunctionList[f].mArguments.size(); a++)
					{
						outstr.append(evt->mFunctionList[f].mArguments[a]);
					}
					outstr.append(")");
				}
			}
			if(e == EventType::onIterate)
			{
				if(outstr.size() > 0)
				{
					int num = it->second.mChannelDuration;
					int den = it->second.mChannelIteration;
					if(den == 0)
						den = 1;
					int iterations = num / den;
					char buffer[256];
					sprintf(buffer, "*(%d)", iterations);
					outstr.append(buffer);
				}
			}
			if(header == true)
			{
				size_t pos = 0;
				int replaced = 0;
				do
				{
					replaced = 0;
					for(size_t s = 0; s < repCount; s++)
					{
						pos = outstr.find(replaceSrc[s]);
						if(pos != std::string::npos)
						{
							size_t srcLen = strlen(replaceSrc[s]);
							//size_t dstLen = strlen(replaceDst[s]);
							outstr.replace(pos, srcLen, replaceDst[s]);
							replaced++;
						}
					}
				} while(replaced > 0);

				if(e > 0)
					fputc('\t', mDebugDumpFile);
				fputs(outstr.c_str(), mDebugDumpFile);
			}
		}
		if(header == true)
			fputs("\r\n", mDebugDumpFile);
	}
	if(mDebugDumpFile != NULL)
		fclose(mDebugDumpFile);
}


//Generic function for random ability scanning stuff that's needed for external statistics or tracking.
//Is not required for regular server operation and can be safely removed or left empty.
void AbilityManager2::DebugStuff(void)
{
	FILE *output = fopen("ItemDef_SkillOrb.txt", "wb");
	if(output == NULL)
		return;
	int itemID = 70000;
	ABILITY_ITERATOR it;
	std::map<int, int> plist;
	for(it = mAbilityIndex.begin(); it != mAbilityIndex.end(); ++it)
	{
		if(it->second.IsPassive() == true)
			continue;
		if(it->second.mAbilityID >= 5000)
			continue;
		if(plist[it->second.mAbilityGroupID] != 0)
			continue;
		int lookFor = it->second.mReqLevel;
		int compare = 0;
		switch(it->second.mTier)
		{
		case 1: compare = 1; break;
		case 2: compare = 6; break;
		case 3: compare = 20; break;
		case 4: compare = 30; break;
		case 5: compare = 40; break;
		case 6: compare = 50; break;
		}
		if(compare == 0 || compare != lookFor)
			continue;

		plist[it->second.mAbilityGroupID]++;

		const char *backgroundIcon = "Icon-32-BG-Grey.png";

		const char *type = it->second.GetRowAsCString(ABROW::CATEGORY);
		if(strcmp(type, "CoreK") == 0) backgroundIcon = "Icon-32-BG-Red.png";
		else if(strcmp(type, "CrossK") == 0) backgroundIcon = "Icon-32-BG-Pink.png";
		else if(strcmp(type, "CoreR") == 0) backgroundIcon = "Icon-32-BG-Green.png";
		else if(strcmp(type, "CrossR") == 0) backgroundIcon = "Icon-32-BG-Cyan.png";
		else if(strcmp(type, "CoreM") == 0) backgroundIcon = "Icon-32-BG-Blue.png";
		else if(strcmp(type, "CrossM") == 0) backgroundIcon = "Icon-32-BG-Aqua.png";
		else if(strcmp(type, "CoreD") == 0) backgroundIcon = "Icon-32-BG-Yellow.png";
		else if(strcmp(type, "CrossD") == 0) backgroundIcon = "Icon-32-BG-Divine.png";

		int level = 0;
		level = Util::ClipInt(it->second.mReqLevel - 15, 1, 50);

		fprintf(output, "[ENTRY]\r\n");
		fprintf(output, "mID=%d\r\n", itemID++);
		fprintf(output, "mType=5\r\n");
		fprintf(output, "mDisplayName=Skill Scroll: %s\r\n", it->second.GetRowAsCString(ABROW::NAME));
		fprintf(output, "mIcon=Icon-32-Scroll2.png|%s\r\n", backgroundIcon);
		fprintf(output, "mIvType1=1\r\n");
		fprintf(output, "mIvMax1=20\r\n");
		fprintf(output, "mLevel=%d\r\n", level);
		fprintf(output, "mUseAbilityId=%d\r\n", it->second.mAbilityID);
		fprintf(output, "mValue=1\r\n");
		fprintf(output, "mValueType=1\r\n");
		fprintf(output, "mQualityLevel=2\r\n");
		fprintf(output, "mMinUseLevel=%d\r\n", level);
		fprintf(output, "Params=verifycast=1\r\n");
		fprintf(output, "\r\n");
	}
	fclose(output);
}


} //namespace Ability2

