#include "SimulatorBase.h"

#include "Config.h"
#include "Globals.h"

#include "Simulator.h"
#include <boost/format.hpp>
#include "Util.h"
#include "util/Log.h"

//PLATFORM_THREADRETURN SimulatorBaseThreadProc(PLATFORM_THREADARGS lpParam);
SimulatorBaseThread SimulatorBase;

SimulatorBaseThread :: SimulatorBaseThread()
{
	//Erases the receiving buffer and resets the byte counters.
	//Optionally if <fullRestart> is set to true, it performs a hard reset
	//of the thread.
	memset(RecBuf, 0, sizeof(RecBuf));

	MessageCountRec = 0;

	InternalIndex = 0;
	GlobalThreadID = 0;

	RecBytes = 0;
	TotalRecBytes = 0;

	SendBytes = 0;
	TotalSendBytes = 0;

	ThreadID = 0;
	isExist = false;
	isActive = false;
	Status = Status_Init;
	HomePort = 0;

	mThread = NULL;

	sc.SetDebugName("SIM_BASE");
}

SimulatorBaseThread :: ~SimulatorBaseThread()
{
	sc.ShutdownServer();
}

void SimulatorBaseThread :: SetHomePort(int port)
{
	HomePort = port;
}

void SimulatorBaseThread :: SetBindAddress(const std::string &address)
{
	BindAddress = address;
}


void SimulatorBaseThread :: InitThread(int instanceindex, int globalThreadID)
{
	//LogMessageL("[HTTP:%d] Address: %08X", instanceindex, this);
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;

	mThread = new boost::thread( { &SimulatorBaseThread::RunMain, this });
}

void SimulatorBaseThread :: OnConnect(void)
{
	return;
}

void SimulatorBaseThread :: Shutdown(void)
{
	isActive = false;
	sc.ShutdownServer();
	mThread->join();
	delete mThread;
}

void SimulatorBaseThread :: RunMain(void) {
	SetNativeThreadName("SimulatorBase");
	isActive = true;
	isExist = true;
	AdjustComponentCount(1);

	RunMainLoop();

	// Thread has been deactivated, shut it down
	sc.ShutdownServer();

	g_Logs.simulator->info("[SimB] Thread shut down.");

	isExist = false;

	AdjustComponentCount(-1);
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
			if(sc.CreateSocket(HomePort, BindAddress) == 0)
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
				//g_Scheduler.Submit(bind(&SimulatorBaseThread::LaunchSimulatorThread, this));
				g_Scheduler.Submit([this] {
						this->LaunchSimulatorThread();
						sc.ClientSocket = SocketClass::Invalid_Socket;
				});
//				LaunchSimulatorThread();
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
					Status = Status_Wait;
				}
			}
		}
		else if(Status == Status_Kick)
		{
			g_Logs.simulator->info("[SimB] Kicking client.");
			sc.DisconnectClient();
			Status = Status_Wait;  //Wait for another connection.
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
//	ThreadRequest threadReq;
//	threadReq.status = threadReq.STATUS_WAITMAIN;
//	g_SimulatorManager.RegisterAction(&threadReq);
//	if(threadReq.WaitForStatus(ThreadRequest::STATUS_WAITWORK, 1, ThreadRequest::DEFAULT_WAIT_TIME) == true)
//	{
//		SimulatorThread *simPtr = g_SimulatorManager.CreateSimulator();
//		if(simPtr != NULL)
//		{
//			simPtr->ResetValues(true);
//			simPtr->InternalID = g_SimulatorManager.nextSimulatorID++;
//			simPtr->sim_cs.Reset();
//			simPtr->sim_cs.Init();
//			simPtr->sim_cs.SetDebugName(str(boost::format("CS_SIM:%d") % simPtr->InternalID));
//			simPtr->sc.TransferClientSocketFrom(sc);
//			simPtr->sc.SetClientNoDelay();
//			simPtr->sc.SetTimeOut(5);
//			simPtr->LastUpdate = g_ServerTime;
//
//			simPtr->InitThread(g_GlobalThreadID++);
//			g_Logs.simulator->info("[SimB] Passing over to simulator ID:%v (socket:%v).", simPtr->InternalID, sc.ClientSocket);
//		}
//		else
//		{
//			g_Logs.simulator->fatal("[SimB] SimBase failed to create simulator.");
//		}
//	}
//	else
//	{
//		g_Logs.simulator->fatal("[SimB] SimBase failed to call a thread launch.");
//	}
//
//	threadReq.status = ThreadRequest::STATUS_COMPLETE;
//	g_SimulatorManager.UnregisterAction(&threadReq);

	SimulatorThread *simPtr = g_SimulatorManager.CreateSimulator();
	if (simPtr != NULL) {
		simPtr->ResetValues(true);
		simPtr->InternalID = g_SimulatorManager.nextSimulatorID++;
		simPtr->sim_cs.Reset();
		simPtr->sim_cs.Init();
		simPtr->sim_cs.SetDebugName(str(boost::format("CS_SIM:%d") % simPtr->InternalID));
		simPtr->sc.TransferClientSocketFrom(sc);
		simPtr->sc.SetClientNoDelay();
		simPtr->sc.SetTimeOut(5);
		simPtr->LastUpdate = g_ServerTime;

		simPtr->InitThread(g_GlobalThreadID++);
		g_Logs.simulator->info(
				"[SimB] Passing over to simulator ID:%v (socket:%v).",
				simPtr->InternalID, sc.ClientSocket);
	} else {
		g_Logs.simulator->fatal(
				"[SimB] SimBase failed to create simulator.");
	}

	return 0;
}
