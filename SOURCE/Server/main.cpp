/*   
   I'm a hobbyist programmer with no formal programming education and no professional experience.
   I learned a lot since I started this project.  The quality of code, coding standards and styles
   and comment details (or lack thereof) will be dramatically different between files and functions.
   Lots of code is really bad.  It seems to work though.

   Most of the code in these project files is my own, with some notable exceptions:
      The MD5 encryption algorithm in the files: md5.hh, md5.cc
	  The derived code in the QuaternionToByteFacing() function.
	  Some enums, variable names, and tables were copied out of the client's source code and adapted
	  accordingly.
	  Code for sockets and mutexes was derived or adapted from various internet tutorials

   Developed in Microsoft Visual C++ 2008 Express Edition.  This compiler is pretty lax when it comes to
   warnings and language standards.  You may see tons of warnings and maybe even build errors if using
   a different compiler (even if all the project settings are set correctly).

   Development began around December 22, 2010 after branching a new project file from a basic WinSock
   tutorial and testing application.  The WinSock project is dated around November 18, 2010.  This is
   the largest project I have ever worked on, and for the longest duration.

   There are no guarantees of security, either for data or program integrity.  Bugs, exploits, hacks, and
   data corruption may all be possible.  Use this software at your own risk.  Maintain backups of
   important files.  It is recommended that you backup server data on a routine basis.

   Brought to you buy a guy with way too much free time,
     -Grethnefar
*/

/*

The story continues.

Around March 2015, myself and a small team of remaining players took Greth's generous release
of his server source, data, and extensive documentation and have started to bolt new things on,
add new content and generally mess with things.

Our goal is to get the incomplete Grunes Tal and Swineland regions open (which has proved a lot of
work for everyone involved, it's made me appreciate even me more what Greth did).

I AM a professional programmer, although have never used C++ before, so expect bad habits,
stupid ideas, and mis-understanding of original ideas. However, it does work, and adds some fun
new things and future possibilities.

PS Greth's disclaimer stands. No guarantees of anything :)

For now I am developing on Linux only, with the intention of revisiting Windows to make sure it
all still compiles (probably not). The project has been converted to autotools format, which
should be familiar to most GCC developers.

- Emerald Icemoon

 */

/*	NOTES ON COMPILING:

>>>	SEE "COMPONENTS.H" for information on compiler and platform specific defines.


ADDING FILES TO THE PROJECT
  In Visual C++:
    In the Solution Explorer pane, there are folders for 'Header Files' and
    'Source Files'.  Right click these folders, then click 'Add > Add Existing Item...' and
	select all appropriate files.

  In Code Blocks
    In the Projects pane, under the Workspace tree.  Right click the project name, then click
	'Add Files...'.

ADD ALL OF THESE FILES TO THE PROJECT
Header Files: All files with *.h and *.hh extensions.
Source Files: All files with *.cpp and *.cc extensions.
Note: The MD5 source code files use the .hh and .cc format.  Make sure to add them too.



If compiling in Microsoft Visual C++ 2008 Express
  Add this #define to ignore deprecation security warnings:
	Project > Properties

	  -- then, in the tree to the left, navigate to --
	  Configuration Properties
		C/C++
		  Preprocessor
-- Then add --mu
		 _CRT_SECURE_NO_WARNINGS
	  -- Into the "Preprocessor Definitions" field. --


If compiling in Code::Blocks,
  Where to add libraries: (Project > Build Options > Linker Settings <Tab>)
  Where to add #defines:  (Project > Build Options > #defines <Tab>)
  Where to add Linker Options: (Project > Build Options > Linker Settings <Tab>)

If using Code::Blocks on WINDOWS, and using the Windows GUI:
	LIBRARIES
		Ws2_32
		Comdlg32
		Gdi32
		Comctl32

	DEFINES
		_WIN32_WINNT=0x0600


If using Code::Blocks on LINUX
	LIBRARIES
		pthread

	LINKER SETTINGS
		-lrt
		-rdynamic                   NOTE: this is needed for the crash backtrace

	DEFINES
		_GLIBCXX_DEBUG
		_GLIBCXX_DEBUG_PEDANTIC

*/


//Enable to include memory leak detection using CRT runtimes (Windows only)
//#define _CRTDEBUGGING

#ifdef _CRTDEBUGGING
 #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>

#ifdef _CRTDEBUGGING
 #include <crtdbg.h>
#endif

#include "DebugTracer.h"

#include "Components.h"
#include "MainWindow.h"
#include "SocketClass3.h"
#include "Account.h"
#include "Character.h"
#include "Stats.h"
#include "Globals.h"
#include "Router.h"
#include "SimulatorBase.h"
#include "Simulator.h"
#include "BroadCast.h"
#include "Util.h"
#include "Scenery2.h"
#include "StringList.h"
#include "Instance.h"
#include "ZoneDef.h"
#include "Session.h"
#include "Chat.h"
#include "Item.h"
#include "ItemSet.h"
#include "Creature.h"
#include "AIScript.h"
#include "AIScript2.h"
#include "Ability2.h"
#include "CreatureSpawner2.h"
#include "Interact.h"
#include "DropTable.h"
#include "Config.h"
#include "DirectoryAccess.h"
#include "RemoteAction.h"
#include "Gamble.h"
#include "QuestScript.h"
#include "ZoneObject.h"
#include "VirtualItem.h"
#include "FriendStatus.h"
#include "Debug.h"
#include "IGForum.h"
#include "EliteMob.h"
#include "Crafting.h"
#include "InstanceScale.h"
#include "CreditShop.h"
#include "Guilds.h"
#include "Clan.h"
#include "Daily.h"
#include "Timer.h"
#include "Leaderboard.h"
#include "http/HTTPService.h"
#include "query/Lobby.h"
#include "query/ClanHandlers.h"
#include "query/PreferenceHandlers.h"
#include "query/GMHandlers.h"
#include "query/CreditShopHandlers.h"
#include "query/VaultHandlers.h"
#include "query/AuctionHouseHandlers.h"
#include "query/ScriptHandlers.h"
#include "query/WarpHandlers.h"
#include "query/MarkerHandlers.h"
#include "query/SidekickHandlers.h"

#ifdef WINDOWS_SERVICE
#include <windows.h>
void  ServiceMain(int argc, char** argv);
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void  ControlHandler(DWORD request);
int InitService();
#endif

//extern GuildManager g_GuildManager;

ChangeData g_AutoSaveTimer;

char GAuxBuf[1024];    //Note, if this size is modified, change all "extern" references
char GSendBuf[32767];  //Note, if this size is modified, change all "extern" references

int InitServerMain(void);
void RunServerMain(void);
void SendHeartbeatMessages(void);
void RunPendingMessages(void);  //Runs all pending messages in the BroadCastMessage class.
void SendDebugPings();
void ShutDown(void);
void UnloadResources(void);
void RunMessageListCrash(void);
void RunMessageListQueue(void);
bool VerifyOperation(void);
void RunActiveInstances(void);
void CheckCharacterAutosave(bool force);
void RunUpgradeCheck(void);

void BroadcastLocationToParty(SimulatorThread &player);

char LogBuffer[4096];

void Debug_FullDump(void);

#ifdef WINDOWS_PLATFORM
void SystemLoop_Windows(void);
#endif

void SystemLoop_Console(void);

#ifdef WINDOWS_PLATFORM
#include <malloc.h>
#endif

// Linux exception handling.
#ifndef WINDOWS_PLATFORM
#include <signal.h>
#include <execinfo.h>

