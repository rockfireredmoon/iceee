#include "GameConfig.h"
#include "Cluster.h"
#include "ZoneDef.h"
#include "util/Log.h"

GameConfig g_GameConfig;

GameConfig::GameConfig() {
	AllowEliteMob = true;
	MegaLootParty = false;
	FallDamage = false;
	BuybackLimit = 32;
	Clans = true;
	ClanCost = 100000;
	OverrideStartLoc = "";
	CapValourLevel = 0;
	CapExperienceLevel = 70;
	CapExperienceAmount = 0;
	VaultDefaultSize = 16;
	VaultInitialPurchaseSize = 8;
	GlobalMovementBonus = 0;
	CustomAbilityMechanics = false;   //Classic emulation by default.

	DexBlockDivisor = 0.0F;
	DexParryDivisor = 0.0F;
	DexDodgeDivisor = 0.0F;
	SpiResistDivisor = 0.0F;
	PsyResistDivisor = 0.0F;

	AprilFools = 0;
	AprilFoolsAccount = 0;
	AprilFoolsName = "";

	DropRateBonusMultMax = 200.0F;
	NamedMobDropMultiplier = 4.0F;
	NamedMobCreditDrops = 1;
	LootMaxRandomizedLevel = 50;
	LootMaxRandomizedSpecialLevel = 55;
	LootNamedMobSpecial = true;
	LootMinimumMobRaritySpecial = 2;
	ProgressiveDropRateBonusMult[0] = 0.0025F;
	ProgressiveDropRateBonusMult[1] = 0.0050F;
	ProgressiveDropRateBonusMult[2] = 0.0100F;
	ProgressiveDropRateBonusMult[3] = 0.0200F;
	ProgressiveDropRateBonusMultMax = 2.0F;

	HeroismQuestLevelTolerance = 3;
	HeroismQuestLevelPenalty = 4;

	NameChangeCost = 300;

	MinPVPPlayerLootItems = 0;
	MaxPVPPlayerLootItems = 3;

	MaxAuctionHours = 24 * 7;
	MinAuctionHours = 1;
	PercentageCommisionPerHour = 5.0 / 24;
	MaxAuctionExpiredHours = 24;

	MaxNewCreditShopItemDays = 14;
	EnvironmentCycle = "Sunrise=05:30,Day=08:30,Sunset=18:00,Night=20:30";

	UseAccountCredits = true;
	UseReagents = true;
	UsePersistentBuffs = true;
	UsePartyLoot = true;
}

GameConfig::~GameConfig() {
}

std::map<std::string, std::string> GameConfig::GetAll() {
	auto map = g_ClusterManager.GetMap(ID_GAME_CONFIG);
	AddIfMissing("AllowEliteMob", "true", map);
	AddIfMissing("MegaLootParty", "false", map);
	AddIfMissing("FallDamage", "false", map);
	AddIfMissing("BuybackLimit", "32", map);
	AddIfMissing("Clans", "true", map);
	AddIfMissing("ClanCost", "100000", map);
	AddIfMissing("OverrideStartLoc", "", map);
	AddIfMissing("CapExperienceLevel", "70", map);
	AddIfMissing("CapExperienceAmount", "0", map);
	AddIfMissing("CapValourLevel", "0", map);
	AddIfMissing("VaultDefaultSize", "16", map);
	AddIfMissing("VaultInitialPurchaseSize", "8", map);
	AddIfMissing("GlobalMovementBonus", "0", map);
	AddIfMissing("CustomAbilityMechanics", "false", map);

	AddIfMissing("AprilFools", "false", map);
	AddIfMissing("AprilFoolsName", "", map);
	AddIfMissing("AprilFoolsAccount", "0", map);

	AddIfMissing("DexBlockDivisor", "0.0", map);
	AddIfMissing("DexParryDivisor", "0.0", map);
	AddIfMissing("DexDodgeDivisor", "0.0", map);
	AddIfMissing("SpiResistDivisor", "0.0", map);
	AddIfMissing("PsyResistDivisor", "0.0", map);

	AddIfMissing("ProgressiveDropRateBonusMult", "0.0025,0.005,0.01,0.02", map);
	AddIfMissing("ProgressiveDropRateBonusMultMax", "2.0", map);
	AddIfMissing("DropRateBonusMultMax", "200.0", map);
	AddIfMissing("NamedMobDropMultiplier", "4.0", map);
	AddIfMissing("NamedMobCreditDrops", "1", map);
	AddIfMissing("LootMaxRandomizedLevel", "50", map);
	AddIfMissing("LootMaxRandomizedSpecialLevel", "55", map);
	AddIfMissing("LootNamedMobSpecial", "true", map);
	AddIfMissing("LootMinimumMobRaritySpecial", "2", map);

	AddIfMissing("HeroismQuestLevelTolerance", "3", map);
	AddIfMissing("HeroismQuestLevelPenalty", "4", map);

	AddIfMissing("NameChangeCost", "300", map);

	AddIfMissing("MinPVPPlayerLootItems", "0", map);
	AddIfMissing("MaxPVPPlayerLootItems", "3", map);

	AddIfMissing("MaxAuctionHours", "168", map);
	AddIfMissing("MinAuctionHours", "1", map);
	AddIfMissing("PercentageCommisionPerHour", "0.208333333", map);
	AddIfMissing("MaxAuctionExpiredHours", "24", map);

	AddIfMissing("MaxNewCreditShopItemDays", "31", map);
	AddIfMissing("EnvironmentCycle", "Sunrise=05:30,Day=08:30,Sunset=18:00,Night=20:30", map);

	AddIfMissing("UseAccountCredits", "true", map);
	AddIfMissing("UseReagents", "true", map);
	AddIfMissing("UsePersistentBuffs", "true", map);
	AddIfMissing("UsePartyLoot", "true", map);

	return map;
}

