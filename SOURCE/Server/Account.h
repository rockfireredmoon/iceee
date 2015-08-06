#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <list>
#include <vector>
#include <string>
#include <map>
#include "Stats.h"
#include "Components.h"
#include "Character.h"
#include "Util.h"  //ChangeData was moved here


class CharacterData;

struct BuildPermissionArea
{
	int ZoneID;
	short x1;
	short y1;
	short x2;
	short y2;
};

//An entry for a single character in the character select window. Holds a cache
//of data so that the entire character list doesn't have to be explicitly loaded.
struct CharacterCacheEntry
{
	int creatureDefID;
	int level;
	int profession;
	std::string display_name;
	std::string appearance;
	std::string eq_appearance;
	CharacterCacheEntry()
	{
		creatureDefID = 0;
		level = 0;
		profession = 0;
	}
	void Clear(void)
	{
		creatureDefID = 0;
		level = 0;
		profession = 0;
		display_name.clear();
		appearance.clear();
		eq_appearance.clear();
	}
};

//Manages the cache data for all characters visible in the character select window.
struct CharacterCacheManager
{
	std::vector<CharacterCacheEntry> cacheData;
	CharacterCacheEntry* GetCacheCharacter(int cdefID);
	CharacterCacheEntry* ForceGetCharacter(int cdefID);
	CharacterCacheEntry* UpdateCharacter(CharacterData *charData);
	void RemoveCharacter(int cdefID);
	void SaveToStream(FILE *output);
	void AddEntry(CharacterCacheEntry &data);
};

class AccountData
{
public:
	AccountData();
	~AccountData();

	static const int EXPIRE_TIME = 5000;
	static const int MAX_CHARACTER_SLOTS = 8;  //Absolute maximum number of characters an account can have
	static const int DEFAULT_CHARACTER_SLOTS = 4;  //Absolute maximum number of characters an account can have

	int ID;
	char Name[48];
	char AuthData[36];
	char RegKey[36];
	std::string RecoveryKeys;
	int CharacterSet[MAX_CHARACTER_SLOTS];
	unsigned int PermissionSet[2];
	bool MatchAuthData(const char *str);
	void ResolveCharacters(const char *debugName);
	int GetCharacterCount(void);
	PreferenceContainer preferenceList;
	int PendingMinorUpdates;

	unsigned long SuspendDurationSec;  //Duration of ban
	unsigned long SuspendTimeSec;      //Time when the ban was first set

	char LastLogOn[32];      //Date and time of last login of any character in the account.

	/* User data  */
	std::vector<InventorySlot> vaultInventory;
	//int CurrentVaultSize;
	/*  End user data. */

	CharacterCacheManager characterCache;
	std::string GroveName;
	std::vector<BuildPermissionArea> BuildPermissionList;
	int MaxCharacters;

	// Transient stuff (not store in character data)
	bool DueDailyRewards;	// When set to true, when login has completely finished the player will be given their daily rewards

	bool HasBuildZone(BuildPermissionArea &bpa);

	bool ExpandCharacterSlots();
	void ClearAll(void);
	void FillAuthorizationHash(const char *hash);
	void FillRegistrationKey(const char *key);
	bool SetPermission(short filterType, const char *name, bool value);
	bool HasPermission(short permissionSet, unsigned int permissionFlag);
	int GetFreeCharacterSlot(void);
	bool CheckBuildPermission(int zoneID, int pagex, int pagey);
	bool CheckBuildPermissionAdv(int zoneID, int PageSize, float posx, float posz);
	bool HasCharacterID(int CDefID);

	void SetBan(int minutes);
	void ClearBan(void);

	void AdjustSessionLoginCount(short count);
	int GetSessionLoginCount(void);
	void SaveToStream(FILE *output);
	bool QualifyGarbage(bool force);

	void GenerateClientPasswordHash(const char *username, const char *password, std::string &outputString);
	static void GenerateSaltedHash(const char *inputString, std::string &outputString);
	void GenerateAndApplyRegistrationKeyRecovery(void);
	bool MatchRecoveryKey(const char *type, const char *hash);
	void SetNewPassword(const char *username, const char *plainTextPassword);
	void SetNewRegistrationKey(const char *regkey);
	bool MatchRegistrationKey(const char *regkey);
	bool IsRegistrationKeyEmpty(void);
	void CheckRecoveryRegistrationKey(const char *regkey);
private:
	short SessionLoginCount;   //Number of Simulators logged into this account.
	unsigned long ExpireTime;  //The server time when the account will be ready for garbage deletion.
};

class FileReader;

struct AccountQuickData
{
	int mID;
	std::string mLoginName;
	std::string mLoginAuth;
	std::string mRegKey;
	std::string mGroveName;
	AccountQuickData()
	{
		mID = 0;
	}
	void Clear(void)
	{
		mID = 0;
		mLoginName.clear();
		mLoginAuth.clear();
		mRegKey.clear();
		mGroveName.clear();
	}
};

