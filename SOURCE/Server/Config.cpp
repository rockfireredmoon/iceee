#include <string.h>
#include "Config.h"

#include "FileReader.h"
#include "Scenery2.h"  //for g_SceneryVars
#include "DirectoryAccess.h"
#include "Stats.h"
#include "util/Log.h"
#include "Util.h"

GlobalConfigData g_Config;

// *******************************  Globals  *******************************

//The protocol version used by the game.  The game uses this version to verify the client
//and is also used internally for some handling of legacy code and version-specific
//buffer formats.
//AuthMode can be either 0 (normal) or 1 (admin) mode.
//AuthKey is a string that is encrypted into the login credentials for extra security.
int g_ProtocolVersion = 33;
int g_AuthMode = 1;
char g_AuthKey[64] = "key";

//The router threads will attempt to open ports starting with this port number.
int g_RouterPort = 4242;

//Holds the prefix address of the router response.  When the auto response is sent,
//the port (g_SimulatorPort) will be appended to the outgoing string.
char g_SimulatorAddress[128] = { 0 };

//Holds the address to bind services to.  Allows mulitple servers to run on the
//same host (by assigning mulitple IPs).
char g_BindAddress[128] = { 0 };

//The simulator base thread will listen for incoming connections on this port.
int g_SimulatorPort = 4300;

// The time delay (milliseconds) between Sleep messages at the end of each thread.
// Smaller delays may increase CPU load with faster concurrent processing.
// Larger delays may decrease CPU load, but also increase latency.
int g_ThreadSleep = 1;
int g_MainSleep = 1;
int g_ErrorSleep = 5000;

unsigned long g_SceneryAutosaveTime = 30000;  //30 seconds

//For the HTTP server
unsigned int g_HTTPListenPort = 80;

#ifndef NO_SSL
unsigned int g_HTTPSListenPort = 0;
std::string g_SSLCertificate;
#endif

//Milliseconds between normal game time to rebroadcast creature and object definitions
unsigned long g_RebroadcastDelay = 25000;

//Milliseconds between rescanning which creatures are within local range of players.
unsigned long g_LocalActivityScanDelay = 3000;
int g_LocalActivityRange = 1200;  //600

int g_ForceUpdateTime = 1500; //Time after a character stops moving to force a character update.  Helps resync elevation if characters are walking on the edge of platform and other clients see someone dropping off the edge even though they didn't.

int g_ItemBindingTypeOverride = -1;
int g_ItemArmorTypeOverride = -1;
int g_ItemWeaponTypeOverride = -1;

//New for Release 10
string g_MOTD_Name = "";
string g_MOTD_Channel = "";
string g_MOTD_Message = "";

// ***************************  Internal Variables  **************************
char g_WorkingDirectory[256] = { 0 }; //If the size of this changes, need to change the parameter by _getcwd() in the main() function.
char g_Executable[512] = { 0 };

const int g_JumpConstant = 32767;
// *******************************  Functions  *******************************

/*
 void CheckNewLine(std::string &dest)
 {
 //Makes sure the string ends with a newline.
 static char newLine[] = "\r\n";
 int pos = dest.rfind(newLine);
 if(pos == string::npos)
 dest.append(newLine);
 else
 if(dest.rfind(newLine) != dest.size() - 2)
 dest.append(newLine);
 }
 */