void segfault_sigaction(int signum, siginfo_t *si, void *arg)
{
	/* Uninstall signal handlers so if any happen while processing
	 * this we don't go into a mad loop
	 */
	signal(SIGPIPE, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);


	g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] signal encountered: %d", signum);
	switch(signum)
	{
	case SIGABRT: fprintf(stderr, "Caught: SIGABRT\n"); break;
	case SIGINT: fprintf(stderr, "Caught: SIGINT\n"); break;
	case SIGSEGV: fprintf(stderr, "Caught: SIGSEGV\n"); break;
	case SIGPIPE: fprintf(stderr, "Caught: SIGPIPE\n"); break;
	default: fprintf(stderr, "Caught signal: %d\n", signum); break;
	}
	void *ptrBuf[256];
	int numPtr = backtrace(ptrBuf, 256);
	fprintf(stderr, "Number of pointers: %d\n", numPtr);
	char **result = backtrace_symbols(ptrBuf, numPtr);
	fprintf(stderr, "Stack trace:\n");
	if(result != NULL)
	{
		for(int a = 0; a < numPtr; a++)
			fprintf(stderr, "  %s\n", result[a]);
		free(result);
	}
	fprintf(stderr, "Stack trace finished\n");
	fflush(stderr);

	fprintf(stderr, "Forcing auto save.\n");
	fflush(stderr);
	CheckCharacterAutosave(true);
	SaveSession("SessionVars.txt");

	fprintf(stderr, "Debug::LastAbility: %d\r\n", Debug::LastAbility);
	fprintf(stderr, "Debug::CreatureDefID: %d\r\n", Debug::CreatureDefID);
	fprintf(stderr, "Debug::LastName: %s\r\n", Debug::LastName);
	fprintf(stderr, "Debug::LastPlayer: %p\r\n", Debug::LastPlayer);
	fprintf(stderr, "Debug::IsPlayer: %d\r\n", Debug::IsPlayer);

	fprintf(stderr, "Debug::LastTileZone: %d\r\n", Debug::LastTileZone);
	fprintf(stderr, "Debug::LastTileX: %d\r\n", Debug::LastTileX);
	fprintf(stderr, "Debug::LastTileY: %d\r\n", Debug::LastTileY);
	fprintf(stderr, "Debug::LastTilePropID: %d\r\n", Debug::LastTilePropID);
	fprintf(stderr, "Debug::LastTilePtr: %p\r\n", Debug::LastTilePtr);
	fprintf(stderr, "Debug::LastTilePackage: %p\r\n", Debug::LastTilePackage);

	fprintf(stderr, "Debug::LastFlushSimulatorID: %d\r\n", Debug::LastFlushSimulatorID);

	fprintf(stderr, "Debug::ActivateAbility_cInst: %p\r\n", Debug::ActivateAbility_cInst);
	fprintf(stderr, "Debug::ActivateAbility_ability: %d\r\n", Debug::ActivateAbility_ability);
	fprintf(stderr, "Debug::ActivateAbility_ActionType: %d\r\n", Debug::ActivateAbility_ActionType);
	fprintf(stderr, "Debug::ActivateAbility_abTargetCount: %d\r\n", Debug::ActivateAbility_abTargetCount);
	fprintf(stderr, "Debug::ActivateAbility_abTargetList: \r\n");
	for(size_t i = 0; i < MAXTARGET; i++)
		fprintf(stderr, "  [%zu]=%p\r\n", i, Debug::ActivateAbility_abTargetList[i]);

	fprintf(stderr, "Debug::LastSimulatorID: %d\r\n", Debug::LastSimulatorID);

	fflush(stderr);
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		fprintf(stderr, "Sim:%d %p = %d\r\n", it->InternalID, &*it, it->pld.CreatureDefID);
	fflush(stderr);

	fprintf(stderr, "Running messages\n");
	fflush(stderr);
	RunMessageListCrash();
	fprintf(stderr, "Finished running messages\n");

	fprintf(stderr, "Writing crash dump\n");
	fflush(stderr);
	Debug_FullDump();

	fprintf(stderr, "Writing trace log\n");
	fflush(stderr);

	fprintf(stderr, "Shutting down.\n");
	fflush(stderr);
	ShutDown();

	fprintf(stderr, "Unloading resources.\n");
	fflush(stderr);
	UnloadResources();
	fprintf(stderr, "Resources unloaded, exiting program.\n");
	fflush(stderr);
	exit(1);
	return;
}

void InstallSignalHandler(void)
{
	//signal(SIGPIPE, SIG_IGN);

	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}
#endif

/*
void Handle_SIGPIPE(int unknown)
{
	fprintf(stderr, "SIGPIPE encountered (%d)\n", unknown);
	fflush(stderr);
}

void Handle_SIGSEGV(int unknown)
{
	fprintf(stderr, "SIGSEGV encountered (%d)\n", unknown);
	fflush(stderr);
	g_ServerStatus = SERVER_STATUS_EXCEPTION;
}

void Handle_SIGBUS(int unknown)
{
	fprintf(stderr, "SIGBUS encountered (%d)\n", unknown);
	fflush(stderr);
	g_ServerStatus = SERVER_STATUS_EXCEPTION;
}

void Handle_SIGFPE(int unknown)
{
	fprintf(stderr, "SIGFPE encountered (%d)\n", unknown);
	fflush(stderr);
	g_ServerStatus = SERVER_STATUS_EXCEPTION;
}

void Handle_SIGILL(int unknown)
{
	fprintf(stderr, "SIGILL encountered (%d)\n", unknown);
	fflush(stderr);
	g_ServerStatus = SERVER_STATUS_EXCEPTION;
}

void Handle_SIGINT(int unknown)
{
	fprintf(stderr, "SIGINT encountered (%d)\n", unknown);
	fflush(stderr);
	g_ServerStatus = SERVER_STATUS_EXCEPTION;
}
*/

//#endif

#ifdef WINDOWS_SERVICE
int main()
#else
#ifdef USE_WINDOWS_GUI
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
#endif
{
#ifdef _CRTDEBUGGING
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	Debug::Init();

	// Linux exception handling.
#ifndef WINDOWS_PLATFORM
	InstallSignalHandler();
	/*
	signal((int)SIGPIPE, Handle_SIGPIPE);
	signal((int)SIGSEGV, Handle_SIGSEGV);
	signal((int)SIGBUS, Handle_SIGBUS);
	signal((int)SIGFPE, Handle_SIGFPE);
	signal((int)SIGILL, Handle_SIGILL);
	signal((int)SIGINT, Handle_SIGINT);
	*/
#endif

#ifdef WINDOWS_SERVICE

	/* By default the working directory will be the \Windows\System32. This is no good for us,
		 * we need to be wherever the exectuable is	 *
		 */
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string dn = Platform::Dirname(buffer);
	PLATFORM_CHDIR(dn.c_str());


	SERVICE_TABLE_ENTRY ServiceTable[2];
	ServiceTable[0].lpServiceName = "TAWD";
	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	ServiceTable[1].lpServiceName = NULL;
	ServiceTable[1].lpServiceProc = NULL;
    StartServiceCtrlDispatcher(ServiceTable);
#else
#ifdef WINDOWS_GUI
	return InitServerMain();
#else
	InitServerMain();
#endif
#endif
}


#ifdef WINDOWS_SERVICE
void ServiceMain(int argc, char** argv) {

    int error;
    ServiceStatus.dwServiceType        = SERVICE_WIN32;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint         = 0;
    ServiceStatus.dwWaitHint           = 0;

    SetServiceStatus (hStatus, &ServiceStatus);

    hStatus = RegisterServiceCtrlHandler(
		"TAWD",
		(LPHANDLER_FUNCTION)ControlHandler);

    if (hStatus == (SERVICE_STATUS_HANDLE)0) {
        return;
    }

//	std::string cwd = Platform::GetDirectory();
//	std::string exe = argv[0];
//
//	if (Platform::IsAbsolute(exe)) {
//		cwd = Platform::Dirname(exe.c_str());
//	} else {
//		cwd = cwd + "\\" + Platform::Dirname(exe.c_str());
//	}
//
//	Platform::SetDirectory(cwd);

    InitServerMain();
}
#endif

