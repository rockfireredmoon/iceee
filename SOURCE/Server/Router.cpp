#include "Router.h"

#include "Config.h"
#include "Globals.h"
#include "Simulator.h"

#include <stdlib.h>  //For itoa()
#include "Util.h"
#include "util/Log.h"

#include <boost/format.hpp>

RouterThread Router;

RouterThread::RouterThread() {

	memset(RecBuf, 0, sizeof(RecBuf));

	InternalIndex = 0;
	GlobalThreadID = 0;
	ThreadID = 0;

	RecBytes = 0;
	TotalRecBytes = 0;

	SendBytes = 0;
	TotalSendBytes = 0;

	isExist = false;
	isActive = false;
	Status = Status_Init;
	HomePort = 0;
	TargetPort = 0;
	mThread = NULL;

	sc.SetDebugName("ROUTER");
}

RouterThread::~RouterThread() {
	sc.ShutdownServer();
}

void RouterThread::SetHomePort(int port) {
	HomePort = port;
}

void RouterThread::SetBindAddress(const std::string &address) {
	BindAddress = address;
}

void RouterThread::SetTargetPort(int port) {
	TargetPort = port;
}

int RouterThread::InitThread(int instanceindex, int globalThreadID) {
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;
	mThread = new boost::thread( { &RouterThread::RunMain, this });
	return 0;
}

void RouterThread::OnConnect(const char *address) {
	//This is the router's job, resolve the target Simulator's address, send
	//the address string, then restart the thread.

	auto simAddress = g_Config.ResolveSimulatorAddress();
	auto simString = str(
			boost::format("%s:%d")
					% (simAddress.length() == 0 ? address : simAddress)
					% g_SimulatorPort);
	auto result = sc.AttemptSend(simString);
	if (result > 0)
		TotalSendBytes += simString.length();

	sc.DisconnectClient();
}

void RouterThread::Shutdown(void) {
	isActive = false;
	g_Logs.router->info("Waiting for router thread for port %v to stop.",
			HomePort);
	sc.ShutdownServer();
	mThread->join();
	delete mThread;
}

void RouterThread::RunMain(void) {
	SetNativeThreadName("Router");
	isExist = true;
	isActive = true;
	AdjustComponentCount(1);

	//The Router thread will loop within its own main function until it's shut down, or
	//a critical error is encountered.
	while (isActive == true) {
		if (Status == Status_Ready) {
			Status = Status_Kick;
		} else if (Status == Status_Init) {
			if (sc.CreateSocket(HomePort, BindAddress) == 0) {
				g_Logs.router->info(
						"Server created, awaiting connection on port %v (socket:%v)",
						HomePort, sc.ListenSocket);
				Status = Status_Wait;
			} else {
				//Keep trying, but wait a bit longer than normal.
				PLATFORM_SLEEP(g_ErrorSleep);
			}
		} else if (Status == Status_Wait) {
			int res = sc.Accept();
			if (res == 0) {
				OnConnect(sc.destAddr);
				//Job is done, get rid of the client.
				Status = Status_Kick;
			} else {
				if (sc.disconnecting) {
					g_Logs.router->info("Router shutdown.");
					Status = Status_None;
				} else {
					g_Logs.router->error("Socket error: %v",
							sc.GetErrorMessage());
					//This shouldn't normally fail.  Need a complete restart.
					Status = Status_Wait;
				}
			}
		}  else if (Status == Status_Kick) {
			sc.DisconnectClient();
			Status = Status_Wait;  //Wait for another connection.
		}
		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(SleepDelayNormal);
	}

	// Thread has been deactivated, shut it down
	sc.ShutdownServer();

	g_Logs.router->info("Thread shut down.");
	isExist = false;

	AdjustComponentCount(-1);
}