bool GlobalConfigData::LoadConfig(const fs::path &filename) {
	bool oauthSet = false;

	//Loads the configuration options from the target file.  These are core options
	//required for the server to operate.

	FileReader lfr;
	if (lfr.OpenText(filename) != Err_OK) {
		return false;
	}
	static char Delimiter[] = { '=', 13, 10 };
	lfr.Delimiter = Delimiter;
	lfr.CommentStyle = Comment_Semi;

	while (lfr.FileOpen() == true) {
		int r = lfr.ReadLine();
		if (r > 0) {
			lfr.SingleBreak("=");
			char *NameBlock = lfr.BlockToString(0);
			if (strcmp(NameBlock, "ProtocolVersion") == 0) {
				g_ProtocolVersion = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "AuthMode") == 0) {
				g_AuthMode = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "AuthKey") == 0) {
				strncpy(g_AuthKey, lfr.BlockToString(1), sizeof(g_AuthKey) - 1);
			} else if (strcmp(NameBlock, "RouterPort") == 0) {
				g_RouterPort = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "SimulatorAddress") == 0) {
				strncpy(g_SimulatorAddress, lfr.BlockToString(1),
						sizeof(g_SimulatorAddress) - 1);
			} else if (strcmp(NameBlock, "HTTPAddress") == 0) {
				g_Config.HTTPAddress = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "BindAddress") == 0) {
				strncpy(g_BindAddress, lfr.BlockToString(1),
						sizeof(g_BindAddress) - 1);
			} else if (strcmp(NameBlock, "SimulatorPort") == 0) {
				g_SimulatorPort = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "ThreadSleep") == 0) {
				g_ThreadSleep = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "ErrorSleep") == 0) {
				g_ErrorSleep = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "MainSleep") == 0) {
				g_MainSleep = lfr.BlockToInt(1);
			}  else if (strcmp(NameBlock, "HTTPBaseFolder") == 0) {
				g_Config.HTTPBaseFolder = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "HTTPCARFolder") == 0) {
				g_Config.HTTPCARFolder = lfr.BlockToString(1);
			} else if (strcmp(NameBlock, "HTTPListenPort") == 0) {
				g_HTTPListenPort = lfr.BlockToInt(1);
			}
#ifndef NO_SSL
			else if(strcmp(NameBlock, "HTTPSListenPort") == 0)
			{
				g_HTTPSListenPort = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "SSLCertificate") == 0)
			{
				AppendString(g_SSLCertificate, lfr.BlockToStringC(1, 0));
			}
