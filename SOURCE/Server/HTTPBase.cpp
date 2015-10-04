
#include "HTTPBase.h"

#include "Config.h"
#include "Globals.h"
#include "StringList.h"
#include "HTTPDistribute.h"
#include "Components.h"
#include <string.h>
#include <stdio.h>
#include "Util.h"

PLATFORM_THREADRETURN HTTPBaseThreadProc(PLATFORM_THREADARGS lpParam);
HTTPBaseThread HTTPBaseServer;

extern Platform_CriticalSection http_cs;

HTTPBaseThread :: HTTPBaseThread()
{
	ResetValues(true);
	NextThreadID = 0;
	http_cs.SetDebugName("CS_HTTPBASE");
	sc.SetDebugName("HTTP_BASE");
}

HTTPBaseThread :: ~HTTPBaseThread()
{
	sc.ShutdownServer();
}

void HTTPBaseThread :: SetHomePort(int port)
{
	//Takes an integer value and converts it into a port string, since this is what
	//the socket init call requires.
	HomePort = port;
	sprintf(HomePortStr, "%d", port);
}

void HTTPBaseThread :: SetBindAddress(const char *address)
{
	//Takes an const value and converts it into a port string, since this is what
	//the socket init call requires.
	sprintf(BindAddress, "%s", address);
}