int InitServerMain() {
	TRACE_INIT(250);

#ifdef USE_WINDOWS_GUI
	//The window needs to be initialized before any commands explicitly adjust
	//the values in any form controls
	if(InitMainWindow(hInstance, nCmdShow) == false)
	{
		MessageBox(NULL, "Could not create window.", "Error", MB_OK);
		return 0;
	}
#endif

	if(PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
	}
	bcm.mlog.reserve(100);

	LOG_OPEN();

#ifndef USE_WINDOWS_GUI
	printf("Loading data files...");
#endif

	//Init the time.  When the environment config is set during the load phase, it needs
	//to know the present time to initialize the first time cycle.
	g_ServerTime = g_PlatformTime.getMilliseconds();

	LoadConfig("ServerConfig.txt");
	LoadSession("SessionVars.txt");
	g_Log.LoggingEnabled = g_GlobalLogging;

	g_Log.AddMessageFormat("Working directory %s.", g_WorkingDirectory);
	// Lobby Query Handlers
	g_QueryManager.lobbyQueryHandlers["account.tracking"] = new AccountTrackingHandler();
	g_QueryManager.lobbyQueryHandlers["persona.list"] = new PersonaListHandler();
	g_QueryManager.lobbyQueryHandlers["persona.create"] = new PersonaCreateHandler();
	g_QueryManager.lobbyQueryHandlers["persona.delete"] = new PersonaDeleteHandler();
	g_QueryManager.lobbyQueryHandlers["mod.getURL"] = new ModGetURLHandler();

	// Game Query Handlers
	g_QueryManager.queryHandlers["clan.disband"] = new ClanDisbandHandler();
	g_QueryManager.queryHandlers["clan.create"] = new ClanCreateHandler();
	g_QueryManager.queryHandlers["clan.info"] = new ClanInfoHandler();
	g_QueryManager.queryHandlers["clan.invite"] = new ClanInviteHandler();
	g_QueryManager.queryHandlers["clan.invite.accept"] = new ClanInviteAcceptHandler();
	g_QueryManager.queryHandlers["clan.leave"] = new ClanLeaveHandler();
	g_QueryManager.queryHandlers["clan.remove"] = new ClanRemoveHandler();
	g_QueryManager.queryHandlers["clan.motd"] = new ClanMotdHandler();
	g_QueryManager.queryHandlers["clan.list"] = new ClanListHandler();
	g_QueryManager.queryHandlers["clan.rank"] = new ClanRankHandler();
	g_QueryManager.queryHandlers["pref.get"] = new PrefGetHandler();
	g_QueryManager.queryHandlers["pref.set"] = new PrefSetHandler();
	g_QueryManager.queryHandlers["util.addFunds"] = new AddFundsHandler();
	g_QueryManager.queryHandlers["item.market.buy"] = new CreditShopBuyHandler();
	g_QueryManager.queryHandlers["item.market.list"] = new CreditShopListHandler();
	g_QueryManager.queryHandlers["item.market.edit"] = new CreditShopEditHandler();
	g_QueryManager.queryHandlers["item.market.reload"] = new CreditShopReloadHandler();
	g_QueryManager.queryHandlers["item.market.purchase.name"] = new CreditShopPurchaseNameHandler();

	g_QueryManager.queryHandlers["skadd"] = new SidekickAddHandler();
	g_QueryManager.queryHandlers["skremove"] = new SidekickRemoveHandler();
	g_QueryManager.queryHandlers["skremoveall"] = new SidekickRemoveAllHandler();
	g_QueryManager.queryHandlers["skattack"] = new SidekickAttackHandler();
	g_QueryManager.queryHandlers["skcall"] = new SidekickCallHandler();
	g_QueryManager.queryHandlers["skwarp"] = new SidekickWarpHandler();
	g_QueryManager.queryHandlers["sklow"] = new SidekickLowHandler();
	g_QueryManager.queryHandlers["skparty"] = new SidekickPartyHandler();
	g_QueryManager.queryHandlers["skscatter"] = new SidekickScatterHandler();

	g_QueryManager.queryHandlers["vault.size"] = new VaultSizeHandler();
	g_QueryManager.queryHandlers["vault.send"] = new VaultSendHandler();
	g_QueryManager.queryHandlers["vault.expand"] = new VaultExpandHandler();
	g_QueryManager.queryHandlers["ah.contents"] = new AuctionHouseContentsHandler();
	g_QueryManager.queryHandlers["ah.auction"] = new AuctionHouseAuctionHandler();
	g_QueryManager.queryHandlers["ah.bid"] = new AuctionHouseBidHandler();
	g_QueryManager.queryHandlers["ah.buy"] = new AuctionHouseBuyHandler();

	g_QueryManager.queryHandlers["script.run"] = new ScriptRunHandler();
	g_QueryManager.queryHandlers["script.load"] = new ScriptLoadHandler();
	g_QueryManager.queryHandlers["script.kill"] = new ScriptKillHandler();
	g_QueryManager.queryHandlers["script.save"] = new ScriptSaveHandler();

	g_QueryManager.queryHandlers["warp"] = new WarpHandler();
	g_QueryManager.queryHandlers["warpi"] = new WarpInstanceHandler();
	g_QueryManager.queryHandlers["warpg"] = new WarpPullHandler();
	g_QueryManager.queryHandlers["warpt"] = new WarpTileHandler();
	g_QueryManager.queryHandlers["warpg"] = new WarpGroveHandler();
	g_QueryManager.queryHandlers["warpextoff"] = new WarpExternalOfflineHandler();
	g_QueryManager.queryHandlers["warpext"] = new WarpExternalHandler();


	g_QueryManager.queryHandlers["marker.list"] = new MarkerListHandler();
	g_QueryManager.queryHandlers["marker.edit"] = new MarkerEditHandler();
	g_QueryManager.queryHandlers["marker.del"] = new MarkerDelHandler();

	// Some are shared

	PrefSetAHandler* prefSetAHandler = new PrefSetAHandler();
	g_QueryManager.lobbyQueryHandlers["pref.setA"] = prefSetAHandler;
	g_QueryManager.queryHandlers["pref.setA"] = prefSetAHandler;

	PrefGetAHandler* prefGetAHandler = new PrefGetAHandler();
	g_QueryManager.lobbyQueryHandlers["pref.getA"] = prefGetAHandler;
	g_QueryManager.queryHandlers["pref.getA"] = prefGetAHandler;

	LobbyPingHandler* pingHandler = new LobbyPingHandler();
	g_QueryManager.lobbyQueryHandlers["util.ping"] = pingHandler;
	g_QueryManager.queryHandlers["util.ping"] = pingHandler;



//	else if(query.name.compare("pref.get") == 0)
//		handle_query_pref_get();
//	else if(query.name.compare("pref.setA") == 0)
//		handle_query_pref_setA();
//	else if(query.name.compare("pref.set") == 0)
//		handle_query_pref_set();



	g_Log.AddMessageFormat("Loaded %d checksums.", g_FileChecksum.mChecksumData.size());

	g_ItemManager.LoadData();
	g_ItemSetManager.LoadData();

	g_ModManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "ModTables.txt"));
	g_Log.AddMessageFormat("Loaded %d ModTables.", g_ModManager.modTable.size());
	g_ModTemplateManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "ModTemplates.txt"));
	g_Log.AddMessageFormat("Loaded %d ModTemplates.", g_ModTemplateManager.equipTemplate.size());
	g_EquipAppearance.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Appearances.txt"));
	g_Log.AddMessageFormat("Loaded %d EquipAppearance.", g_EquipAppearance.dataEntry.size());
	g_EquipIconAppearance.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Icons.txt"));
	g_Log.AddMessageFormat("Loaded %d EquipIconAppearance.", g_EquipIconAppearance.dataEntry.size());
	g_EquipTable.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "EquipTable.txt"));
	g_Log.AddMessageFormat("Loaded %d EquipTable.", g_EquipTable.equipList.size());
	g_NameTemplateManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Names.txt"));
	g_Log.AddMessageFormat("Loaded %d Name Templates.", g_NameTemplateManager.nameTemplate.size());
	g_NameModManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "NameMod.txt"));
	g_Log.AddMessageFormat("Loaded %d NameMods", g_NameModManager.mModList.size());
	g_NameWeightManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "NameWeight.txt"));
	g_Log.AddMessageFormat("Loaded %d NameWeight", g_NameWeightManager.mWeightList.size());
	g_VirtualItemModSystem.LoadSettings();
	g_DropTableManager.LoadData();

	g_AbilityManager.LoadData();


	//g_VirtualItemModSystem.Debug_RunDropDiagnostic(20);

	//g_EquipAppearance.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_appoutput2.txt"));
	//g_EquipIconAppearance.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_Icons2.txt"));
	//g_NameTemplateManager.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_Names.txt"));

	//g_EquipAppearance.Debug_CheckForNames();


	CreatureDef.LoadPackages(Platform::GenerateFilePath(GAuxBuf, "Packages", "CreaturePack.txt"));
	g_Log.AddMessageFormat("Loaded %d CreatureDefs.", CreatureDef.NPC.size());

	g_PetDefManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "Pets.txt"));
	g_Log.AddMessageFormat("Loaded %d PetDefs.", g_PetDefManager.GetStandardCount());

	g_AccountManager.LoadAllData();

	g_GambleManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "Gamble.txt"));
	g_Log.AddMessageFormat("Loaded %d Gamble definitions.", g_GambleManager.GetStandardCount());

	g_GuildManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "GuildDef.txt"));
	g_Log.AddMessageFormat("Loaded %d Guild definitions.", g_GuildManager.GetStandardCount());

	g_ClanManager.LoadClans();
	g_Log.AddMessageFormat("Loaded %d Clans.", g_ClanManager.mClans.size());

	g_CreditShopManager.LoadItems();
	g_Log.AddMessageFormat("Loaded %d Credit Shop items.", g_CreditShopManager.mItems.size());

	g_AuctionHouseManager.LoadItems();
	g_AuctionHouseManager.ConnectToSite();
	g_Log.AddMessageFormat("Loaded %d Auction House items.", g_CreditShopManager.mItems.size());

	g_ZoneDefManager.LoadData();
	g_GroveTemplateManager.LoadData();

	g_ZoneBarrierManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapBarrier.txt"));
	g_Log.AddMessageFormat("Loaded %d MapBarrier.", g_ZoneBarrierManager.GetLoadedCount());

	g_ZoneMarkerDataManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "ZoneMarkers.txt"));
	g_Log.AddMessageFormat("Loaded %d zones with marker data.", g_ZoneMarkerDataManager.zoneList.size());

	MapDef.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapDef.txt"));
	g_Log.AddMessageFormat("Loaded %d MapDef.", MapDef.mMapList.size());

	MapLocation.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapLocations.txt"));
	g_Log.AddMessageFormat("Loaded %d MapLocation zones.", MapLocation.mLocationSet.size());

	g_SceneryManager.LoadData();

	g_SpawnPackageManager.LoadFromFile("SpawnPackages", "SpawnPackageList.txt");
	g_Log.AddMessageFormat("Loaded %d Spawn Package lists.", g_SpawnPackageManager.packageList.size());

	QuestDef.LoadQuestPackages(Platform::GenerateFilePath(GAuxBuf, "Packages", "QuestPack.txt"));
	g_Log.AddMessageFormat("Loaded %d Quests.", QuestDef.mQuests.size());
	QuestScript::LoadQuestScripts(Platform::GenerateFilePath(GAuxBuf, "Data", "QuestScript.txt"));
	QuestDef.ResolveQuestMarkers();

	g_InteractObjectContainer.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "Data", "InteractDef.txt"));
	g_Log.AddMessageFormat("Loaded %d InteractDef.", g_InteractObjectContainer.objList.size());

	g_CraftManager.LoadData();

	g_EliteManager.LoadData();

	Global::LoadResCostTable(Platform::GenerateFilePath(GAuxBuf, "Data", "ResCost.txt"));

	g_InstanceScaleManager.LoadData();
	g_DropRateProfileManager.LoadData();

	g_DailyProfileManager.LoadData();
	g_Log.AddMessageFormat("Loaded %d Daily Profiles.", g_DailyProfileManager.GetNumberOfProfiles());

	g_FriendListManager.LoadAllData();

	aiScriptManager.LoadScripts();
	aiNutManager.LoadScripts();