#endif
			else if (strcmp(NameBlock, "RebroadcastDelay") == 0) {
				g_RebroadcastDelay = lfr.BlockToULongC(1);
			} else if (strcmp(NameBlock, "SceneryAutosaveTime") == 0) {
				g_SceneryAutosaveTime = lfr.BlockToULongC(1);
			} else if (strcmp(NameBlock, "ForceUpdateTime") == 0) {
				g_ForceUpdateTime = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "ItemBindingTypeOverride") == 0) {
				g_ItemBindingTypeOverride = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "ItemArmorTypeOverride") == 0) {
				g_ItemArmorTypeOverride = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "ItemWeaponTypeOverride") == 0) {
				g_ItemWeaponTypeOverride = lfr.BlockToInt(1);
			} else if (strcmp(NameBlock, "MOTD_Name") == 0) {
				g_MOTD_Name = lfr.BlockToStringC(1, 0);
			} else if (strcmp(NameBlock, "MOTD_Channel") == 0) {
				g_MOTD_Channel = lfr.BlockToStringC(1, 0);
			} else if (strcmp(NameBlock, "RemoteAuthenticationPassword") == 0) {
				g_Config.RemoteAuthenticationPassword = lfr.BlockToStringC(1,
						0);
			} else if (strcmp(NameBlock, "ProperSceneryList") == 0) {
				g_Config.ProperSceneryList = lfr.BlockToIntC(1);
			}
			else if (strcmp(NameBlock, "HeartbeatIntervalMS") == 0)
				g_Config.HeartbeatIntervalMS = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "HeartbeatAbortCount") == 0)
				g_Config.HeartbeatAbortCount = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "WarpMovementBlockTime") == 0)
				g_Config.WarpMovementBlockTime = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "IdleCheckVerification") == 0)
				g_Config.IdleCheckVerification = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "IdleCheckFrequency") == 0)
				g_Config.IdleCheckFrequency = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "IdleCheckDistance") == 0)
				g_Config.IdleCheckDistance = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "IdleCheckCast") == 0)
				g_Config.IdleCheckCast = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "IdleCheckCastInterval") == 0)
				g_Config.IdleCheckCastInterval = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "IdleCheckDistanceTolerance") == 0)
				g_Config.IdleCheckDistanceTolerance = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SendLobbyHeartbeat") == 0)
				g_Config.UseLobbyHeartbeat = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "DebugPingServer") == 0)
				g_Config.DebugPingServer = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "DebugPingClient") == 0)
				g_Config.DebugPingClient = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "DebugPingFrequency") == 0)
				g_Config.DebugPingFrequency = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "DebugPingClientPollInterval") == 0)
				g_Config.DebugPingClientPollInterval = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "DebugPingServerLogThreshold") == 0)
				g_Config.DebugPingServerLogThreshold = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "HTTPBacklog") == 0)
				g_Config.HTTPBacklog = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "HTTPThreads") == 0)
				g_Config.HTTPThreads = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "HTTPConnectionQueue") == 0)
				g_Config.HTTPConnectionQueue = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "HTTPAuthDomainCheck") == 0)
				g_Config.HTTPAuthDomainCheck = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "HTTPAuthDomain") == 0)
				g_Config.HTTPAuthDomain = lfr.BlockToString(1);
			else if (strcmp(NameBlock, "PartyPositionSendInterval") == 0)
				g_Config.PartyPositionSendInterval = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "RedisWorkers") == 0)
				g_Config.RedisWorkers = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SchedulerThreads") == 0)
				g_Config.SchedulerThreads = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "DebugPacketSendTrigger") == 0)
				g_Config.DebugPacketSendTrigger = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "DebugPacketSendDelay") == 0)
				g_Config.DebugPacketSendDelay = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "DebugPacketSendMessage") == 0)
				g_Config.DebugPacketSendMessage = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "ForceMaxPacketSize") == 0)
				g_Config.ForceMaxPacketSize = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SceneryAuditDelay") == 0)
				g_Config.SceneryAuditDelay = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SceneryAuditAllow") == 0)
				g_Config.SceneryAuditAllow = lfr.BlockToBoolC(1);
			else if (strcmp(NameBlock, "UseIntegerHealth") == 0) {
				g_Config.UseIntegerHealth = lfr.BlockToBool(1);
				StatManager::SetHealthToInteger(g_Config.UseIntegerHealth);
			} else if (strcmp(NameBlock, "UseMessageBox") == 0)
				g_Config.UseMessageBox = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "UseStopSwim") == 0)
				g_Config.UseStopSwim = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "UseWeather") == 0)
				g_Config.UseWeather = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "UseUserAgentProtection") == 0)
				g_Config.UseUserAgentProtection = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "InvalidLoginMessage") == 0)
				g_Config.InvalidLoginMessage = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "MaintenanceMessage") == 0)
				g_Config.MaintenanceMessage = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "GitHubToken") == 0)
				g_Config.GitHubToken = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "ServiceAuthURL") == 0)
				g_Config.ServiceAuthURL = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "VerifyMovement") == 0)
				g_Config.VerifyMovement = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "VerifySpeed") == 0)
				g_Config.VerifySpeed = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "DebugLogAIScriptUse") == 0)
				g_Config.DebugLogAIScriptUse = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "SquirrelGCCallCount") == 0)
				g_Config.SquirrelGCCallCount = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SquirrelGCDelay") == 0)
				g_Config.SquirrelGCDelay = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SquirrelGCMaxDelay") == 0)
				g_Config.SquirrelGCMaxDelay = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SquirrelVMStackSize") == 0)
				g_Config.SquirrelVMStackSize = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SquirrelQueueSpeed") == 0)
				g_Config.SquirrelQueueSpeed = lfr.BlockToIntC(1);
			else if (strcmp(NameBlock, "SSLVerifyPeer") == 0)
				g_Config.SSLVerifyPeer = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "SSLVerifyHostname") == 0)
				g_Config.SSLVerifyHostname = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "SMTPHost") == 0)
				g_Config.SMTPHost = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "SMTPUsername") == 0)
				g_Config.SMTPUsername = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "SMTPPassword") == 0)
				g_Config.SMTPPassword = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "SMTPPort") == 0)
				g_Config.SMTPPort = lfr.BlockToInt(1);
			else if (strcmp(NameBlock, "SMTPSSL") == 0)
				g_Config.SMTPSSL = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "SMTPSender") == 0)
				g_Config.SMTPSender = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "LegacyAccounts") == 0)
				g_Config.LegacyAccounts = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "PublicAPI") == 0)
				g_Config.PublicAPI = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "DirectoryListing") == 0)
				g_Config.DirectoryListing = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "HTTPKeepAlive") == 0)
				g_Config.HTTPKeepAlive = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "HTTPServeAssets") == 0)
				g_Config.HTTPServeAssets = lfr.BlockToBool(1);
			else if (strcmp(NameBlock, "LegacyServer") == 0)
				g_Config.LegacyServer = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "SiteServiceUsername") == 0)
				g_Config.SiteServiceUsername = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "SiteServicePassword") == 0)
				g_Config.SiteServicePassword = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "APIAuthentication") == 0)
				g_Config.APIAuthentication = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "OAuth2Client") == 0) {
				if(!oauthSet) {
					oauthSet = true;
					g_Config.OAuth2Clients.clear();
				}
				STRINGLIST output;
				Util::Split(lfr.BlockToString(1), "|", output);
				if (output.size() == 3) {
					OAuth2Client *c = new OAuth2Client();
					c->ClientId = output[0];
					c->ClientSecret = output[1];
					c->RedirectURL = output[2];
					g_Config.OAuth2Clients.push_back(c);
				} else {
					g_Logs.data->error("Invalid OAuth2Client string [%v] in config file [%v]",
							lfr.BlockToString(0), filename);
				}
			}
			else if (strcmp(NameBlock, "ShutdownHandlerScript") == 0)
				g_Config.ShutdownHandlerScript = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "StaticDataPath") == 0)
				g_Config.StaticDataPath = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "VariableDataPath") == 0)
				g_Config.VariableDataPath = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "TmpDataPath") == 0)
				g_Config.TmpDataPath = lfr.BlockToStringC(1, 0);
			else if (strcmp(NameBlock, "LogPath") == 0)
				g_Config.LogPath = lfr.BlockToStringC(1, 0);
			else {
				g_Logs.data->error("Unknown identifier [%v] in config file [%v]",
						lfr.BlockToString(0), filename);
			}
		}
	}
	lfr.CloseCurrent();
	return true;
}

