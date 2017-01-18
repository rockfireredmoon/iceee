#pragma once
#ifndef CREATURE_H
#define CREATURE_H

#include <vector>
#include <map>
#include <string>
#include <stack>

using namespace std;

//Need for pointers.
class HateProfile;
class CharacterData;
struct ActiveSpawner;
class SpawnTile;
class SimulatorThread;

#include "Stats.h"
#include "AbilityTime.h"
#include "Arena.h"
#include "DropTable.h"
#include "../squirrel/sqrat/sqrat.h"
#include "Daily.h"
#include "Globals.h"

class CreatureInstance;  //Forward declaration for a pointer in the SelectedObject structure
class AIScriptPlayer;    //Forward declaration for AI script
class AINutPlayer;    //Forward declaration for AI nuts
struct QuestObjective;   //Need for integration with quest system.
class InteractObject;
class QuestItemReward;
class InstanceScaleProfile;
class ReportBuffer;

extern const int CREATURE_WALK_SPEED;
extern const int CREATURE_JOG_SPEED;
extern const int CREATURE_RUN_SPEED;
extern const int AGGRO_ELEV_RANGE;
extern const int AGGRO_DEFAULT_RANGE;
extern const int AGGRO_MAXIMUM_RANGE;
extern const int AGGRO_MINIMUM_RANGE;
extern const int AGGRO_LEVEL_MOD;

extern const double Cone_180;
extern const double Cone_90;

const float DEFAULT_CREATURE_SPEED = 50.0F;      //Should match the 'gDefaultCreatureSpeed' value in the client.  Number of units a creature moves per second.

const int MOVEMENT_THRESHOLD = 30;

//Helps determines where a buff came from.  This assists any function that may need to
//add, update, or erase buffs from a particular source.
struct BuffSource
{
	enum
	{
		ABILITY  = 0,   //Buff was granted by an ability activation.
		ITEM     = 1,   //Buff was granted by an equipped item.  Used for stats on gear like +Strength.
		INSTANCE = 2    //Buff was granted as an instance.
	};
};

struct ServerFlags
{
	enum Enum
	{
		LocalActive       = (1 << 0),  //Creature is within range of at least one simulator and should be considered for iterative processing like target scanning and ability use.
		//Removed unused
		IsPlayer          = (1 << 2),  //This object is a simulator player.  charPtr and simulatorPtr are valid fields.
		IsNPC             = (1 << 3),  //This object is an NPC/creature.  spawnGen and spawnTile are valid fields.
		IsSidekick        = (1 << 4),  //This object is a sidekick.
		InitAttack        = (1 << 5),  //For a player character, indicates that attached sidekicks need to be unleashed on the target.
		AutoTarget        = (1 << 6),  //For sidekicks, indicates that this creature will auto acquire targets without officer directions.
		CalledBack        = (1 << 7),  //For sidekicks, indicates this creature is being called back to its officer and therefore shouldn't engage in attack.
		
		DeadTransform     = (1 << 8),  //For creatures, indicates that a custom appearance has been assigned to a dead creature.
		IsUnusable        = (1 << 9),  //For NPC creatures, indicates that players cannot use this object for interactions (intended when custom behavior is necessary, like quest scripts)
		PathNode          = (1 << 10),  //For NPCs, signals that the movement is currently following a path node.
		LeashRecall       = (1 << 11),  //The creature is heading back to its leash location and cannot be instigated into a fight.
		IsTransformed     = (1 << 12),  //The object's appearance has been temporarily transformed.
		NeutralInactive   = (1 << 13),  //The object is considered neutral and won't auto target creatures.
		Stationary        = (1 << 14),  //The object will not acquire targets or move.
		HasMeleeWeapon    = (1 << 15),  //The player has a melee weapon equipped.

		HasShield         = (1 << 16),  //The player has a shield equipped.
		TriggerDelete     = (1 << 17),  //The object is triggered for deletion, once the current time exceeds the deathTime.
		HateInfoChanged   = (1 << 18),  //For NPCs, indicates the hate time has changed.
		InvisDistChanged  = (1 << 19),  //Invisibility distance has changed and needs to be resent at a reasonable update time.
		Noncombatant      = (1 << 20),  //This prevents acquisition of targets, and being targeted.
		Taunted           = (1 << 21),  //Indicates that the creature is taunted and should attempt to enter melee range.
		ScriptMovement    = (1 << 22),  //This creature has been granted arbitrary movement by a script.  It bends the rules somewhat when dealing with leash and wander radius.
		ScriptNoLinkLoyal = (1 << 23),  //This creature has been specifically disabled from

