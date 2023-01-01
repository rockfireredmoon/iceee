// Contains all basic configuration options for the server program.

#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

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

//For the HTTP server
extern unsigned int g_HTTPListenPort;
extern unsigned int g_HTTPSListenPort;
extern string g_SSLCertificate;

extern unsigned long g_RebroadcastDelay;
extern unsigned long g_LocalActivityScanDelay;
extern int g_LocalActivityRange;

extern unsigned long g_SceneryAutosaveTime;

extern int g_ForceUpdateTime;
extern int g_ItemBindingTypeOverride;
extern int g_ItemArmorTypeOverride;
extern int g_ItemWeaponTypeOverride;

//New for Release 10
extern string g_MOTD_Name;
extern string g_MOTD_Channel;

//Internal variables, shouldn't be set in the config file
extern char g_WorkingDirectory[];
extern char g_Executable[];

extern const int g_JumpConstant;

struct StringKeyVal
{
	string key;
	string value;
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
	string ClientId;
	string ClientSecret;
	string RedirectURL;

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

	fs::path HTTPBaseFolder;
	fs::path HTTPCARFolder;

	string RemoteAuthenticationPassword;

	int ProperSceneryList;     //Should be set to 1.  If zero, this is basically like having a global fastload permission applied to all players, which is bad for normal players.
	bool RemotePasswordMatch(const char *value);

	int HeartbeatIntervalMS;   //How frequent to send heartbeat messages.
	int HeartbeatAbortCount;   //How many skipped hearbeat response messages to trigger a disconnect.

	int WarpMovementBlockTime;

	bool IdleCheckVerification;  //Used to check idle states and to detect botting.  Disabled by default.  If this is true, players may be kicked if the conditions are met.
	int IdleCheckFrequency;      //Milliseconds between running idle checks for kicking.  Default is 20 minutes.
	int IdleCheckDistance;       //Unit of distance.  If characters have not moved at least this much since the last idle check, consider them to be idle.
	int IdleCheckCast;           //Number of ability activations from a location before triggering a verification.
	int IdleCheckCastInterval;   //Milliseconds that the next idle check will be hastened by whenever the CheckCast count is reached.
	int IdleCheckDistanceTolerance;  //Unit of distance.  Abilities used within this distance will be contributed toward the CheckCast count.

	bool DebugPingServer;        //Determine whether the server should send diagnostic pings the the client (modded client required).
	bool DebugPingClient;        //Determine whether the client should be notified to send diagnostic pings to the server (modded client required).
	int DebugPingFrequency;      //Time between server and client diagnostic pings (milliseconds).
	int DebugPingClientPollInterval;  //How many client pings must pass before the client-side ping results are fetched.
	int DebugPingServerLogThreshold;  //Log all server ping times that exceed this time.
	int internalStatus_PingID;    //Used for internal purposes only.  Not a configurable value.
	int internalStatus_PingCount; //Used for internal purposes only.  Not a configurable value.

	int HTTPBacklog; //Maximum number of connections waiting to be accepted by the server operating system.
	int HTTPThreads;    //Number of worker threads.
	int HTTPConnectionQueue;     //Maximum number of accepted connections waiting to be dispatched by a worker thread.
	bool HTTPAuthDomainCheck;    //Check server domain
	string HTTPAuthDomain;    //Server domain

	int PartyPositionSendInterval; //Time (milliseconds) between sending position updates to members outside of local range.

	int DebugPacketSendTrigger;        //Used only for debugging network issues, the interval of repeatedly unfinished send() attempts before noting a message in the log.
	int DebugPacketSendDelay;          //Experimental delay before attempting to resend data on a socket that doesn't seem to be operating correctly.
	bool DebugPacketSendMessage;       //If true, display a diagnostic message if the trigger is activated.

	int ForceMaxPacketSize;         //If set, outgoing packets will not exceed this many bytes per send() attempt.
	
	int SceneryAuditDelay;          //Time, in MS, to delay pending scenery audits from being written.
	bool SceneryAuditAllow;         //If false, scenery audits are globally disabled.

