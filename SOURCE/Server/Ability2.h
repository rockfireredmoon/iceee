#pragma once
#ifndef ABILITY2_H
#define ABILITY2_H

#include <map>
#include <vector>
#include <string>
#include <stdarg.h>
#include "CommonTypes.h"

struct ActiveAbilityInfo;
class CreatureInstance;

namespace ABGlobals
{
	const int MAX_MIGHT = 10;
	const int MAX_WILL  = 10;
	const int MAX_MIGHTCHARGES = 5;
	const int MAX_WILLCHARGES  = 5;
	const float DEFAULT_MIGHT_REGEN = 1.0F;
	const float DEFAULT_WILL_REGEN  = 1.0F;
	const float MINIMAL_FLOAT       = 1.4013e-045F;

	const int DEFAULT_OFFHAND_WEAPON_DAMAGE    = 500;
	const int DEFAULT_CASTING_SETBACK_CHANCE   = 500;
	const int DEFAULT_CHANNELING_BREAK_CHANCE  = 500;

	const int INTEGRAL_FRACTION_TOTAL = 1000;   //Many percentage modifiers used in the client assume a scale of 0-1000 to allow more precise fractional computations while storing as integers.
}

enum
{
	ABILITYID_AUTO_ATTACK = 32766,
	ABILITYID_AUTO_ATTACK_STOP_MELEE = 32759,
	ABILITYID_AUTO_ATTACK_RANGED = 32760,
	ABILITYID_QUEST_INTERACT_OBJECT = 32700,  //Simulates quest interactions.
	ABILITYID_QUEST_GATHER_OBJECT = 32701,    //Simulates quest gather (deleted when successful).
	ABILITYID_INTERACT_OBJECT = 32702,        //Simulates world interactions (like warps).
	RESERVED_ABILITY_SUMMON = 32100,
	RESERVED_ABILITY_BONUS = 32101
};

struct AbilityStatus
{
	enum Enum
	{
		WARMUP           = 0,
		ACTIVATE         = 1,
		SETBACK          = 2,
		CHANNELING       = 3,
		INTERRUPTED      = 4,
		INITIAL_FAIL     = 5,
		ABILITY_ERROR    = 6,
		REQUEST_ACCEPTED = 7,
		ABILITY_FINISHED = 8
	};
};

struct TargetStatus
{
	enum Flags
	{
		TSB_ALIVE   = (1 << 0),
		TSB_DEAD    = (1 << 1),
		TSB_FRIEND  = (1 << 2),
		TSB_ENEMY   = (1 << 3)
	};

	enum Enum
	{
		None          = 0,
		Alive         = TSB_ALIVE,   //Not used in official files, added for my purposes for an indiscriminate mass heal
		Dead          = TSB_DEAD,
		Enemy         = TSB_ENEMY,
		Enemy_Alive   = TSB_ENEMY | TSB_ALIVE,
		Friend        = TSB_FRIEND,
		Friend_Alive  = TSB_FRIEND | TSB_ALIVE,
		Friend_Dead   = TSB_FRIEND | TSB_DEAD
	};
};

/*
struct Action
{
	enum Enum
	{
		None       = 0,
		onIterate  = 1,
		onParry    = 2,
		onRequest  = 3,
		onActivate = 4,
	};
};
*/

namespace EventType
{
	//TODO: These were set up to match the values used in the old ability system.
	//Change them when the old system is phased out.
	enum
	{
		UNDEFINED = -1,
		None = 0,
		onIterate = 1,
		onParry = 2,
		onRequest = 3,
		onActivate = 4,
		onDeactivate = 5,
		MAX_EVENT
	};
	const char *GetNameByEventID(int eventID);
	int GetEventIDByName(const std::string &eventName);
}

struct AbilityType
{
	enum Enum
	{
		None        = 0,
		Instant     = 1,
		Cast        = 2,
		ChannelPrep = 3,
		Channel     = 4,
		Passive     = 5
	};
};

struct DamageType
{
	enum Enum
	{
		MELEE       = 0,
		FIRE        = 1,
		FROST       = 2,
		MYSTIC      = 3,
		DEATH       = 4,
		UNBLOCKABLE = 5
	};
};

namespace Ability2
{

/* Conceptual Structure

AbilityManager core
  AbilityEntry ExampleAttack
    AbilityAction onRequest
	  TargetType Self
	  AbilityFunction Facing()
	  AbilityFunction Range()
	  AbilityFunction Might()
    AbilityAction onActivate
	  TargetType Self
	  AbilityFunction Damage()

  AbilityEntry ExampleExecute
  ...
  ...

*/

class AbilityManager2;

class AbilityVerify
{
public:
	int mError;
	STRINGLIST mErrorMsg;