		// NOTE: if any more are added, check to make sure the following
		// variable and function support the required number of bits.
		//  CreatureInstance::serverFlags 
		//  CreatureInstance::SetServerFlag()
	};
};

struct ActiveAbilityInfo
{
	//int DEBUG_SENTINAL_ARRAY[32];
	//int DEBUG_SENTINAL;
	short abilityID;         //Ability being casted
	char type;               //Casting type (AbilityType::)
	char iterations;         //Iterations remaining
	short iterationDur;      //Total duration that a channel is supposed to run.
	short iterationInt;      //Interval between channel interations.
	unsigned long fireTime;  //Event will trigger at this moment (usually set to current g_ServerTime + Offset)
	bool bPending;           //If true, an ability is being processed in some manner.
	//bool bSecondary;         //If true, the outgoing packet data sends the target list as secondary targets
	bool bUnbreakableChannel;  //For special channels only, indicates casting may not be broken by incoming attacks.
	bool bParallel;           //If true, this is an independently parallel operation (like bleeding damage over time) that might have special freedoms, like continuing operation even if the caster is stunned.
	bool bSecondary;          //If true, the ability data sent to the client should write the target details in the secondary target list.
	short interruptChanceMod;    //Additional chance to interrupt, solely for this ability.
	float x;
	float y;
	float z;
	unsigned long duration;	  // Used when ability activated from a saved session

	char willChargesSpent;   //Charges spent using the ability.  Notifies the client to assist with charge-based animations.  Should not be used for calculations.
	char mightChargesSpent;  //Charges spent using the ability.  Notifies the client to assist with charge-based animations.  Should not be used for calculations.
	
	//Amount to adjust resources when processing ability conditions and actions.
	//Used internally by the server.
	char willAdjust;
	char mightAdjust;
	char willChargeAdjust;
	char mightChargeAdjust;
	bool bResourcesSpent;    //If true, will/might and charges have not been spent yet.

	int TargetCount;
	CreatureInstance *TargetList[MAXTARGET];  //List of targets
	void Duration(int seconds);
	void Clear(const char *debugCaller);
	void TransferTargetList(ActiveAbilityInfo *from);
	void ClearTargetList(void);
	void SetPosition(float newx, float newy, float newz);
	void SetPosition(int newx, int newy, int newz);
	bool IsBusy(void);
};

struct SelectedObject
{
	CreatureInstance *targ;   //Pointer to creature instance
	//CreatureInstance *aaTarg; //Pointer to the creature that the auto-attack is initiated for.
	short desiredRange;  //If the AI is out of range of its target, this is the range it must proceed within
	bool bInstigated;         //If true, this entity is hostile because it was attacked by a player
	int DesLocX;              //For the new movement system, the desired X location to move.
	int DesLocZ;              //For the new movement system, the desired Z location to move.
	int desiredSpeed;		  //For scripting to specify a desired speed
	SelectedObject();
	void Clear(bool UNUSED_eraseAutoAttack);
	bool hasTargetCDef(int CDefID);
};

//This class contains all the visual appearance of a single creature.
class CreatureDefinition
{
public:
	CreatureDefinition();
	~CreatureDefinition();
	void Clear();
	void CopyFrom(CreatureDefinition& source);

	CharacterStatSet css;
	int CreatureDefID;             //Creature Definition ID
	unsigned short DefHints;       //extra creature data
	std::string ExtraData;
	std::vector<int>DefaultEffects;
	
	bool operator < (const CreatureDefinition& other) const;
	bool IsNamedMob(void) const;
	float GetDropRateMult(void) const;
	void SaveToStream(FILE *output);
};

class ActiveInstance;  //Forward declaration is required

struct ActiveStatMod
{
	int priority;          //Notifies the client how to interpret this value.  1 for float, 0 for int.
	short modStatID;       //The stat ID that this is modifier is affecting.
	short abilityID;
	short abilityGroupID;

	float amount;        //When tallied, they may be converted back into (int) if needed.
	float clientAmount;  //Similar to amount, but holds the value it was derived from so the client can display the correct modifier.  For example, Add(STRENGTH,3,3600) = 3, Amp(STRENGTH,0.05,3600) = 0.05

	unsigned long expireTime;  //Must be converted to seconds remaining when sending to the client.

	//These are internally used by the server for managing buffs
	unsigned char tier;
	unsigned char buffCategory;