void HTTPBaseThread :: ResetValues(bool fullRestart)
{
	//Erases the receiving buffer and resets the byte counters.
	//Optionally if <fullRestart> is set to true, it performs a hard reset
	//of the thread.
	memset(RecBuf, 0, sizeof(RecBuf));
	memset(LogBuffer, 0, sizeof(LogBuffer));

	MessageCountRec = 0;

	InternalIndex = 0;
	GlobalThreadID = 0;

	Debug_MaxFullConnections = 0;
	Debug_KickedFullConnections = 0;

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

int HTTPBaseThread :: InitThread(int instanceindex, int globalThreadID)
{
	//LogMessageL("[HTTP:%d] Address: %08X", instanceindex, this);
	ResetValues(false);
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;

	/*
	HANDLE h = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)HTTPBaseThreadProc, this, 0, &ThreadID);
	if(h == NULL)
	{
		isActive = false;
		LogMessageL("[HTTP:%d] Could not create thread.", InternalIndex);
		return 1;
	}*/
	int r = Platform_CreateThread(0, (void*)HTTPBaseThreadProc, this, &ThreadID);
	if(r == 0)
	{
		isActive = false;
		LogMessageL(LOG_CRITICAL, "[HTTP:%d] Could not create thread.", InternalIndex);
		return 1;
	}
	return 0;
}

void HTTPBaseThread :: OnConnect(void)
{
}

void HTTPBaseThread :: Shutdown(void)
{
	http_cs.Enter("HTTPBaseThread::Shutdown");
	isActive = false;
	sc.ShutdownServer();
	Status = Status_Restart;
	http_cs.Leave();
}

//DWORD WINAPI HTTPBaseThreadProc(LPVOID lpParam)
PLATFORM_THREADRETURN HTTPBaseThreadProc(PLATFORM_THREADARGS lpParam)
{
	HTTPBaseThread *controller = (HTTPBaseThread*)lpParam;
	controller->isActive = true;
	controller->isExist = true;
	AdjustComponentCount(1);

	while(controller->isActive == true)
	{
		if(controller->Status == Status_Ready)
		{
			//This listening port is not responsible for sending or receiving
			//data.
			controller->Status = Status_Kick;
		}
		else if(controller->Status == Status_Init)
		{
			if(controller->sc.CreateSocket(controller->HomePortStr, controller->BindAddress) == 0)
			{
				controller->LogMessageL(LOG_ALWAYS, "[HTTP] Server created, awaiting connection on port %d (socket:%d).", controller->HomePort, controller->sc.ListenSocket);
				controller->Status = Status_Wait;
			}
			else
			{
				//Keep trying, but wait a bit longer than normal.
				PLATFORM_SLEEP(g_ErrorSleep);
			}
		}
		else if(controller->Status == Status_Wait)
		{
			int res = controller->sc.Accept();
			if(res == 0)
			{
				controller->LaunchDistributeThread();
			}
			else
			{
				controller->LogMessageL(LOG_ERROR, "[HTTP] Socket error: %s", controller->sc.GetErrorMessage());
				//This shouldn't normally fail.  Need a complete restart.
				controller->Status = Status_Restart;
			}
		}
		else if(controller->Status == Status_Restart)
		{
			controller->LogMessageL(LOG_ERROR, "[HTTP] Disconnecting server.");
			controller->sc.ShutdownServer();
			controller->Status = Status_Init;
			PLATFORM_SLEEP(g_ErrorSleep);
		}
		else if(controller->Status == Status_Kick)
		{
			controller->LogMessageL(LOG_ERROR, "[HTTP] Kicking client.");
			controller->sc.DisconnectClient();
			controller->Status = Status_Wait;  //Wait for another connection.
		}
		else
		{
			controller->LogMessageL(LOG_CRITICAL, "[HTTP] Unknown status.");
			controller->Status = Status_Restart;
		}
		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(SleepDelayNormal);
	}

	// Thread has been deactivated, shut it down
	controller->sc.ShutdownServer();

	controller->LogMessageL(LOG_ALWAYS, "[HTTP] Thread shut down.", controller->InternalIndex);

	controller->isExist = false;

	AdjustComponentCount(-1);
	PLATFORM_CLOSETHREAD(0);
	return 0;
}

char * HTTPBaseThread :: LogMessageL(int logLevel, const char *format, ...)
{
	if(logLevel > g_Config.LogLevelHTTPBase)
		return NULL;

	va_list args;
	va_start (args, format);
	//vsnprintf(LogBuffer, maxSize, format, args);
	Util::SafeFormatArg(LogBuffer, sizeof(LogBuffer), format, args);
	va_end (args);

	g_Log.AddMessage(LogBuffer);
	return LogBuffer;
}

void HTTPBaseThread :: CheckAutoResponse(void)
{
}

int HTTPBaseThread :: LaunchDistributeThread(void)
{
	//Going safe here, since this function attempts to find a distribute thread
	//that isn't in use, even though the distribute threads themselves may be
	//adjusting this information.

	HTTPDistribute *hDist = NULL;
	hDist = g_HTTPDistributeManager.GetNewDistributeSlot();
	if(hDist == NULL)
	{
		LogMessageL(LOG_CRITICAL, "No available HTTP distribute server.");
		sc.DisconnectClient();
		return -1;
	}

	hDist->sc.TransferClientSocketFrom(sc);
	hDist->sc.SetClientNoDelay();
	hDist->sc.SetDebugName("HTTP_DIST");
	hDist->Status = Status_Ready;
	hDist->LastAction = g_ServerTime;
	
	memset(&hDist->remotename, 0, sizeof(hDist->remotename));
	socklen_t size = sizeof(hDist->remotename);
	getpeername(hDist->sc.ClientSocket, &hDist->remotename, &size);

	//Calculate an expire time based on which slot this entry is.  Lower slots
	//will exist longer to minimize thread generation.
	// 300000 = 5 minutes
	int slot = g_HTTPDistributeManager.GetSlotCount();
	int factor = 1000;
	unsigned long timeDelay = 300000 - ((slot * slot) * factor);
	if(timeDelay < 30000)
		timeDelay = 30000; 
	hDist->ExpireDelay = timeDelay;

	memcpy(&hDist->sc.acceptData, &sc.acceptData, sizeof(sc.acceptData));
	
	if(hDist->isExist == false)
	{
		hDist->isExist = true;
		hDist->isActive = true;
		hDist->Status = Status_Ready;
		if(hDist->InitThread(NextThreadID++) != 0)
		{
			LogMessageL(LOG_CRITICAL, "[CRITICAL] Failed to launch thread: %d", hDist->InternalIndex);
			hDist->inUse = false;
			hDist->isExist = false;
		}
		else
			LogMessageL(LOG_NORMAL, "[HTTPB] Thread launched: %d", hDist->InternalIndex);
	}
	else
	{
		//Thread already exists.
		hDist->inUse = true;
		hDist->Status = Status_Ready;
	}

	//Debug
	//if(hDist->isExist == true)
	//	g_Log.AddMessageFormat("Handing over to thread: %d, socket: %d", hDist->InternalIndex, hDist->sc.ClientSocket);

	
	sockaddr_in* sa = (sockaddr_in*) &sc.acceptData;
	LogMessageL(LOG_NORMAL, "[HTTP] [CONNECTION] ID:%d Address:%d.%d.%d.%d:%d",
		hDist->InternalIndex,
		(unsigned char)hDist->remotename.sa_data[2],
		(unsigned char)hDist->remotename.sa_data[3],
		(unsigned char)hDist->remotename.sa_data[4],
		(unsigned char)hDist->remotename.sa_data[5],
		sa->sin_port
		);

	/*
	int len = sizeof(sc.acceptData);
	int name = getsockname(sc.ClientSocket, &sc.acceptData, &len);
	LogMessageL(LOG_NORMAL, "Name: %d", name);
	*/

	return 0;
}
