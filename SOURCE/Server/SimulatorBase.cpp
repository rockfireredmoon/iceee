#include "SimulatorBase.h"

#include "Config.h"
#include "Globals.h"
#include "StringList.h"
#include "Simulator.h"
#include <stdlib.h>   //For itoa()
#include "Util.h"
#include "util/Log.h"

PLATFORM_THREADRETURN SimulatorBaseThreadProc(PLATFORM_THREADARGS lpParam);
SimulatorBaseThread SimulatorBase;

SimulatorBaseThread :: SimulatorBaseThread()
{
	ResetValues(true);
	sc.SetDebugName("SIM_BASE");
}

SimulatorBaseThread :: ~SimulatorBaseThread()
{
	sc.ShutdownServer();
}

void SimulatorBaseThread :: SetHomePort(int port)
{
	//Takes an integer value and converts it into a port string, since this is what
	//the socket init call requires.
	HomePort = port;
	sprintf(HomePortStr, "%d", port);
}

void SimulatorBaseThread :: SetBindAddress(const char *address)
{
	sprintf(BindAddress, "%s", address);
}

void SimulatorBaseThread :: ResetValues(bool fullRestart)
{
	//Erases the receiving buffer and resets the byte counters.
	//Optionally if <fullRestart> is set to true, it performs a hard reset
	//of the thread.
	memset(RecBuf, 0, sizeof(RecBuf));
	memset(LogBuffer, 0, sizeof(LogBuffer));

	MessageCountRec = 0;

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
		memset(HomePortStr, 0, sizeof(HomePortStr));
		HomePort = 0;
	}
}

int SimulatorBaseThread :: InitThread(int instanceindex, int globalThreadID)
{
	//LogMessageL("[HTTP:%d] Address: %08X", instanceindex, this);
	ResetValues(false);
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;

	//Set to active now because the thread might launch and quit 
	//before it gets set to active later.

	/*
	HANDLE h = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)SimulatorBaseThreadProc, this, 0, &ThreadID);
	if(h == NULL)
	{
		isActive = false;
		LogMessageL("[SimB] Could not create thread.");
		return 1;
	}
	*/
	int r = Platform_CreateThread(0, (void*)SimulatorBaseThreadProc, this, &ThreadID);
	if(r == 0)
	{
		isActive = false;
		g_Logs.simulator->error("[SimB] Could not create thread.");
		return 1;
	}
	return 0;
}

void SimulatorBaseThread :: OnConnect(void)
{
	return;
}

void SimulatorBaseThread :: Restart(void)
{
	sc.ShutdownServer();
	Status = Status_Init;
}
void SimulatorBaseThread :: Shutdown(void)
{
	isActive = false;
	Status = Status_Restart;
	sc.ShutdownServer();
}

PLATFORM_THREADRETURN SimulatorBaseThreadProc(PLATFORM_THREADARGS lpParam)
{
	SimulatorBaseThread *controller = (SimulatorBaseThread*)lpParam;

	controller->isActive = true;
	controller->isExist = true;
	AdjustComponentCount(1);

	controller->RunMainLoop();

	// Thread has been deactivated, shut it down
	controller->sc.ShutdownServer();

	g_Logs.simulator->info("[SimB] Thread shut down.");

	controller->isExist = false;

	AdjustComponentCount(-1);
	return 0;
}

void SimulatorBaseThread :: RunMainLoop(void)
{
	while(isActive == true)
	{
		if(Status == Status_Ready)
		{
			//The simulator base is responsible for delegating incoming connections
			//to their Simulator object.
			//This listening port is not responsible for sending or receiving
			//data.
			Status = Status_Kick;
		}
		else if(Status == Status_Init)
		{
			if(sc.CreateSocket(HomePortStr, BindAddress) == 0)
			{
				g_Logs.simulator->info("[SimB] Server created, awaiting connection on port %v (socket:%v).", HomePort, sc.ListenSocket);
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
				LaunchSimulatorThread();
				sc.ClientSocket = SocketClass::Invalid_Socket;
			}
			else
			{
				if(sc.disconnecting) {
					g_Logs.simulator->info("SimulatorBase shutdown.");
					Status = Status_None;
				}
				else {
					g_Logs.simulator->info("Socket error: %v", sc.GetErrorMessage());
					//This shouldn't normally fail.  Need a complete restart.
					Status = Status_Restart;
				}
			}
		}
		else if(Status == Status_Restart)
		{
			g_Logs.simulator->info("[SimB] Disconnecting server.");
			sc.ShutdownServer();
			Status = Status_Init;
			PLATFORM_SLEEP(g_ErrorSleep);
		}
		else if(Status == Status_Kick)
		{
			g_Logs.simulator->info("[SimB] Kicking client.");
			sc.DisconnectClient();
			Status = Status_Wait;  //Wait for another connection.
		}
		else
		{
			g_Logs.simulator->error("[SimB] Unknown status.");
			Status = Status_Restart;
		}
		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(SleepDelayNormal);
	}
}

void SimulatorBaseThread :: CheckAutoResponse(void)
{
}

int SimulatorBaseThread :: LaunchSimulatorThread(void)
{
	ThreadRequest threadReq;
	threadReq.status = threadReq.STATUS_WAITMAIN;
	g_SimulatorManager.RegisterAction(&threadReq);
	if(threadReq.WaitForStatus(ThreadRequest::STATUS_WAITWORK, 1, ThreadRequest::DEFAULT_WAIT_TIME) == true)
	{
		SimulatorThread *simPtr = g_SimulatorManager.CreateSimulator();
		if(simPtr != NULL)
		{
			simPtr->ResetValues(true);
			simPtr->InternalID = g_SimulatorManager.nextSimulatorID++;
			simPtr->sim_cs.Reset();
			char buffer[256];
			sprintf(buffer, "CS_SIM:%d", simPtr->InternalID);
			simPtr->sim_cs.Init();
			simPtr->sim_cs.SetDebugName(buffer);
			simPtr->sc.TransferClientSocketFrom(sc);
			simPtr->sc.SetClientNoDelay();
			simPtr->sc.SetTimeOut(5);
			simPtr->LastUpdate = g_ServerTime;

			int res = simPtr->InitThread(simPtr->InternalID, g_GlobalThreadID++);
			if(res != 0)
				g_Logs.simulator->info("[SimB] Passing over to simulator ID:%v (socket:%v).", simPtr->InternalID, sc.ClientSocket);
			else
				g_Logs.simulator->fatal("[SimB] Failed to launch thread. %v", simPtr->InternalID);
		}
		else
		{
			g_Logs.simulator->fatal("[SimB] SimBase failed to create simulator.");
		}
	}
	else
	{
		g_Logs.simulator->fatal("[SimB] SimBase failed to call a thread launch.");
	}

	threadReq.status = ThreadRequest::STATUS_COMPLETE;
	g_SimulatorManager.UnregisterAction(&threadReq);
	return 0;
}