	char sourceType;   //Keeps track of where this mod was added from (see BuffSource)
	int sourceID;      //The Ability ID or Item ID, depending on what sourceType is.

	ActiveStatMod()
	{
		priority = 0;
		modStatID = 0;
		abilityID = 0;
		abilityGroupID = 0;

		expireTime = 0;

		tier = 0;
		buffCategory = 0;

		sourceType = 0;
		sourceID = 0;

		amount = 0.0F;
		clientAmount = 0.0F;
	}
};

struct ActiveStatusEffect
{
	short modStatusID;         //Status ID to be modified.
	unsigned long startTime;   //Time that the status effect was initialized.
	unsigned long expireTime;  //Time that the status effect will be removed
	ActiveStatusEffect()
	{
		modStatusID = 0;
		startTime = 0;
		expireTime = 0;
	}
	ActiveStatusEffect(short statusID, unsigned long start, unsigned long expire)
	{
		modStatusID = statusID;
		startTime = start;
		expireTime = expire;
	}
};

struct BaseStatData
{
	short StatID;       //Stat ID this modified
	float fBaseVal;     //Preserved base value of the stat before any modifications are made.
	float fModTotal;    //Total value of mods, so that BaseVal + ModTotal = Current Stat Value 
	char RefCount;      //Number of objects adding this stat.  When buffs are removed and RefCount equals zero, the object is destroyed.
	char valueType;     //
	BaseStatData()
	{
		valueType = 0;
		StatID = 0;
		fBaseVal = 0;
		fModTotal = 0;
		RefCount = 0;
	}
};

struct CreatureSearch
{
	CreatureInstance *ptr;
	int attacked;
	CreatureSearch() { ptr = NULL, attacked = 0; }
};

struct ImplicitAction
{
	short eventType;      //Action type.
	short abilityID;       //Ability that this ID is registered by
	short abilityGroup;    //Ability that this ID is registered by
};

//Stacks of appearance transforms manage the current appearance after any transformations have been applied
class AppearanceModifier
{
public:
	virtual ~AppearanceModifier();
	virtual std::string Modify(std::string source) =0;
	virtual std::string ModifyEq(std::string source) =0;
	virtual AppearanceModifier * Clone() =0;
};

class AbstractAppearanceModifier : public AppearanceModifier {
public:
	AbstractAppearanceModifier();
	virtual ~AbstractAppearanceModifier();
	std::string Modify(std::string source);
	std::string ModifyEq(std::string source);
	virtual void ProcessTable(Sqrat::Table *table) =0;
	virtual void ProcessTableEq(Sqrat::Table *table) =0;
	virtual AppearanceModifier * Clone() =0;
private:
	std::string DoModify(std::string source, bool eq);
};

class ReplaceAppearanceModifier : public AppearanceModifier {
public:
	std::string mReplacement;
	ReplaceAppearanceModifier(std::string replacement);
	~ReplaceAppearanceModifier();
	std::string Modify(std::string source);
	std::string ModifyEq(std::string source);
	AppearanceModifier * Clone();
};

class NudifyAppearanceModifier : public AppearanceModifier {
public:
	NudifyAppearanceModifier();
	~NudifyAppearanceModifier();
	std::string Modify(std::string source);
	std::string ModifyEq(std::string source);
	AppearanceModifier * Clone();
};


class CreatureAttributeModifier : public AbstractAppearanceModifier {
public:
	std::string mAttribute;
	std::string mValue;
	CreatureAttributeModifier(std::string attribute,std::string value);
	~CreatureAttributeModifier();
	void ProcessTable(Sqrat::Table *table);
	void ProcessTableEq(Sqrat::Table *table);
	AppearanceModifier * Clone();
};

class AddAttachmentModifier : public AbstractAppearanceModifier {
public:
	std::string mType;
	std::string mNode;
	AddAttachmentModifier(std::string type,std::string node);
	~AddAttachmentModifier();
	void ProcessTable(Sqrat::Table *table);
	void ProcessTableEq(Sqrat::Table *table);
	AppearanceModifier * Clone();
};

class CreatureInstance
{
public:
	CreatureInstance();
	~CreatureInstance();
	typedef std::vector<CreatureSearch> CREATURE_SEARCH;
	typedef std::vector<CreatureInstance*> CREATURE_PTR_SEARCH;
	void UnloadResources(void);
	bool KillAI(void);
	bool StartAI(std::string &errors);
	void RemoveAttachedHateProfile(void);