	AbilityVerify();
	void Clear(void);
	void AddError(const char *format, ...);
	void AddError(const char *format, va_list args);
private:
	char mWriteBuffer[4096];
};

//Verification
void VerifyExpression(const std::string &exp, AbilityVerify &verifyInfo);

//Utility functions for tokenizing strings.
void Split(const std::string &source, const char *delim, STRINGLIST &dest);
void SplitFunctionList(const std::string &input, STRINGLIST &output);
bool SplitFunction(const std::string &input, STRINGLIST &output);
void TrimWhitespace(std::string &modify);
void TrimQuote(std::string &modify);
void TrimParenthesis(std::string &modify);
//void SplitDelimQuote(const std::string &input, STRINGLIST &output);
void RemoveTrailingNewlines(char *str);

//void ToLowerCase(std::string &input);   //Moved to Util.h since an unrelated class needs it.

//Contains information necessary for any kind of function call.
class AbilityFunction2
{
public:
	unsigned char mChance;       //Chance to trigger (1-100%).  Only considered if nonzero.
	unsigned char mChanceID;     //All functions of the same chance block {...} will operate on a single roll.
	std::string mFunctionName;
	STRINGLIST mArguments;
	MULTISTRING mArgumentCache;  //This holds cached argument data, like tables of generated postfix tokens.
	bool mCached;
	
	AbilityFunction2();
	void Clear(void);
	void Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const;
	void AssignFormula(const std::string &formula);
	const char* GetString(size_t argIndex) const;
	int GetInteger(size_t argIndex) const;
	float GetFloat(size_t argIndex) const;
	float GetEvaluation(size_t argIndex, AbilityManager2 *symbolResolver);
};

class AbilityEvent2
{
public:
	std::string mActionType;          //Corresponds to event (ex: "onActivate")
	AbilityFunction2 mTargetTypeStr;  //Target selection function when processing this event.
	std::vector<AbilityFunction2> mFunctionList;   //List of condition or action functions to call when the event is processed.
	
	unsigned char mChanceID;       //Theoretically an event may need to have multiple chance blocks.  This allows them to separate different groups of actions with different rolls.
	int mTargetType;               //Resolved from mTargetTypeStr 
	int mTargetRange;              //Resolved from mTargetTypeStr (first argument of function, if it exists)

	AbilityEvent2();
	~AbilityEvent2();
	void SetFunctionEvent(const std::string &eventFunctionList);
	void SetFullEvent(const STRINGLIST &eventParams);
	void ResolveTargetingInfo(void);
	void DebugPrint(void);
	void Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const;
	const char* GetFunctionArgument(const char *functionName, size_t argIndex);
	bool HasTargetType(void);
	bool HasDifferentTargetType(AbilityEvent2 *other);
	void CopyTargetType(const AbilityEvent2 &other);
private:
	void AddChanceFunctionList(const std::string &eventFunctionList);
};

namespace TargetType
{
	enum TargetTypeEnum
	{
		None = 0,
		Self,           //         Caster only.
		ST,             //         A single target.
		Implicit,       //         used for "onParry" actions 
		Party,          //(200)    Party members within (range)
		Cone180,        //(30)     180 degree arc of (range)
		Cone90,         //(100)    90 degree arc of (range)
		GTAE,           //(100)    All targets in the selected ground area (range)   [ex: swarm, morass]
		PBAE,           //(100)    All enemies around player (range)
		STAE,           //(100)    All objects around the selected target? (range)  [only used for mob skills]
		STXAE,          //(100)    The selected target and all objects around it within (range).  [ex: Cataclysm]
		STP             //         Port a target somewhere.
	};
	const char *GetNameByTargetType(int targetType);
	int GetTargetTypeByName(const std::string &targetTypeName);
}

namespace TargetFilter
{
	//Define some basic flags to make the lower part easier to understand.
	static const int FLAG_ALIVE   = (1 << 0); 
	static const int FLAG_DEAD    = (1 << 1);
	static const int FLAG_FRIEND  = (1 << 2);
	static const int FLAG_ENEMY   = (1 << 3);