	bool UseIntegerHealth;                   // If true, object health is represented as 4 bytes instead of 2.  Requires a modded client!  By default the client uses 2 bytes.  Server and client must match or protocol is broken.
	bool UseMessageBox;                      // If true, send a message to the client that will appear in a popup box rather than a floating info message that disappears after a few seconds.  Requires a modded client.
	bool UseStopSwim;                        // If true, send a custom "stop swimming" notification to the client when sending reposition updates, like warps.  Requires a modded client for the custom event handler.
	bool UseWeather;					     // If true, weather systems will be enabled
	bool UseUserAgentProtection;			 // Only allow HTTP calls from the Sparkplayer client user agent - 'ire3d(VERSION)'
	bool UseLobbyHeartbeat;					 // Send a heartbeat while in lobby

	string InvalidLoginMessage;         //The message string to send to the client if the account is wrong.
	string MaintenanceMessage;          //The message string to send to the client if the server is in maintenance mode (sages and admins only).

	bool VerifyMovement;                     //If true, attempt to validate client movement and report unexpected speeds, coordinates, and update intervals.  May issues some false reports, check for consistency.  Accounts with the 'admin' permission are never validated due to conflicts with the speed command. 
	bool VerifySpeed;                        //If true, attempt to validate client speed
	bool DebugLogAIScriptUse;                //If true, AI Script ability use requests are printed to the log file to help determine which abilities are used.  May cause heavy log spam.

	unsigned int SquirrelGCCallCount;				 //How many function calls must be performed before Garbage Collection is triggered
	unsigned int SquirrelGCDelay;				 	 //How long the queue must be idle for before GC can go ahead.
	unsigned int SquirrelGCMaxDelay;				 	 //How long after reaching call count before GC is forced.
	unsigned long SquirrelVMStackSize;						// Initial VM stack size
	int SquirrelQueueSpeed;							// Global queue speed. This value is divide by the script 'speed' to get the event delay for that script

	string GitHubToken;			// GitHub personal access token for bug reports
	string ServiceAuthURL;			// URL to use for external authentication

	// Global SSL options
	bool SSLVerifyPeer;					// For SSL, the default for peer verification
	bool SSLVerifyHostname;				// For SSL, whether to verify hostname

	// Mail
	string SMTPHost;				// For emails, the SMTP host
	string SMTPUsername;			// For emails, the SMTP username
	string SMTPPassword;			// For emails, the SMTP password
	int SMTPPort;						// For emails, the SMTP port
	bool SMTPSSL;						// For emails, whether to use SSL
	string SMTPSender;				// For emails, the default sender address

	//Holds the address HTTP requests will be made to, from a clients perspect. If not set
	//this will be computed from the simulator address and HTTP port (if any).
	//This is mainly used for sim switching, informing the client of the (possibly new)
	//address that should now be used for HTTP requests
	string HTTPAddress;

	vector<OAuth2Client*> OAuth2Clients;
	bool LegacyAccounts;
	bool PublicAPI;
	bool DirectoryListing;
	bool HTTPKeepAlive;
	bool HTTPServeAssets;
	unsigned int RestartExitStatus;
	unsigned int RedisWorkers;
	unsigned int SchedulerThreads;

	string LegacyServer;			// URL of server to transfer groves from
	string APIAuthentication;		// Username:Password to allow API authentication
	string SiteServiceUsername;
	string SiteServicePassword;
	vector<fs::path> LocalConfigurationPath;

	fs::path ShutdownHandlerScript;
	fs::path StaticDataPath;			// Location of static data (not editable in game).
	fs::path VariableDataPath;		// Location of variable data (editable in game).
	fs::path TmpDataPath;		// Location of temporary data (editable in game).
	fs::path LogPath;				// Location of logs other than Easylogging output (Civet for example)

	fs::path ResolveStaticDataPath();
	fs::path ResolveVariableDataPath();
	fs::path ResolveTmpDataPath();
	fs::path ResolveHTTPBasePath();
	fs::path ResolveHTTPCARPath();
	vector<fs::path> ResolveLocalConfigurationPath();
	fs::path ResolveLogPath();
	string ResolveSimulatorAddress();
	string ResolveHTTPAddress(const string &simAddress);

	bool LoadConfig(const fs::path &filename);

private:
	fs::path ResolvePath(const fs::path &path);
};

extern GlobalConfigData g_Config;

#endif //CONFIG_H