#ifdef WINDOWS_PLATFORM
	WSAData wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(res != 0)
		LogMessage("WSAStartup failed: %d\n", res);
#endif


	if(g_Config.Upgrade == 0)
	{
		Router.SetBindAddress(g_BindAddress);
		SimulatorBase.SetBindAddress(g_BindAddress);
	}

//	if(g_HTTPListenPort > 0 && g_Config.Upgrade == 0)
//	{
//		HTTPBaseServer.SetHomePort(g_HTTPListenPort);
//		HTTPBaseServer.InitThread(0, g_GlobalThreadID++);
//	}

	g_FileChecksum.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "Data", "HTTPChecksum.txt"));

	if(g_RouterPort != 0 && g_Config.Upgrade == 0)
	{
		Router.SetHomePort(g_RouterPort);
		Router.SetTargetPort(g_SimulatorPort);
		Router.InitThread(0, g_GlobalThreadID++);
	}

	SimulatorBase.SetHomePort(g_SimulatorPort);
	SimulatorBase.InitThread(0, g_GlobalThreadID++);


	g_PacketManager.LaunchThread();
	g_SceneryManager.LaunchThread();

	// setup leaderboard
	g_LeaderboardManager.mBoards.push_back(new CharacterLeaderboard());
	g_LeaderboardManager.InitThread(g_GlobalThreadID++);

	g_Log.AddMessage("Server data has finished loading.");

	g_HTTPService.Start();

#ifdef USE_WINDOWS_GUI
	if(g_GlobalLogging == false)
		SetWindowText(MainWindowControlSet[MWCS_Edit_Status], "Note: Global logging is disabled.\r\n");
	AdjustComponentCount(1); 
#endif

	g_ServerLaunchTime = g_PlatformTime.getMilliseconds();
	srand(g_ServerLaunchTime & Platform::MAX_UINT);

	g_ServerStatus = SERVER_STATUS_RUNNING;
	if(VerifyOperation() == false)
		g_ServerStatus = SERVER_STATUS_STOPPED;

	RunUpgradeCheck();

	g_ChatManager.OpenChatLogFile("RegionChat.log");
	
#ifdef WINDOWS_SERVICE
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus (hStatus, &ServiceStatus);
#endif

#ifdef USE_WINDOWS_GUI
	SystemLoop_Windows();
#else
	SystemLoop_Console();
#endif

	ShutDown();

#ifdef WINDOWS_SERVICE
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
    SetServiceStatus (hStatus, &ServiceStatus);
