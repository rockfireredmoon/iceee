/*
 *This file is part of TAWD.
 *
 * TAWD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TAWD is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TAWD.  If not, see <http://www.gnu.org/licenses/
 */

//Enable to include memory leak detection using CRT runtimes (Windows only)
//#define _CRTDEBUGGING

#include <CompilerEnvironment.h>

#ifdef _CRTDEBUGGING
 #define _CRTDBG_MAP_ALLOC
#endif

#include <stdlib.h>

#ifdef _CRTDEBUGGING
 #include <crtdbg.h>
#endif
#include <DebugTracer.h>
#include <util/Log.h>
#include <Components.h>
#include <SocketClass3.h>
#include <Globals.h>
#include <Util.h>
#include <Debug.h>
#include <Config.h>
#include <DirectoryAccess.h>
#include <http/HTTPService.h>
#include <http/CAR.h>

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

int InitServerMain(int argc, char *argv[]);
void RunServerMain(void);
void ShutDown(void);
void UnloadResources(void);

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

bool crashing;

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

	bool ok = signum == SIGTERM || signum == SIGINT;

	if(!ok) {
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
		g_Logs.server->fatal("Number of poXinters: %v", numPtr);
		char **result = backtrace_symbols(ptrBuf, numPtr);
		g_Logs.server->fatal("Stack trace:");
		if(result != NULL)
		{
			for(int a = 0; a < numPtr; a++) {
				std::string line = "UNKNOWN";
				std::vector<std::string> parts;
				Util::Split(result[a], " ", parts);
				if(parts.size() > 0) {
					std::string addr = parts[parts.size() - 1];
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

	ShutDown();
	UnloadResources();

	g_Logs.server->info("Exiting");
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
	ServiceTable[0].lpServiceName = "EEHTTPD";
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
		"EEHTTPD",
		(LPHANDLER_FUNCTION)ControlHandler);

    if (hStatus == (SERVICE_STATUS_HANDLE)0) {
        return;
    }

    InitServerMain();
}
#endif

int InitServerMain(int argc, char *argv[]) {
	TRACE_INIT(250);

	if(PLATFORM_GETCWD(g_WorkingDirectory, 256) == NULL) {
		printf("Failed to get current working directory.");
		return 0;
	}

	bool daemonize = false;
	std::string pidfile = "";
	el::Level lvl = el::Level::Warning;
	bool consoleOut = false;
	bool configSet = false;

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
		else if(strcmp(argv[i], "-c") == 0) {
			if(!configSet) {
				configSet = true;
				g_Config.LocalConfigurationPath.clear();
			}
			g_Config.LocalConfigurationPath.push_back(argv[++i]);
		}
		else if(strcmp(argv[i], "-C") == 0) {
			consoleOut = true;
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
					lvl = el::Level::Info;
				}
				else if(str.compare("debug") == 0) {
					lvl = el::Level::Debug;
				}
				else if(str.compare("error") == 0) {
					lvl = el::Level::Error;
				}
				else if(str.compare("fatal") == 0) {
					lvl = el::Level::Fatal;
				}
				else if(str.compare("trace") == 0) {
					lvl = el::Level::Trace;
				}
				else if(str.compare("warning") == 0) {
					lvl = el::Level::Warning;
				}
			}
		}
	}

	g_Logs.Init(lvl, consoleOut);

	std::vector<std::string> paths = g_Config.ResolveLocalConfigurationPath();
	for (std::vector<std::string>::iterator it = paths.begin();
			it != paths.end(); ++it) {
		std::string dir = *it;
		std::string filename = Platform::JoinPath(dir, "HTTPConfig.txt");
		if(!LoadConfig(filename) && it == paths.begin())
			g_Logs.data->error("Could not open HTTP configuration file: %v", filename);
	}
	g_Logs.server->info("Working directory %v.", g_WorkingDirectory);

	// Initialise time
	g_PlatformTime.Init();
	g_ServerTime = g_PlatformTime.getMilliseconds();

#ifdef WINDOWS_PLATFORM
	WSAData wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(res != 0)
		LogMessage("WSAStartup failed: %d\n", res);
#endif

	g_FileChecksum.LoadFromFile();
	g_Logs.data->info("Loaded %v checksums.",
			g_FileChecksum.mChecksumData.size());

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

	g_Logs.data->info("Starting HTTP server.");

	g_HTTPService.RegisterHandler("**.car$", new HTTPD::CARHandler());
	g_HTTPService.Start();

	srand(g_ServerLaunchTime & Platform::MAX_UINT);

	g_ServerStatus = SERVER_STATUS_RUNNING;

	g_Logs.server->verbose(0, "The server is ready");

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
		}
	}
}

void ShutDown(void)
{
	g_Logs.FlushAll();
	g_Logs.CloseAll();

	g_Logs.server->info("Waiting for threads to shut down...");
	g_Logs.server->info("Shutting down HTTP");
	g_HTTPService.Shutdown();
}

void UnloadResources(void)
{
	g_Logs.server->info("Unloading server resources");
#ifdef WINDOWS_PLATFORM
	WSACleanup();
#endif
	g_Logs.FlushAll();
		//Debugger.Destroy();
	TRACE_FREE();
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

	g_ServerTime = g_PlatformTime.getMilliseconds();
}