class UsedNameDatabase
{
public:
	UsedNameDatabase();
	~UsedNameDatabase();

	//To simplify access of the primary map.
	typedef std::map<int, std::string>& CONTAINER_REF;
	typedef std::map<int, std::string>::iterator ITERATOR;

	UsedNameDatabase::CONTAINER_REF GetData(void);              //Return the primary data object.
	size_t GetDataCount(void);                //Get primary map size.

	void Add(int CDefID, const char *name, bool loadStage = false);   //Add a data pair to both maps.
	void Remove(int CDefID);                  //Remove an ID from both maps.

	bool HasID(int CDefID);                   //Test if the ID exists.
	const char *GetNameByID(int CDefID);      //Get the character name of the corresponding ID.

	bool HasName(const char *name);           //Test if the character name exists.
	int GetIDByName(const char *name);        //Get the ID of the corresponding character name.

	bool HasChanged(void);

	ChangeData mChanges;

private:
	typedef std::map<std::string, int>::iterator NAME_ITERATOR;

	std::map<int, std::string> mData;        //Primary data object.
	std::map<std::string, int> mNameLookup;  //Secondary map for reverse lookups by name.
};

class AccountManager
{
public:
	typedef std::list<AccountData>::iterator ACCOUNT_ITERATOR;
	//typedef pair<int, std::string> USEDCHAR_PAIR;
	//typedef std::vector<USEDCHAR_PAIR> USEDCHAR_VECTOR;

#ifdef PRIVATE_USE_BUILD
	static const int MAX_CONCURRENT_LOGINS = 4;
#endif

	AccountManager();
	~AccountManager();
	void LoadAllData(void);
	void UnloadAllData(void);

	int NextAccountID;

	std::string KeyFileName;
	std::string UsedListFileName;

	Platform_CriticalSection cs;  //Needed for external account management since the HTTP threads might be accessing this concurrently.

	static const int DEFAULT_CHARACTER_ID = -400000000;
	static const int CHARACTER_ID_INCREMENT = -8;
	int NextCharacterID;

	std::vector<std::string> KeyList;
	std::list<AccountData> AccList;
	//USEDCHAR_VECTOR UsedCharacterNames;
	UsedNameDatabase UsedCharacterNames;

	ChangeData KeyListChanges;

	std::vector<AccountQuickData> accountQuickData;
	void AppendQuickData(AccountData *account);
	bool SaveQuickData(void);
	void LoadQuickData(void);
	AccountQuickData * GetAccountQuickDataByUsername(const char *username);
	ChangeData AccountQuickDataChanges;

	AccountData * GetActiveAccountByID(int accountID);
	AccountData * FetchIndividualAccount(int accountID);
	AccountData * LoadAccountID(int accountID);
	const char * GetIndividualFilename(char *buffer, int bufsize, int accountID);

	AccountData * GetValidLogin(const char *loginName, const char *loginAuth);
	void ResolveCharacters(void);

	void LoadKeyList(const char *fileName);
	void ImportKeys(void);
	int GetRegistrationKey(const char *authKey);
	int CreateAccount(const char *username, const char *password, const char *regKey, const char *grovename);
	int ResetPassword(const char *username, const char *newpassword, const char *regKey);
	int AccountRecover(const char *username, const char *keypass, const char *type);
	bool ValidString(const char *str);
	bool ValidGroveString(std::string &nameToAdjust);
	AccountData * FetchAccountByUsername(const char *username);
	const char * GetErrorMessage(int message);
	int CheckAutoSave(bool force);
	int HasPendingMinorUpdates(void);
	void SaveIndividualAccount(AccountData *account);
	void SaveKeyListChanges(void);
	void SaveUsedNameListChanges(void);

	//Placeholder character creation stuff.  Should probably be moved to
	//the character system when that's redone.
	int CreateCharacter(STRINGLIST &args, AccountData *accPtr);
	int GetNewCharacterID(void);
	void DeleteCharacter(int index, AccountData *accPtr);

	bool ValidCharacterName(const std::string &name);
	const char * GetCharacterErrorMessage(int message);

	bool HasUsedCharacterName(const char *characterName);
	int GetCDefFromCharacterName(const char *characterName);
	const char *GetCharacterNameFromCDef(int CDefID);
	void AddUsedCharacterName(int CDefID, const char *characterName);
	void RemoveUsedCharacterName(int CDefID);
	void LoadUsedNameList(const char *fileName);
	void RunUpdateCycle(bool force);
	bool AcceptingLogins(void);
	int ValidateNameParts(const std::string &first, const std::string &last);

