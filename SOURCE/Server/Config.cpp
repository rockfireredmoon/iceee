#include <string.h>
#include "Config.h"
#include "StringList.h"
#include "FileReader.h"
#include "Scenery2.h"  //for g_SceneryVars
#include "DirectoryAccess.h"
#include "ZoneDef.h"
#include "Stats.h"

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
char g_SimulatorAddress[128] = {0};

//Holds the address to bind services to.  Allows mulitple servers to run on the
//same host (by assigning mulitple IPs).
char g_BindAddress[128] = {0};

//The simulator base thread will listen for incoming connections on this port.
int g_SimulatorPort = 4300;

// The time delay (milliseconds) between Sleep messages at the end of each thread.
// Smaller delays may increase CPU load with faster concurrent processing.
// Larger delays may decrease CPU load, but also increase latency.
int g_ThreadSleep = 1;
int g_MainSleep = 1;
int g_ErrorSleep = 5000;


//If true, messages received by the simulator are output to the status window. 
bool g_SimulatorLog = false;

bool g_GlobalLogging = true;

int g_DefX = 0;                  //Default X coordinate on the map
int g_DefY = 0;
int g_DefZ = 0;                  //Default Z coordinate on the map

int g_DefZone = 81;              //Default instance ID to log into

unsigned long g_SceneryAutosaveTime = 300000;  //5 minutes

//For the HTTP server
char g_HTTPBaseFolder[512] = {0};
int g_HTTPListenPort = 80;


//Milliseconds between normal game time to rebroadcast creature and object definitions
unsigned long g_RebroadcastDelay = 25000;

//Milliseconds between rescanning which creatures are within local range of players.
unsigned long g_LocalActivityScanDelay = 3000;
int g_LocalActivityRange = 1200;  //600

std::string g_HTTP404Header;
std::string g_HTTP404Message;
int g_HTTP404Redirect = 0;
//char * g_HTTP404FileData = NULL;

int g_ForceUpdateTime = 1500;    //Time after a character stops moving to force a character update.  Helps resync elevation if characters are walking on the edge of platform and other clients see someone dropping off the edge even though they didn't.

int g_ItemBindingTypeOverride = -1;
int g_ItemArmorTypeOverride = -1;
int g_ItemWeaponTypeOverride = -1;

//New for Release 10
string g_MOTD_Name = "";
string g_MOTD_Channel = "";
string g_MOTD_Message = "";

// ***************************  Internal Variables  **************************
char g_WorkingDirectory[256] = {0};   //If the size of this changes, need to change the parameter by _getcwd() in the main() function.
//long g_HTTP404FileDataSize = 0;



const int g_JumpConstant = 32767;
// *******************************  Functions  *******************************


void AppendString(std::string &dest, char *appendStr)
{
	if(dest.size() == 0)
	{
		dest = appendStr;
	}
	else
	{
		dest.append("\r\n");
		dest.append(appendStr);
	}
}

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