	enum TargetFilterEnum
	{
		None         = 0,            //None/unspecified.
		Alive        = FLAG_ALIVE,   //Not used in official ability table.
		Dead         = FLAG_DEAD,
		Enemy        = FLAG_ENEMY,
		Enemy_Alive  = FLAG_ENEMY | FLAG_ALIVE,
		Friend       = FLAG_FRIEND,
		Friend_Alive = FLAG_FRIEND | FLAG_ALIVE,
		Friend_Dead  = FLAG_FRIEND | FLAG_DEAD
	};
	int GetTargetFlagsByName(const std::string name);
}

// Determines the processing state for abilities that are currently firing.
struct AbilityType
{
	enum Enum
	{
		None           = 0,
		Instant        = 1,   //Is an instant effect.
		Cast           = 2,   //Is casting (waiting for warmup time).
		Channel        = 3,   //Is channeling a channel.
		Passive        = 4    //Same as Instant for processing purposes.
	};
};

// Holds flags for extra information and control processing.
struct AbilityFlags
{
	enum Enum
	{
		Melee              = (1 << 0),  //This is a melee attack (3 meter range or less).
		Ranged             = (1 << 1),  //This is a ranged attack (greater than 3 meter range).
		PhysicalDamage     = (1 << 2),  //The attack applies at least some physical damage.
		MagicDamage        = (1 << 3),  //The attack applies at least some magic damage.
		Charge             = (1 << 4),  //Builds up charges when used.
		Execute            = (1 << 5),  //Spends charges when used.

		Knight             = (1 << 6),  //This ability may be purchased by Knights.
		Rogue              = (1 << 7),  //This ability may be purchased by Rogues.
		Mage               = (1 << 8),  //This ability may be purchased by Mages.
		Druid              = (1 << 9),  //This ability may be purchased by Druids.

		Passive            = (1 << 10),  //This ability is a passive ability (special processing required to initiate effects).
		Channel            = (1 << 11),  //When onActivate finishes, there will be a channel effect.
		ImplicitParry      = (1 << 12),  //This ability registers an implicit action to be performed when the player successfully parries an enemy attack.
		SecondaryTarget    = (1 << 13),  //The channel must notify the client as the secondary target list.  Needed for rare abilities where onIterate has a different TargetType than onActivate.

		//These attribute are unique to this server, and are flags that can be arbitrarily assigned
		//to abilities by named flags via a bonus column in the ability table.
		SecondaryChannel   = (1 << 14),  //A channel that operates in the secondary slot and cannot be interrupted.  Will perform its action in the background allowing other skills to be cast in the meantime.  Usefor for iterative effects like bleeding.
		UnbreakableChannel = (1 << 15),  //A channel that operates in the primary slot, but cannot be interrupted by attacks or movement.  However, it can be interrupted by the player if they try to cast another ability before the channel finishes.
		AllowDeadState     = (1 << 16)   //Allow casting even if the actor is dead.
	};
};

//Matches a client entry of the same name.
//See also:  ABROW::USE_TYPE
struct AbilityUseType
{
	enum Type
	{
		CAST      = 0x1,
		CHANNELED = 0x2,
		PASSIVE   = 0x4
	};
};


class AbilityEntry2
{
public:
	AbilityEntry2();
	~AbilityEntry2();

	void ImportRow(const STRINGLIST &newRowData);
	
	void DebugPrint(void);
	void Verify(AbilityManager2 *parent, AbilityVerify &verifyInfo) const;
	AbilityEvent2* GetEvent(int eventID);
	const char* GetRowAsCString(size_t index) const;
	const std::string& GetRowAsString(size_t index) const;
	int GetRowAsInteger(size_t index) const;

	//This information will be resolved after the skill entry is loaded.
	int mAbilityID;         //Ability ID.  Makes things more handy for the processing system.
	int mAbilityGroupID;    //Group ID (used for buffs and passives to cancel out existing bonuses of the same group.
	int mTargetType;        //Maps to TargetType enum
	int mTargetTypeRange;   //Pre-resolved range for targeting functions that have a numerical parameter (like GTAE).
	int mTargetFilter;      //Maps to TargetFilter enum
	int mAbilityType;        //Maps to AbilityType enum
	int mAbilityFlags;      //Resolved AbilityFlags for various processing and state check needs.
	int mTier;              //Tier (often 1-8)
	int mReqLevel;          //Required level to purchase.
	int mReqCostOtherClass; //Required ability point cost for non-class cross abilities.
	int mReqCostInClass;    //Required ability point cost for same class abilities.
	std::vector<short> mReqAbilityID;  //Required abilities that must already be purchased before this one can be purchased.

	int mWarmupTime;
	int mChannelDuration;
	int mChannelIteration;
	int mCooldownTime;

	STRINGLIST mRowData;              //The input string, broken into rows.

	//Helper functions for ability processing.
	bool IsPhysicalDamageOnly(void) const;
	bool IsPassive(void) const;
	bool IsPurchaseableBy(int profession) const;
	int GetPurchaseCost(void) const;
	int GetPurchaseLevel(void) const;
	void GetPurchasePrereqList(std::vector<short> &output) const;

