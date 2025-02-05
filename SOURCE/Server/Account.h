#ifndef ACCOUNT_H
#define ACCOUNT_H


#include <list>
#include <vector>
#include <string>
#include <map>
#include "Stats.h"
#include "Components.h"
#include "Inventory.h"
#include "Achievements.h"
#include "Character.h"
#include "Entities.h"
#include "PlayerStats.h"
#include "Util.h"  //ChangeData was moved here
#include "json/json.h"
#include "http/HTTP.h"

static std::string KEYPREFIX_CHARACTER_NAME_TO_ID = "CharacterNameToID";
static std::string KEYPREFIX_CHARACTER_ID_TO_NAME = "CharacterIDToName";
static std::string KEYPREFIX_REGISTRATION_KEYS = "RegistrationKeys";
static std::string KEYPREFIX_ACCOUNT_QUICK_DATA = "AccountQuickData";
static std::string KEYPREFIX_ACCOUNT_DATA = "AccountData";
static std::string ID_NEXT_ACCOUNT_ID = "NextAccountID";
static std::string KEYPREFIX_ACCOUNT_SESSIONS = "AccountSessions";

class CharacterData;

class AccessToken
{
public:
	AccessToken();
	~AccessToken();

	enum TokenType {
		AUTHENTICATION_CODE,
		ACCESS_TOKEN
	};

	std::string token;
	int tokenType;
	int uses;
	int accountID;
	unsigned long expire;
};

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
class CharacterCacheEntry: public AbstractEntity {
public:
	int creatureDefID;
	int level;
	int profession;
	std::string display_name;
	std::string appearance;
	std::string eq_appearance;

	CharacterCacheEntry();
	~CharacterCacheEntry();
	void Clear(void);
	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);
};

//Manages the cache data for all characters visible in the character select window.
struct CharacterCacheManager
{
	std::vector<CharacterCacheEntry> cacheData;
	CharacterCacheEntry* GetCacheCharacter(int cdefID);
	CharacterCacheEntry* ForceGetCharacter(int cdefID);
	CharacterCacheEntry* UpdateCharacter(CharacterData *charData);
	void RemoveCharacter(int cdefID);
	bool ReadEntity(AbstractEntityReader *reader);
	bool WriteEntity(AbstractEntityWriter *writer);
	void AddEntry(CharacterCacheEntry &data);
};

class AccountData: public AbstractEntity {
public:
	AccountData();
	virtual ~AccountData();
	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

	static const int EXPIRE_TIME = 5000;
	static const int MAX_CHARACTER_SLOTS = 8;  //Absolute maximum number of characters an account can have
	static const int MAX_DELIVERY_BOX_SLOTS = 3;  //Maximum number of delivery box slots allowed
	static const int DEFAULT_CHARACTER_SLOTS = 4;  //Default number of characters an account can have

	int ID;
	std::string Name;
	std::string AuthData;
	std::string RegKey;
	std::string RecoveryKeys;
	int CharacterSet[MAX_CHARACTER_SLOTS];
	unsigned int PermissionSet[2];

	void SetRoles(std::vector<std::string> &roles);
	bool MatchAuthData(const char *str);
	void ResolveCharacters(const char *debugName);
	int GetCharacterCount(void);
	PreferenceContainer preferenceList;
	int PendingMinorUpdates;

	unsigned long SuspendDurationSec;  //Duration of ban
	unsigned long SuspendTimeSec;      //Time when the ban was first set

	std::string LastLogOn;      //Date and time of last login of any character in the account.
	unsigned long CreatedTimeSec; //Time in seconds since epoch the account was created
	unsigned long LastLogOnTimeSec; //Last login time in seconds since epoch
	int ConsecutiveDaysLoggedIn; // The number of consecutive days the account has logged in
	int Credits; // The number of credits the account has (if AccountCredits is on)
	int DeliveryBoxSlots;
	int VeteranLevel; // If the account is a veteran, they might get privileges
	bool VeteranImported; // If the veteran account has been imported

	// Account wide stats
	PlayerStatSet PlayerStats;

	/* User data  */
//	std::vector<InventorySlot> deliveryInventory;
//	std::vector<InventorySlot> vaultInventory;
	InventoryManager inventory;
	//int CurrentVaultSize;
	/*  End user data. */

	CharacterCacheManager characterCache;
	std::string GroveName;
	std::vector<BuildPermissionArea> BuildPermissionList;
	std::vector<int> AccountQuests;
	std::map<std::string, Achievements::Achievement> Achievements;

	int MaxCharacters;

	// Transient stuff (not store in character data)
	bool DueDailyRewards;	// When set to true, when login has completely finished the player will be given their daily rewards
	HTTPD::SiteSession SiteSession;


	bool HasBuildZone(BuildPermissionArea &bpa);

	void AddAchievement(std::string achievement);
	bool ExpandCharacterSlots();
	bool ExpandDeliveryBoxes();
	void ClearAll(void);
	void FillAuthorizationHash(const char *hash);
	void FillRegistrationKey(const char *key);
	bool SetPermission(short filterType, const char *name, bool value);
	bool HasPermission(short permissionSet, unsigned int permissionFlag);
	bool HasAccountCompletedQuest(int QuestID);
	int GetFreeCharacterSlot(void);
	bool CheckBuildPermission(int zoneID, int pagex, int pagey);
	bool CheckBuildPermissionAdv(int zoneID, int PageSize, float posx, float posz);
	bool HasCharacterID(int CDefID);