#endif

	UnloadResources();
	exit(EXIT_SUCCESS);
#ifdef _CRTDEBUGGING
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

#ifdef WINDOWS_SERVICE

// Service initialization
int InitService() {
    int result;
    //result = g_Log.AddmeWriteToLog("Monitoring started.");
    return(result);
}

// Control handler function
void ControlHandler(DWORD request) {
	// TODO proper cleanup
    switch(request) {

        case SERVICE_CONTROL_STOP:
            //WriteToLog("Monitoring stopped.");

            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;
        case SERVICE_CONTROL_SHUTDOWN:
            //WriteToLog("Monitoring stopped.");
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;
        default:
            break;
    }

    // Report current status
    SetServiceStatus (hStatus,  &ServiceStatus);
    return;
}
#endif


#ifdef WINDOWS_PLATFORM
void SystemLoop_Windows(void)
{
	MSG msg;
	bool Exception = false;

	BEGINTRY
	{
	while(g_ServerStatus == SERVER_STATUS_RUNNING)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) != 0)
		{
			if(GetMessage(&msg, NULL, 0, 0) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				msg.message = 0;
			}
		}
		else
		{
			msg.message = 0;
			RunServerMain();
			PLATFORM_SLEEP(g_MainSleep);
		}
	}
	} //End try
	BEGINCATCH
	{
		POPUP_MESSAGE("An exception occurred, shutting down.", "Critical Error");
		Debug_FullDump();
		ShutDown();
		Exception = true;
	}

}
#endif //#ifdef WINDOWS_PLATFORM

void SystemLoop_Console(void)
{
	BEGINTRY
	{
		while(g_ServerStatus == SERVER_STATUS_RUNNING)
		{
			RunServerMain();
			PLATFORM_SLEEP(g_MainSleep);
		}
	} //End try
	BEGINCATCH
	{
		Debug_FullDump();
	}
}

bool VerifyOperation(void)
{
	// Return true if the server appears to have loaded (all required
	// listening ports are established).

	// If the GUI is functional, the admins can monitor the output and
	// and manually shut down the server for themselves.
#ifdef USE_WINDOWS_GUI
	return true;
#endif


	// Otherwise check the ports.  If any are not operational, prompt
	// the user if they want to quit.

	// Wait a couple seconds to give the threads a chance to start up.
	// Flush messages so any previous messages from the loading stage
	// are visible.
	RunMessageListQueue();
	PLATFORM_SLEEP(2000);

	int errorCount = 0;
	while(true)
	{
//		if(g_HTTPListenPort > 0)
//		{
//			if(HTTPBaseServer.Status != Status_Wait)
//			{
//				printf("The HTTP server is not operational. (Status: %s)\n", StatusPhaseStrings[HTTPBaseServer.Status]);
//				errorCount++;
//			}
//		}
		if(g_RouterPort > 0)
		{
			if(Router.Status != Status_Wait)
			{
				printf("The Router server is not operational. (Status: %s)\n", StatusPhaseStrings[Router.Status]);
				errorCount++;
			}
		}
		if(SimulatorBase.Status != Status_Wait)
		{
			printf("The Simulator base server is not operational. (Status: %s)\n", StatusPhaseStrings[SimulatorBase.Status]);
			errorCount++;
		}

		if(errorCount == 0)
			return true;

		if(errorCount < 10)
		{
			printf("\nWaiting 2 seconds...\n");
			PLATFORM_SLEEP(2000);
		}
		else
		{
			printf("Failed too many attempts, shutting down.");
			return false;
		}
	}
	return false;
}

void ShutDown(void)
{
	Debug::Shutdown();

	g_ChatManager.FlushChatLogFile();
	g_ChatManager.CloseChatLogFile();

	SaveSession("SessionVars.txt");
	g_IGFManager.CheckAutoSave(true);

	//VarDump();
	g_PacketManager.ShutdownThread();
	g_SceneryManager.ShutdownThread();

	g_SceneryManager.CheckAutosave(true);
	g_AccountManager.CheckAutoSave(true);
	g_AccountManager.RunUpdateCycle(true);
	g_ZoneDefManager.CheckAutoSave(true);
	g_ItemManager.CheckVirtualItemAutosave(true);


	g_Log.AddMessage("Waiting for threads to shut down...");

	//To shut down the threads:
	//
	// Set the thread isActive status to false so that the thread's main loop
	// will quit at the next available opportunity.
	//
	// Sockets may be stuck on the accept() or recv() stages.
	// Call ShutdownServer() for systems that are listening for connections.
	// Call DisconnectClient() for systems that are connected to sockets.

	g_HTTPService.Shutdown();

	if(g_RouterPort != 0)
	{
		if(Router.isExist == true)
		{
			Router.isActive = false;
			Router.sc.ShutdownServer();
		}
	}

	SimulatorBase.isActive = false;
	SimulatorBase.sc.ShutdownServer();

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		it->isThreadActive = false;
		it->sc.DisconnectClient();
	}

#ifdef USE_WINDOWS_GUI
	if(hwndMainWindow != NULL)
	{
		ShowWindow(hwndMainWindow, SW_HIDE);
		FreeWindow();
		hwndMainWindow = NULL;
		AdjustComponentCount(-1);  //For main window
	}
#endif

	//If any of the simulators are waiting 
	long waitCount = 0;
	while(ActiveComponents > 0)
	{
		g_SimulatorManager.RunPendingActions();
		waitCount++;
		PLATFORM_SLEEP(5);
		if(((waitCount + 1) % 2000) == 0)
		{
			SIMULATOR_IT it;
			for(it = Simulator.begin(); it != Simulator.end(); ++it)
			{
				if(it->isThreadExist == true)
				{
					fprintf(stderr, "Forcing Sim:%d to shut down\n", it->InternalID);
					it->isThreadActive = false;
					it->sc.DisconnectClient();
				}
			}
		}
	}

	if(waitCount > 0)
		g_Log.AddMessageFormat("[DEBUG] Shutdown() waitcount: %d", waitCount);

	//Characters can be unloaded only after the simulators have stopped using them.
	g_CharacterManager.UnloadAllCharacters();
}

void UnloadResources(void)
{
	//Since we're shutting down, flush the message log.
	g_Log.LoggingEnabled = false;
	RunMessageListQueue();

	g_AccountManager.UnloadAllData();
	g_CharacterManager.Clear();
	//DeleteCharacterList();

#ifdef WINDOWS_PLATFORM
	WSACleanup();
#endif

	bcm.Free();

	g_Log.Destroy();

	MapDef.FreeList();
	g_ZoneDefManager.Free();
	g_ActiveInstanceManager.Clear();  //Instances contain loot packages, which need to be freed before the Item system is destroyed.
	g_ItemManager.Free();
	CreatureDef.Clear();
	MapLocation.mLocationSet.clear();
	g_SceneryManager.Destroy();
	pendingOperations.Free();
	g_SimulatorManager.Free();
	g_FriendListManager.SaveAllData();
		//Debugger.Destroy();
	TRACE_FREE();
	LOG_CLOSE();
}


char *GetDataSizeStr(long value)
{
	//Prepares a string representation as a byte size of the given value.
	//It is converted into an easier to read format of MB, KB, or B.
	//Designed for the calling function that requires up to two distinct strings passed
	//as arguments to sprintf() in a manner such as "%s sent / %s received".
	//Each request is cycled into one of two string buffers, preventing overlap.
	static char Buffer[2][32] = {0};
	static int index = 0;

	index++;
	if(index > 1)
		index = 0;

	if(value >= 1000000)
		sprintf(&Buffer[index][0], "%3.2f MB", (double)value / 1000000.0);
	else if(value >= 1000)
		sprintf(&Buffer[index][0], "%3.2f KB", (double)value / 1000.0);
	else
		sprintf(&Buffer[index][0], "%ld B", value);

	return &Buffer[index][0];
}