	bool CanUseWhileDead(void) const;

	static const int MINIMUM_ABILITY_COST = 2;

private:
	AbilityEvent2 mEvents[EventType::MAX_EVENT];
	void InitializeData(void);
	void FinalizeData(void);
	const char* GetFunctionArgument(int eventType, const char *functionName, size_t argIndex);
};

struct ABVerifier
{
	//Argument types may have different verification requirements.
	enum argType
	{
		NONE = 0,       //No verification required.
		AMOUNT,         //A generic numerical amount that may be calculated by an arbitrary expression 
		STATID,         //A string that resolves into a stat ID (ex: STRENGTH)
		EFFECT,         //A string that resolves into a status effect (ex: STUN)
		TIER,           //Ability tiers are normally expressed as integers.
		TIME,           //Time values are normally expressed as integers.
		DIST,           //Distance values are normally expressed as integers.
		BUFF,           //A string that resolves into a buff category.
		ITEMID,         //A string that resolves into an item ID.
		STRING          //A generic string.
	};

	ABVerifier()
	{
		for(size_t i = 0; i < MAX_ARG; i++)
			argIndex[i] = 0;
	}
	ABVerifier(int arg0)
	{
		argIndex[0] = arg0;
		argIndex[1] = 0;
		argIndex[2] = 0;
		argIndex[3] = 0;
	}
	ABVerifier(int arg0, int arg1)
	{
		argIndex[0] = arg0;
		argIndex[1] = arg1;
		argIndex[2] = 0;
		argIndex[3] = 0;
	}
	ABVerifier(int arg0, int arg1, int arg2)
	{
		argIndex[0] = arg0;
		argIndex[1] = arg1;
		argIndex[2] = arg2;
		argIndex[3] = 0;
	}
	ABVerifier(int arg0, int arg1, int arg2, int arg3)
	{
		argIndex[0] = arg0;
		argIndex[1] = arg1;
		argIndex[2] = arg2;
		argIndex[3] = arg3;
	}
	//Normally we'd max at 3, but some functions have 4, and this helps to suppress warnings while still verifying everything.
	static const int MAX_ARG = 4;  //The maximum number of arguments that exist in the verifier parameter list, not the max number of arguments for the functions themselves.
	int argIndex[MAX_ARG];
};


enum AbilityReturnCode
{
	ABILITY_SUCCESS = 0,
	
	//Special case critical error codes, for undefined ability or event calls.
	ABILITY_NOT_FOUND = 1,  //The ability was not found in the loaded ability table.
	ABILITY_BAD_EVENT,      //The ability has an invalid event type.
	ABILITY_NOT_SPECIAL,    //The ability does not have a hack for special internal processing.
	ABILITY_SPECIAL,        //The ability does have a hack for special internal processing (special case, just needs to have a different value than ABILITY_NOT_SPECIAL for comparison purposes).

