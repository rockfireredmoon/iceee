// Contains all basic configuration options for the server program.

#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>

using namespace std;

// External declarations and prototypes
extern int g_ProtocolVersion;
extern int g_AuthMode;
extern char g_AuthKey[];

extern int g_RouterPort;
extern int g_SimulatorPort;
extern int g_ThreadSleep;
extern int g_MainSleep;
extern int g_ErrorSleep;

extern char g_SimulatorAddress[128];
extern char g_BindAddress[128];
extern bool g_SimulatorLog;
extern bool g_GlobalLogging;

extern int g_DefX;
extern int g_DefY;
extern int g_DefZ;
extern int g_DefZone;
extern int g_DefRotation;

//For the HTTP server
extern char g_HTTPBaseFolder[512];
extern unsigned int g_HTTPListenPort;
extern unsigned int g_HTTPSListenPort;
extern std::string g_SSLCertificate;

extern unsigned long g_RebroadcastDelay;
extern unsigned long g_LocalActivityScanDelay;
extern int g_LocalActivityRange;

extern unsigned long g_SceneryAutosaveTime;

extern std::string g_HTTP404Header;
extern std::string g_HTTP404Message;
extern int g_HTTP404Redirect;

extern int g_ForceUpdateTime;
extern int g_ItemBindingTypeOverride;
extern int g_ItemArmorTypeOverride;
extern int g_ItemWeaponTypeOverride;

//New for Release 10
extern string g_MOTD_Name;
extern string g_MOTD_Channel;
extern string g_MOTD_Message;

//Internal variables, shouldn't be set in the config file
extern char g_WorkingDirectory[];
//extern long g_HTTP404FileDataSize;

extern const int g_JumpConstant;

struct StringKeyVal
{
	string key;
	string value;
};

enum AdministrativeBehaviorFlags
{
	ADMIN_BEHAVIOR_DEBUGPACKETS = 1,
	ADMIN_BEHAVIOR_VERIFYSPEED = 256,
	ADMIN_BEHAVIOR_TRACEGROUND = 128
};

enum LogLevel
{
	LOG_ALWAYS = -1,   //Some basic messages we'll always want to see for more important server operation info.
	LOG_NONE   = 0,
	LOG_CRITICAL,
	LOG_ERROR,
	LOG_WARNING,
	LOG_NORMAL,
	LOG_VERBOSE
};

/* For OAuth2 authentication (used for migrating PF accounts to TAW)
 * Each 'client' string consists of a Client ID (can be public), a
 * Client Secret (kept private) and an allowed redirect URL
 *
 * <clientId>^<clientSecret>^<redirectURL>
 */
class OAuth2Client
{
public:
	std::string ClientId;
	std::string ClientSecret;
	std::string RedirectURL;

	OAuth2Client() {
	}

	~OAuth2Client() {
	}
};

class GlobalConfigData
{
public:
	GlobalConfigData();
	~GlobalConfigData();
	std::string RemoteAuthenticationPassword;

	//These control whether messages from different components will be entered into the
	//central logging system. (see LogLevel values)
	//If the message provided is less than or equal to the specified LogLevel value, the message
	//will be added.

	int LogLevelHTTPBase;
	int LogLevelSimulatorBase;
	int LogLevelHTTPDistribute;

	int BuybackLimit;
	int ProperSceneryList;     //Should be set to 1.  If zero, this is basically like having a global fastload permission applied to all players, which is bad for normal players.
	bool RemotePasswordMatch(const char *value);
	int Upgrade;

	int HeartbeatIntervalMS;   //How frequent to send heartbeat messages.
	int HeartbeatAbortCount;   //How many skipped hearbeat response messages to trigger a disconnect.

	int WarpMovementBlockTime;

	bool IdleCheckVerification;  //Used to check idle states and to detect botting.  Disabled by default.  If this is true, players may be kicked if the conditions are met.
	int IdleCheckFrequency;      //Milliseconds between running idle checks for kicking.  Default is 20 minutes.
	int IdleCheckDistance;       //Unit of distance.  If characters have not moved at least this much since the last idle check, consider them to be idle.
	int IdleCheckCast;           //Number of ability activations from a location before triggering a verification.
	int IdleCheckCastInterval;   //Milliseconds that the next idle check will be hastened by whenever the CheckCast count is reached.
	int IdleCheckDistanceTolerance;  //Unit of distance.  Abilities used within this distance will be contributed toward the CheckCast count.