void RunServerMain(void)
{
#ifdef DEBUG_TIME
	Debug::TimeTrack("RunServerMain", 200);
#endif

#ifdef WINDOWS_PLATFORM
	static Timer heapTimer;
	if(heapTimer.ReadyWithUpdate(2000) == true)
	{
		int r = _heapchk();
		switch(r)
		{
		case _HEAPBADBEGIN:
			g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Initial header information is bad or cannot be found");
			break;
		case _HEAPBADNODE:
			g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Bad node has been found or heap is damaged");
			break;
		case _HEAPBADPTR:
			g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Pointer into heap is not valid");
			break;
		case _HEAPEMPTY:
			g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Heap has not been initialized");
			break;
		}
	}
#endif

	static Timer simTimer;
	if(simTimer.ReadyWithUpdate(2000))
	{
		SIMULATOR_IT it = Simulator.begin();
		while(it != Simulator.end())
		{
			if(it->isThreadExist == false && it->isConnected == false && (g_ServerTime > it->LastUpdate + 2000))
			{
				g_Log.AddMessageFormat("Erasing sim:%d", it->InternalID);
				g_SimulatorManager.baseByteSent += it->TotalSendBytes;
				g_SimulatorManager.baseByteRec += it->TotalRecBytes;
				Simulator.erase(it++);
			}
			else
				++it;
		}
	}

	static Timer logTimer;
	if(logTimer.ReadyWithUpdate(300000))
		g_ChatManager.FlushChatLogFile();
	Debug::CheckAutoSave(false);

	g_SimulatorManager.RunPendingActions();
	g_CharacterManager.CheckGarbageCharacters();

#ifdef USE_WINDOWS_GUI
	if(VisPage == PAGECHAT && NewChat == true)
	{
		RefreshChatBox();
		NewChat = false;
	}
#endif

	RunMessageListQueue();

	g_ServerTime = g_PlatformTime.getMilliseconds();

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == false)
			continue;
		if(it->ProtocolState != 1)
			continue;
		//Most common gameplay state, listed first for fasted processing
		if(it->LoadStage >= SimulatorThread::LOADSTAGE_LOADED)
		{
			if(it->IsGMInvisible() == true)
				continue;
			//Gameplay loop
			if(it->pld.PendingMovement > 0)
			{
				if(it->creatureInst->Speed == 0)
				{
					if(g_ServerTime - it->pld.MovementTime >= (unsigned long)g_ForceUpdateTime)
					{
						it->pld.PendingMovement = 0;
						it->pld.MovementTime = 0;
						it->AddMessage((long)it->creatureInst, 0, BCM_UpdateFullPosition);
					}
				}
			}
			else if(g_ServerTime - it->LastUpdate >= g_RebroadcastDelay)
			{
				it->AddMessage((long)it->creatureInst, 0, BCM_UpdatePosInc);
				it->LastUpdate = g_ServerTime;
			}
			BroadcastLocationToParty(*it);
		}
	}

	SendHeartbeatMessages();
	RunPendingMessages();
	SendDebugPings();
	g_SceneryManager.CheckAutosave(false);

	if(g_AutoSaveTimer.IsLastChangeSince(5000) == true)
	{
		int r = 0;
		r += g_AccountManager.CheckAutoSave(false);
		r += g_ZoneDefManager.CheckAutoSave(false);
		if(r > 0)
			SaveSession("SessionVars.txt");
		g_AutoSaveTimer.ClearPending();
	}

	if(SessionVarsChangeData.CheckUpdateAndClear(300000) == true)
		SaveSession("SessionVars.txt");

	RunActiveInstances();
	g_TimerManager.RunTasks();
	g_SimulatorManager.FlushHangingSimulators();
	g_ItemManager.CheckVirtualItemAutosave(false);
	CheckCharacterAutosave(false);
	g_IGFManager.CheckAutoSave(false);
	g_IGFManager.RunGarbageCheck();
	g_AccountManager.RunUpdateCycle(false);
}

void CheckCharacterAutosave(bool force)
{
	if(Simulator.size() == 0)
		return;
	static Timer characterAutoSave;
	if(characterAutoSave.ReadyWithUpdate(300000) == true || (force == true))
	{
		int count = 0;
		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
		{
			if(it->ProtocolState != 1)
				continue;

			if(it->pld.charPtr == NULL)
				continue;
			
			it->SaveCharacterStats();
			g_CharacterManager.SaveCharacter(it->pld.CreatureDefID);
			count++;
		}
		g_Log.AddMessageFormat("Finished autosave on %d characters.", count);
	}
}

void SendHeartbeatMessages(void)
{
	static Timer heartbeatTimer;
	int wpos = 0;
	if(heartbeatTimer.ReadyWithUpdate(g_Config.HeartbeatIntervalMS) == false)
		return;

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == false)
			continue;

		if(it->ProtocolState == 0 && g_Config.SendLobbyHeartbeat == false)
			continue;

		//Don't send any message if they're actively sending data to the server.
		if(g_ServerTime < (it->LastRecv + g_Config.HeartbeatIntervalMS))
			continue;

		//Receiving a heartbeat message during casts seems to interrupt animations.
		if(it->creatureInst->ab[0].bPending == true)
			continue;

		if(wpos == 0) {  //Prep the buffer once, but only if we need it.
			unsigned long elapsed = ( g_ServerTime - g_ServerLaunchTime);
			wpos = PrepExt_SendHeartbeatMessage(GSendBuf, elapsed - it->LastHeartbeatSend);
			it->LastHeartbeatSend = elapsed;
		}

		it->PendingHeartbeatResponse++;
		it->AttemptSend(GSendBuf, wpos);
	}
}


void RunMessageListCrash(void)
{
	//Dumps the message queue in the event of a crash.
	if(g_Log.pendingCount <= 0)
		return;

	for(size_t i = 0; i < g_Log.stringList.size(); i++)
	{
		const char *buf = g_Log.stringList[i].c_str();
		fprintf(stderr, "[msg:%02lu]: %s\r\n", i, buf);
		fflush(stderr);
		LOG_WRITE(buf);
	}
	g_Log.stringList.clear();
}

void RunMessageListQueue(void)
{
	
	if(g_Log.pendingCount <= 0)
		return;

	/*
	g_Log.GetThread();
	g_Log.stringList.clear();
	g_Log.pendingCount = 0;
	g_Log.ReleaseThread();
	*/

#ifdef USE_WINDOWS_GUI
	bool SetText = false;
	GetWindowText(MainWindowControlSet[MWCS_Edit_Status], WindowTextBuffer, sizeof(WindowTextBuffer));
	int WritePos = strlen(WindowTextBuffer);
	int Remain = sizeof(WindowTextBuffer) - WritePos - 2;
#endif

	g_Log.GetThread("RunMessageListQueue");
	for(size_t i = 0; i < g_Log.stringList.size(); i++)
	{
		const char *buf = g_Log.stringList[i].c_str();
		LOG_WRITE(buf);

#ifdef USE_WINDOWS_GUI
		int ToCopy = strlen(buf);
		if(Remain >= ToCopy + 3)
		{
			int OldPos = WritePos;
			strncpy(&WindowTextBuffer[WritePos], buf, ToCopy);
			WritePos += ToCopy;
			if(Remain > 3)
			{
				WindowTextBuffer[WritePos++] = '\r';
				WindowTextBuffer[WritePos++] = '\n';
			}
			WindowTextBuffer[WritePos] = 0;
			Remain -= (WritePos - OldPos);
			SetText = true;
		}
#else //
	#ifdef WINDOWS_PLATFORM
		printf("%s\r\n", buf);
	#else
		printf("%s\n", buf);
	#endif
#endif //USE_WINDOWS_GUI

	}
	g_Log.stringList.clear();
	g_Log.pendingCount = 0;
	g_Log.ReleaseThread();

#ifdef USE_WINDOWS_GUI
	//Added the messages to the string buffer, set the window text
	if(SetText == true)
	{
		SetWindowText(MainWindowControlSet[MWCS_Edit_Status], WindowTextBuffer);

		//Scroll to the end
		HRESULT hres;
		hres = SendMessage(MainWindowControlSet[MWCS_Edit_Status], EM_GETLINECOUNT, NULL, NULL);
		SendMessage(MainWindowControlSet[MWCS_Edit_Status], EM_LINESCROLL, NULL, hres);
	}
#endif //USE_WINDOWS_GUI
}