	int CreatureDefID;   //Associated Creature Def ID
	int CreatureID;      //Creature Instance ID

//	union
//	{
		ActiveSpawner *spawnGen;   //For NPCs, the pointer of the spawner object that generated this creature
		CharacterData *charPtr;  //For Players, the pointer back the character definition
//	};
//
//	union
//	{
		SpawnTile *spawnTile;    //For NPCs, spawntile that this object is linked to.
		SimulatorThread *simulatorPtr; //For players, a pointer back to the simulator that is controlling this object.
//	};

	ActiveInstance *actInst;

	unsigned int statusEffects[MAX_STATUSEFFECTBYTES];

	char ZoneString[40];
	int CurrentX;           //Position on map (horizontal position on grid)
	int CurrentY;           //Position on map (elevation)
	int CurrentZ;           //Position on map (vertical position on grid)
	CreatureInstance *AnchorObject;  //For sidekicks, this is the host player they follow.

	unsigned char Heading;
	unsigned char Rotation;
	unsigned char Speed;

	short MainDamage[2];
	short OffhandDamage[2];
	short RangedDamage[2];
	char EquippedWeapons[3]; //Holds the weapon type in each of the three slots [0] = main, [1] = offhand, [2] = ranged

	char Faction;
	int PartyID;
	unsigned long serverFlags;
	int transformCreatureId;
	int transformAbilityId;
	AppearanceModifier *transformModifier;

	SelectedObject CurrentTarget;

	AIScriptPlayer *aiScript;
	AINutPlayer *aiNut;
	long scriptMoveEvent;

	union
	{
		unsigned long movementTime;  //Estimated movement time to reach target.
		unsigned long deathTime;     //If dead, this tracks the last update time.
	};
	
	unsigned long LastCharge;  //Keeps track of the last charge time, so that might/will charges can reduce over time
	unsigned long timer_autoattack;   //Time when next autoattack will fire
	unsigned long timer_mightregen;   //Time when next might regen tick will fire
	unsigned long timer_willregen;    //Time when next will regen tick will fire
	unsigned long timer_lasthealthupdate; //Time when the next health regen tick will fire

	HateProfile *hateProfilePtr;
	int activeLootID;

	CharacterStatSet css;   //Contains the active character stat set
	vector<ActiveStatMod> activeStatMod;
	vector<ActiveStatusEffect> activeStatusEffect;
	vector<ImplicitAction> implicitActions;

	std::vector<AppearanceModifier*> appearanceModifiers;

	ActiveBuffManager buffManager;
	ActiveCooldownManager cooldownManager;

	int LastUseDefID;            //Used for players, necessary for communicating script control with instances.

	vector<BaseStatData> baseStats;  //Base stats.  New entries are saved to this list when a unique Add() is made.

	bool swimming; // Whether the player is swimming

	ActiveBuff * AddMod(unsigned char tier, int buffCategory, short abID, short abgID, double durationSec);
	void AttachItem(const char *type, const char *node);
	void DetachItem(const char *type, const char *node);
	void PushAppearanceModifier(AppearanceModifier *modifier);
	void RemoveAppearanceModifier(AppearanceModifier *modifier);
	void ClearAppearanceModifiers();
	std::string PeekAppearanceEq();
	std::string PeekAppearance();
	int _GetBaseStatModIndex(int statID);
	int _GetExistingModIndex(int type, int groupID, int statID);
	void SendUpdatedBuffs(void);
	void RemoveStatModsBySource(int buffSource);
	void AddItemStatMod(int itemID, int statID, float amount);
	void ApplyItemStatModFromConfig(int itemID, const std::string configStr);
	void SubtractAbilityBuffStat(int statID, int abgID, float amount);
	void CheckRemovedBuffs(void);
	bool IsAtTether();
	void AddBaseStatMod(int statID, float amount);
	void SubtractBaseStatMod(int statID, float amount);
	void UpdateBaseStatMinimum(int statID, float amount);
	void RemoveBuffsFromAbility(int abilityID, bool send);
	void RemoveBuffIndex(size_t index);
	void RemoveAllBuffs(bool send);
	void RemoveAllBuffsExceptGroupID(bool send, int abilityGroupID);
	void PushStatMod(ActiveStatMod &mod);
	void RemoveAbilityBuffTypeExcept(int buffCategory, int abilityID, int abilityGroupID);
	bool RemoveAbilityBuffWithStat(int statID, float sign);
	void ActivateSavedAbilities(void);