	int CapExperienceLevel;
	int CapExperienceAmount;

	int CapValourLevel;

	bool CustomAbilityMechanics;  //If true, certain abilities may be processed with custom mechanics differently than a classic official server might.

	bool SendLobbyHeartbeat;

	bool DebugPingServer;        //Determine whether the server should send diagnostic pings the the client (modded client required).
	bool DebugPingClient;        //Determine whether the client should be notified to send diagnostic pings to the server (modded client required).
	int DebugPingFrequency;      //Time between server and client diagnostic pings (milliseconds).
	int DebugPingClientPollInterval;  //How many client pings must pass before the client-side ping results are fetched.
	int DebugPingServerLogThreshold;  //Log all server ping times that exceed this time.
	int internalStatus_PingID;    //Used for internal purposes only.  Not a configurable value.
	int internalStatus_PingCount; //Used for internal purposes only.  Not a configurable value.

	int AprilFools;
	std::string AprilFoolsName;
	int AprilFoolsAccount;

	int HTTPDeleteDisconnectedTime; //Time (milliseconds) to wait before removing a disconnected (no longer useful) Distribute thread.
	int HTTPDeleteConnectedTime;    //Time (milliseconds) to wait before forcing a connected (but inactive) Distribute thread.
	int HTTPDeleteRecheckDelay;     //Time interval (milliseconds) between checking for inactive HTTP Distribute threads

	int PartyPositionSendInterval; //Time (milliseconds) between sending position updates to members outside of local range.

	int VaultDefaultSize;            //Number of vault slots that all characters have.  If characters have not purchased any slots at all, this amount will still be available.
	int VaultInitialPurchaseSize;    //Newly created characters will be given this many free slots (considered as purchased space).

	int GlobalMovementBonus;         //If nonzero, all objects placed into a instance (players, mobs, NPCs, etc) will gain this default modifier to run speed.
	bool AllowEliteMob;              //If true, mobs may spawn as elite variants.
	
	float DexBlockDivisor;           //Points of dexterity may provide a bonus chance to block physical attacks.
	float DexParryDivisor;           //Points of dexterity may provide a bonus chance to parry physical attacks.
	float DexDodgeDivisor;           //Points of dexterity may provide a bonus chance to dodge physical attacks.
	float SpiResistDivisor;          //Points of spirit may provide bonus resistance to certain elemental attacks.
	float PsyResistDivisor;          //Points of psyche may provide bonus resistance to certain elemental attacks.

	float NamedMobDropMultiplier;    //All mobs marked as named (see "ExtraData" field for CreatureDefinitions) will receive a drop rate bonus for randomized items.
	int NamedMobCreditDrops;       //All mobs marked as named (see "ExtraData" field for CreatureDefinitions) will drop credits (when the player is at or below level, bonus given for parties).

	int DebugPacketSendTrigger;        //Used only for debugging network issues, the interval of repeatedly unfinished send() attempts before noting a message in the log.
	int DebugPacketSendDelay;          //Experimental delay before attempting to resend data on a socket that doesn't seem to be operating correctly.
	bool DebugPacketSendMessage;       //If true, display a diagnostic message if the trigger is activated.

	int ForceMaxPacketSize;         //If set, outgoing packets will not exceed this many bytes per send() attempt.
	
	int SceneryAuditDelay;          //Time, in MS, to delay pending scenery audits from being written.
	bool SceneryAuditAllow;         //If false, scenery audits are globally disabled.

	bool MegaLootParty;					//For fun and testing, when 1, everything always drops, usually more than once.
	int LootMaxRandomizedLevel;         //The randomizer cannot generate loot above this level for typical mobs.
	int LootMaxRandomizedSpecialLevel;  //The randomizer cannot generate loot above this level for "special" mobs
	bool LootNamedMobSpecial;           //If true, named mobs are considered for the special item level cap.
	int LootMinimumMobRaritySpecial;    //The minimum quality level for a mob to be considered special.

	int HeroismQuestLevelTolerance;  //How many levels above the quest level that the player is allowed to be to receive full heroism.
	int HeroismQuestLevelPenalty;    //Points of heroism to lose per level if over the quest tolerance level.

