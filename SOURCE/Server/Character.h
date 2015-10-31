#pragma once

#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>
#include <map>

#include "Stats.h"
#include "Inventory.h"
#include "Creature.h"
#include "Quest.h"
#include "PlayerStats.h"

#include "Components.h"
#include "DirectoryAccess.h"
#include "CreatureSpawner2.h" //For timer
#include "AbilityTime.h"  //For cooldown data storage.
#include "Leaderboard.h"

using namespace std;

class ItemDef;
struct ChangeData;

#define MAX_LEVEL       70
#define MAX_PROFESSION   5
#define MAX_SIDEKICK     1

#define SIDEKICK_ABILITY_GROUP_ID	549

//This class is responsible for maintaining a list of ability identifiers.
//It is nothing more than a list of ability IDs that are available to a player.
class AbilityContainer
{
public:
	AbilityContainer();
	~AbilityContainer();

	std::vector<int> AbilityList;

	int GetAbilityIndex(int value);
	int AddAbility(int value);
};

class IntContainer
{
public:
	IntContainer();
	~IntContainer();

	std::vector<int> List;

	int GetIndex(int value);
	int Add(int value);
	int Remove(int value);
	void Clear(void);
};

struct _PreferencePair
{
	string name;
	string value;
};

//This class is for maintaining a list of preference strings used or set by a player.
class PreferenceContainer
{
public:
	PreferenceContainer();
	~PreferenceContainer();

	std::vector<_PreferencePair> PrefList;

	int GetPrefIndex(const char *name);
	const char * GetPrefValue(const char *name);
	int SetPref(const char *name, const char *value);
};

class FriendListObject
{
public:
	FriendListObject();
	~FriendListObject();

	string Name;
	int CDefID;

	void Clear(void);
};

struct GuildListObject
{
	int GuildDefID;
	int Valour;

	GuildListObject() { Clear(); };
	void WriteToJSON(Json::Value &value);
	void Clear(void);
};

struct SidekickObject
{
	static const int GENERIC = 0;
	static const int ABILITY = 1;
	static const int PET     = 2;
	static const int QUEST   = 3;

	int CDefID;           //Creature Definition this sidekick is derived from
	char summonType;      //Type of summon (GENERIC, ABILITY, PET)
	short summonParam;    //For Type:ABILITY, this is the ability group ID that summoned this object
	int CID;			  //The CID of the instantiated sidekick
	SidekickObject() { Clear(); };
	SidekickObject(int cdefid) { CDefID = cdefid; summonType = GENERIC; summonParam = 0; CID = 0; }
	SidekickObject(int cdefid, char type, short param) { CDefID = cdefid; summonType = type; summonParam = param; CID = 0; }
	void Clear(void) { CDefID = 0; summonType = GENERIC; summonParam = 0; CID = 0; }
};


// Holds session data for characters that is not specific to the client stat list.
struct ActiveCharacterData
{
	int CurInstance;
	int CurZone;
	int CurX;
	int CurY;
	int CurZ;
	unsigned char CurRotation;
	short MainDamage[2];
	short OffhandDamage[2];
	short RangedDamage[2];
	void CopyFrom(ActiveCharacterData *source)
	{
		if(source == NULL || source == this)
			return;
		memcpy(this, source, sizeof(ActiveCharacterData));
	}
};

struct NamedLocation
{
	std::string mName;
	int mX;
	int mY;
	int mZ;
	int mZone;
	void SetData(const char *data);
	void CopyFrom(const NamedLocation &source);
};

class CharacterData
{
public:
	CharacterData();
	~CharacterData();

	//static const int DEFAULT_VAULT_SIZE = 16;
	static const int MAX_VAULT_SIZE = 120;             //The maximum number of slots available in the client vault screen.
	static const int VAULT_EXPAND_SIZE = 8;            //Number of slots to add per vault expansion.
	static const int VAULT_EXPAND_CREDIT_COST = 200;

	int AccountID;            //Account ID that this character belongs to.
	int characterVersion;
	int pendingChanges;       //Determines whether this character needs to be resaved.
	unsigned long expireTime; //If loaded as a temporary resource, this is the expire time.

	//char Order[4];          //Unknown, but is a parameter used when sending an account's character list to a client.

	unsigned long SecondsLogged;  //Cumulative time count of seconds logged in.
	unsigned long CreatedTimeSec; //Time in seconds since epoch the character was created
	int SessionsLogged;      //Cumulative number of sessions logged in.
	char TimeLogged[32];     //Human readable format of SecondsLogged (hh:mm:ss format)
	char LastSession[32];    //Time logged during the last game session (hh:mm:ss format)
	char LastLogOn[32];      //Date and time of last login.
	char LastLogOff[32];     //Data and time of last logoff.

	int CurrentVaultSize;    //Vault size in slot count (8 slots per row)
	int CreditsPurchased;    //Unused. Total number of credits purchased.
	int CreditsSpent;        //Total number of credits spent.
	int ExtraAbilityPoints;  //Bonus ability points (purchased or rewarded) that are provided even after respec.

	int MaxSidekicks;       //Max number of sidekicks that may be active

	int Mode;				//PVP/PVP mode

	int groveReturnPoint[4];  //x, y, z, zoneID
	int bindReturnPoint[4];   //x, y, z, zoneID
	int pvpReturnPoint[4];    //x, y, z, zoneID
	unsigned long LastWarpTime; 
	int UnstickCount;               //Total number of times the Unstick ability was used.
	unsigned long LastUnstickTime;  //Server time (in minutes) when the last unstick was used.