	void Clear(void);         //Clear all data
	void CopyFrom(CreatureInstance *source);
	void CopyBuffsFrom(CreatureInstance *source);
	void Instantiate(void);   //A central way of initializing common data like health, might/will, etc.
	void RemoveFromSpawner(bool wasKilled);

	void RemoveNoncombatantStatus(const char *debugCaller);
	int GetKillExperience(int attackerLevel);
	void AddValour(int amount, int guildDefId);
	void AddCredits(int amount);
	void AddExperience(int amount);
	void AddHeroism(int amount);
	void AddHeroismForQuest(int amount, int questLevel);
	void AddHeroismForKill(int targetLevel, int targetRarity);
	void OnHeroismChange(void);
	void SendStatUpdate(int statID);
	void SetLevel(int newLevel);
	void CheckQuestKill(CreatureInstance *target);
	void CheckQuestInteract(int CreatureDefID);
	int ProcessQuestRewards(int QuestID, const std::vector<QuestItemReward>& itemsToGive);
	int QuestInteractObject(char *buffer, const char *text, float time, bool gather);
	int NormalInteractObject(char *outBuf, InteractObject *interactObj);
	void RunQuestObjectInteraction(CreatureInstance *target, bool deleteObject);
	void RunObjectInteraction(CreatureInstance *target);

	void SendUpdatedLoot(void);
	float GetDropRateMultiplier(CreatureDefinition *cdef);

	void PlayerLoot(int level, std::vector<DailyProfile> profiles);
	void CreateLoot(int finderLevel);
	void AddLootableID(int newLootableID);

	void ApplyRawDamage(int amount);
	void CheckFallDamage(int elevation);

	bool IsValidForPVP(void);

	//These functions operate on "activeStatusEffect"
	int _HasStatusList(int statusID);
	void _AddStatusList(int statusID, long msDuration);
	void _RemoveStatusList(int statusID);
	void CheckActiveStatusEffects(void);
	int GetStatDurationSec(int index);
	
	//Active ability processing
	ActiveAbilityInfo ab[2];
	char setbackCount;
	bool bOrbChanged;  //Flag that determines whether might/will or their charges need to be updated.

	int previousPathNode;   //If nonzero, this is the ID of the spawnpoint that was last used.
	int nextPathNode;       //If nonzero, the target ID of the next path node.
	int lastIdleX;          //If aggro, this is the previous idle location.
	int lastIdleZ;          //If aggro, this is the previous idle location.
	int tetherNodeX;        //The anchor point of the current tether location.
	int tetherNodeZ;        //The anchor point of the current tether location.
	int tetherFacing;		//The direction to face when tethered

	bool initialisingAbilities; // Set to true when persistent abilities are being set up

	void CheckPathLocation(void);
	void CheckLeashMovement(void);

	bool CanPVPTarget(CreatureInstance *target);
	bool _ValidTargetFlag(CreatureInstance *compare, int abilityRestrict);  //Return true if the creature is a valid target with the given faction and targeting flags.
	int _AddTargetsInCone(double halfAngle, int targetType, int distance, int abilityRestrict);  //Fill target list with valid targets within the required range and distance
	bool isTargetInCone(CreatureInstance * target, double amount);
	void RegisterHostility(CreatureInstance *attacker, int hostility);
	void SetCombatStatus(void);
	void Untransform(void);
	int GetOrbRegenerationTime(float regenFactor);
	void CheckUpdateTimers(void);
	void CheckLootTimers(void);

	void CancelPending(void);
	void CancelPending_Ex(ActiveAbilityInfo *ability);

	void OverrideCurrentAbility(int abilityID);
	void RegisterInstant(int abilityID);
	void RegisterChannel(int abilityID, int duration, int interval, bool bOverride = false, int interruptResist = 0);  //Initializes the activation of a channel ability.
	void RegisterCast(int abilityID, int warmupTime);                 //Initializes the activation of timed cast ability.

	void RegisterImplicit(int eventType, int abID, int abGroup);

	int CallAbilityEvent(int abilityID, int eventType);
	int RequestAbilityActivation(int abilityID);
	void ProcessAbility_Ex(ActiveAbilityInfo *ability);
	void RunActiveAbilities(void);  //Called by the main thread to run ability processing for this entity