	//Function error return codes begin here.  The player may need to be informed of this information
	ABILITY_GENERIC,               //Non-specific failure.
	ABILITY_NO_TARGET,             //No targets found.
	ABILITY_COOLDOWN,              //Ability is still on cooldown and can't be used yet.
	ABILITY_PENDING,               //An ability is already being processed.
	ABILITY_FACING,                //Not facing target.
	ABILITY_INRANGE,               //Not within range of target.
	ABILITY_NOTSILENCED,           //Player is silenced.
	ABILITY_MIGHT,                 //Not enough might to cast.
	ABILITY_WILL,                  //Not enough will to cast.
	ABILITY_MIGHT_CHARGE,          //Not enough might charges to cast.
	ABILITY_WILL_CHARGE,           //Not enough will charges to cast.
	ABILITY_HASMAINHAND,           //No mainhand weapon equipped.
	ABILITY_HASOFFHAND,            //No offhand weapon equipped.
	ABILITY_HASMELEE,              //No melee weapon equipped.
	ABILITY_HASSHIELD,             //No shield equipped.
	ABILITY_HASBOW,                //No bow equipped.
	ABILITY_HAS2HORPOLE,           //No 2-hand or pole weapon equipped.
	ABILITY_HASWAND,               //No wand equipped.
	ABILITY_STATUS,                //The status effect condition does not match the requirement (HasStatus or NotStatus)
	ABILITY_HEALTH_TOO_LOW,        //The caster does not have enough health to use the ability.
	ABILITY_BEHIND,                //The caster is not behind its target.
	ABILITY_NEARBY_SANCTUARY,      //The caster is not near a sanctuary.
	ABILITY_DISARM,                //The caster is disarmed and cannot perform physical attacks.
	ABILITY_STUN,                  //The caster is stunned.
	ABILITY_DAZE,                  //The caster is dazed.
	ABILITY_DEAD,                  //The caster is dead.
};

//A more descriptive way to access which string element in the ability row's data.
//Each of these can be considered a column index.
struct ABROW
{
	enum RowIndex
	{
		ID = 0,               //<integer> Ability ID.
		NAME,                 //<string> Ability Display Name
		HOSTILITY,            //<integer> Unused.  Common values: {-1, 0, 1}
		WARMUP_TIME,          //<integer> Warmup time (milliseconds) to cast an ability.
		WARMUP_CUE,           //<string> Visual effect to play during warmup.
		DURATION,             //<integer> Duration time (milliseconds) of a channel effect.
		INTERVAL,             //<integer> Interval time (milliseconds) between processing channel iterations.
		COOLDOWN_CATEGORY,    //<string> All abilities with the same cooldown category will share the same cooldown timers.
		COOLDOWN_TIME,        //<integer> Time delay (milliseconds) before abilities with this cooldown category can be casted again.
		ACTIVATION_CRITERIA,  //<string> A variable length list of function checks required to activate an ability {onRequest}.
		ACTIVATION_ACTIONS,   //<string> A variable length list of function actions {onActive, onIterate, onParry}
		VISUAL_CUE,           //<string> Effect to play when the ability is activated.
		TIER,                 //<integer> Ability tier.  Commonly {1 ... 9}
		PREREQ,               //<string> A list of purchase requirements (tier, ability cost, prereq abilities, etc).
		ICON,                 //<string> Icon to display in the client ability window.
		DESCRIPTION,          //<string> Description to display in the client ability window.
		CATEGORY,             //<string> Typically describes which section of the client ability window it can be found in.
		COORDINATE_X,         //<integer> In the client ability window, the X coordinate on the grid to display the icon.
		COORDINATE_Y,         //<integer> In the client ability window, the X coordinate on the grid to display the icon.
		GROUP_ID,             //<integer> Used to group abilities to block buffs of similar or lower tier.
		ABILITY_CLASS,        //<string> A general string describing how the skill operates ("Charge", "Execute", "Buff", etc)
		USE_TYPE,             //<integer> Maps to a client use type.
		ADD_MELEE_CHARGE,     //<integer> Unknown/unused.
		ADD_MAGIC_CHARGE,     //<integer> Unknown/unused.
		OWNAGE,               //<integer> Unknown/unused.
		GOLD_COST,            //<integer> Coin amount (in copper) to purchase the ability.
		BUFF_CATEGORY,        //<string> Denotes the buff category.  Often present for any ability that uses the CheckBuffLimits(), which matches that parameter.
		TARGET_STATUS,        //<string> Indicates a restriction of target types "Enemy Alive", "Friend Alive", etc.
		SERVER_FLAGS          //<string> Added for this server. Specifies additional behavioral flags or overrides for certain abilities.  Originally it was <integer> Unknown/unused.  Every entry in the official table is set to 0.
	};
};


//External abilities allow special operations to function on single target.
struct AbilityImplicitAction
{
	CreatureInstance *source;  //
	CreatureInstance *target;  //
	int abilityID;
	int eventType;             //Corresponds to EventType:: object.
	int damage;                //Certain actions like parries need to know how many damage to respond to.
	void Clear(void);
};

//Controls the results of an ability while the ability request is being processed.
//Holds data information
class AbilityCalculator
{
public:
	AbilityCalculator();
	~AbilityCalculator();
	
	typedef AbilityFunction2& ARGUMENT_LIST;   //The type of the argument list passed to ability function processing.

	void Clear(void);       //Initializes and clears the entire object.
	void ResetState(void);  //Clear only the combat states and damage tallies, useful when changing targets or abilities.
	void ResetOrbs(void);   //Clear only might/will and physical/magic charge adjustments.

	CreatureInstance *ciSource;
	ActiveAbilityInfo *ciSourceAb;
	CreatureInstance *ciTarget;
	
	AbilityEntry2 *mAbilityEntry;  //The current ability being processed.  Required since some action functions need extra data about the ability, like what group ID it has.
	bool mIsRequestGTAE;           //The InRange() function needs to know if a GTAE requests is being processed so it can look at the explicit ability coordinates for distance checks rather than the target list.

	int mTotalDamage;           //All damage of any type tallied.
	void SendDamageString(const char *abilityName); //Apply all damage that the internal state has collected.
	void ModifyOrbs(void);      //Applies change to might/will and charges.
	void ConsumeReagent(void);  //Checks for any reagents that may have been registered to a cast, and remove them.

	void RegisterTargetImplicitActions(int eventType, int damage);
	void RunImplicitActions(void);   //The ability activator may need to call this to perform any implicit actions that were triggered by an ability.

