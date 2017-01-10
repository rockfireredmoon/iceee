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
#include "util/Log.h"
INITIALIZE_EASYLOGGINGPP

#include "Components.h"
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
#include "Books.h"
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
#include "query/MarkerHandlers.h"
#include "query/SidekickHandlers.h"
#include "query/QuestHandlers.h"
#include "query/IGFHandlers.h"
#include "query/TradeHandlers.h"
#include "query/SceneryHandlers.h"
#include "query/SupportHandlers.h"
#include "query/BookHandlers.h"
#include "query/ItemHandlers.h"
#include "query/CommandHandlers.h"
#include "query/LootHandlers.h"
#include "query/FriendHandlers.h"
#include "query/AbilityHandlers.h"
#include "query/CreatureHandlers.h"
#include "query/PetHandlers.h"
#include "query/SpawnHandlers.h"
#include "query/ZoneHandlers.h"
#include "query/StatusHandlers.h"
#include <curl/curl.h>

#ifdef OUTPUT_TO_CONSOLE
#define DAEMON_NO_CLOSE 1
#else
#define DAEMON_NO_CLOSE 0
#endif
#ifdef WINDOWS_SERVICE
#include <windows.h>
void  ServiceMain(int argc, char** argv);
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
void  ControlHandler(DWORD request);
int InitService();
#else
#include <unistd.h>
#endif

//extern GuildManager g_GuildManager;

ChangeData g_AutoSaveTimer;

char GAuxBuf[1024];    //Note, if this size is modified, change all "extern" references
char GSendBuf[32767];  //Note, if this size is modified, change all "extern" references

int InitServerMain(int argc, char *argv[]);
void RunServerMain(void);
void SendHeartbeatMessages(void);
void RunPendingMessages(void);  //Runs all pending messages in the BroadCastMessage class.
void SendDebugPings();
void ShutDown(void);
void UnloadResources(void);
bool VerifyOperation(void);
void RunActiveInstances(void);
void CheckCharacterAutosave(bool force);
void RunUpgradeCheck(void);

void BroadcastLocationToParty(SimulatorThread &player);