GlobalConfigData::GlobalConfigData() {
	//Set defaults for config settings.
	ProperSceneryList = 1;

	HeartbeatIntervalMS = 10000;
	HeartbeatAbortCount = -1; //Requires a modded client to perform this task, so disable by default.

	WarpMovementBlockTime = 2000;

	IdleCheckVerification = false;
	IdleCheckFrequency = 1200000; //20 minutes
	IdleCheckDistance = 50;
	IdleCheckCast = 6;
	IdleCheckCastInterval = 60000; //1 minute
	IdleCheckDistanceTolerance = 100;

	UseLobbyHeartbeat = false; //Requires a modified client to receive and handle the heartbeat message in Lobby mode.

	DebugPingServer = false;     //Modded client required.
	DebugPingClient = false;     //Modded client required.
	DebugPingFrequency = 1000;   //Milliseconds.
	DebugPingClientPollInterval = 60;  //Number of pings between status polling.
	DebugPingServerLogThreshold = 2000; //Log all pings higher than this to file.
	DebugPacketSendTrigger = 128;
	DebugPacketSendDelay = 0;
	DebugPacketSendMessage = false;
	ForceMaxPacketSize = 1000;
	internalStatus_PingID = 0;
	internalStatus_PingCount = 0;

	HTTPBacklog = 200;
	HTTPThreads = 50;
	HTTPConnectionQueue = 20;
	HTTPAuthDomainCheck = false;
	HTTPAuthDomain= "";
	PartyPositionSendInterval = 15000;

	SceneryAuditDelay = 120000;  //2 minutes;
	SceneryAuditAllow = true;

	UseIntegerHealth = true;
	UseMessageBox = true;
	UseStopSwim = true;
	UseWeather = true;
	UseUserAgentProtection = true;

	VerifyMovement = false;
	VerifySpeed = false;
	DebugLogAIScriptUse = false;

	SquirrelGCCallCount = 1000;
	SquirrelGCDelay = 10000;
	SquirrelGCMaxDelay = 60000;
	SquirrelVMStackSize = 512;
	SquirrelQueueSpeed = 10000;

	SSLVerifyPeer = true;
	SSLVerifyHostname = true;

	SMTPHost = "";
	SMTPUsername = "";
	SMTPPassword = "";
	SMTPPort = 25;
	SMTPSSL = false;
	SMTPSender = "";

	PublicAPI = true;
	LegacyAccounts = false;
	LegacyServer = "";
	APIAuthentication = "";
	RedisWorkers = 10;
	SchedulerThreads = 4;

	InvalidLoginMessage = "Account not found.  Check username and password.";
	MaintenanceMessage = "";

	HTTPKeepAlive = false; // TODO not yet supported by client
	HTTPServeAssets = true;
	DirectoryListing = false;

	SiteServiceUsername = "";
	SiteServicePassword = "";

	ShutdownHandlerScript = "";

	StaticDataPath = "Static";
	VariableDataPath = "Variable";
	TmpDataPath = "Tmp";
	LogPath = "Logs";

	STRINGLIST l;
	Util::Split(LOCALCONFIGDIR, ":", l);
	for(auto i = l.begin(); i != l.end(); ++i) {
		LocalConfigurationPath.push_back(fs::path(*i));
	}
}