void GameConfig::LoadMap() {
	auto map = GetAll();
	AllowEliteMob = map["AllowEliteMob"] == "true";
	MegaLootParty = map["MegaLootParty"] == "true";
	FallDamage = map["FallDamage"] == "true";
	BuybackLimit = std::stoi(map["BuybackLimit"]);
	Clans = map["Clans"] == "true";
	ClanCost = std::stoi(map["ClanCost"]);
	OverrideStartLoc = map["OverrideStartLoc"];
	CapExperienceLevel = std::stoi(map["CapExperienceLevel"]);
	CapExperienceAmount = std::stoi(map["CapExperienceAmount"]);
	CapValourLevel = std::stoi(map["CapValourLevel"]);
	VaultDefaultSize = std::stoi(map["VaultDefaultSize"]);
	VaultInitialPurchaseSize = std::stoi(map["VaultInitialPurchaseSize"]);
	GlobalMovementBonus = std::stoi(map["GlobalMovementBonus"]);
	CustomAbilityMechanics = map["CustomAbilityMechanics"] == "true";

	AprilFools = map["AprilFools"] == "true";
	AprilFoolsName = map["AprilFoolsName"];
	AprilFoolsAccount = std::stoi(map["AprilFoolsAccount"]);

	DexBlockDivisor = std::stof(map["DexBlockDivisor"]);
	DexParryDivisor = std::stof(map["DexParryDivisor"]);
	DexDodgeDivisor = std::stof(map["DexDodgeDivisor"]);
	SpiResistDivisor = std::stof(map["SpiResistDivisor"]);
	PsyResistDivisor = std::stof(map["PsyResistDivisor"]);

	NamedMobDropMultiplier = std::stof(map["NamedMobDropMultiplier"]);
	NamedMobCreditDrops = std::stoi(map["NamedMobCreditDrops"]);

	DropRateBonusMultMax = std::stof(map["DropRateBonusMultMax"]);
	ProgressiveDropRateBonusMultMax = std::stof(map["ProgressiveDropRateBonusMultMax"]);
	Util::AssignFloatArrayFromStringSplit(
							ProgressiveDropRateBonusMult,
							COUNT_ARRAY_ELEMENTS(
									ProgressiveDropRateBonusMult),
									map["ProgressiveDropRateBonusMult"]);
	LootMaxRandomizedLevel = std::stoi(map["VaultInitialPurchaseSize"]);
	LootMaxRandomizedSpecialLevel = std::stoi(map["GlobalMovementBonus"]);
	LootNamedMobSpecial = map["LootNamedMobSpecial"] == "true";
	LootMinimumMobRaritySpecial = std::stoi(map["LootMinimumMobRaritySpecial"]);

	HeroismQuestLevelTolerance = std::stoi(map["HeroismQuestLevelTolerance"]);
	HeroismQuestLevelPenalty = std::stoi(map["HeroismQuestLevelPenalty"]);

	NameChangeCost = std::stoi(map["NameChangeCost"]);

	MinPVPPlayerLootItems = std::stoi(map["MinPVPPlayerLootItems"]);
	MaxPVPPlayerLootItems = std::stoi(map["MaxPVPPlayerLootItems"]);

	MaxAuctionHours = std::stoi(map["MaxAuctionHours"]);
	MinAuctionHours = std::stoi(map["MinAuctionHours"]);
	MaxAuctionExpiredHours = std::stoi(map["MaxAuctionExpiredHours"]);
	PercentageCommisionPerHour = std::stof(map["PercentageCommisionPerHour"]);
	EnvironmentCycle = map["EnvironmentCycle"];

	UseAccountCredits = map["UseAccountCredits"] == "true";
	UseReagents = map["UseReagents"] == "true";
	UsePersistentBuffs = map["UsePersistentBuffs"] == "true";
	UsePartyLoot = map["UsePartyLoot"] == "true";
}

void GameConfig::Reload() {
	LoadMap();
	g_EnvironmentCycleManager.ApplyConfig(EnvironmentCycle);
}

bool GameConfig::HasKey(const std::string &key) {
	auto map = GetAll();
	return map.find(key) != map.end();
}

bool GameConfig::Set(const std::string &key, const std::string &value) {
	if(HasKey(key)) {
		g_ClusterManager.SetMapVal(ID_GAME_CONFIG, key, value);
		g_Logs.cluster->info("Game configuration '%v' changed to '%v'.", key, value);
		g_ClusterManager.GameConfigChanged(key, value);
		Reload();
		return true;
	}
	else {
		return false;
	}
}

std::string GameConfig::Get(const std::string &key) {
	return GetAll()[key];
}

void GameConfig::Init(void) {
	LoadMap();
}

void GameConfig::AddIfMissing(const std::string &key, const std::string &value, std::map<std::string, std::string> &map) {
	if(map.find(key) == map.end())
		map[key] = value;
}