char LogBuffer[4096];
bool crashing;

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
	crashing = true;

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


	if(signum != SIGTERM) {
		g_Logs.server->fatal("Signal encountered: %v", signum);
		switch(signum)
		{
		case SIGABRT: g_Logs.server->fatal("Caught: SIGABRT"); break;
		case SIGINT: g_Logs.server->fatal("Caught: SIGINT"); break;
		case SIGSEGV: g_Logs.server->fatal("Caught: SIGSEGV"); break;
		case SIGPIPE: g_Logs.server->fatal("Caught: SIGPIPE"); break;
		default: g_Logs.server->fatal("Caught signal: %v", signum); break;
		}
		void *ptrBuf[256];

		int numPtr = backtrace(ptrBuf, 256);
		g_Logs.server->fatal("Number of pointers: %v", numPtr);
		char **result = backtrace_symbols(ptrBuf, numPtr);
		g_Logs.server->fatal("Stack trace:");
		if(result != NULL)
		{
			for(int a = 0; a < numPtr; a++) {
				std::string line = "UNKNOWN";
				std::vector<std::string> parts;
				Util::Split(result[a], " ", parts);
				if(parts.size() > 0) {
					std:string addr = parts[parts.size() - 1];
					char cmd[512];
					Util::SafeFormat(cmd, sizeof(cmd), "addr2line -e %s -a %s", g_Executable, addr.c_str());
					line = Util::CaptureCommand(cmd);
				}
				g_Logs.server->fatal("  %v %v", result[a], line);

			}
			free(result);
		}
		g_Logs.server->fatal("Stack trace finished");
	}

	if(signum != SIGTERM) {
		g_Logs.server->fatal("Debug::LastAbility: %v", Debug::LastAbility);
		g_Logs.server->fatal("Debug::CreatureDefID: %v", Debug::CreatureDefID);
		g_Logs.server->fatal("Debug::LastName: %v", Debug::LastName);
		g_Logs.server->fatal("Debug::LastPlayer: %v", Debug::LastPlayer);
		g_Logs.server->fatal("Debug::IsPlayer: %v", Debug::IsPlayer);

		g_Logs.server->fatal("Debug::LastTileZone: %v", Debug::LastTileZone);
		g_Logs.server->fatal("Debug::LastTileX: %v", Debug::LastTileX);
		g_Logs.server->fatal("Debug::LastTileY: %v", Debug::LastTileY);
		g_Logs.server->fatal("Debug::LastTilePropID: %v", Debug::LastTilePropID);
		g_Logs.server->fatal("Debug::LastTilePtr: %v", Debug::LastTilePtr);
		g_Logs.server->fatal("Debug::LastTilePackage: %v", Debug::LastTilePackage);

		g_Logs.server->fatal("Debug::LastFlushSimulatorID: %v", Debug::LastFlushSimulatorID);

		g_Logs.server->fatal("Debug::ActivateAbility_cInst: %v", Debug::ActivateAbility_cInst);
		g_Logs.server->fatal("Debug::ActivateAbility_ability: %v", Debug::ActivateAbility_ability);
		g_Logs.server->fatal("Debug::ActivateAbility_ActionType: %v", Debug::ActivateAbility_ActionType);
		g_Logs.server->fatal("Debug::ActivateAbility_abTargetCount: %v", Debug::ActivateAbility_abTargetCount);
		g_Logs.server->fatal("Debug::ActivateAbility_abTargetList: ");
		for(size_t i = 0; i < MAXTARGET; i++)
			g_Logs.server->fatal("  [%v]=%v", i, Debug::ActivateAbility_abTargetList[i]);

		g_Logs.server->fatal("Debug::LastSimulatorID: %v", Debug::LastSimulatorID);
		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
			g_Logs.server->fatal("Sim:%v %v = %v", it->InternalID, &*it, it->pld.CreatureDefID);
	}

	if(signum != SIGTERM) {
		g_Logs.server->fatal("Finished running messages");
		Debug_FullDump();
	}
	ShutDown();
	UnloadResources();

	if (signum != SIGTERM && g_Config.ServiceAuthURL.size() > 0 && g_Config.SiteServiceUsername.size() > 0) {
		g_Logs.server->fatal("Posting forum report");
		SiteClient siteClient(g_Config.ServiceAuthURL);
		HTTPD::SiteSession siteSession;
		siteClient.refreshXCSRF(&siteSession);
		siteClient.login(&siteSession, g_Config.SiteServiceUsername, g_Config.SiteServicePassword);
		if (siteClient.postCrashReport(&siteSession, signum) == 0) {
			g_Logs.server->fatal("Posted forum report");
		} else {
			g_Logs.server->fatal("Failed to post forum report");
		}
	}

	if(signum != SIGTERM && g_Config.ShutdownHandlerScript.size() > 0) {
		char scriptCall[g_Config.ShutdownHandlerScript.size() + 64];
		g_Logs.server->fatal("Calling shutdown handler script %v", g_Config.ShutdownHandlerScript);
		Util::SafeFormat(scriptCall, sizeof(scriptCall), "%s %d",
				g_Config.ShutdownHandlerScript.c_str(),
				signum);

		g_Logs.server->fatal("Shutdown handler script %v completed with status %v", g_Config.ShutdownHandlerScript, system(scriptCall));
	}

	g_Logs.server->fatal("Exiting");
	g_Logs.FlushAll();
	exit(signum == SIGTERM ? 0 : 1);
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
int main() {
	int argc = 0;
	char *argv[0];
#else
int main(int argc, char *argv[]) {
#endif
#ifdef _CRTDEBUGGING
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

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
	return InitServerMain(0, 0);
#else
	InitServerMain(argc, argv);
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

int InitServerMain(int argc, char *argv[]) {
	TRACE_INIT(250);

	if(PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
	}

	START_EASYLOGGINGPP(argc, argv);

	bool daemonize = false;
	std::string pidfile = "";

	for(int i = 0 ; i < argc ; i++) {
		if(i == 0) {
			if(Util::HasBeginning(argv[i], "/"))
				strcpy(g_Executable, argv[i]);
			else
				Util::SafeFormat(g_Executable, 512, "%s/%s", g_WorkingDirectory, argv[i]);
		}
		else if(strcmp(argv[i], "-d") == 0)
			daemonize = true;
		else if(strcmp(argv[i], "-p") == 0) {
			pidfile = argv[++i];
		}
		else if(strcmp(argv[i], "-I") == 0) {
			el::Loggers::addFlag(el::LoggingFlag::ImmediateFlush);
		}
		else if(strcmp(argv[i], "-L") == 0) {
			i++;
			if(i < argc) {
				std::string str = argv[i];
				Util::ToLowerCase(str);
				if(str.compare("info") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Info);
				}
				else if(str.compare("debug") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Debug);
				}
				else if(str.compare("error") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Error);
				}
				else if(str.compare("fatal") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Fatal);
				}
				else if(str.compare("trace") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Trace);
				}
				else if(str.compare("verbose") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Verbose);
				}
				else if(str.compare("warning") == 0) {
					el::Loggers::setLoggingLevel(el::Level::Warning);
				}
			}
		}
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);
	bcm.mlog.reserve(100);

	g_Logs.server->info("Loading data files...");



	//
	// TODO
	//
	// Get to the bottom of this. When errors (such as missing script functions) throw an
	// exception (during event handling?), it seems to mess up the stack. I am not sure
	// if this is only in the Sqrat type of handling (e.g. .Evaluate() functions) or
	// more general to Squirrel (e.g. when throw exception is true)
	//	Sqrat::ErrorHandling::Enable(false);

	//Init the time.  When the environment config is set during the load phase, it needs
	//to know the present time to initialize the first time cycle.
	g_ServerTime = g_PlatformTime.getMilliseconds();

	LoadConfig("ServerConfig.txt");
	LoadSession("SessionVars.txt");

	g_Logs.server->info("Working directory %v.", g_WorkingDirectory);

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
	g_QueryManager.queryHandlers["zone.mode"] = new PVPZoneModeHandler();
	g_QueryManager.queryHandlers["item.market.buy"] = new CreditShopBuyHandler();
	g_QueryManager.queryHandlers["item.market.list"] = new CreditShopListHandler();
	g_QueryManager.queryHandlers["item.market.edit"] = new CreditShopEditHandler();
	g_QueryManager.queryHandlers["item.market.reload"] = new CreditShopReloadHandler();
	g_QueryManager.queryHandlers["item.market.purchase.name"] = new CreditShopPurchaseNameHandler();

	g_QueryManager.queryHandlers["skadd"] = new SidekickAddHandler();
	g_QueryManager.queryHandlers["skremove"] = new SidekickRemoveHandler();
	g_QueryManager.queryHandlers["skremoveall"] = new SidekickRemoveAllHandler();
	g_QueryManager.queryHandlers["skattack"] = new SidekickAttackHandler();
	g_QueryManager.queryHandlers["skdefend"] = new SidekickDefendHandler();
	g_QueryManager.queryHandlers["skcall"] = new SidekickCallHandler();
	g_QueryManager.queryHandlers["skwarp"] = new SidekickWarpHandler();
	g_QueryManager.queryHandlers["sklow"] = new SidekickLowHandler();
	g_QueryManager.queryHandlers["skparty"] = new SidekickPartyHandler();
	g_QueryManager.queryHandlers["skscatter"] = new SidekickScatterHandler();
	g_QueryManager.queryHandlers["sidekick.notifyex"] = new SidekickNotifyExpHandler();

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

	g_QueryManager.queryHandlers["marker.list"] = new MarkerListHandler();
	g_QueryManager.queryHandlers["marker.edit"] = new MarkerEditHandler();
	g_QueryManager.queryHandlers["marker.del"] = new MarkerDelHandler();

	g_QueryManager.queryHandlers["quest.indicator"] = new QuestIndicatorHandler();
	g_QueryManager.queryHandlers["quest.getquestoffer"] = new QuestGetOfferHandler();
	g_QueryManager.queryHandlers["quest.genericdata"] = new QuestGenericDataHandler();
	g_QueryManager.queryHandlers["quest.join"] = new QuestJoinHandler();
	g_QueryManager.queryHandlers["quest.list"] = new QuestListHandler();
	g_QueryManager.queryHandlers["quest.data"] = new QuestDataHandler();
	g_QueryManager.queryHandlers["quest.getcompletequest"] = new QuestGetCompleteHandler();
	g_QueryManager.queryHandlers["quest.complete"] = new QuestCompleteHandler();
	g_QueryManager.queryHandlers["quest.leave"] = new QuestLeaveHandler();
	g_QueryManager.queryHandlers["quest.hack"] = new QuestHackHandler();
	g_QueryManager.queryHandlers["quest.share"] = new QuestShareHandler();

	g_QueryManager.queryHandlers["mod.igforum.getcategory"] = new IGFGetCategoryHandler();
	g_QueryManager.queryHandlers["mod.igforum.opencategory"] = new IGFOpenCategoryHandler();
	g_QueryManager.queryHandlers["mod.igforum.openthread"] = new IGFOpenThreadHandler();
	g_QueryManager.queryHandlers["mod.igforum.sendpost"] = new IGFSendPostHandler();
	g_QueryManager.queryHandlers["mod.igforum.deletepost"] = new IGFDeletePostHandler();
	g_QueryManager.queryHandlers["mod.igforum.setlockstatus"] = new IGFSetLockStatusHandler();
	g_QueryManager.queryHandlers["mod.igforum.setstickystatus"] = new IGFSetStickyStatusHandler();
	g_QueryManager.queryHandlers["mod.igforum.editobject"] = new IGFEditObjectHandler();
	g_QueryManager.queryHandlers["mod.igforum.deleteobject"] = new IGFDeleteObjectHandler();
	g_QueryManager.queryHandlers["mod.igforum.runaction"] = new IGFRunActionHandler();
	g_QueryManager.queryHandlers["mod.igforum.move"] = new IGFMoveHandler();

	g_QueryManager.queryHandlers["trade.shop"] = new TradeShopHandler();
	g_QueryManager.queryHandlers["trade.items"] = new TradeItemsHandler();
	g_QueryManager.queryHandlers["trade.essence"] = new TradeEssenceHandler();
	g_QueryManager.queryHandlers["trade.start"] = new TradeStartHandler();
	g_QueryManager.queryHandlers["trade.cancel"] = new TradeCancelHandler();
	g_QueryManager.queryHandlers["trade.offer"] = new TradeOfferHandler();
	g_QueryManager.queryHandlers["trade.accept"] = new TradeAcceptHandler();
	g_QueryManager.queryHandlers["trade.currency"] = new TradeCurrencyHandler();

	g_QueryManager.queryHandlers["scenery.list"] = new SceneryListHandler();
	g_QueryManager.queryHandlers["scenery.edit"] = new SceneryEditHandler();
	g_QueryManager.queryHandlers["scenery.delete"] = new SceneryDeleteHandler();
	g_QueryManager.queryHandlers["scenery.link.add"] = new SceneryLinkAddHandler();
	g_QueryManager.queryHandlers["scenery.link.del"] = new SceneryLinkDelHandler();


	g_QueryManager.queryHandlers["bug.report"] = new BugReportHandler();
	g_QueryManager.queryHandlers["petition.send"] = new PetitionSendHandler();
	g_QueryManager.queryHandlers["petition.list"] = new PetitionListHandler();
	g_QueryManager.queryHandlers["petition.doaction"] = new PetitionDoActionHandler();

	g_QueryManager.queryHandlers["book.list"] = new BookListHandler();
	g_QueryManager.queryHandlers["book.get"] = new BookGetHandler();

	g_QueryManager.queryHandlers["item.use"] = new ItemUseHandler();
	g_QueryManager.queryHandlers["item.def.use"] = new ItemDefUseHandler();
	g_QueryManager.queryHandlers["item.contents"] = new ItemContentsHandler();
	g_QueryManager.queryHandlers["item.move"] = new ItemMoveHandler();
	g_QueryManager.queryHandlers["item.split"] = new ItemSplitHandler();
	g_QueryManager.queryHandlers["item.delete"] = new ItemDeleteHandler();
	g_QueryManager.queryHandlers["item.create"] = new ItemCreateHandler();
	g_QueryManager.queryHandlers["itemdef.contents"] = new ItemDefContentsHandler();
	g_QueryManager.queryHandlers["itemdef.delete"] = new ItemDefDeleteHandler();
	g_QueryManager.queryHandlers["item.morph"] = new ItemMorphHandler();
	g_QueryManager.queryHandlers["shop.contents"] = new ShopContentsHandler();
	g_QueryManager.queryHandlers["essenceShop.contents"] = new EssenceShopContentsHandler();

	g_QueryManager.queryHandlers["loot.list"] = new LootListHandler();
	g_QueryManager.queryHandlers["loot.item"] = new LootItemHandler();
	g_QueryManager.queryHandlers["loot.exit"] = new LootExitHandler();

	g_QueryManager.queryHandlers["friends.add"] = new AddFriendHandler();
	g_QueryManager.queryHandlers["friends.list"] = new ListFriendsHandler();
	g_QueryManager.queryHandlers["friends.remove"] = new RemoveFriendHandler();
	g_QueryManager.queryHandlers["friends.status"] = new FriendStatusHandler();
	g_QueryManager.queryHandlers["friends.getstatus"] = new GetFriendStatusHandler();

	g_QueryManager.queryHandlers["ab.remainingcooldowns"] = new AbilityRemainingCooldownsHandler();
	g_QueryManager.queryHandlers["ab.ownage.list"] = new AbilityOwnageListHandler();
	g_QueryManager.queryHandlers["ab.buy"] = new AbilityBuyHandler();
	g_QueryManager.queryHandlers["ab.respec"] = new AbilityRespecHandler();
	g_QueryManager.queryHandlers["ab.respec.price"] = new AbilityRespecPriceHandler();

	g_QueryManager.queryHandlers["creature.isusable"] = new CreatureIsUsableHandler();
	g_QueryManager.queryHandlers["creature.def.edit"] = new CreatureDefEditHandler();
	g_QueryManager.queryHandlers["creature.use"] = new CreatureUseHandler();
	g_QueryManager.queryHandlers["creature.delete"] = new CreatureDeleteHandler();

	g_QueryManager.queryHandlers["mod.pet.list"] = new PetListHandler();
	g_QueryManager.queryHandlers["mod.pet.purchase"] = new PetPurchaseHandler();
	g_QueryManager.queryHandlers["mod.pet.preview"] = new PetPreviewHandler();
	g_QueryManager.queryHandlers["mod.getpet"] = new GetPetHandler();

	g_QueryManager.queryHandlers["spawn.list"] = new SpawnListHandler();
	g_QueryManager.queryHandlers["spawn.create"] = new SpawnCreateHandler();
	g_QueryManager.queryHandlers["spawn.property"] = new SpawnPropertyHandler();
	g_QueryManager.queryHandlers["spawn.emitters"] = new SpawnEmittersHandler();

	g_QueryManager.queryHandlers["go"] = new GoHandler();
	g_QueryManager.queryHandlers["mod.grove.togglecycle"] = new GroveEnvironmentCycleToggleHandler();
	g_QueryManager.queryHandlers["mod.setenvironment"] = new SetEnvironmentHandler();
	g_QueryManager.queryHandlers["shard.list"] = new ShardListHandler();
	g_QueryManager.queryHandlers["shard.set"] = new ShardSetHandler();
	g_QueryManager.queryHandlers["gm.spawn"] = new GMSpawnHandler();
	g_QueryManager.queryHandlers["henge.setDest"] = new HengeSetDestHandler();
	g_QueryManager.queryHandlers["mod.morestats"] = new MoreStatsHandler();

	// Commands
	g_QueryManager.queryHandlers["team"] = new PVPTeamHandler();
	g_QueryManager.queryHandlers["mode"] = new PVPModeHandler();
	g_QueryManager.queryHandlers["help"] = new HelpHandler();
	g_QueryManager.queryHandlers["adjustexp"] = new AdjustExpHandler();
	g_QueryManager.queryHandlers["unstick"] = new UnstickHandler();
	g_QueryManager.queryHandlers["pose"] = new PoseHandler();
	g_QueryManager.queryHandlers["pose2"] = new Pose2Handler();
	g_QueryManager.queryHandlers["esay"] = new EsayHandler();
	g_QueryManager.queryHandlers["health"] = new HealthHandler();
	g_QueryManager.queryHandlers["speed"] = new SpeedHandler();
	g_QueryManager.queryHandlers["fa"] = new ForceAbilityHandler();
	g_QueryManager.queryHandlers["partylowest"] = new PartyLowestHandler();
	g_QueryManager.queryHandlers["who"] = new WhoHandler();
	g_QueryManager.queryHandlers["gmwho"] = new GMWhoHandler();
	g_QueryManager.queryHandlers["chwho"] = new CHWhoHandler();
	g_QueryManager.queryHandlers["give"] = new GiveHandler();
	g_QueryManager.queryHandlers["giveid"] = new GiveIDHandler();
	g_QueryManager.queryHandlers["giveall"] = new GiveAllHandler();
	g_QueryManager.queryHandlers["giveapp"] = new GiveAppHandler();
	g_QueryManager.queryHandlers["deleteall"] = new DeleteAllHandler();
	g_QueryManager.queryHandlers["deleteabove"] = new DeleteAboveHandler();
	g_QueryManager.queryHandlers["grove"] = new GroveHandler();
	g_QueryManager.queryHandlers["pvp"] = new PVPHandler();
	g_QueryManager.queryHandlers["complete"] = new CompleteHandler();
	g_QueryManager.queryHandlers["refashion"] = new RefashionHandler();
	g_QueryManager.queryHandlers["backup"] = new BackupHandler();
	g_QueryManager.queryHandlers["restore1"] = new RestoreHandler();
	g_QueryManager.queryHandlers["god"] = new GodHandler();
	g_QueryManager.queryHandlers["setstat"] = new SetStatHandler();
	g_QueryManager.queryHandlers["scale"] = new ScaleHandler();
	g_QueryManager.queryHandlers["partyall"] = new PartyAllHandler();
	g_QueryManager.queryHandlers["partyquit"] = new PartyQuitHandler();
	g_QueryManager.queryHandlers["ccc"] = new CCCHandler();
	g_QueryManager.queryHandlers["ban"] = new BanHandler();
	g_QueryManager.queryHandlers["unban"] = new UnbanHandler();
	g_QueryManager.queryHandlers["setpermission"] = new SetPermissionHandler();
	g_QueryManager.queryHandlers["setbuildpermission"] = new SetBuildPermissionHandler();
	g_QueryManager.queryHandlers["setpermissionc"] = new SetPermissionCHandler();
	g_QueryManager.queryHandlers["setbehavior"] = new SetBehaviorHandler();
	g_QueryManager.queryHandlers["deriveset"] = new DeriveSetHandler();
	g_QueryManager.queryHandlers["igstatus"] = new IGStatusHandler();
	g_QueryManager.queryHandlers["partyzap"] = new PartyZapHandler();
	g_QueryManager.queryHandlers["partyinvite"] = new PartyInviteHandler();
	g_QueryManager.queryHandlers["roll"] = new RollHandler();
	g_QueryManager.queryHandlers["forumlock"] = new ForumLockHandler();
	g_QueryManager.queryHandlers["zonename"] = new ZoneNameHandler();
	g_QueryManager.queryHandlers["dtrig"] = new DtrigHandler();
	g_QueryManager.queryHandlers["sdiag"] = new SdiagHandler();
	g_QueryManager.queryHandlers["sping"] = new SpingHandler();
	g_QueryManager.queryHandlers["info"] = new InfoHandler();
	g_QueryManager.queryHandlers["grovesetting"] = new GroveSettingHandler();
	g_QueryManager.queryHandlers["grovepermission"] = new GrovePermissionsHandler();
	g_QueryManager.queryHandlers["dngscale"] = new DngScaleHandler();
	g_QueryManager.queryHandlers["pathlinks"] = new PathLinksHandler();
	g_QueryManager.queryHandlers["targ"] = new TargHandler();
	g_QueryManager.queryHandlers["elev"] = new ElevHandler();
	g_QueryManager.queryHandlers["cycle"] = new CycleHandler();
	g_QueryManager.queryHandlers["searsize"] = new SearSizeHandler();
	g_QueryManager.queryHandlers["stailsize"] = new StailSizeHandler();
	g_QueryManager.queryHandlers["daily"] = new DailyHandler();
	g_QueryManager.queryHandlers["warp"] = new WarpHandler();
	g_QueryManager.queryHandlers["rot"] = new RotHandler();
	g_QueryManager.queryHandlers["warpi"] = new WarpInstanceHandler();
	g_QueryManager.queryHandlers["warpg"] = new WarpPullHandler();
	g_QueryManager.queryHandlers["warpt"] = new WarpTileHandler();
	g_QueryManager.queryHandlers["warpg"] = new WarpGroveHandler();
	g_QueryManager.queryHandlers["warpextoff"] = new WarpExternalOfflineHandler();
	g_QueryManager.queryHandlers["warpext"] = new WarpExternalHandler();
	g_QueryManager.queryHandlers["instance"] = new InstanceHandler();
	g_QueryManager.queryHandlers["script.exec"] = new ScriptExecHandler();
	g_QueryManager.queryHandlers["script.time"] = new ScriptTimeHandler();
	g_QueryManager.queryHandlers["script.gc"] = new ScriptGCHandler();
	g_QueryManager.queryHandlers["script.wakevm"] = new ScriptWakeVMHandler();
	g_QueryManager.queryHandlers["script.clearqueue"] = new ScriptClearQueueHandler();

	// Some are shared
	LootNeedGreedPassHandler *lootNeedGreedPassHandler = new LootNeedGreedPassHandler();
	g_QueryManager.queryHandlers["loot.need"] = lootNeedGreedPassHandler;
	g_QueryManager.queryHandlers["loot.greed"] = lootNeedGreedPassHandler;
	g_QueryManager.queryHandlers["loot.pass"] = lootNeedGreedPassHandler;

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



	g_Logs.data->info("Loaded %v checksums.", g_FileChecksum.mChecksumData.size());

	g_CharacterManager.CreateDefaultCharacter();
	g_ItemManager.LoadData();
	g_ItemSetManager.LoadData();

	g_ModManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "ModTables.txt"));
	g_Logs.data->info("Loaded %v ModTables.", g_ModManager.modTable.size());
	g_ModTemplateManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "ModTemplates.txt"));
	g_Logs.data->info("Loaded %v ModTemplates.", g_ModTemplateManager.equipTemplate.size());
	g_EquipAppearance.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Appearances.txt"));
	g_Logs.data->info("Loaded %v EquipAppearance.", g_EquipAppearance.dataEntry.size());
	g_EquipIconAppearance.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Icons.txt"));
	g_Logs.data->info("Loaded %v EquipIconAppearance.", g_EquipIconAppearance.dataEntry.size());
	g_EquipTable.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "EquipTable.txt"));
	g_Logs.data->info("Loaded %v EquipTable.", g_EquipTable.equipList.size());
	g_NameTemplateManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "Names.txt"));
	g_Logs.data->info("Loaded %v Name Templates.", g_NameTemplateManager.nameTemplate.size());
	g_NameModManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "NameMod.txt"));
	g_Logs.data->info("Loaded %v NameMods", g_NameModManager.mModList.size());
	g_NameWeightManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "NameWeight.txt"));
	g_Logs.data->info("Loaded %v NameWeight", g_NameWeightManager.mWeightList.size());
	g_VirtualItemModSystem.LoadSettings();
	g_DropTableManager.LoadData();

	g_AbilityManager.LoadData();


	//g_VirtualItemModSystem.Debug_RunDropDiagnostic(20);

	//g_EquipAppearance.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_appoutput2.txt"));
	//g_EquipIconAppearance.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_Icons2.txt"));
	//g_NameTemplateManager.DebugSaveToFile(Platform::GenerateFilePath(GAuxBuf, "ItemMod", "sorted_Names.txt"));

	//g_EquipAppearance.Debug_CheckForNames();


	CreatureDef.LoadPackages(Platform::GenerateFilePath(GAuxBuf, "Packages", "CreaturePack.txt"));
	g_Logs.data->info("Loaded %v CreatureDefs.", CreatureDef.NPC.size());

	g_PetDefManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "Pets.txt"));
	g_Logs.data->info("Loaded %v PetDefs.", g_PetDefManager.GetStandardCount());

	g_AccountManager.LoadAllData();

	g_GambleManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "Gamble.txt"));
	g_Logs.data->info("Loaded %v Gamble definitions.", g_GambleManager.GetStandardCount());

	g_GuildManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "GuildDef.txt"));
	g_Logs.data->info("Loaded %v Guild definitions.", g_GuildManager.GetStandardCount());

	g_ClanManager.LoadClans();
	g_Logs.data->info("Loaded %v Clans.", g_ClanManager.mClans.size());
	g_BookManager.Init();
	g_Logs.data->info("Loaded %v Books.", g_BookManager.books.size());

	g_CreditShopManager.LoadItems();
	g_Logs.data->info("Loaded %v Credit Shop items.", g_CreditShopManager.mItems.size());

	g_AuctionHouseManager.LoadItems();
	g_AuctionHouseManager.ConnectToSite();
	g_Logs.data->info("Loaded %v Auction House items.", g_CreditShopManager.mItems.size());

	g_ZoneDefManager.LoadData();
	g_GroveTemplateManager.LoadData();

	g_ZoneBarrierManager.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapBarrier.txt"));
	g_Logs.data->info("Loaded %v MapBarrier.", g_ZoneBarrierManager.GetLoadedCount());

	g_ZoneMarkerDataManager.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "ZoneMarkers.txt"));
	g_Logs.data->info("Loaded %v zones with marker data.", g_ZoneMarkerDataManager.zoneList.size());

	MapDef.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapDef.txt"));
	g_Logs.data->info("Loaded %v MapDef.", MapDef.mMapList.size());

	MapLocation.LoadFile(Platform::GenerateFilePath(GAuxBuf, "Data", "MapLocations.txt"));
	g_Logs.data->info("Loaded %v MapLocation zones.", MapLocation.mLocationSet.size());

	g_SceneryManager.LoadData();

	g_SpawnPackageManager.LoadFromFile("SpawnPackages", "SpawnPackageList.txt");
	g_Logs.data->info("Loaded %v Spawn Package lists.", g_SpawnPackageManager.packageList.size());

	QuestDef.LoadQuestPackages(Platform::GenerateFilePath(GAuxBuf, "Packages", "QuestPack.txt"));
	g_Logs.data->info("Loaded %v Quests.", QuestDef.mQuests.size());
	QuestScript::LoadQuestScripts(Platform::GenerateFilePath(GAuxBuf, "Data", "QuestScript.txt"));
	QuestDef.ResolveQuestMarkers();

	g_InteractObjectContainer.LoadFromFile(Platform::GenerateFilePath(GAuxBuf, "Data", "InteractDef.txt"));
	g_Logs.data->info("Loaded %v InteractDef.", g_InteractObjectContainer.objList.size());

	g_CraftManager.LoadData();

	g_EliteManager.LoadData();

	Global::LoadResCostTable(Platform::GenerateFilePath(GAuxBuf, "Data", "ResCost.txt"));

	g_InstanceScaleManager.LoadData();
	g_DropRateProfileManager.LoadData();

	g_DailyProfileManager.LoadData();
	g_Logs.data->info("Loaded %v Daily Profiles.", g_DailyProfileManager.GetNumberOfProfiles());

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

	if(daemonize) {
		int ret = daemon(1, DAEMON_NO_CLOSE);
		if(ret == 0) {
			g_Logs.server->info("Daemonized!\n");
			FILE *output = fopen(pidfile.c_str(), "wb");
			if(output != NULL) {
				fprintf(output, "%d\n", getpid());
				fclose(output);
			}
		}
		else {
			g_Logs.FlushAll();
			g_Logs.server->fatal("Failed to daemonize. %v\n", ret);
			exit(1);
		}
	}

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

	g_Logs.data->info("Server data has finished loading.");

	g_HTTPService.Start();

	g_ServerLaunchTime = g_PlatformTime.getMilliseconds();
	srand(g_ServerLaunchTime & Platform::MAX_UINT);

	g_ServerStatus = SERVER_STATUS_RUNNING;
	if(VerifyOperation() == false)
		g_ServerStatus = SERVER_STATUS_STOPPED;

	RunUpgradeCheck();