	CreatureDefinition cdef;
	string originalAppearance;  //Holds the original appearance in case it was altered with creature tweak.

	string StatusText;      //Status text to show in the friend list
	string PrivateChannelName;
	string PrivateChannelPassword;

	ActiveCharacterData activeData;
	AbilityContainer abilityList;
	PreferenceContainer preferenceList;
	vector<GuildListObject> guildList;
	vector<FriendListObject> friendList;
	vector<SidekickObject> SidekickList;  //Holds a list of Creature Def IDs for each attached sidekick.
	vector<int> hengeList;     //List of CreatureDefIDs of henges that have been activated.

	std::vector<NamedLocation> namedLocation;

	InventoryManager inventory;
	bool localCharacterVault;  //If true, the vault inventory is stored locally on the character and should not be pushed into the account vault when unloading.

	QuestJournal questJournal;

	std::string InstanceScaler;

	ActiveCooldownManager cooldownManager;
	ActiveBuffManager buffManager;
	std::vector<ActiveStatMod> offlineStatMods;

	//short equipStat[Stat_MaxRow][Stat_MaxCol];
	unsigned int PermissionSet[2];

	// Character wide death/kill stats
	PlayerStatSet PlayerStats;

	int clan;

	void ClearAll();
	void CopyFrom(CharacterData &source);
	void EraseExpirationTime(void);
	void SetExpireTime(void);
	void ExtendExpireTime(void);

	int GetValour(int GuildDefID);
	void JoinGuild(int GuildDefID);
	void LeaveGuild(int GuildDefID);
	void AddValour(int GuildDefID, int valour);
	bool IsInGuildAndHasValour(int GuildDefID, int valour);

	void AddFriend(int CDefID, const char *name);
	int RemoveFriend(const char *name);

	int GetFriendIndex(int CDefID);

	void UpdateBaseStats(CreatureInstance *destData, bool setStats);
	void UpdateEquipStats(CreatureInstance *destData);
	void UpdateEqAppearance(void);
	void BackupAppearance(void);

	void BuildAvailableQuests(QuestDefinitionContainer &questList);
	void QuestJoin(int questID);
	void OnFinishedLoading(void);
	void CheckVersion(void);
	void VersionUpgradeCharacterItems(void);
	void OnCharacterCreation(void);
	void OnLevelChange(int newLevel);
	void OnRankChange(int newRank);
	void SetPlayerDefaults(void);
	void AbilityRespec(CreatureInstance *ptr);
	void SetLastChannel(const char *name, const char *password);

	bool HengeHas(int creatureDefID);
	void HengeAdd(int creatureDefID);
	void HengeSort(void);

	bool SetPermission(short filterType, const char *name, bool value);
	bool HasPermission(short permissionSet, unsigned int permissionFlag);
	bool QualifyGarbage(void);

	void RemoveSidekicks(int sidekickType);
	void AddSidekick(SidekickObject& skobj);
	int CountSidekick(int sidekickType);

	void AddAbilityPoints(int abilityPointCount);
	bool NotifyUnstick(bool peek);

	int VaultGetTotalCapacity(void);
	bool VaultIsMaximumCapacity(void);
	void VaultDoPurchaseExpand(void);

	void WritePrivateToJSON(Json::Value &value);
	void WriteToJSON(Json::Value &value);

//	void NamedLocationUpdate(const NamedLocation &location);
//	void NamedLocationGetPtr(const char *name);	

	void Debug_CountItems(int *intArr);

};

class CharacterLeaderboard : public Leaderboard {
public:
	CharacterLeaderboard();
	~CharacterLeaderboard();
	void OnBuild(std::vector<Leader> *leaders);
};

// Maintains characters that are currently loaded in memory.
class CharacterManager
{
public:
	typedef std::map<int, CharacterData> CHARACTER_MAP;
	typedef std::pair<int, CharacterData> CHARACTER_PAIR;

	CharacterManager();
	~CharacterManager();
	void Clear(void);

	Timer GarbageTimer;
	void CheckGarbageCharacters(void);
	void RemoveGarbageCharacters(void);
	void UnloadAllCharacters(void);
	bool SaveCharacter(int CDefID);
	void UnloadCharacter(int CDefID);

	CHARACTER_MAP charList;
	int LoadCharacter(int CDefID, bool tempResource);
	CharacterData *GetPointerByID(int CDefID);
	CharacterData *GetCharacterByName(const char *name);
	CharacterData *GetDefaultCharacter(void);
	CharacterData * RequestCharacter(int CDefID, bool tempOnly);
	void CreateDefaultCharacter(void);
	void AddExternalCharacter(int CDefID, CharacterData &newChar);
	void Compatibility_SaveList(FILE *output);
	void Compatibility_ResolveCharacters(void);

	static const int TEMP_EXPIRE_TIME = 20000;  //Temporary characters may be removed by garbage checks after this much time.
	static const int TEMP_GARBAGE_INTERVAL = 20000;

	void GetThread(const char *request);
	void ReleaseThread(void);
private:
	Platform_CriticalSection cs;
	bool RemoveSingleGarbage(void);  //CharacterData &charData);
	bool RemoveSingleCharacter(void);
};


extern CharacterManager g_CharacterManager;

//extern vector<CharacterData> CharList;
//int GetCharacterIndex(int CDefID);
//int GetCharacterIndexByName(char *name);

void SaveCharacterToStream(FILE *output, CharacterData &cd);

int PrepExt_FriendsAdd(char *buffer, CharacterData *charData);
int PrepExt_FriendsLogStatus(char *buffer, CharacterData *charData, int logStatus);


#endif //CHARACTER_H