	float ProgressiveDropRateBonusMult[4];   //Needs to hold rarities [0,1,2,3].  Additive amount to increase instance drop rates per kill by a creature of a certain rarity.
	float ProgressiveDropRateBonusMultMax;   //The maximum instance drop rate bonus from additive kills.
	float DropRateBonusMultMax;              //The maximum drop rate bonus multiplier that any kill may have.  This affects the absolute total after all drop rate calculations have been applied.

	bool UseIntegerHealth;                   //If true, object health is represented as 4 bytes instead of 2.  Requires a modded client!  By default the client uses 2 bytes.  Server and client must match or protocol is broken.
	bool UseMessageBox;                      //If true, send a message to the client that will appear in a popup box rather than a floating info message that disappears after a few seconds.  Requires a modded client.
	bool UseStopSwim;                        //If true, send a custom "stop swimming" notification to the client when sending reposition updates, like warps.  Requires a modded client for the custom event handler.
	std::string InvalidLoginMessage;         //The message string to send to the client if the account is wrong.

	bool VerifyMovement;                     //If true, attempt to validate client movement and report unexpected speeds, coordinates, and update intervals.  May issues some false reports, check for consistency.  Accounts with the 'admin' permission are never validated due to conflicts with the speed command. 
	bool DebugLogAIScriptUse;                //If true, AI Script ability use requests are printed to the log file to help determine which abilities are used.  May cause heavy log spam.

	unsigned int SquirrelGCCallCount;				 //How many function calls must be performed before Garbage Collection is triggered
	unsigned int SquirrelGCDelay;				 	 //How long the queue must be idle for before GC can go ahead.
	unsigned int SquirrelGCMaxDelay;				 	 //How long after reaching call count before GC is forced.
	int SquirrelVMStackSize;						// Initial VM stack size
	int SquirrelQueueSpeed;							// Global queue speed. This value is divide by the script 'speed' to get the event delay for that script

	bool PersistentBuffs;              //If true, active buffs will be saved and restored on next login
	bool PartyLoot;						// Whether to allow party loot

	bool FallDamage;					// If true, fall damage is enabled

	bool AccountCredits;				// If true, credits will be stored at the account level rather than per character and shared across all characters
	unsigned int NameChangeCost;					// Number of credits a last name change costs
	unsigned int MinPVPPlayerLootItems;				// Minimum number of items that will be dropped by the player after a PVP fight
	unsigned int MaxPVPPlayerLootItems;				// Minimum number of items that will be dropped by the player after a PVP fight

	std::string GitHubToken;			// GitHub personal access token for bug reports
	std::string ServiceAuthURL;			// URL to use for external authentication

	// Global SSL options
	bool SSLVerifyPeer;					// For SSL, the default for peer verification
	bool SSLVerifyHostname;				// For SSL, whether to verify hostname

	// Mail
	std::string SMTPHost;				// For emails, the SMTP host
	std::string SMTPUsername;			// For emails, the SMTP username
	std::string SMTPPassword;			// For emails, the SMTP password
	int SMTPPort;						// For emails, the SMTP port
	bool SMTPSSL;						// For emails, whether to use SSL
	std::string SMTPSender;				// For emails, the default sender address

	std::vector<OAuth2Client*> OAuth2Clients;
	bool LegacyAccounts;
	bool PublicAPI;
	bool DirectoryListing;
	bool HTTPKeepAlive;

	std::string LegacyServer;			// URL of server to transfer groves from
	std::string APIAuthentication;		// Username:Password to allow API authentication

	unsigned int ClanCost;
	bool Clans;

	int MaxAuctionHours;
	int MinAuctionHours;
	float PercentageCommisionPerHour;
	int MaxAuctionExpiredHours;

	unsigned long debugAdministrativeBehaviorFlags;
	void SetAdministrativeBehaviorFlag(unsigned long bitValue, bool state);
	bool HasAdministrativeBehaviorFlag(unsigned long bitValue);
};

extern GlobalConfigData g_Config;

void AppendString(std::string &dest, char *appendStr);
void LoadConfig(const char *filename);
bool CheckDefaultHTTPBaseFolder(void);
void SetHTTPBaseFolderToCurrent(void);
void LoadFileIntoString(std::string &dest, char *filename);
int SaveSession(const char *filename);
int LoadSession(const char *filename);
int LoadStringsFile(const char *filename, vector<string> &list);
int LoadStringKeyValFile(const char *filename, vector<StringKeyVal> &list);

#endif //CONFIG_H