#ifdef WINDOWS_SERVICE
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus (hStatus, &ServiceStatus);
#endif

	SystemLoop_Console();

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
		if(!crashing) {
			POPUP_MESSAGE("An exception occurred, shutting down.", "Critical Error");
			Debug_FullDump();
			ShutDown();
			Exception = true;
		}
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
		if(crashing) {
			g_Logs.server->fatal("Exception occurred while handling a crash.");
		} else {
			Debug_FullDump();
		}
	}
}

bool VerifyOperation(void)
{
	// Return true if the server appears to have loaded (all required
	// listening ports are established).

	// If the GUI is functional, the admins can monitor the output and
	// and manually shut down the server for themselves.

	// Otherwise check the ports.  If any are not operational, prompt
	// the user if they want to quit.

	// Wait a couple seconds to give the threads a chance to start up.
	// Flush messages so any previous messages from the loading stage
	// are visible.
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
	g_Logs.FlushAll();
	g_Logs.CloseAll();

	SaveSession("SessionVars.txt");
	g_IGFManager.CheckAutoSave(true);

	//VarDump();
	CheckCharacterAutosave(true);

	g_PacketManager.ShutdownThread();
	g_SceneryManager.ShutdownThread();

	g_SceneryManager.CheckAutosave(true);
	g_AccountManager.CheckAutoSave(true);
	g_AccountManager.RunUpdateCycle(true);
	g_ZoneDefManager.CheckAutoSave(true);
	g_ItemManager.CheckVirtualItemAutosave(true);

	g_Logs.server->info("Waiting for threads to shut down...");

	//To shut down the threads:
	//
	// Set the thread isActive status to false so that the thread's main loop
	// will quit at the next available opportunity.
	//
	// Sockets may be stuck on the accept() or recv() stages.
	// Call ShutdownServer() for systems that are listening for connections.
	// Call DisconnectClient() for systems that are connected to sockets.

	g_Logs.server->info("Shutting down HTTP");
	g_HTTPService.Shutdown();

	if(g_RouterPort != 0)
	{
		if(Router.isExist == true)
		{
			g_Logs.server->info("Shutting down Router");
			Router.isActive = false;
			Router.sc.ShutdownServer();
		}
	}

	g_LeaderboardManager.Shutdown();

	g_Logs.server->info("Shutting down SimulatorBase");
	SimulatorBase.isActive = false;
	SimulatorBase.sc.ShutdownServer();
	g_Logs.server->info("Threads shut down, disconnecting all clients...");

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		it->isThreadActive = false;
		it->sc.DisconnectClient();
	}

	//If any of the simulators are waiting 
	long waitCount = 0;
	if(ActiveComponents > 0) {
		g_Logs.server->info("Running any pending actions.");
	}
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
		g_Logs.server->debug("Shutdown() waitcount: %v", waitCount);

	//Characters can be unloaded only after the simulators have stopped using them.
	g_CharacterManager.UnloadAllCharacters();
}