	void AddTargetsST(CREATURE_PTR_SEARCH &results, int abilityRestrict);
	void AddTargetsConeAngle(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance, double halfAngle);
	void AddTargetsGTAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance);
	void AddTargetsPBAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance);
	void AddTargetsSTAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance);
	void AddTargetsSTXAE(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance);
	void AddTargetsParty(CREATURE_PTR_SEARCH &results, int abilityRestrict, int distance);
	void AddTargetsSTP(CREATURE_PTR_SEARCH &results, int abilityRestrict);

	int TransferTargets(const CREATURE_PTR_SEARCH &results, ActiveAbilityInfo &ab);

	bool HasCooldown(int cooldownCategory);
	void RegisterCooldown(int cooldownCategory, int duration);

	void ForceAbilityActivate(int abilityID, int abilityEvent, int targetID);  //For effects
	void SimulateEffect(const char *effect, CreatureInstance *target);  //For effects, simpler and better forced ability activation.
	void SendAutoAttack(int abilityID, int targetID);

	//Utility functions
	int CalcRestrictedHealth(int health, bool addmod);
	int GetMaxHealth(bool addmod);
	void LimitValueOverflows(void);
	float GetHealthRatio(void);
	void RunHealTick(void);
	void _SetStatusFlag(int statusID);
	void _ClearStatusFlag(int statusID);
	void SendFlags(void);
	int _CountStatusFlags(void);
	void _OnDeath(void);
	void _OnStun(void);
	void OnDaze(void);
	void OnUnstick(void);
	void ProcessPVPGoal(void);
	void PrepareDeath(void);
	void ProcessDeath(void);
	void AddCreaturePointer(CREATURE_SEARCH& output, CreatureInstance* ptr, int attacked);
	void ResolveAttackers(CREATURE_SEARCH& results);
	int GetHighestLevel(CREATURE_SEARCH& creatureList);
	void _OnModChanged(void);
	void _UpdateHealthMod(void);
	void UpdateAggroPlayers(short state);
	float _GetBaseStat(int statID);
	float _GetTotalStat(int statID, int abilityGroup);
	void SetAutoAttack(CreatureInstance *target, int statusID);
	void RequestTarget(int CreatureID);
	void SelectTarget(CreatureInstance *newTarget);
	void SetServerFlag(unsigned long flag, bool status);
	void AICheckAbilityFailure(int abilityReturnInfo);
	bool AIAbilityFailureAllowRetry(int abilityReturnInfo);
	bool AICheckIfAbilityBusy(void);
	int AIFillCreaturesNear(int range, float x, float z, int playerAbilityRestrict, int npcAbilityRestrict, int sidekickAbilityRestrict, CREATURE_PTR_SEARCH& creatures);
	int AICountEnemyNear(int range, float x, float z);
	int AIGetIdleMob(int creatureDefID);
	void AIOtherSetTarget(int creatureID, int creatureIDTarget);
	void AIOtherCallLabel(int creatureID, const char *aiScriptLabel);
	bool AIIsTargetEnemy(void);
	bool AIIsTargetFriend(void);
	float AIGetProperty(const char *propName, bool useTarget);
	int AIGetBuffTier(int abilityGroupID, bool useTarget);
	int AIGetTargetRange(void);
	void AIDispelTargetProperty(const char *propName, int sign);
	void AISetGTAE(void);
	void AISetGTAETo(int x, int y, int z);

	void SendEffect(const char *effectName, int targetCID);
	void SendSay(const char *message);
	void SendPlaySound(const char *assetPackage, const char *soundFile);

	//Internal processing
	int RemoveCreatureReference(CreatureInstance *target);
	void RunAIScript(void);
	int RotateToTarget(void);
	int RunMovementStep(void);
	void UpdateDestination(void);
	void MoveToTarget_Ex2(void);
	void StopMovement(int result);
	void MoveTo(int x, int z, int range, int speed);
	void ProcessRegen(void);
	void ProcessAutoAttack(void);
	bool isNPCReadyMovement(void);
	void RunProcessingCycle(void);
	void CheckMostHatedTarget(bool forceCheck);
	void RunAutoTargetSelection(void);
	float GetMaxInvisibilityDistance(bool dexBased);
	float GetRegenInvisibilityDistance(bool dexBased);
	bool IsCombatReactive(void);
	int GetDistance(CreatureInstance *object, int threshold);

	//Skill activation requirements.  These functions all return true if the
	//condition passes
	bool NotSilenced(void);
	bool Facing(bool useGroundLocation);
	bool CheckBuffLimits(int level, int buffCategory, bool addBuff = false);
	bool InRange(float dist, bool useGroundLocation);
	bool InRange_Target(float dist);
	bool Will(int amount);
	bool Might(int amount);
	int WillCharge(int min, int max);
	int MightCharge(int min, int max);

	//This batch of functions assists skill emulation by accessing and
	//manipulating charges, but are not actual functions native to the ability
	//table.
	int AdjustWill(int amount);
	int AdjustMight(int amount);
	int AddWillCharge(int amount);
	int AddMightCharge(int amount);

	HateProfile* GetHateProfile(void);
	int GetMitigatedDamage(int damageAmount, int armorRating, int reductionMod);
	int GetReducedDamage(int armorRating, int damageAmount);
	int GetResistedDamage(int resistRating, int damageAmount);
	int GetAdditiveWeaponSpecialization(int amount, int itemEquipSlot);
	int GetAdjustedDamage(CreatureInstance *attacker, int damageType, int amount);
	int ApplyPostDamageModifiers(CreatureInstance *attacker, int amount, int &absorbedDamage);
	void OnApplyDamage(CreatureInstance *attacker, int amount);
	void CheckStatusInterruptOnHit(void);
	void CancelInvisibility(void);
	void CheckInterrupts(void);
	void CheckMovement(float movementStep);
	void CheckMovementInterrupt(void);
	void CastSetback(void);

	int Status(int statusID, float durationSec);
	bool NotStatus(int statusID);
	bool HasStatus(int statusID);
	
	bool Reagent(int itemID, int amount);
	int Add(unsigned char tier, unsigned char buffCategory, int abID, int abgID, int statID, float calcAmount, float descAmount, float durationSec);
	void Heal(int amount);
	void RestrictHealth();
	bool hasOffHandWeapon(void);
	bool hasMeleeWeapon(void);
	bool hasShield(void);
	bool hasBow(void);
	bool has2HorPoleWeapon(void);

	void AddHate(CreatureInstance *attacker, int amount);
	void Taunt(CreatureInstance *attacker, int seconds);
	void Amp(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, float percent, int time);
	void Set(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, float amount, int time);
	void WalkInShadows(int duration, int counter);

	void GoSanctuary(void);
	void RemoveAltBuff(void);
	void RemoveEtcDebuff(void);
	void RemoveStatDebuff(void);
	void RemoveAltDebuff(void);

	bool CanUseAnyWeapon(void);
	bool hasMainHandWeapon(void);
	void Go(const char *destname);
	void PortalRequest(CreatureInstance *caster, const char *internalname, const char *externalname);
	void BindTranslocate(void);
	void Resurrect(float healthratio, float luckratio, int abilityID);
	void TopHated(int amount);
	bool StatValueLessThan(int statID, int amount);
	void Invisible(int duration, int counter, int unknown);
	int MWD(void);
	int RWD(void);
	int WMD(void);

	void AddWDesc(int statID, int unknown1, int unknown2, const char *name);
	void Duration(int amount);
	void Harm(int amount);
	bool Behind(void);
	void UnHate(void);
	void LeaveCombat(void);

	void Nullify(unsigned char tier, unsigned char buffType, int abID, int abgID, int statID, int duration);
	void AmpCore(unsigned char tier, unsigned char buffType, int abID, int abgID, float ratio, int duration);
	bool hasWand(void);
	void Interrupt(void);
	void Spin(void);
	void RemoveStatBuff(void);
	void RemoveEtcBuff(void);
	void RemoveHealthBuff(void);

	bool NearbySanctuary();
	bool Translocate(bool test);
	void FullMight(void);
	void FullWill(void);

	bool PercentMaxHealth(double ratio);
	void HealthSacrifice(int amount);

	void Regenerate(int amount);
	void DoNothing(void);
	//void AttackMelee(int unknown);
	//void AttackRanged(int unknown);
	void SummonPet(int unknown);

	void Jump(void);
	bool hasRangedWeapon(void);
	void AdjustCopper(int coinChange);
	int GetAggroRange(CreatureInstance *target);
	void NotifySuperCrit(int TargetCreatureID);
	int GetMainhandDamage(void);
	int GetOffhandDamage(void);
	void Respec(void);
	bool ValidateEquippableWeapon(int mWeaponType);
	void OnEquipmentChange(float oldHealthRatio);
	int GetLevelScaledValue(int originalAmount, int targetLevel, float multPerLevel);
	int GetScaledValue(int originalAmount, float additionalMult);
	//void PerformLevelScale(int targetLevel, float vitalMult, float weaponMult);
	void PerformLevelScale(const InstanceScaleProfile *scaleProfile, int targetLevel);
	void BuildZoneString(int instanceID, int zoneID, int unknownID);

	void OnInstanceEnter(const ArenaRuleset &arenaRuleset);
	void OnInstanceExit(void);
	void ApplyGlobalInstanceBuffs(void);


	//Custom Ability Functions, called through the ability system to handle special operations
	bool IsTransformed();
	bool CAF_Nudify(int durationS);
	bool CAF_Transform(int CDefID, int abId, int durationS);
	bool CAF_Untransform();
	int CAF_SummonSidekick(int CDefID, int maxSummon, short abGroupID);
	void CAF_RunSidekickStatFilter(int abGroupID);
	int CAF_RegisterTargetSidekick(int abGroupID);
	void StatScaleToLevel(int statID, int targetLevel);

	void BroadcastUpdateElevationSelf(void);
	void BroadcastLocal(const char *buffer, int dataLen, int simulatorFilter = -1);
	float GetTotalSize(void);
	bool IsObjectInRange(CreatureInstance *object, float distance);
	bool IsSelfNearPoint(float x, float z, float distance);
	void DebugGenerateReport(ReportBuffer &report);
