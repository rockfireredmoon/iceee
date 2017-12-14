#include "Router.h"

#include "Config.h"
#include "Globals.h"
#include "Simulator.h"

#include <stdlib.h>  //For itoa()
#include "Util.h"
#include "util/Log.h"

//This is the main function of the router thread.  A thread must be created for each port
//since the connecting function will halt until a connection is established.
//DWORD WINAPI RouterThreadProc(LPVOID lpParam);
PLATFORM_THREADRETURN RouterThreadProc(PLATFORM_THREADARGS lpParam);

RouterThread Router;

RouterThread :: RouterThread()
{
	ResetValues(true);
	sc.SetDebugName("ROUTER");
}

RouterThread :: ~RouterThread()
{
	sc.ShutdownServer();
}

void RouterThread :: SetHomePort(int port)
{
	//Takes an integer value and converts it into a port string, since this is what
	//the socket init call requires.
	HomePort = port;
	sprintf(HomePortStr, "%d", port);
}

void RouterThread :: SetBindAddress(const char *address)
{
	sprintf(BindAddress, "%s", address);
}

void RouterThread :: SetTargetPort(int port)
{
	//Takes an integer value and converts it into a port string, this will be the
	//port sent to the target.
	TargetPort = port;
	sprintf(TargetPortStr, "%d", port);
}

void RouterThread :: ResetValues(bool fullRestart)
{
	//Erases the receiving buffer and resets the byte counters.
	//Optionally if <fullRestart> is set to true, it performs a hard reset
	//of the thread.
	memset(RecBuf, 0, sizeof(RecBuf));
	memset(LogBuffer, 0, sizeof(LogBuffer));

	InternalIndex = 0;
	GlobalThreadID = 0;

	RecBytes = 0;
	TotalRecBytes = 0;

	SendBytes = 0;
	TotalSendBytes = 0;

	if(fullRestart == true)
	{
		ThreadID = 0;
		isExist = false;
		isActive = false;
		Status = Status_Init;
		memset(SimTarget, 0, sizeof(SimTarget));
		memset(HomePortStr, 0, sizeof(HomePortStr));
		memset(TargetPortStr, 0, sizeof(TargetPortStr));
		HomePort = 0;
		TargetPort = 0;
	}
}

int RouterThread :: InitThread(int instanceindex, int globalThreadID)
{
	//LogMessageL("[Router:%d] Address: %08X", instanceindex, this);
	ResetValues(false);
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;

	/*
	HANDLE h = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)RouterThreadProc, this, 0, &ThreadID);
	if(h == NULL)
	{
		isActive = false;
		LogMessageL("[Router:%d] Could not create thread.", instanceindex);
		return 1;
	}
	*/

	int r = Platform_CreateThread(0, (void*)RouterThreadProc, this, &ThreadID);
	if(r == 0)
	{
		isActive = false;
		g_Logs.router->error("[%v] Could not create thread.", instanceindex);
		return 1;
	}
	return 0;
}

void RouterThread :: OnConnect(const char *address)
{
	//This is the router's job, resolve the target Simulator's address, send
	//the address string, then restart the thread.
	int res = ResolvePort(address, g_SimulatorPort);
	if(res != 0)
	{
		int size = strlen(SimTarget);
		sc.AttemptSend(SimTarget, size);
		g_Logs.router->info("[%v] Sent connection string: %v", InternalIndex, SimTarget);
		TotalSendBytes += size;
	}

	sc.DisconnectClient();
}

void RouterThread :: Shutdown(void)
{
	isActive = false;
	Status = Status_Restart;
	sc.ShutdownServer();
}

int RouterThread :: ResolvePort(const char *address, int port)
{
	if(strlen(g_SimulatorAddress) == 0)
		sprintf(SimTarget, "%s:%d", address, port);
	else
		sprintf(SimTarget, "%s:%d", g_SimulatorAddress, port);
	return 1;
}

void RouterThread :: Restart(void)
{
	sc.ShutdownServer();
	Status = Status_Init;
}


//This thread exists outside the class, but a new instance is created when a new
//thread is launched.  It takes a parameter of the address back to its router class
//so it can call the necessary functions.
//DWORD WINAPI RouterThreadProc(LPVOID lpParam)
PLATFORM_THREADRETURN RouterThreadProc(PLATFORM_THREADARGS lpParam)
{
	RouterThread *controller = (RouterThread*)lpParam;
	controller->isExist = true;
	controller->isActive = true;
	AdjustComponentCount(1);

	//The Router thread will loop within its own main function until it's shut down, or
	//a critical error is encountered.
	controller->RunMainLoop();

	// Thread has been deactivated, shut it down
	controller->sc.ShutdownServer();

	g_Logs.router->info("Thread shut down.");
	controller->isExist = false;

	AdjustComponentCount(-1);
	PLATFORM_CLOSETHREAD(0);
	return 0;
}

void RouterThread :: RunMainLoop(void)
{
	while(isActive == true)
	{
		if(Status == Status_Ready)
		{
			Status = Status_Kick;
		}
		else if(Status == Status_Init)
		{
			if(sc.CreateSocket(HomePortStr, BindAddress) == 0)
			{
				g_Logs.router->info("Server created, awaiting connection on port %v (socket:%v)", HomePort, sc.ListenSocket);
				Status = Status_Wait;
			}
			else
			{
				//Keep trying, but wait a bit longer than normal.
				PLATFORM_SLEEP(g_ErrorSleep);
			}
		}
		else if(Status == Status_Wait)
		{
			int res = sc.Accept();
			if(res == 0)
			{
				OnConnect(sc.destAddr);
				//Job is done, get rid of the client.
				Status = Status_Kick;
			}
			else
			{
				if(sc.disconnecting) {
					g_Logs.router->info("Router shutdown.");
					Status = Status_None;
				}
				else {
					g_Logs.router->error("Socket error: %v", sc.GetErrorMessage());
					//This shouldn't normally fail.  Need a complete restart.
					Status = Status_Restart;
				}
			}
		}
		else if(Status == Status_Restart)
		{
			g_Logs.router->info("Disconnecting server.");
			sc.ShutdownServer();
			Status = Status_Init;
			PLATFORM_SLEEP(g_ErrorSleep);
		}
		else if(Status == Status_Kick)
		{
			sc.DisconnectClient();
			Status = Status_Wait;  //Wait for another connection.
		}
		else
		{
			g_Logs.router->warn("Unknown status %v.", Status);
			Status = Status_Restart;
		}
		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(SleepDelayNormal);
	}
}

