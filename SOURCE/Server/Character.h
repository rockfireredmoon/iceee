#pragma once

#ifndef CHARACTER_H
#define CHARACTER_H

#include <vector>
#include <map>

#include "Stats.h"
#include "Creature.h"
#include "Quest.h"

#include "Components.h"
#include "DirectoryAccess.h"
#include "CreatureSpawner2.h" //For timer
#include "AbilityTime.h"  //For cooldown data storage.


class ItemDef;
struct ChangeData;

#define MAXCONTAINER     6
#define MAX_LEVEL       70
#define MAX_PROFESSION   5
#define MAX_SIDEKICK     1

//This class is responsible for maintaining a list of ability identifiers.
//It is nothing more than a list of ability IDs that are available to a player.
class AbilityContainer
{
public:
	AbilityContainer();
	~AbilityContainer();

	vector<int> AbilityList;

	int GetAbilityIndex(int value);
	int AddAbility(int value);
};

class IntContainer
{
public:
	IntContainer();
	~IntContainer();

	vector<int> List;

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

	vector<_PreferencePair> PrefList;

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

struct SidekickObject
{
	static const int GENERIC = 0;
	static const int ABILITY = 1;
	static const int PET     = 2;

	int CDefID;           //Creature Definition this sidekick is derived from
	char summonType;      //Type of summon (GENERIC, ABILITY, PET)
	short summonParam;    //For Type:ABILITY, this is the ability group ID that summoned this object
	SidekickObject() { Clear(); };
	SidekickObject(int cdefid) { CDefID = cdefid; summonType = GENERIC; summonParam = 0; }
	SidekickObject(int cdefid, char type, short param) { CDefID = cdefid; summonType = type; summonParam = param; }
	void Clear(void) { CDefID = 0; summonType = GENERIC; summonParam = 0; }
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

struct InventorySlot
{
	unsigned int CCSID;      //Combined Container/Slot ID
	int IID;                 //Item ID.
	ItemDef *dataPtr;        //Points back to the source item.
	int count;               //Number of items in this stack.
	int customLook;          //If refashioned, this is the Item ID of the new look.
	char bindStatus;         //If true, item is bound to the player.
	int secondsRemaining;    //If nonzero, this is the active play time left until the item is destroyed (offline doesn't count)
	InventorySlot()
	{
		CCSID = 0;
		IID = 0;
		dataPtr = NULL;
		count = 0;
		customLook = 0;
		bindStatus = 0;
		secondsRemaining = -1;
	}
	int GetStackCount(void);
	int GetMaxStack(void);
	void CopyFrom(InventorySlot &source, bool copyCCSID);
	void CopyWithoutCount(InventorySlot &source, bool copyCCSID);
	ItemDef * ResolveItemPtr(void);
	ItemDef * ResolveSafeItemPtr(void);
	int GetLookID(void);
	unsigned short GetContainer(void);
	unsigned short GetSlot(void);
	void SaveToAccountStream(const char *containerName, FILE *output);
	bool VerifyItemExist(void);
	bool TestEquivalent(InventorySlot &other);
	void ApplyItemIntegerType(int IvType, int IvMax);
	int GetTimeRemaining(void);
};

struct InventoryQuery
{
	unsigned int CCSID;
	int count;
	char type;
	InventorySlot *ptr;
	static const int TYPE_NONE = 0;
	static const int TYPE_REMOVE = 1;
	static const int TYPE_MODIFIED = 2;
};

class InventoryManager
{
public:
	InventoryManager();
	~InventoryManager();
	vector<InventorySlot> containerList[MAXCONTAINER];
	int MaxContainerSlot[MAXCONTAINER];

	int buybackSlot;

	static const int ERROR_NONE   = 0;   //No error.
	static const int ERROR_ITEM   = 1;   //Item could not be found in the server item list.
	static const int ERROR_SPACE  = 2;   //Not enough space for the item.

	static const int EQ_ERROR_NONE       =  0;  //No error.
	static const int EQ_ERROR_ITEM       = -1;  //The item does not exist.
	static const int EQ_ERROR_LEVEL      = -2;  //The item cannot be worn by the given character level.
	static const int EQ_ERROR_PROFESSION = -3;  //The item cannot be worn by the given character class.
	static const int EQ_ERROR_SLOT       = -4;  //The item cannot be equipped in the given slot.

	unsigned char LastError;

	void SetError(int value);
	InventorySlot * GetExistingPartialStack(int containerID, ItemDef *itemDef);
	int AddItem(int containerID, InventorySlot &item);
	InventorySlot * AddItem_Ex(int containerID, int itemID, int count);
	int ScanRemoveItems(int containerID, int itemID, int count, vector<InventoryQuery> &resultList);
	int RemoveItemsAndUpdate(int container, int itemID, int itemCost, char *packetBuffer);
	int FindNextItem(int containerID, int itemID, int start);
	int RemoveItems(int containerID, vector<InventoryQuery> &resultList);

	int RemItem(unsigned int editCCSID);
	int GetItemByCCSID(unsigned int findCCSID);
	InventorySlot * GetItemPtrByCCSID(unsigned int findCCSID);
	InventorySlot * GetItemPtrByID(int itemID);
	int GetItemBySlot(int containerID, int slot);
	void ClearAll(void);
	unsigned int GetCCSID(unsigned short container, unsigned short slot);
	unsigned int GetCCSIDFromHexID(const char *hexStr);

	int GetFreeSlot(int containerID);
	int CountFreeSlots(int containerID);
	void CountInventorySlots(void);
	int GetItemCount(int containerID, int itemID);
	InventorySlot * GetFirstItem(int containerID, int itemID);

	int ItemMove(char *buffer, char *convBuf, CharacterStatSet *css, bool localCharacterVault, int origContainer, int origSlot, int destContainer, int destSlot);
	int AddItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
	int RemoveItemUpdate(char *buffer, char *convBuf, InventorySlot *slot);
	int SendItemIDUpdate(char *buffer, char *convBuf, InventorySlot *oldSlot, InventorySlot *newSlot);

	void FixBuyBack(void);
	int AddBuyBack(InventorySlot *item, char *buffer);

	bool VerifyContainerSlotBoundary(int container, int slot);
	int VerifyEquipItem(ItemDef *itemDef, int destSlot, int charLevel, int charProfession);
	static const char * GetEqErrorString(int code);
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
	vector<FriendListObject> friendList;
	vector<SidekickObject> SidekickList;  //Holds a list of Creature Def IDs for each attached sidekick.
	vector<int> hengeList;     //List of CreatureDefIDs of henges that have been activated.

	std::vector<NamedLocation> namedLocation;

	InventoryManager inventory;
	bool localCharacterVault;  //If true, the vault inventory is stored locally on the character and should not be pushed into the account vault when unloading.

	QuestJournal questJournal;

	std::string InstanceScaler;

	ActiveCooldownManager cooldownManager;
	std::vector<ActiveStatMod> offlineStatMods;

	//short equipStat[Stat_MaxRow][Stat_MaxCol];
	unsigned int PermissionSet[2];

	void ClearAll();
	void CopyFrom(CharacterData &source);
	void EraseExpirationTime(void);
	void SetExpireTime(void);
	void ExtendExpireTime(void);

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

//	void NamedLocationUpdate(const NamedLocation &location);
//	void NamedLocationGetPtr(const char *name);	

	void Debug_CountItems(int *intArr);

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

#endif //CHARACTER_H