void LoadConfig(const char *filename)
{
	//Loads the configuration options from the target file.  These are core options
	//required for the server to operate.
	
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormatW(MSG_SHOW, "Could not open configuration file: %s", filename);
		return;
	}
	static char Delimiter[] = {'=', 13, 10};
	lfr.Delimiter = Delimiter;
	lfr.CommentStyle = Comment_Semi;

	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			lfr.SingleBreak("=");
			char *NameBlock = lfr.BlockToString(0);
			if(strcmp(NameBlock, "ProtocolVersion") == 0)
			{
				g_ProtocolVersion = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "AuthMode") == 0)
			{
				g_AuthMode = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "AuthKey") == 0)
			{
				strncpy(g_AuthKey, lfr.BlockToString(1), sizeof(g_AuthKey) - 1);
			}
			else if(strcmp(NameBlock, "RouterPort") == 0)
			{
				g_RouterPort = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "SimulatorAddress") == 0)
			{
				#ifdef LOCALHOST
				g_Log.AddMessageFormatW(MSG_SHOW, "SimulatorAddress %s in %s is not supported in this edition.", lfr.BlockToString(0), filename);
				#else
				strncpy(g_SimulatorAddress, lfr.BlockToString(1), sizeof(g_SimulatorAddress) - 1);
				#endif
			}
			else if(strcmp(NameBlock, "BindAddress") == 0)
			{
				#ifdef LOCALHOST
				g_Log.AddMessageFormatW(MSG_SHOW, "BindAddress %s in %s is not supported in this edition.", lfr.BlockToString(0), filename);
				#else
				strncpy(g_BindAddress, lfr.BlockToString(1), sizeof(g_BindAddress) - 1);
				#endif
			}
			else if(strcmp(NameBlock, "SimulatorPort") == 0)
			{
				g_SimulatorPort = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ThreadSleep") == 0)
			{
				g_ThreadSleep = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ErrorSleep") == 0)
			{
				g_ErrorSleep = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "MainSleep") == 0)
			{
				g_MainSleep = lfr.BlockToInt(1);
			}	
			else if(strcmp(NameBlock, "DefX") == 0)
			{
				g_DefX = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "DefY") == 0)
			{
				g_DefY = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "DefZ") == 0)
			{
				g_DefZ = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "DefZone") == 0)
			{
				g_DefZone = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "HTTPBaseFolder") == 0)
			{
				Util::SafeCopy(g_HTTPBaseFolder, lfr.BlockToString(1), sizeof(g_HTTPBaseFolder));
				if(CheckDefaultHTTPBaseFolder() == true)
					SetHTTPBaseFolderToCurrent();
			}
			else if(strcmp(NameBlock, "SimulatorLog") == 0)
			{
				g_SimulatorLog = lfr.BlockToBool(1);
			}
			else if(strcmp(NameBlock, "GlobalLogging") == 0)
			{
				g_GlobalLogging = lfr.BlockToBool(1);
			}
			else if(strcmp(NameBlock, "LogLevelHTTPBase") == 0)
				g_Config.LogLevelHTTPBase = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "LogLevelSimulatorBase") == 0)
				g_Config.LogLevelSimulatorBase = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "LogLevelHTTPDistribute") == 0)
				g_Config.LogLevelHTTPDistribute = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "HTTPListenPort") == 0)
			{
				g_HTTPListenPort = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "RebroadcastDelay") == 0)
			{
				g_RebroadcastDelay = lfr.BlockToULongC(1);
			}
			else if(strcmp(NameBlock, "SceneryAutosaveTime") == 0)
			{
				g_SceneryAutosaveTime = lfr.BlockToULongC(1);
			}
			else if(strcmp(NameBlock, "HTTP404Header") == 0)
			{
				AppendString(g_HTTP404Header, lfr.BlockToStringC(1, 0));
			}
			else if(strcmp(NameBlock, "HTTP404Redirect") == 0)
			{
				g_HTTP404Redirect = lfr.BlockToIntC(1);
			}
			else if(strcmp(NameBlock, "HTTP404Message") == 0)
			{
				AppendString(g_HTTP404Message, lfr.BlockToStringC(1, 0));
			}
			else if(strcmp(NameBlock, "HTTP404MessageFile") == 0)
			{
				LoadFileIntoString(g_HTTP404Message, lfr.BlockToStringC(1, 0));
			}
			else if(strcmp(NameBlock, "BaseSceneryID") == 0)
			{
				g_SceneryVars.BaseSceneryID = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ForceUpdateTime") == 0)
			{
				g_ForceUpdateTime = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ItemBindingTypeOverride") == 0)
			{
				g_ItemBindingTypeOverride = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ItemArmorTypeOverride") == 0)
			{
				g_ItemArmorTypeOverride = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "ItemWeaponTypeOverride") == 0)
			{
				g_ItemWeaponTypeOverride = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "MOTD_Name") == 0)
			{
				g_MOTD_Name = lfr.BlockToStringC(1, 0);
			}
			else if(strcmp(NameBlock, "MOTD_Channel") == 0)
			{
				g_MOTD_Channel = lfr.BlockToStringC(1, 0);
			}
			else if(strcmp(NameBlock, "MOTD_Message") == 0)
			{
				g_MOTD_Message = lfr.BlockToStringC(1, 0);
			}
			else if(strcmp(NameBlock, "MessageLevel") == 0)
			{
				g_Log.Filter = lfr.BlockToInt(1);
			}
			else if(strcmp(NameBlock, "RemoteAuthenticationPassword") == 0)
			{
				g_Config.RemoteAuthenticationPassword = lfr.BlockToStringC(1, 0);
			}
			else if(strcmp(NameBlock, "ProperSceneryList") == 0)
			{
				g_Config.ProperSceneryList = lfr.BlockToIntC(1);
			}
			else if(strcmp(NameBlock, "BuybackLimit") == 0)
			{
				g_Config.BuybackLimit = lfr.BlockToIntC(1);
			}
			else if(strcmp(NameBlock, "Upgrade") == 0)
				g_Config.Upgrade = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HeartbeatIntervalMS") == 0)
				g_Config.HeartbeatIntervalMS = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HeartbeatAbortCount") == 0)
				g_Config.HeartbeatAbortCount = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "WarpMovementBlockTime") == 0)
				g_Config.WarpMovementBlockTime = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "IdleCheckVerification") == 0)
				g_Config.IdleCheckVerification = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "IdleCheckFrequency") == 0)
				g_Config.IdleCheckFrequency = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "IdleCheckDistance") == 0)
				g_Config.IdleCheckDistance = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "IdleCheckCast") == 0)
				g_Config.IdleCheckCast = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "IdleCheckCastInterval") == 0)
				g_Config.IdleCheckCastInterval = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "IdleCheckDistanceTolerance") == 0)
				g_Config.IdleCheckDistanceTolerance = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "EnvironmentCycle") == 0)
				g_EnvironmentCycleManager.ApplyConfig(lfr.BlockToStringC(1, 0));
			else if(strcmp(NameBlock, "SendLobbyHeartbeat") == 0)
				g_Config.SendLobbyHeartbeat = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "CapExperienceLevel") == 0)
				g_Config.CapExperienceLevel = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "CapExperienceAmount") == 0)
				g_Config.CapExperienceAmount = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "CustomAbilityMechanics") == 0)
				g_Config.CustomAbilityMechanics = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "DebugPingServer") == 0)
				g_Config.DebugPingServer = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "DebugPingClient") == 0)
				g_Config.DebugPingClient = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "DebugPingFrequency") == 0)
				g_Config.DebugPingFrequency = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "DebugPingClientPollInterval") == 0)
				g_Config.DebugPingClientPollInterval = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "DebugPingServerLogThreshold") == 0)
				g_Config.DebugPingServerLogThreshold = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HTTPDeleteConnectedTime") == 0)
				g_Config.HTTPDeleteConnectedTime = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HTTPDeleteDisconnectedTime") == 0)
				g_Config.HTTPDeleteDisconnectedTime = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HTTPDeleteRecheckDelay") == 0)
				g_Config.HTTPDeleteRecheckDelay = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "PartyPositionSendInterval") == 0)
				g_Config.PartyPositionSendInterval = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "VaultDefaultSize") == 0)
				g_Config.VaultDefaultSize = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "VaultInitialPurchaseSize") == 0)
				g_Config.VaultInitialPurchaseSize = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "DebugPacketSendTrigger") == 0)
				g_Config.DebugPacketSendTrigger = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "DebugPacketSendDelay") == 0)
				g_Config.DebugPacketSendDelay = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "DebugPacketSendMessage") == 0)
				g_Config.DebugPacketSendMessage = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "GlobalMovementBonus") == 0)
				g_Config.GlobalMovementBonus = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "AllowEliteMob") == 0)
				g_Config.AllowEliteMob = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "DexBlockDivisor") == 0)
				g_Config.DexBlockDivisor = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "DexParryDivisor") == 0)
				g_Config.DexParryDivisor = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "DexDodgeDivisor") == 0)
				g_Config.DexDodgeDivisor = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "SpiResistDivisor") == 0)
				g_Config.SpiResistDivisor = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "PsyResistDivisor") == 0)
				g_Config.PsyResistDivisor = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "ForceMaxPacketSize") == 0)
				g_Config.ForceMaxPacketSize = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SceneryAuditDelay") == 0)
				g_Config.SceneryAuditDelay = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SceneryAuditAllow") == 0)
				g_Config.SceneryAuditAllow = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "MegaLootParty") == 0)
				g_Config.MegaLootParty = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "LootMaxRandomizedLevel") == 0)
				g_Config.LootMaxRandomizedLevel = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "LootMaxRandomizedSpecialLevel") == 0)
				g_Config.LootMaxRandomizedSpecialLevel = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "LootNamedMobSpecial") == 0)
				g_Config.LootNamedMobSpecial = lfr.BlockToBoolC(1);
			else if(strcmp(NameBlock, "LootMinimumMobRaritySpecial") == 0)
				g_Config.LootMinimumMobRaritySpecial = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HeroismQuestLevelTolerance") == 0)
				g_Config.HeroismQuestLevelTolerance = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "HeroismQuestLevelPenalty") == 0)
				g_Config.HeroismQuestLevelPenalty = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "NamedMobDropMultiplier") == 0)
				g_Config.NamedMobDropMultiplier = static_cast<float>(lfr.BlockToDblC(1));
			else if(strcmp(NameBlock, "NamedMobCreditDrops") == 0)
				g_Config.NamedMobCreditDrops = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "ProgressiveDropRateBonusMult") == 0)
				Util::AssignFloatArrayFromStringSplit(g_Config.ProgressiveDropRateBonusMult, COUNT_ARRAY_ELEMENTS(g_Config.ProgressiveDropRateBonusMult), lfr.BlockToStringC(1, 0));
			else if(strcmp(NameBlock, "ProgressiveDropRateBonusMultMax") == 0)
				g_Config.ProgressiveDropRateBonusMultMax = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "DropRateBonusMultMax") == 0)
				g_Config.DropRateBonusMultMax = lfr.BlockToFloatC(1);
			else if(strcmp(NameBlock, "UseIntegerHealth") == 0)
			{
				g_Config.UseIntegerHealth = lfr.BlockToBool(1);
				StatManager::SetHealthToInteger(g_Config.UseIntegerHealth);
			}
			else if(strcmp(NameBlock, "UseMessageBox") == 0)
				g_Config.UseMessageBox = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "UseStopSwim") == 0)
				g_Config.UseStopSwim = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "InvalidLoginMessage") == 0)
				g_Config.InvalidLoginMessage = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "GitHubToken") == 0)
				g_Config.GitHubToken = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "ServiceAuthURL") == 0)
				g_Config.ServiceAuthURL = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "VerifyMovement") == 0)
				g_Config.VerifyMovement = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "DebugLogAIScriptUse") == 0)
				g_Config.DebugLogAIScriptUse = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "SquirrelGCCallCount") == 0)
				g_Config.SquirrelGCCallCount = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SquirrelGCDelay") == 0)
				g_Config.SquirrelGCDelay = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SquirrelGCMaxDelay") == 0)
				g_Config.SquirrelGCMaxDelay = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SquirrelVMStackSize") == 0)
				g_Config.SquirrelVMStackSize = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "SquirrelQueueSpeed") == 0)
				g_Config.SquirrelQueueSpeed = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "PersistentBuffs") == 0)
				g_Config.PersistentBuffs = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "PartyLoot") == 0)
				g_Config.PartyLoot = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "AccountCredits") == 0)
				g_Config.AccountCredits = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "MinPVPPlayerLootItems") == 0)
				g_Config.MinPVPPlayerLootItems = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "MaxPVPPlayerLootItems") == 0)
				g_Config.MaxPVPPlayerLootItems = lfr.BlockToIntC(1);
			else if(strcmp(NameBlock, "NameChangeCost") == 0)
				g_Config.NameChangeCost = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "SSLVerifyPeer") == 0)
				g_Config.SSLVerifyPeer = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "SSLVerifyHostname") == 0)
				g_Config.SSLVerifyHostname = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "SMTPHost") == 0)
				g_Config.SMTPHost = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "SMTPUsername") == 0)
				g_Config.SMTPUsername = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "SMTPPassword") == 0)
				g_Config.SMTPPassword = lfr.BlockToStringC(1, 0);
			else if(strcmp(NameBlock, "SMTPPort") == 0)
				g_Config.SMTPPort = lfr.BlockToInt(1);
			else if(strcmp(NameBlock, "SMTPSSL") == 0)
				g_Config.SMTPSSL = lfr.BlockToBool(1);
			else if(strcmp(NameBlock, "SMTPSender") == 0)
				g_Config.SMTPSender = lfr.BlockToStringC(1, 0);
			else
			{
				g_Log.AddMessageFormatW(MSG_SHOW, "Unknown identifier [%s] in config file [%s]", lfr.BlockToString(0), filename);
			}
		}
	}
	lfr.CloseCurrent();
}