void RunActiveInstances(void)
{
	bool envUpdated = g_EnvironmentCycleManager.HasCycleUpdated();
	const char * envString = NULL;
	if(envUpdated == true)
		envString = g_EnvironmentCycleManager.GetCurrentTimeOfDay();

	for(size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++)
	{
		g_ActiveInstanceManager.instListPtr[i]->RunProcessingCycle();
		if(envUpdated == true)
			g_ActiveInstanceManager.instListPtr[i]->UpdateEnvironmentCycle(envString);
	}
	pendingOperations.UpdateList_Process();
	pendingOperations.DeathList_Process();

	//Check if any instances have zero players.  If they're not persistent,
	//they need to be deleted after a certain amount of time.
	g_ActiveInstanceManager.CheckActiveInstances();

	
	if(g_ZoneDefManager.ZoneUnloadReady() == true)
	{
		std::vector<int> activeList;
		for(size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++)
			activeList.push_back(g_ActiveInstanceManager.instListPtr[i]->mZone);
		g_ZoneDefManager.UnloadInactiveZones(activeList);
	}

	if(g_SceneryManager.IsGarbageCheckReady() == true)
	{
		ActiveLocation::CONTAINER activeList;
		for(size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++)
		{
			ActiveInstance *inst = g_ActiveInstanceManager.instListPtr[i];
			activeList.push_back(inst->mZone);
			/*
			for(size_t p = 0; p < inst->PlayerListPtr.size(); p++)
			{
				int zone = inst->mZone;
				int tileX = inst->PlayerListPtr[p]->CurrentX / inst->mZoneDefPtr->mPageSize;
				int tileY = inst->PlayerListPtr[p]->CurrentZ / inst->mZoneDefPtr->mPageSize;
				activeList.push_back(ActiveLocation(zone, tileX, tileY));
			}*/
		}
		g_SceneryManager.GetThread("ActiveInstanceManager::CheckActiveInstances");
		g_SceneryManager.TransferActiveLocations(activeList);
		g_SceneryManager.ReleaseThread();
	}
}


void RunPendingMessages(void)
{
	//Get a quick size to see if operations are pending.
	//If so, get the real size from within the critical section, since a mismatch
	//might occur.  Process the messages in the order they were added.  Delete
	//the list when done.
	if(bcm.mlog.size() == 0)
		return;

	bcm.EnterCS("RunPendingMessages");
	for(size_t i = 0; i < bcm.mlog.size(); i++)
	{
#ifdef DEBUG_TIME
		Debug::TimeTrack("RunPendingMessages", 100);
#endif

		MessageComponent *msg = &bcm.mlog[i];

		if(msg->actInst != NULL)
			msg->actInst->ProcessMessage(msg);
		else
			g_Log.AddMessageFormat("[ERROR] Invalid Active Instance [%p] message for Sim:%d", msg->actInst, msg->SimulatorID);
	}
	bcm.mlog.clear();
	bcm.LeaveCS();
}

void SendDebugPings(void)
{
	if(g_Config.DebugPingServer == false)
		return;

	static char sendBuf[256];
	static Timer pingTimer;
	if(pingTimer.ReadyWithUpdate(g_Config.DebugPingFrequency) == false)
		return;

	int wpos = 0;
	SIMULATOR_IT it;
	bool report = false;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->CheckStateGameplayActive() == false)
			continue;

		if(wpos == 0) //Need to build packet
		{
			wpos += PutByte(&sendBuf[wpos], 119);             //Modded client: _handleDebugServerPing
			wpos += PutShort(&sendBuf[wpos], 0);
			wpos += PutInteger(&sendBuf[wpos], g_Config.internalStatus_PingID++);     //Global ping ID
			wpos += PutInteger(&sendBuf[wpos], g_ServerTime - g_ServerLaunchTime);    //Send time
			PutShort(&sendBuf[1], wpos - 3);

			//If the time is right, request information from the client.
			g_Config.internalStatus_PingCount++;
			if(g_Config.internalStatus_PingCount >= g_Config.DebugPingClientPollInterval)
			{
				int mpos = wpos;
				wpos += PutByte(&sendBuf[wpos], 100);   //_handleModMessage   REQUIRES MODDED CLIENT
				wpos += PutShort(&sendBuf[wpos], 0);
				wpos += PutByte(&sendBuf[wpos], MODMESSAGE_EVENT_PING_QUERY);
				PutShort(&sendBuf[mpos + 1], wpos - mpos - 3);

				g_Config.internalStatus_PingCount = 0;
				report = true;
			}
		}
		it->pld.DebugPingServerSent++;
		it->AttemptSend(sendBuf, wpos);
		if(report == true)
			it->LogPingStatistics(true, false);
	}
}


char *LogMessage(const char *format, ...)
{
	g_Log.string_cs.Enter("StringList::AddMessage");
	va_list args;
	va_start (args, format);
	//vsnprintf(LogBuffer, maxSize, format, args);
	Util::SafeFormatArg(LogBuffer, sizeof(LogBuffer), format, args);
	va_end (args);

	g_Log.AddMessage(LogBuffer);
	g_Log.string_cs.Leave();

	return LogBuffer;
}