	void SetBan(int minutes);
	void ClearBan(void);

	void AdjustSessionLoginCount(short count);
	bool QualifyGarbage(bool force);
	void ExpireOn(unsigned long);

	static void GenerateClientPasswordHash(const char *username, const char *password, std::string &outputString);
	static void GenerateSaltedHash(const char *inputString, std::string &outputString);
	void GenerateAndApplyRegistrationKeyRecovery(void);
	bool MatchRecoveryKey(const char *type, const char *hash);
	void SetNewPassword(const char *username, const char *plainTextPassword);
	void SetNewRegistrationKey(const char *regkey);
	bool MatchRegistrationKey(const char *regkey);
	bool IsRegistrationKeyEmpty(void);
	void CheckRecoveryRegistrationKey(const char *regkey);
	int GetTotalAchievementObjectives();
	int GetTotalCompletedAchievements();
	void WriteToJSON(Json::Value &value);
	void ReadFromJSON(Json::Value &value);
	void UpdateExpiry();
private:
	unsigned long ExpireTime;  //The server time when the account will be ready for garbage deletion.
};

class FileReader;

class AccountQuickData: public AbstractEntity {
public:
	int mID;
	std::string mLoginName;
	std::string mLoginAuth;
	std::string mRegKey;
	std::string mGroveName;

	AccountQuickData()
	{
		mID = 0;
	}

	bool WriteEntity(AbstractEntityWriter *writer);
	bool ReadEntity(AbstractEntityReader *reader);
	bool EntityKeys(AbstractEntityReader *reader);

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

	void Add(int CDefID, const std::string &name);   //Add a data pair to both maps.
	void Remove(int CDefID);                  //Remove an ID from both maps.

	bool HasID(int CDefID);                   //Test if the ID exists.
	const char *GetNameByID(int CDefID);      //Get the character name of the corresponding ID.

	bool HasName(const std::string &name);           //Test if the character name exists.
	int GetIDByName(const std::string &name);        //Get the ID of the corresponding character name.

};

class AccountManager
{
public:
	typedef std::list<AccountData>::iterator ACCOUNT_ITERATOR;
	//typedef pair<int, std::string> USEDCHAR_PAIR;
	//typedef std::vector<USEDCHAR_PAIR> USEDCHAR_VECTOR;

	AccountManager();
	~AccountManager();
	void LoadAllData(void);
	void UnloadAllData(void);

	Platform_CriticalSection cs;  //Needed for external account management since the HTTP threads might be accessing this concurrently.

	static const int DEFAULT_CHARACTER_ID = -400000000;
	static const int CHARACTER_ID_INCREMENT = -8;

	std::list<AccountData> AccList;

	void AppendQuickData(AccountData *account, bool sync = false);
	AccountQuickData GetAccountQuickDataByUsername(const std::string &username);

	AccountData * GetActiveAccountByID(int accountID);
	AccountData * FetchIndividualAccount(int accountID);
	AccountData * LoadAccountID(int accountID);
	AccountData * ReloadAccountID(int accountID);
	void UnloadAccountID(int accountID);

	AccountData * GetValidLogin(const char *loginName, const char *loginAuth);
	void ResolveCharacters(void);

	void LoadKeyList(std::string fileName);
	void ImportKeys(void);
	void ImportKey(const char *key);
	STRINGLIST GetRegistrationKeys();
	bool PopRegistrationKey(const std::string &authKey);
	int CreateAccountFromService(const char *username);
	int CreateAccount(const char *username, const char *password, const char *regKey, const char *grovename);
	int ResetPassword(const char *username, const char *newpassword, const char *regKey, bool checkPermission);
	int AccountRecover(const char *username, const char *keypass, const char *type);
	bool ValidString(const char *str);
	bool ValidGroveString(std::string &nameToAdjust);
	AccountData * FetchAccountByUsername(const char *username);
	STRINGLIST MatchAccountNames(std::string globPattern);

	const char * GetErrorMessage(int message);
	int CheckAutoSave(bool force);
	int HasPendingMinorUpdates(void);
	void SaveIndividualAccount(AccountData *account, bool sync);

	//Placeholder character creation stuff.  Should probably be moved to
	//the character system when that's redone.
	int CreateCharacter(STRINGLIST &args, AccountData *accPtr, CharacterData &newChar);
	int GetNewCharacterID(void);
	void DeleteCharacter(int index, AccountData *accPtr);

	bool ValidCharacterName(const std::string &name);
	const char * GetCharacterErrorMessage(int message);

	void RunUpdateCycle(bool force);
	bool AcceptingLogins(void);
	int ValidateNameParts(const std::string &first, const std::string &last);
	std::string GenerateToken(int accountID, unsigned long ttl, int tokenType, int uses);
	AccessToken *GetToken(std::string token);

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
	Timer TimerGeneralUpdate;
	std::map<std::string, AccessToken*> Tokens;
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
	Permission_PasswordReset  = 0x20000000,     //The password can be reset on this account.c
	Permission_Builder        = 0x30000000,     //Can build anywhere.
	Permission_Veteran        = 0x40000000,     //Not actually used, but signals the player has been around since the previous major version.
	Permission_Developer      = 0x80000000,     //Developer (opens terrain editing).

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
extern UsedNameDatabase g_UsedNameDatabase;

void DeleteCharacterList(void);

int GetCharacterIndex(int ID);

int CheckLoginIndex(char *LoginName, char *LoginAuth);

#endif //ACCOUNT_H