	//These helper functions run the actual damage processing that may be requested by any of
	//the following ability action functions:
	//	MeleeDamage(), FireDamage(), FrostDamage(), MysticDamage(), DeathDamage()
	//  AttackMelee(), AttackRanged()

	void _DoMeleeDamage(int amount);
	void _DoFireDamage(int amount);
	void _DoFrostDamage(int amount);
	void _DoMysticDamage(int amount);
	void _DoDeathDamage(int amount);
	void _DoElementalAutoDamage();

	//static bool Chance(int amount);
	bool CheckActivationChance(unsigned char requiredChance, unsigned char chanceGroupID);

	//Emulated ability functions are placed here since some of them, especially the ones that need
	//better access to creature stats, will have simpler code.
	int Facing(ARGUMENT_LIST args);
	int InRange(ARGUMENT_LIST args);
	int NotSilenced(ARGUMENT_LIST args);
	int HasStatus(ARGUMENT_LIST args);
	int NotStatus(ARGUMENT_LIST args);
	int Interrupt(ARGUMENT_LIST args);

	int Might(ARGUMENT_LIST args);
	int Will(ARGUMENT_LIST args);
	int MightCharge(ARGUMENT_LIST args);
	int WillCharge(ARGUMENT_LIST args);
	int MeleeDamage(ARGUMENT_LIST args);
	int FireDamage(ARGUMENT_LIST args);
	int FrostDamage(ARGUMENT_LIST args);
	int MysticDamage(ARGUMENT_LIST args);
	int DeathDamage(ARGUMENT_LIST args);
	int Heal(ARGUMENT_LIST args);
	int A_Heal(ARGUMENT_LIST args);
	int Harm(ARGUMENT_LIST args);
	int PercentMaxHealth(ARGUMENT_LIST args);
	int Regenerate(ARGUMENT_LIST args);
	int AttackMelee(ARGUMENT_LIST args);
	int AttackRanged(ARGUMENT_LIST args);
	int _AttackMelee(int unused);  //Special case, handled by an internal hack instead of an ability function.
	int _AttackRanged(int unused); //Special case, handled by an internal hack instead of an ability function.
	int A_AddMightCharge(ARGUMENT_LIST args);
	int A_AddWillCharge(ARGUMENT_LIST args);
	int T_AddMightCharge(ARGUMENT_LIST args);
	int T_AddWillCharge(ARGUMENT_LIST args);
	int StealWillCharge(ARGUMENT_LIST args);
	int StealMightCharge(ARGUMENT_LIST args);
	int FullMight(ARGUMENT_LIST args);
	int FullWill(ARGUMENT_LIST args);
	int hasMainHandWeapon(ARGUMENT_LIST args);
	int hasOffHandWeapon(ARGUMENT_LIST args);
	int hasMeleeWeapon(ARGUMENT_LIST args);
	int hasRangedWeapon(ARGUMENT_LIST args);
	int hasShield(ARGUMENT_LIST args);
	int hasBow(ARGUMENT_LIST args);
	int has2HorPoleWeapon(ARGUMENT_LIST args);
	int hasWand(ARGUMENT_LIST args);

	int Status(ARGUMENT_LIST args);
	int StatusSelf(ARGUMENT_LIST args);
	int Set(ARGUMENT_LIST args);
	int Amp(ARGUMENT_LIST args);
	int Add(ARGUMENT_LIST args);
	int AddDeliveryBox(ARGUMENT_LIST args);
	int AddGrove(ARGUMENT_LIST args);
	int AddSlot(ARGUMENT_LIST args);
	int AmpCore(ARGUMENT_LIST args);
	int Nullify(ARGUMENT_LIST args);
	int CheckBuffLimits(ARGUMENT_LIST args);
	int Taunt(ARGUMENT_LIST args);
	int AddHate(ARGUMENT_LIST args);
	int TopHated(ARGUMENT_LIST args);
	int UnHate(ARGUMENT_LIST args);
	int LeaveCombat(ARGUMENT_LIST args);

	int Behind(ARGUMENT_LIST args);
	int Spin(ARGUMENT_LIST args);
	int Duration(ARGUMENT_LIST args);

	int Resurrect(ARGUMENT_LIST args);
	int NearbySanctuary(ARGUMENT_LIST args);
	int BindTranslocate(ARGUMENT_LIST args);
	int Translocate(ARGUMENT_LIST args);
	int PortalRequest(ARGUMENT_LIST args);
	int Go(ARGUMENT_LIST args);
	int GoSanctuary(ARGUMENT_LIST args);