void Debug_OutputCharacter(FILE *output, int index, CreatureInstance *cInst)
{
	//Helper function for outputting creature data.
	fprintf(output, "  [%02d] = %s (Ptr: %p) ID: %d, CDef: %d, Fact: %d\r\n", index, cInst->css.display_name, cInst, cInst->CreatureID, cInst->CreatureDefID, cInst->Faction);
	fprintf(output, "    Pos: %d, %d, %d\r\n", cInst->CurrentX, cInst->CurrentY, cInst->CurrentZ);
	if(cInst->aiScript != NULL)
	{
		fprintf(output, "    AI TLS Ptr: %p (%s) curInst: %d\r\n", cInst->aiScript, cInst->aiScript->def->scriptName.c_str(), cInst->aiScript->curInst);
	}
	if(cInst->aiNut != NULL)
	{
		fprintf(output, "    AI Squirrel Ptr: %p (%s)\r\n", cInst->aiNut, cInst->aiNut->def->scriptName.c_str());
	}
	fprintf(output, "    Target: %s (Ptr: %p)\r\n", (cInst->CurrentTarget.targ != NULL) ? cInst->CurrentTarget.targ->css.display_name : "null", cInst->CurrentTarget.targ);
	fprintf(output, "    Might: %d (%d), Will: %d (%d)\r\n", cInst->css.might, cInst->css.might_charges, cInst->css.will, cInst->css.will_charges);
	if(cInst->charPtr != NULL)
	{
		QuestReferenceContainer act = cInst->charPtr->questJournal.activeQuests;
		for(std::vector<QuestReference>::iterator it = act.itemList.begin(); it != act.itemList.end(); ++it) {
			QuestReference ref = *it;
			fprintf(output, "    Quest: %d (ACT %d)\r\n", ref.QuestID, ref.CurAct);

			g_QuestNutManager.cs.Enter("QuestNutManager::GetActiveScript");
			QuestScript::QuestNutPlayer *questScript = g_QuestNutManager.GetActiveScript(cInst->CreatureID, ref.QuestID);
			g_QuestNutManager.cs.Leave();

			if(questScript != NULL && questScript->def != NULL) {
				fprintf(output, "    Quest Script: %s \r\n", questScript->def->scriptName.c_str());
			}
		}
	}
	int abi;
	for(abi = 0; abi <= 1; abi++)
	{
		if(cInst->ab[abi].bPending == true)
		{
			fprintf(output, "    ab[%d] abID: %d, %d targets\r\n", abi, cInst->ab[abi].abilityID, cInst->ab[abi].TargetCount);
			int b;
			for(b = 0; b < cInst->ab[abi].TargetCount; b++)
			{
				CreatureInstance *targ = cInst->ab[abi].TargetList[b];
				fprintf(output, "      [%d] = %s (Ptr: %p)\r\n", b, (targ != NULL) ? targ->css.display_name : "null", targ);
			}
		}
		if(cInst->ab[abi].x != 0.0F)
			fprintf(output, "    ab[%d] target loc: %g, %g, %g\r\n", abi, cInst->ab[abi].x, cInst->ab[abi].y, cInst->ab[abi].z);
	}
	if(cInst->AnchorObject != NULL)
		fprintf(output, "    Officer: %s (Ptr: %p)\r\n", cInst->AnchorObject->css.display_name, cInst->AnchorObject);

	if(cInst->HasStatus(StatusEffects::AUTO_ATTACK))
		fprintf(output, "    AUTO_ATTACK");
	if(cInst->HasStatus(StatusEffects::AUTO_ATTACK_RANGED))
		fprintf(output, "    AUTO_ATTACK_RANGED");
	if(cInst->HasStatus(StatusEffects::DEAD))
		fprintf(output, "    DEAD");
	fprintf(output, "\r\n");
}

void Debug_FullDump(void)
{
	FILE *output = fopen("crash_dump.txt", "wb");
	if(output == NULL)
		return;
	g_Log.AddMessageFormat("Writing crash dump.");

	fprintf(output, "Account Data:\r\n");
	g_AccountManager.cs.Enter("Debug_FullDump");
	AccountManager::ACCOUNT_ITERATOR accit;
	for(accit = g_AccountManager.AccList.begin(); accit != g_AccountManager.AccList.end(); ++accit)
		fprintf(output, "%s = %p\r\n", accit->Name, &*accit);
	g_AccountManager.cs.Leave();
	fprintf(output, "\r\n\r\n");
	fflush(output);

	fprintf(output, "Character Data:\r\n");
	g_CharacterManager.GetThread("Debug_FullDump");
	CharacterManager::CHARACTER_MAP::iterator charit;
	for(charit = g_CharacterManager.charList.begin(); charit != g_CharacterManager.charList.end(); ++charit)
		fprintf(output, "[%d,%d] Name: %s, Ptr: %p\r\n", charit->first, charit->second.cdef.CreatureDefID, charit->second.cdef.css.display_name, &charit->second);
	g_CharacterManager.ReleaseThread();
	fprintf(output, "\r\n\r\n");
	fflush(output);

	//TODO: REVAMP
	//Debug_GenerateSimulatorReports(&report);
	fprintf(output, "Simulator Report:\r\n");
	fflush(output);

	fprintf(output, "Active Instances: %lu\r\n", g_ActiveInstanceManager.instList.size());
	list<ActiveInstance>::iterator aInst;
	int a;
	for(aInst = g_ActiveInstanceManager.instList.begin(); aInst != g_ActiveInstanceManager.instList.end(); ++aInst)
	{
		fprintf(output, "ZoneID: %d, Players: %d\r\n", aInst->mZone, aInst->mPlayers);
		fprintf(output, "Players: %lu\r\n", aInst->PlayerListPtr.size());
		for(a = 0; a < (int)aInst->PlayerListPtr.size(); a++)
			Debug_OutputCharacter(output, a, aInst->PlayerListPtr[a]);

		fprintf(output, "\r\nSidekicks: %lu\r\n", aInst->SidekickListPtr.size());
		for(a = 0; a < (int)aInst->SidekickListPtr.size(); a++)
			Debug_OutputCharacter(output, a, aInst->SidekickListPtr[a]);
		fprintf(output, "\r\n\r\n");

#ifndef CREATUREMAP
		fprintf(output, "\r\nNPCs: %d\r\n", aInst->NPCListPtr.size());
		for(a = 0; a < (int)aInst->NPCListPtr.size(); a++)
			Debug_OutputCharacter(output, a, aInst->NPCListPtr[a]);
		fprintf(output, "\r\n\r\n");
#else
		ActiveInstance::CREATURE_IT it;
		for(it = aInst->NPCList.begin(); it != aInst->NPCList.end(); ++it)
			Debug_OutputCharacter(output, a, &it->second);
		fprintf(output, "\r\n\r\n");
#endif
	}
	fclose(output);
}


void EraseRegistrationKeys(void)
{
	for(size_t i = 0; i < g_AccountManager.accountQuickData.size(); i++)
	{
		AccountQuickData *aqd = &g_AccountManager.accountQuickData[i];
		AccountData *accPtr = g_AccountManager.FetchIndividualAccount(g_AccountManager.accountQuickData[i].mID);
		if(accPtr == NULL || aqd == NULL)
		{
			g_Log.AddMessageFormat("[ERROR] Account not found: %s", g_AccountManager.accountQuickData[i].mLoginName.c_str());
			continue;
		}

		//Remove the reset permission
		accPtr->SetPermission(Perm_Account, "passwordreset", false);

		//Apply recovery key if applicable
		accPtr->GenerateAndApplyRegistrationKeyRecovery();

		accPtr->FillRegistrationKey("!");
		aqd->mRegKey = "!";

		g_AccountManager.AccountQuickDataChanges.AddChange();
		accPtr->PendingMinorUpdates++;
	}
	g_AccountManager.CheckAutoSave(true);
}

void RunUpgradeCheck(void)
{
	if(g_Config.Upgrade <= 0)
		return;

	if(g_Config.Upgrade == 200)
	{
		EraseRegistrationKeys();
	}

	g_Log.AddMessageFormat("g_Config: Server upgraded, shutting down.");
	g_ServerStatus = SERVER_STATUS_STOPPED;
}

void BroadcastLocationToParty(SimulatorThread &player)
{
	if(player.creatureInst->PartyID == 0)
		return;

	if(g_ServerTime < player.pld.NextPartyLocationBroadcastTime)
		return;

	player.pld.NextPartyLocationBroadcastTime = g_ServerTime + g_Config.PartyPositionSendInterval;

	int size = 0;

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->InternalID == player.InternalID)
			continue;
		if(it->isConnected == false)
			continue;
		if(it->ProtocolState != 1)
			continue;
		if(it->creatureInst->PartyID != player.creatureInst->PartyID)
			continue;
		//if(it->pld.CurrentInstanceID != player.pld.CurrentInstanceID)
		//	continue;

		int dist = ActiveInstance::GetPlaneRange(player.creatureInst, it->creatureInst, DISTANCE_FAILED);
		if(dist > g_CreatureListenRange)
		{
			if(size == 0)
				size = PrepExt_CreaturePos(GSendBuf, player.creatureInst);
			it->AttemptSend(GSendBuf, size);
		}
	}
}
