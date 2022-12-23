
#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <vector>
#include <map>
#include <string>

#include "Util.h"

static std::string ID_GAME_CONFIG = "GameConfig";

class GameConfig
{
public:
	GameConfig();
	~GameConfig();

	void Reload();
	std::map<std::string, std::string> GetAll();
	bool HasKey(const std::string &key);
	bool Set(const std::string &key, const std::string &value);
	std::string Get(const std::string &key);
	void Init(void);

	bool AllowEliteMob;              	// If true, mobs may spawn as elite variants.
	bool MegaLootParty;					// For fun and testing, when 1, everything always drops, usually more than once.
	bool FallDamage;					// If true, fall damage is enabled
	int BuybackLimit;					// The maximum number of items that will be held in the "Buyback" tab at vendors.

	bool Clans;							// Whether clans are enabled or not
	unsigned int ClanCost;				// How much it costs to form a clan

	std::string OverrideStartLoc;		// Override default zone,location and rotation provided by static game data

	int CapExperienceLevel;				// At what level should the server start to limit the amount of experience that would be added for a given event (e.g. kill). When this occurs, the amount to add will be limited to whatever is set in **CapExperienceAmount** set below. When set to zero, this cap will never occur.
	int CapExperienceAmount;			// When **CapExperienceLevel** is reached, what is the maximum amount of XP that can be added for any event (e.g. kill). If this is set to **0**, then no more experience can be added.
	int CapValourLevel;					// Currently unused until valour system is activated. Will be a hard cap on the valour level.

	int AprilFools;						// Activate april fools appearance joke
	std::string AprilFoolsName;			// Activate april fools character name joke (requires appear joke to be active to)
	int AprilFoolsAccount;				// Account number that april fools joke applies to. *waves at Disaster Master*

	int VaultDefaultSize;            	// Number of vault slots that all characters have.  If characters have not purchased any slots at all, this amount will still be available.
	int VaultInitialPurchaseSize;    	// Newly created characters will be given this many free slots (considered as purchased space).

	int GlobalMovementBonus;         	// If nonzero, all objects placed into a instance (players, mobs, NPCs, etc) will gain this default modifier to run speed.
	bool CustomAbilityMechanics;  		// If true, certain abilities may be processed with custom mechanics differently than a classic official server might.

	float DexBlockDivisor;           	// Points of dexterity may provide a bonus chance to block physical attacks.
	float DexParryDivisor;           	// Points of dexterity may provide a bonus chance to parry physical attacks.
	float DexDodgeDivisor;           	// Points of dexterity may provide a bonus chance to dodge physical attacks.
	float SpiResistDivisor;          	// Points of spirit may provide bonus resistance to certain elemental attacks.
	float PsyResistDivisor;          	// Points of psyche may provide bonus resistance to certain elemental attacks.

	float DropRateBonusMultMax;         // The maximum drop rate bonus multiplier that any kill may have.  This affects the absolute total after all drop rate calculations have been applied.
	float ProgressiveDropRateBonusMult[4];   // Needs to hold rarities [0,1,2,3].  Additive amount to increase instance drop rates per kill by a creature of a certain rarity.
	float ProgressiveDropRateBonusMultMax;   // The maximum instance drop rate bonus from additive kills.
	float NamedMobDropMultiplier;    	// All mobs marked as named (see "ExtraData" field for CreatureDefinitions) will receive a drop rate bonus for randomized items.
	int NamedMobCreditDrops;       		// All mobs marked as named (see "ExtraData" field for CreatureDefinitions) will drop credits (when the player is at or below level, bonus given for parties).
	int LootMaxRandomizedLevel;         // The randomizer cannot generate loot above this level for typical mobs.
	int LootMaxRandomizedSpecialLevel;  // The randomizer cannot generate loot above this level for "special" mobs
	bool LootNamedMobSpecial;           // If true, named mobs are considered for the special item level cap.
	int LootMinimumMobRaritySpecial;    // The minimum quality level for a mob to be considered special.

	int HeroismQuestLevelTolerance;  	// How many levels above the quest level that the player is allowed to be to receive full heroism.
	int HeroismQuestLevelPenalty;    	// Points of heroism to lose per level if over the quest tolerance level.

	unsigned int MinPVPPlayerLootItems;	// Minimum number of items that will be dropped by the player after a PVP fight
	unsigned int MaxPVPPlayerLootItems;	// Minimum number of items that will be dropped by the player after a PVP fight

	unsigned int NameChangeCost;		// Number of credits a last name change costs

	int MaxAuctionHours;				// Maximum number of hours an auction can last
	int MinAuctionHours;				// Minimum number of hours an auction can last
	float PercentageCommisionPerHour;   // Percentage to take per hour
	int MaxAuctionExpiredHours;		    // Maximum number of hours an auction can be expired

	int MaxNewCreditShopItemDays;		// Maximum number of days an item in the credit shop is considered 'New'

	std::string EnvironmentCycle;		// String that determines schedule for day -> night changes.

	bool UseReagents;					// If true, reagents are enabled and required for certain abilities and scrolls
	bool UseAccountCredits;				// If true, credits will be stored at the account level rather than per character and shared across all characters
	bool UsePersistentBuffs;                 // If true, active buffs will be saved and restored on next login
	bool UsePartyLoot;						 // Whether to allow party loot

private:
	void LoadMap();
	void AddIfMissing(const std::string &key, const std::string &value, std::map<std::string, std::string> &map);

};

extern GameConfig g_GameConfig;

#endif  //#define GAMECONFIG_H