	int RemoveAltDebuff(ARGUMENT_LIST args);
	int RemoveStatDebuff(ARGUMENT_LIST args);
	int RemoveEtcDebuff(ARGUMENT_LIST args);
	int RemoveAltBuff(ARGUMENT_LIST args);
	int RemoveStatBuff(ARGUMENT_LIST args);
	int RemoveEtcBuff(ARGUMENT_LIST args);
	int RemoveHealthBuff(ARGUMENT_LIST args);

	int Invisible(ARGUMENT_LIST args);
	int WalkInShadows(ARGUMENT_LIST args);
	int AddWDesc(ARGUMENT_LIST args);
	int HealthRestrict(ARGUMENT_LIST args);
	int HealthSacrifice(ARGUMENT_LIST args);
	int DoNothing(ARGUMENT_LIST args);
	int Reagent(ARGUMENT_LIST args);

	//Unofficial ability functions (custom features unique to this server)
	int SetAutoAttack(ARGUMENT_LIST args);
	int DisplayEffect(ARGUMENT_LIST args);
	int InterruptChance(ARGUMENT_LIST args);
	int Transform(ARGUMENT_LIST args);
	int Nudify(ARGUMENT_LIST args);
	int Untransform(ARGUMENT_LIST args);
	int NotTransformed(ARGUMENT_LIST args);

	//Server-side helper functions.
	int GetImplicitDamage(void);
	int GetAdjustedChannelDuration(int initialDuration);
	int GetAdjustedInterruptChance(void);
private:
	//Internal data when processing skill functions.  Many skills operate several functions and
	//different states or tallies must be accumulated.
	
	//Cumulative damage tallies for each damage type.  For example, a skill might deal base physical
	//damage from multiple calculations (base weapon damage, and a separate bonus),
	//with additional fire damage from a buff.
	int mTotalDamageMelee;    //Total physical damage.
	int mTotalDamageFire;
	int mTotalDamageFrost;
	int mTotalDamageMystic;
	int mTotalDamageDeath;
	int mAbsorbedDamage;

	//Track ability resources when they are checked for conditions, so that they can be
	//modified when the ability successfully fires.
	int mMightAdjust;
	int mMightChargeAdjust;
	int mWillAdjust;
	int mWillChargeAdjust;

	//Track reagents used.
	int mReagentItemID;
	int mReagentItemCount;

	int mChannelExtendedDurationSec;
	int mChannelInterruptChanceMod;

	//Certain attacks (usually physical) can be nullified by chances to block, parry, or dodge.
	//These flags indicate whether any of these states occurred.
	bool mIsBlocked;
	bool mIsParried;
	bool mIsDodged;
	bool mIsLightHit;
	bool mIsMissed;

	unsigned char mCriticalHitState;  //0 for none, 1 for normal critical, 2 for supercrit.

	unsigned char mChanceRoll;        //Determines the roll of a single batch of events.
	unsigned char mChanceRollID;      //Helps determine if a new roll is needed for a new batch of events.

	bool mImplicitInProgress;   //Important status flag that indicates whether an implicit action is being triggered, which prevents this action from potentially chaining an unpredictable number of additional implicit actions.
	int mImplicitMeleeDamage;   //Used when calculating actions in a response to incoming damage.
	std::vector<AbilityImplicitAction> mImplicitActions;  //List of implicit actions that have been triggered by an ability.  to perform after an ability has finished processing.

	char mDamageStringBuf[64];
	int mDamageStringPos;
	std::vector<short> mStatUpdate;

	static const int INTEGRAL_FRACTION_TOTAL = 1000;  //Skills often use integers to represent percentage values. (10 = 1%).  This allows functions to compute a (float) ratio with the given integer amount.
	static const int INNATE_BLOCK_CHANCE = 50;        //All players begin with this base chance to block physical attacks (10 = 1%)
	static const int INNATE_PARRY_CHANCE = 50;        //All players begin with this base chance to parry physical attack (10 = 1%)
	static const int CRITICAL_NORMAL = 1;
	static const int CRITICAL_SUPER = 2;
	static const int MAX_MIGHT_WILL = 10;
	static const int MAX_CHARGES = 5;

	static const int NO_RETURN_VALUE = 0;             //A more descriptive way to represent that return values from action functions are not intended to be used.

	//These internal functions provide additional sub-processing and calculations for certain functions,
	//such as damage calculation.
	void ApplyBlockChance(int &amount);
	void ApplyParryChance(int &amount);
	void ApplyDodgeChance(int &amount);
	void ApplyHitChances(int &amount, int damageType);
	void CheckCritical(int &amount, int damageType);
	void AddDamageString(int damageType, int damageAmount);
	int ResolveBuffCategoryID(const char *buffName);
	void RegisterImplicitAction(int implicitActionType, int damage);
};

class AbilityManager2
{
public:
	AbilityManager2();
	~AbilityManager2();