bool CheckDefaultHTTPBaseFolder(void)
{
	if(strlen(g_HTTPBaseFolder) == 0)
		return true;
	if(strcmp(g_HTTPBaseFolder, "this") == 0)
		return true;
	if(strcmp(g_HTTPBaseFolder, ".") == 0)
		return true;

	return false;
}

void SetHTTPBaseFolderToCurrent(void)
{
	memset(g_HTTPBaseFolder, 0, sizeof(g_HTTPBaseFolder));
	PLATFORM_GETCWD(g_HTTPBaseFolder, sizeof(g_HTTPBaseFolder) - 1);
	size_t len = strlen(g_HTTPBaseFolder);
	if(len > 0)
	{
		if(g_HTTPBaseFolder[len - 1] == '\\' || g_HTTPBaseFolder[len - 1] == '/')
			g_HTTPBaseFolder[len - 1] = 0;

		g_Log.AddMessageFormatW(MSG_SHOW, "HTTPBaseFolder defaulting to current working directory [%s]", g_HTTPBaseFolder);
	}
}
void LoadFileIntoString(std::string &dest, char *filename)
{
	FILE *input = fopen(filename, "rb");
	if(input == NULL)
	{
		g_Log.AddMessageFormatW(MSG_SHOW, "Generic error opening file [%s].", filename);
		return;
	}
	char buffer[4096];
	while(!feof(input))
	{
		fgets(buffer, sizeof(buffer), input);
		if(!feof(input))
		{
			if(dest.size() == 0)
				dest = buffer;
			else
				dest.append(buffer);
		}
	}
	fclose(input);
}