	enum ErrorCode
	{
		ACCOUNT_SUCCESS  = 0,   //Account creation was successful.
		ACCOUNT_KEY         ,   //The registration key was rejected.
		ACCOUNT_SIZENAME    ,   //The username is either too long or too short.
		ACCOUNT_INVNAME     ,   //The username is invalid.
		ACCOUNT_HASNAME     ,   //The username already exists.
		ACCOUNT_SIZEPASS    ,   //The password is either too long or too short.
		ACCOUNT_INVPASS     ,   //The password is invalid.
		ACCOUNT_SIZEGROVE   ,   //The grove name is either too long or too short.
		ACCOUNT_INVGROVE    ,   //The grove name is invalid.
		ACCOUNT_HASGROVE    ,   //The grove name already exists.
		ACCOUNT_REGMISMATCH,      //The registration key provided does not match the account.
		ACCOUNT_PASSWORDRESETOK,  //Password reset successful.
		ACCOUNT_USERNOTFOUND,     //The account username was not found.
		ACCOUNT_PERMISSIONRESET,  //The account must be approved for a password reset.

		//Account recovery error codes
		ACCOUNT_BADUSERNAME,      //Username was missing or not supplied.
		ACCOUNT_BADKEY,           //Registration key was missing or not supplied.
		ACCOUNT_BADPASSWORD,      //Password was missing or not supplied.
		ACCOUNT_REJECTED,         //Account recover failed, information does not match.
		ACCOUNT_SUCCESSRECOVER,   //The account was recovered successfully.
		ACCOUNT_BADREQUEST,       //The required information was malformed or incomplete.
		ACCOUNT_CANNOTRECOVER,    //This account cannot be recovered.  If recovered before, it cannot be recovered again.
	};

	enum CharacterErrorCode
	{
		CHARACTER_SUCCESS    =  0,
		CHARACTER_INVQUERY   = -1,
		CHARACTER_NOSLOTS    = -2,
		CHARACTER_FIRSTINV   = -3,
		CHARACTER_FIRSTSHORT = -4,
		CHARACTER_FIRSTLONG  = -5,
		CHARACTER_LASTINV    = -6,
		CHARACTER_LASTSHORT  = -7,
		CHARACTER_LASTLONG   = -8,
		CHARACTER_NAMEEXIST  = -9 
	};
	
private:
	void LoadAccountFromStream(FileReader &fr, AccountData &ad, const char *debugFilename);
	void LoadSectionGeneral(FileReader &fr, AccountData &ad, const char *debugFilename);
	void LoadSectionCharacterCache(FileReader &fr, AccountData &ad, const char *debugFilename);
	Timer TimerGeneralUpdate;
	static const int GENERAL_UPDATE_FREQUENCY = 5000;
};

enum PermissionTypeEnum
{
	Perm_Account = 0,
	Perm_Character
};

enum PermissionFlagEnum
{
	//Permission set 0
	Permission_TweakOther     = 0x00000001,     //Can edit other players via creaturetweak
	Permission_TweakSelf      = 0x00000002,     //Can edit self via creaturetweak
	Permission_TweakNPC       = 0x00000004,     //Can edit NPCs via creaturetweak
	Permission_TweakClient    = 0x00000008,     //Creaturetweak effects are client-side only (no server changes, unlike the other tweak permissions).
	Permission_SysChat        = 0x00000100,     //Params not used.
	Permission_GMChat         = 0x00000200,     //Params not used.
	Permission_RegionChat     = 0x00000400,     //Params not used.
	Permission_ForumPost      = 0x00001000,     //Can post on the forums.
	Permission_ForumAdmin     = 0x00004000,     //Can perform all administrative tasks in the forums.
	Permission_Debug          = 0x00100000,     //Params not used.
	Permission_Sage           = 0x00200000,     //Permissions intended for moderator duty.
	Permission_ItemGive       = 0x00400800,     //Params not used.
	Permission_FastLoad       = 0x01000000,     //This will load the scenery in the background.
	Permission_Admin          = 0x02000000,     //Special admin permissions that exceed ordinary debug stuff.
	Permission_Invisible      = 0x04000000,     //Admin invisibility.  No login/logout notification, does not appear in /who, does not broadcast movement.
	Permission_Troll          = 0x00010000,     //Just for fun.
	Permission_TrollChat      = 0x00020000,     //Just for fun.  Uses a customizable phrase replace on region chat messages.
	Permission_SelfDiag       = 0x10000000,     //Not a "permission" but helps to track down arbitrary stuff for specific players.
	Permission_PasswordReset  = 0x20000000,     //The password can be reset on this account.

	//Full permissions for all flags of a given set
	Permission_FullSet    = 0xFFFFFFFF     //Full permissions for all flags of this set
};

struct PermissionInfo
{
	short type;      //Permission type, either Perm_Account or Perm_Character
	short index;     //Index into the flag array
	unsigned int flag;  //The bit for this flag
	const char *name;     //The name to use for this flag (when loading from the file)
};

extern const int MaxPermissionDef;
extern PermissionInfo PermissionDef[];

extern AccountManager g_AccountManager;

void DeleteCharacterList(void);

int GetCharacterIndex(int ID);

int CheckLoginIndex(char *LoginName, char *LoginAuth);

#endif //ACCOUNT_H