	void InitFunctionTables(void);
	void LoadData(void);
	void DebugPrint(void);
	void DebugStuff(void);
	void Verify(void);
	void DamageTest(CreatureInstance *playerData);
	bool VerifyFunctionName(const std::string &functionName);
	void VerifyFunctionArgument(const std::string &functionName, size_t argIndex, const std::string &argumentString, AbilityVerify &verifyInfo);

	int EnumerateTargets(CreatureInstance *actor, int targetType, int targetFilter, int distance);
	int ActivateAbility(CreatureInstance *cInst, short abilityID, int eventType, ActiveAbilityInfo *abInfo);
	int ActivateImplicit(CreatureInstance *cInst, CreatureInstance *target, short abilityID, int eventType);
	void RunImplicitActions(void);
	int CheckActivateSpecialAbility(CreatureInstance *cInst, short abilityID, int eventType);

	static const int REQUIRED_ROW_ENTRIES = 28;
	static const int NON_PURCHASE_ID_THRESHOLD = 5000;

	//Special case function needs to be called by the formula evaluation function.
	bool CheckValidVariableName(const std::string &token);
	double ResolveSymbol(const std::string &symbol);
	int ResolveBuffCategoryID(const char *buffName);
	const char* ResolveBuffCategoryName(int buffCategoryID);
	int ResolveCooldownCategoryID(const char *cooldownName);
	const char* ResolveCooldownCategoryName(int cooldownCategoryID);
	int ResolveStatID(const char *statName);
	int ResolveStatusEffectID(const char *statusEffectName);
	int ResolveItemID(const char *itemID);

	static int GetAbilityErrorCode(int value);
	static int GetAbilityErrorParameter(int value);
	static int CreateAbilityError(int errorCode, int parameter);
	static bool IsGlobalIntrinsicAbility(int abilityID);
	const AbilityEntry2* GetAbilityPtrByID(int abilityID);

	void GetCooldownCategoryStrings(STRINGLIST &output);
	const char* GetAbilityNameByID(int abilityID);
	int GetAbilityIDByName(const char *name);

private:

	typedef AbilityFunction2& ARGUMENT_LIST;   //The type of the argument list passed to ability function processing.
	typedef int (AbilityCalculator::*FunctionPtr)(ARGUMENT_LIST);

	 //Stores all ability definitions.
	typedef std::map<int, AbilityEntry2>::iterator ABILITY_ITERATOR;
	std::map<int, AbilityEntry2> mAbilityIndex;
	std::map<std::string, int> mAbilityStringIndex;
	void InsertAbility(int abilityID, const STRINGLIST &rowData);
	void LoadAbilityTable(const char *filename);

	//Stores all scripted emulation functions.
	typedef std::map<std::string, FunctionPtr>::iterator FUNCTION_ITERATOR;
	std::map<std::string, FunctionPtr> mFunctionMap;
	void InsertFunction(const char *name, FunctionPtr function);

	//Stores argument verification information for scripted emulated functions.
	typedef std::map<std::string, ABVerifier>::iterator VERIFIER_ITERATOR;
	std::map<std::string, ABVerifier> mVerifierMap;
	void InsertVerifier(const char *functionName, const ABVerifier &argInfo);

	bool mFunctionTablesLoaded;

	typedef std::map<std::string, int> CONSTANT_MAP;
	CONSTANT_MAP mBuffCategories;
	CONSTANT_MAP mCooldownCategories;
	void InitializeCategories(void);
	void StatNameLowercase(void);

	AbilityCalculator abProcessing;

	static const int ABILITY_CODE_BITMASK = 0xFF;  //Bitmask to extract the error code directly from the value.
	static const int ABILITY_CODE_BITCOUNT = 8;    //Number of bits to shift value to return the parameter itself.
};


//This is a data class intended solely for the UniqueAbilityList class below. 
struct UniqueAbility
{
	int mID;
	int mGroupID;
	int mLevel;
	UniqueAbility(int ID, int groupID, int level);
};

//This class is a helper class to maintain a running list of the highest ability group
//of skills that are passed to it, for sorting out the highest level abilities.  Helpful
//for identifying which passives to apply to the character.
class UniqueAbilityList
{
public:
	std::vector<UniqueAbility> mAbilityList;
	void AddAbilityToList(const AbilityEntry2 *abilityPtr);
};

} //namespace Ability2

extern Ability2::AbilityManager2 g_AbilityManager;


#endif //#ifndef ABILITY2_H