private:
	void _LimitAdjust(short &value, int add, int min, int max);
	void AddBuff(int buffSource, int buffCategory, int abTier, int abID, int abGroupID, int statID, float calcAmount, float descAmount, float durationSec);
};

//This class manages creature definitions for player characters and NPCs.
class CreatureDefManager
{
public:
	CreatureDefManager();
	~CreatureDefManager();

	typedef std::map<int, CreatureDefinition> CREATURE_MAP;
	typedef std::pair<int, CreatureDefinition> CREATURE_PAIR;

	CREATURE_MAP NPC;

	void SaveCreatureTweak(CreatureDefinition *def);
	void AddNPCDef(CreatureDefinition &newItem);
	CreatureDefinition* GetPointerByCDef(int CDefID);
	CreatureDefinition* GetPointerByName(const char *name);

	int LoadPackages(const char *listFile);
	int LoadFile(const char *filename);

	int GetSpawnList(const char *searchType, const char *searchStr, vector<int> &resultList);

	void Clear(void);                 //Clear all data
private:
	const char * GetIndividualFilename(char *buffer, int bufsize, int accountID);
};

extern CreatureDefManager CreatureDef;

struct PendingCreatureUpdate
{
	unsigned short mask;
	CreatureInstance *object;
	std::vector<short> statUpdate;
	void addStatUpdate(short statID);
};