int LoadStringsFile(const char *filename, vector<string> &list)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormatW(MSG_SHOW, "Error opening file: %s", filename);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		if(r > 0)
		{
			list.push_back(lfr.DataBuffer);
		}
	}
	lfr.CloseCurrent();
	return list.size();
}

int LoadStringKeyValFile(const char *filename, vector<StringKeyVal> &list)
{
	FileReader lfr;
	if(lfr.OpenText(filename) != Err_OK)
	{
		g_Log.AddMessageFormatW(MSG_SHOW, "Error opening file: %s", filename);
		return -1;
	}

	lfr.CommentStyle = Comment_Semi;
	StringKeyVal newItem;
	while(lfr.FileOpen() == true)
	{
		int r = lfr.ReadLine();
		lfr.SingleBreak("=");
		if(r > 0)
		{
			newItem.key = lfr.BlockToStringC(0, 0);
			newItem.value = lfr.BlockToStringC(1, 0);
			list.push_back(newItem);
		}
	}
	lfr.CloseCurrent();
	return list.size();
}

GlobalConfigData :: GlobalConfigData()
{
	//Set defaults for config settings.
	ProperSceneryList = 1;
	BuybackLimit = 32;

	MegaLootParty = false;
	CapValourLevel = 0;

	LogLevelHTTPBase = LOG_WARNING;
	LogLevelSimulatorBase = LOG_WARNING;
	LogLevelHTTPDistribute = LOG_WARNING;

	debugAdministrativeBehaviorFlags = 0;
	Upgrade = 0;

	HeartbeatIntervalMS = 10000;
	HeartbeatAbortCount = -1;    //Requires a modded client to perform this task, so disable by default.

	WarpMovementBlockTime = 2000;

	IdleCheckVerification = false;
	IdleCheckFrequency = 1200000; //20 minutes
	IdleCheckDistance = 50;
	IdleCheckCast = 6;
	IdleCheckCastInterval = 60000; //1 minute
	IdleCheckDistanceTolerance = 100;

	CapExperienceLevel = 70;  //If the player level is this or greater, set incoming experience amount to the below value
	CapExperienceAmount = 0;  //Experience amount to give if the level cap is reached.

	CustomAbilityMechanics = false;   //Classic emulation by default.

	SendLobbyHeartbeat = false;  //Requires a modified client to receive and handle the heartbeat message in Lobby mode.

	DebugPingServer = false;     //Modded client required.
	DebugPingClient = false;     //Modded client required.
	DebugPingFrequency = 1000;   //Milliseconds.
	DebugPingClientPollInterval = 60;  //Number of pings between status polling.
	DebugPingServerLogThreshold = 2000; //Log all pings higher than this to file.
	internalStatus_PingID = 0;
	internalStatus_PingCount = 0;

	AprilFools = 0;
	AprilFoolsAccount = 0;

	HTTPDeleteDisconnectedTime = 60000;
	HTTPDeleteConnectedTime = 300000;
	HTTPDeleteRecheckDelay = 60000;
	PartyPositionSendInterval = 15000;

	VaultDefaultSize = 16;
	VaultInitialPurchaseSize = 8;

	GlobalMovementBonus = 0;
	AllowEliteMob = true;
	DexBlockDivisor = 0.0F;
	DexParryDivisor = 0.0F;
	DexDodgeDivisor = 0.0F;
	SpiResistDivisor = 0.0F;
	PsyResistDivisor = 0.0F;

	NamedMobDropMultiplier = 4.0F;
	NamedMobCreditDrops = 1;

	SceneryAuditDelay = 120000;  //2 minutes;
	SceneryAuditAllow = true;

	LootMaxRandomizedLevel = 50;
	LootMaxRandomizedSpecialLevel = 55;
	LootNamedMobSpecial = true;
	LootMinimumMobRaritySpecial = 2;

	HeroismQuestLevelTolerance = 3;
	HeroismQuestLevelPenalty = 4;

	DebugPacketSendTrigger = 128;
	DebugPacketSendDelay = 0;
	DebugPacketSendMessage = false;
	ForceMaxPacketSize = 1000;

	ProgressiveDropRateBonusMult[0] = 0.0025F;
	ProgressiveDropRateBonusMult[1] = 0.0050F;
	ProgressiveDropRateBonusMult[2] = 0.0100F;
	ProgressiveDropRateBonusMult[3] = 0.0200F;
	ProgressiveDropRateBonusMultMax = 2.0F;
	DropRateBonusMultMax = 200.0F;

	UseIntegerHealth = false;
	UseMessageBox = false;
	UseStopSwim = false;

	VerifyMovement = false;
	DebugLogAIScriptUse = false;

	SquirrelGCCallCount = 1000;
	SquirrelGCDelay = 10000;
	SquirrelGCMaxDelay = 60000;
	SquirrelVMStackSize = 512;
	SquirrelQueueSpeed = 10000;

	PersistentBuffs = false;
	PartyLoot = false;

	AccountCredits = true;
	NameChangeCost = 300;
	MinPVPPlayerLootItems = 0;
	MaxPVPPlayerLootItems = 0;

	SSLVerifyPeer = true;
	SSLVerifyHostname = true;

	SMTPHost = "";
	SMTPUsername = "";
	SMTPPassword = "";
	SMTPPort = 25;
	SMTPSSL = false;
	SMTPSender = "";

	InvalidLoginMessage = "Account not found.  Check username and password.";

}

GlobalConfigData :: ~GlobalConfigData()
{
}

bool GlobalConfigData :: RemotePasswordMatch(const char *value)
{
#ifdef PRIVATE_USE_BUILD
	return true;
#endif

	if(RemoteAuthenticationPassword.compare(value) == 0)
		return true;
	return false;
}

void GlobalConfigData :: SetAdministrativeBehaviorFlag(unsigned long bitValue, bool state)
{
	if(state == true)
		debugAdministrativeBehaviorFlags |= bitValue;
	else
		debugAdministrativeBehaviorFlags &= (~(bitValue));
}

bool GlobalConfigData :: HasAdministrativeBehaviorFlag(unsigned long bitValue)
{
	return ((debugAdministrativeBehaviorFlags & bitValue) != 0);
}