void UnloadResources(void)
{
	g_Logs.server->info("Unloading server resources");

	g_AccountManager.UnloadAllData();
	g_CharacterManager.Clear();
	//DeleteCharacterList();

#ifdef WINDOWS_PLATFORM
	WSACleanup();
#endif

	bcm.Free();

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
	g_Logs.FlushAll();
		//Debugger.Destroy();
	TRACE_FREE();
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
				g_Logs.simulator->info("[%v] Erasing sim", it->InternalID);
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
		g_Logs.chat->flush();

	g_SimulatorManager.RunPendingActions();
	g_CharacterManager.CheckGarbageCharacters();

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
		g_Logs.server->info("Finished autosave on %v characters.", count);
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
			g_Logs.server->error("Invalid Active Instance [%v] message for Sim:%v", msg->actInst, msg->SimulatorID);
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
	if(cInst->serverFlags & ServerFlags::IsPlayer)
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
	g_Logs.server->fatal("Writing crash dump.");
	FILE *output = fopen("crash_dump.txt", "wb");
	if(output == NULL)
		return;

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
			g_Logs.server->error("Account not found: %v", g_AccountManager.accountQuickData[i].mLoginName.c_str());
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

	g_Logs.server->info("g_Config: Server upgraded, shutting down.");
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