class PendingOperation
{
public:
	PendingOperation();
	~PendingOperation();

	vector<CreatureInstance*> DeathList;
	vector<PendingCreatureUpdate> UpdateList;
	void DeathList_Add(CreatureInstance *obj);
	void DeathList_Process(void);
	void DeathList_Remove(CreatureInstance *obj);

	int UpdateList_GetExistingObject(CreatureInstance *obj);
	void UpdateList_Add(unsigned short mask, CreatureInstance *obj, short statID);
	void UpdateList_Process(void);
	void UpdateList_Remove(CreatureInstance *obj);
	void Free(void);
	bool Debug_HasCreature(CreatureInstance *obj);
};

extern PendingOperation pendingOperations;

int PrepExt_CreatureDef(char *buffer, CreatureDefinition *cdef);
int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, vector<short> &statList);
int PrepExt_SendSpecificStats(char *buffer, CreatureInstance *cInst, const short *statList, int statCount);
int PrepExt_UpdateMods(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateOrbs(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateAppearance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreatureInstance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreatureFullInstance(char *buffer, CreatureInstance *cInst);
int PrepExt_CreaturePos(char *buffer, CreatureInstance *cInst);
int PrepExt_VelocityEvent(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateVelocity(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdatePosInc(char *buffer, CreatureInstance *cInst);
int PrepExt_GeneralMoveUpdate(char *buffer, CreatureInstance *cInst);  //General server movement update (combines 3 flags of data)
int PrepExt_UpdateElevation(char *buffer, CreatureInstance *cInst);
int PrepExt_UpdateFullPosition(char *buffer, CreatureInstance *cInst);
int PrepExt_AbilityActivate(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent, bool ground = false);
int PrepExt_AbilityActivateEmpty(char *buffer, CreatureInstance *cInst, ActiveAbilityInfo *ability, int aevent);
int PrepExt_UpdateEquipStats(char *buffer, CreatureInstance *cInst);

#endif //CREATURE_H