GlobalConfigData::~GlobalConfigData() {
}

fs::path GlobalConfigData::ResolveStaticDataPath() {
	return ResolvePath(StaticDataPath);
}

std::vector<fs::path> GlobalConfigData::ResolveLocalConfigurationPath() {
	return LocalConfigurationPath;
}

fs::path GlobalConfigData::ResolveHTTPBasePath() {
	return ResolvePath(HTTPBaseFolder);
}

fs::path GlobalConfigData::ResolveHTTPCARPath() {
	return ResolvePath(HTTPCARFolder);
}

fs::path GlobalConfigData::ResolveVariableDataPath() {
	return ResolvePath(VariableDataPath);
}

fs::path GlobalConfigData::ResolveTmpDataPath() {
	return ResolvePath(TmpDataPath);
}

fs::path GlobalConfigData::ResolveLogPath() {
	return ResolvePath(LogPath);
}

fs::path GlobalConfigData::ResolvePath(const fs::path &path) {
	return path;
}

std::string GlobalConfigData::ResolveSimulatorAddress() {
	if(strlen(g_SimulatorAddress) == 0) {
		// TODO - get assigned IP address on server
		// This is not that easy
		return "127.0.0.1";
	}
	else
		return g_SimulatorAddress;
}

std::string GlobalConfigData::ResolveHTTPAddress(const std::string &simAddress) {
	if(HTTPAddress == "") {
#ifndef NO_SSL
		if(g_HTTPSListenPort > 0)
		{
			if(g_HTTPSListenPort == 443)
				return Util::Format("https://%s/Release/Current", simAddress.c_str());
			else
				return Util::Format("https://%s:%d/Release/Current", simAddress.c_str(), g_HTTPSListenPort);
		}
#endif
		if(g_HTTPListenPort == 80)
			return Util::Format("http://%s/Release/Current", simAddress.c_str());
		else {
			return Util::Format("http://%s:%d/Release/Current", simAddress.c_str(), g_HTTPListenPort);
		}
	}
	else
		return HTTPAddress;
}

bool GlobalConfigData::RemotePasswordMatch(const char *value) {
	if (RemoteAuthenticationPassword.compare(value) == 0)
		return true;
	return false;
}
