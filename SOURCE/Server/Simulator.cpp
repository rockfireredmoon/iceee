#include <ctime>
#include <math.h>
#include <fstream>
#include <string>
#include <time.h>

#include "Simulator.h"
#include "StringList.h"
#include "Debug.h"
#include "Util.h"
#include "Config.h"
#include "Globals.h"
#include "PartyManager.h"
#include "Account.h"
#include "AIScript.h"
#include "AIScript2.h"
#include "Chat.h"
#include "Instance.h"
#include "Item.h"
#include "VirtualItem.h"
#include "FriendStatus.h"
#include "Ability2.h"
#include "DebugProfiler.h"
#include "ZoneDef.h"
#include "Interact.h"
#include "ChatChannel.h"
#include "IGForum.h"
#include <algorithm>  //for std::replace
#include "ZoneObject.h"
#include "Fun.h"
#include "Crafting.h"
#include "URL.h"
#include "InstanceScale.h"
#include "Combat.h"
#include "ConfigString.h"
#include "GM.h"
#include "QuestScript.h"
#include "CreditShop.h"


//This is the main function of the simulator thread.  A thread must be created for each port
//since the connecting function will halt until a connection is established.
PLATFORM_THREADRETURN SimulatorThreadProc(PLATFORM_THREADARGS lpParam);

struct StatFormat
{
	static const int TYPE_INT = 1;
	static const int TYPE_PINT = 2;
	static const int TYPE_STR = 3;

	int StatID;
	const char *formatStr;
	int Type;
};

void FormatStat(int statID, const char *valueStr, std::string &output)
{
	int index = GetStatIndex(statID);
	if(index == -1)
	{
		output = "<error:undefined>";
		return;
	}

	float fvalue = 0.0F;
	const char *formatStr = NULL;

	static const StatFormat formatArray[] = {
		{ STAT::MOD_MELEE_TO_CRIT, "+%g%% Physical Critical Chance", StatFormat::TYPE_PINT },
		{ STAT::MOD_MAGIC_TO_CRIT, "+%g%% Magic Critical Chance", StatFormat::TYPE_PINT },
		{ STAT::BASE_BLOCK, "+%g%% Block Chance", StatFormat::TYPE_PINT },
		{ STAT::BASE_PARRY, "+%g%% Parry Chance", StatFormat::TYPE_PINT },
		{ STAT::BASE_DODGE, "+%g%% Dodge Chance", StatFormat::TYPE_PINT },
		{ STAT::MOD_MOVEMENT, "+%g%% Movement Speed", StatFormat::TYPE_INT },
		{ STAT::EXPERIENCE_GAIN_RATE, "+%g%% Experience Gain", StatFormat::TYPE_INT },
		{ STAT::MELEE_ATTACK_SPEED, "+%g%% Increased Attack Speed", StatFormat::TYPE_PINT },
		{ STAT::MAGIC_ATTACK_SPEED, "+%g%% Increased Cast Rate", StatFormat::TYPE_PINT },
		{ STAT::DMG_MOD_FIRE, "+%g%% Fire Specialization", StatFormat::TYPE_PINT },
		{ STAT::DMG_MOD_FROST, "+%g%% Frost Specialization", StatFormat::TYPE_PINT },
		{ STAT::DMG_MOD_MYSTIC, "+%g%% Mystic Specialization", StatFormat::TYPE_PINT },
		{ STAT::DMG_MOD_DEATH, "+%g%% Death Specialization", StatFormat::TYPE_PINT },
		{ STAT::BASE_HEALING, "+%g%% Healing Specialization", StatFormat::TYPE_PINT },
		{ STAT::CASTING_SETBACK_CHANCE, "%g%% Casting Setback Chance", StatFormat::TYPE_PINT },
		{ STAT::CHANNELING_BREAK_CHANCE, "%g%% Channel Break Chance", StatFormat::TYPE_PINT },
		{ STAT::MOD_HEALTH_REGEN, "+%g Hitpoint Regeneration", StatFormat::TYPE_INT }
	};
	static const int formatArraySize = sizeof(formatArray) / sizeof(formatArray[0]);
	for(int i = 0; i < formatArraySize; i++)
	{
		if(statID != formatArray[i].StatID)
			continue;
		fvalue = 0.0F;
		if(formatArray[i].Type != StatFormat::TYPE_STR)
			fvalue = static_cast<float>(atof(valueStr));
		if(formatArray[i].Type == StatFormat::TYPE_PINT)
			fvalue /= 10.0F;

		char buffer[64];
		if(formatArray[i].Type == StatFormat::TYPE_STR)
			Util::SafeFormat(buffer, sizeof(buffer), formatArray[i].formatStr, valueStr);
		else
			Util::SafeFormat(buffer, sizeof(buffer), formatArray[i].formatStr, fvalue);
		
		output = buffer;
		return;
	}
}


OnExitFunctionClearBuffers::OnExitFunctionClearBuffers(SimulatorThread *caller)
{
	mCaller = caller;
}
OnExitFunctionClearBuffers::~OnExitFunctionClearBuffers()
{
	if(mCaller)
	{
		mCaller->ClearAuxBuffers();
		mCaller = NULL;
	}
}

#ifndef WINDOWS_PLATFORM
#include <errno.h>
#endif

const char EXIT_GUILD_HALL[] = "EXIT GUILD HALL";
const char EXIT_GROVE[] = "EXIT GROVE";
const char EXIT_PVP[] = "EXIT PVP";

std::list<SimulatorThread> Simulator;

SimulatorManager g_SimulatorManager;

const int MapTickChange = 20;

const int UNREFASHION_LOWERBOUND = 8980;  //These items remove a refashioned effect.
const int UNREFASHION_UPPERBOUND = 8989;

extern char GAuxBuf[1024];
extern char GSendBuf[32767];

SimulatorThread * GetSimulatorByID(int ID)
{
	//In the old system, it uses an index into the hardcoded Simulator array.
	//In the new system, iterate across the simulator list and search for the unique ID.
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		if(it->InternalID == ID)
			return &*it;
	return NULL;
}

SimulatorThread * GetSimulatorByCharacterName(const char *name)
{
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		if(it->ProtocolState == 1 && it->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY)
			if(it->creatureInst != NULL)
				if(strcmp(it->creatureInst->css.display_name, name) == 0)
					return &*it;
	return NULL;
}


ThreadRequest :: ThreadRequest()
{
	status = STATUS_NONE;
}

ThreadRequest :: ~ThreadRequest()
{
}

bool ThreadRequest :: WaitForStatus(int statusID, int checkInterval, int maxError)
{
	int errCount = 0;
	while(status != statusID)
	{
		errCount++;
		if(errCount == maxError)
			return false;

		PLATFORM_SLEEP(checkInterval);
	}
	return true;
}

SimulatorManager :: SimulatorManager()
{
	debug_acquired = false;

	pendingActions = 0;
	ActiveCount = 0;
	nextSimulatorID = 0;

	baseByteSent = 0;
	baseByteRec = 0;
	cs.SetDebugName("CS_SIMMGR");
	cs.notifyWait = false;
	cs.Init();
	nextFlushTime = 0;
}

SimulatorManager :: ~SimulatorManager()
{
	Free();
}

void SimulatorManager :: Free(void)
{
	regList.clear();
	pendingActions = 0;
}

//void SimulatorManager :: RegisterAction(SimulatorThread *simData)
void SimulatorManager :: RegisterAction(ThreadRequest *reqData)
{
	cs.Enter("SimulatorManager::RegisterAction");
	regList.push_back(reqData);
	pendingActions++;
	cs.Leave();
}

void SimulatorManager :: UnregisterAction(ThreadRequest *reqData)
{
	cs.Enter("SimulatorManager::UnregisterAction");
	//while(DeleteAction(reqData) == true);
	size_t pos = 0;
	while(pos < regList.size())
	{
		if(regList[pos] == reqData)
			regList.erase(regList.begin() + pos);
		else
			pos++;
	}
	cs.Leave();
}

void SimulatorManager :: RunPendingActions(void)
{
	ProcessPendingDisconnects();
	ProcessPendingPacketData();
	ProcessPendingActions();
}

SimulatorThread* SimulatorManager :: GetPtrByID(int simID)
{
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		if(it->InternalID == simID)
			return &*it;
	return NULL;
}

void SimulatorManager::AddPendingDisconnect(SimulatorThread *callObject)
{
	cs.Enter("SimulatorManager::AddPendingDisconnect");
	pendingDisconnects.push_back(callObject);
	cs.Leave();
}

void SimulatorManager::AddPendingPacketData(SimulatorThread *callObject)
{
	cs.Enter("SimulatorManager::AddPendingPacketData");
	for(size_t i = 0; i < pendingPacketData.size(); i++)
	{
		if(pendingPacketData[i] == callObject)
		{
			cs.Leave();
			return;
		}
	}
	pendingPacketData.push_back(callObject);
	cs.Leave();
}

void SimulatorManager::ProcessPendingDisconnects(void)
{
	if(pendingDisconnects.size() == 0)
		return;

	cs.Enter("SimulatorManager::AddPendingDisconnect");
	for(size_t i = 0; i < pendingDisconnects.size(); i++)
	{
		g_Log.AddMessageFormat("ProcessPendingDisconnects - Sim:%d", pendingDisconnects[i]->InternalID);
		pendingDisconnects[i]->ProcessDisconnect();
	}

	pendingDisconnects.clear();

	cs.Leave();
}

void SimulatorManager::ProcessPendingPacketData(void)
{
	if(pendingPacketData.size() == 0)
		return;

	cs.Enter("SimulatorManager::ProcessPendingPackets");
	for(size_t i = 0; i < pendingPacketData.size(); i++)
		pendingPacketData[i]->HandleReceivedMessage2();

	pendingPacketData.clear();
	cs.Leave();
}

void SimulatorManager::ProcessPendingActions(void)
{
	if(pendingActions == 0)
		return;

#ifdef DEBUG_TIME
	Debug::TimeTrack("ProcessPendingActions", 100);
#endif

	cs.Enter("SimulatorManager::RunPendingActions");
	debug_acquired = true;

	for(size_t i = 0; i < regList.size(); i++)
	{
		//if(regList[i]->threadsys.status != ThreadRequest::STATUS_WAITMAIN)
		if(regList[i]->status != ThreadRequest::STATUS_WAITMAIN)
			continue;

		//regList[i]->threadsys.status = ThreadRequest::STATUS_WAITWORK;
		regList[i]->status = ThreadRequest::STATUS_WAITWORK;
		//bool res = regList[i]->threadsys.WaitForStatus(ThreadRequest::STATUS_COMPLETE, 5, 1000);
		bool res = regList[i]->WaitForStatus(ThreadRequest::STATUS_COMPLETE, 1, ThreadRequest::DEFAULT_WAIT_TIME);
		if(res == false)
			g_Log.AddMessageFormatW(MSG_WARN, "[WARNING] RunPendingActions() timed out while waiting for worker thread to complete."); 
	}

	regList.clear();
	pendingActions = 0;

	debug_acquired = false;
	cs.Leave();
}


// Scan for a simulator that is logged in to a particular account and
// remove if the simulator has been inactive for a certain duration.
void SimulatorManager :: CheckIdleSimulatorBoot(AccountData *account)
{
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->pld.accPtr != account)
			continue;
		if(it->LoadStage >= SimulatorThread::LOADSTAGE_LOADING && (g_ServerTime > it->LastRecv + SIMULATOR_BOOT_INACTIVE))
		{
			it->ForceErrorMessage("Your connection is being kicked because there is another login on your account.", INFOMSG_INFO);
			g_Log.AddMessageFormat("[KICK] CheckIdleSimulatorBoot Forcing Simulator:%d to shut down", it->InternalID);
			it->Disconnect("SimulatorManager::CheckIdleSimulatorBoot");
			return;
		}
	}
}

void SimulatorManager :: FlushHangingSimulators(void)
{
	if(g_ServerTime < nextFlushTime)
		return;

	nextFlushTime = g_ServerTime + SIMULATOR_FLUSH_INTERVAL;

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == false)
			continue;

		if(it->ProtocolState == 0 && it->LoadStage == 0)
		{
			long testTime = SIMULATOR_FLUSH_INACTIVE;
			if(it->characterCreation == true)
				testTime = SIMULATOR_FLUSH_CHARCREATE;
			if(g_ServerTime > it->LastRecv + testTime)
			{
				it->ForceErrorMessage("Closing connection.", INFOMSG_INFO);
				g_Log.AddMessageFormat("Forcing Simulator:%d to shut down", it->InternalID);
				it->Disconnect("SimulatorManager::FlushHangingSimulators");
			}
		}
		else if(it->ProtocolState == 1 && it->LoadStage > 0)
		{
			//While we're here, might as well check for idle characters and boot them.
			if(g_ServerTime >= it->pld.NextIdleCheckTime)
			{
				if(it->pld.VerifyIdle() == true)
				{
					g_Log.AddMessageFormat("[BOT] Forcing Simulator:%d to shut down (detected idle)", it->InternalID);
					it->ForceErrorMessage("You have been disconnected for inactivity.", INFOMSG_INFO);
					it->Disconnect("SimulatorManager::FlushHangingSimulators");
					continue;
				}
				it->pld.UpdateNextIdleCheckTime();
			}

			if(g_Config.HeartbeatAbortCount == -1)
				continue;
			if(it->PendingHeartbeatResponse < g_Config.HeartbeatAbortCount)
				continue;
			long testTime = g_Config.HeartbeatIntervalMS * g_Config.HeartbeatAbortCount;
			if(g_ServerTime > it->LastRecv + testTime)
			{
				it->ForceErrorMessage("The server has not received a routine message from the client. Disconnecting.", INFOMSG_INFO);
				g_Log.AddMessageFormat("Forcing Simulator:%d to shut down (no heartbeat response)", it->InternalID);
				it->Disconnect("SimulatorManager::FlushHangingSimulators");
			}
		}
	}
}

void SimulatorManager :: BroadcastMessage(const char *message)
{
	g_Log.AddMessageFormat("Broadcast message '%s'", message);
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->ProtocolState == 0)
		{
			continue;
		}

		if(it->isConnected == false)
			continue;

		if(it->pld.charPtr == NULL)
			continue;

		it->BroadcastMessage(message);
	}
}

SimulatorThread * SimulatorManager :: CreateSimulator(void)
{
	SimulatorThread *simPtr = NULL;

	SimulatorThread newSim;
	Simulator.push_back(newSim);
	simPtr = &Simulator.back();
	return simPtr;
}

SimulatorQuery :: SimulatorQuery()
{
	Clear();
}

SimulatorQuery :: ~SimulatorQuery()
{
	Clear();
}

void SimulatorQuery :: Clear(void)
{
	ID = 0;
	name.clear();
	args.clear();
	argCount = 0;
};

bool SimulatorQuery :: ValidArgIndex(int argIndex)
{
	if(argIndex < 0 || argIndex >= argCount)
	{
		g_Log.AddMessageFormat("[WARNING] Invalid index: %d for query: %s", argIndex, name.c_str());
		return false;
	}
	return true;
}

const char* SimulatorQuery :: GetString(int argIndex)
{
	return ValidArgIndex(argIndex) ? args[argIndex].c_str() : NULL;
}

int SimulatorQuery :: GetInteger(int argIndex)
{
	return ValidArgIndex(argIndex) ? atoi(args[argIndex].c_str()) : 0;
}

float SimulatorQuery :: GetFloat(int argIndex)
{
	return ValidArgIndex(argIndex) ? static_cast<float>(atof(args[argIndex].c_str())) : 0.0F;
}

bool SimulatorQuery :: GetBool(int argIndex)
{
	return ValidArgIndex(argIndex) ? (atoi(args[argIndex].c_str()) != 0) : false;
}

SimulatorThread :: SimulatorThread()
{
	//ResetValues(true);
	Debug_TestCondition = 0;

	//sim_cs.Init();
	ThreadID = 0;
	isThreadExist = false;
	isThreadActive = false;
	Status = 0;
	isConnected = false;
	firstConnect = true;

	memset(LogBuffer, 0, sizeof(LogBuffer));
	InternalIndex = 0;
	InternalID = 0;
	GlobalThreadID = 0;

	sc.Clear();
	memset(RecBuf, 0, sizeof(RecBuf));
	readPtr = NULL;
	//recvData.Clear();  Don't need to call these, but leaving for reference
	//procData.Clear();
	memset(SendBuf, 0, sizeof(SendBuf));
	SocketStatus = 0;
	TotalMessageSent = 0;
	TotalMessageReceived = 0;
	TotalSendBytes = 0;
	TotalRecBytes = 0;
	MessageEnd = 0;
	ReadPos = 0;
	WritePos = 0;
	PendingSend = false;
	memset(Aux1, 0, sizeof(Aux1));
	memset(Aux2, 0, sizeof(Aux2));
	memset(Aux3, 0, sizeof(Aux3));
	memset(clientPeerName, 0, sizeof(clientPeerName));
	LastChatTime = 0;
	chatMessageFrequency = 0;

	ProtocolState = 0;
	ClientLoading = false;
	LoadStage = 0;
	LastUpdate = 0;
	LastRecv = 0;
	characterCreation = false;

	PendingHeartbeatResponse = 0;

	pld.Reset();
	query.Clear();
	creatureInst = &defcInst;
	defcInst.Clear();
	TimeOnline = 0;
	TimeLastAutoSave = 0;
	creatureTweakModifier = NULL;

	threadsys.status = 0;
	
	questScript.Clear();
}

SimulatorThread :: ~SimulatorThread()
{
	//sim_cs.Free();
}

void SimulatorThread :: ResetValues(bool hardReset)
{
	Debug_TestCondition = 0;
	readPtr = &RecBuf[0];

	memset(LogBuffer, 0, sizeof(LogBuffer));
	memset(RecBuf, 0, sizeof(RecBuf));
	recvData.Clear();
	procData.Clear();
	memset(SendBuf, 0, sizeof(SendBuf));
	memset(Aux1, 0, sizeof(Aux1));
	memset(Aux2, 0, sizeof(Aux2));
	memset(Aux3, 0, sizeof(Aux3));

	if(hardReset == true)
	{
		isThreadActive = false;
		TotalSendBytes = 0;
		TotalRecBytes = 0;
	}

	SocketStatus = 0;

	ProtocolState = 0;
	ClientLoading = false;
	LoadStage = LOADSTAGE_UNLOADED;
	Status = Status_Wait;
	isConnected = false;
	LastUpdate = 0;
	firstConnect = true;
	
	MessageEnd = 0;
	ReadPos = 0;
	WritePos = 0;
	PendingSend = false;

	defcInst.Clear();
	creatureInst = &defcInst;

	TimeOnline = 0;
	TimeLastAutoSave = 0;

	pld.Reset();

	PendingHeartbeatResponse = 0;
}

int SimulatorThread :: InitThread(int instanceindex, int globalThreadID)
{
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;
	
	int res = Platform_CreateThread(0, (void*)SimulatorThreadProc, this, &ThreadID);
	if(res == 0)
		LogMessageL(MSG_ERROR, "Could not create thread.");

	return res;
}

//This thread exists outside the class, but a new instance is created when a new
//thread is launched.  It takes a parameter of the address back to its simulator class
//so it can call the necessary functions.
//DWORD WINAPI SimulatorThreadProc(LPVOID lpParam)
PLATFORM_THREADRETURN SimulatorThreadProc(PLATFORM_THREADARGS lpParam)
{
	SimulatorThread *controller = (SimulatorThread*)lpParam;
	if(controller->sim_cs.initialized == false)
	{
		g_Log.AddMessageFormat("[CRITICAL] Simulator critical section is not initialized.");
		return 0;
	}
	controller->LastUpdate = g_ServerTime;
	controller->isThreadActive = true;
	controller->isThreadExist = true;
	controller->Status = Status_Init;
	
	AdjustComponentCount(1);

	//The Simulator will loop within its own main function until it's shut down, or
	//a critical error is encountered.
	controller->RunMainLoop();

	//controller->sc.ShutdownServer();
	//controller->Disconnect("SimulatorThreadProc");
	//controller->Connected = false;

	controller->isThreadExist = false;
	controller->LastUpdate = g_ServerTime;
	AdjustComponentCount(-1);
	controller->AddPendingDisconnect();
	g_Log.AddMessageFormat("Thread for Sim:%d shut down.", controller->InternalID);
	PLATFORM_CLOSETHREAD(0);
	return 0;
}

void SimulatorThread :: RunMainLoop(void)
{
	cs.Init();
	while(isThreadActive == true)
	{
		BEGINTRY
		{
			//The server should always be in the ready state.
		if(Status == Status_Ready)
		{
			if(SocketStatus != 0)
			{
				LogMessageL(MSG_ERROR, "[ERROR] Socket is invalidated.");
				isThreadActive = false;
				//sc.DisconnectClient();
				Disconnect("RunMainLoop");
				Status = Status_Wait;
			}
			else
			{
				/* DEBUGGING WITH SOME SIMULATED LAG
				Sleep(1);
				static unsigned long nextTrigger = 0;
				while(g_ServerTime < nextTrigger)
				{
					Sleep(100);
				}
				nextTrigger = g_ServerTime + 2000;
				*/

				int res = sc.tryrecv(RecBuf, sizeof(RecBuf));
				if(res > 0)
				{
					cs.Enter("RunMainLoop");
					recvData.Append(RecBuf, res);
					if(recvData.mData.size() > sizeof(RecBuf))
						LogMessageL(MSG_WARN, "[WARNING] Pending buffer has %d bytes", recvData.mData.size());
					else if(recvData.mData.size() > 32000)
						LogMessageL(MSG_WARN, "[ERROR] Pending buffer is accumulating %d bytes", recvData.mData.size());
					else if(recvData.mData.size() > 64000)
					{
						LogMessageL(MSG_WARN, "[ERROR] Pending buffer too full %d bytes", recvData.mData.size());
						Disconnect("RunMainLoop");
					}
					cs.Leave();
					g_SimulatorManager.AddPendingPacketData(this);

					//Adding it like this allows it to handle additional data if a
					//large message has been broken into pieces.
					TotalRecBytes += res;

					//Update the message end point.
					RecBuf[res] = 0;

					//HandleReceivedMessage2();  //OBSOLETE, routing through the main thread from AddPendingPacketData()
				}
				else if(res == 0)
				{
					LogMessageL(MSG_DIAG, "Connection closed message.");
					Status = Status_Restart;
				}
				else if(res == -2)
				{
					LogMessageL(MSG_ERROR, "[ERROR] Socket select error.");
					Status = Status_Restart;
				}
				else
				{
					LogMessageL(MSG_ERROR, "sim recv failed: %s", sc.GetErrorMessage());
					Status = Status_Restart;
				}
			}
		}
		else if(Status == Status_Wait)
		{
			//Wait for the simulator base to pass an incoming connection.
			PLATFORM_SLEEP(1000);
		}
		else if(Status == Status_Init)
		{
			//If this has been set to the Init phase, it means a connection
			//has been supplied by the simulator base.
			OnConnect();
			Status = Status_Ready;
		}
		else if(Status == Status_Restart)
		{
			Disconnect("RunMainLoop");
			Status = Status_Wait;
		}
		else
		{
			LogMessageL(MSG_WARN, "Unknown status.");
			Status = Status_Wait;
			PLATFORM_SLEEP(SleepDelayError);
		}

		//Keep it from burning up unnecessary CPU cycles.
		PLATFORM_SLEEP(g_ThreadSleep);

		} //End try
		//__except((filter(GetExceptionCode(), GetExceptionInformation())))
		BEGINCATCH
		{
			g_Log.AddMessageFormat("[ERROR] Exception occurred in [Sim:%d]", InternalIndex);
			ForceErrorMessage("CRITICAL ERROR: EMERGENCY DISCONNECT", INFOMSG_ERROR);
			Disconnect("RunMainLoop");
		}
	}
}


void SimulatorThread :: LogMessageL(unsigned short messageType, const char *format, ...)
{
	if(g_Log.LoggingEnabled == false)
		return;

	if(g_SimulatorLog == false)
		return;

	if(!(g_Log.Filter & messageType))
		return;

	sprintf(LogBuffer, "[Sim:%d] ", InternalIndex);
	int pos = strlen(LogBuffer);

	va_list args;
	va_start (args, format);
	Util::SafeFormatArg(&LogBuffer[pos], sizeof(LogBuffer) - pos, format, args);
	va_end (args);

	g_Log.AddMessage(LogBuffer);
}

//Since this function was previously called from both the Simulator and Main threads, this is now
//a global event.  When the main thread processes it, it calls ProcessDisconnect().
void SimulatorThread :: Disconnect(const char *debugCaller)
{
	PLATFORM_SLEEP(1);
	isThreadActive = false;
	sc.DisconnectClient();

	/*
	//sc.DisconnectClient();
	LogMessageL(MSG_SHOW, "Disconnecting: [%s]", debugCaller);
	g_SimulatorManager.AddPendingDisconnect(this);
	*/
	
}

void SimulatorThread :: AddPendingDisconnect(void)
{
	g_SimulatorManager.AddPendingDisconnect(this);
}

void SimulatorThread :: ProcessDisconnect(void)
{
	if(sim_cs.GetLockCount() > 0)
		LogMessageL(MSG_WARN, "[DEBUG] Disconnect() LockCount is %d", sim_cs.GetLockCount());

	//sim_cs.Enter();
	LogMessageL(MSG_SHOW, "Disconnecting client");

	sc.ShutdownServer();

	if(pld.charPtr != NULL)
	{
		if(LoadStage == LOADSTAGE_GAMEPLAY)
		{
			g_PartyManager.CheckMemberLogin(creatureInst);
			if(CheckPermissionSimple(Perm_Account, Permission_Invisible) == false)
			{

				sprintf(Aux1, "%s has disconnected.", pld.charPtr->cdef.css.display_name);
				LogChatMessage(Aux1);
				WritePos = PrepExt_SendInfoMessage(SendBuf, Aux1, INFOMSG_INFO);
				SendToAllSimulator(SendBuf, WritePos, InternalID);

				WritePos = PrepExt_FriendsLogStatus(SendBuf, pld.charPtr, 0);
				SendToFriendSimulator(SendBuf, WritePos, pld.charPtr->cdef.CreatureDefID);
			}

			WritePos = PrepExt_RemoveCreature(SendBuf, pld.CreatureID);
			creatureInst->actInst->LSendToAllSimulator(SendBuf, WritePos, InternalID);
		}
		UpdateSocialEntry(false, false);  //Place here since the character may not be fully logged in if they quit.
		SetLoadingStatus(false, true);

		SaveCharacterStats();
		SetAccountCharacterCache();
		pld.charPtr->SetExpireTime();
		pld.charPtr->pendingChanges = 1;
		g_CharacterManager.SaveCharacter(pld.CreatureDefID);
		g_ChatChannelManager.LeaveChannel(InternalID, NULL);  //Remove the simulator from any chat channels so they don't pollute the active member list.

		pld.charPtr = NULL;  //Can't believe I forgot this for so long... Disconnect was often called twice, too.
	}
	g_PartyManager.RemovePlayerReferences(pld.CreatureDefID, true);
	MainCallHelperInstanceUnregister();

	if(pld.accPtr != NULL)
	{
		pld.accPtr->AdjustSessionLoginCount(-1);
		pld.accPtr = NULL;
		//g_AccountManager.UnloadAccount(pld.accPtr->ID);
	}

	if(creatureInst->actInst != NULL)
		LogMessageL(MSG_ERROR, "[ERROR] Disconnect() actInst still registered when resetting values");

	//Soft reset, don't adjust threading state or total bandwidth counts.
	//ResetValues(false);
	
	LogMessageL(MSG_SHOW, "Finished disconnecting.");
	//sim_cs.Leave();

	if(procData.mData.size() > 0)
	{
		LogMessageL(MSG_ERROR, "[DISCONNECT] Clearing pending data: %d (receive: %d)", procData.mData.size(), recvData.mData.size());
		procData.Clear();
	}

	isConnected = false;

	//Allow the thread's main loop to terminate and shut down the thread.
	isThreadActive = false;
}

void SimulatorThread :: ForceErrorMessage(const char *message, int msgtype)
{
	int size = 0;
	if(g_Config.UseMessageBox == false)
	{
		size = PrepExt_SendInfoMessage(SendBuf, message, msgtype);
	}
	else
	{
		size += PutByte(&SendBuf[size], 100);   //_handleModMessage   REQUIRES MODDED CLIENT (AddOns.nut)
		size += PutShort(&SendBuf[size], 0);    //Reserve for size

		size += PutByte(&SendBuf[size], MODMESSAGE_EVENT_POPUP_MSG);   //event for advanced emote
		size += PutStringUTF(&SendBuf[size], message);

		PutShort(&SendBuf[1], size - 3);       //Set message size
	}
	g_PacketManager.GetThread("SimulatorThread::ForceErrorMessage");
	sc.AttemptSendNoBlock(SendBuf, size);
	g_PacketManager.ReleaseThread();
}

int SimulatorThread :: AttemptSend(const char *buffer, int buflen)
{
#ifdef DEBUG_TIME
	Debug::TimeTrack("SimulatorThread::AttemptSend");
#endif

	if(buflen >= sizeof(SendBuf))
		g_Log.AddMessageFormat("[CRITICAL] AttemptSend() bytecount: %d", buflen);
	VerifyGenericBuffer(buffer, buflen);

	Packet sendData;
	sendData.Assign(buffer, buflen);

	g_PacketManager.GetThread("SimulatorThread::AttemptSend");
	g_PacketManager.AddOutgoingPacket2(sc.ClientSocket, sendData);
	g_PacketManager.ReleaseThread();

	TotalSendBytes += buflen;
	TotalMessageSent++;
	return 0;
}

void SimulatorThread :: OnConnect(void)
{
	sockaddr remotename;
	memset(&remotename, 0, sizeof(remotename));
	socklen_t remotesize = sizeof(remotename);
	getpeername(sc.ClientSocket, &remotename, &remotesize);

	Util::SafeFormat(clientPeerName, sizeof(clientPeerName), "%d.%d.%d.%d", 
		(unsigned char)remotename.sa_data[2],
		(unsigned char)remotename.sa_data[3],
		(unsigned char)remotename.sa_data[4],
		(unsigned char)remotename.sa_data[5]
	);
	int dpos = 0;
	int cpos = 0;
	for(int i = 0; i < sizeof(remotename.sa_data); i++)
	{
		dpos += sprintf(&SendBuf[dpos], "%d.", (unsigned char)remotename.sa_data[i]);
		int c = remotename.sa_data[i];
		if(c <= ' ')
			c = 'x';
		cpos += sprintf(&Aux1[cpos], "%c.", c);
	}
	LogMessageL(MSG_SHOW, "[CONNECT] num [%s]", SendBuf);
	LogMessageL(MSG_SHOW, "[CONNECT] char [%s]", Aux1);
	LogMessageL(MSG_SHOW, "[CONNECT] Connecting Socket:%d", sc.ClientSocket);

	//int len = HexToByte(g_SimulatorOnConnect, SendBuf, sizeof(SendBuf));
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 40);  //_handleAuthenticationRequestMsg
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutInteger(&SendBuf[wpos], g_ProtocolVersion);
	wpos += PutInteger(&SendBuf[wpos], g_AuthMode);
	wpos += PutStringUTF(&SendBuf[wpos], g_AuthKey);
	PutShort(&SendBuf[1], wpos - 3);

	AttemptSend(SendBuf, wpos);
	isConnected = true;

	//Update the last communication to the connection time, for good measure
	LastRecv = g_ServerTime;
}

// Return true if the client is connected and capable of receiving gameplay-related messages.
bool SimulatorThread :: CheckStateGameplayProtocol(void) const
{
	if(isConnected == false)
		return false;
	if(ProtocolState == 0)
		return false;
	return true;
}

// Return true if the client is connected and capable of receiving gameplay-related messages.
// Also check to see if the player is likely capable of interacting with the game
// (client is fully loaded).
bool SimulatorThread :: CheckStateGameplayActive(void) const
{
	if(isConnected == false)
		return false;
	if(ProtocolState == 0)
		return false;
	if(LoadStage != LOADSTAGE_GAMEPLAY)
		return false;
	return true;
}

void SimulatorThread :: HandleReceivedMessage2(void)
{
	Debug::LastSimulatorID = InternalID;

	//Grab the data from the simulator's pending buffer
	cs.Enter("HandleReceivedMessage2");
	procData.Append(recvData);
	recvData.Clear();
	cs.Leave();

	if(isConnected == false)
	{
		g_Log.AddMessageFormat("[CRITICAL] Should not be processing data (size: %d)", procData.mData.size());
		return;
		procData.Clear();
	}

	if(procData.mData.size() == 0)
	{
		LogMessageL(MSG_SHOW, "[WARNING] HandleReceivedMessage2 procData size is zero");
		return;
	}

	LastRecv = g_ServerTime;
	PendingHeartbeatResponse = 0;
	TotalMessageReceived++;

	int remain = procData.mData.size();
	if(remain < 3)
	{
		LogMessageL(MSG_ERROR, "HandleReceivedMessage2() invalid message size: %d", remain);
		Disconnect("SimulatorThread::HandleReceivedMessage2");
		return;
	}

	ReadPos = 0;
	//int debugCount = 0;
	readPtr = procData.mData.data();
	while(remain > 0)
	{
#ifdef DEBUG_PROFILER
		unsigned long debugTimeStart = g_ServerTime;
		Debug::TimeTrack("HandleReceivedMessage2", 100);
#endif
		int startRead = ReadPos;
		unsigned char msgType = GetByte(&readPtr[ReadPos], ReadPos);
		unsigned short msgSize = GetShort(&readPtr[ReadPos], ReadPos);
		if(firstConnect == true)
		{
			if(msgType != 1)
			{
				LogMessageL(MSG_ERROR, "HandleReceivedMessage() Invalid first request, disconnecting (type: %d, size: %d)", msgType, msgSize);
				Disconnect("SimulatorThread::HandleReceivedMessage2");
				return;
			}
			firstConnect = false;
		}
		if(msgSize > remain - 3)
		{
			LogMessageL(MSG_ERROR, "HandleReceivedMessage() expected more data, has: %d, needs: %d", remain - 3, msgSize);
			break;
		}

		

		if(ProtocolState == PROTOCOLSTATE_GAME)
			HandleGameMsg(msgType);
		else
			HandleLobbyMsg(msgType);

		if(PendingSend == true)
		{
			if(WritePos != 0)
			{
				AttemptSend(SendBuf, WritePos);
				WritePos = 0;
			}
			else
				LogMessageL(MSG_WARN, "PendingSend marked for zero bytes.");

			PendingSend = false;
		}
		remain -= (msgSize + 3);
		
		//Safeguard in case not all bytes are handled.
		ReadPos = startRead + msgSize + 3;
		//LogMessageL(MSG_SHOW, "Message: %d, type: %d, size: %d, remain: %d", debugCount, msgType, msgSize, remain);
		//debugCount++;

#ifdef DEBUG_PROFILER
		unsigned long debugTimePassed = g_ServerTime - debugTimeStart;
		if(ProtocolState == PROTOCOLSTATE_GAME)
			_DebugProfiler.IncrementGameMsg(msgType, debugTimePassed);
		else
			_DebugProfiler.IncrementLobbyMsg(msgType, debugTimePassed);
#endif
	}
	//if(debugCount > 1)
	//	LogMessageL(MSG_SHOW, "[DEBUG] DEBUG COUNT: %d", debugCount);


	if(remain == 0)
		procData.Clear();
	else
	{
		int offset = procData.mData.size() - remain;
		procData.TrimFront(offset);
	}
}

void SimulatorThread :: HandleGameMsg(int msgType)
{
#ifdef DEBUG_TIME
	Debug::TimeTrack("HandleGameMsg", 50);
#endif
	//LogMessageL(MSG_SHOW, "[DEBUG] HandleGameMsg:%d", msgType);

	switch(msgType)
	{
	case 0: handle_inspectCreatureDef(); break;
	case 1: handle_updateVelocity(); break;
	case 2: handle_game_query(); break;
	case 3: handle_selectTarget(); break;
	case 4: handle_communicate(); break;
	case 5: handle_inspectCreature(); break;
	case 6: handle_abilityActivate(); break;
	case 9: handle_inspectItemDef(); break;
	case 10: break;  //This message is sent when the player transitions between swim/non-swim states in the client.  No effect on the server side so just ignore it so it doesn't generate error messages.
	case 11: handle_disconnect(); break;

	case 19: handle_debugServerPing(); break;       //Sent by a modded client only.
	case 20: handle_acknowledgeHeartbeat(); break;  //Sent by a modded client only.
	default:
		LogMessageL(MSG_ERROR, "Unhandled message in Game mode: %d", msgType);
	}
}

void SimulatorThread :: HandleLobbyMsg(int msgType)
{
	switch(msgType)
	{
	case 1: handle_lobby_authenticate(); break;
	case 2: handle_lobby_selectPersona(); break;
	case 3: handle_lobby_query(); break;
	case 4: handle_inspectItemDef(); break;
	case 20: handle_acknowledgeHeartbeat(); break;  //Sent by a modded client only.
	default:
		LogMessageL(MSG_ERROR, "Unhandled message in Lobby mode: %d", msgType);
	}
}

void SimulatorThread :: handle_lobby_authenticate(void)
{
	char authMethod = GetByte(&readPtr[ReadPos], ReadPos);
	GetStringUTF(&readPtr[ReadPos], Aux2, sizeof(Aux2), ReadPos);  //login name
	GetStringUTF(&readPtr[ReadPos], Aux3, sizeof(Aux3), ReadPos);  //authorization hash

	//We want to clear the the buffers when we leave scope, so that the authorization stuff
	//doesn't stick around in memory.  If the online web panel were compromised, the token could
	//potentially be stolen when viewing the simulator data report.
	OnExitFunctionClearBuffers fnCleanup(this);
	
	if(g_AccountManager.AcceptingLogins() == false)
	{
		ForceErrorMessage("Not accepting more logins.", INFOMSG_ERROR);
		Disconnect("SimulatorThread::handle_lobby_authenticate");
		return;
	}

	//Convert client password to server password.
	std::string password;
	AccountData::GenerateSaltedHash(Aux3, password);

	AccountData *accPtr = NULL;
	if(authMethod == AuthMethod::DEV)
	{
		g_AccountManager.cs.Enter("SimulatorThread::handle_lobby_authenticate");
		accPtr = g_AccountManager.GetValidLogin(Aux2, password.c_str());
		g_AccountManager.cs.Leave();
	}
	else
	{
		//AuthMethod::EXTERNAL routes through an external service for authentication.
		//Not supported here.
	}

	if(accPtr == NULL)
	{
		LogMessageL(MSG_ERROR, "Could not find account: %s:%s", Aux2, password.c_str());
		ForceErrorMessage(g_Config.InvalidLoginMessage.c_str(), INFOMSG_ERROR);
		Disconnect("SimulatorThread::handle_lobby_authenticate");
		return;
	}

	//Check for ban.
	if(accPtr->SuspendTimeSec >= 0)
	{
		unsigned long timePassed = g_PlatformTime.getAbsoluteSeconds() - accPtr->SuspendTimeSec;
		if(timePassed < accPtr->SuspendDurationSec)
		{
			unsigned long remain = accPtr->SuspendDurationSec - timePassed;
			Util::FormatTime(Aux3, sizeof(Aux3), remain);
			Util::SafeFormat(Aux1, sizeof(Aux1), "Your account has been suspended. Time remaining: %s", Aux3);
			ForceErrorMessage(Aux1, INFOMSG_ERROR);
			Disconnect("SimulatorThread::handle_lobby_authenticate");
			return;
		}
		else
			accPtr->ClearBan();
	}

	//Check for multiple logins.
	if(accPtr->GetSessionLoginCount() > 0)
	{
		if(accPtr->HasPermission(Perm_Account, Permission_Admin) == false)
		{
			g_SimulatorManager.CheckIdleSimulatorBoot(accPtr);
			ForceErrorMessage("That account is already logged in.", INFOMSG_ERROR);
			Disconnect("SimulatorThread::handle_lobby_authenticate");
			return;
		}
	}

	//If we get here, we can successfully log into the account.
	accPtr->AdjustSessionLoginCount(1);
	LogMessageL(MSG_SHOW, "Logging in as: %s [Socket:%d]", accPtr->Name, sc.ClientSocket);
	unsigned long startTime = g_PlatformTime.getMilliseconds();
	LoadAccountCharacters(accPtr);
	LogMessageL(MSG_SHOW, "[DEBUG] TIME PASS loading account chars: %d ms", g_PlatformTime.getMilliseconds() - startTime);
	
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 50);       //_handleLoginQueueMessage
	wpos += PutShort(&SendBuf[wpos], 0);       //Placeholder for message size
	wpos += PutInteger(&SendBuf[wpos], 0);     //Queue position
	PutShort(&SendBuf[1], wpos - 3);
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: ClearAuxBuffers(void)
{
	memset(Aux1, 0, sizeof(Aux1));
	memset(Aux2, 0, sizeof(Aux2));
	memset(Aux3, 0, sizeof(Aux3));
}

void SimulatorThread :: BroadcastMessage(const char *message)
{
	LogMessageL(MSG_SHOW, "Broadcast message '%s'", message);
	int wpos = PrepExt_Broadcast(SendBuf, message);
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: SendInfoMessage(const char *message, char eventID)
{
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 0);       //_handleInfoMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Placeholder for size

	wpos += PutStringUTF(&SendBuf[wpos], message);
	wpos += PutByte(&SendBuf[wpos], eventID);       //Event type for error message

	PutShort(&SendBuf[1], wpos - 3);     //Set size

	AttemptSend(SendBuf, wpos);
}


void SimulatorThread :: JoinGuild(GuildDefinition *gDef, int startValour)
{
	g_CharacterManager.GetThread("CharacterData::JoinGuild");
	pld.charPtr->JoinGuild(gDef->guildDefinitionID);
	pld.charPtr->AddValour(gDef->guildDefinitionID, startValour);
	pld.charPtr->cdef.css.SetSubName(gDef->defName);
	g_CharacterManager.ReleaseThread();
	creatureInst->SendStatUpdate(STAT::SUB_NAME);
	AddMessage((long)&pld.charPtr->cdef, 0, BCM_UpdateCreatureDef);
	pld.charPtr->pendingChanges++;
}

void SimulatorThread :: SendPlaySound(const char *assetPackage, const char *soundFile)
{
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 100);       //_handleInfoMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Placeholder for size

	wpos += PutByte(&SendBuf[wpos], 4); //Event
	wpos += PutStringUTF(&SendBuf[wpos], assetPackage);
	wpos += PutStringUTF(&SendBuf[wpos], soundFile);

	PutShort(&SendBuf[1], wpos - 3);     //Set size

	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: LoadAccountCharacters(AccountData *accPtr)
{
	//Called when authentication is successful, assigns the account data.
	pld.Reset();
	pld.accPtr = accPtr;

	/* OBSOLETE: no longer need to load characters here.  The persona list uses a cache stored in the
	   accounts file.  If no cache entry is found for a particular character, the character file
	   is loaded there.  This improves load time by preventing unnecessary loading of character files.

	//Need to load all the characters associated with this account.
	g_CharacterManager.GetThread("SimulatorThread::LoadAccountCharacters");
	for(int i = 0; i < MaxCharacters; i++)
		if(accPtr->CharacterSet[i] != 0)
			g_CharacterManager.RequestCharacter(accPtr->CharacterSet[i], true);
	g_CharacterManager.ReleaseThread();
	*/
}

void SimulatorThread :: handle_lobby_query(void)
{
	int PendingData = 0;
	query.Clear();  //Clear it before processing so that debug polls can fetch the most recent query.
	ReadQueryFromMessage();

	if(query.name.compare("persona.list") == 0)
		handle_query_persona_list();
	else if(query.name.compare("persona.create") == 0)
		PendingData = handle_query_persona_create();
	else if(query.name.compare("persona.delete") == 0)
		PendingData = handle_query_persona_delete();
	else if(query.name.compare("pref.getA") == 0)
		handle_query_pref_getA();
	else if(query.name.compare("account.tracking") == 0)
		handle_query_account_tracking();
	else if(query.name.compare("util.ping") == 0)
		handle_query_util_ping();
	else
		LogMessageL(MSG_WARN, "[WARNING] Unhandled query in lobby: %s", query.name.c_str());

	if(PendingData > 0)
	{
		AttemptSend(SendBuf, PendingData);
		PendingSend = false;
	}
}

void SimulatorThread :: ReadQueryFromMessage(void)
{
	if(query.ID != 0)
		query.Clear();

	query.ID = GetInteger(&readPtr[ReadPos], ReadPos);

	GetStringUTF(&readPtr[ReadPos], Aux1, sizeof(Aux1), ReadPos);
	query.name = Aux1;
	//LogMessageL(MSG_DIAGV, "Query [%d]=[%s]", query.ID, Aux1);

	query.argCount = GetByte(&readPtr[ReadPos], ReadPos);
	for(int i = 0; i < query.argCount; i++)
	{
		GetStringUTF(&readPtr[ReadPos], Aux1, sizeof(Aux1), ReadPos);
		query.args.push_back(Aux1);
		//LogMessageL(MSG_SHOW, "  [%d]=%s", i, Aux1);
	}
	
	//Should never happen, but ensure that argCount is a safe to use
	//for traversing the args list.
	if(query.argCount != query.args.size())
	{
		LogMessageL(MSG_ERROR, "[ERROR] Query argCount mismatch.");
		query.argCount = query.args.size();
	}
}

void SimulatorThread :: handle_query_persona_list(void)
{
	/* Query: persona.list
	   Args : [none]
	   Notes: First query sent to the server.
	   Response: Send back the list of characters available on this account.
	*/

	//Seems to be a rare condition when the account can indeed be NULL at this point.  Possibly
	//disconnecting after the query is sent, but before it's processed?
	if(pld.accPtr == NULL)
	{
		LogMessageL(MSG_CRIT, "[CRITICAL] persona.list null account");
		return;
	}

	g_Log.AddMessageFormat("Retrieving persona list for account:%d", pld.accPtr->ID);
	//TODO: Fix a potential buffer overflow.

	WritePos = 0;
	WritePos += PutByte(&SendBuf[WritePos], 1);            //_handleQueryResultMsg
	WritePos += PutShort(&SendBuf[WritePos], 0);           //Message size
	WritePos += PutInteger(&SendBuf[WritePos], query.ID);  //Query ID

	//Character row count
	int charCount = pld.accPtr->GetCharacterCount();
	if(g_ProtocolVersion >= 38)
	{
		//Version 0.8.9 has a an extra row in the beginning
		//with one string, the number of characters following.
		WritePos += PutShort(&SendBuf[WritePos], charCount + 1);

		WritePos += PutByte(&SendBuf[WritePos], 1);
		sprintf(Aux3, "%d", charCount + 1);
		WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);
	}
	else
	{
		WritePos += PutShort(&SendBuf[WritePos], charCount);
	}


	g_CharacterManager.GetThread("SimulatorThread::handle_query_persona_list");

	int b;
	for(b = 0; b < AccountData::MAX_CHARACTER_SLOTS; b++)
	{
		if(pld.accPtr->CharacterSet[b] != 0)
		{
			int cdefid = pld.accPtr->CharacterSet[b];
			CharacterCacheEntry *cce = pld.accPtr->characterCache.ForceGetCharacter(cdefid);
			if(cce == NULL)
			{
				LogMessageL(MSG_ERROR, "[ERROR] Could not request character: %d", cdefid);
				ForceErrorMessage("Critical: could not load a character.", INFOMSG_ERROR);
				Disconnect("SimulatorThread::handle_query_persona_list");
				return;
			}

			if(g_Config.AprilFools != 0)
			{
				WritePos += PutByte(&SendBuf[WritePos], 6);       //6 character data strings
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->display_name.c_str()); //Seems to be sent twice in 0.8.9.  Unknown purpose.
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->display_name.c_str());
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->appearance.c_str());

				const char *eqApp = cce->eq_appearance.c_str();
				switch(cce->profession)
				{
				case 1: eqApp = "{[1]=3163,[0]=141760,[6]=3019,[10]=3008,[11]=2831}"; break;
				case 2: eqApp = "{[0]=141763,[1]=141764,[6]=2107,[10]=2442,[11]=2898}"; break;
				case 3: eqApp = "{[6]=2810,[10]=1980,[11]=2108,[2]=141765}"; break;
				case 4: eqApp = "{[2]=143609,[6]=3160,[10]=3161,[11]=3162}"; break;
				}
				WritePos += PutStringUTF(&SendBuf[WritePos], eqApp);

				//sprintf(Aux3, "%d", cce->level);
				WritePos += PutStringUTF(&SendBuf[WritePos], "1");

				sprintf(Aux3, "%d", cce->profession);
				WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);
				if(WritePos >= sizeof(SendBuf))
					g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Buffer overflow in persona.list");
			}
			else
			{
				//Normal stuff.
				WritePos += PutByte(&SendBuf[WritePos], 6);       //6 character data strings
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->display_name.c_str()); //Seems to be sent twice in 0.8.9.  Unknown purpose.
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->display_name.c_str());
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->appearance.c_str());
				WritePos += PutStringUTF(&SendBuf[WritePos], cce->eq_appearance.c_str());

				sprintf(Aux3, "%d", cce->level);
				WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);

				sprintf(Aux3, "%d", cce->profession);
				WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);
				if(WritePos >= sizeof(SendBuf))
					g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] Buffer overflow in persona.list");
			}
		}
	}
	PutShort(&SendBuf[1], WritePos - 3);       //Set message size
	g_CharacterManager.ReleaseThread();

	PendingSend = true;
}

int SimulatorThread :: handle_query_persona_create(void)
{
	/* Query: persona.create
	   Args : [variable] list of attributes
	   Notes: Attributes are customized by the player in the Character Creation
	          steps.
	   Return: Unknown.  Possibly an index for use in the client?
	*/
	g_AccountManager.cs.Enter("SimulatorThread::handle_query_persona_create");
	int r = g_AccountManager.CreateCharacter(query.args, pld.accPtr);
	g_AccountManager.cs.Leave();
	if(r < AccountManager::CHARACTER_SUCCESS)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_AccountManager.GetCharacterErrorMessage(r));

	sprintf(Aux1, "%d", r + 1);
	return PrepExt_QueryResponseString(SendBuf, query.ID, Aux1);
}

int SimulatorThread :: handle_query_persona_delete(void)
{
	/* Query: persona.delete
	   Args : [0] index to remove
	*/
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query");

	g_AccountManager.DeleteCharacter(query.GetInteger(0), pld.accPtr);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


void SimulatorThread :: handle_query_pref_getA(void)
{
	/* Query: pref.getA
	   Args : [variable] list of preferences (by named string) to retrieve.
	   Notes: Retrieves the account preferences.  Sent after the client login is
	          successful.
	   Response: Search for each preference in the account data and send back the
	          string data.
	*/
	RespondPrefGet(&pld.accPtr->preferenceList);
	PendingSend = true;
}

void SimulatorThread :: handle_query_pref_get(void)
{
	/* Query: pref.get
	   Args : [variable] list of preferences (by named string) to retrieve.
	   Notes: Retrieves the character preferences.
	   Response: Search for each preference in the character data and send back
	          the string data.
	*/

	RespondPrefGet(&pld.charPtr->preferenceList);
	PendingSend = true;
}

void SimulatorThread :: handle_query_pref_setA(void)
{
	/* Query: pref.setA
	   Args : [variable, multiple of two] list of string pairs of the name and
	          value to set (in the account permission set).
	   Response: Return standard "OK".
	*/

	for(int i = 0; i < query.argCount; i += 2)
		pld.accPtr->preferenceList.SetPref(query.args[i].c_str(), query.args[i + 1].c_str());

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	//Account data was changed, flag the change so the system can autosave the
	//file when needed.
	pld.accPtr->PendingMinorUpdates++;

	PendingSend = true;
}

void SimulatorThread :: handle_query_pref_set(void)
{
	/* Query: pref.set
	   Args : [variable, multiple of two] list of string pairs of the name and
	          value to set (in the character permission set).
	   Response: Return standard "OK".
	*/

	//The character system needs extra debug steps.
	for(int i = 0; i < query.argCount; i += 2)
	{
		const char *name = query.args[i].c_str();
		const char *value = query.args[i + 1].c_str();
		//if(LoadStage < LOADSTAGE_LOADED)
		//	LogMessageL(MSG_WARN, "[WARNING] Pref set while loading [%s]=[%s"], name, value);

		bool allow = true;
		if(strstr(name, "quickbar") != NULL)
		{
			if(LoadStage < 2)
			{
				allow = false;
				LogMessageL(MSG_WARN, "[WARNING] Tried to set quickbar preference before gameplay [%s]=[%s]", name, value);
			}
			else if(strlen(value) < 3)
			{
				allow = false;
				LogMessageL(MSG_WARN, "[WARNING] Tried to set quickbar preference to NULL [%s]=[%s]", name, value);
			}
		}

		if(allow == true)
			pld.charPtr->preferenceList.SetPref(name, value);
	}

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	RespondPrefGet(&pld.charPtr->preferenceList);
	PendingSend = true;
}

void SimulatorThread :: RespondPrefGet(PreferenceContainer *prefSet)
{
	//Helper function for the "pref.getA" and "pref.get" queries.
	//Since the both accounts and characters use the same class to store
	//preferences, the query handlers will call this function with the
	//appropriate pointer.

	WritePos = 0;
	WritePos += PutByte(&SendBuf[WritePos], 1);            //_handleQueryResultMsg
	WritePos += PutShort(&SendBuf[WritePos], 0);           //Message size
	WritePos += PutInteger(&SendBuf[WritePos], query.ID);  //Query response index

	//Each preference request will have a matching response field.
	WritePos += PutShort(&SendBuf[WritePos], query.argCount);

	for(int i = 0; i < query.argCount; i++)
	{
		const char * pref = prefSet->GetPrefValue(query.args[i].c_str());

		//One string for each preference result.
		WritePos += PutByte(&SendBuf[WritePos], 1);
		if(pref != NULL)
		{
			WritePos += PutStringUTF(&SendBuf[WritePos], pref);
		}
		else
		{
			WritePos += PutStringUTF(&SendBuf[WritePos], "");
		}
	}

	PutShort(&SendBuf[1], WritePos - 3);
}

void SimulatorThread :: handle_inspectItemDef(void)
{
	int itemID = GetInteger(&readPtr[ReadPos], ReadPos);
	//LogMessageL(MSG_SHOW, "inspectItemDef requested for %d", itemID);

	ItemDef *item = g_ItemManager.GetSafePointerByID(itemID);
	WritePos = 0;

	//_handleItemDefUpdateMsg is [4] for lobby, [71] for play (most common) protocol
	char message = 71;
	if(ProtocolState == 0)
		message = 4;

	WritePos += PutByte(&SendBuf[WritePos], message);
	WritePos += PutShort(&SendBuf[WritePos], 0);      //Message size

	WritePos += PutInteger(&SendBuf[WritePos], itemID);

	//Fill the item properties
	WritePos += PutByte(&SendBuf[WritePos], item->mType);
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mDisplayName.c_str());
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mAppearance.c_str());
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mIcon.c_str());

	WritePos += PutByte(&SendBuf[WritePos], item->mIvType1);
	WritePos += PutShort(&SendBuf[WritePos], item->mIvMax1);
	WritePos += PutByte(&SendBuf[WritePos], item->mIvType2);
	WritePos += PutShort(&SendBuf[WritePos], item->mIvMax2);
	WritePos += PutStringUTF(&SendBuf[WritePos], item->mSv1.c_str());

	if(g_ProtocolVersion < 5)
		WritePos += PutInteger(&SendBuf[WritePos], item->_mCopper);

	WritePos += PutShort(&SendBuf[WritePos], item->mContainerSlots);
	WritePos += PutByte(&SendBuf[WritePos], item->mAutoTitleType);
	WritePos += PutShort(&SendBuf[WritePos], item->mLevel);
	WritePos += PutByte(&SendBuf[WritePos], item->mBindingType);
	WritePos += PutByte(&SendBuf[WritePos], item->mEquipType);

	WritePos += PutByte(&SendBuf[WritePos], item->mWeaponType);
	if(item->mWeaponType != 0)
	{
		if(g_ProtocolVersion == 7)
		{
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponDamageMin);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponDamageMax);
			WritePos += PutByte(&SendBuf[WritePos], item->_mSpeed);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamangeRating);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamageType);
		}
		else
		{
			WritePos += PutInteger(&SendBuf[WritePos], item->mWeaponDamageMin);
			WritePos += PutInteger(&SendBuf[WritePos], item->mWeaponDamageMax);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamangeRating);
			WritePos += PutByte(&SendBuf[WritePos], item->mWeaponExtraDamageType);
		}
	}

	WritePos += PutInteger(&SendBuf[WritePos], item->mEquipEffectId);
	WritePos += PutInteger(&SendBuf[WritePos], item->mUseAbilityId);
	WritePos += PutInteger(&SendBuf[WritePos], item->mActionAbilityId);
	WritePos += PutByte(&SendBuf[WritePos], item->mArmorType);
	if(item->mArmorType != 0)
	{
		if(g_ProtocolVersion == 7)
		{
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistMelee);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistFire);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistFrost);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistMystic);
			WritePos += PutByte(&SendBuf[WritePos], item->mArmorResistDeath);
		}
		else
		{
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistMelee);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistFire);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistFrost);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistMystic);
			WritePos += PutInteger(&SendBuf[WritePos], item->mArmorResistDeath);
		}
	}
	if(g_ProtocolVersion == 7)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusStrength);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusDexterity);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusConstitution);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusPsyche);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusSpirit);
		WritePos += PutByte(&SendBuf[WritePos], item->_mBonusHealth);
		WritePos += PutByte(&SendBuf[WritePos], item->mBonusWill);
	}
	else
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusStrength);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusDexterity);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusConstitution);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusPsyche);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusSpirit);

		if(g_ProtocolVersion < 32)
			WritePos += PutInteger(&SendBuf[WritePos], item->_mBonusHealth);
		WritePos += PutInteger(&SendBuf[WritePos], item->mBonusWill);
	}

	if(g_ProtocolVersion >= 4)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->isCharm);
		if(item->isCharm != 0)
		{
			WritePos += PutFloat(&SendBuf[WritePos], item->mMeleeHitMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMeleeCritMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMagicHitMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mMagicCritMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mParryMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mBlockMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mRunSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mRegenHealthMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mAttackSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mCastSpeedMod);
			WritePos += PutFloat(&SendBuf[WritePos], item->mHealingMod);
		}
	}

	if(g_ProtocolVersion >= 5)
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->mValue);
		WritePos += PutByte(&SendBuf[WritePos], item->mValueType);
	}

	bool ItemUpdateDefMsgCraft = false;
	if(g_ProtocolVersion >= 7)
		ItemUpdateDefMsgCraft = true;
	if(ItemUpdateDefMsgCraft == true)
	{
		WritePos += PutInteger(&SendBuf[WritePos], item->resultItemId);
		WritePos += PutInteger(&SendBuf[WritePos], item->keyComponentId);
		WritePos += PutInteger(&SendBuf[WritePos], item->numberOfItems);
		for(size_t i = 0; i < item->craftItemDefId.size(); i++)
			WritePos += PutInteger(&SendBuf[WritePos], item->craftItemDefId[i]);

		if(item->numberOfItems != item->craftItemDefId.size())
			g_Log.AddMessageFormatW(MSG_WARN, "[ERROR] Crafting material item count mismatch for ID: %d", item->mID);
	}

	if(g_ProtocolVersion >= 9)
		WritePos += PutStringUTF(&SendBuf[WritePos], item->mFlavorText.c_str());

	if(g_ProtocolVersion >= 18)
		WritePos += PutByte(&SendBuf[WritePos], item->mSpecialItemType);

	if(g_ProtocolVersion >= 30)
		WritePos += PutByte(&SendBuf[WritePos], item->mOwnershipRestriction);

	if(g_ProtocolVersion >= 31)
	{
		WritePos += PutByte(&SendBuf[WritePos], item->mQualityLevel);
		WritePos += PutShort(&SendBuf[WritePos], item->mMinUseLevel);
	}

	PutShort(&SendBuf[1], WritePos - 3);
	PendingSend = true;
}

void SimulatorThread :: handle_disconnect(void)
{
	//This command is used by 0.8.9.
	LogMessageL(MSG_SHOW, "The client sent a disconnect message.");
	Disconnect("SimulatorThread: handle_disconnect");
}

void SimulatorThread :: handle_debugServerPing(void)
{
	//Received response from a diagnostic ping initiated by the server (to a modded client).
	//Verify the data and notify the user if necessary.

	int MessageID = GetInteger(&readPtr[ReadPos], ReadPos);
	int InitialSendTime = GetInteger(&readPtr[ReadPos], ReadPos);

	int TimeDiff = static_cast<int>(g_ServerTime - g_ServerLaunchTime) - InitialSendTime;

	pld.DebugPingServerLastMsgReceived = MessageID;
	pld.DebugPingServerTotalMsgReceived++;
	pld.DebugPingServerTotalReceived++;
	pld.DebugPingServerTotalTime += TimeDiff;

	if((TimeDiff < pld.DebugPingServerLowest) || (pld.DebugPingServerLowest == 0))
		pld.DebugPingServerLowest = TimeDiff;
	else if((TimeDiff > pld.DebugPingServerHighest) || (pld.DebugPingServerHighest == 0))
		pld.DebugPingServerHighest = TimeDiff;

	if((pld.DebugPingServerNotifyTime != 0) && (TimeDiff > pld.DebugPingServerNotifyTime))
	{
		sprintf(Aux1, "Server detected a ping of %d ms",  TimeDiff);
		SendInfoMessage(Aux1, INFOMSG_INFO);
	}
	if(TimeDiff >= g_Config.DebugPingServerLogThreshold)
	{
		const char *name = "<unknown>";
		if(creatureInst != NULL)
			name = creatureInst->css.display_name;
		int severity = TimeDiff / 1000;
		unsigned long sec = static_cast<unsigned long>(g_PlatformTime.getAbsoluteSeconds());
		LogMessageL(MSG_SHOW, "[PING][WARNING]%s|ID:%d|sTime:%d|Time:%lu|Difference:%lu|Severity:%02d",
			name,
			MessageID,
			InitialSendTime,
			sec,
			TimeDiff,
			severity);
	}
}

void SimulatorThread :: handle_acknowledgeHeartbeat(void)
{
	//Sent by modded client only.
	//We don't need to do anything here since any response to recv will reset the response.
	//LogMessageL(MSG_SHOW, "Received response.");
}

void SimulatorThread :: handle_lobby_selectPersona(void)
{
	short personaIndex = GetShort(&readPtr[ReadPos], ReadPos);
	if(g_ProtocolVersion >= 38)
	{
		//Extra stuff for 0.8.9
		int var1 = GetInteger(&readPtr[ReadPos], ReadPos);
		int var2 = GetInteger(&readPtr[ReadPos], ReadPos);
		LogMessageL(MSG_DIAGV, "[DEBUG] 0.8.9 login data - Persona:%d, Unknowns:%d,%d", personaIndex, var1, var2);
	}
	SetPersona(personaIndex);
	LogMessageL(MSG_DIAGV, "-- SetPersona --");
}


void SimulatorThread :: SetPersona(int personaIndex)
{
	if(pld.accPtr == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SetPersona() accPtr is NULL");
		Disconnect("SimulatorThread::SetPersona");
		return;
	}
	if(personaIndex < 0 || personaIndex >= AccountData::MAX_CHARACTER_SLOTS)
	{
		LogMessageL(MSG_ERROR, "Invalid persona index: %d", personaIndex);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}

	int CDefID = pld.accPtr->CharacterSet[personaIndex];
	if(CDefID == 0)
	{
		LogMessageL(MSG_ERROR, "[ERROR] Character index is not valid: %s:%d", pld.accPtr->Name, personaIndex);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}
	LogMessageL(MSG_DIAGV, "Setting character: %s:%d", pld.accPtr->Name, personaIndex);

	//Check to make sure the persona isn't already in the active list
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true && it->ProtocolState == 1)
		{
			if(it->pld.CreatureDefID == CDefID)
			{
				ForceErrorMessage("That character is already logged in.", INFOMSG_INFO);
				Disconnect("SimulatorThread::SetPersona");
				return;
			}
		}
	}

	g_CharacterManager.GetThread("SimulatorThread::SetPersona");
	pld.charPtr = g_CharacterManager.RequestCharacter(CDefID, false);
	g_CharacterManager.ReleaseThread();

	if(pld.charPtr == NULL)
	{
		ForceErrorMessage("ERROR: Character could not be loaded.", INFOMSG_ERROR);
		LogMessageL(MSG_ERROR, "Character [%d] could not be loaded.", CDefID);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}
	pld.charPtr->EraseExpirationTime();


	pld.CreatureDefID = pld.charPtr->cdef.CreatureDefID;
	//pld.CreatureID will be set when the character is registered into an instance

	creatureInst->CreatureID = 0;
	creatureInst->CreatureDefID = pld.charPtr->cdef.CreatureDefID;
	//creatureInst->SimulatorIndex = InternalIndex;
	creatureInst->simulatorPtr = this;
	creatureInst->charPtr = pld.charPtr;
	creatureInst->css.CopyFrom(&pld.charPtr->cdef.css);

	LoadCharacterSession();
	creatureInst->Rotation = pld.charPtr->activeData.CurRotation;
	creatureInst->Heading = pld.charPtr->activeData.CurRotation;

	ChangeProtocol(1);

	int ZoneID = pld.charPtr->activeData.CurZone;
	if(ZoneID <= 0)
	{
		//No zone has been set, maybe missing data in the character file.
		//Set default zone and position.
		ZoneID = g_DefZone;
		pld.charPtr->activeData.CurZone = g_DefZone;
		pld.charPtr->activeData.CurX = g_DefX;
		pld.charPtr->activeData.CurY = g_DefY;
		pld.charPtr->activeData.CurZ = g_DefZ;
	}
	//int InstanceID = pld.charPtr->activeData.CurInstance;
	int InstanceID = 0;

	if(ProtectedSetZone(ZoneID, InstanceID) == false)
	{
		ForceErrorMessage("SetPersona: critical error setting zone.", INFOMSG_ERROR);
		g_Log.AddMessageFormatW(MSG_ERROR, "SetPersona: critical error setting zone: %d\n", ZoneID);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}

	//If an instance, reset players back to the start if nothing has been killed.
	//Prevents an exploit with boss camping and relogging into spawned instances.
	if(pld.zoneDef->IsDungeon() == true && creatureInst->actInst->mKillCount == 0)
		SetPosition(pld.zoneDef->DefX, pld.zoneDef->DefY, pld.zoneDef->DefZ, 1);
	else
		SetPosition(pld.charPtr->activeData.CurX, pld.charPtr->activeData.CurY, pld.charPtr->activeData.CurZ, 1);

	creatureInst->actInst->FetchNearbyCreatures(this, creatureInst);

	SendSetAvatar(creatureInst->CreatureID);

	//The zone change updates the map, but need to resend again otherwise the
	//terrain won't load when you first log in.
	SendSetMap();
	UpdateSocialEntry(true, false);
	UpdateSocialEntry(true, true);
	BroadcastShardChanged();


	//Since the character has been loaded into an instance and has acquired
	//a character instance, run processing for abilities.
	creatureInst->ActivateSavedAbilities();
	ActivatePassiveAbilities();

	UpdateEqAppearance();
	UpdateEqAppearance();

	LogMessageL(MSG_DIAG, "Persona set to index:%d, (ID: %d, CDef: %d) (%s)", personaIndex, pld.CreatureID, pld.CreatureDefID, pld.charPtr->cdef.css.display_name);

	//Hack to reset an empty quickbar preference.
	const char *value = pld.charPtr->preferenceList.GetPrefValue("quickbar.0");
	bool reset = false;
	if(value == NULL)
		reset = true;
	else
	{
		if(strlen(value) < 3)
			reset = true;
	}
	if(reset == true)
	{
		LogMessageL(MSG_SHOW, "[WARNING] Have to reset [quickbar.0]=[%s]", value);
		CharacterData *defChar = g_CharacterManager.GetDefaultCharacter();
		const char *newValue = defChar->preferenceList.GetPrefValue("quickbar.0");
		if(newValue != NULL)
			pld.charPtr->preferenceList.SetPref("quickbar.0", newValue);
	}

	pld.UpdateNextIdleCheckTime();

	LogMessageL(MSG_SHOW, "[LOGIN] %s has logged in [%s]", creatureInst->css.display_name, clientPeerName);
}


/*
bool SimulatorThread :: RegisterMainCall3(const char *debugFunction)
{
	return true;
	//OBSOLETE BELOW

	//Set the status before registering.
	if(threadsys.status == ThreadRequest::STATUS_WAITWORK)
	{
		LogMessageL(MSG_WARN, "[WARNING] RegisterMainCall3 already waiting");
		return true;
	}

	threadsys.status = ThreadRequest::STATUS_WAITMAIN;
	g_SimulatorManager.RegisterAction(&threadsys);
	bool res = threadsys.WaitForStatus(ThreadRequest::STATUS_WAITWORK, 1, ThreadRequest::DEFAULT_WAIT_TIME);
	if(res == false)
	{
		LogMessageL(MSG_WARN, "[WARNING] RegisterMainCall3 timed out, operation [%s]", debugFunction);
		//UnregisterMainCall3();
	}
	return res;
}

void SimulatorThread :: UnregisterMainCall3(void)
{
	return;
	//OBSOLETE BELOW

	threadsys.status = ThreadRequest::STATUS_COMPLETE;
	g_SimulatorManager.UnregisterAction(&threadsys);
}
*/

bool SimulatorThread :: MainCallSetZone(int newZoneID, int newInstanceID, bool setDefaultLocation)
{
	//int newZoneID = MainCallData.param.GetLong();
	LogMessageL(MSG_SHOW, "Attempting to set zone: %d", newZoneID);

	if(pld.zoneDef != NULL)
	{
		//Don't do anything if already in the required zone.
		if(newZoneID == pld.zoneDef->mID && newInstanceID == 0)
			return true;
		
		MainCallHelperInstanceUnregister();
	}

	//The party needs to be checked before placing into a zone.
	if(LoadStage == LOADSTAGE_UNLOADED)
	{
		ActiveParty *party = g_PartyManager.GetPartyWithMember(pld.CreatureDefID);
		if(party != NULL)
		{
			defcInst.PartyID = party->mPartyID;
			LogMessageL(MSG_SHOW, "[DEBUG] Setting party: %d", party->mPartyID);
		}
	}

	MainCallHelperInstanceRegister(newZoneID, newInstanceID);
	if(ValidPointers() == false)
	{
		SendInfoMessage("Critical error changing zones", INFOMSG_ERROR);
		Disconnect("SimulatorThread::MainCallSetZone");
		return false;
	}

	pld.MovementBlockTime = g_ServerTime + g_Config.WarpMovementBlockTime;
	if(setDefaultLocation == true)
		SetPosition(pld.zoneDef->DefX, pld.zoneDef->DefY, pld.zoneDef->DefZ, 1);

	//If a player joins another instance, it will be given a new ID.
	//The client needs to be informed of the change, otherwise it won't
	//accept proper updates for the player's creature ID.
	SendSetAvatar(creatureInst->CreatureID);

	pld.CurrentZoneID = pld.zoneDef->mID;
	pld.CurrentInstanceID = creatureInst->actInst->mInstanceID;

	Util::SafeFormat(pld.CurrentZone, sizeof(pld.CurrentZone), "[%d-%d-0]", pld.CurrentInstanceID, pld.CurrentZoneID);
	pld.LastMapTick = MapTickChange;
	SendZoneInfo();

	CheckSpawnTileUpdate(true);

	CheckMapUpdate(true);

	BroadcastShardChanged();  //Let friends know we changed shards.

	int r = pld.charPtr->questJournal.CheckTravelLocations(creatureInst->CreatureID, Aux1, creatureInst->CurrentX, creatureInst->CurrentY, creatureInst->CurrentZ, pld.CurrentZoneID);
	if(r > 0)
		AttemptSend(Aux1, r);

	if(CheckPermissionSimple(Perm_Account, Permission_Invisible) == true)
		creatureInst->_AddStatusList(StatusEffects::GM_INVISIBLE, -1);

	g_PartyManager.UpdatePlayerReferences(creatureInst);

	creatureInst->SetServerFlag(ServerFlags::Noncombatant, true);
	pld.IgnoreNextMovement = true;
	return true;
}

bool SimulatorThread :: ProtectedSetZone(int newZoneID, int newInstanceID)
{
	//The function originally had thread potection when using a faulty method of threading.
	//Returns false if the operation failed.
	MainCallSetZone(newZoneID, newInstanceID, false);
	
	return ValidPointers();
}


void SimulatorThread :: LoadCharacterSession(void)
{
	//Loads session information from the Character data.
	TimeOnline = g_ServerTime;
	TimeLastAutoSave = g_ServerTime;

	time_t curtime;
	time(&curtime);
	strftime(pld.charPtr->LastLogOn, sizeof(pld.charPtr->LastLogOn), "%Y-%m-%d, %I:%M %p", localtime(&curtime));
	pld.charPtr->SessionsLogged++;

	defcInst.CurrentX = pld.charPtr->activeData.CurX;
	defcInst.CurrentY = pld.charPtr->activeData.CurY;
	defcInst.CurrentZ = pld.charPtr->activeData.CurZ;
	defcInst.Rotation = pld.charPtr->activeData.CurRotation;
	defcInst.Heading = pld.charPtr->activeData.CurRotation;
	defcInst.cooldownManager.CopyFrom(pld.charPtr->cooldownManager);
	defcInst.buffManager.CopyFrom(pld.charPtr->buffManager);
}

void SimulatorThread :: ChangeProtocol(int newProto)
{
	LogMessageL(MSG_SHOW, "Changing Protocol: %d", newProto);
	ProtocolState = newProto;
	if(ProtocolState == 1)
	{
		int wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 255);             //_handleProtocolChangedMsg
		wpos += PutShort(&SendBuf[wpos], 1);              //Message size
		wpos += PutByte(&SendBuf[wpos], ProtocolState);   //Set to Play protocol
		AttemptSend(SendBuf, wpos);
	}
}

bool SimulatorThread :: ValidPointers(void)
{
	//Standard debug check.  Makes sure the pointers are valid.
	if(pld.accPtr == NULL)
	{
		LogMessageL(MSG_CRIT, "[ERROR] ValidPointers failed accPtr");
		return false;
	}
	if(pld.charPtr == NULL)
	{
		LogMessageL(MSG_CRIT, "[ERROR] ValidPointers failed charPtr");
		return false;
	}
	if(creatureInst == NULL)
	{
		LogMessageL(MSG_CRIT, "[ERROR] ValidPointers failed creatureInst");
		return false;
	}
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_CRIT, "[ERROR] ValidPointers failed actInst");
		return false;
	}
	if(pld.zoneDef == NULL)
	{
		LogMessageL(MSG_CRIT, "[ERROR] ValidPointers failed zoneDef");
		return false;
	}
	return true;
}

void SimulatorThread :: SendSetAvatar(int CreatureID)
{
	//Notify the client to link a creature instance ID to its active
	//avatar object.
	//Note: must be in play protocol to process this message.
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 4);  //_handleCreatureEventMsg
	wpos += PutShort(&SendBuf[wpos], 0);

	wpos += PutInteger(&SendBuf[wpos], CreatureID);   //actorID
	wpos += PutByte(&SendBuf[wpos], 1);     //1 = Event to set avatar.

	PutShort(&SendBuf[1], wpos - 3);       //Set message size
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: MainCallHelperInstanceUnregister(void)
{
	if(creatureInst->actInst != NULL)
		creatureInst->actInst->UnregisterPlayer(this);

	g_PartyManager.RemovePlayerReferences(creatureInst->CreatureDefID, false);

	if(creatureInst->actInst == NULL)
		return;


	int wpos = PrepExt_RemoveCreature(SendBuf, creatureInst->CreatureID);
	creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, InternalIndex);

	creatureInst->actInst->SidekickUnregister(creatureInst);
	g_ActiveInstanceManager.FlushSimulator(InternalID);

	creatureInst->OnInstanceExit();
	defcInst.CopyFrom(creatureInst);
	creatureInst->actInst->UnloadPlayer(this);
	
	creatureInst = &defcInst;  //The former CreatureInstance object no longer exists.
	defcInst.actInst = NULL;
	pld.zoneDef = NULL;
	pld.CurrentZoneID = 0;
}

void SimulatorThread :: FillPlayerInstancePlacementData(PlayerInstancePlacementData &pd, int ZoneID, int InstanceID)
{
	pd.in_cInst = &defcInst;                 //This is the "default" holder of creature data when swapping instances.
	pd.in_simPtr = this;
	pd.in_instanceID = InstanceID;
	pd.in_creatureDefID = pld.CreatureDefID;
	pd.in_partyID = creatureInst->PartyID;   //Use current instance party.
	pd.in_serverSessionID = 0;
	pd.in_zoneID = ZoneID;
	pd.in_playerLevel = defcInst.css.level;
	pd.SetInstanceScaler(pld.charPtr->InstanceScaler);
}

void SimulatorThread :: MainCallHelperInstanceRegister(int ZoneID, int InstanceID)
{
	/*
	if(g_ActiveInstanceManager.AddSimulator(ZoneID, InstanceID, this, 0) == -1)
	{
		LogMessageL(MSG_SHOW, "InstanceRegister() failed.");
		return;
	}
	LogMessageL(MSG_SHOW, "InstanceRegister() success:  cInst: %p (def: %p) aInst: %p", creatureInst, &defcInst, creatureInst->actInst);
	pld.CreatureID = creatureInst->CreatureID;
	*/

	PlayerInstancePlacementData pd;
	FillPlayerInstancePlacementData(pd, ZoneID, InstanceID);

	if(g_ActiveInstanceManager.AddSimulator_Ex(pd) == -1)
	{
		LogMessageL(MSG_SHOW, "InstanceRegister() failed (Zone:%d)", ZoneID);
		return;
	}

	if(pd.out_cInst != NULL)
	{
		creatureInst = pd.out_cInst;
		pld.zoneDef = pd.out_zoneDef;
		pld.CurrentInstanceID = pd.out_instanceID;
		pld.CreatureID = pd.out_cInst->CreatureID;
		creatureInst->actInst->SidekickRegister(creatureInst, &pld.charPtr->SidekickList);
	}
	else
	{
		LogMessageL(MSG_SHOW, "[CRITICAL] InstanceRegister() failed (Zone:%d)", ZoneID);
		creatureInst = &defcInst;
	}
}

void SimulatorThread :: SendZoneInfo(void)
{
	//Sends zone information to the client.  This includes terrain and
	//environment info.

	if(creatureInst == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SendSetMap() creature instance is NULL");
		return;
	}
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SendSetMap() creature active instance is NULL");
		return;
	}

	int wpos = PrepExt_SendEnvironmentUpdateMsg(SendBuf, pld.CurrentZone, pld.zoneDef, creatureInst->CurrentX, creatureInst->CurrentZ);
	AttemptSend(SendBuf, wpos);

	SendTimeOfDay(NULL);
}

const char * SimulatorThread :: GetTimeOfDay(void)
{
	static const char *DEFAULT = "Day";

	if(CheckPermissionSimple(Perm_Account, Permission_Troll))
		return DEFAULT;

	if(pld.zoneDef == NULL)
		return DEFAULT;

	//If the environment time string is null, attempt to find the active time for the
	//current zone, if applicable.
	if(pld.zoneDef->mEnvironmentCycle == true)
		return g_EnvironmentCycleManager.GetCurrentTimeOfDay();
	
	return DEFAULT;
}

void SimulatorThread :: SendTimeOfDay(const char *envType)
{
	/*
	if(CheckPermissionSimple(Perm_Account, Permission_Troll))
		envType = "Day";

	if(pld.zoneDef == NULL)
		return;

	//If the environment time string is null, attempt to find the active time for the
	//current zone, if applicable.
	if(envType == NULL)
	{
		if(pld.zoneDef->mEnvironmentCycle == false)
			return;

		envType = g_EnvironmentCycleManager.GetCurrentTimeOfDay();
	}

	if(envType == NULL)
		return;
	*/

	//If not specified (NULL), get the current string from the zone.  If still NULL, there's nothing
	//to send.
	if(envType == NULL)
		envType = GetTimeOfDay();
	if(envType == NULL)
		return;

	int wpos = PrepExt_SendTimeOfDayMsg(SendBuf, envType);
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: CheckMapUpdate(bool force)
{
	//Send minimap region title information to the client.
	if(pld.zoneDef == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] CheckMapUpdate() ZoneDef is NULL");
		return;
	}

	pld.LastMapTick++;
	if(pld.LastMapTick >= MapTickChange || force == true)
	{
		pld.LastMapTick = 0;
		int NewMap = 0;
		NewMap = MapLocation.SearchLocation(pld.CurrentZoneID, creatureInst->CurrentX, creatureInst->CurrentZ);
		if(NewMap == -1)
			NewMap = MapDef.SearchMap(pld.zoneDef->mMapName.c_str(), creatureInst->CurrentX, creatureInst->CurrentZ);

		if(NewMap != -1)
		{
			if(pld.CurrentMapInt != NewMap)
			{
				pld.CurrentMapInt = NewMap;
				SendInfoMessage(MapDef.mMapList[pld.CurrentMapInt].Name.c_str(), INFOMSG_LOCATION);
				SendInfoMessage(pld.zoneDef->mShardName.c_str(), INFOMSG_SHARD);
				SendInfoMessage(MapDef.mMapList[pld.CurrentMapInt].image.c_str(), INFOMSG_MAPNAME);
			}
		}
		else
		{
			//Most instances and smaller maps don't have specific regions, so reset the
			//internal map index so that a warp back to a map area will refresh.
			pld.CurrentMapInt = -1;

			SendInfoMessage(pld.zoneDef->mName.c_str(), INFOMSG_LOCATION);
			SendInfoMessage(pld.zoneDef->mShardName.c_str(), INFOMSG_SHARD);
		}

		// EM - Added this for tile specific environments, the message always gets sent,
		// hopefully the client will ignore it if the environment is already correct
		if(creatureInst != NULL) {
			std::string * tEnv = pld.zoneDef->GetTileEnvironment(creatureInst->CurrentX, creatureInst->CurrentZ);
			if(strcmp(tEnv->c_str(), pld.CurrentEnv) != 0) {
				g_Log.AddMessageFormat("Sending environment change to %s", tEnv->c_str());
				SendSetMap();
			}
		}
	}
}

void SimulatorThread :: UpdateSocialEntry(bool newOnlineStatus, bool onlyUpdateFriendList)
{
	if(pld.charPtr == NULL)
	{
		LogMessageL(MSG_CRIT, "[CRITICAL] UpdateSocialEntry charPtr is NULL");
		return;
	}
	if(pld.zoneDef == NULL)
	{
		LogMessageL(MSG_CRIT, "[CRITICAL] UpdateSocialEntry zoneDef is NULL");
		return;
	}
	if(onlyUpdateFriendList == true)
	{
		std::vector<int> friendDefID;
		for(size_t i = 0; i < pld.charPtr->friendList.size(); i++)
			friendDefID.push_back(pld.charPtr->friendList[i].CDefID);
		g_FriendListManager.UpdateNetworkEntry(pld.CreatureDefID, friendDefID);

		return;
	}

	SocialWindowEntry data;
	data.creatureDefID = pld.CreatureDefID;
	data.name = creatureInst->css.display_name;
	data.level = creatureInst->css.level;
	data.profession = static_cast<char>(creatureInst->css.profession);
	data.online = newOnlineStatus;
	data.status = pld.charPtr->StatusText;
	data.shard = pld.zoneDef->mShardName;

	g_FriendListManager.UpdateSocialEntry(data);
}

void SimulatorThread :: BroadcastGuildChange(int guildDefID)
{
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 99);
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutInteger(&SendBuf[wpos], guildDefID);
	PutShort(&SendBuf[1], wpos - 3);
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: BroadcastShardChanged(void)
{
	if(IsGMInvisible() == true)
		return;

	// When this Simulator has changed shards, update all other players who
	// have this player on their friend list.  Notify them of the shard change.
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);

	wpos += PutByte(&SendBuf[wpos], 15); //Event to change shards
	wpos += PutStringUTF(&SendBuf[wpos], creatureInst->css.display_name);
	wpos += PutStringUTF(&SendBuf[wpos], pld.zoneDef->mShardName.c_str());
	PutShort(&SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->InternalID == InternalID)
			continue;

		if(it->isConnected == true && it->ProtocolState == 1)
		{
			if(it->pld.charPtr->GetFriendIndex(pld.CreatureDefID) >= 0)
				it->AttemptSend(SendBuf, wpos);
		}
	}
}

void SimulatorThread :: handle_game_query(void)
{
#ifdef DEBUG_TIME
	unsigned long startTime = g_PlatformTime.getMilliseconds();
#endif

	int PendingData = 0;
	query.Clear();  //Clear it here so that debug polls can fetch the most recent query.
	ReadQueryFromMessage();

	//LogMessageL(MSG_SHOW, "[DEBUG] handle_game_query:%s (ID:%d)", query.name.c_str(), query.ID);

	// Debug
	/*
	LogMessageL(MSG_SHOW, "Query: %d=%s", query.ID, query.name.c_str());
	for(int i = 0; i < query.argCount; i++)
		LogMessageL(MSG_SHOW, "  %d=%s", i, query.args[i].c_str());
	*/
	
	if(HandleQuery(PendingData) == false)
	{
		if(HandleCommand(PendingData) == false)
		{
			// See if the instance script will handle the command
			if(creatureInst != NULL && creatureInst->actInst != NULL && creatureInst->actInst->nutScriptPlayer != NULL) {
				std::vector<ScriptCore::ScriptParam> p;
				p.push_back(creatureInst->CreatureID);
				p.insert(p.end(), query.args.begin(), query.args.end());
				Util::SafeFormat(Aux1, sizeof(Aux1), "on_command_%s", query.name.c_str());
				if(creatureInst->actInst->nutScriptPlayer->RunFunction(Aux1, p, true)) {
					WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK.");
					PendingSend = true;
					return;
				}
			}

			LogMessageL(MSG_WARN, "[WARNING] Unhandled query in game: %s", query.name.c_str());
			for(int i = 0; i < query.argCount; i++)
				LogMessageL(MSG_WARN, "  [%d]=[%s]", i, query.args[i].c_str());

			WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "Unknown query.");
			PendingSend = true;
		}
	}

	if(PendingData > 0)
	{
		AttemptSend(SendBuf, PendingData);
		PendingSend = false;
	}

#ifdef DEBUG_TIME
	unsigned long passTime = g_PlatformTime.getMilliseconds() - startTime;
	if(passTime > 50)
	{
		LogMessageL(MSG_SHOW, "[DEBUG] TIME PASS handle_game_query() %d ms for query:%s (ID:%d)", passTime, query.name.c_str(), query.ID);
		for(int i = 0; i < query.argCount; i++)
			LogMessageL(MSG_SHOW, "  [%d]=%s", i, query.args[i].c_str());
	}

#ifdef DEBUG_PROFILER
	_DebugProfiler.AddQuery(query.name, passTime);
#endif

#endif
}

bool SimulatorThread :: HandleQuery(int &PendingData)
{
	if(query.name.compare("client.loading") == 0)
		handle_query_client_loading();
	else if(query.name.compare("scenery.list") == 0)
		PendingData = handle_query_scenery_list();
	else if(query.name.compare("pref.getA") == 0)
		handle_query_pref_getA();
	else if(query.name.compare("pref.get") == 0)
		handle_query_pref_get();
	else if(query.name.compare("pref.setA") == 0)
		handle_query_pref_setA();
	else if(query.name.compare("pref.set") == 0)
		handle_query_pref_set();
	else if(query.name.compare("creature.isusable") == 0)
		handle_query_creature_isusable();
	else if(query.name.compare("creature.def.edit") == 0)
		PendingData = handle_query_creature_def_edit();
	else if(query.name.compare("item.contents") == 0)
		PendingData = handle_query_item_contents();
	else if(query.name.compare("item.move") == 0)
		PendingData = handle_query_item_move();
	else if(query.name.compare("item.delete") == 0)
		PendingData = handle_query_item_delete();
	else if(query.name.compare("item.split") == 0)
		handle_query_item_split();
	else if(query.name.compare("admin.check") == 0)
		PendingData = handle_query_admin_check();
	else if(query.name.compare("account.fulfill") == 0)
		handle_query_account_fulfill();
	else if(query.name.compare("ab.remainingcooldowns") == 0)
		PendingData = handle_query_ab_remainingcooldowns();
	else if(query.name.compare("map.marker") == 0)
		PendingData = handle_query_map_marker();
	else if(query.name.compare("loot.list") == 0)
		handle_query_loot_list();
	else if(query.name.compare("loot.item") == 0)
		handle_query_loot_item();
	else if(query.name.compare("loot.exit") == 0)
		handle_query_loot_exit();
	else if(query.name.compare("creature.use") == 0)
		PendingData = handle_query_creature_use();
	else if(query.name.compare("account.info") == 0)
		PendingData = handle_query_account_info();
	else if(query.name.compare("statuseffect.set") == 0)
		PendingData = handle_query_statuseffect_set();
	else if(query.name.compare("friends.list") == 0)
		PendingData = handle_query_friends_list();
	else if(query.name.compare("friends.add") == 0)
		handle_query_friends_add();
	else if(query.name.compare("friends.remove") == 0)
		PendingData = handle_query_friends_remove();
	else if(query.name.compare("friends.status") == 0)
		handle_query_friends_status();
	else if(query.name.compare("friends.getstatus") == 0)
		handle_query_friends_getstatus();
	else if(query.name.compare("clan.info") == 0)
		PendingData = handle_query_clan_info();
	else if(query.name.compare("clan.list") == 0)
		PendingData = handle_query_clan_list();
	else if(query.name.compare("guild.info") == 0)
		PendingData = handle_query_guild_info();
	else if(query.name.compare("petition.send") == 0)
		PendingData = handle_query_petition_send();
	else if(query.name.compare("petition.list") == 0)
		PendingData = handle_query_petition_list();
	else if(query.name.compare("petition.doaction") == 0)
		PendingData = handle_query_petition_doaction();
	else if(query.name.compare("itemdef.contents") == 0)
		PendingData = handle_query_itemdef_contents();
	else if(query.name.compare("itemdef.delete") == 0)
		PendingData = handle_query_itemdef_delete();
	else if(query.name.compare("util.addFunds") == 0)
		PendingData = handle_query_util_addfunds();
	else if(query.name.compare("validate.name") == 0)
		PendingData = handle_query_validate_name();
	else if(query.name.compare("item.create") == 0)
		PendingData = handle_query_item_create();
	else if(query.name.compare("item.market.list") == 0)
		PendingData = handle_query_item_market_list();
	else if(query.name.compare("item.market.edit") == 0)
		PendingData = handle_query_item_market_edit();
	else if(query.name.compare("item.market.reload") == 0)
		PendingData = handle_query_item_market_reload();
	else if(query.name.compare("item.market.buy") == 0)
		PendingData = handle_query_item_market_buy();
	else if(query.name.compare("marker.list") == 0)
		PendingData = handle_query_marker_list();
	else if(query.name.compare("marker.edit") == 0)
		PendingData = handle_query_marker_edit();
	else if(query.name.compare("marker.del") == 0)
		PendingData = handle_query_marker_del();
	else if(query.name.compare("script.load") == 0)
		PendingData = handle_query_script_op(0);
	else if(query.name.compare("script.kill") == 0)
		PendingData = handle_query_script_op(1);
	else if(query.name.compare("script.run") == 0)
		PendingData = handle_query_script_op(2);
	else if(query.name.compare("script.save") == 0)
		PendingData = handle_query_script_op(3);
	else if(query.name.compare("guild.leave") == 0)
		PendingData = handle_query_guild_leave();
	else if(query.name.compare("scenery.edit") == 0)
		handle_query_scenery_edit();
	else if(query.name.compare("scenery.delete") == 0)
		handle_query_scenery_delete();
	else if(query.name.compare("item.def.use") == 0)
		handle_query_item_def_use();
	else if(query.name.compare("item.use") == 0)
		handle_query_item_use();
	else if(query.name.compare("ab.ownage.list") == 0)
		handle_query_ab_ownage_list();
	else if(query.name.compare("ab.buy") == 0)
		PendingData = handle_query_ab_buy();
	else if(query.name.compare("ab.respec") == 0)
		handle_query_ab_respec();
	else if(query.name.compare("buff.remove") == 0)
		PendingData = handle_query_buff_remove();
	else if(query.name.compare("util.ping") == 0)
		PendingData = handle_query_util_ping();
	else if(query.name.compare("util.pingsim") == 0)
		handle_query_util_pingsim();
	else if(query.name.compare("util.pingrouter") == 0)
		handle_query_util_pingrouter();
	else if(query.name.compare("essenceShop.contents") == 0)
		handle_query_essenceShop_contents();
	else if(query.name.compare("shop.contents") == 0)
		handle_query_shop_contents();
	else if(query.name.compare("trade.shop") == 0)
		PendingData = handle_query_trade_shop();
	else if(query.name.compare("trade.items") == 0)
		PendingData = handle_query_trade_items();
	else if(query.name.compare("trade.essence") == 0)
		PendingData = handle_query_trade_essence();
	else if(query.name.compare("visWeapon") == 0)
		PendingData = handle_query_visWeapon();
	else if(query.name.compare("quest.indicator") == 0)
		PendingData = handle_query_quest_indicator();
	else if(query.name.compare("quest.getquestoffer") == 0)
		PendingData = handle_query_quest_getquestoffer();
	else if(query.name.compare("quest.genericdata") == 0)
		PendingData = handle_query_quest_genericdata();
	else if(query.name.compare("quest.join") == 0)
		PendingData = handle_query_quest_join();
	else if(query.name.compare("quest.list") == 0)
		PendingData = handle_query_quest_list();
	else if(query.name.compare("quest.data") == 0)
		PendingData = handle_query_quest_data();
	else if(query.name.compare("quest.getcompletequest") == 0)
		PendingData = handle_query_quest_getcompletequest();
	else if(query.name.compare("quest.complete") == 0)
		PendingData = handle_query_quest_complete();
	else if(query.name.compare("quest.leave") == 0)
		PendingData = handle_query_quest_leave();
	else if(query.name.compare("quest.hack") == 0)
		PendingData = handle_query_quest_hack();
	else if(query.name.compare("henge.setDest") == 0)
		PendingData = handle_query_henge_setDest();
	else if(query.name.compare("portal.acceptRequest") == 0)
		PendingData = handle_query_portal_acceptRequest();
	else if(query.name.compare("trade.start") == 0)
		PendingData = handle_query_trade_start();
	else if(query.name.compare("trade.items") == 0)
		PendingData = handle_query_trade_items();
	else if(query.name.compare("trade.cancel") == 0)
		PendingData = handle_query_trade_cancel();
	else if(query.name.compare("trade.offer") == 0)
		PendingData = handle_query_trade_offer();
	else if(query.name.compare("trade.accept") == 0)
		PendingData = handle_query_trade_accept();
	else if(query.name.compare("trade.currency") == 0)
		PendingData = handle_query_trade_currency();
	else if(query.name.compare("craft.create") == 0)
		PendingData = handle_query_craft_create();
	else if(query.name.compare("item.morph") == 0)
		PendingData = handle_query_item_morph();
	else if(query.name.compare("shard.list") == 0)
		PendingData = handle_query_shard_list();
	else if(query.name.compare("sidekick.notifyexp") == 0)
		PendingData = handle_query_sidekick_notifyexp();
	else if(query.name.compare("ab.respec.price") == 0)
		PendingData = handle_query_ab_respec_price();
	else if(query.name.compare("util.version") == 0)
		PendingData = handle_query_util_version();
	else if(query.name.compare("persona.resCost") == 0)
		PendingData = handle_query_persona_resCost();
	else if(query.name.compare("spawn.list") == 0)
		PendingData = handle_query_spawn_list();
	else if(query.name.compare("spawn.create") == 0)
		PendingData = handle_query_spawn_create();
	else if(query.name.compare("creature.delete") == 0)
		PendingData = handle_query_creature_delete();
	else if(query.name.compare("build.template.list") == 0)
		PendingData = handle_query_build_template_list();
	else if(query.name.compare("spawn.property") == 0)
		PendingData = handle_query_spawn_property();
	else if(query.name.compare("spawn.emitters") == 0)
		PendingData = handle_query_spawn_emitters();
	else if(query.name.compare("scenery.link.add") == 0)
		PendingData = handle_query_scenery_link_add();
	else if(query.name.compare("scenery.link.del") == 0)
		PendingData = handle_query_scenery_link_del();
	else if(query.name.compare("ps.join") == 0)
		PendingData = handle_query_ps_join();
	else if(query.name.compare("ps.leave") == 0)
		PendingData = handle_query_ps_leave();
	else if(query.name.compare("persona.gm") == 0)
		PendingData = handle_query_persona_gm();
	else if(query.name.compare("party") == 0)
		PendingData = handle_query_party();
	else if(query.name.compare("vault.size") == 0)
		PendingData = handle_query_vault_size();
	else if(query.name.compare("vault.expand") == 0)
		PendingData = handle_query_vault_expand();
	else if(query.name.compare("vault.deliverycontents") == 0)
		PendingData = handle_query_vault_deliverycontents();
	else if(query.name.compare("quest.share") == 0)
		PendingData = handle_query_quest_share();
	else if(query.name.compare("mod.emote") == 0)
		PendingData = handle_query_mod_emote();
	else if(query.name.compare("mod.emotecontrol") == 0)
		PendingData = handle_query_mod_emotecontrol();
	else if(query.name.compare("mod.getpet") == 0)
		PendingData = handle_query_mod_getpet();
	else if(query.name.compare("mod.setgrovestart") == 0)
		PendingData = handle_query_mod_setgrovestart();
	else if(query.name.compare("mod.setenvironment") == 0)
		PendingData = handle_query_mod_setenvironment();
	else if(query.name.compare("mod.grove.togglecycle") == 0)
		PendingData = handle_query_mod_grove_togglecycle();
	else if(query.name.compare("mod.setats") == 0)
		PendingData = handle_query_mod_setats();
	else if(query.name.compare("mod.getURL") == 0)
		PendingData = handle_query_mod_getURL();
	/*
	else if(query.name.compare("mod.igforum.createcategory") == 0)
		PendingData = handle_query_mod_igforum_createcategory();
	*/
	else if(query.name.compare("mod.igforum.getcategory") == 0)
		PendingData = handle_query_mod_igforum_getcategory();
	else if(query.name.compare("mod.igforum.opencategory") == 0)
		PendingData = handle_query_mod_igforum_opencategory();
	else if(query.name.compare("mod.igforum.openthread") == 0)
		PendingData = handle_query_mod_igforum_openthread();
	else if(query.name.compare("mod.igforum.sendpost") == 0)
		PendingData = handle_query_mod_igforum_sendpost();
	else if(query.name.compare("mod.igforum.deletepost") == 0)
		PendingData = handle_query_mod_igforum_deletepost();
	/* OBSOLETE
	else if(query.name.compare("mod.igforum.deletethread") == 0)
		PendingData = handle_query_mod_igforum_deletethread();
	*/
	else if(query.name.compare("mod.igforum.setlockstatus") == 0)
		PendingData = handle_query_mod_igforum_setlockstatus();
	else if(query.name.compare("mod.igforum.setstickystatus") == 0)
		PendingData = handle_query_mod_igforum_setstickystatus();
	else if(query.name.compare("mod.igforum.editobject") == 0)
		PendingData = handle_query_mod_igforum_editobject();
	else if(query.name.compare("mod.igforum.deleteobject") == 0)
		PendingData = handle_query_mod_igforum_deleteobject();
	else if(query.name.compare("mod.igforum.runaction") == 0)
		PendingData = handle_query_mod_igforum_runaction();
	else if(query.name.compare("mod.igforum.move") == 0)
		PendingData = handle_query_mod_igforum_move();
	else if(query.name.compare("mod.itempreview") == 0)
		PendingData = handle_query_mod_itempreview();
	else if(query.name.compare("mod.restoreappearance") == 0)
		PendingData = handle_query_mod_restoreappearance();
	else if(query.name.compare("mod.pet.list") == 0)
		PendingData = handle_query_mod_pet_list();
	else if(query.name.compare("mod.pet.purchase") == 0)
		PendingData = handle_query_mod_pet_purchase();
	else if(query.name.compare("mod.pet.preview") == 0)
		PendingData = handle_query_mod_pet_preview();
	else if(query.name.compare("mod.ping.statistics") == 0)
		PendingData = handle_query_mod_ping_statistics();
	else if(query.name.compare("mod.craft") == 0)
		PendingData = handle_query_mod_craft();
	else if(query.name.compare("mod.getdungeonprofiles") == 0)
		PendingData = handle_query_mod_getdungeonprofiles();
	else if(query.name.compare("mod.morestats") == 0)
		PendingData = handle_query_mod_morestats();
	else if(query.name.compare("updateContent") == 0)
		PendingData = handle_query_updateContent();
	else if(query.name.compare("instance") == 0)
		PendingData = handle_query_instance();
	else if(query.name.compare("go") == 0)
		PendingData = handle_query_go();
	else if(query.name.compare("script.time") == 0)
		PendingData = handle_query_script_time();
	else if(query.name.compare("script.exec") == 0)
		PendingData = handle_query_script_exec();
	else if(query.name.compare("script.gc") == 0)
		PendingData = handle_query_script_gc();
	else {
		g_Log.AddMessageFormat("Unhandled query '%s'.", query.name.c_str());
		return false;
	}

	return true;
}

bool SimulatorThread :: HandleCommand(int &PendingData)
{
	if(query.name.compare("unstick") == 0)
		PendingData = handle_query_unstick();
	else if(query.name.compare("pose") == 0)
		PendingData = handle_command_pose();
	else if(query.name.compare("pose2") == 0)
		PendingData = handle_command_pose2();
	else if(query.name.compare("esay") == 0)
		PendingData = handle_command_esay();
	else if(query.name.compare("warp") == 0)
		PendingData = handle_command_warp();
	else if(query.name.compare("warpi") == 0)
		PendingData = handle_command_warpi();
	else if(query.name.compare("warpt") == 0)
		PendingData = handle_command_warpt();
	else if(query.name.compare("warpp") == 0)
		PendingData = handle_command_warpp();
	else if(query.name.compare("warpg") == 0)
		PendingData = handle_command_warpg();
	else if(query.name.compare("warpextoff") == 0)
		PendingData = handle_command_warpextoff();
	else if(query.name.compare("warpext") == 0)
		PendingData = handle_command_warpext();
	else if(query.name.compare("health") == 0)
		PendingData = handle_command_health();
	else if(query.name.compare("speed") == 0)
		PendingData = handle_command_speed();
	else if(query.name.compare("fa") == 0)
		PendingData = handle_command_fa();
	else if(query.name.compare("skadd") == 0)
		PendingData = handle_command_skadd();
	else if(query.name.compare("skremove") == 0)
		handle_command_skremove();
	else if(query.name.compare("skremoveall") == 0)
		handle_command_skremoveall();
	else if(query.name.compare("skattack") == 0)
		handle_command_skattack();
	else if(query.name.compare("skcall") == 0)
		handle_command_skcall();
	else if(query.name.compare("skwarp") == 0)
		handle_command_skwarp();
	else if(query.name.compare("skscatter") == 0)
		handle_command_skscatter();
	else if(query.name.compare("sklow") == 0)
		handle_command_sklow();
	else if(query.name.compare("skparty") == 0)
		handle_command_skparty();
	else if(query.name.compare("partylowest") == 0)
		PendingData = handle_command_partylowest();
	else if(query.name.compare("who") == 0)
		PendingData = handle_command_who();
	else if(query.name.compare("gmwho") == 0)
		PendingData = handle_command_gmwho();
	else if(query.name.compare("chwho") == 0)
		PendingData = handle_command_chwho();
	else if(query.name.compare("give") == 0)
		PendingData = handle_command_give();
	else if(query.name.compare("giveid") == 0)
		PendingData = handle_command_giveid();
	else if(query.name.compare("giveall") == 0)
		PendingData = handle_command_giveall();
	else if(query.name.compare("giveapp") == 0)
		PendingData = handle_command_giveapp();
	else if(query.name.compare("deleteall") == 0)
		PendingData = handle_command_deleteall();
	else if(query.name.compare("deleteabove") == 0)
		handle_command_deleteabove();
	else if(query.name.compare("grove") == 0)
		PendingData = handle_command_grove();
	else if(query.name.compare("pvp") == 0)
		PendingData = handle_command_pvp();
	else if(query.name.compare("complete") == 0)
		PendingData = handle_command_complete();
	else if(query.name.compare("refashion") == 0)
		PendingData = handle_command_refashion();
	else if(query.name.compare("backup") == 0)
		PendingData = handle_command_backup();
	else if(query.name.compare("restore1") == 0)
		PendingData = handle_command_restore();
	else if(query.name.compare("god") == 0)
		PendingData = handle_command_god();
	else if(query.name.compare("setstat") == 0)
		PendingData = handle_command_setstat();
	else if(query.name.compare("adjustexp") == 0)
		PendingData = handle_command_adjustexp();
	else if(query.name.compare("scale") == 0)
		PendingData = handle_command_scale();
	else if(query.name.compare("partyall") == 0)
		PendingData = handle_command_partyall();
	else if(query.name.compare("partyquit") == 0)
		PendingData = handle_command_partyquit();
	else if(query.name.compare("ccc") == 0)
		PendingData = handle_command_ccc();
	else if(query.name.compare("ban") == 0)
		PendingData = handle_command_ban();
	else if(query.name.compare("unban") == 0)
		PendingData = handle_command_unban();
	else if(query.name.compare("setpermission") == 0)
		PendingData = handle_command_setpermission();
	else if(query.name.compare("setbuildpermission") == 0)
		PendingData = handle_command_setbuildpermission();
	else if(query.name.compare("setpermissionc") == 0)
		PendingData = handle_command_setpermissionc();
	else if(query.name.compare("setbehavior") == 0)
		PendingData = handle_command_setbehavior();
	else if(query.name.compare("deriveset") == 0)
		PendingData = handle_command_deriveset();
	else if(query.name.compare("igstatus") == 0)
		PendingData = handle_command_igstatus();
	else if(query.name.compare("partyzap") == 0)
		PendingData = handle_command_partyzap();
	else if(query.name.compare("partyinvite") == 0)
		PendingData = handle_command_partyinvite();
	else if(query.name.compare("roll") == 0)
		PendingData = handle_command_roll();
	else if(query.name.compare("forumlock") == 0)
		PendingData = handle_command_forumlock();
	else if(query.name.compare("zonename") == 0)
		PendingData = handle_command_zonename();
	else if(query.name.compare("dtrig") == 0)
		PendingData = handle_command_dtrig();
	else if(query.name.compare("sdiag") == 0)
		PendingData = handle_command_sdiag();
	else if(query.name.compare("sping") == 0)
		PendingData = handle_command_sping();
	else if(query.name.compare("info") == 0)
		PendingData = handle_command_info();
	else if(query.name.compare("grovesetting") == 0)
		PendingData = handle_command_grovesetting();
	else if(query.name.compare("grovepermission") == 0)
		PendingData = handle_command_grovepermission();
	else if(query.name.compare("dngscale") == 0)
		PendingData = handle_command_dngscale();
	else if(query.name.compare("pathlinks") == 0)
		PendingData = handle_command_pathlinks();
	else if(query.name.compare("targ") == 0)
	{
		int wpos = 0;
		if(creatureInst->CurrentTarget.targ != NULL && CheckPermissionSimple(Perm_Account, Permission_Debug) == true)
		{
			int dist = ActiveInstance::GetPlaneRange(creatureInst, creatureInst->CurrentTarget.targ, 99999);
			sprintf(Aux1, "Name: %s (%d), ID: %d, CDef: %d (%d) (D:%d)", creatureInst->CurrentTarget.targ->css.display_name, creatureInst->CurrentTarget.targ->css.level, creatureInst->CurrentTarget.targ->CreatureID, creatureInst->CurrentTarget.targ->CreatureDefID, creatureInst->CurrentTarget.targ->css.health, dist);
			wpos += ::PrepExt_SendInfoMessage(SendBuf, Aux1, INFOMSG_INFO);
		}
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
		PendingData = wpos;
	}
	else if(query.name.compare("elev") == 0)
	{
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == true && query.argCount > 0)
		{
			creatureInst->CurrentY = query.GetInteger(0);
			creatureInst->BroadcastUpdateElevationSelf();
		}
		PendingData = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	else if(query.name.compare("cycle") == 0)
	{
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == true)
			g_EnvironmentCycleManager.EndCurrentCycle();
		PendingData = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	else if(query.name.compare("searsize") == 0)
		PendingData = handle_command_set_earsize();
	else if(query.name.compare("stailsize") == 0)
		PendingData = handle_command_set_tailsize();
	else
		return false;

	return true;
}

void SimulatorThread :: handle_inspectCreatureDef(void)
{
	int CDefID = GetInteger(&readPtr[ReadPos], ReadPos);

	g_CharacterManager.GetThread("SimulatorThread::handle_inspectCreatureDef");
	CharacterData *charData = g_CharacterManager.GetPointerByID(CDefID);
	g_CharacterManager.ReleaseThread();
	if(charData != NULL) {
		/* Is a player, is it for a player in the current active instance? If so, we need
		 * appearance modifiers so use the creature instance's calculated appearance.
		 *
		 * TODO Em - I don't really like this. It makes me wonder if modifiers should
		 * be on the creature def.
		 */
		CreatureInstance* cInst = creatureInst == NULL || creatureInst->actInst == NULL ? NULL : creatureInst->actInst->GetPlayerByCDefID(charData->cdef.CreatureDefID);
		if(cInst != NULL) {
			CreatureDefinition cd(charData->cdef);
			cd.css.SetAppearance(cInst->PeekAppearance().c_str());
			cd.css.SetEqAppearance(cInst->PeekAppearanceEq().c_str());
			AttemptSend(SendBuf, PrepExt_CreatureDef(SendBuf, &cd));


//			std::string currentAppearance = charData->cdef.css.appearance;
//			std::string currentEqAppearance = charData->cdef.css.eq_appearance;
//			charData->cdef.css.SetAppearance(cInst->PeekAppearance().c_str());
//			charData->cdef.css.SetEqAppearance(cInst->PeekAppearanceEq().c_str());
//
//			AttemptSend(SendBuf, PrepExt_CreatureDef(SendBuf, &charData->cdef));
//
//			charData->cdef.css.SetAppearance(currentAppearance.c_str());
//			charData->cdef.css.SetEqAppearance(currentEqAppearance.c_str());
		}
		else
			AttemptSend(SendBuf, PrepExt_CreatureDef(SendBuf, &charData->cdef));
	}
	else {
		CreatureDefinition *target = CreatureDef.GetPointerByCDef(CDefID);
		if(target != NULL)
			AttemptSend(SendBuf, PrepExt_CreatureDef(SendBuf, target));
		else
			LogMessageL(MSG_WARN, "[WARNING] inspectCreatureDef: could not find ID [%d]", CDefID);
	}
}

void SimulatorThread :: handle_inspectCreature(void)
{
	int CreatureID = GetInteger(&readPtr[ReadPos], ReadPos);

	CreatureInstance* cInst = creatureInst->actInst->inspectCreature(CreatureID);
	if(cInst != NULL)
	{
		int size = PrepExt_CreatureFullInstance(SendBuf, cInst);
		AttemptSend(SendBuf, size);
	}
}

void SimulatorThread :: AddMessage(long param1, long param2, int message)
{
	//Normal instance check, plus hack to get the new disconnect working.
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] AddMessage on NULL instance (msg: %d, param1: %d (%p), param2: %d (%p))", message, param1, param1, param2, param2);
		return;
	}


#ifdef DEBUG_TIME
	Debug::TimeTrack("SimulatorThread::AddMessage", 30);
#endif

	/*
	if(bcm.AddEvent2(InternalIndex, param1, param2, message, creatureInst->actInst) == -1)
		LogMessageL(MSG_ERROR, "[ERROR] Could not add BroadCast event.");
	*/
	MessageComponent msg;
	msg.SimulatorID = InternalIndex;
	msg.actInst = creatureInst->actInst;
	msg.param1 = param1;
	msg.param2 = param2;
	msg.message = message;
	msg.x = creatureInst->CurrentX;
	msg.z = creatureInst->CurrentZ;
	bcm.AddEventCopy(msg);
}

void SimulatorThread :: SendSetMap(void)
{
	if(creatureInst == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SendSetMap() creature instance is NULL");
		return;
	}
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SendSetMap() creature active instance is NULL");
		return;
	}

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 42);   //_handleEnvironmentUpdateMsg
	wpos += PutShort(&SendBuf[wpos], 0);

	wpos += PutByte(&SendBuf[wpos], 1);   //Mask

	wpos += PutStringUTF(&SendBuf[wpos], pld.CurrentZone);    //zoneID
	wpos += PutInteger(&SendBuf[wpos], pld.zoneDef->mID);      //zoneDefID
	wpos += PutShort(&SendBuf[wpos], pld.zoneDef->mPageSize);  //zonePageSize
	wpos += PutStringUTF(&SendBuf[wpos], pld.zoneDef->mTerrainConfig.c_str());   //Terrain
	std::string * tEnv = pld.zoneDef->GetTileEnvironment(creatureInst->CurrentX, creatureInst->CurrentZ);
	wpos += PutStringUTF(&SendBuf[wpos], tEnv->c_str());   //envtype
	wpos += PutStringUTF(&SendBuf[wpos], pld.zoneDef->mMapName.c_str());   //mapName

	strcpy(pld.CurrentEnv, tEnv->c_str());

	PutShort(&SendBuf[1], wpos - 3);       //Set message size
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: SetPosition(int xpos, int ypos, int zpos, int update)
{
	creatureInst->CurrentX = xpos;
	creatureInst->CurrentY = ypos;
	creatureInst->CurrentZ = zpos;

	if(update == 1)
	{
		pld.MovementBlockTime = g_ServerTime + g_Config.WarpMovementBlockTime;

		if(IsGMInvisible() == true)
		{
			int size = PrepExt_CreaturePos(SendBuf, creatureInst);
			size += PrepExt_GeneralMoveUpdate(&SendBuf[size],creatureInst);
			if(g_Config.UseStopSwim == true)
				size += PrepExt_ModStopSwimFlag(&SendBuf[size]);
			AttemptSend(SendBuf, size);
		}
		else
		{
			int size = PrepExt_UpdateFullPosition(SendBuf, creatureInst);
			if(g_Config.UseStopSwim == true)
				size += PrepExt_ModStopSwimFlag(&SendBuf[size]);
			creatureInst->actInst->LSendToLocalSimulator(SendBuf, size, creatureInst->CurrentX, creatureInst->CurrentZ);
		}

		int r = pld.charPtr->questJournal.CheckTravelLocations(creatureInst->CreatureID, Aux1, creatureInst->CurrentX, creatureInst->CurrentY, creatureInst->CurrentZ, pld.CurrentZoneID);
		if(r > 0)
			AttemptSend(Aux1, r);
	}
}

void SimulatorThread :: UpdateEqAppearance(void)
{
	pld.charPtr->UpdateEqAppearance();
	//Util::SafeCopy(creatureInst->css.eq_appearance, pld.charPtr->cdef.css.eq_appearance, sizeof(creatureInst->css.eq_appearance));
	creatureInst->css.SetEqAppearance(pld.charPtr->cdef.css.eq_appearance.c_str());

	int wpos = PrepExt_SendEqAppearance(SendBuf, pld.CreatureDefID, creatureInst->PeekAppearanceEq().c_str());
	creatureInst->BroadcastLocal(SendBuf, wpos);

	//Stats
	float oldHealthRatio = (float)creatureInst->css.health / (float)creatureInst->GetMaxHealth(true);

	creatureInst->RemoveStatModsBySource(BuffSource::ITEM);
	pld.charPtr->UpdateBaseStats(creatureInst, true);
	pld.charPtr->UpdateEquipStats(creatureInst);

	creatureInst->OnEquipmentChange(oldHealthRatio);
}

void SimulatorThread :: ActivatePassiveAbilities(void)
{
	Ability2::UniqueAbilityList uniqueList;
	const Ability2::AbilityEntry2 *abEntry = NULL;
	for(size_t i = 0; i < pld.charPtr->abilityList.AbilityList.size(); i++)
	{
		int id = pld.charPtr->abilityList.AbilityList[i];
		abEntry = g_AbilityManager.GetAbilityPtrByID(id);
		if(abEntry != NULL)
		{
			if(abEntry->IsPassive() == true)
				uniqueList.AddAbilityToList(abEntry);
		}
	}

	for(size_t i = 0; i < uniqueList.mAbilityList.size(); i++)
	{
		int abID = uniqueList.mAbilityList[i].mID;
		
		//Need request to fill the target data, then run activation.
		creatureInst->CallAbilityEvent(abID, EventType::onRequest);
		creatureInst->CallAbilityEvent(abID, EventType::onActivate);
		creatureInst->ab[0].Clear("ActivatePassiveAbilities");
	}
}

void SimulatorThread :: handle_query_client_loading(void)
{
	/* Query: client.loading
	   Args : [0] = status to notify, either "true" or "false"
	   Response: Standard query success "OK"
	*/

	if(query.argCount > 0)
	{
		if(query.args[0].compare("true") == 0)
			SetLoadingStatus(true, false);
		else
			SetLoadingStatus(false, false);
	}

	if(LoadStage == LOADSTAGE_LOADED)
		SendSetAvatar(creatureInst->CreatureID);

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	if(LoadStage == LOADSTAGE_LOADED)
	{
		if(creatureInst == NULL) {
			WritePos += PrepExt_SetMap(&SendBuf[WritePos], &pld, -1, -1);
		}
		else {
			WritePos += PrepExt_SetMap(&SendBuf[WritePos], &pld, creatureInst->CurrentX, creatureInst->CurrentZ);
		}
		LoadStage = LOADSTAGE_GAMEPLAY;

		if(pld.charPtr->PrivateChannelName.size() > 0)
			JoinPrivateChannel(pld.charPtr->PrivateChannelName.c_str(), pld.charPtr->PrivateChannelPassword.c_str());
	}
	if(LoadStage == LOADSTAGE_GAMEPLAY)  //Note: Every time the client triggers a load screen this will be resent.
	{
		std::vector<short> stats;
		stats.push_back(STAT::MOD_MOVEMENT);
		stats.push_back(STAT::BASE_MOVEMENT);
		//Seems to fix the 120% speed problem when first logging in
		WritePos += PrepExt_SendSpecificStats(&SendBuf[WritePos], creatureInst, stats);
		//pld.ResendSpeedTime = g_ServerTime + 2000;

		if(g_Config.DebugPingClient == true)
		{
			int mpos = WritePos;
			WritePos += PutByte(&SendBuf[WritePos], 100);   //_handleModMessage   REQUIRES MODDED CLIENT
			WritePos += PutShort(&SendBuf[WritePos], 0);    //Reserve for size
			WritePos += PutByte(&SendBuf[WritePos], MODMESSAGE_EVENT_PING_START); 
			WritePos += PutInteger(&SendBuf[WritePos], g_Config.DebugPingFrequency);
			PutShort(&SendBuf[mpos + 1], WritePos - mpos - 3);       //Set message size
		}
	}
	PendingSend = true;
}

int SimulatorThread :: handle_query_admin_check(void)
{
	/* Query: admin.check
	   Args : none
	   Response: Return an error if the account does not have admin permissions.

	   This is used to unlock some debug features in the client.
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_persona_gm(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: SetLoadingStatus(bool status, bool shutdown)
{
	//Called when a "client.loading" query is received.
	ClientLoading = status;

	// Process abnormal shutdown. This avoids sending the notification messages
	// below.  Disconnect() may call this function, this avoids pointer invalidation
	// and and yet another Disconnect() call below.
	if(shutdown == true)
	{
		if(LoadStage == LOADSTAGE_LOADING)
			LogMessageL(MSG_SHOW, "SetLoadingStatus() LOADSTAGE_LOADING when shutting down.");

		LoadStage = LOADSTAGE_UNLOADED;
		return;
	}

	if(ClientLoading == true)
	{
		if(LoadStage == LOADSTAGE_UNLOADED)
		{
			LoadStage = LOADSTAGE_LOADING;
			SendSetAvatar(creatureInst->CreatureID);
		}
	}
	else
	{
		if(LoadStage == LOADSTAGE_LOADING)
		{
			if(ValidPointers() == false)
			{
				ForceErrorMessage("Unexpected error when setting final load stage.", MSG_ERROR);
				sc.DisconnectClient();
				return;
			}
			//AddMessageG(SMSG_PlayerLogIn, (long)pld->charPtr, InternalIndex);
			//AddMessageG(SMSG_PlayerFriendLogState, (long)pld->charPtr, 1);
			if(CheckPermissionSimple(Perm_Account, Permission_Invisible) == false)
			{
				AddMessage((long)pld.charPtr, 0, BCM_PlayerLogIn);
				AddMessage((long)this, 1, BCM_PlayerFriendLogState);
				AddMessage((long)creatureInst, 0, BCM_UpdatePosition);
				/* This was originally added to try and solve the initial 120% speed issue that
				was sometimes present in the client.  Disabled here since it would override the
				new global speed increase.
				creatureInst->Speed = 0;
				creatureInst->css.mod_movement = 0;
				creatureInst->css.base_movement = 0;
				*/
								
				static const short stats[2] = {STAT::MOD_MOVEMENT, STAT::BASE_MOVEMENT };
				int wpos = PrepExt_SendSpecificStats(SendBuf, creatureInst, &stats[0], 2);
				AttemptSend(SendBuf, wpos);
			}

			g_PartyManager.CheckMemberLogin(creatureInst);
			AddMessage(0, 0, BCM_Notice_MOTD);
			LoadStage = LOADSTAGE_LOADED;  //Initial loading screen is finished, players should be able to control their characters.
		}
	}
}

int SimulatorThread :: handle_query_item_contents(void)
{
	/* Query: item.contents
	   Args : container name to retrieve
	*/

	if(HasQueryArgs(1) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	const char *contName = query.args[0].c_str();
	int contID = GetContainerIDFromName(contName);
	if(contID == -1)
	{
		LogMessageL(MSG_WARN, "WARNING: invalid [item.contents] container: [%s]", contName);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid item container.");
	}

	SendInventoryData(pld.charPtr->inventory.containerList[contID]);

	WritePos = 0;

	WritePos += PutByte(&SendBuf[WritePos], 1);       //_handleQueryResultMsg
	WritePos += PutShort(&SendBuf[WritePos], 0);      //Message size
	WritePos += PutInteger(&SendBuf[WritePos], query.ID);  //Query response index

	sprintf(Aux2, "%d", contID);
	sprintf(Aux3, "%d", (int)pld.charPtr->inventory.containerList[contID].size());
	WritePos += PutShort(&SendBuf[WritePos], 1);      //Array count
	WritePos += PutByte(&SendBuf[WritePos], 2);       //String count
	WritePos += PutStringUTF(&SendBuf[WritePos], Aux2);  //ID
	WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);  //Count
	PutShort(&SendBuf[1], WritePos - 3);             //Set message size

	return WritePos;
}

int SimulatorThread :: SendInventoryData(std::vector<InventorySlot> &cont)
{
	//NOTE: This function assumes that the inventory tables are not currently being edited.
	//Should not be assumed to be thread safe if other functions are adding/removing items.

	int count = (int)cont.size();
	int proc = 0;  //Current index being processed
	int tproc = 0; //Total processed;
	int batch = 0; //Count of the number of items contained in this batch of requests.
	int wpos = 0;  //Current Write position
	while(proc < count)
	{
		wpos += AddItemUpdate(&SendBuf[wpos], Aux3, &cont[proc]);
		proc++;
		batch++;
		tproc++;
		if(batch > 20)
		{
			AttemptSend(SendBuf, wpos);
			wpos = 0;
			batch = 0;
		}
	}
	if(batch > 0)
		AttemptSend(SendBuf, wpos);
	LogMessageL(MSG_DIAGV, "Sent %d item slots.", tproc);
	return tproc;
}

int SimulatorThread :: handle_query_scenery_list(void)
{
	/* Query: scenery.list
	   Args : 0=ZoneID, 1=TileX, 2=TileZ
	*/

	if(HasQueryArgs(3) == false)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	int zone = query.GetInteger(0);
	int x = query.GetInteger(1);
	int y = query.GetInteger(2);
	
	LogMessageL(MSG_SHOW, "[DEBUG] scenery.list: %d, %d, %d", zone, x, y);

	bool skipQuery = false;
	if(g_Config.ProperSceneryList == 0 || (CheckPermissionSimple(Perm_Account, Permission_FastLoad) == true))
		skipQuery = true;

	g_SceneryManager.GetThread("SimulatorThread::handle_query_scenery_list");
	std::list<int> excludedProps;

	/* If this is a request for the players current zone and active instance, retrieve the list
	 * of props to exclude (that may be the result of script prop removal)
	 */
	if(creatureInst != NULL && creatureInst->actInst != NULL && creatureInst->actInst->mZone == zone) {
		excludedProps.insert(excludedProps.begin(), creatureInst->actInst->RemovedProps.begin(), creatureInst->actInst->RemovedProps.end());
	}

	g_SceneryManager.AddPageRequest(sc.ClientSocket, query.ID, zone, x, y, skipQuery, excludedProps);
	g_SceneryManager.ReleaseThread();

	if(skipQuery == true)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	return 0;
}

void SimulatorThread :: handle_query_account_tracking(void)
{
	/* Query: account.tracking
	   Args : 0 = Seems to be a bit flag corresponding to the following
	          screens:
	          1 = Character Selection
	          2 = Character Creation Page 1 (Gender/Race/Body/Face)
	          4 = Character Creation Page 2 (Detail Colors/Size)
	          8 = Character Creation Page 3 (Class)
	          16 = Character Creation Page 4 (Name, Clothing)
	   Notes: Doesn't seem to be sent in 0.6.0 or 0.8.9, but is sent
	          in 0.8.6.  The proper response for this query is unknown.
	          The query is sent in any forward page changes, but only sends for
			  back pages from 4 to 16.
	*/
	int value = 0;
	if(query.argCount >= 1)
		value = atoi(query.args[0].c_str());
	if(value >= 2)
		characterCreation = true;
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_query_account_fulfill(void)
{
	/* Query: account.fulfill
	   Args : none
	   Notes: 0.8.9 seems to return zero
	*/
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "0");
	PendingSend = true;
}

int SimulatorThread :: handle_query_ab_remainingcooldowns(void)
{
	/* Query: ab.remainingcooldowns
	   Args : none
	*/
	//Each row sent back to the client must have:
	// [0] = category name, [1] = remaining time(ms), [2] = time elapsed(ms)
	MULTISTRING response;
	STRINGLIST row;
	for(size_t i = 0; i < creatureInst->cooldownManager.cooldownList.size(); i++)
	{
		ActiveCooldown *cd = &creatureInst->cooldownManager.cooldownList[i];
		long remain = cd->castEndTimeMS - g_ServerTime;
		if(remain > 0)
		{
			const char *cooldownName = g_AbilityManager.ResolveCooldownCategoryName(cd->category);
			if(cooldownName == NULL)
				continue;
			row.push_back(cooldownName);  // [0] = category name
			sprintf(Aux1, "%d", cd->GetRemainTimeMS());
			row.push_back(Aux1);
			sprintf(Aux1, "%d", cd->GetElapsedTimeMS());
			row.push_back(Aux1);
			response.push_back(row);
			row.clear();
		}
	}
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, response);
}

int SimulatorThread :: handle_query_map_marker(void)
{
	/* Query: map.marker
	   Args : at least 1
	          [0] = Zone ID
			  [...] typically a list, "Shop", "Vault", "QuestGiver", "Henge", "Sanctuary"
	*/
	//TODO: this doesn't yet handle all the client requests
	if(query.argCount < 1)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	int zoneID = strtol(query.args[0].c_str(), NULL, 10);
	if(zoneID != pld.CurrentZoneID)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	MULTISTRING qRes;
	for(int i = 1; i < query.argCount; i++)
	{
		if(query.args[i].compare("QuestGiver") == 0)
		{
			for(size_t qi = 0; qi < pld.charPtr->questJournal.availableQuests.itemList.size(); qi++)
			{
				QuestDefinition *qd = pld.charPtr->questJournal.availableQuests.itemList[qi].GetQuestPointer();
				if(qd == NULL)
					continue;
				if(qd->giverZone != pld.CurrentZoneID)
					continue;
				if(creatureInst->css.level < qd->levelMin)
					continue;
				if(qd->levelMax != 0)
					if(creatureInst->css.level > qd->levelMax)
						continue;
				if(abs(creatureInst->CurrentX - qd->giverX) > 1920)
					continue;
				if(abs(creatureInst->CurrentZ - qd->giverZ) > 1920)
					continue;
				if(qd->Requires > 0)
					if(pld.charPtr->questJournal.completedQuests.HasQuestID(qd->Requires) == -1)
						continue;

				// Guild start quest
				if(qd->guildStart && creatureInst->charPtr->IsInGuildAndHasValour(qd->guildId, 0))
					continue;

				// Guild requirements
				if(!qd->guildStart && qd->guildId != 0 && !creatureInst->charPtr->IsInGuildAndHasValour(qd->guildId, qd->valourRequired))
					continue;

				qRes.push_back(STRINGLIST());
				qRes.back().push_back(qd->title.c_str());
				Util::SafeFormat(Aux1, sizeof(Aux1), "(%d %d %d)", qd->giverX, qd->giverY, qd->giverZ);
//				LogMessageL(MSG_SHOW, "FOUND MARKER: %s", Aux1);
				qRes.back().push_back(Aux1);
				qRes.back().push_back("QuestGiver");
			}
		}
	}
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, qRes);
}

void SimulatorThread :: handle_updateVelocity(void)
{
	if(g_ServerTime < pld.MovementBlockTime)
	{
		creatureInst->Speed = 0;
		return;
	}
	//When entering a new zone from a portal, a velocity update always seems to be sent.
	//This is designed to block that message from interrupting the noncombat status.
	if(pld.IgnoreNextMovement == true)
		pld.IgnoreNextMovement = false;
	else
		creatureInst->RemoveNoncombatantStatus("updateVelocity");

	int x = GetShort(&readPtr[ReadPos], ReadPos);
	int z = GetShort(&readPtr[ReadPos], ReadPos);
	int y = GetShort(&readPtr[ReadPos], ReadPos);
	creatureInst->Heading = GetByte(&readPtr[ReadPos], ReadPos);
	creatureInst->Rotation = GetByte(&readPtr[ReadPos], ReadPos);
	int speed = GetByte(&readPtr[ReadPos], ReadPos);


	LogMessageL(MSG_WARN, "REMOVEME %d,%d,%d : heading %d rot: %d speed: %d", x, z, y, creatureInst->Heading, creatureInst->Rotation, speed);

	//LogMessageL(MSG_SHOW, "Heading:%d, Rot:%d, Spd:%d", creatureInst->Heading, creatureInst->Rotation, speed);

	/*
	int deltaY = creatureInst->CurrentY - y;
	if(deltaY > 30)
		pld.bFalling = true;
	if(pld.bFalling == true)
	{
		pld.DeltaY += deltaY;
		LogMessageL(MSG_SHOW, "Delta: %d, %d", deltaY, pld.DeltaY);
	}
	if(deltaY < 30)
	{
		if(pld.bFalling == true)
		{
			creatureInst->CheckFallDamage(pld.DeltaY);
			LogMessageL(MSG_SHOW, "Damage: %d", pld.DeltaY);
			pld.bFalling = false;
		}
		pld.DeltaY = 0;
	}
	*/

	if(g_Config.HasAdministrativeBehaviorFlag(ADMIN_BEHAVIOR_VERIFYSPEED) == true)
	{
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		{
			int xlen = abs(x - (creatureInst->CurrentX & 0xFFFF));
			int zlen = abs(z - (creatureInst->CurrentZ & 0xFFFF));
			if(xlen < OverflowThreshold && zlen < OverflowThreshold)
			{
				int pspeed = 120 + creatureInst->css.mod_movement;
				int distPerSecond = (int)(((float)pspeed / 100.0F) * 40.0F);
				//The number of units to move for this update.
				if(speed > pspeed || xlen > distPerSecond || zlen > distPerSecond)
				{
					LogMessageL(MSG_WARN, "SPEED WARNING: spd: %d / %d, xlen: %d, zlen: %d (%d)", speed, pspeed, xlen, zlen, distPerSecond);
					int size = PrepExt_GeneralMoveUpdate(SendBuf, creatureInst);
					AttemptSend(SendBuf, size);
					return;
				}
			}
		}
	}

	creatureInst->Speed = speed;

	int oldX = creatureInst->CurrentX;
	int oldZ = creatureInst->CurrentZ;
	int newX = oldX;
	int newZ = oldZ;

	//Since these updates are unsigned short, the maximum value they can relay
	//is 65535.  Some maps are much larger (Europe), thus some kind of overflow
	//handling must be supplied to prevent characters from having their
	//position reach 0. Check for an overflow by comparing the new location
	//with the last location.  If the distance is too great, a rollover
	//must've happened (although coordinate warping might be a possibility
	//too?) If the resulting short value is small, it means it overflowed
	//into a higher value.  If not, it underflowed into a smaller value.
	if(abs(x - (oldX & 0xFFFF)) >= OverflowThreshold)
	{
		if(x < OverflowThreshold)
			newX += OverflowAdditive;
		else
			newX -= OverflowAdditive;
	}
	if(abs(z - (oldZ & 0xFFFF)) >= OverflowThreshold)
	{
		if(z < OverflowThreshold)
			newZ += OverflowAdditive;
		else
			newZ -= OverflowAdditive;
	}

	//Zero out the lower short, then OR it back with the short coords we received from the client.
	newX = (newX ^ (newX & 0xFFFF)) | x;
	newZ = (newZ ^ (newZ & 0xFFFF)) | z;
	creatureInst->CurrentX = newX;
	creatureInst->CurrentY = y;
	creatureInst->CurrentZ = newZ;

	//LogMessageL(MSG_SHOW, "Loc: X:%d, Y:%d, Z:%d", x, y, z);
	//LogMessageL(MSG_SHOW, "Vel: Hd:%d, Rot:%d, Spd:%d", creatureInst->Heading, creatureInst->Rotation, creatureInst->Speed);


	if(creatureInst->Speed != 0)
	{
		//Check movement timers.
		int xlen = newX - oldX;
		int zlen = newZ - oldZ;
		float offset = sqrt((float)((xlen * xlen) + (zlen * zlen)));
		creatureInst->CheckMovement(offset);
		pld.TotalDistanceMoved += (int)offset;

		//Movement data verification against cheats.  Only run it for normal players.
		//If administrative /setstat command was used to increase speed above 255 (speed is 1 byte)
		//the expected velocity won't match.
		
		//The client uses a recurring event pulse to send the avatar's velocity updates to the server.
		//The pulse activates at 0.25 second intervals, regardless of whether movement occurred.
		//If the client's update flag is true (at least some movement happened), the update is sent
		//with the player's current position and rotation, then the update flag is cleared.  The
		//closed beta does not use any forced updates, although the client function allows that as
		//an optional parameter (forced updating would send the velocity update immediately,
		//independently from the update pulse, and would not trigger the update flag.
		
		//The pulse is not always accurate, and can vary with framerate.  Also, when first logging in
		//the pulse seems to be ignored, instead sending sporadic updates whenever required.  For example,
		//autorunning into a hill too steep to climb produces an indefinite flood of updates.  At some
		//currently unknown point, the pulse seems to take over and subsequent floods will stop.

		//There are tons of weird cases where verification fails on legit movement, so it may be necessary
		//to just disable it entirely.
				
		if((g_Config.VerifyMovement == true) && (CheckPermissionSimple(Perm_Account, Permission_Admin) == false))
		{
			int expectedSpeed = 100 + creatureInst->css.base_movement + creatureInst->css.mod_movement;
			
			//The heading and rotation fields use only a single byte (0 to 255) to represent 360 degree
			//rotation.  So 90 degrees (360/4) -> (255/4) = 64.
			//Usually if we're moving backward in a single direction (pressing Backward only)
			//then the difference is almost always ~128.  However, if we're moving Left or Right while
			//moving backward, sometimes heading and rotation is the same so we can't judge by that.
			//Instead we have to look at the incoming speed.  Since moving backward is half speed,
			//division may be off slightly so we allow a small tolerance.
			int halfExpected = abs(speed - (expectedSpeed / 2));
			if((speed != expectedSpeed) && (halfExpected > 3))
			{
				Debug::cheatLogger.Log("[SPEED] Unexpected speed by %s @ Zn:%d,X%d,Y%d, Spd:%d, Expected:%d", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, expectedSpeed);
				LogMessageL(MSG_SHOW, "[SPEED] Unexpected speed by %s @ Zn:%d,X%d,Y%d, Spd:%d, Expected:%d", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, expectedSpeed);
			}
			else
			{
				//Moving backwards, change to half speed for distance calculations.
				if((speed != expectedSpeed) && (halfExpected <= 3))
					expectedSpeed /= 2;
			}
			//Movement updates are typically every 250 milliseconds, but even on localhost can vary a bit.
			//Using the client, update intervals of ~218ms were semi common.
			unsigned long timeSinceLastMovement = g_ServerTime - pld.MovementTime;
			/* Removed: natural latency triggers this so often that it spams too many logs messages.
			if(timeSinceLastMovement < 200)
			{
				Debug::cheatLogger.Log("[SPEED] Rapid request by %s @ Zn:%d,X%d,Y%d, Spd:%d, Time:%lu", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, timeSinceLastMovement);
				LogMessageL(MSG_SHOW, "[SPEED] Rapid request by %s @ Zn:%d,X%d,Y%d, Spd:%d, Time:%lu", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, timeSinceLastMovement);
			}
			*/

			//We want to limit this to a quarter second so that we can calculate our expected
			//distance correctly, accepting lower update intervals but not large intervals which
			//would otherwise translate to potentially huge gaps with spoofed movement.
			//if(timeSinceLastMovement > 250)
				timeSinceLastMovement = 250;

			//Calculate how far the creature should have moved.  Default speed is 100, so
			//convert that into an expected distance while factoring the incoming time interval
			//and a 50% tolerance.
			float expectedOffset = ((expectedSpeed / 100.0F) * DEFAULT_CREATURE_SPEED) * (timeSinceLastMovement / 1000.0F) * 1.5F;

			// Disabled: was testing incoming movement for verification but it doesn't really help.
			/*
			Sleep(1);
			g_Log.AddMessageFormat("Offset:%g, Expected:%g", offset, expectedOffset);
			*/

			if(offset > expectedOffset)
			{
				Debug::cheatLogger.Log("[SPEED] Moved too far by %s @ Zn:%d,X%d,Z%d, Spd:%d, Offset X:%d,Z:%d  Moved:%g/Expected:%g", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, xlen, zlen, offset, expectedOffset);
				LogMessageL(MSG_SHOW, "[SPEED] Moved too far by %s @ Zn:%d,X%d,Z%d, Spd:%d, Offset X:%d,Z:%d  Moved:%g/Expected:%g", creatureInst->css.display_name, pld.CurrentZoneID, newX, newZ, speed, xlen, zlen, offset, expectedOffset);
			}
		}

		//Check our current position against any quest objective locations.
		//The movement counter will help us check every other step instead as a small optimization.
		if((pld.PendingMovement & 1) && (pld.charPtr != NULL))
		{
			int r = pld.charPtr->questJournal.CheckTravelLocations(creatureInst->CreatureID, Aux1, creatureInst->CurrentX, creatureInst->CurrentY, creatureInst->CurrentZ, pld.CurrentZoneID);
			if(r > 0)
				AttemptSend(Aux1, r);
		}

		if(pld.PendingMovement >= 10)
		{
			pld.PendingMovement = 0;
		}

	}


	//Check for zone boundaries, if a player is trying to go somewhere they should not be able to access.
	if(g_ZoneBarrierManager.CheckCollision(pld.CurrentZoneID, creatureInst->CurrentX, creatureInst->CurrentZ) == true)
		AddMessage((long)creatureInst, 0, BCM_UpdateFullPosition);

	//If GM invisible, send the update to ourself.  Otherwise broadcast it.
	if(IsGMInvisible() == true)
	{
		int wpos = PrepExt_GeneralMoveUpdate(SendBuf, creatureInst);
		AttemptSend(SendBuf, wpos);
	}
	else
	{
		AddMessage((long)creatureInst, 0, BCM_UpdateVelocity);
		AddMessage((long)creatureInst, 0, BCM_UpdatePosInc);
	}

	pld.PendingMovement++;
	pld.MovementTime = g_ServerTime;

	CheckSpawnTileUpdate(false);
	CheckMapUpdate(false);
}

void SimulatorThread :: handle_selectTarget(void)
{
	
	int targetID = GetInteger(&readPtr[ReadPos], ReadPos);
	if(targetID != 0)
		creatureInst->RemoveNoncombatantStatus("selectTarget");
	creatureInst->RequestTarget(targetID);
}

void SimulatorThread :: handle_abilityActivate(void)
{
	creatureInst->RemoveNoncombatantStatus("abilityActivate");

	// Flags appears to be intended for party casting, but isn't used outside the /do command.
	// We don't use it in the server, but we still need to read it from the message.
	short aID = GetShort(&readPtr[ReadPos], ReadPos);            //Ability ID

	// Unused?
	GetByte(&readPtr[ReadPos], ReadPos);
	//unsigned char flags = GetByte(&readPtr[ReadPos], ReadPos);   //flags

	unsigned char ground = GetByte(&readPtr[ReadPos], ReadPos);  //ground
	float x, y, z;

	//LogMessageL(MSG_SHOW, "abilityActivate: %d, flags: %d", aID, flags);

	if(ground != 0)
	{
		x = GetFloat(&RecBuf[ReadPos], ReadPos);
		y = GetFloat(&RecBuf[ReadPos], ReadPos);
		z = GetFloat(&RecBuf[ReadPos], ReadPos);
		creatureInst->ab[0].SetPosition(x, y, z);
		//LogMessageL(MSG_DIAGV, "ground: %d, x: %g, y: %g, z: %g", ground, x, y, z);
	}

	
	if(creatureInst->serverFlags & ServerFlags::IsTransformed)
	{
		const Ability2::AbilityEntry2 *abData = g_AbilityManager.GetAbilityPtrByID(aID);
		if(abData != NULL)
		{
			/* TODO: old junk, update or remove this
			if(abData->isMagic == true)
			{
				SendInfoMessage("You may not use elemental abilities while transformed.", INFOMSG_INFO);
				return;
			}
			if(abData->isRanged == true)
			{
				SendInfoMessage("You may not use ranged abilities while transformed.", INFOMSG_INFO);
				return;
			}
			*/
		}
	}

	if(aID == g_JumpConstant)
	{
		if(IsGMInvisible() == false)
			AddMessage(pld.CreatureID, 0, BCM_ActorJump);
	}
	else
	{
		bool allow = false;
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == true)
			allow = true;
		else if(pld.charPtr->abilityList.GetAbilityIndex(aID) >= 0)
			allow = true;
		else
		{
			if(g_AbilityManager.IsGlobalIntrinsicAbility(aID) == true)
				allow = true;
		}

		if(allow == true)
		{
			creatureInst->RequestAbilityActivation(aID);
			if(TargetRarityAboveNormal() == false)
			{
				if(pld.NotifyCast(creatureInst->CurrentX, creatureInst->CurrentZ, aID) == true)
				{
					CreatureInstance *target = creatureInst->CurrentTarget.targ;
					const char *targetName = "no target";
					int targetID = 0;
					if(target != NULL)
					{
						targetID = target->CreatureID;
						targetName = target->css.display_name;
					}
					Debug::cheatLogger.Log("[BOT] Attacks in area by %s @ %d:%d,%d(%d) (%d:%s) [%s:%d]",
						creatureInst->css.display_name,
						pld.CurrentZoneID, creatureInst->CurrentX, creatureInst->CurrentZ, creatureInst->Rotation,
						aID, g_AbilityManager.GetAbilityNameByID(aID),
						targetName, targetID);
				}
			}
		}
	}
}




int SimulatorThread :: handle_query_item_move(void)
{
	/*
	--- Some samples of server queries for different operations:
	Generally it follows this format:
	[0] = CCSID        Hex string formed by the Container ID and Container Slot,
	                   which can be extracted by bitmasks.
	[1] = inv          Target container name.
	[2] = 23           Target container slot.
	[3] = inventory    Current container name, varies depending on move operation.
	[4] = -1           Current container slot, varies depending on the container name
	                   and operation.

	When moving from an equipped slot to backpack slot:
	[0] = CCSID
	[1] = inv          (target container)
	[2] = 23           (target slot)
	[3] = inventory
	[4] = -1

	When moving from backpack slot to an equipped slot:
	[0] = CCSID
	[1] = eq           (target container)
	[2] = 3            (target slot, where slots are mapped to specific attachment points)
	[3] = inventory    (current container)
	[4] = 23           (current index)

	When moving between backpack slots:
	[0] = CCSID
	[1] = inv          (target container)
	[2] = 30           (target slot)
	[3] = inventory    (current container)
	[4] = 31           (current slot)

	When moving a weapon from backpack to equipped slot:
	[0] = 10002
	[1] = eq           (target container)
	[2] = 2            (target slot)
	[3] = eq_ranged    (unknown)
	[4] = -1           (unknown)

	When moving a weapon from equipped slot to backpack:
	[0] = 10002
	[1] = inv          (target container)
	[2] = 42           (target slot)
	[3] = eq_ranged    (unknown)
	[4] = 0            (unknown)

	*/

	/* Query: item.move
	   Args : [0] = Item ID (in hexadecimal)
	          [1] = Name of the destination container.
			  [2] = Destination slot.
			  [3] = Name of the source container (previous item location).
			  [4] = Source slot.
	*/

	//if(HasQueryArgs(5) == false)
	//	return;
	if(query.argCount < 5)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	/* Query debug output
	for(int i = 0; i < query.argCount; i++)
		LogMessageL(MSG_SHOW, "[%d]=%s", i, query.args[i].c_str());
	*/

	unsigned long CCSID = strtol(query.args[0].c_str(), NULL, 16);
	int origContainer = (CCSID & CONTAINER_ID) >> 16;
	int origSlot = CCSID & CONTAINER_SLOT;

	int destContainer = GetContainerIDFromName(query.args[1].c_str());
	if(destContainer == -1)
	{
		LogMessageL(MSG_ERROR, "[ERROR] item.move: unknown destination container [%s] for CCSID [%lu]", query.args[0].c_str(), CCSID);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Unknown destination container.");
	}

	if(CanMoveItems() == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You may not move items while busy.");

	//This is a pretty ugly hack to prevent some out of sync and potential duping issues.
	//If the player submits two items to trade, then removes one item from trade by dragging the
	//item and dropping it back into the inventory over the other item, it will swap the items and
	//allow that item to be submitted for trade twice, thus duping.
	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;
	TradeTransaction *tradeData = creatureInst->actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData != NULL)
	{
		creatureInst->actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You may not move items while trading.");
	}
	//End of hack.


	int destSlot = strtol(query.args[2].c_str(), NULL, 10);

	//Hack to enforce class restrictions for weapons.
	/* DEACTIVATED SINCE IT'S NOT SYNCHRONIZED WITH THE CLIENT
	if(destContainer == EQ_CONTAINER)
	{
		int origItemIndex = GetItemBySlot(origContainer, origSlot);
		if(origItemIndex >= 0)
		{
			InventorySlot &source = containerList[origContainer][origItemIndex];
			ItemDef *sourceItem = source.ResolveSafeItemPtr();
			if(sourceItem->mWeaponType != 0)
				if(creatureInst->ValidateEquippableWeapon(sourceItem->mWeaponType) == false)
					return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot equip weapons of that type.");
		}
	}
	*/

	//Need this or else accounts with debug permissions (multiple simultaneous logins per account)
	//will overwrite each other's vault contents. (Originally intended for shared vaults, which
	//implementation was never finished, but might as well keep)
	if(((destContainer == BANK_CONTAINER) || (origContainer == BANK_CONTAINER)) && (pld.accPtr->GetSessionLoginCount() > 1))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot use vaults while logged into multiple account characters at once.");

	if( (pld.charPtr->inventory.VerifyContainerSlotBoundary(origContainer, origSlot) == false) ||
		(pld.charPtr->inventory.VerifyContainerSlotBoundary(destContainer, destSlot) == false) )
	{
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to move item.");
	}

	if(destContainer == EQ_CONTAINER)
	{
		int origItemIndex = pld.charPtr->inventory.GetItemBySlot(origContainer, origSlot);
		if(origItemIndex < 0)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid item.");

		InventorySlot &source = pld.charPtr->inventory.containerList[origContainer][origItemIndex];
		ItemDef *sourceItem = source.ResolveItemPtr();
		int res = pld.charPtr->inventory.VerifyEquipItem(sourceItem, destSlot, creatureInst->css.level, creatureInst->css.profession);
		if(res != InventoryManager::EQ_ERROR_NONE)
			return PrepExt_QueryResponseError(SendBuf, query.ID, InventoryManager::GetEqErrorString(res));
	}

	int mpos = pld.charPtr->inventory.ItemMove(Aux1, Aux3, &creatureInst->css, pld.charPtr->localCharacterVault, origContainer, origSlot, destContainer, destSlot);
	if(mpos > 0)
	{
		AttemptSend(Aux1, mpos);
		//Send the query response after the item updates.
		int wpos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		AttemptSend(SendBuf, wpos);

		if(origContainer == EQ_CONTAINER || destContainer == EQ_CONTAINER)
		{
			//Just in case a container was equipped or unequipped, update the max inventory slots
			pld.charPtr->inventory.CountInventorySlots();

			//Update the eq_appearance string and broadcast an update to all other players
			UpdateEqAppearance();
		}

		/* DISABLED, NEVER FINISHED
		//Check quest objectives.
		if(origContainer == INV_CONTAINER || destContainer == INV_CONTAINER)
		{
			wpos = pld.charPtr->questJournal.RefreshItemQuests(pld.charPtr->inventory, SendBuf);
			if(wpos > 0)
				AttemptSend(SendBuf, wpos);
		}
		*/
		return 0;
	}

	//Ugly error code handling.
	if(mpos == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot equip a two-handed weapon while using an off-hand item.");
	else if(mpos == -2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot equip an offhand item while using a two-handed weapon.");
	else if(mpos == -3)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");  //Hack to allow movement when the origin and destination is the same.  The request must return successful, otherwise the client will spam retry requests.
	else if(mpos == -4)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Items bound to you cannot be placed into your vault.");
	else if(mpos < -100)
		return PrepExt_QueryResponseError(SendBuf, query.ID, InventoryManager::GetEqErrorString(mpos + 100));

	return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to move item.");
}

int SimulatorThread :: handle_query_item_delete(void)
{
	/* Query: item.delete
	   Args : [0] = Item ID (in hexadecimal)
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	unsigned long InventoryID = strtol(query.args[0].c_str(), NULL, 16);
	int origContainer = (InventoryID & CONTAINER_ID) >> 16;
	int origSlot = InventoryID & CONTAINER_SLOT;
	int r = pld.charPtr->inventory.GetItemBySlot(origContainer, origSlot);
	if(r == -1)
	{
		LogMessageL(MSG_ERROR, "Server error: Item ID not found [%lu]", InventoryID);
		return PrepExt_QueryResponseError(SendBuf, query.ID, LogBuffer);
	}

	int itemID = pld.charPtr->inventory.containerList[origContainer][r].IID;

	//NOTE: FOR DEBUG PURPOSES
	ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
	if(itemDef != NULL)
		LogMessageL(MSG_SHOW, "Deleting item %d [%s]", itemID, itemDef->mDisplayName.c_str());


	//Append a delete notice to the response string
	int wpos = 0;
	wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, &pld.charPtr->inventory.containerList[origContainer][r]);
	AttemptSend(SendBuf, wpos);

	//Remove from server's inventory list
	pld.charPtr->inventory.RemItem(InventoryID);

	//Just in case a container was equipped or unequipped, update the max inventory slots
	pld.charPtr->inventory.CountInventorySlots();

	//Update the eq_appearance string and broadcast an update to all other players
	UpdateEqAppearance();

	g_ItemManager.NotifyDestroy(itemID, "item.delete");

	return PrepExt_QueryResponseNull(SendBuf, query.ID);
}

void SimulatorThread :: handle_query_item_split(void)
{
	if(HasQueryArgs(2) == false)
		return;

	long CCSID = strtol(query.args[0].c_str(), NULL, 16);
	int amount = atoi(query.args[1].c_str());
	InventorySlot *item = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if(item == NULL)
	{
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "Item not found in inventory.");
		PendingSend = true;
		return;
	}

	int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if(slot == -1)
	{
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "No free inventory space.");
		PendingSend = true;
		return;
	}

	int currentCount = item->GetStackCount();
	if(amount < 1 || amount >= currentCount)
	{
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "Cannot split that amount.");
		PendingSend = true;
		return;
	}

	InventorySlot newItem;
	newItem.CopyFrom(*item, false);
	newItem.CCSID = pld.charPtr->inventory.GetCCSID(INV_CONTAINER, slot);
	newItem.count = amount - 1;

	int r = pld.charPtr->inventory.AddItem(INV_CONTAINER, newItem);
	if(r == -1)
	{
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID,  "Error creating new stack.");
		PendingSend = true;
		return;
	}

	//Need to refetch since the pointer might've changed when adding the item.
	item = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if(item == NULL)
	{
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID,  "Item not found in inventory.");
		PendingSend = true;
		return;
	}
	item->count -= amount;

	WritePos = AddItemUpdate(SendBuf, Aux1, item);
	WritePos += AddItemUpdate(&SendBuf[WritePos], Aux1, &newItem);
	WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_communicate(void)
{
	//For security and privacy reasons, we want to clear the buffers so that the buffer contents
	//are not visible in the debug report.
	OnExitFunctionClearBuffers fnCleanup(this);

	//Since this function merges two older functions, create some aliases
	//to make things easier to keep track of.
	GetStringUTF(&readPtr[ReadPos], Aux2, sizeof(Aux2), ReadPos);   //Channel (small buffer)
	GetStringUTF(&readPtr[ReadPos], Aux1, sizeof(Aux1), ReadPos);   //Message (large ubffer)

	Util::SanitizeClientString(Aux1);

	if(CheckPermissionSimple(Perm_Account, Permission_TrollChat) == true)
	{
		if(strcmp(Aux2, "rc/") == 0)
		{
			std::string messageCopy;
			if(Fun::oFunReplace.Replace(Aux1, messageCopy) == true)
				Util::SafeCopy(Aux1, messageCopy.c_str(), sizeof(Aux1));
		}
	}

	//Aliases to make the stuff below easier.
	const char *channel = Aux2;
	const char *message = Aux1;


	if(Aux1[0] == 0 || Aux2[0] == 0)
	{
		LogMessageL(MSG_WARN, "[WARNING] Invalid communication [%s][%s].", channel, message);
		return;
	}

	const ChannelCompare* channelInfo = GetChatInfoByChannel(Aux2);

	//Tell is a special case.  It's not in the scope list because the target
	//character is embedded into the channel text.

	bool tell = false;
	bool perm = true;
//	bool privateChannel = false;
	PrivateChannel* privateChannelData = NULL;
	if(strncmp(channel, "t/", 2) == 0)
	{
		tell = true;
		//Extract the target character name from the channel string into a variable
		//that will be used to compare against the names of currently logged characters.
		//The string might contain quotations around a character name, so they
		//must not be copied.
		size_t len = strlen(channel);

		//If for some strange reason the incoming name is too long, clip it down to
		//prevent a possible buffer overflow.
		if(len > sizeof(Aux3) - 1)
			len = sizeof(Aux3) - 1;

		if(channel[2] == '"' && channel[len - 1] == '"')
		{
			if(len - 4 < 0)
			{
				LogMessageL(MSG_CRIT, "[CRITICAL] invalid offset (%d)", len);
				return;
			}

			//strncpy(Aux3, &channel[3], len - 4);
			Util::SafeCopyN(Aux3, &channel[3], sizeof(Aux3), len - 4);
			Aux3[len - 4] = 0;
		}
		else
		{
			if(len - 2 < 0)
			{
				LogMessageL(MSG_CRIT, "[CRITICAL] invalid offset (%d)", len);
				return;
			}

			//strncpy(Aux3, &channel[2], len - 2);
			Util::SafeCopyN(Aux3, &channel[2], sizeof(Aux3), len - 2);
			Aux3[len - 2] = 0;
		}
	}
	else if(strncmp(channel, "ch/", 3) == 0)
	{
		//Format: ch/<channel>
		privateChannelData = g_ChatChannelManager.GetChannelForMessage(InternalID, channel + 3);
		if(privateChannelData == NULL)
		{
			SendInfoMessage("You are not registered in that channel.", INFOMSG_ERROR);
			return;
		}
//		privateChannel = true;
		channelInfo = GetChatInfoByChannel("ch/");
	}
	else
	{
		if(channelInfo->chatScope == CHAT_SCOPE_NONE)
		{
			LogMessageL(MSG_WARN, "[WARNING] Unknown chat channel: %s", channel);
			return;
		}
		if(strcmp(channel, "emote") == 0)
		{
			int size = pld.charPtr->questJournal.FilterEmote(creatureInst->CreatureID, SendBuf, message, creatureInst->CurrentX, creatureInst->CurrentZ, pld.CurrentZoneID);
			if(size > 0)
				AttemptSend(SendBuf, size);
		}
		if(strcmp(channel, "gm/earthsages") == 0)
			perm = CheckPermissionSimple(Perm_Account, Permission_GMChat);
		else if(strcmp(channel, "*SysChat") == 0)
			perm = CheckPermissionSimple(Perm_Account, Permission_SysChat);
		else if(strcmp(channel, "rc/") == 0)
			perm = CheckPermissionSimple(Perm_Account, Permission_RegionChat);
		else if(strcmp(channel, "tc/") == 0)
			perm = CheckPermissionSimple(Perm_Account, Permission_RegionChat);

		if(creatureInst->HasStatus(StatusEffects::GM_SILENCED))
		{
			SendInfoMessage("You are currently silenced by an Earthsage: you cannot use that channel.", INFOMSG_ERROR);
			return;
		}

		if(perm == false)
		{
			SendInfoMessage("Permission denied: you cannot use that channel.", INFOMSG_ERROR);
			return;
		}
	}

	//Compose the packet for the outgoing chat data

	const char *charName = pld.charPtr->cdef.css.display_name;
	std::string newCharName;
	if(chatHeader.size() != 0)
	{
		newCharName = chatHeader;
		newCharName.append(charName);
		newCharName.append(chatFooter);
		charName = newCharName.c_str();
	}
	if(g_Config.AprilFoolsAccount == pld.accPtr->ID)
	{
		if(g_Config.AprilFoolsName.size() > 0)
			charName = g_Config.AprilFoolsName.c_str();
	}

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 50);       //_handleCommunicationMsg
	wpos += PutShort(&SendBuf[wpos], 0);         //Placeholder for size
	wpos += PutInteger(&SendBuf[wpos], pld.CreatureID);    //Character ID who's sending the message
	wpos += PutStringUTF(&SendBuf[wpos], charName); //pld.charPtr->cdef.css.display_name);  //Character name
	wpos += PutStringUTF(&SendBuf[wpos], channel);
	wpos += PutStringUTF(&SendBuf[wpos], message);
	PutShort(&SendBuf[1], wpos - 3);     //Set size

	bool found = false;
	bool log = false;

	bool breakLoop = false;
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(breakLoop == true)
			break;

		if(it->ProtocolState == 0)
		{
			//LogMessageL(MSG_ERROR, "[WARNING] Cannot not send chat to lobby protocol simulator");
			continue;
		}

		if(it->isConnected == false)
			continue;

		if(it->pld.charPtr == NULL)
			continue;

		bool send = false;
		if(tell == true)
		{
			if(strcmp(it->pld.charPtr->cdef.css.display_name, Aux3) == 0)
			{
				send = true;
				found = true;
			}
		}
		else
		{
			switch(channelInfo->chatScope)
			{
			case CHAT_SCOPE_LOCAL:
				if(it->pld.CurrentInstanceID != pld.CurrentInstanceID)
					break;
				if(ActiveInstance::GetPlaneRange(it->creatureInst, creatureInst, LOCAL_CHAT_RANGE) > LOCAL_CHAT_RANGE)
					break;

				send = true;
				break;

			case CHAT_SCOPE_REGION:
				if(it->pld.CurrentInstanceID != pld.CurrentInstanceID)
					break;
				log = true;
				send = true;
				break;
			case CHAT_SCOPE_SERVER:
				log = true;
				send = true;
				break;
			case CHAT_SCOPE_FRIEND:
				if(it->pld.CreatureDefID == pld.CreatureDefID)  //Send to self
					send = true;
				//else if(g_SocialManager.pld.charPtr->GetFriendIndex(it->pld.CreatureDefID) >= 0)
				else if(g_FriendListManager.IsMutualFriendship(it->pld.CreatureDefID, pld.CreatureDefID) ==true)
					send = true;
				break;
			case CHAT_SCOPE_CHANNEL:
				for(size_t i = 0; i < privateChannelData->mMemberList.size(); i++)
					if(it->InternalID == privateChannelData->mMemberList[i].mSimulatorID)
						send = true;
				break;
			case CHAT_SCOPE_PARTY:
				g_PartyManager.BroadCastPacket(creatureInst->PartyID, creatureInst->CreatureDefID, SendBuf, wpos);
				breakLoop = true;
				break;
			case CHAT_SCOPE_CLAN:
				if(it->pld.CreatureDefID == pld.CreatureDefID)  //Send to self
					send = true;
				else if(g_GuildManager.IsMutualGuild(it->pld.CreatureDefID, pld.CreatureDefID) ==true)
					send = true;
				break;
			default:
				break;
			}
		}

		if(send == true)
			it->AttemptSend(SendBuf, wpos);
	}

	if(log == true)
	{
		if(channelInfo->prefix != NULL)
			Util::SafeFormat(LogBuffer, sizeof(LogBuffer), "%s %s: %s", channelInfo->prefix, pld.charPtr->cdef.css.display_name, message);
		else
			Util::SafeFormat(LogBuffer, sizeof(LogBuffer), "%s: %s", pld.charPtr->cdef.css.display_name, message);
		LogChatMessage(LogBuffer);
	}

	if(tell == true && found == false)
	{
		sprintf(LogBuffer, "Player \"%s\" is not logged in.", Aux3);
		SendInfoMessage(LogBuffer, INFOMSG_ERROR);
	}
}

bool SimulatorThread :: CheckPermissionSimple(int permissionSet, unsigned int permissionFlag)
{
	//Simple permission check.  Look to see if the given flag exists in the account
	//permissions.  Return true if it does, or false if it doesn't.
	//Intended to check for a specific permission or set of flags.
	if(pld.accPtr == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] CheckPermissionSimple accPtr is NULL");
		return false;
	}
	if(pld.charPtr == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] CheckPermissionSimple charPtr is NULL");
		return false;
	}

	if(pld.accPtr->HasPermission(permissionSet, permissionFlag) == true)
		return true;

	return pld.charPtr->HasPermission(permissionSet, permissionFlag);
}

int SimulatorThread :: handle_command_pose(void)
{
	//Requires a modded client to perform advanced actions.
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /pose emote [speed] [loop]");

	const char *emoteName = query.GetString(0);
	float emoteSpeed = 1.0F;
	int loop = 0;

	if(query.argCount > 1)
		emoteSpeed = query.GetFloat(1);
	if(query.argCount > 2)
		loop = query.GetInteger(2);

	if(emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	int wpos = 0;
	if(query.argCount == 1)
		wpos = PrepExt_GenericChatMessage(SendBuf, pld.CreatureID, pld.charPtr->cdef.css.display_name, "emote", query.args[0].c_str());
	else
	{
		wpos = PrepExt_SendAdvancedEmote(SendBuf, creatureInst->CreatureID, emoteName, emoteSpeed, loop);
		if(query.argCount >= 3 && loop == 0)
			wpos += PrepExt_SendEmoteControl(&SendBuf[wpos], creatureInst->CreatureID, 1);
	}

	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_pose2(void)
{
	//Requires a modded client to perform advanced actions.
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /pose2 emote [speed] [loop]");

	const char *emoteName = query.GetString(0);
	float emoteSpeed = 1.0F;
	int loop = 0;

	if(query.argCount > 1)
		emoteSpeed = query.GetFloat(1);
	if(query.argCount > 2)
		loop = query.GetInteger(2);

	if(emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	int targetID = ResolveEmoteTarget(1);

	int wpos = 0;
	if(query.argCount == 1)
		wpos = PrepExt_GenericChatMessage(SendBuf, targetID, pld.charPtr->cdef.css.display_name, "emote", query.args[0].c_str());
	else
	{
		wpos = PrepExt_SendAdvancedEmote(SendBuf, targetID, emoteName, emoteSpeed, loop);
		if(query.argCount >= 3 && loop == 0)
			wpos += PrepExt_SendEmoteControl(&SendBuf[wpos], targetID, 1);
	}

	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

//Dual command to emote and /say at the same time.
int SimulatorThread :: handle_command_esay(void)
{
	if(query.argCount == 0)
	{
		SendInfoMessage("Usage: /esay Emote \"optional local say message\"", INFOMSG_INFO);
	}
	else
	{
		const char *emoteName = query.GetString(0);
		const char *sayText = NULL;
		if(query.argCount > 1)
			sayText = query.GetString(1);

		int wpos = 0;
		wpos = PrepExt_GenericChatMessage(SendBuf, pld.CreatureID, pld.charPtr->cdef.css.display_name, "emote", emoteName);
		if(sayText != NULL)
			wpos += PrepExt_GenericChatMessage(&SendBuf[wpos], pld.CreatureID, pld.charPtr->cdef.css.display_name, "s", sayText);
		creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

const char * SimulatorThread :: GetGenericErrorString(int errCode)
{
	switch(errCode)
	{
	case ERROR_NONE: return "No error.";
	case ERROR_INVALIDZONE: return "Zone does not exist.";
	case ERROR_WARPGROVEONLY: return "Permission denied: you may only warp to other groves.";
	case ERROR_USERBLOCK: return "You are not allowed to enter that grove.";
	}
	return "Unknown error.";
};


int SimulatorThread :: CheckValidWarpZone(int ZoneID)
{
	//Determine whether the player is capable of warping to the target zone.  This involves checking
	//permissions of grove status or private lists.
	
	ZoneDefInfo *zonePtr = g_ZoneDefManager.GetPointerByID(ZoneID);
	if(zonePtr == NULL)
	{
		LogMessageL(MSG_SHOW, "[ERROR] Invalid ZoneID for warp: %d", ZoneID);
		return ERROR_INVALIDZONE;
	}

	//For administrative access
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == true)
		return ERROR_NONE;

	//For regular players
	if(zonePtr->mGrove == false && zonePtr->mArena == false && zonePtr ->mGuildHall)
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return ERROR_WARPGROVEONLY;

	//For guild hall
	if(zonePtr -> mGuildHall) {
		// Must be in guild
		GuildDefinition *gdef = g_GuildManager.GetGuildDefinitionForGuildHallZoneID(ZoneID);
		if(!pld.charPtr->IsInGuildAndHasValour(gdef->guildDefinitionID, 0))
		{
			LogMessageL(MSG_SHOW, "[ERROR] Not allowed to warp to guild hall unless in guild: %d", ZoneID);
			return ERROR_INVALIDZONE;
		}
		else
		{
			return ERROR_NONE;
		}
	}

	if(zonePtr->CanPlayerWarp(creatureInst->CreatureDefID, pld.accPtr->ID) == false)
		return ERROR_USERBLOCK;

	return ERROR_NONE;
}

int SimulatorThread :: handle_command_warp(void)
{
	/* Query: warp
	   Args : Optional types:
	     [0] = characterName 
		 [0] = direction [n|e|s|w]
		 [0] = direction [n|e|s|w], [1] = distance
		 [0] = XPos, [1] = Ypos
    */

	if(pld.zoneDef->mGrove == false)
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /warp x y [or] /warp \"Character Name\"");

	int zone = pld.CurrentZoneID;
	//int instance = pld.CurrentInstanceID;
	int instance = 0;  //Only set the instance if we need an explicit warp there.
	int xpos = creatureInst->CurrentX;
	int ypos = creatureInst->CurrentY;
	int zpos = creatureInst->CurrentZ;

	if(query.argCount >= 2)
	{
		const char *param1 = query.args[0].c_str();
		int param2 = atoi(query.args[1].c_str());

		//First check for relative directional positions before
		//processing a raw coordinate
		if(strcmp(param1, "n") == 0)
			zpos -= param2;
		else if(strcmp(param1, "s") == 0)
			zpos += param2;
		else if(strcmp(param1, "w") == 0)
			xpos -= param2;
		else if(strcmp(param1, "e") == 0)
			xpos += param2;
		else
		{
			//Coordinate warp
			xpos = atoi(param1);
			zpos = param2;
		}
	}
	else if(query.argCount == 1)
	{
		const char *target = query.args[0].c_str();
		if(strcmp(target, "n") == 0)
			zpos -= DefaultWarpDistance;
		else if(strcmp(target, "s") == 0)
			zpos += DefaultWarpDistance;
		else if(strcmp(target, "w") == 0)
			xpos -= DefaultWarpDistance;
		else if(strcmp(target, "e") == 0)
			xpos += DefaultWarpDistance;
		else if(strchr(target, ',') != NULL)
		{
			std::vector<std::string> args;
			Util::Split(query.args[0], ",", args);
			if(args.size() >= 2)
			{
				xpos = atoi(args[0].c_str());
				zpos = atoi(args[1].c_str());
				if(args.size() >= 3)
					zpos = atoi(args[2].c_str());   //Hack for x,y,z strings since they're often copy/pasted
			}
		}
		else
		{
			//Check names
			bool bFound = false;

			SIMULATOR_IT it;
			for(it = Simulator.begin(); it != Simulator.end(); ++it)
			{
				if(it->isConnected == true && it->ProtocolState == 1)
				{
					if(it->IsGMInvisible() == true)
						continue;
					if(strstr(it->pld.charPtr->cdef.css.display_name, target) != NULL)
					{
						zone = it->pld.CurrentZoneID;
						instance = it->pld.CurrentInstanceID;
						xpos = it->creatureInst->CurrentX;
						ypos = it->creatureInst->CurrentY;
						zpos = it->creatureInst->CurrentZ;
						bFound = true;
						break;
					}
				}
			}
			if(bFound == false)
			{
				Util::SafeFormat(Aux1, sizeof(Aux1), "Could not find target for warp: %s", target);
				SendInfoMessage(Aux1, INFOMSG_ERROR);
			}
		}
	}

	if(zone != pld.CurrentZoneID)
	{
		int errCode = CheckValidWarpZone(zone);
		if(errCode != ERROR_NONE)
			return PrepExt_QueryResponseError(SendBuf, query.ID, GetGenericErrorString(errCode));
	}

	DoWarp(zone, instance, xpos, ypos, zpos);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

//This function performs a warp.  Assumes that the target is a verified destination for the player.
void SimulatorThread :: DoWarp(int zoneID, int instanceID, int xpos, int ypos, int zpos)
{
	if(zoneID != pld.CurrentZoneID)
	{
		if(ProtectedSetZone(zoneID, instanceID) == false)
		{
			ForceErrorMessage("Critical error while changing zones.", INFOMSG_ERROR);
			Disconnect("SimulatorThread::handle_command_warp");
			return;
		}
	}
	if(xpos != creatureInst->CurrentX || ypos != creatureInst->CurrentY || zpos != creatureInst->CurrentZ || zoneID != pld.CurrentZoneID)
	{
		SendInfoMessage("Warping.", INFOMSG_INFO);
		SetPosition(xpos, ypos, zpos, 1);
		pld.LastMapTick = MapTickChange;
		CheckSpawnTileUpdate(true);
		CheckMapUpdate(true);
	}
}

int SimulatorThread :: handle_command_warpi(void)
{
	/*  Query: warpi
		Handles on-demand warping to instances.
		Args : 1, [0] = Instance Name
    */

	if(pld.zoneDef->mGrove == false)
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /warpi groveName");

	const char *warpTarg = query.args[0].c_str();
	ZoneDefInfo *targZone = g_ZoneDefManager.GetPointerByPartialWarpName(warpTarg);
	if(targZone == NULL)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "Zone name not found: %s", warpTarg);
		LogMessageL(MSG_ERROR, "%s", Aux1);
		SendInfoMessage(Aux1, INFOMSG_ERROR);
	}
	else
	{
		int errCode = CheckValidWarpZone(targZone->mID);
		if(errCode != ERROR_NONE)
			return PrepExt_QueryResponseError(SendBuf, query.ID, GetGenericErrorString(errCode));

		//If the avatar is running, it will glitch the position.

		// EM - Is this really needed?
//		SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);

		if(ProtectedSetZone(targZone->mID, 0) == false)
		{
			ForceErrorMessage("Critical error while changing zones.", INFOMSG_ERROR);
			Disconnect("SimulatorThread::handle_command_warpi");
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Critical error.");
		}
		SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_warpt(void)
{
	/*  Query: warpt
		Handles on-demand warping to a scenery tile coordinate within the instance.
		Args : 2, [0] = TileX, [1] = TileY
    */

	if(pld.zoneDef->mGrove == false)
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /warpt x y");

	int x = atoi(query.args[0].c_str());
	int z = atoi(query.args[1].c_str());

	int xtarg = Util::ClipInt(x, 0, 100) * pld.zoneDef->mPageSize;
	int ztarg = Util::ClipInt(z, 0, 100) * pld.zoneDef->mPageSize;

	SetPosition(xtarg, creatureInst->CurrentY, ztarg, 1);
	SendInfoMessage("Warping.", INFOMSG_INFO);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_warpp(void)
{
	/*  Query: warpp
		An external warp that pull a target directly to the player.
    */
	if(CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	CreatureInstance *targ = NULL;
	if(query.argCount == 0)
		targ = creatureInst->CurrentTarget.targ;
	else
		targ = creatureInst->actInst->GetPlayerByName(query.args[0].c_str());

	if(targ == NULL || targ == creatureInst)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Must select a target.");

	targ->CurrentX = creatureInst->CurrentX;
	targ->CurrentY = creatureInst->CurrentY;
	targ->CurrentZ = creatureInst->CurrentZ;

	AddMessage((long)targ, 0, BCM_UpdatePosition);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_warpg(void)
{
	const char *grove = NULL;
	if(query.argCount > 0)
		grove = query.GetString(0);

	if(grove == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "No grove specified.");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(grove);
	if(zoneDef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Grove not found.");
	if(zoneDef->mGrove == false && !zoneDef->mGuildHall)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Destination is not a grove.");

	int errCode = CheckValidWarpZone(zoneDef->mID);
	if(errCode != ERROR_NONE)
		return PrepExt_QueryResponseError(SendBuf, query.ID, GetGenericErrorString(errCode));

	int xLoc = 0;
	int zLoc = 0;
	if(query.argCount == 3)
	{
		xLoc = query.GetInteger(1);
		zLoc = query.GetInteger(2);
	}
	WarpToZone(zoneDef, xLoc, 0, zLoc);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_warpextoff(void)
{
	/*  Query: warpext
		Warp an external target to the player.  Used to set positions of offline characters.
		Args: [0] = Character Name [required]
		      [1] = Zone ID to warp the character to [required]
			  [2][3] = X and Z coordinate to warp [optional]
			  [4] = Y coordinate to warp [optional]
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /warpext \"Char Name\" zoneID [x z] [y]");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(atoi(query.args[1].c_str()));
	if(zoneDef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Zone not found.");

	CharacterData *cd = NULL;
	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.GetCharacterByName(query.args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if(cd != NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Character must be offline.");

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_warpext");
	int CDef = g_AccountManager.GetCDefFromCharacterName(query.args[0].c_str());
	g_AccountManager.cs.Leave();

	if(CDef == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Character name not found.");

	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.RequestCharacter(CDef, true);
	g_CharacterManager.ReleaseThread();

	if(cd == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to load character.");
	cd->ExtendExpireTime();

	int warpx = zoneDef->DefX;
	int warpy = zoneDef->DefY;
	int warpz = zoneDef->DefZ;
	if(query.argCount >= 4)
	{
		warpx = atoi(query.args[2].c_str());
		warpz = atoi(query.args[3].c_str());
	}
	if(query.argCount >= 5)
		warpy = atoi(query.args[4].c_str());

	cd->activeData.CurZone = zoneDef->mID;
	cd->activeData.CurX = warpx;
	cd->activeData.CurY = warpy;
	cd->activeData.CurZ = warpz;
	cd->SetExpireTime();

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_warpext(void)
{
	/*  Query: warpext2
		Warp an external target to a new zone ID.
		Args: [0] = Character Name [required]
		      [1] = Zone ID to warp the character to [required]
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /warpext \"Char Name\" zoneID");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(atoi(query.args[1].c_str()));
	if(zoneDef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Zone not found.");

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		if(it->ProtocolState == 1)
			if(strcmp(it->creatureInst->css.display_name, query.args[0].c_str()) == 0)
			{
				SendInfoMessage("Warping target.", INFOMSG_INFO);
				it->MainCallSetZone(zoneDef->mID, 0, true);
				return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
			}
	return PrepExt_QueryResponseError(SendBuf, query.ID, "Target not found."); 
}


int SimulatorThread :: handle_command_health(void)
{
	/*  Query: health
		Cheat to change your health on demand.
		Args : [0] = Hit Points to assign.
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /health hitPoints");

	int amount = atoi(query.args[0].c_str());
	int max = creatureInst->GetMaxHealth(true);
	Util::ClipInt(amount, 1, max);
	amount = amount & 0xFFFF;
	creatureInst->css.health = amount;
	AddMessage(pld.CreatureID, amount, BCM_SendHealth);

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	return WritePos;
}

int SimulatorThread :: handle_command_speed(void)
{
	/*  Query: health
		Cheat to change your speed on demand.
		Args : [0] = Base Speed.
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /speed bonusAmount");

	int amount = atoi(query.args[0].c_str());
	amount = Util::ClipInt(amount, 0, 0xFFFF);

	creatureInst->css.mod_movement = amount;
	AddMessage((long)creatureInst, 0, BCM_UpdateCreatureInstance);
	SendInfoMessage("Speed set.", INFOMSG_INFO);

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	return WritePos;
}

int SimulatorThread :: handle_command_fa(void)
{
	/*  Query: fa
		Cheat to force activation of an ability.  Bypasses client-enforced cooldowns.
		Args : [0] = ID of the ability to use.
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(HasQueryArgs(1) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Query error.");;

	int abID = 0;
	int ground = 0;
	abID = atoi(query.args[0].c_str());

	if(abID >= 26000 && abID <= 26100)
	{
		if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		{
			SendInfoMessage("Thou art not a deity.", INFOMSG_ERROR);
			abID = 0;
		}
	}

	if(abID > 0)
	{
		if(query.argCount == 2)
		{
			ground = atoi(query.args[1].c_str());
			if(ground > 0)
			{
				CreatureInstance *ptr = creatureInst->CurrentTarget.targ;
				if(ptr == NULL)
					ptr = creatureInst;
				creatureInst->ab[0].SetPosition(ptr->CurrentX, ptr->CurrentY, ptr->CurrentZ);
			}
		}
		else if(query.argCount >= 3)
		{
			int xoffset = atoi(query.args[1].c_str());
			int zoffset = atoi(query.args[2].c_str());

			creatureInst->ab[0].SetPosition(creatureInst->CurrentX + xoffset, creatureInst->CurrentY, creatureInst->CurrentZ + zoffset);
		}
		//AddMessage((long)creatureInst, abID, BCM_AbilityRequest);
		creatureInst->RequestAbilityActivation(abID);
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_command_skadd(void)
{
	/*  Query: skadd
		Cheat to add a sidekick.
		Args : 0 or 1, [0] = CreatureDefID
    */

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int CDefID = 0;
	if(query.argCount >= 1)
		CDefID = atoi(query.args[0].c_str());
	else
	{
		if(creatureInst->CurrentTarget.targ != NULL)
			CDefID = creatureInst->CurrentTarget.targ->CreatureDefID;
	}

	int wpos = 0;
	if(CDefID == 0)
		wpos = PrepExt_SendInfoMessage(SendBuf, "Usage: /skadd ID", INFOMSG_ERROR);
	else
		AddSidekick(CDefID);
	
	//UpdateSidekick(SidekickObject::PET, CDefID);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

void SimulatorThread :: handle_command_skremove(void)
{
	/*  Query: skremove
		Cheat to remove a sidekick (must be selected).
		Args : 0 or 1, [0] = CreatureDefID
    */

	if(creatureInst->CurrentTarget.targ == NULL)
	{
		WritePos = PrepExt_SendInfoMessage(SendBuf, "You must select a sidekick to remove.", INFOMSG_ERROR);
		WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
		PendingSend = true;
		return;
	}

	int r = creatureInst->actInst->SidekickRemoveOne(creatureInst, &pld.charPtr->SidekickList);

	WritePos = 0;
	if(r == -1)
		WritePos = PrepExt_SendInfoMessage(SendBuf, "Server error: Target is not a sidekick.", INFOMSG_ERROR);

	WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_skremoveall(void)
{
	/*  Query: skremoveall
		Cheat to remove all sidekicks.
		Args : none
    */
	creatureInst->actInst->SidekickRemoveAll(creatureInst, &pld.charPtr->SidekickList);
	WritePos = PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_skattack(void)
{
	creatureInst->RemoveNoncombatantStatus("skattack");

	AddMessage((long)creatureInst, 0, BCM_SidekickAttack);
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_skcall(void)
{
	AddMessage((long)creatureInst, 0, BCM_SidekickCall);
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_skwarp(void)
{
	AddMessage((long)creatureInst, 0, BCM_SidekickWarp);
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_skscatter(void)
{
	AddMessage((long)creatureInst, 0, BCM_SidekickScatter);
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_command_sklow(void)
{
	/*  Query: sklow
		Select the sidekick with the lowest health.
		Args : [0] = optional, health percentage threshold to search below.
    */
	int percent = 99;
	if(query.argCount > 0)
		percent = Util::ClipInt(atoi(query.args[0].c_str()), 1, 99);

	int r = creatureInst->actInst->SidekickLow(creatureInst, percent);

	WritePos = 0;
	if(r == 1)
	{
		//Success, the current target will have been changed to the sidekick
		//with the lowest percent health.
		//Send a target change notification back to the Simulator.
		WritePos = PrepExt_ChangeTarget(&SendBuf[WritePos], creatureInst->CreatureID, creatureInst->CurrentTarget.targ->CreatureID);
	}
	WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	PendingSend = true;
}


void SimulatorThread :: handle_command_skparty(void)
{
	/*  Query: skparty
		Create a virtual party with all sidekicks as members.  Virtual in this case means the health
		bars appear in the client, but there is no server side activity whatsoever.
		Args : none
    */
	WritePos = creatureInst->actInst->SidekickParty(creatureInst, SendBuf);

	WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	PendingSend = true;
}

int SimulatorThread :: handle_command_partylowest(void)
{
	float lowestHealthRatio = 1.0F;
	CreatureInstance *oldTarget = creatureInst->CurrentTarget.targ;
	CreatureInstance *newTarget = oldTarget;
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
	if(party != NULL)
	{
		for(size_t i = 0; i < party->mMemberList.size(); i++)
		{
			CreatureInstance *partyMember = creatureInst->actInst->GetPlayerByCDefID(party->mMemberList[i].mCreatureDefID);
			if(partyMember == NULL)
				continue;
			float healthRatio = partyMember->GetHealthRatio();
			if(healthRatio < lowestHealthRatio)
			{
				lowestHealthRatio = healthRatio;
				newTarget = partyMember;
			}
		}
		if(newTarget != oldTarget)
		{
			creatureInst->SelectTarget(newTarget);
			int wpos = PrepExt_ChangeTarget(SendBuf, pld.CreatureID, newTarget->CreatureID);
			AttemptSend(SendBuf, wpos);
		}
	}
	return PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
}

int SimulatorThread :: AddSidekick(int CDefID)
{
	int exist = pld.charPtr->CountSidekick(SidekickObject::PET);
	if(exist > 0)
	{
		SendInfoMessage("You already have a pet.", INFOMSG_INFO);
		return 0;
	}
	SidekickObject skobj(CDefID);
	skobj.summonType = SidekickObject::PET;

	pld.charPtr->AddSidekick(skobj);
	int r = creatureInst->actInst->CreateSidekick(creatureInst, skobj);
	if(r == -1)
		SendInfoMessage("Server error: Invalid Creature ID for sidekick.", INFOMSG_ERROR);
	return r;

	/*
	if((int)pld.charPtr->SidekickList.size() >= pld.charPtr->MaxSidekicks)
	{
		SendInfoMessage("You have reached your maximum sidekick limit.", INFOMSG_ERROR);
		return -1;
	}

	if(RegisterMainCallForQuery(true) == false)
		return -1;

	SidekickObject skobj(CDefID); //Code::Blocks wouldn't allow this as an argument below.
	int r = creatureInst->actInst->CreateSidekick(creatureInst, skobj, &pld.charPtr->SidekickList);
	UnregisterMainCall2();

	if(r == -1)
	{
		SendInfoMessage("Server error: Invalid Creature ID for sidekick.", INFOMSG_ERROR);
		return -1;
	}
	return 0;
	*/
}

void SimulatorThread :: AddPet(int CDefID)
{
	int exist = pld.charPtr->CountSidekick(SidekickObject::PET);
	if(exist > 0)
	{
		creatureInst->actInst->SidekickRemoveAll(creatureInst, &pld.charPtr->SidekickList);
		return;
	}

	SidekickObject skobj(CDefID);
	skobj.summonType = SidekickObject::PET;

	pld.charPtr->AddSidekick(skobj);
	int r = creatureInst->actInst->CreateSidekick(creatureInst, skobj);
	if(r == -1)
	{
		SendInfoMessage("Server error: Invalid Creature ID for sidekick.", INFOMSG_ERROR);
		return;
	}
}

int SimulatorThread :: handle_command_who(void)
{
	/*  Query: who
		Notify the client of a list of players currently logged in.
		Args : none
    */

	WritePos = 0;

	bool debug = CheckPermissionSimple(Perm_Account, Permission_Debug);
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true && it->ProtocolState == 1 && it->LoadStage == LOADSTAGE_GAMEPLAY)
		{
			if(it->IsGMInvisible() == true)  //Hide GM Invisibility from the list.
				continue;
			CharacterData *cd = it->pld.charPtr;
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(it->pld.CurrentZoneID);
			if(cd != NULL && zd != NULL)
			{
				if(debug == true)
					Util::SafeFormat(Aux1, sizeof(Aux1), "%s (%d, %d) %s (%s)", cd->cdef.css.display_name, it->creatureInst->CurrentX, it->creatureInst->CurrentZ, zd->mName.c_str(), zd->mShardName.c_str());
				else
					Util::SafeFormat(Aux1, sizeof(Aux1), "%s (%s.%d) %s (%s)", cd->cdef.css.display_name, Professions::GetAbbreviation(cd->cdef.css.profession), cd->cdef.css.level, zd->mName.c_str(), zd->mShardName.c_str());

				WritePos += PrepExt_SendInfoMessage(&SendBuf[WritePos], Aux1, INFOMSG_INFO);
				CheckWriteFlush(WritePos);
			}
		}
	}
	WritePos += PrepExt_QueryResponseString(&SendBuf[WritePos], query.ID, "OK");
	return WritePos;
}


// Advanced /who command for administrators shows account names and ID.  Useful for identifying alts
// or account info for other admin commands.
int SimulatorThread :: handle_command_gmwho(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int wpos = 0;
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true && it->ProtocolState == 1 && it->LoadStage == LOADSTAGE_GAMEPLAY)
		{
			CharacterData *cd = it->pld.charPtr;
			Util::SafeFormat(Aux1, sizeof(Aux1), "%s (%s, AID:%d)", cd->cdef.css.display_name, it->pld.accPtr->Name, it->pld.accPtr->ID);
			wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], Aux1, INFOMSG_INFO);
		}
	}
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_chwho(void)
{
	//Inform the client with a list of player names that this player has joined a private chat channel with.
	//Return a search of all channels, or if the optional channel name argument is supplied, that specific
	//channel.
	// Arguments: [0] = channel name (optional)
	const char *channelName = NULL;
	if(query.argCount > 1)
		channelName = query.GetString(1);

	//Retrieves a list of strings as raw output, including channel name.
	//The strings will be broadcast as a series of client info messages.
	STRINGLIST results;
	g_ChatChannelManager.EnumPlayersInMemberChannels(InternalID, channelName, results);
	int wpos = 0;
	for(size_t i = 0; i < results.size(); i++)
	{
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], results[i].c_str(), INFOMSG_INFO);
		CheckWriteFlush(wpos);
	}
	if(results.size() == 0)
	{
		const char *errorMsg = "You are not in a channel.";
		if(channelName != NULL)
		{
			Util::SafeFormat(Aux3, sizeof(Aux3), "You are not in channel [%s].", channelName);
			errorMsg = Aux3;
		}
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], errorMsg, INFOMSG_ERROR);
	}
	if(wpos > 0)
		AttemptSend(SendBuf, wpos);

	wpos = 0; //Reset to zero for correct query response.
	return PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
}

bool SimulatorThread :: CheckWriteFlush(int &curPos)
{
	if(curPos > Global::MAX_SEND_CHUNK_SIZE)
	{
		AttemptSend(SendBuf, curPos);
		curPos = 0;
		return true;
	}
	return false;
}

int SimulatorThread :: handle_command_give(void)
{
	/*  Query: give
		Cheat to give the player an item by name.
		Args : [0] = name of object to search for.
    */

	if(CheckPermissionSimple(Perm_Account, Permission_ItemGive) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /give \"Item Name\"");

	int wpos = 0;
	const char *itemName = query.GetString(0);

	ItemDef *item = g_ItemManager.GetSafePointerByPartialName(itemName);
	if(item->mID == 0)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "Item name not found [%s]", itemName);
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], Aux1, INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
		return wpos;
	}

	int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if(slot == -1)
	{
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "No free backpack slots", INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
		return wpos;
	}

	InventorySlot *sendSlot = pld.charPtr->inventory.AddItem_Ex(INV_CONTAINER, item->mID, 1);
	if(sendSlot != NULL)
		wpos += AddItemUpdate(&SendBuf[wpos], Aux1, sendSlot);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_giveid(void)
{
	/*  Query: vgive
		Cheat to give the player an item by ID.  Works for virtual items.
		NOTE: virtual items should not be duplicated.  If one is destroyed by any means, deleted
		from inventory, pushed off byback, etc, it will be removed from the database and the duplicates
		will be invalidated.
		Args : [0] = ID of object to give.
    */
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount == 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /giveid ItemID <count>");

	int itemID = query.GetInteger(0);
	int count = 1;
	if(query.argCount > 1)
		count = query.GetInteger(1);

	ItemDef *item = g_ItemManager.GetPointerByID(itemID);
	int wpos = 0;
	if(item == NULL)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "Item ID not found [%d]", itemID);
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], Aux1, INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
		return wpos;
	}

	int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if(slot == -1)
	{
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "No free backpack slots", INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
		return WritePos;
	}

	InventorySlot *sendSlot = pld.charPtr->inventory.AddItem_Ex(INV_CONTAINER, item->mID, count);
	if(sendSlot != NULL)
		wpos += AddItemUpdate(&SendBuf[wpos], Aux1, sendSlot);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_giveall(void)
{
	/*  Query: giveall
		Cheat to give the player all items (or as many as possible) that have
		a partial match with the given name.
		Args : [0] = name of object to search for.
    */

	if(CheckPermissionSimple(Perm_Account, Permission_ItemGive) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /giveall \"Search Name\"");

	const char *itemName = query.GetString(0);

	int slotremain = pld.charPtr->inventory.MaxContainerSlot[INV_CONTAINER] - pld.charPtr->inventory.containerList[INV_CONTAINER].size();
	if(slotremain <= 0)
	{
		SendInfoMessage("No free backpack slots", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	int wpos = 0;
	InventorySlot newItem;
	std::vector<ItemDef*> resultList;
	int found = g_ItemManager.EnumPointersByPartialName(itemName, resultList, slotremain);
	for(int i = 0; i < found; i++)
	{
		int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot >= 0)
		{
			slotremain--;
			newItem.dataPtr = resultList[i];
			newItem.IID = resultList[i]->mID;
			newItem.CCSID = (INV_CONTAINER << 16) | slot;
			pld.charPtr->inventory.AddItem(INV_CONTAINER, newItem);
			wpos += AddItemUpdate(&SendBuf[wpos], Aux3, &newItem);
		}
		else
		{
			LogMessageL(MSG_DIAGV, "No more free slots.");
			break;
		}
	}

	if(wpos > 0)
		AttemptSend(SendBuf, wpos);

	sprintf(Aux1, "Gave %d items.", found);
	SendInfoMessage(Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_giveapp(void)
{
	/*  Query: giveapp
		Cheat to give the player all items (or as many as possible) that have
		a partial match with the given equipment type and appearance string.
		Args : [0] = equip type ID
		       [1] = partial appearance string to match
    */

	if(CheckPermissionSimple(Perm_Account, Permission_ItemGive) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /giveapp equipType appString");

	int equipType = query.GetInteger(0);
	const char *searchName = query.GetString(1);

	int slotremain = pld.charPtr->inventory.MaxContainerSlot[INV_CONTAINER] - pld.charPtr->inventory.containerList[INV_CONTAINER].size();
	if(slotremain <= 0)
	{
		SendInfoMessage("No free backpack slots", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}

	int wpos = 0;
	InventorySlot newItem;
	std::vector<ItemDef*> resultList;
	int found = g_ItemManager.EnumPointersByPartialAppearance(searchName, resultList, slotremain);
	int given = 0;
	for(int i = 0; i < found; i++)
	{
		if((resultList[i]->mEquipType == equipType) || (equipType <= 0))
		{
			int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
			if(slot >= 0)
			{
				slotremain--;
				newItem.dataPtr = resultList[i];
				newItem.IID = resultList[i]->mID;
				newItem.CCSID = (INV_CONTAINER << 16) | slot;
				pld.charPtr->inventory.AddItem(INV_CONTAINER, newItem);
				wpos += AddItemUpdate(&SendBuf[wpos], Aux3, &newItem);
				given++;
			}
			else
			{
				LogMessageL(MSG_DIAGV, "No more free slots.");
				break;
			}
		}
	}

	if(wpos > 0)
		AttemptSend(SendBuf, wpos);

	sprintf(Aux1, "Gave %d items.", given);
	SendInfoMessage(Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_deleteall(void)
{
	/*  Query: deleteall
		Cheat to remove all items from the player's inventory.
		NOTE: This does not "destroy" the items so it is the only safe way to remove duplicate virtual
		items aside from manually removing them from a character file.
		Args : none
    */

	if(CheckPermissionSimple(Perm_Account, Permission_ItemGive) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int size = pld.charPtr->inventory.containerList[INV_CONTAINER].size();
	int a;
	int count = 0;
	WritePos = 0;
	for(a = 0; a < size; a++)
	{
		WritePos += RemoveItemUpdate(&SendBuf[WritePos], Aux1, &pld.charPtr->inventory.containerList[INV_CONTAINER][a]);
		count++;
		if(CheckWriteFlush(WritePos) == true)
			count = 0;
	}
	if(count > 0)
		AttemptSend(SendBuf, WritePos);

	pld.charPtr->inventory.containerList[INV_CONTAINER].clear();
	
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_query_loot_list(void)
{
	/*  Query: loot.list
		Retrieves a list of loot for a particular dead creature.
		Args : [0] = Creature Instance ID.
    */

	if(HasQueryArgs(1) == false)
		return;

	WritePos = 0;

	int CreatureID = atoi(query.args[0].c_str());

	int r = creatureInst->actInst->lootsys.GetCreature(CreatureID);
	if(r == -1)
		WritePos = PrepExt_QueryResponseString2(SendBuf, query.ID, "FAIL", "Creature does not have any loot.");
	else
		WritePos = creatureInst->actInst->lootsys.WriteLootQueryToBuffer(r, SendBuf, Aux2, query.ID);

	PendingSend = true;
}

int SimulatorThread :: protected_CheckDistance(int creatureID)
{
	return protected_CheckDistanceBetweenCreatures(creatureInst, creatureID);
}

int SimulatorThread :: protected_CheckDistanceBetweenCreatures(CreatureInstance *sourceCreatureInst, int creatureID)
{
	return protected_CheckDistanceBetweenCreaturesFor(sourceCreatureInst, creatureID, INTERACT_RANGE);
}

int SimulatorThread :: protected_CheckDistanceBetweenCreaturesFor(CreatureInstance *sourceCreatureInst, int creatureID, int range)
{
	if(sourceCreatureInst == NULL)
		return QueryErrorMsg::GENERIC;
	if(sourceCreatureInst->actInst == NULL)
		return QueryErrorMsg::GENERIC;

	CreatureInstance *object = sourceCreatureInst->actInst->GetNPCInstanceByCID(creatureID);
	if(object == NULL)
		return QueryErrorMsg::INVALIDOBJ;
	int dist = sourceCreatureInst->actInst->GetBoxRange(sourceCreatureInst, object);
	if(dist > range)
		return QueryErrorMsg::OUTOFRANGE;

	return 0;
}

int SimulatorThread :: protected_helper_query_loot_item(void)
{
	// 1. Free for all + Need or greed OFF. Loot will go to the looter
	// 2. Free for all + Need or greed ON. Loot will be offered to all, player is picked accordig to greed roll
	// 3. Round Robin + Need or greed OFF. Loot will be offered to robin, then party to greed or pass.
	// 4. Round Robin + Need or greed ON. Loot will be offered to robin, then party to need, greed or pass.
	// 5. Loot Master + Need or greed OFF. Loot will be offered to leader, then party to greed or pass.
	// 6. Loot Master + Need or greed ON. Loot will be offered to leader, then party to need, greed or pass.

	if(query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	int CID = atoi(query.args[0].c_str());
	int ItemID = atoi(query.args[1].c_str());
	//LogMessageL(MSG_SHOW, "loot.item: %d, %d", CID, ItemID);

	CreatureInstance *receivingCreature = creatureInst;
	CharacterData *charData = pld.charPtr;
	ItemDef *itemDef = g_ItemManager.GetPointerByID(ItemID);

	/* If the player is in a party, check the party loot rules.
	 */
	ActiveParty *party = g_PartyManager.GetPartyByID(receivingCreature->PartyID);
	bool partyLootable = party != NULL && (itemDef->mQualityLevel >= 2 || ( party->mLootFlags & MUNDANE ) > 0);
	bool needOrGreed = partyLootable && ( party->mLootFlags & NEED_B4_GREED ) > 0;

	ActiveInstance *aInst = receivingCreature->actInst;

	//The client uses creature definitions as lootable IDs.
	int PlayerCDefID = receivingCreature->CreatureDefID;

	// Make sure this object isn't too far away.
	int distCheck = protected_CheckDistance(CID);
	if(distCheck != 0)
		return distCheck;

	int r = aInst->lootsys.GetCreature(CID);
	if(r == -1)
		return QueryErrorMsg::LOOT;

	ActiveLootContainer *loot = &aInst->lootsys.creatureList[r];

	int canLoot = loot->HasLootableID(PlayerCDefID);
	if(canLoot == -1)
		return QueryErrorMsg::LOOTDENIED;

	int conIndex = loot->HasItem(ItemID);
	if(conIndex == -1)
		return QueryErrorMsg::LOOTMISSING;

	// Check if already waiting on rolls
	if(partyLootable && party->HasTags(PlayerCDefID, ItemID)) {
		LogMessageL(MSG_SHOW, "Denying loot. Already looting %d", ItemID);
		return QueryErrorMsg::LOOTDENIED;
	}

	// Offer the loot instead if appropriate
	bool offered = partyLootable && !( party->mLootMode == FREE_FOR_ALL && !needOrGreed ) && OfferLoot(party->mLootMode, loot, party, receivingCreature, ItemID, needOrGreed, CID, conIndex) > 0;

	if(!offered)
	{
		// Either there is no party, or the loot rules decided that the looter just gets the item

		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot == -1)
			return QueryErrorMsg::INVSPACE;

		InventorySlot *newItem = charData->inventory.AddItem_Ex(INV_CONTAINER, ItemID, 1);
		if(newItem == NULL)
			return QueryErrorMsg::INVCREATE;

		loot->RemoveItem(conIndex);
		if(loot->itemList.size() == 0)
		{
			CreatureInstance *lootCreature = aInst->GetNPCInstanceByCID(CID);
			if(lootCreature != NULL)
			{
				lootCreature->activeLootID = 0;
				lootCreature->css.ClearLootSeeablePlayerIDs();
				lootCreature->css.ClearLootablePlayerIDs();
				lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
				lootCreature->css.appearance_override = LootSystem::DefaultTombstoneAppearanceOverride;
				static const short statList[3] = {STAT::APPEARANCE_OVERRIDE, STAT::LOOTABLE_PLAYER_IDS, STAT::LOOT_SEEABLE_PLAYER_IDS};
				WritePos = PrepExt_SendSpecificStats(SendBuf, lootCreature, &statList[0], 3);
				aInst->LSendToLocalSimulator(SendBuf, WritePos, creatureInst->CurrentX, creatureInst->CurrentZ);
			}
			aInst->lootsys.RemoveCreature(CID);
		}

		WritePos = AddItemUpdate(SendBuf, Aux3, newItem);
	}

	// Always response to the looter
	STRINGLIST qresponse;
	qresponse.push_back("OK");
	sprintf(Aux3, "%d", conIndex);
	qresponse.push_back(Aux3);
	WritePos += PrepExt_QueryResponseStringList(&SendBuf[WritePos], query.ID, qresponse);

	return WritePos;
}

int SimulatorThread :: protected_helper_query_loot_need_greed_pass(void)
{
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
	if(party == NULL)
		return QueryErrorMsg::GENERIC;

	if(query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	ActiveInstance *aInst = creatureInst->actInst;
	int lootTag = atoi(query.args[1].c_str());
	LootTag *tag = party->lootTags[lootTag];
	if(tag == NULL)
	{
		LogMessageL(MSG_SHOW, "loot tag missing %d", lootTag);
		return QueryErrorMsg::INVALIDITEM;
	}
	ActiveLootContainer *loot = &aInst->lootsys.creatureList[tag->mLootCreatureId];
	if(tag == NULL)
	{
		LogMessageL(MSG_SHOW, "loot container missing %d", tag->mLootCreatureId);
		return QueryErrorMsg::INVALIDITEM;
	}
	LogMessageL(MSG_SHOW, "%d is choosing on %d (%d / %d)", creatureInst->CreatureID, lootTag, tag->mItemId, tag->mCreatureId, tag->mLootCreatureId);
	if(loot->HasAnyDecided(tag->mItemId, creatureInst->CreatureID))
	{
		LogMessageL(MSG_SHOW, "%d has already made loot decision on %d", creatureInst->CreatureID, tag->mItemId);
		return QueryErrorMsg::LOOTDENIED;
	}
	if(tag->mCreatureId != creatureInst->CreatureID)
	{
		LogMessageL(MSG_SHOW, "Loot tag %d is for a different creature. The tag said %d, but this player is %d.", lootTag, tag->mCreatureId, creatureInst->CreatureID);
		return QueryErrorMsg::LOOTDENIED;
	}

	const char *command = query.args[0].c_str();
	if(strcmp(command, "loot.need") == 0)	{
		loot->Need(tag->mItemId, tag->mCreatureId);
	}
	else if(strcmp(command, "loot.greed") == 0)	{
		loot->Greed(tag->mItemId, tag->mCreatureId);
	}
	else if(strcmp(command, "loot.pass") == 0)	{
		loot->Pass(tag->mItemId, tag->mCreatureId);
	}
	CheckIfLootReadyToDistribute(loot, tag);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_query_loot_need_greed_pass(void)
{
	/*  Query: loot.need
		Signals players interested in being needy.
		Args : [0] = Loot tag
    */

	if(HasQueryArgs(2) == false)
		return;

	WritePos = 0;
	WritePos = protected_helper_query_loot_need_greed_pass();
	if(WritePos <= 0)
		WritePos = PrepExt_QueryResponseString2(SendBuf, query.ID, "FAIL", GetErrorString(WritePos));
	PendingSend = true;
}

void SimulatorThread :: handle_query_loot_item(void)
{
	/*  Query: loot.item
		Loots a particular item off a creature.
		Args : [0] = Creature Instance ID
		       [1] = Item Definition ID
    */

	if(HasQueryArgs(2) == false)
		return;

	WritePos = 0;

	WritePos = protected_helper_query_loot_item();
	if(WritePos <= 0)
		WritePos = PrepExt_QueryResponseString2(SendBuf, query.ID, "FAIL", GetErrorString(WritePos));
	PendingSend = true;
}

void SimulatorThread :: handle_query_loot_exit(void)
{
	/*  Query: loot.exit
		Signals the server that the loot window has been closed.
		The server currently does not process this information.
    */
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

const char * SimulatorThread :: GetErrorString(int error)
{
	switch(error)
	{
	case QueryErrorMsg::GENERIC:
		return "Server error.";
	case QueryErrorMsg::INVSPACE:
		return "Not enough free inventory space.";
	case QueryErrorMsg::LOOT:
		return "Creature does not have any loot.";
	case QueryErrorMsg::LOOTDENIED:
		return "You cannot loot this creature.";
	case QueryErrorMsg::LOOTMISSING:
		return "That item no longer exists as loot.";
	case QueryErrorMsg::INVCREATE:
		return "Unable to create inventory item.";
	case QueryErrorMsg::INVALIDITEM:
		return "Item not found in server database.";
	case QueryErrorMsg::INVMISSING:
		return "You are missing a required item.";
	case QueryErrorMsg::INVALIDOBJ:
		return "Could not find object.";
	case QueryErrorMsg::OUTOFRANGE:
		return "Too far away to interact.";
	case QueryErrorMsg::COIN:
		return "You do not have enough coin.";
	case QueryErrorMsg::TRADENOTFOUND:
		return "You are not conducting a trade.";
	case QueryErrorMsg::TRADENOTOPENED:
		return "The other player has not accepted the trade request yet. Remove any items from the trade and try again.";
	case QueryErrorMsg::PROPNOEXIST:
		return "The prop could not be found.";
	case QueryErrorMsg::PROPLOCATION:
		return "Prop location is off limits.";
	case QueryErrorMsg::PROPATS:
		return "The prop references an invalid ATS.";
	case QueryErrorMsg::PROPOWNER:
		return "You may not edit props by other players.";
	case QueryErrorMsg::PROPASSETNULL:
		return "The prop cannot be assigned a blank asset name.";
	case QueryErrorMsg::TRADEBUSY:
		return "That player is busy with a trade.";
	case QueryErrorMsg::SELFBUSYSKILL:
		return "You cannot trade while casting spells or interacting with quest objects.";
	case QueryErrorMsg::OTHERBUSYSKILL:
		return "That player is busy performing a spell or interaction.";
	}
	return "Server error: undefined";
}

bool SimulatorThread :: HasQueryArgs(int minCount)
{
	if(query.argCount < minCount)
	{
		LogMessageL(MSG_WARN, "[WARNING] Expected more arguments for %s", query.name.c_str());
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		PendingSend = true;
		return false;
	}
	return true;
}

void SimulatorThread :: handle_query_creature_isusable(void)
{
	/*  Query: creature.isusable
		Determines whether a creature can be used by a specific player.
		Commonly indicated in the client with a shimmer effect and/or paw icon
		during mouse over.
		Args : [0] = Creature Instance ID
    */

	if(HasQueryArgs(1) == false)
		return;

	//Creatures cannot be used in groves, to prevent abuse of shops.
	if(creatureInst->actInst->mZoneDefPtr->mGrove == true)
	{
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "N");
		PendingSend = true;
		return;
	}

	WritePos = 0;

	WritePos = 0;
	int CID = query.GetInteger(0);
	int CDef = -1;
	CreatureInstance *target = creatureInst->actInst->GetNPCInstanceByCID(CID);
	bool failed = false;
	if(target != NULL)
	{
		CDef = target->CreatureDefID;
		if(target->serverFlags & ServerFlags::IsUnusable)
			failed = true;
	}
	else
		failed = true;

	if(failed == true)
	{
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "N");
		PendingSend = true;
		return;
	}

	//creatureInst->actInst->ResolveCreatureDef(CID, &CDef);

	int lootable = creatureInst->actInst->lootsys.GetCreature(CID);
	if(lootable > 0)
	{
		int self = creatureInst->actInst->lootsys.creatureList[lootable].HasLootableID(creatureInst->CreatureDefID);
		if(self >= 0)
			WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "Q");
	}
	else
	{
		const char *status = pld.charPtr->questJournal.CreatureIsusable(CDef);
		if(status[0] == 'N')
		{
			InteractObject *ptr = g_InteractObjectContainer.GetObjectByID(CDef, pld.CurrentZoneID);
			if(ptr != NULL)
			{
				bool hasReq = false;
				if(ptr->questReq != 0)
				{
					if(pld.charPtr->questJournal.completedQuests.HasQuestID(ptr->questReq) >= 0)
						hasReq = true;
					else
					{
						if(ptr->questComp == false)
							if(pld.charPtr->questJournal.activeQuests.HasQuestID(ptr->questReq) >= 0)
								hasReq = true;
					}
				}
				else
					hasReq = true;

				status = (hasReq == true) ? "Y" : "N";
			}


			CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(target->CreatureDefID);
			if(cdef != NULL && (cdef->DefHints & CDEF_HINT_USABLE))
				status = "Y";
			else if(cdef != NULL && (cdef->DefHints & CDEF_HINT_USABLE_SPARKLY))
				status = "Q";
			else if(target->HasStatus(StatusEffects::HENGE))
				status = "Y";
		}
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, status);
		//LogMessageL(MSG_SHOW, "  creature.isusable: %d (%d) = %s", CID, CDef, status);
	}

	if(WritePos == 0)
	{
		//LogMessageL(MSG_WARN, "[DEBUG] Unhandled creature.isusable for object ID:%d CDef:%d", CID, CDef);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "N");
	}
	PendingSend = true;
}

/*
  Check for build permission at the specified coordinate (in the player's current standing
  zone.  If a prop is supplied, use the coordinates of the prop instead.
*/
bool SimulatorThread :: HasPropEditPermission(SceneryObject *prop, float x, float z)
{
	/*
	if(pld.zoneDef->mGrove == true)
	{
		int accID = pld.zoneDef->mAccountID;
		int leaderID = 0;
		ActiveParty *party = NULL;
		if(creatureInst->PartyID != 0)
			party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
		if(party != NULL)
			leaderID = party->mLeaderDefID;
		AccountData *accPtr = g_AccountManager.GetActiveAccountByID(accID);
		if(leaderID != 0 && accPtr != NULL)
		{
			if(accPtr->HasCharacterID(leaderID) == true)
				return QueryErrorMsg::NONE;
		}
	}
	*/
	float checkX = x;
	float checkZ = z;
	if(prop != NULL)
	{
		checkX = prop->LocationX;
		checkZ = prop->LocationZ;
	}

	if(pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID, pld.zoneDef->mPageSize, checkX, checkZ) == true)
		return true;

	if(pld.zoneDef->HasEditPermission(pld.accPtr->ID, pld.CreatureDefID, creatureInst->css.display_name, checkX, checkZ) == true)
		return true;

	return false;
}

int SimulatorThread :: protected_helper_query_scenery_edit(void)
{
	if(query.argCount < 1)
		return QueryErrorMsg::GENERIC;

	if(pld.zoneDef == NULL)
		return QueryErrorMsg::GENERIC;

	SceneryObject prop;
	SceneryObject *propPtr = NULL;
	bool newProp = false;

	SceneryObject oldPropData;

	int PropID = 0;
	if(query.args[0].compare("NEW") != 0)
	{
		//If not a new prop, get the existing one.
		PropID = query.GetInteger(0);
		//LogMessageL(MSG_SHOW, "[DEBUG] scenery.edit: %d", PropID);

		g_SceneryManager.GetThread("SimulatorThread::protected_helper_query_scenery_edit");
		//propPtr = g_SceneryManager.GetPropPtr(pld.CurrentZoneID, PropID, NULL);
		propPtr = g_SceneryManager.GlobalGetPropPtr(pld.CurrentZoneID, PropID, NULL);
		g_SceneryManager.ReleaseThread();

		if(propPtr == NULL)
			return QueryErrorMsg::PROPNOEXIST;
		prop.copyFrom(propPtr);

		//Save the existing prop data in case there's an error
		oldPropData.copyFrom(&prop);

		if(HasPropEditPermission(&prop) == false)
		{
			int wpos = PrepExt_UpdateScenery(SendBuf, &oldPropData);
			AttemptSend(SendBuf, wpos);
			return QueryErrorMsg::PROPLOCATION;
		}
	}
	else
	{
		//New prop, give it an ID.
		newProp = true;
		prop.ID = g_SceneryVars.BaseSceneryID + g_SceneryVars.SceneryAdditive++;
		SessionVarsChangeData.AddChange();
		LogMessageL(MSG_SHOW, "[DEBUG] scenery.edit: (new) %d", prop.ID);
	}

	for(int i = 1; i < query.argCount; i += 2)
	{
		const char *field = query.args[i].c_str();
		const char *data = query.args[i + 1].c_str();

		if(strcmp(field, "asset") == 0)
			prop.SetAsset(data);
		else if(strcmp(field, "p") == 0)
		{
			prop.SetPosition(data);
		}
		else if(strcmp(field, "q") == 0)
		{
			prop.SetQ(data);
		}
		else if(strcmp(field, "s") == 0)
			prop.SetS(data);
		else if(strcmp(field, "flags") == 0)
			prop.Flags = atoi(data);
		else if(strcmp(field, "name") == 0)
			prop.SetName(data);
		else if(strcmp(field, "layer") == 0)
			prop.Layer = atoi(data);
		else if(strcmp(field, "patrolSpeed") == 0)
			prop.patrolSpeed = atoi(data);
		else if(strcmp(field, "patrolEvent") == 0)
			prop.SetPatrolEvent(data);
		else if(strcmp(field, "ID") == 0)
		{
			//Don't do anything for this, otherwise it might create a
			//duplicate entry.
			//prop.ID = atoi(data);
		}
		else if(prop.SetExtendedProperty(field, data) == true)
		{
			LogMessageL(MSG_SHOW, "scenery.edit extended property was set [%s=%s]", field, data);
		}
		else
			g_Log.AddMessageFormat("Unknown property [%s] for scenery.edit", field);
	}

	if(strlen(prop.Asset) == 0)
		return QueryErrorMsg::PROPASSETNULL;

	//The client makes changes internally without waiting for confirmation.
	//If the prop edit fails, we need to send back the old prop data to force
	//the client to revert to its prior state.

	//if(pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID, pld.zoneDef->mPageSize, prop.LocationX, prop.LocationZ) == false)
	if(HasPropEditPermission(&prop) == false)
	{
		if(newProp == false)
		{
			int wpos = PrepExt_UpdateScenery(SendBuf, &oldPropData);
			AttemptSend(SendBuf, wpos);
		}
		return QueryErrorMsg::PROPLOCATION;
	}

	//Check valid asset
	if(g_SceneryManager.VerifyATS(prop) == false)
	{
		if(newProp == false)
		{
			int wpos = PrepExt_UpdateScenery(SendBuf, &oldPropData);
			AttemptSend(SendBuf, wpos);
		}
		return QueryErrorMsg::PROPATS;
	}

	//Prop is good to add.
	g_SceneryManager.GetThread("SimulatorThread::protected_helper_query_scenery_edit");

	bool isSpawnPoint = prop.IsSpawnPoint();

	SceneryObject *retProp = NULL;
	if(PropID == 0)
	{
		retProp = g_SceneryManager.AddProp(pld.zoneDef->mID, prop);
		//LogMessageL(MSG_SHOW, "Added prop: %d, %s", prop.ID, prop.Asset);
	}
	else
	{
		if(isSpawnPoint == true)
			creatureInst->actInst->spawnsys.RemoveSpawnPoint(prop.ID);

		retProp = g_SceneryManager.ReplaceProp(pld.zoneDef->mID, prop);
		//LogMessageL(MSG_SHOW, "Replaced prop: %d, %s", prop.ID, prop.Asset);
	}

	// The spawn system needs to reference a stable prop location.  If it tries to
	// reference this prop, it will crash after this local instance is destructed.
	// So we use retProp instead, which returns with a valid pointer if AddProp()
	// or ReplaceProp() was successful.
	if(isSpawnPoint == true && retProp != NULL)
		creatureInst->actInst->spawnsys.UpdateSpawnPoint(retProp);

	g_SceneryManager.ReleaseThread();

	int opType = ((newProp == true) ? SceneryAudit::OP_NEW : SceneryAudit::OP_EDIT);
	pld.zoneDef->AuditScenery(creatureInst->css.display_name, pld.CurrentZoneID, &prop, opType);
	
	int wpos = PrepExt_UpdateScenery(SendBuf, &prop);
	//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_query_scenery_edit(void)
{
	/*  Query: scenery.edit
		Creates a new scenery object or edits an existing one.
		Args : [variable]
    */

	WritePos = 0;

	WritePos = protected_helper_query_scenery_edit();
	if(WritePos <= 0)
	{
		SendInfoMessage(GetErrorString(WritePos), INFOMSG_ERROR);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	PendingSend = true;
}

void SimulatorThread :: handle_query_scenery_delete(void)
{
	WritePos = 0;

	WritePos = protected_helper_query_scenery_delete();
	if(WritePos <= 0)
	{
		SendInfoMessage(GetErrorString(WritePos), INFOMSG_ERROR);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	PendingSend = true;
}

int SimulatorThread :: protected_helper_query_scenery_delete(void)
{
	if(query.argCount < 1)
		return QueryErrorMsg::GENERIC;

	int PropID = query.GetInteger(0);
	SceneryObject *propPtr = NULL;

	LogMessageL(MSG_SHOW, "[DEBUG] scenery.delete: %d", PropID);

	g_SceneryManager.GetThread("SimulatorThread::protected_helper_query_scenery_delete");
	//propPtr = g_SceneryManager.pageData.GlobalFindProp(PropID, pld.CurrentZoneID, NULL);
	propPtr = g_SceneryManager.GlobalGetPropPtr(pld.CurrentZoneID, PropID, NULL);
	g_SceneryManager.ReleaseThread();

	if(propPtr == NULL)
		return QueryErrorMsg::PROPNOEXIST;

	if(HasPropEditPermission(propPtr) == false)
		return QueryErrorMsg::PROPLOCATION;
	/* OLD
	if(pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID, pld.zoneDef->mPageSize, propPtr->LocationX, propPtr->LocationZ) == false)
		return QueryErrorMsg::PROPLOCATION;
	*/

	pld.zoneDef->AuditScenery(creatureInst->css.display_name, pld.CurrentZoneID, propPtr, SceneryAudit::OP_DELETE);

	//Spawn point must be deactivated before it is deleted, otherwise pointers
	//will be invalidated and it may crash.
	creatureInst->actInst->spawnsys.RemoveSpawnPoint(PropID);

	g_SceneryManager.GetThread("SimulatorThread::protected_helper_query_scenery_delete");
	//g_SceneryManager.pageData.DeleteProp(pld.CurrentZoneID, PropID);
	g_SceneryManager.DeleteProp(pld.CurrentZoneID, PropID);
	g_SceneryManager.ReleaseThread();

	//Generate a bare prop that only has the necessary data for a delete
	//operation.
	SceneryObject prop;
	prop.ID = PropID;
	prop.Asset[0] = 0;
	int wpos = PrepExt_UpdateScenery(SendBuf, &prop);
	//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_friends_list(void)
{
	/*  Query: friends.list
		Retrieves the friend list, with information like name, level, shard, etc.
		Args : [none]
    */

	if(pld.zoneDef == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] HandleQuery_friends_list() ZoneDef is NULL");
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	}

	FriendListManager::SEARCH_INPUT search;
	FriendListManager::SEARCH_OUTPUT resList;
	for(size_t i = 0; i < pld.charPtr->friendList.size(); i++)
		search.push_back(FriendListManager::SEARCH_PAIR(pld.charPtr->friendList[i].CDefID, pld.charPtr->friendList[i].Name));
	g_FriendListManager.EnumerateFriends(search, resList);

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);           //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);
	
	wpos += PutShort(&SendBuf[wpos], resList.size());   //Array count
	for(size_t i = 0; i < resList.size(); i++)
	{
		wpos += PutByte(&SendBuf[wpos], 6);  //String count

		//Character Name
		wpos += PutStringUTF(&SendBuf[wpos], resList[i].name.c_str());

		//Level
		sprintf(Aux1, "%d", resList[i].level);
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);

		//Profession (integer)
		sprintf(Aux1, "%d", resList[i].profession);
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);

		//Online ("true", "false")
		wpos += PutStringUTF(&SendBuf[wpos], (resList[i].online == true) ? "true" : "false");

		//Status Message
		wpos += PutStringUTF(&SendBuf[wpos], resList[i].status.c_str());

		//Shard
		wpos += PutStringUTF(&SendBuf[wpos], resList[i].shard.c_str());
	}
	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}

void SimulatorThread :: handle_query_friends_add(void)
{
	/*  Query: friends.add
		Adds a player to the friend list.
		Args : [0] = Character name to add.
    */

	if(HasQueryArgs(1) == false)
		return;

	const char *friendName = query.args[0].c_str();

	if(strcmp(friendName, creatureInst->css.display_name) == 0)
	{
		SendInfoMessage("You cannot add yourself as a friend.", INFOMSG_ERROR);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		PendingSend = true;
		return;
	}

	g_CharacterManager.GetThread("SimulatorThread::handle_query_friends_add");
	CharacterData *friendPtr = g_CharacterManager.GetCharacterByName(friendName);
	if(friendPtr != NULL)
	{
		pld.charPtr->AddFriend(friendPtr->cdef.CreatureDefID, friendPtr->cdef.css.display_name);
		UpdateSocialEntry(true, true);
		int wpos = PrepExt_FriendsAdd(SendBuf, friendPtr);
		AttemptSend(SendBuf, wpos);
	}
	g_CharacterManager.ReleaseThread();

	if(friendPtr == NULL)
	{
		Util::SafeFormat(LogBuffer, sizeof(LogBuffer), "Character is not online, or does not exist [%s]", friendName);
		SendInfoMessage(LogBuffer, INFOMSG_ERROR);
	}
	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

int SimulatorThread :: handle_query_friends_remove(void)
{
	/*  Query: friends.remove
		Removes a player from the friend list.
		Args : [0] = Character name to remove.
    */

	if(HasQueryArgs(1) == false)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	const char *friendName = query.args[0].c_str();

	g_CharacterManager.GetThread("SimulatorThread::handle_query_friends_remove");
	int res = pld.charPtr->RemoveFriend(friendName);
	UpdateSocialEntry(true, true);
	g_CharacterManager.ReleaseThread();

	if(res == 0)
	{
		Util::SafeFormat(LogBuffer, sizeof(LogBuffer), "Character [%s] was not found in friend list", friendName);
		SendInfoMessage(LogBuffer, INFOMSG_ERROR);
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_query_friends_status(void)
{
	/*  Query: friends.status
		Changes the player's status text.
		Args : [0] = New status text.
    */

	if(HasQueryArgs(1) == false)
		return;

	const char *newStatus = query.args[0].c_str();

	pld.charPtr->StatusText = newStatus;
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 43);  //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);  //size
	wpos += PutByte(&SendBuf[wpos], 3);   //message type for status changed
	wpos += PutStringUTF(&SendBuf[wpos], pld.charPtr->cdef.css.display_name);
	wpos += PutStringUTF(&SendBuf[wpos], newStatus);
	PutShort(&SendBuf[1], wpos - 3);

	size_t b;

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->ProtocolState == 1 && it->isConnected == true)
		{
			for(b = 0; b < it->pld.charPtr->friendList.size(); b++)
			{
				if((it->pld.charPtr->friendList[b].CDefID == pld.CreatureDefID) || (it->pld.CreatureDefID == pld.CreatureDefID))
				{
					it->AttemptSend(SendBuf, wpos);
					break;
				}
			}
		}
	}

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

void SimulatorThread :: handle_query_friends_getstatus(void)
{
	/*  Query: friends.getstatus
		Retrieves the player's friend status text.
		Args : [none]
    */

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, pld.charPtr->StatusText.c_str());
	PendingSend = true;
}

int SimulatorThread :: handle_query_statuseffect_set(void)
{
	if(HasQueryArgs(3) == false)
		return 0;

	if(!CheckPermissionSimple(0, Permission_Sage) && !!CheckPermissionSimple(0, Permission_Admin)) {
		SendInfoMessage("Permission denied.", INFOMSG_ERROR);
	}
	else {
		int CDefID = atoi(query.args[0].c_str());
		int statusEffect = atoi(query.args[1].c_str());
		int state = atoi(query.args[2].c_str());

//		::_Connection.sendQuery("statuseffect.set", this, [
//						targetId,
//						this.StatusEffects.GM_SILENCED,
//						1,
//						time,
//						this.mGMSilenceReasonPopup.getText()
//					]);

		CreatureInstance *creature = creatureInst->actInst->GetPlayerByID(CDefID);
		if(creature == NULL)
			SendInfoMessage("No target!", INFOMSG_ERROR);
		else
		{
			if(state == 1) {
				int time = query.argCount > 2 ? atoi(query.args[3].c_str()) : 1;
				creature->_AddStatusList(statusEffect, time * 1000 * 60);
				Util::SafeFormat(Aux2, sizeof(Aux2), "Status effect %d turned on for %s for %d minutes", statusEffect, creature->css.display_name, time);
				SendInfoMessage(Aux2, INFOMSG_INFO);
				if(statusEffect == StatusEffects::GM_SILENCED) {
					Util::SafeFormat(Aux2, sizeof(Aux2), "You have been silenced by %s for %d minutes because '%s'", pld.charPtr->cdef.css.display_name, time, query.args[4].c_str());
					creature->simulatorPtr->SendInfoMessage(Aux2, INFOMSG_INFO);
					g_Log.AddMessageFormat("[SAGE] %s silenced %s for %d minutes because",
							pld.charPtr->cdef.css.display_name, creature->charPtr->cdef.css.display_name,
							time, query.args[4].c_str());
				}
				else if(statusEffect == StatusEffects::GM_FROZEN) {
					Util::SafeFormat(Aux2, sizeof(Aux2), "You have been frozen by %s for %d minutes.", pld.charPtr->cdef.css.display_name, time);
					creature->simulatorPtr->SendInfoMessage(Aux2, INFOMSG_INFO);
					g_Log.AddMessageFormat("[SAGE] %s froze %s for %d minutes",
							pld.charPtr->cdef.css.display_name, creature->charPtr->cdef.css.display_name, time);
				}
				else {
					g_Log.AddMessageFormat("[SAGE] %s removed status effect %d from %s",
							pld.charPtr->cdef.css.display_name, statusEffect, creature->charPtr->cdef.css.display_name);
				}
			}
			else  {
				creature->_RemoveStatusList(statusEffect);
				Util::SafeFormat(Aux2, sizeof(Aux2), "Status effect %d turned off for %s", statusEffect, creature->css.display_name);
				SendInfoMessage(Aux2, INFOMSG_INFO);
				if(statusEffect == StatusEffects::GM_SILENCED) {
					Util::SafeFormat(Aux2, sizeof(Aux2), "You have been unsilenced by %s", pld.charPtr->cdef.css.display_name);
					creature->simulatorPtr->SendInfoMessage(Aux2, INFOMSG_INFO);
					g_Log.AddMessageFormat("[SAGE] %s unsilenced %s",
							pld.charPtr->cdef.css.display_name, creature->charPtr->cdef.css.display_name);
				}
				else if(statusEffect == StatusEffects::GM_FROZEN) {
					Util::SafeFormat(Aux2, sizeof(Aux2), "You have been unfrozen by %s", pld.charPtr->cdef.css.display_name);
					creature->simulatorPtr->SendInfoMessage(Aux2, INFOMSG_INFO);
					g_Log.AddMessageFormat("[SAGE] %s unfroze %s",
							pld.charPtr->cdef.css.display_name, creature->charPtr->cdef.css.display_name);
				}
				else {
					g_Log.AddMessageFormat("[SAGE] %s set status effect %d for %d minutes on %s",
							pld.charPtr->cdef.css.display_name, statusEffect, time, creature->charPtr->cdef.css.display_name);
				}
			}
		}
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: SaveCharacterStats(void)
{
	//This function pushes the Active Character data back into the main Character Data
	//in preparation for an autosave.

	LogPingStatistics(true, true);

	if(pld.charPtr == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] SaveCharacterStats() charPtr is NULL");
		return;
	}

	pld.charPtr->activeData.CurInstance = pld.CurrentInstanceID;
	pld.charPtr->activeData.CurZone = pld.CurrentZoneID;
	pld.charPtr->activeData.CurX = creatureInst->CurrentX;  //pld->CurrentX;
	pld.charPtr->activeData.CurY = creatureInst->CurrentY;  //pld->CurrentY;
	pld.charPtr->activeData.CurZ = creatureInst->CurrentZ;  //pld->CurrentZ;
	pld.charPtr->activeData.CurRotation = creatureInst->Rotation;  //pld->CurrentZ;

	int TimeSpan = (g_ServerTime - TimeOnline) / 1000;
	int Hour = TimeSpan / 3600;
	int Minute = (TimeSpan / 60) % 60;
	int Second = TimeSpan % 60;

	Util::SafeFormat(pld.charPtr->LastSession, sizeof(pld.charPtr->LastSession), "%02d:%02d:%02d", Hour, Minute, Second);

	int secSinceAutoSave = (g_ServerTime - TimeLastAutoSave) / 1000;
	TimeLastAutoSave = g_ServerTime;

	pld.charPtr->SecondsLogged += secSinceAutoSave;
	int TotalSec = pld.charPtr->SecondsLogged;

	Hour = TotalSec / 3600;
	Minute = (TotalSec / 60) % 60;
	Second = TotalSec % 60;

	Util::SafeFormat(pld.charPtr->TimeLogged, sizeof(pld.charPtr->TimeLogged), "%02d:%02d:%02d", Hour, Minute, Second);

	time_t curtime;
	time(&curtime);
	strftime(pld.charPtr->LastLogOff, sizeof(pld.charPtr->LastLogOn), "%Y-%m-%d, %I:%M %p", localtime(&curtime));

	//Need to copy the stats to the character definition, otherwise the updated
	//information won't save out.
	//First restore any transformed appearances.
//	creatureInst->AppearanceUnTransform();
	pld.charPtr->cdef.css.CopyFrom(&creatureInst->css);

	//Restore originals from any active stat mods.
	for(size_t i = 0; i < creatureInst->baseStats.size(); i++)
	{
		float value = creatureInst->baseStats[i].fBaseVal;
		WriteValueToStat(creatureInst->baseStats[i].StatID, value, &pld.charPtr->cdef.css);
	}
	pld.charPtr->cooldownManager.CopyFrom(creatureInst->cooldownManager);
	pld.charPtr->buffManager.CopyFrom(creatureInst->buffManager);
}

void SimulatorThread :: SetAccountCharacterCache(void)
{
	if(pld.accPtr != NULL && pld.charPtr != NULL)
		pld.accPtr->characterCache.UpdateCharacter(pld.charPtr);
	pld.accPtr->PendingMinorUpdates++;
}

int SimulatorThread :: handle_query_creature_def_edit(void)
{
	/*  Query: creature.def.edit
		Notifies the server of an updated creature definition, as modified by
		CreatureTweak
		Args : [0] = Creature Def ID
		       [n+0] = Property name to change.
			   [n+1] = New value for property.
			   [...]
    */

	//::_Connection.sendQuery("creature.def.edit", this, [
	//				"REASON",
//					this.mRenameReasonPopup.getText(),
//					targetId,
//					"name",
//					name
//				]);
//				::_Connection.sendQuery("creature.def.edit", this, [
//					"REASON",
//					this.mRenameReasonPopup.getText(),
//					targetId,
//					"DISPLAY_NAME",
//					name
//				]);

	if(HasQueryArgs(1) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");


	// Em - If the first argument is 'REASON', then this is a character rename
	// from the GM screen. Weird they overloaded this
	int argOffset = 0;
	if(strcmp(query.GetString(0), "REASON") == 0) {
		argOffset = 2;
	}

	int CDefID = atoi(query.args[argOffset].c_str());


	CreatureInstance* cInst = NULL;
	CreatureDefinition* cDef = NULL;
	CharacterData* charData = NULL;
	cInst = creatureInst->actInst->GetPlayerByCDefID(CDefID);
	if(cInst == NULL)
	{
		cDef = CreatureDef.GetPointerByCDef(CDefID);
		/*
		int index = CreatureDef.GetIndex(CDefID);
		if(index >= 0)
			cDef = &CreatureDef.NPC[index];
		*/
	}
	else
	{
		charData = g_CharacterManager.GetPointerByID(CDefID);
		if(charData != NULL)
			cDef = &charData->cdef;
	}

	if(cDef == NULL)
	{
		SendInfoMessage("Could not resolve creature template.", INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	}



	if(argOffset > 0)
	{
		// A character rename, so sage permissions needed
		if(CheckPermissionSimple(0, Permission_Sage) == false)
		{
			SendInfoMessage("Permission denied: Only sages can rename characters.", INFOMSG_ERROR);
			return PrepExt_QueryResponseNull(SendBuf, query.ID);
		}
	}
	else
	{
		if(CheckPermissionSimple(0, Permission_TweakClient) == true)
		{
			const char *appearance = NULL;
			for(int i = 1 + argOffset; i < query.argCount; i += 2)
			{
				const char *name = query.args[i].c_str();
				const char *value = query.args[i + 1].c_str();
				if(strcmp(name, "appearance") == 0)
				{
					appearance = value;
					break;
				}
			}
			int size = 0;
			if(appearance != NULL)
			{
				std::vector<short> statID;
				statID.push_back(STAT::APPEARANCE);
				CharacterStatSet data;
				//Util::SafeCopy(data.appearance, appearance, sizeof(data.appearance));
				data.SetAppearance(appearance);
				size = PrepExt_UpdateCreatureDef(SendBuf, CDefID, cDef->DefHints, statID, &data);
			}
			size += PrepExt_QueryResponseString(&SendBuf[size], query.ID, "OK");
			return size;
		}
		if(pld.CreatureDefID == CDefID)
		{
			if(CheckPermissionSimple(0, Permission_TweakSelf) == false)
			{
				SendInfoMessage("Permission denied: cannot edit self.", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(SendBuf, query.ID);
			}
		}
		else
		{
			if(charData != NULL)
			{
				if(CheckPermissionSimple(0, Permission_TweakOther) == false)
				{
					SendInfoMessage("Permission denied: cannot edit other players.", INFOMSG_ERROR);
					return PrepExt_QueryResponseNull(SendBuf, query.ID);
				}
			}
			else
			{
				if(CheckPermissionSimple(0, Permission_TweakNPC) == false)
				{
					SendInfoMessage("Permission denied: cannot edit creatures.", INFOMSG_ERROR);
					return PrepExt_QueryResponseNull(SendBuf, query.ID);
				}
			}
		}
	}

	CharacterStatSet *css = &cDef->css;

	//Now there should be an even number of data pairs of a key
	//and value.  The key specifies which item to change,
	//(appearance so far), and the value contains the information
	//to apply.
	for(int i = 1 + argOffset; i < query.argCount; i += 2)
	{
		std::string n = query.args[i];
		std::transform(n.begin(), n.end(), n.begin(), ::tolower);
		const char *name = n.c_str();
		const char *value = query.args[i + 1].c_str();

		LogMessageL(MSG_DIAGV, "  Name: %s", name);
		LogMessageL(MSG_DIAGV, "  Value: %s", value);

		if(strcmp(name, "name") == 0)
		{
			if(charData == NULL)
			{
				SendInfoMessage("Error setting name, no character data.", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(SendBuf, query.ID);
			}
			AccountData * acc = g_AccountManager.FetchIndividualAccount(charData->AccountID);
			if(acc == NULL)
			{
				SendInfoMessage("Error setting name. Missing account", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(SendBuf, query.ID);
			}
			CharacterCacheEntry *ce = acc->characterCache.GetCacheCharacter(cInst->CreatureDefID);
			if(ce == NULL)
			{
				SendInfoMessage("Error setting name. Missing cache entry", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(SendBuf, query.ID);
			}
			ce->display_name = value;
			acc->PendingMinorUpdates++;
		}
		else if(strcmp(name, "appearance") == 0 ||
		   strcmp(name, "display_name") == 0)
		{
			int wr = WriteStatToSetByName(name, value, css);
			if(wr == -1)
			{
				SendInfoMessage("Error setting attribute.", INFOMSG_ERROR);
				return PrepExt_QueryResponseNull(SendBuf, query.ID);
			}
		}
	}

	//Update the associated definition for a player.
	if(cInst != NULL)
		cInst->css.CopyFrom(css);

	AddMessage((long)cDef, 0, BCM_UpdateCreatureDef);

	if(argOffset > 0 && cInst != NULL) {
		// If update from a sage, sent the updates to the target as well
		cInst->simulatorPtr->AddMessage((long)cDef, 0, BCM_UpdateCreatureDef);
	}

	if(pld.CreatureDefID == CDefID)
	{
		//Set this so the main thread won't try to auto update
		//the character data for this simulator.
		//The message broadcast will take care of it.
		LastUpdate = g_ServerTime;
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_item_def_use(void)
{
	/*  Query: item.def.use
		Attempt to use an item.  Sent when an item is used from the quickbar.
		Args : [0] = Item Def ID
		       [1] = Creature/%d (Avatar)        Selected target.
    */

	g_Log.AddMessageFormat("item.def.use");
	if(query.argCount > 0)
	{
		int itemID = query.GetInteger(0);
		InventorySlot *item = pld.charPtr->inventory.GetItemPtrByID(itemID);
		if(item != NULL)
		{
			UseItem(item->CCSID);
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_item_use(void)
{
	/*  Query: item.use
		Attempt to use an item.  Sent when an object was double-clicked
		in the inventory.
		Args : [0] = Hex identifier of the inventory item.
    */

	if(query.argCount > 0)
	{
		unsigned int CCSID = pld.charPtr->inventory.GetCCSIDFromHexID(query.GetString(0));
		UseItem(CCSID);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: UseItem(unsigned int CCSID)
{
	if(creatureInst->activeLootID != 0)
	{
		SendInfoMessage("You cannot use items while trading.", INFOMSG_ERROR);
		return -1;
	}

	InventorySlot *slot = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if(slot == NULL)
	{
		SendInfoMessage("Item not found in inventory.", INFOMSG_ERROR);
		return -1;
	}

	ItemDef *itemDef = slot->ResolveItemPtr();
	if(itemDef == NULL)
	{
		SendInfoMessage("Item not found in database.", INFOMSG_ERROR);
		return -1;
	}

	if(itemDef->mType != ItemType::CONSUMABLE)
		return -1;

	if(creatureInst->css.level < itemDef->mMinUseLevel)
	{
		SendInfoMessage("Your level is too low.", INFOMSG_ERROR);
		return -1;
	}

	// The AddSidekick() function has its own thread guard, so we don't
	// need one here.
	bool removeOnUse = true;
	if(itemDef->mUseAbilityId != 0)
	{
		ConfigString cfg(itemDef->Params);
		//AddMessage((long)creatureInst, itemDef->mUseAbilityId, BCM_AbilityRequest);
		int r = creatureInst->RequestAbilityActivation(itemDef->mUseAbilityId);
		if(r != Ability2::ABILITY_SUCCESS)
		{
			removeOnUse = false;
			return -1;
		}
		else
		{
			if(creatureInst->ab[0].type == AbilityType::Cast)
			{
				pld.mItemUseInProgress = true;
				pld.mItemUseCCSID = CCSID;
				removeOnUse = false;  //We'll remove it later.  Look for code uses of RunFinishedCast().
			}
			else
			{

				int keep = cfg.GetValueInt("keep");
				if(keep != 0)
				{
					removeOnUse = false;
				}

				pld.mItemUseInProgress = false;
				pld.mItemUseCCSID = 0;
			}
		}
	}
	else
	{
		ConfigString cfg(itemDef->Params);
		int petSpawnID = cfg.GetValueInt("pet");
		if(petSpawnID != 0)
		{
			AddPet(petSpawnID);
			removeOnUse = false;
		}
		else
		{
			int credits = cfg.GetValueInt("credits");
			int abPoints = cfg.GetValueInt("abilitypoints");
			if(credits > 0)
			{
				creatureInst->css.credits += credits;
				Util::SafeFormat(Aux1, sizeof(Aux1), "You gain %d credits.", credits);
				SendInfoMessage(Aux1, INFOMSG_INFO);
				creatureInst->SendStatUpdate(STAT::CREDITS);
			}
			if(abPoints > 0)
			{
				creatureInst->css.current_ability_points += abPoints;
				creatureInst->css.total_ability_points += abPoints;
				pld.charPtr->AddAbilityPoints(abPoints);
				creatureInst->SendStatUpdate(STAT::CURRENT_ABILITY_POINTS);
				creatureInst->SendStatUpdate(STAT::TOTAL_ABILITY_POINTS);
				//We don't need to send a text notification for ability points, the client will notify the user.
			}
			removeOnUse = true;
		}
	}

	/*
	else if(itemDef->mActionAbilityId != 0)
	{
		if(itemDef->mActionAbilityId == RESERVED_ABILITY_SUMMON)
		{
			ArbitraryKeyValueData adat;
			adat.Assign(itemDef->Params);
			int petSpawnID = adat.GetValueInt("pet");
			if(petSpawnID != 0)
			{
				AddPet(petSpawnID);
				removeOnUse = false;
			}
			else
			{
				if(AddSidekick(itemDef->mActionAbilityId) == -1)
					removeOnUse = false;
			}
		}
		else if(itemDef->mActionAbilityId == RESERVED_ABILITY_BONUS)
		{
			ArbitraryKeyValueData adat;
			adat.Assign(itemDef->Params);
			int credits = adat.GetValueInt("credits");
			int abPoints = adat.GetValueInt("abilitypoints");
			if(credits > 0)
			{
				creatureInst->css.credits += credits;
				Util::SafeFormat(Aux1, sizeof(Aux1), "You gain %d credits.", credits);
				SendInfoMessage(Aux1, INFOMSG_INFO);
				creatureInst->SendStatUpdate(STAT::CREDITS);
			}
			if(abPoints > 0)
			{
				creatureInst->css.current_ability_points += abPoints;
				creatureInst->css.total_ability_points += abPoints;
				pld.charPtr->AddAbilityPoints(abPoints);
				creatureInst->SendStatUpdate(STAT::CURRENT_ABILITY_POINTS);
				creatureInst->SendStatUpdate(STAT::TOTAL_ABILITY_POINTS);
				//We don't need to send a text notification for ability points, the client will notify the user.
			}
			removeOnUse = true;
		}
	}
	else
	{
		LogMessageL(MSG_SHOW, "Checking pet");
		ArbitraryKeyValueData adat;
		adat.Assign(itemDef->Params);
		int petSpawnID = adat.GetValueInt("pet");
		if(petSpawnID != 0)
		{
			LogMessageL(MSG_SHOW, "Adding pet");
			AddPet(petSpawnID);
			removeOnUse = false;
		}
	}
	*/

	if(removeOnUse == false)  //No need for the rest, leave early.
		return 0;

	//If it's stackable, need to remove an object from the stack,
	//and delete the item if there are no more.
	DecrementStack(slot);

	return 0;
}

void SimulatorThread :: DecrementStack(InventorySlot *slot)
{
	if(slot == NULL)
		return;

	int wpos = 0;
	if(slot->GetMaxStack() > 0)
	{
		slot->count--;
		if(slot->count < 0)
		{
			wpos += RemoveItemUpdate(SendBuf, Aux3, slot);
			pld.charPtr->inventory.RemItem(slot->CCSID);
		}
		else
			wpos += AddItemUpdate(SendBuf, Aux3, slot);
	}
	if(wpos > 0)
		AttemptSend(SendBuf, wpos);
}

void SimulatorThread :: RunFinishedCast(bool success)
{
	if(pld.mItemUseInProgress == true)
	{
		pld.mItemUseInProgress = false;
		if(success == true)
		{
			InventorySlot *slot = pld.charPtr->inventory.GetItemPtrByCCSID(pld.mItemUseCCSID);
			if(slot != NULL)
				DecrementStack(slot);
		}
	}
}

bool SimulatorThread :: CanMoveItems(void)
{
	if(pld.mItemUseInProgress == true)
		return false;

	return true;
}

void SimulatorThread :: handle_query_ab_ownage_list(void)
{
	/*  Query: ab.ownage.list
		Retrieves the list of abilities which the player is able to use.
		Args : [none]
    */

	WritePos = 0;
	WritePos += PutByte(&SendBuf[WritePos], 1);   //_handleQueryResultMsg
	WritePos += PutShort(&SendBuf[WritePos], 0);          //Message size
	WritePos += PutInteger(&SendBuf[WritePos], query.ID);

	int size = pld.charPtr->abilityList.AbilityList.size();
	WritePos += PutShort(&SendBuf[WritePos], size);      //Row count

	int a;
	for(a = 0; a < size; a++)
	{
		WritePos += PutByte(&SendBuf[WritePos], 1);
		sprintf(Aux3, "%d", pld.charPtr->abilityList.AbilityList[a]);
		WritePos += PutStringUTF(&SendBuf[WritePos], Aux3);
	}

	PutShort(&SendBuf[1], WritePos - 3);             //Set message size
	PendingSend = true;
}

int SimulatorThread :: handle_query_ab_buy(void)
{
	/*  Query: ab.buy
		Attempt to purchase an ability.
		Args : [0] = Ability ID to purchase.
    */

	if(HasQueryArgs(1) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Query error.");

	int abilityID = query.GetInteger(0);
	if(abilityID >= Ability2::AbilityManager2::NON_PURCHASE_ID_THRESHOLD)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot purchase that ability.");

	if(pld.charPtr->abilityList.GetAbilityIndex(abilityID) >= 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You already have that ability.");

	const Ability2::AbilityEntry2 *abData = g_AbilityManager.GetAbilityPtrByID(abilityID);
	if(abData == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Ability does not exist.");

	int pointCost = abData->GetPurchaseCost();
	if(creatureInst->css.current_ability_points < pointCost)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Not enough ability points.");

	if(creatureInst->css.level < abData->GetPurchaseLevel())
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Your level is not high enough.");
	
	if(abData->IsPurchaseableBy(creatureInst->css.profession) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Your class cannot use that ability.");

	std::vector<short> preReq;
	abData->GetPurchasePrereqList(preReq);
	for(size_t i = 0; i < preReq.size(); i++)
	{
		if(preReq[i] > 0)
			if(pld.charPtr->abilityList.GetAbilityIndex(preReq[i]) == -1)
				return PrepExt_QueryResponseError(SendBuf, query.ID, "You are missing prerequisite abilities.");
	}

	pld.charPtr->abilityList.AddAbility(abilityID);
	creatureInst->css.current_ability_points -= pointCost;

	if(abData->IsPassive() == true)
	{
		creatureInst->CallAbilityEvent(abilityID, EventType::onRequest);
		creatureInst->CallAbilityEvent(abilityID, EventType::onActivate);

		//creatureInst->actInst->ActivateAbility(creatureInst, abilityID, Action::onRequest, 0);
		//creatureInst->actInst->ActivateAbility(creatureInst, abilityID, Action::onActivate, 0);
	}

	AddMessage((long)creatureInst, 0, BCM_UpdateCreatureInstance);
	return PrepExt_QueryResponseString(SendBuf, query.ID, query.GetString(0));
}
	
void SimulatorThread :: handle_query_ab_respec(void)
{
	/*  Query: ab.respec
		Remove all abilities from the purchase list.
		Args : [none]
    */

	creatureInst->Respec();
	pld.charPtr->AbilityRespec(creatureInst);
	ActivatePassiveAbilities();
	UpdateEqAppearance();

	AddMessage((long)creatureInst, 0, BCM_UpdateCreatureInstance);
	AddMessage((long)&pld.charPtr->cdef, 0, BCM_CreatureDefRequest);

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "true");
	PendingSend = true;
}

int SimulatorThread :: handle_query_buff_remove(void)
{
	/*  Query: buff.remove
		Sent when a player attempts to cancel a buff on their character.
		Args : [0] = Ability ID
    */

	if(query.argCount > 0)
	{
		int abilityID = query.GetInteger(0);
		creatureInst->RemoveBuffsFromAbility(abilityID, true);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_command_deleteabove(void)
{
	/*  Query: deleteabove
		Cheat to remove all items in the player's inventory above the given
		slot number.
		Args : [0] = Slot number to begin deletion (slot adjusted to start at 1)
    */
	if(query.argCount == 0)
		SendInfoMessage("Usage: /deleteabove slotNumber", INFOMSG_ERROR);
	if(HasQueryArgs(1) == false)
		return;

	int delSlot = atoi(query.args[0].c_str());

	WritePos = 0;
	int rcount = 0;
	bool bFound = true;
	while(bFound == true)
	{
		bFound = false;
		for(size_t a = 0; a < pld.charPtr->inventory.containerList[INV_CONTAINER].size(); a++)
		{
			unsigned long CCSID = pld.charPtr->inventory.containerList[INV_CONTAINER][a].CCSID;
			int slot = CCSID & CONTAINER_SLOT;
			if(slot >= delSlot)
			{
				WritePos += RemoveItemUpdate(&SendBuf[WritePos], Aux3, &pld.charPtr->inventory.containerList[INV_CONTAINER][a]);
				pld.charPtr->inventory.RemItem(CCSID);
				rcount++;
				CheckWriteFlush(WritePos);
				bFound = true;
				break;
			}
		}
	}
	if(WritePos > 0)
		AttemptSend(SendBuf, WritePos);

	sprintf(Aux1, "Removed %d items.", rcount);
	SendInfoMessage(Aux1, INFOMSG_INFO);

	WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	PendingSend = true;
}

int SimulatorThread :: handle_query_util_ping(void)
{
	/*  Query: util.ping
		Sent by the client in odd times, such as when communication is idle.
		Args : [none]
    */
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: handle_query_util_pingsim(void)
{
	/*  Query: util.pingsim
		Sent by the /ping command.
		Args : [none]
    */
	
	WritePos = PrepExt_QueryResponseNull(SendBuf, query.ID);
	PendingSend = true;
}

void SimulatorThread :: handle_query_util_pingrouter(void)
{
	/*  Query: util.pingrouter
		Sent by the /ping command.
		Args : [none]
    */

	WritePos = PrepExt_QueryResponseNull(SendBuf, query.ID);
	PendingSend = true;
}

void SimulatorThread :: handle_query_essenceShop_contents(void)
{
	/*  Query: essenceShop.contents
		Sent when an essence shop (chest) is clicked on.
		Args : [0] = Creature ID of the chest spawn object.
    */

	if(HasQueryArgs(1) == false)
		return;

	int CID = atoi(query.args[0].c_str());
	LogMessageL(MSG_DIAGV, "essenceShop.contents: %d", CID);

	int CDef = ResolveCreatureDef(CID);

	WritePos = creatureInst->actInst->essenceShopList.ProcessQueryEssenceShop(SendBuf, Aux1, CDef, query.ID);
	if(WritePos == 0)
	{
		LogMessageL(MSG_WARN, "[WARNING] EssenceShop not found for [%d]", CDef);
		sprintf(Aux3, "EssenceShop not found for [%d]", CDef);
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, Aux3);
	}
	PendingSend = true;
}


int SimulatorThread :: handle_query_account_info(void)
{
	if(HasQueryArgs(1) == false)
		return 0;

	AccountQuickData *quickData = g_AccountManager.GetAccountQuickDataByUsername(query.args[0].c_str());
	AccountData * data = NULL;
	if(quickData == NULL)
	{
		g_CharacterManager.GetThread("SimulatorThread::handle_query_account_info");
		CharacterData *friendPtr = g_CharacterManager.GetCharacterByName(query.args[0].c_str());
		if(friendPtr != NULL)
			data = g_AccountManager.GetActiveAccountByID(friendPtr->AccountID);
		g_CharacterManager.ReleaseThread();
	}
	else {
		data = g_AccountManager.GetActiveAccountByID(quickData->mID);
	}
	if(data == NULL)
	{
		Util::SafeFormat(Aux2, sizeof(Aux2), "Could not find account for '%s'", query.args[0].c_str());
		SendInfoMessage(Aux2, INFOMSG_INFO);
	}
	else
	{
		Util::SafeFormat(Aux2, sizeof(Aux2), "Username: %s (%d) with %d characters  Grove: %s", data->Name, data->ID, data->GetCharacterCount(), data->GroveName.c_str());
		SendInfoMessage(Aux2, INFOMSG_INFO);
		int b;
		for(b = 0; b < AccountData::MAX_CHARACTER_SLOTS; b++)
		{
			if(data->CharacterSet[b] != 0)
			{
				int cdefid = data->CharacterSet[b];
				g_CharacterManager.GetThread("SimulatorThread::handle_query_account_info");
				CharacterData *friendPtr = g_CharacterManager.GetPointerByID(cdefid);
				if(friendPtr != NULL)
				{
					Util::SafeFormat(Aux2, sizeof(Aux2), "    %s (%d) Lvl: %d Copper: %d", friendPtr->cdef.css.display_name, friendPtr->cdef.CreatureDefID, friendPtr->cdef.css.level, friendPtr->cdef.css.copper);
					SendInfoMessage(Aux2, INFOMSG_INFO);
				}
				g_CharacterManager.ReleaseThread();
			}
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

/* ALTERNATE POSSIBLE FUNCTION
int SimulatorThread :: ResolveCreatureDef(int CreatureInstanceID)
{
	if(RegisterMainCall2() == false)
		return 0;

	if(creatureInst == NULL)
	{
		LogMessageL(MSG_ERROR, "ResolveCreatureDef() creatureInst is NULL");
		return 0;
	}
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_ERROR, "ResolveCreatureDef() actInst is NULL");
		return 0;
	}
	int ret = 0;
	creatureInst->actInst->ResolveCreatureDef(CreatureInstanceID, &ret);
	UnregisterMainCall2();
	return ret;
}*/

bool SimulatorThread :: WaitUntilNonzero(int *data)
{
	//Waits for an integer value to be changed by another thread.
	unsigned long quitTime = g_ServerTime + 5000;
	while(*data == 0)
	{
		PLATFORM_SLEEP(1);
		if(g_ServerTime >= quitTime)
			break;
	};
	if(data == 0)
		return false;

	return true;
}

int SimulatorThread :: ResolveCreatureDef(int CreatureID)
{
	int response = 0;
	creatureInst->actInst->ResolveCreatureDef(CreatureID, &response);
	return response;
}

CreatureInstance* SimulatorThread :: ResolveCreatureInstance(int CreatureID, int searchHint)
{
	if(creatureInst == NULL)
	{
		LogMessageL(MSG_CRIT, "[CRITICAL] ResolveCreatureInstance creatureInst is NULL");
		return NULL;
	}
	if(creatureInst->actInst == NULL)
	{
		LogMessageL(MSG_CRIT, "[CRITICAL] ResolveCreatureInstance actInst is NULL");
		return NULL;
	}
	switch(searchHint)
	{
	case 1: return creatureInst->actInst->GetNPCInstanceByCID(CreatureID);

	case 0:  //No hints is same as default
	default:
		 return creatureInst->actInst->inspectCreature(CreatureID);
	}

	return NULL;
}

void SimulatorThread :: handle_query_shop_contents(void)
{
	/*  Query: shop.contents
		Sent when an NPC shop is clicked on.
		Args : [0] = Creature ID of NPC.
    */

	if(HasQueryArgs(1) == false)
		return;

	int CID = atoi(query.args[0].c_str());
	LogMessageL(MSG_SHOW, "shop.contents: %d", CID);
		
	if(CID == creatureInst->CreatureID)
	{
		helper_shop_contents_self();  //query contents and pending status are set here
		return;
	}

	int CDef = ResolveCreatureDef(CID);

	int wpos = creatureInst->actInst->itemShopList.ProcessQueryShop(SendBuf, Aux1, CDef, query.ID);
	if(wpos == 0)
	{
		LogMessageL(MSG_WARN, "[WARNING] Shop Contents not found for [%d]", CDef);
		sprintf(Aux3, "Shop Contents not found for [%d]", CDef);
		wpos = PrepExt_QueryResponseError(SendBuf, query.ID, Aux3);
	}
	WritePos = wpos;

	PendingSend = true;
}

void SimulatorThread :: helper_shop_contents_self(void)
{
	//Fill the query with the contents of the buyback information.
	LogMessageL(MSG_SHOW, "Shop contents self.");
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);             //Placeholder for message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);     //Query response ID

	int rowCount = (int)pld.charPtr->inventory.containerList[BUYBACK_CONTAINER].size();
	wpos += PutShort(&SendBuf[wpos], rowCount);

	//Note: low index = newest, also highest slot number
	for(int index = 0; index < rowCount; index++)
	{
		wpos += PutByte(&SendBuf[wpos], 1);       //Always one string for regular items.
		GetItemProto(Aux3, pld.charPtr->inventory.containerList[BUYBACK_CONTAINER][index].IID, 0);
		wpos += PutStringUTF(&SendBuf[wpos], Aux3);
	}
	PutShort(&SendBuf[1], wpos - 3);
	
	WritePos = wpos;
	PendingSend = true;
}

int SimulatorThread :: handle_query_trade_shop(void)
{
	/*  Query: trade.shop
		Sent when an item is purchased from a shop.
		Args : [0] = Creature ID of the NPC

		       If buying from shop:
	           [1] = Item proto of selection (ex: item143548:0:0:0);

			   If buying from buyback:
			   [1] = Hex string of item container/slot ID

			   If selling
			   [1] = String constant: "sell"
			   [2] = Hex string of item container/slot ID
    */

	if(query.argCount < 2)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
 
	if(pld.zoneDef->mGrove == true)
		if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "You cannot use shops in a grove.");

	//if(HasQueryArgs(2) == false)
	//	return 0;

	int CID = atoi(query.args[0].c_str());
	int CDef = ResolveCreatureDef(CID);

	if(query.args[1].compare("sell") == 0)
	{
		//if(HasQueryArgs(3) == false)
		//	return 0;
		if(query.argCount < 3)
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

		//[2] = Hex String, pass it to the helper function
		return helper_trade_shop_sell(query.args[2].c_str());
	}

	//Since the Essence Shop scanning functions modify the string while searching
	//for tokens, copy it to a buffer here.
	Util::SafeCopy(Aux1, query.args[1].c_str(), sizeof(Aux1));

	EssenceShopItem *iptr = NULL;
	EssenceShop *esptr = creatureInst->actInst->itemShopList.GetEssenceShopPtr(CDef, Aux1, &iptr);
	if(esptr == NULL || iptr == NULL)
	{
		//This might be a buyback item.
		unsigned int CCSID = strtol(Aux1, NULL, 16);
		int r = helper_trade_shop_buyback(CCSID);
		if(r > 0)
			return r;

		LogMessageL(MSG_ERROR, "[ERROR] Failed to process Shop item [%s] for CreatureDef [%d]", Aux1, CDef);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to determine item selection.");
	}

	ItemDef *itemPtr = g_ItemManager.GetPointerByID(iptr->ItemID);
	if(itemPtr == NULL)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "Server error: item does not exist.");

	int cost = (int)(itemPtr->mValue * g_VendorMarkup);
	if(creatureInst->css.copper < cost)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "Not enough coin.");

	//Check to see if this was a gambled item, and modify its ID accordingly.
	int newItemID = 0;
	newItemID = g_ItemManager.RunPurchaseModifier(itemPtr->mID);

	//If the normal gamble options failed, it will return the same ID.  Check for new randomized gamble options instead.
	if(newItemID == itemPtr->mID)
	{
		VirtualItemSpawnParams viParam;
		if(g_ItemManager.CheckVirtualItemGamble(itemPtr, creatureInst->css.level, &viParam, cost) == true)
		{
			//Coin amount may be modified.
			if(creatureInst->css.copper < cost)
				return PrepExt_QueryResponseString(SendBuf, query.ID, "Not enough coin.");

			newItemID = g_ItemManager.RollVirtualItem(viParam);
		}
	}
	if(newItemID == 0)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "A purchase modifier has failed.");

	InventorySlot *newItem = pld.charPtr->inventory.AddItem_Ex(INV_CONTAINER, newItemID, 1);
	if(newItem == NULL)
	{
		int err = pld.charPtr->inventory.LastError;
		if(err == InventoryManager::ERROR_ITEM)
			return PrepExt_QueryResponseString(SendBuf, query.ID, "Server error: item does not exist.");
		else if(err == InventoryManager::ERROR_SPACE)
			return PrepExt_QueryResponseString(SendBuf, query.ID, "You do not have any free inventory space.");
		else
			return PrepExt_QueryResponseString(SendBuf, query.ID, "Server error: undefined error.");
	}

	int wpos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	wpos += AddItemUpdate(&SendBuf[wpos], Aux3, newItem);
	creatureInst->AdjustCopper(-cost);
	return wpos;
}

int SimulatorThread :: helper_trade_shop_sell(const char *itemHex)
{
	//Return the query response when selling stuff to an NPC shop.
	unsigned int CCSID = strtol(itemHex, NULL, 16);
	InventorySlot *item = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if(item == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Item not found in inventory.");

	ItemDef *itemPtr = item->ResolveItemPtr();
	if(itemPtr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Item does not exist in server database.");

	int wpos = 0;
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");

	int cost = itemPtr->mValue;
	cost *= item->GetStackCount();
	wpos += pld.charPtr->inventory.AddBuyBack(item, &SendBuf[wpos]);
	wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, item);
	pld.charPtr->inventory.RemItem(CCSID);
	creatureInst->AdjustCopper(cost);

	return wpos;
}

int SimulatorThread :: helper_trade_shop_buyback(unsigned int CCSID)
{
	InventorySlot *buybackItem = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if(buybackItem == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Item not found in buyback list.");

	int freeslot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if(freeslot == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "No free inventory space.");

	ItemDef *itemPtr = buybackItem->ResolveItemPtr();
	if(itemPtr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Item does not exist in database.");

	int cost = itemPtr->mValue * buybackItem->GetStackCount();
	if(creatureInst->css.copper < cost)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Not enough coin.");

	InventorySlot newItem;
	newItem.CopyFrom(*buybackItem, false);
	newItem.CCSID = pld.charPtr->inventory.GetCCSID(INV_CONTAINER, freeslot);

	int r = pld.charPtr->inventory.AddItem(INV_CONTAINER, newItem);
	if(r == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to create item.");

	int wpos = 0;
	wpos = RemoveItemUpdate(SendBuf, Aux3, buybackItem);
	wpos += AddItemUpdate(&SendBuf[wpos], Aux3, &newItem);
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	pld.charPtr->inventory.RemItem(buybackItem->CCSID);
	creatureInst->AdjustCopper(-cost);
	return wpos;
}

int SimulatorThread :: handle_query_trade_essence(void)
{
	/*  Query: trade.essence
		Sent when an item is purchased from a chest using tokens or
		essences, instead of gold.
		Args : [0] = Creature Instance ID
		       [1] = Item proto of the player's selection.
	*/

	// [0] = Creature Instance ID
	// [1] = Item Proto that was selected ex: "item143548:0:0:0"

	if(query.argCount < 2)
		return 0;

	int CID = atoi(query.args[0].c_str());
	int CDef = ResolveCreatureDef(CID);

	//Since the Essence Shop scanning functions modify the string while searching
	//for tokens, copy it to a buffer here.
	Util::SafeCopy(Aux1, query.args[1].c_str(), sizeof(Aux1));

	EssenceShopItem *iptr = NULL;
	EssenceShop *esptr = creatureInst->actInst->essenceShopList.GetEssenceShopPtr(CDef, Aux1, &iptr);
	if(esptr == NULL || iptr == NULL)
	{
		LogMessageL(MSG_ERROR, "[ERROR] Failed to process EssenceShop item [%s] for CreatureDef [%d]", Aux1, CDef);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to determine item selection.");
	}

	InventoryManager &inv = pld.charPtr->inventory;
	int currentItemCount = inv.GetItemCount(INV_CONTAINER, esptr->EssenceID);
	if(currentItemCount < iptr->EssenceCost)
	{
		LogMessageL(MSG_SHOW, "DEBUG] Essence requirement for item %d: %d / %d", esptr->EssenceID, currentItemCount, iptr->EssenceCost);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "You do not have enough essences.");
	}

	InventorySlot *newItem = pld.charPtr->inventory.AddItem_Ex(INV_CONTAINER, iptr->ItemID, 1);
	if(newItem == NULL)
	{
		int err = pld.charPtr->inventory.LastError;
		if(err == InventoryManager::ERROR_ITEM)
			return PrepExt_QueryResponseString(SendBuf, query.ID, "Server error: item does not exist.");
		else if(err == InventoryManager::ERROR_SPACE)
			return PrepExt_QueryResponseString(SendBuf, query.ID, "You do not have any free inventory space.");
		else
			return PrepExt_QueryResponseString(SendBuf, query.ID, "Server error: undefined error.");
	}
	STRINGLIST result;
	result.push_back("OK");
	sprintf(Aux3, "%d", iptr->EssenceCost);
	result.push_back(Aux3);
	int wpos = PrepExt_QueryResponseStringList(SendBuf, query.ID, result);
	wpos += AddItemUpdate(&SendBuf[wpos], Aux3, newItem);

	wpos += inv.RemoveItemsAndUpdate(INV_CONTAINER, esptr->EssenceID, iptr->EssenceCost, &SendBuf[wpos]);
	return wpos;
}

int SimulatorThread :: handle_command_grove(void)
{
	std::vector<std::string> groveList;

	int groveCount = g_ZoneDefManager.EnumerateGroves(pld.accPtr->ID,  pld.CreatureDefID, groveList);
	int wpos = 0;
	if(groveCount > 0)
	{
		if(groveCount >= 254)
			groveCount = 254;

		wpos += PutByte(&SendBuf[wpos], 4);   //_handleCreatureEventMsg
		wpos += PutShort(&SendBuf[wpos], 0);  //size

		wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);
		wpos += PutByte(&SendBuf[wpos], 14);  //Event for henge click

		wpos += PutInteger(&SendBuf[wpos], ZoneDefManager::HENGE_ID_CUSTOMWARP);

		if(pld.zoneDef->mGuildHall == true)
		{
			wpos += PutByte(&SendBuf[wpos], groveCount + 1);
			wpos += PutStringUTF(&SendBuf[wpos], EXIT_GUILD_HALL);
			wpos += PutInteger(&SendBuf[wpos], 0);
		}
		else if(pld.zoneDef->mGrove == true)
		{
			wpos += PutByte(&SendBuf[wpos], groveCount + 1);
			wpos += PutStringUTF(&SendBuf[wpos], EXIT_GROVE);
			wpos += PutInteger(&SendBuf[wpos], 0);
		}
		else
			wpos += PutByte(&SendBuf[wpos], groveCount);
		for(int i = 0; i < groveCount; i++)
		{
			//Henge Name, Cost
			//wpos += PutStringUTF(&SendBuf[wpos], groveList[i]->mWarpName.c_str());
			wpos += PutStringUTF(&SendBuf[wpos], groveList[i].c_str());
			wpos += PutInteger(&SendBuf[wpos], 0);
		}
		PutShort(&SendBuf[1], wpos - 3);  //size
	}
	else
	{
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "No groves found.", INFOMSG_INFO);
	}
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_pvp(void)
{
	std::vector<std::string> zoneList;
	int zoneCount = g_ZoneDefManager.EnumerateArenas(zoneList);
	int wpos = 0;
	if(zoneCount > 0)
	{
		if(zoneCount >= 254)
			zoneCount = 254;

		wpos += PutByte(&SendBuf[wpos], 4);   //_handleCreatureEventMsg
		wpos += PutShort(&SendBuf[wpos], 0);  //size

		wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);
		wpos += PutByte(&SendBuf[wpos], 14);  //Event for henge click

		wpos += PutInteger(&SendBuf[wpos], ZoneDefManager::HENGE_ID_CUSTOMWARP);

		if(pld.zoneDef->mArena == true)
		{
			wpos += PutByte(&SendBuf[wpos], zoneCount + 1);
			wpos += PutStringUTF(&SendBuf[wpos], EXIT_PVP);
			wpos += PutInteger(&SendBuf[wpos], 0);
		}
		else
			wpos += PutByte(&SendBuf[wpos], zoneCount);
		for(int i = 0; i < zoneCount; i++)
		{
			//Henge Name, Cost
			//wpos += PutStringUTF(&SendBuf[wpos], groveList[i]->mWarpName.c_str());
			wpos += PutStringUTF(&SendBuf[wpos], zoneList[i].c_str());
			wpos += PutInteger(&SendBuf[wpos], 0);
		}
		PutShort(&SendBuf[1], wpos - 3);  //size
	}
	else
	{
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "No arenas found.", INFOMSG_INFO);
	}
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: CheckValidHengeDestination(const char *destName, int creatureID)
{
	if(pld.zoneDef == NULL || creatureInst == NULL)
		return false;

	if(creatureInst->actInst == NULL)
		return false;

	if(creatureInst->Speed != 0)
	{
		SendInfoMessage("You must be stationary.", INFOMSG_ERROR);
		return false;
	}
	if(creatureInst->HasStatus(StatusEffects::DEAD))
	{
		SendInfoMessage("You must be alive.", INFOMSG_ERROR);
		return false;
	}

	if(creatureID != ZoneDefManager::HENGE_ID_CUSTOMWARP)
	{
		//We've clicked a henge object.  The target zone must exist in the henge interact list.
		CreatureInstance *target = creatureInst->actInst->inspectCreature(creatureID);
		if(target == NULL)
		{
			SendInfoMessage("Could not find object.", INFOMSG_ERROR);
			return false;
		}
		if(target->HasStatus(StatusEffects::HENGE) == false)
		{
			SendInfoMessage("Object is not a henge.", INFOMSG_ERROR);
			return false;
		}
		if(creatureInst->IsObjectInRange(target, INTERACT_RANGE) == false)
		{
			SendInfoMessage("Out of range.", INFOMSG_ERROR);
			return false;
		}
		if(g_InteractObjectContainer.GetHengeByTargetName(destName) == NULL)
		{
			SendInfoMessage("There is no henge with that name.", INFOMSG_ERROR);
			return false;
		}
		if(pld.zoneDef->IsFreeTravel() == true)
		{
			SendInfoMessage("You cannot use henges in groves or arenas.", INFOMSG_ERROR);
			return false;
		}
	}
	else
	{
		//We're using the henge screen as a makeshift destination selector like from /grove or /pvp.
		//The target zone must be a grove or arena, or an exit command for those respective areas.
		

		//Quick check to make sure the exit commands are not used outside their respective zones.
		if(strcmp(destName, EXIT_GROVE) == 0)
		{
			if(pld.zoneDef->mGrove == false)
			{
				SendInfoMessage("You are not in a grove.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}
		if(strcmp(destName, EXIT_GUILD_HALL) == 0)
		{
			if(pld.zoneDef->mGuildHall == false)
			{
				SendInfoMessage("You are not in a guild hall.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}
		if(strcmp(destName, EXIT_PVP) == 0)
		{
			if(pld.zoneDef->mArena == false)
			{
				SendInfoMessage("You are not in a PvP arena.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}

		//If we're not in a grove or arena, make sure we're near a sanctuary.
		if(pld.zoneDef->IsFreeTravel() == false) 
		{
			WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(pld.zoneDef->mID, creatureInst->CurrentX, creatureInst->CurrentZ, SANCTUARY_PROXIMITY_USE); 
			if(sanct == NULL)
			{
				SendInfoMessage("You must be within range of a sanctuary.", INFOMSG_ERROR);
				return false;
			}
		}

		ZoneDefInfo *destZone = g_ZoneDefManager.GetPointerByExactWarpName(destName);
		if(destZone == NULL)
			return false;

		//Make sure the target zone is a grove or arena.
		if(destZone->IsFreeTravel() == false)
			return false;

		//Certain groves may disallow player warps, so check that here too.
		if(CheckValidWarpZone(destZone->mID) != ERROR_NONE)
			return false;
	}

	//If we get here, no error conditions have bailed out.
	return true;
}


int SimulatorThread :: handle_query_henge_setDest(void)
{
	/*  Query: henge.setDest
		Sent when a henge selection is made.
		Args : [0] = Destination Name
		       [1] = Creature ID (when using henge interacts, but also uses custom values for certain things like groves).
	*/
	
	if(query.argCount >= 2)
	{
		const char *destName = query.GetString(0);
		int sourceID = query.GetInteger(1);

		if(CheckValidHengeDestination(destName, sourceID) == true)
		{
			ZoneDefInfo *zoneDef = NULL;
			InteractObject *interact = NULL;
			bool blockAttempt = false;

			if((strcmp(destName, EXIT_GROVE) == 0) || (strcmp(destName, EXIT_PVP) == 0) || (strcmp(destName, EXIT_GUILD_HALL) == 0))
			{
				int destID = pld.charPtr->groveReturnPoint[3];  //x, y, z, zoneID
				if(destID == 0)
				{
					if(creatureInst->css.level < 3)
						destID = 59;
					else if(creatureInst->css.level < 11)
						destID = 58;
					else
						destID = 81;
				}
				zoneDef = g_ZoneDefManager.GetPointerByID(destID);
			}
			else
			{
				interact = g_InteractObjectContainer.GetHengeByTargetName(destName);
				if(interact == NULL)
					zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(destName);
				else
					zoneDef = g_ZoneDefManager.GetPointerByID(interact->WarpID);
			}

			if(interact != NULL)
			{
				if(pld.CurrentZoneID == interact->WarpID)
				{
					if(creatureInst->IsSelfNearPoint((float)interact->WarpX, (float)interact->WarpZ, INTERACT_RANGE * 4) == true)
					{
						SendInfoMessage("You are already at that location.", INFOMSG_ERROR);
						blockAttempt = true;
					}
				}

				if(blockAttempt == false && interact->cost != 0)
				{
					if(creatureInst->css.copper < interact->cost)
					{
						SendInfoMessage("Not enough coin.", INFOMSG_ERROR);
						blockAttempt = true;
					}
					else
					{
						creatureInst->AdjustCopper(-interact->cost);
					}
				}
			}

			if(zoneDef == NULL)
			{
				Util::SafeFormat(Aux1, sizeof(Aux1), "Warp target not found: %s", destName);
				SendInfoMessage(Aux1, INFOMSG_ERROR);
			}
			else if(blockAttempt == false)
			{
				//Ready to warp!
				int xLoc = 0, yLoc = 0, zLoc = 0;
				if(interact != NULL)
				{
					xLoc = interact->WarpX;
					yLoc = interact->WarpY;
					zLoc = interact->WarpZ;
				}
				WarpToZone(zoneDef, xLoc, yLoc, zLoc);
			}
		}
	}

		/*
		//We need to verify the warp is acceptable.  This adds some exploit protection since it
		//could otherwise technically be entered as a chat command and interpreted as a valid warp
		//for civilian players.
		if(verificationRequired == true)
		{
			int errCode = CheckValidWarpZone(zoneDef->mID);
			if(errCode != ERROR_NONE)
			{
				SendInfoMessage(GetGenericErrorString(errCode), INFOMSG_ERROR);
				goto endfunction;
			}
		}
		*/
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_portal_acceptRequest(void)
{
	AddMessage((long)this, 0, BCM_RunPortalRequest);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

/* DISABLED, NEVER FINISHED
void SimulatorThread :: CheckQuestItems(void)
{
	pld.charPtr->questJournal.CheckInventoryItems(pld.charPtr->inventory);
}
*/

int SimulatorThread :: handle_query_creature_use(void)
{
	/*  Query: creature.use
		Sent when a usable creature is clicked on (quest giver/ender, quest objective
		gather/interact, etc)
		Args : [0] = Creature Instance ID
	*/
	creatureInst->RemoveNoncombatantStatus("creature_use");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	//Creatures cannot be interacted with in groves.
	if(creatureInst->actInst->mZoneDefPtr->mGrove == true)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	if(creatureInst->HasStatus(StatusEffects::DEAD))
	{
		SendInfoMessage("You must be alive.", INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	}

	creatureInst->CancelInvisibility();

	int CID = atoi(query.args[0].c_str());
	CreatureInstance *target = creatureInst->actInst->GetNPCInstanceByCID(CID);

	if(target == NULL)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	if(creatureInst->actInst->GetBoxRange(creatureInst, target) > INTERACT_RANGE)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	//LogMessageL(MSG_DIAG, "  Request creature.use for %d", CID);
	//int CDef = ResolveCreatureDef(CID);
	int CDef = target->CreatureDefID;

	if(target->HasStatus(StatusEffects::HENGE))
	{
		CreatureUseHenge(CID, CDef);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}

	//For any other interact besides henge, notify the instance on the off chance that it needs to do something.
	creatureInst->actInst->ScriptCallUse(creatureInst->CreatureID, CDef);

	int QuestID = 0;
	int QuestAct = 0;
	QuestObjective *qo = pld.charPtr->questJournal.CreatureUse(CDef, QuestID, QuestAct);
	if(qo == NULL)
	{
		//Not a quest interact, check for interactibles.
		InteractObject *intObj = g_InteractObjectContainer.GetObjectByID(CDef, pld.CurrentZoneID);
		if(intObj != NULL)
		{
			if(intObj->opType == InteractObject::TYPE_WARP || intObj->opType == InteractObject::TYPE_LOCATIONRETURN)
			{
				if(intObj->WarpID != pld.CurrentZoneID)
				{
					ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(intObj->WarpID);
					if(zoneDef != NULL)
					{
						PlayerInstancePlacementData pd;
						FillPlayerInstancePlacementData(pd, intObj->WarpID, 0);
						ActiveInstance *inst = g_ActiveInstanceManager.ResolveExistingInstance(pd, zoneDef);
						std::string outputMsg;
						if(inst != NULL)
						{
							if(inst->scaleConfig.IsScaled() == true)
							{
								outputMsg = "You are entering a scaled instance. Difficulty: ";
								if(inst->scaleProfile != NULL)
									outputMsg.append(inst->scaleProfile->mDifficultyName);
							}
						}
						else
						{
							if(zoneDef->IsMobScalable() == true && pd.in_scaleProfile != NULL)
							{
								outputMsg = "You are about to create a new scaled instance. Difficulty: ";
								outputMsg.append(pd.in_scaleProfile->mDifficultyName);
							}
						}
						if(outputMsg.size() > 0)
						{
							SendInfoMessage(outputMsg.c_str(), INFOMSG_INFO);
						}
					}
				}
			}

			creatureInst->LastUseDefID = CDef;
			int size = creatureInst->NormalInteractObject(SendBuf, intObj);
			if(size > 0)
				AttemptSend(SendBuf, size);

			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		}


		CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(target->CreatureDefID);
		if(cdef != NULL && ((cdef->DefHints & CDEF_HINT_USABLE) ||(cdef->DefHints & CDEF_HINT_USABLE_SPARKLY)))
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");


		return PrepExt_QueryResponseError(SendBuf, query.ID, "Cannot use object.");
	}

	int wpos = 0;
	if(qo->type == QuestObjective::OBJECTIVE_TYPE_ACTIVATE)
	{
		if(creatureInst->CurrentTarget.targ != NULL)
		{
			creatureInst->LastUseDefID = CDef;

			QuestScript::QuestNutPlayer *questNutScript = g_QuestNutManager.GetActiveScript(creatureInst->CreatureID, QuestID);
			if(questNutScript == NULL )
			{
				// Old script system
				questScript.def = &g_QuestScript;
				questScript.simCall = this;
				int wpos = creatureInst->QuestInteractObject(qo);
				sprintf(Aux1, "onActivate_%d_%d", QuestID, QuestAct);
				if(questScript.JumpToLabel(Aux1) == true)
				{
					questScript.mActive = true;
					questScript.nextFire = 0;
					questScript.sourceID = creatureInst->CreatureID;
					questScript.targetID = CID;
					questScript.targetCDef = CDef;
					questScript.QuestID = QuestID;
					questScript.QuestAct = QuestAct;
					questScript.actInst = creatureInst->actInst;
					questScript.RunFlags = 0;
					questScript.activateX = target->CurrentX;
					questScript.activateY = target->CurrentY;
					questScript.activateZ = target->CurrentZ;
					//questScript.RunScript();
					creatureInst->actInst->questScriptList.push_back(questScript);
				}
				//creatureInst->actInst->ScriptCallUse(CDef);
				AttemptSend(GSendBuf, wpos);
			}
			else
			{
				// New script system
				int wpos = creatureInst->QuestInteractObject(qo);
				questNutScript->target = target;
				questNutScript->QuestAct = QuestAct;
				questNutScript->activate.Set(target->CurrentX, target->CurrentY, target->CurrentZ);
				sprintf(Aux1, "on_activate_%d", QuestAct);
				questNutScript->JumpToLabel(Aux1);
				AttemptSend(GSendBuf, wpos);

				// Queue up the on_activated as well, this MIGHT not be get called if the activation is interrupted
				sprintf(Aux1, "on_activated_%d", QuestAct);
				questNutScript->activateEvent  =
						new ScriptCore::NutScriptEvent(
								new ScriptCore::TimeCondition(qo->ActivateTime),
								new ScriptCore::RunFunctionCallback(
										questNutScript, string(Aux1)));
				questNutScript->QueueAdd(questNutScript->activateEvent);

			}
		}
	}
	else
	{
		wpos += pld.charPtr->questJournal.CheckQuestTalk(&SendBuf[wpos], CDef, CID);
	}

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_query_visWeapon(void)
{
	/*  Query: visWeapon
		Sent to notify the server of a weapon visibility change, such as swapping
		melee and ranged.
	    Note: despite the nonstandard query name, this is an actual query and not
		a user command.
		Args : [0] = New visibility status (0, 1, or 2).
	*/
	if(query.argCount > 0)
	{
		int visState = query.GetInteger(0);
		creatureInst->css.vis_weapon = visState;
		int wpos = PrepExt_SendVisWeapon(SendBuf, creatureInst->CreatureID, visState);
		creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_quest_indicator(void)
{
	/*  Query: quest.indicator
		Part 1 of the quest accept procedure.
		The client is requesting if an NPC has a quest icon over its head, and if so,
		what kind.
		Args : [0] = Creature Instance ID
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	//Quests cannot be interacted with in groves.
	if(creatureInst->actInst->mZoneDefPtr->mGrove == true)
		return PrepExt_QueryResponseString(SendBuf, query.ID, QuestIndicator::QueryResponse[QuestIndicator::NONE]);

	int CID = atoi(query.args[0].c_str());
	//LogMessageL(MSG_DIAGV, "  Request quest.indicator for %d", CID);

	int CDef = ResolveCreatureDef(CID);

	const char *status = pld.charPtr->questJournal.QuestIndicator(CDef);
	//LogMessageL(MSG_SHOW, "  quest.indicator for %d (%d) = %s", CID, CDef, status);
	return PrepExt_QueryResponseString(SendBuf, query.ID, status);
}

int SimulatorThread :: handle_query_quest_getquestoffer(void)
{
	/*  Query: quest.getquestoffer
		Part 2 of the quest accept procedure.
		Sent when an NPC (with an available quest) is clicked.
		Returns the Quest ID of a new quest associated with this NPC.
		Args : [0] = Creature Instance ID
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int CID = atoi(query.args[0].c_str());
	LogMessageL(MSG_DIAGV, "  Request quest.getquestoffer for %d", CID);

	int CDef = ResolveCreatureDef(CID);

	char *response = pld.charPtr->questJournal.QuestGetQuestOffer(CDef, Aux3);
	LogMessageL(MSG_DIAGV, "  quest.getquestoffer for %d = %s", CID, response);
	return PrepExt_QueryResponseString(SendBuf, query.ID, response);
}


int SimulatorThread :: handle_query_quest_genericdata(void)
{
	/*  Query: quest.genericdata
		Part 3 of the quest accept procedure.
		This retrieves the quest data so that the player may review stuff like
		description, objectives, and rewards before accepting the quest.
		Args : [0] = Quest ID
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int QID = atoi(query.args[0].c_str());
	LogMessageL(MSG_DIAGV, "  Requested quest.genericdata for %d", QID);
	return pld.charPtr->questJournal.QuestGenericData(SendBuf, sizeof(SendBuf), Aux3, QID, query.ID);
}

int SimulatorThread :: handle_query_quest_join(void)
{
	/*  Query: quest.join
		Part 4 of the quest accept procedure.
		This query is followed by "quest.list" to refresh the active quest list.
		Signals the server that the player has reviewed and accepted the quest.
		Args : [0] = Quest ID
		       [1] = Creature Instance ID of the NPC that issued the quest.
	*/
	if(query.argCount < 2)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");  //Fail silently.

	int QuestID = query.GetInteger(0);
	int CID = query.GetInteger(1);
	CreatureInstance *giverNPC = ResolveCreatureInstance(CID, 0);
	if(giverNPC == NULL)
		return ErrorMessageAndQueryOK(SendBuf, "Quest giver does not exist.");

	if(ActiveInstance::GetPlaneRange(creatureInst, giverNPC, INTERACT_RANGE) > INTERACT_RANGE)
		return ErrorMessageAndQueryOK(SendBuf, "Quest giver not in range.");

	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qdef == NULL)
		return ErrorMessageAndQueryOK(SendBuf, "Quest does not exist.");
	
	if(qdef->QuestGiverID != giverNPC->CreatureDefID)
		return ErrorMessageAndQueryOK(SendBuf, "That object does not give that quest.");

	if(qdef->mScriptAcceptCondition.ExecuteAllCommands(this) < 0)
		return ErrorMessageAndQueryOK(SendBuf, "Cannot accept the quest yet.");
	qdef->mScriptAcceptAction.ExecuteAllCommands(this);

	LogMessageL(MSG_DIAGV, "  Request quest.join (QuestID: %d, CID: %d)", QuestID, CID);
	int wpos = pld.charPtr->questJournal.QuestJoin(SendBuf, QuestID, query.ID);

	g_QuestNutManager.AddActiveScript(creatureInst, QuestID);

	return wpos;
}

int SimulatorThread :: handle_query_quest_list(void)
{
	/*  Query: quest.list
		Sent whenever the client needs to refresh the active quest list.
		Args : [none]
	*/

	int wpos = pld.charPtr->questJournal.QuestList(SendBuf, Aux3, query.ID);
	return wpos;
}

int SimulatorThread :: handle_query_quest_data(void)
{
	/*  Query: quest.data
		Fetches the active quest data for a particular quest.  Often queried in
		response to "quest.list" or when objectives are updated.
		Args : [0] = Quest ID
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int QID = atoi(query.args[0].c_str());

	return pld.charPtr->questJournal.QuestData(SendBuf, Aux3, QID, query.ID);
}

int SimulatorThread :: handle_query_quest_getcompletequest(void)
{
	/*  Query: quest.getcompletequest
		Sent when a quest is redeemed. The result will allow it to display
		the quest ending screen, with completion text, reward selection, etc.
		Args : [0] = Creature ID of the NPC
	*/

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int CID = atoi(query.args[0].c_str());
	LogMessageL(MSG_DIAGV, "  Request quest.getcompletequest for %d", CID);

	int CDef = ResolveCreatureDef(CID);
	return pld.charPtr->questJournal.QuestGetCompleteQuest(SendBuf, Aux3, CDef, query.ID);
}

int SimulatorThread :: handle_query_quest_complete(void)
{
	/*  Query: quest.complete
		Sent when the quest is redeemed, possibly after a "creature.use" on the quest
		redeemer.  The quest window has closed, rewards are applied, and the quest
		is removed from the active list.
		Args : [0] = Quest ID
		       [1] = Creature Instance ID of NPC being interacted with
			   [2][...] = A variable number of arguments (but usually just 1) indicating the index
			         of any user-selected choice rewards.
	*/

	int QID = 0;
	int CID = 0;
	//int Reward = -1;  //Zero or above in the server reward processing indicates an item was selected from the reward list.

	std::vector<int> selectionList;

	if(query.argCount > 0)
		QID = query.GetInteger(0);
	if(query.argCount > 1)
		CID = query.GetInteger(1);
	if(query.argCount > 2)
	{
		for(int i = 2; i < query.argCount; i++)
			selectionList.push_back(query.GetInteger(i));
	}

	QuestJournal *qj = pld.GetQuestJournal();
	if(qj == NULL)
		return ErrorMessageAndQueryOK(SendBuf, "Critical server error.");

	CreatureInstance *questEnder = ResolveCreatureInstance(CID, 1);
	if(questEnder == NULL)
		return ErrorMessageAndQueryOK(SendBuf, "Server error: creature does not exist.");

	if(ActiveInstance::GetBoxRange(creatureInst, questEnder) > INTERACT_RANGE)
		return ErrorMessageAndQueryOK(SendBuf, "You are too far away from the quest ender.");

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QID);
	if(qd == NULL)
		return ErrorMessageAndQueryOK(SendBuf, "Quest not found.");

	if(qj->IsQuestRedeemable(qd, QID, questEnder->CreatureDefID) == false)
		return ErrorMessageAndQueryOK(SendBuf, "Quest cannot be redeemed yet.");

	std::vector<QuestItemReward> rewardedItems;
	if(qd->FilterSelectedRewards(selectionList, rewardedItems) != true)
		return ErrorMessageAndQueryOK(SendBuf, "Failed to select quest reward items.");

	int freeSlots = pld.charPtr->inventory.CountFreeSlots(INV_CONTAINER);
	if(rewardedItems.size() > static_cast<size_t>(freeSlots))
		return ErrorMessageAndQueryOK(SendBuf, "No free inventory space.");

	if(qd->mScriptCompleteCondition.ExecuteAllCommands(this) < 0)
		return ErrorMessageAndQueryOK(SendBuf, "Cannot redeem the quest yet.");
	qd->mScriptCompleteAction.ExecuteAllCommands(this);

	QuestScript::QuestNutPlayer *player =  g_QuestNutManager.GetActiveScript(creatureInst->CreatureID, QID);
	if(player != NULL) {
		player->RunFunction("on_complete", std::vector<ScriptCore::ScriptParam>(), false);
		player->HaltExecution();
	}


	/* OBSOLETE
	int questindex = QuestDef.GetQuestByID(QID);
	if(Reward == -1 && questindex >= 0)
	{
		//Check if there's a default quest reward that needs to be returned.
		if(QuestDef.defList[questindex].numRewards <= 1)
			if(QuestDef.defList[questindex].rewardItem[0].itemID != 0)
				Reward = 0;
	}

	if(Reward >= 0)
	{
		int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot == -1)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "No free inventory space.");
	}
	*/

	int response = pld.charPtr->questJournal.QuestComplete(QID);
	if(response == -1)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	response = creatureInst->ProcessQuestRewards(QID, rewardedItems);



	if(response < 0)
	{
		if(response == -2)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "No free inventory space.");

		return PrepExt_QueryResponseError(SendBuf, query.ID, "Unable to complete quest.");
	}

	int wpos = 0;

	wpos += PutByte(&SendBuf[wpos], 7);   //_handleQuestEventMsg
	wpos += PutShort(&SendBuf[wpos], 0);  //size
	wpos += PutInteger(&SendBuf[wpos], QID);
	wpos += PutByte(&SendBuf[wpos], QuestObjective::EVENTMSG_TURNEDIN);
	wpos += PutInteger(&SendBuf[wpos], CID);
	PutShort(&SendBuf[1], wpos -  3);  //size

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");

	return wpos;
}

int SimulatorThread :: handle_query_quest_leave(void)
{
	/* Query: quest.leave
	   Args : 1, [0] = Quest ID
	*/
	int QID = 0;
	if(query.argCount > 0)
		QID = query.GetInteger(0);

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QID);
	if(qd != NULL)
	{
		if(qd->unabandon == true)
		{
			SendInfoMessage("You cannot abandon that quest.", INFOMSG_ERROR);
			QID = 0;  //Set to zero so it's not actually removed in the server or the client.
		}
	}
	pld.charPtr->questJournal.QuestLeave(pld.CreatureID, QID);
	sprintf(Aux1, "%d", QID);

	return PrepExt_QueryResponseString(SendBuf, query.ID, Aux1);
}

int SimulatorThread :: handle_query_quest_hack(void)
{

	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	QuestDefinition *qdef;
	int QuestID = query.GetInteger(1);
	if(QuestID == 0) {
		qdef = QuestDef.GetQuestDefPtrByName(query.GetString(1));
	}
	else {
		qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	}
	if(qdef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Quest does not exist.");

	g_CharacterManager.GetThread("SimulatorThread::QuestHack");
	CreatureInstance *creature;
	if(strcmp(query.GetString(2), "SELECT_TARGET") == 0) {
		creature = creatureInst->CurrentTarget.targ;
		if(creature == NULL)
			creature = creatureInst;
	}
	else {
		creature = creatureInst->actInst->GetPlayerByID(query.GetInteger(2));
	}
	if(creature == NULL) {
		g_Log.AddMessageFormat("No creature for  quest hack op for quest %s and creature %s.", query.GetString(1), query.GetString(2));
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "Selected creature does not exist.");
	}
	else if(query.args[0].compare("add") == 0) {
		LogMessageL(MSG_DIAGV, "[SAGE] Quest join (QuestID: %d, CID: %d)", qdef->questID, creature->CreatureID);
		creature->simulatorPtr->QuestJoin(qdef->questID);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	else if(query.args[0].compare("remove") == 0) {
		LogMessageL(MSG_DIAGV, "[SAGE] Quest remove (QuestID: %d, CID: %d)", qdef->questID, creature->CreatureID);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		creature->simulatorPtr->QuestClear(qdef->questID);
	}
	else if(query.args[0].compare("complete") == 0) {
		LogMessageL(MSG_DIAGV, "[SAGE] Quest complete (QuestID: %d, CID: %d)", qdef->questID, creature->CreatureID);
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		int wpos = creature->charPtr->questJournal.ForceComplete(creature->CreatureID, qdef->questID, &Aux1[0]);
		creature->simulatorPtr->AttemptSend(Aux1, wpos);
	}
	else {
		g_Log.AddMessageFormat("Unknown quest hack op for quest %s and creature %s.", query.GetString(1), query.GetString(2));
		WritePos = PrepExt_QueryResponseError(SendBuf, query.ID, "Unknown quest hack op.");
	}
	g_CharacterManager.ReleaseThread();
	return WritePos;
}

int SimulatorThread :: handle_command_complete(void)
{
	/*  Query: complete
		Cheat to force completion of the current act for all active quests.
		Args : [none]
	*/
	if(CheckPermissionSimple(0, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission Denied");

	int wpos = pld.charPtr->questJournal.ForceAllComplete(creatureInst->CreatureID, SendBuf);
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}


int SimulatorThread :: handle_query_trade_start(void)
{
	/*  Query: trade.start
		Attempt to open a trade with another player.
		Args : [0] = Creature ID of the player to trade with.
	*/
	int rval = protected_helper_query_trade_start();
	if(rval <= 0)
	{
		rval = PrepExt_SendInfoMessage(SendBuf, GetErrorString(rval), INFOMSG_ERROR);
		rval += PrepExt_QueryResponseError(&SendBuf[rval], query.ID, "");
	}

	return rval;
}

int SimulatorThread :: protected_helper_query_trade_start(void)
{
	// Note: query errors don't seem to do anything in the client.

	// Note: The trade starter has an argument for the target player.
	//       When the target player accepts, there is no argument to this
	//       query.

	int selfPlayerID = creatureInst->CreatureID;
	int otherPlayerID = 0;
	if(query.argCount > 0)
		otherPlayerID = query.GetInteger(0);

	ActiveInstance *actInst = creatureInst->actInst;
	if(actInst == NULL)
	{
		g_Log.AddMessageFormat("[CRITICAL] trade.start active instance is NULL");
		return QueryErrorMsg::INVALIDOBJ;
	}

	CreatureInstance *target = NULL;
	if(otherPlayerID != 0)
	{
		//Make sure we're not busy.
		if(CanMoveItems() == false)
			return QueryErrorMsg::SELFBUSYSKILL;

		//Make sure the other player exists
		target = actInst->GetPlayerByID(otherPlayerID);
		if(target == NULL)
			return QueryErrorMsg::INVALIDOBJ;

		//Make sure the other player isn't busy
		if(actInst->tradesys.GetExistingTradeForPlayer(otherPlayerID) != NULL)
			return QueryErrorMsg::TRADEBUSY;

		if(target->simulatorPtr->CanMoveItems() == false)
			return QueryErrorMsg::OTHERBUSYSKILL;
	}

	//It only takes one player to start the trade for both, so initialize a
	//transaction for both players.
	int CheckID = creatureInst->activeLootID;
	if(CheckID == 0)
		CheckID = selfPlayerID;
	TradeTransaction *tradeData = actInst->tradesys.GetNewTransaction(CheckID);
	//tradeData->Clear();
	bool initiator = false;
	if(otherPlayerID != 0)
	{ 
		initiator = true;
		tradeData->SetPlayers(creatureInst, target);
		creatureInst->activeLootID = CheckID;
		target->activeLootID = CheckID;
		tradeData->init = true;
	}
	else
	{
		//This is the accepting player, so the target must be the origin.
		otherPlayerID = tradeData->player[0].selfPlayerID;
		target = tradeData->player[0].cInst;
	}

	if(target == NULL || otherPlayerID == 0)
		return QueryErrorMsg::INVALIDOBJ;

	tradeData->GetPlayerData(selfPlayerID)->tradeWindowOpen = true;

	if(initiator == true)
	{
		// Send trade request message to the other player.
		int wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&SendBuf[wpos], 0);
		wpos += PutInteger(&SendBuf[wpos], selfPlayerID);   //traderID
		wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST);    //eventType
		PutShort(&SendBuf[1], wpos - 3);
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
	}
	else
	{
		int wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&SendBuf[wpos], 0);
		wpos += PutInteger(&SendBuf[wpos], selfPlayerID);   //traderID
		wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_ACCEPTED);    //eventType
		PutShort(&SendBuf[1], wpos - 3);
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}



int SimulatorThread :: handle_query_trade_items(void)
{
	/*  Query: trade.items
		A player has offered an item to trade.
		Args : [variable] = List of items offered, Hex Inventory Item ID format.
	*/
	int rval = protected_helper_query_trade_items();
	if(rval <= 0)
	{
		SendInfoMessage(GetErrorString(rval), INFOMSG_INFO);
		rval = PrepExt_QueryResponseNull(SendBuf, query.ID);
	}
	return rval;
}

int SimulatorThread :: protected_helper_query_trade_items(void)
{
	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;

	ActiveInstance *actInst = creatureInst->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if(pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);

	if(pData->otherPlayerData->tradeWindowOpen == false)
		return QueryErrorMsg::TRADENOTOPENED;

	InventorySlot item;
	pData->itemList.clear();
	for(int a = 0; a < query.argCount; a++)
	{
		unsigned long CCSID = strtol(query.args[a].c_str(), NULL, 16);
		InventorySlot *itemPtr = pld.charPtr->inventory.GetItemPtrByCCSID(CCSID);
		if(itemPtr == NULL)
			return QueryErrorMsg::INVALIDITEM;

		item.CopyFrom(*itemPtr, false);
		item.CCSID = CCSID;
		pData->itemList.push_back(item);
	}

	CreatureInstance *cInst = pData->otherPlayerData->cInst;
	int wpos = PrepExt_TradeItemOffer(SendBuf, Aux3, selfID, pData->itemList);


	SendToOneSimulator(SendBuf, wpos, cInst->simulatorPtr);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_trade_cancel(void)
{
	/*  Query: trade.cancel
		A player has cancelled the trade.
		Args : [none]
	*/
	int rval = protected_helper_query_trade_cancel();
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_query_trade_cancel(void)
{
	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;
	ActiveInstance *actInst = creatureInst->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;
	actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_trade_offer(void)
{
	/*  Query: trade.offer
		A player has offered a coin amount.
		Args : [none]
	*/
	int rval = protected_helper_query_trade_offer();
	if(rval <= 0)
	{
		SendInfoMessage(GetErrorString(rval), INFOMSG_INFO);
		rval = PrepExt_QueryResponseNull(SendBuf, query.ID);
	}

	return rval;
}

int SimulatorThread :: protected_helper_query_trade_offer(void)
{
	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;
	ActiveInstance *actInst = creatureInst->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if(pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);   //traderID
	wpos += PutByte(&SendBuf[wpos], TradeEventTypes::OFFER_MADE);     //eventType
	PutShort(&SendBuf[1], wpos - 3);       //Set message size

	SendToOneSimulator(SendBuf, wpos, cInst->simulatorPtr);
	//actInst->LSendToOneSimulator(SendBuf, wpos, cInst->SimulatorIndex);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_trade_accept(void)
{
	/*  Query: trade.accept
		A player has accepted the trade offer.
		Args : [none]
	*/
	int rval = protected_helper_query_trade_accept();
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_query_trade_accept(void)
{
	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;
	ActiveInstance *actInst = creatureInst->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if(pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	pData->SetAccepted(true);

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 51);     //_handleTradeMsg
	wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
	wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);   //traderID
	wpos += PutByte(&SendBuf[wpos], TradeEventTypes::OFFER_ACCEPTED);     //eventType
	PutShort(&SendBuf[1], wpos - 3);       //Set message size
	SendToOneSimulator(SendBuf, wpos, cInst->simulatorPtr);
	//actInst->LSendToOneSimulator(SendBuf, wpos, cInst->SimulatorIndex);

	CreatureInstance *origin = tradeData->player[0].cInst;
	CreatureInstance *target = tradeData->player[1].cInst;
	if(origin == NULL || target == NULL)
		return QueryErrorMsg::INVALIDOBJ;

	if(tradeData->MutualAccept() == true)
	{
		//Process the trade.
		int wpos = 0;

		//When counting slots, get the currently free slots.
		//Then add the number of items that would be traded (given away).
		//This allows a currently full inventory to potentially receive items
		//after the transaction is processed.

		int oslots = origin->charPtr->inventory.CountFreeSlots(INV_CONTAINER);
		oslots += tradeData->player[0].itemList.size();

		int tslots = target->charPtr->inventory.CountFreeSlots(INV_CONTAINER);
		tslots += tradeData->player[1].itemList.size();

		if(oslots < (int)tradeData->player[1].itemList.size())
		{
			//Origin player does not have enough space to receive items.
			wpos = 0;
			wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&SendBuf[wpos], origin->CreatureID);     //traderID
			wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
			wpos += PutByte(&SendBuf[wpos], CloseReasons::INSUFFICIENT_SPACE);     //eventType
			PutShort(&SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Log.AddMessageFormat("Origin lacks space\n");
			goto exit;
		}
		if(tslots < (int)tradeData->player[0].itemList.size())
		{
			//Target player does not have enough space to receive items.
			wpos = 0;
			wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&SendBuf[wpos], target->CreatureID);     //traderID
			wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
			wpos += PutByte(&SendBuf[wpos], CloseReasons::INSUFFICIENT_SPACE);     //eventType
			PutShort(&SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Log.AddMessageFormat("Target lacks space\n");
			goto exit;
		}

		//Check that each player has the required currencies.
		if(tradeData->player[0].coin > origin->css.copper)
		{
			wpos = 0;
			wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&SendBuf[wpos], origin->CreatureID);     //traderID
			wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
			wpos += PutByte(&SendBuf[wpos], CloseReasons::INSUFFICIENT_FUNDS);     //eventType
			PutShort(&SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Log.AddMessageFormat("Origin lacks copper\n");
			goto exit;
		}
		if(tradeData->player[1].coin > target->css.copper)
		{
			wpos = 0;
			wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
			wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
			wpos += PutInteger(&SendBuf[wpos], target->CreatureID);     //traderID
			wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
			wpos += PutByte(&SendBuf[wpos], CloseReasons::INSUFFICIENT_FUNDS);     //eventType
			PutShort(&SendBuf[1], wpos - 3);       //Set message size
			SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
			SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
			//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
			//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
			g_Log.AddMessageFormat("Target lacks copper\n");
			goto exit;
		}

		//Ready to trade.
		g_Log.AddMessageFormat("Trade requirements passed\n");


		//Adjust and send coin transfer to both players.
		origin->css.copper -= tradeData->player[0].coin;
		target->css.copper -= tradeData->player[1].coin;

		origin->css.copper += tradeData->player[1].coin;
		target->css.copper += tradeData->player[0].coin;

		static const short statSend = STAT::COPPER;
		wpos = PrepExt_SendSpecificStats(SendBuf, origin, &statSend, 1);
		SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);

		wpos = PrepExt_SendSpecificStats(SendBuf, target, &statSend, 1);
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);

		//Adjust and send items for first player.

		//Remove items from first player.
		wpos = 0;
		CharacterData *p1 = origin->charPtr;
		CharacterData *p2 = target->charPtr;
		g_Log.AddMessageFormat("[DEBUG] Trade betweeen [%s] and [%s]", p1->cdef.css.display_name, p2->cdef.css.display_name);
		for(size_t a = 0; a < tradeData->player[0].itemList.size(); a++)
		{
			unsigned long CCSID = tradeData->player[0].itemList[a].CCSID;
			InventorySlot *item = p1->inventory.GetItemPtrByCCSID(CCSID);
			if(item == NULL)
			{
				g_Log.AddMessageFormat("Failed to remove item from first player.");
			}
			else
			{
				wpos += p1->inventory.RemoveItemUpdate(&SendBuf[wpos], Aux3, item);
				p1->inventory.RemItem(CCSID);
			}
		}
		SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
		g_Log.AddMessageFormat("Removed %d items from first player.", tradeData->player[0].itemList.size());


		//Remove items from second player.
		wpos = 0;
		for(size_t a = 0; a < tradeData->player[1].itemList.size(); a++)
		{
			unsigned long CCSID = tradeData->player[1].itemList[a].CCSID;
			InventorySlot *item = p2->inventory.GetItemPtrByCCSID(CCSID);
			if(item == NULL)
			{
				g_Log.AddMessageFormat("Failed to remove item from first player.");
			}
			else
			{
				wpos += p2->inventory.RemoveItemUpdate(&SendBuf[wpos], Aux3, item);
				p2->inventory.RemItem(CCSID);
			}
		}
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
		g_Log.AddMessageFormat("Removed %d items from second player.", tradeData->player[1].itemList.size());


		//Give items to first player
		wpos = 0;
		for(size_t a = 0; a < tradeData->player[1].itemList.size(); a++)
		{
			int itemID = tradeData->player[1].itemList[a].IID;
			int count = tradeData->player[1].itemList[a].count + 1;
			InventorySlot *item = p1->inventory.AddItem_Ex(INV_CONTAINER, itemID, count);
			if(item == NULL)
				g_Log.AddMessageFormat("Failed to add item to first player.");
			else
			{
				Debug::Log("[TRADE] From %s to %s (%d)", tradeData->player[1].cInst->css.display_name, tradeData->player[0].cInst->css.display_name, item->IID);
				item->CopyWithoutCount(tradeData->player[1].itemList[a], false);
				wpos += AddItemUpdate(&SendBuf[wpos], Aux3, item);
			}
		}
		SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);
		g_Log.AddMessageFormat("Gave %d items to first player.", tradeData->player[1].itemList.size());


		//Give items to second player
		wpos = 0;
		for(size_t a = 0; a < tradeData->player[0].itemList.size(); a++)
		{
			int itemID = tradeData->player[0].itemList[a].IID;
			int count = tradeData->player[0].itemList[a].count + 1;
			InventorySlot *item = p2->inventory.AddItem_Ex(INV_CONTAINER, itemID, count);
			if(item == NULL)
				g_Log.AddMessageFormat("Failed to add item to second player.");
			else
			{
				Debug::Log("[TRADE] From %s to %s (%d)", tradeData->player[0].cInst->css.display_name, tradeData->player[1].cInst->css.display_name, item->IID);
				item->CopyWithoutCount(tradeData->player[0].itemList[a], false);
				wpos += AddItemUpdate(&SendBuf[wpos], Aux3, item);
			}
		}
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);
		g_Log.AddMessageFormat("Gave %d items to second player.", tradeData->player[0].itemList.size());

		//Send trade completion message.
		wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 51);    //_handleTradeMsg
		wpos += PutShort(&SendBuf[wpos], 0);    //Placeholder for size
		wpos += PutInteger(&SendBuf[wpos], origin->CreatureID);     //traderID
		wpos += PutByte(&SendBuf[wpos], TradeEventTypes::REQUEST_CLOSED);     //eventType
		wpos += PutByte(&SendBuf[wpos], CloseReasons::COMPLETE);     //eventType
		PutShort(&SendBuf[1], wpos - 3);       //Set message size

		SendToOneSimulator(SendBuf, wpos, origin->simulatorPtr);
		//origin->actInst->LSendToOneSimulator(SendBuf, wpos, origin->SimulatorIndex);

		PutInteger(&SendBuf[3], target->CreatureID);     //traderID
		SendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
		//target->actInst->LSendToOneSimulator(SendBuf, wpos, target->SimulatorIndex);

		//Clear trade IDs.
		origin->activeLootID = 0;
		target->activeLootID = 0;

		g_Log.AddMessageFormat("Trade complete\n");
		actInst->tradesys.RemoveTransaction(tradeID);
	}

	//Yes, I'm using goto.
	//Yes, I know this whole thing is badly programmed.
	//Deal with it.

exit:
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_trade_currency(void)
{
	/*  Query: trade.currency
	    Sent when the player offers a coin amount.
		Args : [0] currency name, ex: "COPPER"
		       [1] amount offered
		*/
	int rval = protected_helper_query_trade_currency();
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_query_trade_currency(void)
{
	if(query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	//Ignore currency type for now, assume always copper.  Just get the amount.
	// Note: I found no evidence that the credit currency could actually be used
	// in trades.
	//args[0] is usually COPPER
	int amount = strtol(query.args[1].c_str(), NULL, 10);

	int selfID = creatureInst->CreatureID;
	int tradeID = creatureInst->activeLootID;
	ActiveInstance *actInst = creatureInst->actInst;
	TradeTransaction *tradeData = actInst->tradesys.GetExistingTransaction(tradeID);
	if(tradeData == NULL)
		return QueryErrorMsg::TRADENOTFOUND;

	TradePlayerData *pData = tradeData->GetPlayerData(selfID);
	if(pData == NULL)
		return actInst->tradesys.CancelTransaction(selfID, tradeID, SendBuf);

	CreatureInstance *cInst = pData->otherPlayerData->cInst;

	pData->SetCoin(amount);

	if(tradeData->player->otherPlayerData->tradeWindowOpen == true)
	{
		int wpos = PrepExt_TradeCurrencyOffer(SendBuf, selfID, amount);
		SendToOneSimulator(SendBuf, wpos, cInst->simulatorPtr);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_craft_create(void)
{
	/*  Query: craft.create
	    Sent when an item is crafted via the crafting NPC.
		Args : [0] Item ID of the plan to create from.
		       [1] Creature Instance ID of the vendor who processed this action.
		*/
	int rval = protected_helper_query_craft_create();
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_query_craft_create(void)
{
	if(query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	int ItemID = atoi(query.args[0].c_str());
	int CreatureID = atoi(query.args[1].c_str());

	// Make sure this object isn't too far away.
	int distCheck = protected_helper_checkdistance(CreatureID);
	if(distCheck != 0)
		return distCheck;

	//Check that the item plan exists in the database.
	ItemDef *itemPlan = g_ItemManager.GetPointerByID(ItemID);
	if(itemPlan == NULL)
		return QueryErrorMsg::INVALIDITEM;

	//Check that the resulting item exists in the database.
	ItemDef *resultItem = g_ItemManager.GetPointerByID(itemPlan->resultItemId);
	if(resultItem == NULL)
		return QueryErrorMsg::INVALIDITEM;

	//Create an alias to make the code a bit cleaner.
	InventoryManager &inv = pld.charPtr->inventory;

	// Check that the crafting component exists in the player's inventory.
	int count = inv.GetItemCount(INV_CONTAINER, itemPlan->keyComponentId);
	if(count < 1)
		return QueryErrorMsg::INVMISSING;

	//Build a list of component counts.
	std::vector<int> CraftID;
	std::vector<int> CraftReq;
	for(size_t i = 0; i < itemPlan->craftItemDefId.size(); i++)
	{
		int materialID = itemPlan->craftItemDefId[i];

		bool bFound = false;
		for(size_t s = 0; s < CraftID.size(); s++)
		{
			if(CraftID[s] == materialID)
			{
				bFound = true;
				CraftReq[s]++;
				break;
			}
		}
		if(bFound == false)
		{
			CraftID.push_back(materialID);
			CraftReq.push_back(1);
		}
	}

	//Check that the player has the required components.
	for(size_t i = 0; i < CraftID.size(); i++)
	{
		ItemDef *craftItem = g_ItemManager.GetPointerByID(CraftID[i]);
		if(craftItem == NULL)
			return QueryErrorMsg::INVALIDITEM;
		count = inv.GetItemCount(INV_CONTAINER, CraftID[i]);
		if(count < CraftReq[i])
			return QueryErrorMsg::INVMISSING;
	}

	//Check that the player has a free slot.
	int slotIndex = inv.GetFreeSlot(INV_CONTAINER);
	if(slotIndex == -1)
		return QueryErrorMsg::INVSPACE;

	//Good to go, create the item.
	InventorySlot *itemPtr = NULL;
	itemPtr = inv.AddItem_Ex(INV_CONTAINER, itemPlan->resultItemId, 1);
	if(itemPtr == NULL)
		return QueryErrorMsg::INVCREATE;

	int wpos = AddItemUpdate(SendBuf, Aux3, itemPtr);

	//Remove the crafting materials.
	for(size_t i = 0; i < CraftID.size(); i++)
		wpos += inv.RemoveItemsAndUpdate(INV_CONTAINER, CraftID[i], CraftReq[i], &SendBuf[wpos]);

	vector<InventoryQuery> iq;

	//Remove the crafting component.
	inv.ScanRemoveItems(INV_CONTAINER, itemPlan->keyComponentId, 1, iq);
	if(iq.size() > 0)
		wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, iq[0].ptr);
	inv.RemoveItems(INV_CONTAINER, iq);

	//Remove the plan.
	inv.ScanRemoveItems(INV_CONTAINER, itemPlan->mID, 1, iq);
	if(iq.size() > 0)
		wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, iq[0].ptr);
	inv.RemoveItems(INV_CONTAINER, iq);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_query_item_morph(void)
{
	/*  Query: item.morph
	    Sent when an item is refashioned via the NPC.
		Args : [0] Item Inventory Hex ID (Original Item)
		       [1] Item Inventory Hex ID (New Equipment Look)
			   [2] = Creature Instance ID of the vendor who processed this action.
			   */
	int rval = protected_helper_query_item_morph(false);
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_query_item_morph(bool command)
{
	LogMessageL(MSG_SHOW, "protected_helper_query_item_morph: %d", command);
	if(query.argCount < 3)
		return QueryErrorMsg::GENERIC;

	unsigned long origLook = strtol(query.args[0].c_str(), NULL, 16);
	unsigned long newLook = strtol(query.args[1].c_str(), NULL, 16);
	int creatureID = strtol(query.args[2].c_str(), NULL, 10);

	// Run a distance check for a normal query.  The /refashion command
	// also uses this function but with a spoofed query.
	if(command == false)
	{
		// Make sure this object isn't too far away.
		int distCheck = protected_helper_checkdistance(creatureID);
		if(distCheck != 0)
			return distCheck;
	}

	InventorySlot * origPtr = pld.charPtr->inventory.GetItemPtrByCCSID(origLook);
	InventorySlot * newPtr = pld.charPtr->inventory.GetItemPtrByCCSID(newLook);
	if(origPtr == NULL || newPtr == NULL)
		return QueryErrorMsg::INVMISSING;

	ItemDef *itemPtr1 = g_ItemManager.GetPointerByID(origPtr->IID);
	ItemDef *itemPtr2 = g_ItemManager.GetPointerByID(newPtr->IID);
	if(itemPtr1 == NULL || itemPtr2 == NULL)
		return QueryErrorMsg::INVALIDITEM;

	int cost = (itemPtr1->mValue + itemPtr2->mValue) / 2;
	if(creatureInst->css.copper < cost)
		return QueryErrorMsg::COIN;
	creatureInst->css.copper -= cost;

	//Log just before the actual refashion so we can print the look ID before it's modified.
	Debug::Log("[REFASHION] %s : CCSID Orig:%lu (%d), Look:%lu (%d)", creatureInst->css.display_name, origLook, origPtr->IID, newLook, newPtr->IID);
	Debug::Log("[REFASHION] %s : Look %s (%d, %d) applied to %s (%d, %d)", creatureInst->css.display_name, itemPtr1->mDisplayName.c_str(), itemPtr1->mID, origPtr->GetLookID(), itemPtr2->mDisplayName.c_str(), itemPtr2->mID, newPtr->GetLookID());

	origPtr->customLook = newPtr->GetLookID();
	if(origPtr->customLook >= UNREFASHION_LOWERBOUND && origPtr->customLook <= UNREFASHION_UPPERBOUND)
		origPtr->customLook = 0;

	int wpos = 0;

	static const short statSend = STAT::COPPER;
	wpos += PrepExt_SendSpecificStats(&SendBuf[wpos], creatureInst, &statSend, 1);

	wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, newPtr);
	pld.charPtr->inventory.RemItem(newLook);
	wpos += AddItemUpdate(&SendBuf[wpos], Aux3, origPtr);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_refashion(void)
{
	/*  Query: refashion
	    Cheat to refashion an item directly from your inventory slots (also works
		for weapons).  Erases the query and substitutes a new one so that the normal
		refashion function can process the request.
		Args : [0] Exact name of source item (for user verification).
	*/

	sprintf(Aux3, "%d", InternalID + 1);

	bool error = false;
	if(query.argCount < 1)
		error = true;
	else
		if(query.args[0].compare(Aux3) != 0)
			error = true;

	InventorySlot *slot1 = NULL;
	InventorySlot *slot2 = NULL;
	ItemDef *itemPtr1 = NULL;
	ItemDef *itemPtr2 = NULL;
	unsigned long origCCSID = (INV_CONTAINER << 16) | 0;
	unsigned long lookCCSID = (INV_CONTAINER << 16) | 1;


	slot1 = pld.charPtr->inventory.GetItemPtrByCCSID(origCCSID);
	slot2 = pld.charPtr->inventory.GetItemPtrByCCSID(lookCCSID);
	if(slot1 != NULL)
		itemPtr1 = g_ItemManager.GetPointerByID(slot1->IID);
	if(slot2 != NULL)
		itemPtr2 = g_ItemManager.GetPointerByID(slot2->IID);

	if(error == true)
	{
		Util::SafeFormat(Aux3, sizeof(Aux3), "Stat item in first slot, new look in second.  Use [/refashion %d] to confirm.", InternalID + 1);
		SendInfoMessage(Aux3, INFOMSG_INFO);
		if(itemPtr1 != NULL && itemPtr2 != NULL)
		{
			Util::SafeFormat(Aux3, sizeof(Aux3), "%s will be refashioned to look like %s.", itemPtr1->mDisplayName.c_str(), itemPtr2->mDisplayName.c_str());
			SendInfoMessage(Aux3, INFOMSG_INFO);
			Util::SafeFormat(Aux3, sizeof(Aux3), "%s will then be destroyed!", itemPtr2->mDisplayName.c_str());
			SendInfoMessage(Aux3, INFOMSG_ERROR);
		}
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}

	if(slot1 == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must have an item in the first slot.");
	if(slot2 == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must have an item in the second slot.");

	if(itemPtr1 == NULL || itemPtr2 == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "An item does not exist in the server database.");

	bool sameType = true;
	if(itemPtr1->mType != itemPtr2->mType)
		sameType = false;
	if(itemPtr1->mEquipType != itemPtr2->mEquipType)
		sameType = false;
	if(itemPtr1->mType != itemPtr2->mType)
		sameType = false;

	if(itemPtr1->mType == 2)
		if(itemPtr1->mWeaponType != itemPtr2->mWeaponType)
			sameType = false;

	if(sameType == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Items must be equivalent type (including weapon class).");

	//Clear the arguments only.  The ID must be retained for a proper query response.
	query.args.clear();
	sprintf(Aux3, "%X", (uint)origCCSID);
	query.args.push_back(Aux3);

	sprintf(Aux3, "%X", (uint)lookCCSID);
	query.args.push_back(Aux3);

	query.args.push_back("0");
	query.argCount = query.args.size();

	//Run the virtual query.
	int rval = protected_helper_query_item_morph(true);
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_checkdistance(int creatureID)
{
	//Could only be called from within a guarded thread operation.
	if(creatureInst == NULL)
		return QueryErrorMsg::GENERIC;
	if(creatureInst->actInst == NULL)
		return QueryErrorMsg::GENERIC;

	CreatureInstance *object = creatureInst->actInst->GetNPCInstanceByCID(creatureID);
	if(object == NULL)
		return QueryErrorMsg::INVALIDOBJ;
	int dist = creatureInst->actInst->GetBoxRange(creatureInst, object);
	if(dist > INTERACT_RANGE)
		return QueryErrorMsg::OUTOFRANGE;

	return 0;
}

int SimulatorThread :: handle_command_backup(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	pld.charPtr->originalAppearance = creatureInst->css.appearance;
	int wpos = 0;
	wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "Appearance saved.  Use /restore1 to reload this appearance when necessary.", INFOMSG_INFO);
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_restore(void)
{
	//if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
	//	return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	//Restore a player's appearance based on their original string.
	int wpos = 0;
	if(pld.charPtr->originalAppearance.size() == 0)
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "Your character does not have an appearance backup.", INFOMSG_ERROR);
	else
	{
		//Util::SafeCopy(pld.charPtr->cdef.css.appearance, pld.charPtr->originalAppearance.c_str(), sizeof(pld.charPtr->cdef.css.appearance));
		//Util::SafeCopy(creatureInst->css.appearance, pld.charPtr->originalAppearance.c_str(), sizeof(creatureInst->css.appearance));
		pld.charPtr->cdef.css.SetAppearance(pld.charPtr->originalAppearance.c_str());
		creatureInst->css.SetAppearance(pld.charPtr->originalAppearance.c_str());

		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "Appearance restored.", INFOMSG_INFO);
		AddMessage((long)creatureInst, 0, BCM_UpdateAppearance);
	}
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}


int SimulatorThread :: handle_command_god(void)
{
	/*  Query: god
	    Cheat to enable or disable auto aggro hostility between players and mobs.
		Args : [0] zero or nonzero
	*/
	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int wpos = 0;
	if(query.argCount > 0)
	{
		int mode = atoi(query.args[0].c_str());
		if(mode != 0)
		{
			creatureInst->_AddStatusList(StatusEffects::INVINCIBLE, -1);
			SendInfoMessage("God mode ON", INFOMSG_INFO);
		}
		else
		{
			creatureInst->_RemoveStatusList(StatusEffects::INVINCIBLE);
			SendInfoMessage("God mode OFF", INFOMSG_INFO);
		}
	}
	wpos += ::PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_setstat(void)
{
	/*  Query: setstat
	    Cheat to set any object's stats (must be selected).
		Args : [0] stat name
		       [1] stat value
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
	{
		SendInfoMessage("Usage: select target and use /setstat statName newValue", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}

	CreatureInstance *targ = creatureInst->CurrentTarget.targ;
	if(targ == NULL)
		targ = creatureInst;

	const char *statName = query.args[0].c_str();
	const char *statValue = query.args[1].c_str();
	WriteStatToSetByName(statName, statValue, &targ->css);
	if(targ == creatureInst)
		WriteStatToSetByName(statName, statValue, &pld.charPtr->cdef.css);
	AddMessage((long)targ, 0, BCM_UpdateCreatureInstance);
	AddMessage((long)&pld.charPtr->cdef, 0, BCM_UpdateCreatureDef);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_adjustexp(void)
{
	/*  Query: adjustexp
	    Cheat to adjust the player's experience.
		Args : [0] amount
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
	{
		SendInfoMessage("Usage: /adjustexp amount", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}

	int amount = atoi(query.args[0].c_str());
	creatureInst->AddExperience(amount);
	if(amount < 0)
	{
		for(int i = 0; i < MAX_LEVEL; i++)
		{
			if(creatureInst->css.experience >= LevelRequirements[i][1])
			{
				creatureInst->css.level = i;
				break;
			}
		}
	}
	AddMessage((long)creatureInst, 0, BCM_UpdateCreatureInstance);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_command_scale(void)
{
	/*  Query: scale
	    Command to adjust the individual axis scales of a prop.
		Args : [0] = propid
		       [1] = scale   OR   [1] = x, [2] = y, [3] = z
	*/

	int rval = protected_helper_command_scale();
	if(rval <= 0)
		rval = PrepExt_QueryResponseError(SendBuf, query.ID, GetErrorString(rval));

	return rval;
}

int SimulatorThread :: protected_helper_command_scale(void)
{
	//"Usage: /scale propid scale
	//        /scale propid x y z

	if(query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	int PropID = atoi(query.args[0].c_str());
	float scalex = (float)atof(query.args[1].c_str());
	float scaley = scalex;
	float scalez = scalex;
	if(query.argCount >= 4)
	{
		scaley = (float)atof(query.args[2].c_str());
		scalez = (float)atof(query.args[3].c_str());
	}
	SceneryObject prop;
	SceneryObject *propPtr = NULL;

	g_SceneryManager.GetThread("SimulatorThread::protected_helper_command_scale");
	//propPtr = g_SceneryManager.GetPropPtr(pld.CurrentZoneID, PropID, NULL);
	propPtr = g_SceneryManager.GlobalGetPropPtr(pld.CurrentZoneID, PropID, NULL);
	if(propPtr != NULL)
		prop.copyFrom(propPtr);
	g_SceneryManager.ReleaseThread();

	if(propPtr == NULL)
		return QueryErrorMsg::PROPNOEXIST;

	//Check location permission
//	int tx = (int)(propPtr->LocationX / pld.zoneDef->mPageSize);
//	int ty = (int)(propPtr->LocationZ / pld.zoneDef->mPageSize);

	if(HasPropEditPermission(propPtr) == false)
		return QueryErrorMsg::PROPLOCATION;

	/*  OLD
	if(pld.accPtr->CheckBuildPermission(pld.CurrentZoneID, tx, ty) == false)
		return QueryErrorMsg::PROPLOCATION;
	*/

	if(scalex == 0.0F)
		scalex = 1.0F;
	if(scaley == 0.0F)
		scaley = 1.0F;
	if(scalez == 0.0F)
		scalez = 1.0F;

	prop.ScaleX = scalex;
	prop.ScaleY = scaley;
	prop.ScaleZ = scalez;

	g_SceneryManager.GetThread("SimulatorThread::protected_helper_command_scale");
	g_SceneryManager.ReplaceProp(pld.CurrentZoneID, prop);
	g_SceneryManager.ReleaseThread();

	int wpos = PrepExt_UpdateScenery(SendBuf, &prop);
	//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_shard_list(void)
{
	/*  Query: shard.list
		Requests a list of shards for use in the minimap dropdown list.
	*/
	return PrepExt_QueryResponseString(SendBuf, query.ID, pld.zoneDef->mName.c_str());
}

int SimulatorThread :: handle_query_sidekick_notifyexp(void)
{
	/*  Query: sidekick.notifyexp
		Only sent in 0.8.9.  Requests the sidekick experience that was gained
		while the user was offline.
	*/
	return PrepExt_QueryResponseString(SendBuf, query.ID, "0");
}

int SimulatorThread :: handle_query_ab_respec_price(void)
{
	/*  Query: ab.respec.price
		Only sent in 0.8.9.  Seems to be a different way of fetching the
		respec cost.
	*/
	return PrepExt_QueryResponseString(SendBuf, query.ID, "0");
}

int SimulatorThread :: handle_query_util_version(void)
{
	/*  Query: util.version
		Args: [none]
	*/

	// The client seems to expect a list of rows, each with two strings for name
	// and value.
	return PrepExt_QueryResponseString2(SendBuf, query.ID, "Build", VersionString);
}

int SimulatorThread :: handle_query_persona_resCost(void)
{
	/*  Query: persona.resCost
		Sent when the player dies, to determine the cost of each resurrection option.
		Args: [none]
	*/
	//For the three resurrect options [0,1,2], the client only asks for [1,2]
	int cost = 0;

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);           //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);          //Placeholder for message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index
	wpos += PutShort(&SendBuf[wpos], 2);           //Number of arrays

	cost = Global::GetResurrectCost(creatureInst->css.level, 1);
	Util::SafeFormat(Aux3, sizeof(Aux3), "%d", cost);
	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutStringUTF(&SendBuf[wpos], Aux3);

	cost = Global::GetResurrectCost(creatureInst->css.level, 2);
	Util::SafeFormat(Aux3, sizeof(Aux3), "%d", cost);
	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutStringUTF(&SendBuf[wpos], Aux3);

	PutShort(&SendBuf[1], wpos - 3);             //Set message size
	return wpos;
}

int SimulatorThread :: handle_query_clan_info(void)
{
	/*  Query: clan.info
		Retrieves the clan info of the Simulator player.
		Args: [none]
	*/
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

	wpos += PutShort(&SendBuf[wpos], 3);         //Row count

	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutStringUTF(&SendBuf[wpos], g_ClanName);   //Clan name

	wpos += PutByte(&SendBuf[wpos], 1);          
	wpos += PutStringUTF(&SendBuf[wpos], g_ClanMOTD);   //Message of the day

	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutStringUTF(&SendBuf[wpos], g_ClanLeader); //Clan leader's name.

	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}

int SimulatorThread :: handle_query_clan_list(void)
{
	/*  Query: clan.list
		Retrieves the list of clan members for the player's clan.
		Args: [none]
	*/

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);           //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

	// Number of rows (number of clan members)
	wpos += PutShort(&SendBuf[wpos], 0);

	/*	For each member, 5 elements per row:
		[0] = Character Name
		[1] = Level
		[2] = Profession (ex: "1", "2", "3", "4")
		[3] = Online Status (ex: "true" or "false")
		[4] = Arbitrary Rank Title (ex: "Leader", "Officer")
	*/
	
	int a, b;
//	for(a = 0; a < g_ClanMemberCount; a++)
//	{
//		wpos += PutByte(&SendBuf[wpos], 5);   //Five data items per character
//		for(b = 0; b < 5; b++)
//			wpos += PutStringUTF(&SendBuf[wpos], g_ClanMemberList[a][b]);
//	}
	
	PutShort(&SendBuf[1], wpos - 3);             //Set message size
	return wpos;
}

int SimulatorThread :: handle_query_spawn_list(void)
{
	/*  Query: spawn.list
		Retrieves a list of spawn types that match the search query.
		Args: [0] = Category (ex: "", "Creatures", "Packages", "NoAppearanceCreatures"
		      [1] = Search text to match.
	*/

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	const char *category = query.args[0].c_str();
	const char *search = query.args[1].c_str();

	//Response for each creature
	//  {id=row[0],name=row[1],type=mTypeMap[row[2]]};
	//  type should be "C" for Creature or "P" for Package.

	vector<int> resList;
	CreatureDef.GetSpawnList(category, search, resList);

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);          //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);         //Placeholder for message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index
	wpos += PutShort(&SendBuf[wpos], resList.size());         //Number of rows
	for(size_t a = 0; a < resList.size(); a++)
	{
		//3 elements: ID, Name (Level), Package Type
		wpos += PutByte(&SendBuf[wpos], 3);

		sprintf(Aux3, "%d", CreatureDef.NPC[resList[a]].CreatureDefID);
		wpos += PutStringUTF(&SendBuf[wpos], Aux3);

		sprintf(Aux3, "%s (%d)", CreatureDef.NPC[resList[a]].css.display_name, CreatureDef.NPC[resList[a]].css.level);
		wpos += PutStringUTF(&SendBuf[wpos], Aux3);

		wpos += PutStringUTF(&SendBuf[wpos], "C");
	}

	PutShort(&SendBuf[1], wpos - 3);             //Set message size
	resList.clear();
	return wpos;
}


int SimulatorThread :: handle_query_spawn_create(void)
{
	/*  Query: spawn.create
		Sent when a spawn is requested by /creaturebrowse
		Args: [0] = Object Type (ex: "CREATURE")
		      [1] = CreatureDef ID.
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	//For now, just ignore the object type and spawn the desired creature def.
	int CDef = atoi(query.args[1].c_str());
	AddMessage((long)creatureInst, CDef, BCM_SpawnCreateCreature);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_creature_delete(void)
{
	/*  Query: creature.delete
		Sent when a selected creature is deleted while in build mode (/togglebuilding)
		Args: [0] = Creature ID
		      [1] = Operation Type (ex: "PERMANENT")
	*/

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int CreatureID = query.GetInteger(0);
	CreatureInstance *creature = creatureInst->actInst->GetNPCInstanceByCID(CreatureID);
	if(creature != NULL)
	{
		if(HasPropEditPermission(NULL, (float)creature->CurrentX, (float)creature->CurrentZ) == true)
		{
			AddMessage(CreatureID, 0, BCM_CreatureDelete);
		}
	}

	/* OLD
	if(pld.accPtr->CheckBuildPermissionAdv(pld.CurrentZoneID, pld.zoneDef->mPageSize, (float)creatureInst->CurrentX, (float)creatureInst->CurrentZ) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Creature location is off limits.");
	*/
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_build_template_list(void)
{
	/*  Query: build.template.list
		Sent when build mode is activated in the client.  Only sent for the first
		activation.
		Not sure what the purpose of this data is for.  It seems to form a list
		of valid searchable templates.
		Args: [none]
	*/

	//The client script code indicates that only one row is used, but with a variable
	//number of elements.
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);           //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);    //Query response index
	wpos += PutShort(&SendBuf[wpos], 1);           //Array count
	wpos += PutByte(&SendBuf[wpos], 3);            //String count
	wpos += PutStringUTF(&SendBuf[wpos], "Crate1");  //String data
	wpos += PutStringUTF(&SendBuf[wpos], "Crate2");  //String data
	wpos += PutStringUTF(&SendBuf[wpos], "Crate3");  //String data
	PutShort(&SendBuf[1], wpos - 3);               //Set message size
	return wpos;
}

int SimulatorThread :: handle_query_spawn_property(void)
{
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Query error.");

	//LogMessageL(MSG_SHOW, "spawn.property");
	//for(int i = 0; i < query.argCount; i++)
	//	LogMessageL(MSG_SHOW, "[%d]=[%s]", i, query.args[i].c_str());
		
	int propID = atoi(query.args[0].c_str());
	const char *propName = query.args[1].c_str();
	g_SceneryManager.GetThread("SimulatorThread::handle_query_spawn_property");
	//SceneryObject *so = g_SceneryManager.pageData.GlobalFindProp(propID, pld.CurrentZoneID, NULL);
	SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(pld.CurrentZoneID, propID, NULL);
	Aux1[0] = 0;
	if(so != NULL)
	{
		if(so->extraData != NULL)
		{
			if(strcmp(propName, "spawnName") == 0)
				Util::SafeCopy(Aux1, so->extraData->spawnName, sizeof(Aux1));
			else if(strcmp(propName, "leaseTime") == 0)
				sprintf(Aux1, "%d", so->extraData->leaseTime);
			else if(strcmp(propName, "spawnPackage") == 0)
				Util::SafeCopy(Aux1, so->extraData->spawnPackage, sizeof(Aux1));
			else if(strcmp(propName, "mobTotal") == 0)
				sprintf(Aux1, "%d", so->extraData->mobTotal);
			else if(strcmp(propName, "maxActive") == 0)
				sprintf(Aux1, "%d", so->extraData->maxActive);
			else if(strcmp(propName, "aiModule") == 0)
				Util::SafeCopy(Aux1, so->extraData->aiModule, sizeof(Aux1));
			else if(strcmp(propName, "maxLeash") == 0)
				sprintf(Aux1, "%d", so->extraData->maxLeash);
			else if(strcmp(propName, "loyaltyRadius") == 0)
				sprintf(Aux1, "%d", so->extraData->loyaltyRadius);
			else if(strcmp(propName, "wanderRadius") == 0)
				sprintf(Aux1, "%d", so->extraData->wanderRadius);
			else if(strcmp(propName, "despawnTime") == 0)
				sprintf(Aux1, "%d", so->extraData->despawnTime);
			else if(strcmp(propName, "sequential") == 0)
				sprintf(Aux1, "%d", so->extraData->sequential);
			else if(strcmp(propName, "spawnLayer") == 0)
				Util::SafeCopy(Aux1, so->extraData->spawnLayer, sizeof(Aux1));
			else
				LogMessageL(MSG_WARN, "[WARNING] spawn.property unknown request: [%s]", propName);
		}
		else
			LogMessageL(MSG_WARN, "[WARNING] spawn.property requested for standard object [%d, %s]", so->ID, so->Asset);
	}
	g_SceneryManager.ReleaseThread();
	return PrepExt_QueryResponseString(SendBuf, query.ID, Aux1);
}

int SimulatorThread :: handle_query_spawn_emitters(void)
{
	//LogMessageL(MSG_SHOW, "spawn.emitters");
	//for(int i = 0; i < query.argCount; i++)
	//	LogMessageL(MSG_SHOW, "[%d]=%s", i, query.args[i].c_str());
	return PrepExt_QueryResponseNull(SendBuf, query.ID);
}

int SimulatorThread :: handle_query_party(void)
{
	/*  Query: party
		Command to create a virtual party with the selected target.
		Args: [none]
	*/
	if(query.argCount < 1)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	const char *command = query.args[0].c_str();

	if(strcmp(command, "invite") == 0)
	{
		if(query.argCount >= 2)
		{
			if(creatureInst->PartyID != 0)
			{
				if(g_PartyManager.GetPartyByLeader(creatureInst->CreatureDefID) == NULL)
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Only party leaders may send invites.");
			}

			CreatureInstance *target = NULL;
			int playerID = query.GetInteger(1);
			target = creatureInst->actInst->GetPlayerByID(playerID);

			//Hack to look up by name since we're re-using this query as a direct command
			//to party by string.
			bool byName = false;
			if(target == NULL)
			{
				target = g_ActiveInstanceManager.GetPlayerCreatureByName(query.GetString(1));
				if(target != NULL)
				{
					byName = true;
					if(target->actInst->mZoneDefPtr->IsDungeon() == true)
						return PrepExt_QueryResponseError(SendBuf, query.ID, "You may not invite players if they are already inside a dungeon.");
				}
			}

			if(byName == true)
			{
				if(creatureInst->actInst->mZoneDefPtr->IsDungeon() == false)
					return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be inside a dungeon to invite players by command.");
			}

			if(target != NULL) 
			{
				if(target->PartyID == 0)
				{
					int wpos = PartyManager::WriteInvite(SendBuf, creatureInst->CreatureDefID, creatureInst->css.display_name);
					creatureInst->actInst->LSendToOneSimulator(SendBuf, wpos, target->simulatorPtr);
				}
				else
				{
					return PrepExt_QueryResponseError(SendBuf, query.ID, "That player is already in a party.");
				}
			}
		}
	}
	else if(strcmp(command, "accept.invite") == 0)
	{
		if(query.argCount >= 2)
		{
			int leaderDefID = query.GetInteger(1);
			CreatureInstance *leader = g_ActiveInstanceManager.GetPlayerCreatureByDefID(leaderDefID);
			if(leader != NULL)
			{
				g_PartyManager.AcceptInvite(creatureInst, leader);
				//creatureInst->PartyID = g_PartyManager.AcceptInvite(leader->CreatureDefID, pld.CreatureDefID, pld.CreatureID, creatureInst->css.display_name);
				int wpos = g_PartyManager.PrepMemberList(SendBuf, creatureInst->PartyID, creatureInst->CreatureID);
				if(wpos > 0)
					AttemptSend(SendBuf, wpos);
				g_PartyManager.BroadcastAddMember(creatureInst);
			}
		}
	}
	else if(strcmp(command, "reject.invite") == 0)
	{
		if(query.argCount >= 2)
		{
			int leaderDefID = atoi(query.args[1].c_str());
			//CreatureInstance *leader = creatureInst->actInst->GetPlayerByCDefID(leaderDefID);
			CreatureInstance *leader = g_ActiveInstanceManager.GetPlayerCreatureByDefID(leaderDefID);
			if(leader != NULL)
			{
				int wpos = PartyManager::WriteRejectInvite(SendBuf, creatureInst->css.display_name);
				leader->simulatorPtr->AttemptSend(SendBuf, wpos);
			}
		}
	}
	else if(strcmp(command, "setLeader") == 0)
	{
		if(query.argCount >= 2)
		{
			int targetID = atoi(query.args[1].c_str());
			g_PartyManager.DoSetLeader(creatureInst, targetID);
		}
	}
	else if(strcmp(command, "kick") == 0)
	{
		if(query.argCount >= 2)
		{
			int targetID = atoi(query.args[1].c_str());
			g_PartyManager.DoKick(creatureInst, targetID);
		}
	}
	else if(strcmp(command, "quest.invite") == 0)
	{
		if(query.argCount >= 2)
		{
			int questID = atoi(query.args[1].c_str());
			QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(questID);
			ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
			if(qdef != NULL && party != NULL)
			{
				int scount = 0;
				int fcount = 0;
				int wpos = PartyManager::WriteQuestInvite(SendBuf, qdef->title.c_str(), qdef->questID);
				SIMULATOR_IT it;
				for(it = Simulator.begin(); it != Simulator.end(); ++it)
				{
					if(it->InternalID == InternalID)
						continue;
					if(it->LoadStage != LOADSTAGE_GAMEPLAY)
						continue;
					if(it->creatureInst->PartyID != party->mPartyID)
						continue;
					if(party->HasMember(it->creatureInst->CreatureDefID) == false)
						continue;
					if(it->pld.charPtr->questJournal.CheckQuestShare(questID) == QuestJournal::SHARE_SUCCESS_QUALIFIES)
					{
						it->AttemptSend(SendBuf, wpos);
						scount++;
					}
					else fcount++;
				}
				if(scount > 0)
				{
					sprintf(Aux1, "Sent quest invite to %d %s.", scount, ((scount == 1) ? "player" : "players"));
					SendInfoMessage(Aux1, INFOMSG_INFO);
				}
				if(fcount > 0)
				{
					sprintf(Aux1, "%d %s cannot take that quest.", fcount, ((fcount == 1) ? "player" : "players"));
					SendInfoMessage(Aux1, INFOMSG_INFO);
				}
			}
			else if(party == NULL)
				SendInfoMessage("You must be in a party to share quests.", INFOMSG_INFO);
		}
	}
	else if(strcmp(command, "loot.mode") == 0) {
		ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
		if(party == NULL)
			SendInfoMessage("You must be in a party to set loot mode.", INFOMSG_INFO);
		else {

			if(query.argCount >= 2)
			{
				int mode = atoi(query.args[1].c_str());
				party->mLootMode = static_cast<LootMode>(mode);
				switch(party->mLootMode)
				{
				case LOOT_MASTER:
					Util::SafeFormat(Aux1, sizeof(Aux1), "Loot master. The party leader gets all the loot.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
					break;
				case ROUND_ROBIN:
					Util::SafeFormat(Aux1, sizeof(Aux1), "Round robin. Loot is offered to each player in  turn.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
					break;
				case FREE_FOR_ALL:
					Util::SafeFormat(Aux1, sizeof(Aux1), "Free for all. Whoever picks up the loot first wins.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
					break;
				}
				int wpos = PartyManager::StrategyChange(SendBuf, party->mLootMode);
				party->BroadCast(SendBuf, wpos);
			}
		}
	}
	else if(strcmp(command, "loot.flags") == 0)	{
		ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
		if(party == NULL)
			SendInfoMessage("You must be in a party to set loot mode.", INFOMSG_INFO);
		else {
			if(query.argCount >= 2)
			{
				int wasFlags = party->mLootFlags;
				int flags = atoi(query.args[1].c_str());
				if(strcmp(query.args[2].c_str(), "true") == 0) {
					party->mLootFlags |= static_cast<LootFlags>(flags);
				}
				else {
					party->mLootFlags &= ~(static_cast<LootFlags>(flags));
				}
				if((party->mLootFlags & MUNDANE) > 0 && (wasFlags & MUNDANE) == 0)
				{
					Util::SafeFormat(Aux1, sizeof(Aux1), "Mundane items included in looting rules.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
				}
				else if((party->mLootFlags & MUNDANE) == 0 && (wasFlags & MUNDANE) > 0)
				{
					Util::SafeFormat(Aux1, sizeof(Aux1), "Mundane items no longer included in looting rules.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
				}
				if((party->mLootFlags & NEED_B4_GREED) > 0 && (wasFlags & NEED_B4_GREED) == 0)
				{
					Util::SafeFormat(Aux1, sizeof(Aux1), "Need before greed now active.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
				}
				else if((party->mLootFlags & NEED_B4_GREED) == 0 && (wasFlags & NEED_B4_GREED) > 0)
				{
					Util::SafeFormat(Aux1, sizeof(Aux1), "Need before greed no longer active.");
					party->BroadcastInfoMessageToAllMembers(Aux1);
				}
				int wpos = PartyManager::StrategyFlagsChange(SendBuf, party->mLootFlags);
				party->BroadCast(SendBuf, wpos);
			}
		}
	}
	else if(strcmp(command, "loot.need") == 0 || strcmp(command, "loot.greed") == 0 || strcmp(command, "loot.pass") == 0)	{
		handle_query_loot_need_greed_pass();
	}
	else if(strcmp(command, "quit") == 0)
	{
		g_PartyManager.DoQuit(creatureInst);
		int wpos = PartyManager::WriteLeftParty(SendBuf);
		AttemptSend(SendBuf, wpos);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	/*
	int wpos = 0;
	if(creatureInst->CurrentTarget.targ == NULL)
		SendInfoMessage("You must select a target.", INFOMSG_INFO);
	else
		wpos = PrepExt_JoinParty(SendBuf, creatureInst, creatureInst->CurrentTarget.targ);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
	*/
}

int SimulatorThread :: handle_query_vault_size(void)
{
	sprintf(Aux1, "%d", pld.charPtr->VaultGetTotalCapacity());
	return PrepExt_QueryResponseString(SendBuf, query.ID, Aux1);
}

int SimulatorThread :: handle_query_vault_expand(void)
{
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int CID = query.GetInteger(0);
	int r = protected_CheckDistance(CID);
	if(r != 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are too far away.");
	
	int CDefID = ResolveCreatureDef(CID);
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if(cdef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid creature.");
	if(!(cdef->DefHints & CDEF_HINT_VAULT))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must speak to a vault keeper.");

	if(pld.charPtr->VaultIsMaximumCapacity() == true)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Your vault space is at maximum capacity.");
	if(creatureInst->css.credits < CharacterData::VAULT_EXPAND_CREDIT_COST)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You do not have enough credits.");

	pld.charPtr->VaultDoPurchaseExpand();
	int newSize = pld.charPtr->VaultGetTotalCapacity();
	creatureInst->css.credits -= CharacterData::VAULT_EXPAND_CREDIT_COST;
	
	creatureInst->SendStatUpdate(STAT::CREDITS);

	int wpos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	wpos += PrepExt_CreatureEventVaultSize(&SendBuf[wpos], creatureInst->CreatureID, newSize);
	return wpos;
}

int SimulatorThread :: handle_query_vault_deliverycontents(void)
{
	return PrepExt_QueryResponseNull(SendBuf, query.ID);
}

int SimulatorThread :: handle_query_quest_share(void)
{
	if(query.argCount < 1)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	int questID = atoi(query.args[0].c_str());
	int res = pld.charPtr->questJournal.CheckQuestShare(questID);
	if(res != QuestJournal::SHARE_SUCCESS_QUALIFIES)
		SendInfoMessage(QuestJournal::GetQuestShareErrorString(res), INFOMSG_ERROR);
	else
	{
		if(pld.charPtr->questJournal.QuestJoin_Helper(questID) == 0)
		{
			int wpos = QuestJournal::WriteQuestJoin(SendBuf, questID);
			wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "You have joined the quest.", INFOMSG_INFO);
			AttemptSend(SendBuf, wpos);
		}
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_partyall(void)
{
	/*  Query: partyall
		Command to add all players to the virtual party.
		Args: [none]
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int wpos = creatureInst->actInst->PartyAll(creatureInst, SendBuf);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_partyquit(void)
{
	/*  Query: partyquit
		Command to disband the virtual party.
		Args: [none]
	*/
	g_PartyManager.DebugForceRemove(creatureInst);

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 6);  //_handlePartyUpdateMsg
	wpos += PutShort(&SendBuf[wpos], 0);

	wpos += PutByte(&SendBuf[wpos], 6);  //Left Party

	PutShort(&SendBuf[1], wpos - 3);       //Set message size

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

void SimulatorThread :: Debug_GenerateReport(ReportBuffer *report)
{
	report->AddLine("Sim:%d", InternalIndex);
	if(creatureInst != NULL)
		report->AddLine("display_name=%s", creatureInst->css.display_name);
	if(pld.accPtr != NULL)
		report->AddLine("AccountID=%d", pld.accPtr->ID);
	if(pld.charPtr != NULL)
	{
		int sec = ((g_ServerTime - TimeLastAutoSave) / 1000) + pld.charPtr->SecondsLogged;
		Util::FormatTime(Aux3, sizeof(Aux3), sec);
		report->AddLine("Time logged=%s", Aux3);
	}
	report->AddLine("clientPeerName=%s", clientPeerName);
	report->AddLine("ThreadID=%d", ThreadID);
	report->AddLine("LockCount=%d", sim_cs.GetLockCount());
	report->AddLine("isActive=%d", isThreadActive);
	report->AddLine("Status=%d", Status);
	report->AddLine("Connected=%d", isConnected);
	report->AddLine("firstConnect=%d", firstConnect);
	report->AddLine("LogBuffer [%d,%d]=%s", Util::IsStringTerminated(LogBuffer, sizeof(LogBuffer)), strlen(LogBuffer), LogBuffer);
	report->AddLine("RecBuf [%d,%d]=%s", Util::IsStringTerminated(RecBuf, sizeof(RecBuf)), strlen(RecBuf), RecBuf);
	report->AddLine("SendBuf [%d,%d]=%s", Util::IsStringTerminated(SendBuf, sizeof(SendBuf)), strlen(SendBuf), SendBuf);
	report->AddLine("TotalMessageReceived=%ld", TotalMessageReceived);
	report->AddLine("TotalMessageSent=%ld", TotalMessageSent);
	report->AddLine("TotalRecBytes=%ld", TotalRecBytes);
	report->AddLine("TotalSendBytes=%ld", TotalSendBytes);
	report->AddLine("SocketStatus=%d", SocketStatus);
	report->AddLine("Socket=%d", sc.ClientSocket);
	report->AddLine("MessageEnd=%d", MessageEnd);
	report->AddLine("ReadPos=%d", ReadPos);
	report->AddLine("WritePos=%d", WritePos);
	report->AddLine("PendingSend=%d", PendingSend);
	report->AddLine("Aux1 [%d,%d]=%s", Util::IsStringTerminated(Aux1, sizeof(Aux1)), strlen(Aux1), Aux1);
	report->AddLine("Aux2 [%d,%d]=%s", Util::IsStringTerminated(Aux2, sizeof(Aux2)), strlen(Aux2), Aux2);
	report->AddLine("Aux3 [%d,%d]=%s", Util::IsStringTerminated(Aux3, sizeof(Aux3)), strlen(Aux3), Aux3);
	report->AddLine("ProtocolState=%d", ProtocolState);
	report->AddLine("ClientLoading=%d", ClientLoading);
	report->AddLine("LoadStage=%d", LoadStage);
	report->AddLine("LastUpdate=%ld", LastUpdate);
	report->AddLine("pld.CreatureDefID=%d", pld.CreatureDefID);
	report->AddLine("pld.CreatureID=%d", pld.CreatureID);
	report->AddLine("pld.accPtr=%p", pld.accPtr);
	report->AddLine("pld.charPtr=%p", pld.charPtr);
	report->AddLine("pld.zoneDef=%p", pld.zoneDef);
	report->AddLine("pld.CurrentInstanceID=%d", pld.CurrentInstanceID);
	report->AddLine("pld.CurrentZoneID=%d", pld.CurrentZoneID);
	report->AddLine("pld.CurrentZone=%s", pld.CurrentZone);
	report->AddLine("pld.oldSpawnX=%d", pld.oldSpawnX);
	report->AddLine("pld.oldSpawnZ=%d", pld.oldSpawnZ);
	report->AddLine("pld.PendingMovement=%d", pld.PendingMovement);
	report->AddLine("pld.MovementStep=%d", pld.MovementStep);
	report->AddLine("pld.MovementTime=%ld", pld.MovementTime);
	report->AddLine("pld.MovementBlockTime=%lu", pld.MovementBlockTime);
	report->AddLine("pld.ResendSpeedTime=%lu", pld.ResendSpeedTime);
	report->AddLine("pld.TotalDistanceMoved=%ld", pld.TotalDistanceMoved);
	report->AddLine("pld.LastCheckDistanceMoved=%ld", pld.LastCheckDistanceMoved);
	report->AddLine("pld.NextIdleCheckTime=%lu", pld.NextIdleCheckTime);
	report->AddLine("pld.LastCastX=%d", pld.LastCastX);
	report->AddLine("pld.LastCastZ=%d", pld.LastCastZ);
	report->AddLine("pld.IdleCastCount=%d", pld.IdleCastCount);
	report->AddLine("pld.CurrentMapInt=%d", pld.CurrentMapInt);
	report->AddLine("pld.LastMapTick=%d", pld.LastMapTick);

	report->AddLine("pld.DebugPingServerLastMsgReceived=%d", pld.DebugPingServerLastMsgReceived);
	report->AddLine("pld.DebugPingServerTotalMsgReceived=%d", pld.DebugPingServerTotalMsgReceived);
	report->AddLine("pld.DebugPingServerNotifyTime=%d", pld.DebugPingServerNotifyTime);
	report->AddLine("pld.DebugPingServerLowest=%d", pld.DebugPingServerLowest);
	report->AddLine("pld.DebugPingServerHighest=%d", pld.DebugPingServerHighest);
	report->AddLine("pld.DebugPingServerTotalTime=%d", pld.DebugPingServerTotalTime);
	report->AddLine("pld.DebugPingServerTotalReceived=%d", pld.DebugPingServerTotalReceived);
	report->AddLine("pld.DebugPingServerSent=%d", pld.DebugPingServerSent);

	report->AddLine("creatureInst=%p", creatureInst);
	if(creatureInst != NULL)
		report->AddLine("creatureInst->css.display_name=%s", creatureInst->css.display_name);
	report->AddLine("defcInst=%p", &defcInst);
	report->AddLine("TimeOnline=%ld", TimeOnline);
	report->AddLine("query.ID=%d", query.ID);
	report->AddLine("query.name=%s", query.name.c_str());
	report->AddLine("query.argCount=%d", query.argCount);
	for(size_t i = 0; i < query.args.size(); i++)
		report->AddLine("query.args[%d]=%s", i, query.args[i].c_str());

	report->AddLine(NULL);
}

void Debug_GenerateSimulatorReports(ReportBuffer *report)
{
	report->AddLine("Simulator pointers:");
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
		report->AddLine("Sim:%d [%p]", it->InternalID, &*it);
	report->AddLine(NULL);

	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		it->sim_cs.Enter("Debug_GenerateSimulatorReports");
		it->Debug_GenerateReport(report);
		it->sim_cs.Leave();
	}
}

int SimulatorThread :: handle_command_ccc(void)
{
	/*  Query: ccc
		Cheat to clear cooldowns categories.
		Args: [none]
	*/

	if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	const char *category = NULL;
	if(query.argCount > 0)
		category = query.args[0].c_str();

	creatureInst->cooldownManager.Clear();
	int wpos = 0;
	if(category != NULL)
		wpos = PrepExt_CooldownExpired(SendBuf, creatureInst->CreatureID, category);
	else
	{
		STRINGLIST categoryList;
		g_AbilityManager.GetCooldownCategoryStrings(categoryList);
		for(size_t i = 0; i < categoryList.size(); i++)
		{
			wpos += PrepExt_CooldownExpired(&SendBuf[wpos], creatureInst->CreatureID, categoryList[i].c_str());
			CheckWriteFlush(wpos);
		}
	}

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

void SimulatorThread :: CheckSpawnTileUpdate(bool force)
{
	int tx = creatureInst->CurrentX / SpawnTile::SPAWN_TILE_SIZE;
	int tz = creatureInst->CurrentZ / SpawnTile::SPAWN_TILE_SIZE;
	if((tx != pld.oldSpawnX || tz != pld.oldSpawnZ) || (force == true))
	{
		unsigned long startTime = g_PlatformTime.getMilliseconds();

		pld.oldSpawnX = tx;
		pld.oldSpawnZ = tz;
		//int scount = 0;

		creatureInst->actInst->spawnsys.GenerateAreaTile(tx, tz);
		creatureInst->actInst->spawnsys.RunProcessing(true);

		int wpos = 0;
#ifdef CREATUREQAV
		for(size_t i = 0; i < creatureInst->actInst->NPCListPtr.size(); i++)
		{
			CreatureInstance *cInst = creatureInst->actInst->NPCListPtr[i];
#else
		ActiveInstance::CREATURE_IT it;
		for(it = creatureInst->actInst->NPCList.begin(); it != creatureInst->actInst->NPCList.end(); ++it)
		{
			CreatureInstance *cInst = &it->second;
#endif
			if(cInst->CurrentX < (creatureInst->CurrentX - g_CreatureListenRange))
				continue;
			if(cInst->CurrentX > (creatureInst->CurrentX + g_CreatureListenRange))
				continue;
			if(cInst->CurrentZ < (creatureInst->CurrentZ - g_CreatureListenRange))
				continue;
			if(cInst->CurrentZ > (creatureInst->CurrentZ + g_CreatureListenRange))
				continue;
			wpos += PrepExt_GeneralMoveUpdate(&SendBuf[wpos], cInst); 
			CheckWriteFlush(wpos);
			//scount++;
		}
		if(wpos > 0)
			AttemptSend(SendBuf, wpos);

		unsigned long timeDif = g_PlatformTime.getMilliseconds() - startTime;
		if(timeDif > 50)
			LogMessageL(MSG_SHOW, "[DEBUG] CheckSpawnTileUpdate complete in %d ms, zone:%d", timeDif, pld.CurrentZoneID);
		//LogMessageL(MSG_SHOW, "[DEBUG] Updating %d creatures", scount);
	}
}

int SimulatorThread :: handle_query_scenery_link_add(void)
{
	/* Query: scenery.link.add
	   Args : 3, [0] = PropID, [1] = PropID, [2] = type
	*/
	if(query.argCount < 3)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	int p1 = query.GetInteger(0);
	int p2 = query.GetInteger(1);
	int type = query.GetInteger(2);

	g_SceneryManager.GetThread("SimulatorThread::handle_query_scenery_link_add");
	bool result = g_SceneryManager.UpdateLink(pld.CurrentZoneID, p1, p2, type);
	g_SceneryManager.ReleaseThread();

	if(result == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Link failed.");

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_scenery_link_del(void)
{
	/* Query: scenery.link.del
	   Args : 2, [0] = PropID, [1] = PropID
	*/
	if(query.argCount < 2)
		return PrepExt_QueryResponseNull(SendBuf, query.ID);

	int p1 = atoi(query.args[0].c_str());
	int p2 = atoi(query.args[1].c_str());

	LogMessageL(MSG_SHOW, "[DEBUG] scenery.link.del: %d, %d", p1, p2);

	g_SceneryManager.GetThread("SimulatorThread::handle_query_scenery_link_del");
	int r = g_SceneryManager.UpdateLink(pld.CurrentZoneID, p1, p2, -1);
	g_SceneryManager.ReleaseThread();

	if(r == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Unlink failed.");

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_unstick(void)
{
	/* Query: unstick
	*/
	//if(creatureInst->HasStatus(StatusEffects::DEAD))
	//	SendInfoMessage("You cannot unstick while dead.", INFOMSG_ERROR);
	//else
	//SendInfoMessage("Unsticking will apply a temporary stat penalty for 2 minutes.", INFOMSG_INFO);
	if(pld.charPtr->NotifyUnstick(true) == true)
		SendInfoMessage("Unsticking too often will apply a temporary stat penalty for 2 minutes.", INFOMSG_INFO);

	//AddMessage((long)creatureInst, 10006, BCM_AbilityRequest);
	creatureInst->RequestAbilityActivation(10006);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: WarpToZone(ZoneDefInfo *zoneDef, int xOverride, int yOverride, int zOverride)
{
	if(zoneDef == NULL)
		return;

	int targX = zoneDef->DefX;
	int targY = zoneDef->DefY;
	int targZ = zoneDef->DefZ;


	//Hack for Arena too, semantics are the same.
	if(zoneDef->mGrove == true || zoneDef->mArena == true)
	{
		if(pld.zoneDef->mGrove == false && pld.zoneDef->mArena == false)
		{
			//We're warping from a non-grove to grove.

			//Make sure we're in range of a sanctuary.
			WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(pld.zoneDef->mID, creatureInst->CurrentX, creatureInst->CurrentZ, SANCTUARY_PROXIMITY_USE); 
			if(sanct == NULL)
			{
				SendInfoMessage("You must be within range of a sanctuary.", INFOMSG_ERROR);
				return;
			}
			//Need to save a special set of return coordinates so the player can
			//return to this location.
			pld.charPtr->groveReturnPoint[0] = (int)sanct->x; //creatureInst->CurrentX;
			pld.charPtr->groveReturnPoint[1] = (int)sanct->y + SANCTUARY_ELEVATION_ADDITIVE; //creatureInst->CurrentY;
			pld.charPtr->groveReturnPoint[2] = (int)sanct->z; //creatureInst->CurrentZ;
			pld.charPtr->groveReturnPoint[3] = pld.zoneDef->mID;
		}
	}
	else
	{
		if(pld.zoneDef->mGrove == true || pld.zoneDef->mArena == true)
		{
			//Warping from a grove back to normal territory.
			if(pld.charPtr->groveReturnPoint[0] != 0 && pld.charPtr->groveReturnPoint[2] != 0)
			{
				targX = pld.charPtr->groveReturnPoint[0];
				targY = pld.charPtr->groveReturnPoint[1];
				targZ = pld.charPtr->groveReturnPoint[2];
			}
		}
	}

	SetPosition(creatureInst->CurrentX, creatureInst->CurrentY, creatureInst->CurrentZ, 1);
	MainCallSetZone(zoneDef->mID, 0, false);
	if(ValidPointers() == false)
	{
		ForceErrorMessage("Critical error while changing zones.", INFOMSG_ERROR);
		Disconnect("SimulatorThread::WarpToZone");
		return;
	}

	if(xOverride != 0) { targX = xOverride; }
	if(yOverride != 0) { targY = yOverride; }
	if(zOverride != 0) { targZ = zOverride; }
	
	SetPosition(targX, targY, targZ, 1);
	//SendInfoMessage("Changing zone.", INFOMSG_INFO);
	CheckSpawnTileUpdate(true);
	CheckMapUpdate(true);
}



int SimulatorThread :: handle_command_ban(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /ban \"Character Name\" duration");

	char denom = 0;
	if(query.argCount >= 3)
		if(query.args[2].size() > 0)
			denom = query.args[2][0];

	const char *charName = query.args[0].c_str();
	int duration = atoi(query.args[1].c_str());

	if(denom == 'h')
		duration *= 60;      //Value should be interpreted as hours
	else if(denom == 'd')
		duration *= 1440;    //As days (60 minutes * 24 hours)
	else if(denom == 'w')
		duration *= 10080;   //As weeks (60 minutes * 24 hours * 7 days)
	else if(denom == 'y')
		duration *= 525600;  //As years (60 minutes * 24 hours * 365 days)
	
	CharacterData *banChar = NULL;
	g_CharacterManager.GetThread("SimulatorThread::handle_command_ban");
	banChar = g_CharacterManager.GetCharacterByName(query.args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if(banChar == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Character not found.");

	bool bFound = false;
	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->pld.charPtr == banChar)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "%s [%s] has been suspended.", charName, it->pld.accPtr->Name);
			it->pld.accPtr->SetBan(duration);
			it->Disconnect("SimulatorThread::handle_command_ban");
			bFound = true;
			break;
		}
	}

	if(bFound == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Character not logged in.");

	SendInfoMessage(Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_unban(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /unban \"AccountName\"");

	const char *userName = query.args[0].c_str();

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_unban");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(userName);
	g_AccountManager.cs.Leave();
	if(accPtr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Could not find user.");

	accPtr->ClearBan();
	accPtr->AdjustSessionLoginCount(0);  //Force refresh.

	SendInfoMessage("Ban cleared.", INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_setbuildpermission(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	//[0] = Account name
	//[1] = permission name
	//[2] = flag state (set or clear)

	if(query.argCount != 2 && query.argCount != 6)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /setbuildpermission name instance [x1,y1,x2,y2]");

	int zoneID = query.GetInteger(1);
	g_AccountManager.cs.Enter("SimulatorThread::handle_command_setbuildpermission");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(query.args[0].c_str());
	if(accPtr == NULL) {
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Could not find user.");
	}

	if(query.argCount == 2) {
		// Remove
		bool found = false;
		for(std::vector<BuildPermissionArea>::iterator it = accPtr->BuildPermissionList.begin(); it != accPtr->BuildPermissionList.end() ; ++it) {
			BuildPermissionArea pa = *it;
			if(pa.ZoneID == zoneID) {
				accPtr->BuildPermissionList.erase(it);
				Util::SafeFormat(Aux1, sizeof(Aux1), "Removed permission for %s in zone %d (%d,%d,%d,%d)",
						accPtr->Name, zoneID, pa.x1, pa.y1, pa.x2, pa.y2);
				SendInfoMessage(Aux1, INFOMSG_INFO);
				found = true;
				break;
			}
		}
		if(!found) {
			Util::SafeFormat(Aux1, sizeof(Aux1), "Could not find permission for %s in zone %d",accPtr->Name, zoneID);
			SendInfoMessage(Aux1, INFOMSG_ERROR);
		}
	}
	else {
		// Add / update
		bool found = false;
		for(std::vector<BuildPermissionArea>::iterator it = accPtr->BuildPermissionList.begin(); it != accPtr->BuildPermissionList.end() ; ++it) {
			BuildPermissionArea pa = *it;
			if(pa.ZoneID == zoneID) {
				pa.x1 = query.GetInteger(2);
				pa.y1 = query.GetInteger(3);
				pa.x2 = query.GetInteger(4);
				pa.y2 = query.GetInteger(5);
				found = true;
				Util::SafeFormat(Aux1, sizeof(Aux1), "Updated permission for %s in zone %d (%d,%d,%d,%d)",
						accPtr->Name, pa.ZoneID, pa.x1, pa.y1, pa.x2, pa.y2);
				SendInfoMessage(Aux1, INFOMSG_INFO);
				break;
			}
		}
		if(!found) {
			BuildPermissionArea pa;
			pa.ZoneID = zoneID;
			pa.x1 = query.GetInteger(2);
			pa.y1 = query.GetInteger(3);
			pa.x2 = query.GetInteger(4);
			pa.y2 = query.GetInteger(5);
			accPtr->BuildPermissionList.push_back(pa);
			Util::SafeFormat(Aux1, sizeof(Aux1), "Added permission for %s in zone %d (%d,%d,%d,%d)",
					accPtr->Name, pa.ZoneID, pa.x1, pa.y1, pa.y1, pa.y2);
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}
	}
	accPtr->PendingMinorUpdates++;
	g_AccountManager.cs.Leave();
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_setpermission(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	//[0] = Account name
	//[1] = permission name
	//[2] = flag state (set or clear)

	if(query.argCount < 3)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /setpermission name value permission");

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_setpermission");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(query.args[0].c_str());
	g_AccountManager.cs.Leave();
	if(accPtr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Could not find user.");

	int state = atoi(query.args[2].c_str());
	bool r = accPtr->SetPermission(Perm_Account, query.args[1].c_str(), (state != 0) ? true : false);
	if(r == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to set.");

	accPtr->PendingMinorUpdates++;

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_setpermissionc(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	//[0] = Character name
	//[1] = permission name
	//[2] = flag state (set or clear)

	if(query.argCount < 3)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /setpermissionc name value permission");

	g_CharacterManager.GetThread("SimulatorThread::handle_command_setpermissionc");
	CharacterData *charPtr = g_CharacterManager.GetCharacterByName(query.args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if(charPtr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Could not find user.");

	int state = atoi(query.args[2].c_str());
	bool r = charPtr->SetPermission(Perm_Account, query.args[1].c_str(), (state != 0) ? true : false);
	if(r == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to set.");

	charPtr->pendingChanges++;

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_setbehavior(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	//[0] = flag bit value
	//[1] = state

	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /setbehavior bitvalue state");

	int value = atoi(query.args[0].c_str());
	int state = atoi(query.args[1].c_str());
	g_Config.SetAdministrativeBehaviorFlag(value, (state != 0) ? true : false);
	sprintf(Aux1, "BehaviorFlag is now: %lu", g_Config.debugAdministrativeBehaviorFlags);
	SendInfoMessage(Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_deriveset(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(creatureInst->CurrentTarget.targ == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Must select a target.");

	std::string eqApp = creatureInst->CurrentTarget.targ->css.eq_appearance;

	uint pos = 0;
	while(pos != string::npos)
	{
		pos = eqApp.find_first_of("{}[]");
		if(pos != string::npos)
			eqApp.erase(pos, 1);
	}

	STRINGLIST items;
	STRINGLIST itementry;
	std::vector<int> itemIDList;
	Util::Split(eqApp, ",", items);
	for(size_t i = 0; i < items.size(); i++)
	{
		Util::Split(items[i], "=", itementry);
		if(itementry.size() > 1)
			itemIDList.push_back(atoi(itementry[1].c_str()));
	}

	struct SetMatch
	{
		int ItemID;
		int equipType;
	};
	static const SetMatch baseSet[] = {
		{8900,   16}, //"Refashionable Boots",       
		{8901,   11}, //"Refashionable Vest",        
		{8902,   13}, //"Refashionable Gloves",      
		{8903,   15}, //"Refashionable Leggings",    
		{8904,   12}, //"Refashionable Sleeves",     
		{8905,    7}, //"Refashionable Buckler",     
		{8906,    9}, //"Refashionable Collar",      
		{8907,   14}, //"Refashionable Belt",        
		{8908,   10}, //"Refashionable Shoulderpads",
		{8909,    8}, //"Refashionable Cap",         
	};
	static const int numItemTemplate = sizeof(baseSet) / sizeof(SetMatch);

	int wpos = 0;
	bool error = false;
	InventorySlot newItem;
	for(size_t i = 0; i < itemIDList.size(); i++)
	{
		ItemDef *itemDef = g_ItemManager.GetPointerByID(itemIDList[i]);
		if(itemDef != NULL)
		{
			for(int s = 0; s < numItemTemplate; s++)
			{
				if(itemDef->mEquipType == baseSet[s].equipType)
				{
					int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
					if(slot == -1)
					{
						if(error == false)
						{
							wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], "Out of space", INFOMSG_ERROR);
							error = true;
						}
					}
					else
					{
						newItem.CCSID = pld.charPtr->inventory.GetCCSID(INV_CONTAINER, slot);
						newItem.count = 0;
						newItem.IID = baseSet[s].ItemID;
						newItem.customLook = itemDef->mID;
						newItem.dataPtr = NULL;
						pld.charPtr->inventory.AddItem(INV_CONTAINER, newItem);
						wpos += AddItemUpdate(&SendBuf[wpos], Aux1, &newItem);
					}
				}
			}
		}
	}
	wpos += ::PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_igstatus(void)
{
	//Sets in-game status effects for the player character.
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int EffectID = GetStatusIDByName(query.args[0].c_str());
	if(EffectID == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid effect.");

	CreatureInstance *target = creatureInst;
	if(query.argCount >= 3)
		target = creatureInst->CurrentTarget.targ;
	if(target == NULL)
		target = creatureInst;

	int value = atoi(query.args[1].c_str());
	if(value == 0)
		target->_RemoveStatusList(EffectID);
	else
		target->_AddStatusList(EffectID, -1);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_partyzap(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	g_PartyManager.DebugDestroyParties();

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_zonename(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	if(query.argCount >= 2)
	{
		int t = atoi(query.args[0].c_str());
		const char *name = query.args[1].c_str();
		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(pld.CurrentZoneID);
		if(zoneDef != NULL)
		{
			if(zoneDef->mGrove == true)
			{
				if(t == 1)
					zoneDef->ChangeShardName(name);
				else
					zoneDef->ChangeName(name);
				g_ZoneDefManager.NotifyConfigurationChange();
				SendSetMap();
				SendInfoMessage(pld.zoneDef->mShardName.c_str(), INFOMSG_SHARD);
			}
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_script_op(int op)
{

	/*  Query: script.op
		Get a script (instance script etc)
		Args: [type]

		0 = Instance
		1 = Quest
		2 = AI

		0 = Load
		1 = Kill
		2 = Run
		3 = Save
	*/
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Incorrect arguments, require type and parameter.");

	int type = query.GetInteger(0);
	const char *parameter = query.GetString(1);

	g_Log.AddMessageFormat("Script op %d type %d param: %s", op, type, parameter);

	bool admin = CheckPermissionSimple(Perm_Account, Permission_Admin);
	bool ok = admin;
	bool ownGrove = pld.zoneDef->mGrove == true && pld.zoneDef->mAccountID == pld.accPtr->ID;
	if(!ok) {
		// Players can edit their own grove scripts (some commands will be restricted)
		if(type == 0 && ownGrove)
			ok = true;
	}
	if(!ok)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");


	string path;
	int instanceId = 0;
	int questId = 0;
	string scriptName;
	ScriptCore::NutPlayer *player = NULL;
	ScriptCore::ScriptPlayer *oldPlayer = NULL;
	ActiveInstance *instance = NULL;
	CreatureInstance *targetCreature = NULL;
	bool ownPlayer = true;

	switch(type) {
	case 0:
	{
		// Instance script
		if(strcmp(parameter, "") == 0) {
			g_Log.AddMessageFormat("[REMOVEME] default grove");
			instance = creatureInst->actInst;
			instanceId = instance->mZone;
			player = instance->nutScriptPlayer;
			oldPlayer = instance->scriptPlayer;
		}
		else if(admin) {
			instanceId = atoi(parameter);
			if(instanceId == 0) {
				SendInfoMessage("Invalid zone ID", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid zone ID.");
			}
		}
		else {
			SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
		}

		// If editing the current zone, always use that as active script
		if(instanceId == creatureInst->actInst->mZone) {
			instance = creatureInst->actInst;
			player = instance->nutScriptPlayer;
			oldPlayer = instance->scriptPlayer;
		}
		else {
			ownPlayer = false;
		}

		g_Log.AddMessageFormat("[REMOVEME] using zone %d", instanceId);
		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(instanceId);
		path = InstanceScript::InstanceNutDef::GetInstanceScriptPath(instanceId, false, zoneDef != NULL && zoneDef->mGrove);
		break;
	}
	case 1:
		// Quest script
		if(!admin) {
			SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
		}

		if(strcmp(parameter, "") == 0) {
			SendInfoMessage("Unsupported, please provide quest ID", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Unsupported.");
		}

		ownPlayer = false;
		questId = atoi(parameter);
		Util::SafeFormat(Aux1, sizeof(Aux1), "QuestScripts/%d.nut", questId);
		path = Aux1;
		break;
	case 2:
		// AI script
		if(strcmp(parameter, "") == 0) {
			g_Log.AddMessageFormat("[REMOVEME] char AI ");

			targetCreature = creatureInst->CurrentTarget.targ;
			if(targetCreature == NULL)
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Must select a creature to edit or provide a CDefID.");

			scriptName = targetCreature->css.ai_package;
			if(scriptName.length() == 0)
				scriptName = targetCreature->charPtr->cdef.css.ai_package;

			player = targetCreature->aiNut;
			oldPlayer = targetCreature->aiScript;
		}
		else if(admin) {
			scriptName = parameter;
		}
		else {
			SendInfoMessage("Permission denied", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
		}

		// If editing a creatures AI, always use that as active script
		if(strcmp(parameter, "") != 0 && creatureInst->CurrentTarget.targ != NULL) {
			scriptName = creatureInst->CurrentTarget.targ->css.ai_package;
			if(scriptName.length() == 0) {
				scriptName = creatureInst->CurrentTarget.targ->charPtr->cdef.css.ai_package;
			}
			if(scriptName.compare(parameter) == 0) {
				targetCreature = creatureInst->CurrentTarget.targ;
				player = targetCreature->aiNut;
				oldPlayer = targetCreature->aiScript;
			}
			else {
				ownPlayer = false;
			}
		}

		ScriptCore::NutScriptCallStringParser p(scriptName);

		AINutDef *def = aiNutManager.GetScriptByName(p.mScriptName.c_str());
		if(def == NULL) {
			AIScriptDef * oldDef = aiScriptManager.GetScriptByName(p.mScriptName.c_str());
			if(oldDef != NULL) {
				Util::SafeFormat(Aux1, sizeof(Aux1), "AIScript/%s.txt", p.mScriptName.c_str());
				path = string(Aux1);
			}

		}
		else
			path = def->mSourceFile;

		break;
	}

	switch(op) {
	// Load
		case 0:
		{
			if(path.length() == 0) {
				Util::SafeFormat(Aux1, sizeof(Aux1), "Unable to open script.");
				SendInfoMessage(Aux1, INFOMSG_ERROR);
				LogMessageL(MSG_WARN, "[WARNING] Load script query unable to open script for zone: %d", creatureInst->actInst->mZone);
				return 0;
			}
			FileReader lfr;
			std::vector<std::string> lines;
			g_Log.AddMessageFormat("[REMOVEME] loading %s", path.c_str());
			if(Platform::FileExists(path.c_str()))
			{
				if(lfr.OpenText(path.c_str()) != Err_OK) {

					LogMessageL(MSG_WARN, "[WARNING] Load script query unable to open file: %s", path.c_str());
					return 0;
				}

				while(lfr.FileOpen() == true)
				{
					lfr.ReadLine();
					lines.push_back(lfr.DataBuffer);
				}
			}


			int wpos = 0;
			wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
			wpos += PutShort(&SendBuf[wpos], 0);      //Message size
			wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index
			wpos += PutShort(&SendBuf[wpos], lines.size() + 1);
			for(uint i = 0 ; i < lines.size() ; i++) {
				wpos += PutByte(&SendBuf[wpos], 1);
				wpos += PutStringUTF(&SendBuf[wpos], lines[i].c_str());
			}

			// The last record contains info about the instance itself for the editor UI
			wpos += PutByte(&SendBuf[wpos], 2);

			if(player != NULL) {
				g_Log.AddMessageFormat("Using active new player %s", player->mActive ? "active" : "inactive");
				wpos += PutStringUTF(&SendBuf[wpos], player->mActive ? "true" : "false"); // active
			}
			else if(oldPlayer != NULL) {
				g_Log.AddMessageFormat("Using active old player %s", oldPlayer->mActive ? "active" : "inactive");
				wpos += PutStringUTF(&SendBuf[wpos], oldPlayer->mActive ? "true" : "false"); // active
			}
			else {
				g_Log.AddMessageFormat("No player %s!", ownPlayer ? "false" : "unknown");
				wpos += PutStringUTF(&SendBuf[wpos], ownPlayer ? "false" : "unknown"); // active
			}

			switch(type) {
				case 0:
					sprintf(Aux1, "%d", instanceId);
					break;
				case 1:
					sprintf(Aux1, "%d", questId);
					break;
				case 2:
					sprintf(Aux1, "%s", scriptName.c_str());
					break;
			}
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			PutShort(&SendBuf[1], wpos - 3);
			return wpos;
		}
	case 1:
		// Kill
		switch(type) {
			case 0:
				if(instance == NULL) {
					SendInfoMessage("Can only kill scripts in current instance.", INFOMSG_ERROR);
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Can only kill scripts in current instance.");
				}
				if(!instance->KillScript()) {
					SendInfoMessage("Script not running.", INFOMSG_ERROR);
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Script not running.");
				}
				SendInfoMessage("Script killed.", INFOMSG_INFO);
				break;
			case 1:
				SendInfoMessage("Not supported.", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Not supported.");
			default:
				// AI script
				if(targetCreature == NULL) {
					SendInfoMessage("Must select a creature.", INFOMSG_ERROR);
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Must select a creature.");
				}
				if(!targetCreature->KillAI()) {
					SendInfoMessage("Script not running.", INFOMSG_ERROR);
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Script not running.");
				}
				SendInfoMessage("Script killed.", INFOMSG_INFO);
				break;
		}

		break;
	case 2:
	{
		// Run
		LogMessageL(MSG_SHOW, "Handling script run");

		std::string errors;

		switch(type) {
		case 0:
			if(instance == NULL) {
				SendInfoMessage("Can only run scripts in current instance.", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Can only run scripts in current instance.");
			}
			if(!instance->RunScript(errors)) {
				SendInfoMessage("Script already running.", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Script already running.");
			}
			break;
		case 1:
			SendInfoMessage("Not supported.", INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Not yet supported.");
		default:
			// AI script
			if(targetCreature == NULL) {
				SendInfoMessage("Must select a creature.", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Must select a creature.");
			}
			if(!targetCreature->StartAI(errors)) {
				SendInfoMessage("Script already running.", INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Script already running.");
			}
			break;
		}

		if(errors.length() > 0) {
			Util::SafeFormat(Aux1, sizeof(Aux1), "Failed to run script %s", errors.c_str());
			SendInfoMessage(Aux1, INFOMSG_ERROR);
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Script already running.");
		}
		else {
			Util::SafeFormat(Aux1, sizeof(Aux1), "Script now running.");
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}
		break;
	}
	case 3:
	{
		if(query.argCount < 3)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Incorrect arguments, require script content.");

		std::string scriptText = query.GetString(2);

		std::string nutHeader = "#!/bin/sq";
		std::string tslHeader = "#!/bin/tsl";

		if(path.length() == 0) {

			switch(type) {
				case 0:
					if(scriptText.substr(0, nutHeader.size()) == nutHeader)
						path = InstanceScript::InstanceNutDef::GetInstanceNutScriptPath(creatureInst->actInst->mZone, creatureInst->actInst->mZoneDefPtr->mGrove);
					else if(scriptText.substr(0, tslHeader.size()) == tslHeader)
						path = InstanceScript::InstanceScriptDef::GetInstanceTslScriptPath(creatureInst->actInst->mZone, creatureInst->actInst->mZoneDefPtr->mGrove);
					break;
				case 2:

					if(scriptText.substr(0, nutHeader.size()) == nutHeader)
						path = InstanceScript::InstanceNutDef::GetInstanceNutScriptPath(creatureInst->actInst->mZone, creatureInst->actInst->mZoneDefPtr->mGrove);
					else if(scriptText.substr(0, tslHeader.size()) == tslHeader)
						path = InstanceScript::InstanceScriptDef::GetInstanceTslScriptPath(creatureInst->actInst->mZone, creatureInst->actInst->mZoneDefPtr->mGrove);
					break;
			}

			if(path.length() == 0) {
				Util::SafeFormat(Aux1, sizeof(Aux1), "Not supported.");
				SendInfoMessage(Aux1, INFOMSG_ERROR);
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Script not supported.");
			}
		}

		// Create the directory
		string dir = Platform::Dirname(path.c_str());
		Platform::FixPaths(dir);
		Platform::MakeDirectory(dir.c_str());

		g_Log.AddMessageFormat("Saving to %s in %s", path.c_str(), dir.c_str());

		// Save to temporary file first in case the save fails (leaving some hope of recovery)
		string tpath = path;
		tpath.append(".tmp");

		// If the script is empty, delete it
		if(scriptText.length() == 0) {
			Platform::Delete(tpath.c_str());
			Util::SafeFormat(Aux1, sizeof(Aux1), "Script for %d deleted.", creatureInst->actInst->mZone);
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}
		else {
			std::ofstream out(tpath.c_str());
			out << scriptText;
			out.close();

			// If we wrote OK, delete the old file and swap in the new one
			if(!Platform::FileExists(path.c_str()) || remove(path.c_str()) == 0) {
				if(rename(tpath.c_str(), path.c_str()) == 0) {
					Util::SafeFormat(Aux1, sizeof(Aux1), "Script for %d saved.", creatureInst->actInst->mZone);
					SendInfoMessage(Aux1, INFOMSG_INFO);
					return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
				}
				else {
					LogMessageL(MSG_WARN, "[WARNING] Failed to rename %s to %s, old script will no longer be available, neither will new", tpath.c_str(), path.c_str());
					return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to rename new file, the script may now be missing!");
				}
			}
			else {
				LogMessageL(MSG_WARN, "[WARNING] Failed to remove %s, new script will not be available.", path.c_str());
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to delete old script file, new one not swapped in.");
			}
		}
		break;
	}
	default:
		SendInfoMessage("Not supported.", INFOMSG_ERROR);
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Not supported.");
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_petition_list(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

	vector<Petition> pets = g_PetitionManager.GetPetitions(pld.charPtr->cdef.CreatureDefID);
	vector<Petition>::iterator it;
	wpos += PutShort(&SendBuf[wpos], pets.size());
	struct tm * timeinfo;
	g_CharacterManager.GetThread("SimulatorThread::PetitionList");
	for(it = pets.begin(); it != pets.end(); ++it)
	{
		wpos += PutByte(&SendBuf[wpos], 8);
		wpos += PutStringUTF(&SendBuf[wpos], it->status == PENDING ? "pending" : "mine");
		CharacterData *petitioner = g_CharacterManager.GetPointerByID(it->petitionerCDefID);
		if(petitioner == NULL)
			wpos += PutStringUTF(&SendBuf[wpos], "<Deleted>");
		else
			wpos += PutStringUTF(&SendBuf[wpos], petitioner->cdef.css.display_name);
		sprintf(Aux1, "%d", it->petitionId);
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);
		if(petitioner == NULL)
			wpos += PutStringUTF(&SendBuf[wpos], "");
		else {
			AccountData * accData = g_AccountManager.FetchIndividualAccount(petitioner->AccountID);
			if(accData == NULL)
				wpos += PutStringUTF(&SendBuf[wpos], "<Missing account>");
			else
			{
				int c = 0;
				sprintf(Aux1, "");
				for(uint i = 0 ; i < accData->MAX_CHARACTER_SLOTS; i++) {
					if(accData->CharacterSet[i] != 0 && accData->CharacterSet[i] != petitioner->cdef.CreatureDefID) {
						CharacterCacheEntry *cce = accData->characterCache.ForceGetCharacter(accData->CharacterSet[i]);
						if(cce != NULL)
						{
							if(c > 0)
								strcat(Aux1, ",");
							strcat(Aux1, cce->display_name.c_str());
							c++;
						}
					}
				}
				wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			}
		}
		sprintf(Aux1, "%d", it->category);
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);
		wpos += PutStringUTF(&SendBuf[wpos], it->description);
		wpos += PutStringUTF(&SendBuf[wpos], "0");  // TODO score
		time_t ts;
		timeinfo = localtime (&ts);
		sprintf(Aux1, "%s", asctime(timeinfo));
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);
	}
	g_CharacterManager.ReleaseThread();

	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}

int SimulatorThread :: handle_query_itemdef_contents(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");


	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index
	wpos += PutShort(&SendBuf[wpos], 0);
	int id = atoi(query.args[0].c_str());
	g_CharacterManager.GetThread("SimulatorThread::PetitionList");
	CreatureInstance *creature = creatureInst->actInst->GetPlayerByID(id);
	int count = 0;
	if(creature != NULL)
	{
		CharacterData *cdata = creature->charPtr;
		int size = cdata->inventory.containerList[INV_CONTAINER].size();
		int a;
		WritePos = 0;
		for(a = 0; a < size; a++)
		{
			InventorySlot *slot = &cdata->inventory.containerList[INV_CONTAINER][a];
			if(slot != NULL) {
				wpos += PutByte(&SendBuf[wpos], 2);
				Util::SafeFormat(Aux1, sizeof(Aux1), "%d", slot->IID);
				wpos += PutStringUTF(&SendBuf[wpos], Aux1);
				wpos += PutStringUTF(&SendBuf[wpos], slot->dataPtr->mDisplayName.c_str());
				count++;
			}
		}
	}
	g_CharacterManager.ReleaseThread();
	PutShort(&SendBuf[7], count);
	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}

int SimulatorThread :: handle_query_itemdef_delete(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	int itemID = atoi(query.args[0].c_str());
	int targetID = atoi(query.args[1].c_str());
	g_CharacterManager.GetThread("SimulatorThread::ItemCreate");
	CreatureInstance *creature = creatureInst->actInst->GetPlayerByID(targetID);
	if(creature != NULL) {
		ItemDef * item;
		if(itemID == 0) {
			item = g_ItemManager.GetSafePointerByPartialName(query.args[0].c_str());
		}
		else {
			item = g_ItemManager.GetSafePointerByID(itemID);
		}
		if(item != NULL) {

			CharacterData *cdata = creature->charPtr;
			int size = cdata->inventory.containerList[INV_CONTAINER].size();
			int a;
			for(a = 0; a < size; a++)
			{
				InventorySlot *slot = &cdata->inventory.containerList[INV_CONTAINER][a];
				if(slot != NULL) {
					if(slot->IID == item->mID)
					{
						g_Log.AddMessageFormat("[SAGE] %s removed %s from %s because '%s'", pld.charPtr->cdef.css.display_name, item->mDisplayName.c_str(), creature->charPtr->cdef.css.display_name, query.args[2].c_str());
						int wpos = RemoveItemUpdate(&Aux1[0], Aux2, slot);
						g_CharacterManager.ReleaseThread();
						creature->simulatorPtr->AttemptSend(Aux1, wpos);
						cdata->inventory.RemItem(slot->CCSID);
						return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
					}
				}
			}
		}
	}
	g_CharacterManager.ReleaseThread();
	return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to delete item.");
}

int SimulatorThread :: handle_query_util_addfunds() {
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	if(query.args[0].compare("COPPER") == 0) {
		int amount = atoi(query.args[1].c_str());
		g_CharacterManager.GetThread("SimulatorThread::AddFunds");
		CreatureInstance *creature = creatureInst->actInst->GetPlayerByName(query.args[3].c_str());
		if(creature != NULL) {
			creature->AdjustCopper(amount);
			g_Log.AddMessageFormat("[SAGE] %s gave %s %d copper because '%s'",
					pld.charPtr->cdef.css.display_name, creature->charPtr->cdef.css.display_name, amount, query.args[2].c_str());
			g_CharacterManager.ReleaseThread();
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		}
		else {
			SendInfoMessage("Player must be logged on to receive funds.", INFOMSG_ERROR);
		}
		g_CharacterManager.ReleaseThread();
	}
	return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to add funds.");
}

int SimulatorThread :: handle_query_validate_name() {
	int res = g_AccountManager.ValidateNameParts(query.args[0].c_str(), query.args[1].c_str());
	if(res == AccountManager::CHARACTER_SUCCESS) {
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	else {
		Util::SafeFormat(Aux1, sizeof(Aux1), "Renamed to %s %s returned error %d", query.args[0].c_str(), query.args[1].c_str(), res);
		return PrepExt_QueryResponseError(SendBuf, query.ID, Aux1);
	}
}

int SimulatorThread :: handle_query_item_create(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	int itemID = atoi(query.args[0].c_str());
	int targetID = atoi(query.args[1].c_str());
	g_CharacterManager.GetThread("SimulatorThread::ItemCreate");
	CreatureInstance *creature = creatureInst->actInst->GetPlayerByID(targetID);
	if(creature != NULL) {
		ItemDef * item;
		if(itemID == 0) {
			item = g_ItemManager.GetSafePointerByPartialName(query.args[0].c_str());
		}
		else {
			item = g_ItemManager.GetSafePointerByID(itemID);
		}
		if(item != NULL) {
			int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
			if(slot != -1)
			{
				InventorySlot *sendSlot = creature->charPtr->inventory.AddItem_Ex(INV_CONTAINER, item->mID, 1);
				if(sendSlot != NULL)
				{
					g_Log.AddMessageFormat("[SAGE] %s gave %s to %s because '%s'", pld.charPtr->cdef.css.display_name, item->mDisplayName.c_str(), creature->charPtr->cdef.css.display_name, query.args[2].c_str());
					int wpos = AddItemUpdate(&Aux1[0], Aux2, sendSlot);
					g_CharacterManager.ReleaseThread();
					creature->simulatorPtr->AttemptSend(Aux1, wpos);
					return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
				}
			}

		}
	}
	g_CharacterManager.ReleaseThread();
	return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to give item.");
}

int SimulatorThread :: handle_query_petition_doaction(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int id = atoi(query.args[0].c_str());
	if(query.args[1].compare("take") == 0) {
		if(g_PetitionManager.Take(id, pld.CreatureDefID)) {
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		} else {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to take petition.");
		}
	}
	else if(query.args[1].compare("untake") == 0) {
		if(g_PetitionManager.Untake(id, pld.CreatureDefID)) {
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		} else {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to untake petition.");
		}
	}
	else if(query.args[1].compare("close") == 0) {
		if(g_PetitionManager.Close(id, pld.CreatureDefID)) {
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		} else {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to close petition.");
		}
	}
	else {
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Unknown petition op.");
	}

}

int SimulatorThread :: handle_query_petition_send(void)
{
	int pet = g_PetitionManager.NewPetition(pld.CreatureDefID, atoi(query.args[0].c_str()), query.args[1].c_str());
	if(pet > -1) {
		Util::SafeFormat(Aux1, sizeof(Aux1), "**%s has submitted petition %d, could the next available Earthsage please take the petition**",
				pld.charPtr->cdef.css.display_name, pet);
		char buffer[4096];
		int wpos = PrepExt_GenericChatMessage(buffer, 0, "Petition Manager", "gm/earthsages", Aux1);
		SIMULATOR_IT it;
		for(it = Simulator.begin(); it != Simulator.end(); ++it)
			if(it->isConnected == true && it->ProtocolState == 1 && it->pld.accPtr->HasPermission(Perm_Account, Permission_Sage))
				it->AttemptSend(buffer, wpos);
		return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
	}
	else {
		g_Log.AddMessageFormat("Failed to create petition.");
		return PrepExt_QueryResponseString(SendBuf, query.ID, "Failed"); //??
	}
}

int SimulatorThread :: handle_query_marker_del(void)
{
	bool ok = CheckPermissionSimple(Perm_Account, Permission_Admin);
	if(!ok) {
		if(pld.zoneDef->mGrove == true && pld.zoneDef->mAccountID != pld.accPtr->ID)
			ok = true;
	}
	if(!ok)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	vector<WorldMarker>::iterator it;

	for (it = creatureInst->actInst->worldMarkers.WorldMarkerList.begin(); it != creatureInst->actInst->worldMarkers.WorldMarkerList.end(); ++it) {
		if(strcmp(it->Name, query.args[0].c_str()) == 0) {
			cs.Enter("SimulatorThread::WorldMarkers");
			creatureInst->actInst->worldMarkers.WorldMarkerList.erase(it);
			creatureInst->actInst->worldMarkers.Save();
			cs.Leave();
			break;
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_marker_edit(void)
{
	bool ok = CheckPermissionSimple(Perm_Account, Permission_Admin);
	if(!ok) {
		if(pld.zoneDef->mGrove == true && pld.zoneDef->mAccountID != pld.accPtr->ID)
			ok = true;
	}
	if(!ok)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	vector<WorldMarker>::iterator it;
	for (it = creatureInst->actInst->worldMarkers.WorldMarkerList.begin(); it != creatureInst->actInst->worldMarkers.WorldMarkerList.end(); ++it) {
		if(strcmp(it->Name, query.args[0].c_str()) == 0) {
			cs.Enter("SimulatorThread::WorldMarkers");
			Util::SafeCopy(it->Name, query.args[2].c_str(), sizeof(it->Name));
			Util::SafeCopy(it->Comment, query.args[4].c_str(), sizeof(it->Comment));
			it->X = creatureInst->CurrentX;
			it->Y = creatureInst->CurrentY;
			it->Z = creatureInst->CurrentZ;
			creatureInst->actInst->worldMarkers.Save();
			cs.Leave();
			return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		}
	}
	g_Log.AddMessageFormat("Creating new marker %s in zone %d at %s.", query.args[2].c_str(), creatureInst->actInst->mZone, query.args[4].c_str());
	WorldMarker wm;
	cs.Enter("SimulatorThread::UpdateWorldMarkers");
	Util::SafeCopy(wm.Name, query.args[2].c_str(), sizeof(wm.Name));
	Util::SafeCopy(wm.Comment, query.args[4].c_str(), sizeof(wm.Comment));
	wm.X = creatureInst->CurrentX;
	wm.Y = creatureInst->CurrentY;
	wm.Z = creatureInst->CurrentZ;
	creatureInst->actInst->worldMarkers.WorldMarkerList.push_back(wm);
	creatureInst->actInst->worldMarkers.Save();
	cs.Leave();
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_item_market_buy(void)
{
	if(query.args.size() < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");
	int id = query.GetInteger(0);
	CS::CreditShopItem * csItem = g_CSManager.GetItem(id);
	g_CharacterManager.GetThread("SimulatorThread::MarketReload");
	if(csItem == NULL) {
		return PrepExt_QueryResponseError(SendBuf, query.ID, "No such item.");
	}
	else {

		if(csItem->mQuantityLimit != 0 && (csItem->mQuantityLimit - csItem->mQuantitySold) < 1) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Sold out!");
		}

		if(csItem->mPriceCurrency == Currency::COPPER && creatureInst->css.copper < csItem->mPriceAmount) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "You do not have enough copper!");
		}
		else if(csItem->mPriceCurrency == Currency::CREDITS && creatureInst->css.credits < csItem->mPriceAmount) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "You do not have enough credits!");
		}

		if(csItem->mStartDate !=0 && g_ServerTime < (csItem->mStartDate * 1000UL)) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "This item is not yet available!");
		}

		if(csItem->mEndDate !=0 && g_ServerTime >= (csItem->mEndDate * 1000UL)) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "This item is no longer available!");
		}

		ItemDef * item= g_ItemManager.GetSafePointerByID(csItem->mItemId);
		if(item == NULL) {
			return PrepExt_QueryResponseError(SendBuf, query.ID, "No such item!");
		}
		int slot = pld.charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot == -1)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "You do not have enough slots in your inventory!");


		InventorySlot *sendSlot = creatureInst->charPtr->inventory.AddItem_Ex(INV_CONTAINER, item->mID, 1);
		if(sendSlot == NULL)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to add item to inventory.");

		g_CharacterManager.GetThread("Simulator::MarketBuy");

		if(csItem->mPriceCurrency == Currency::COPPER) {
			creatureInst->css.copper -= csItem->mPriceAmount;
			creatureInst->SendStatUpdate(STAT::COPPER);
		}
		else if(csItem->mPriceCurrency == Currency::CREDITS) {
			creatureInst->css.credits -= csItem->mPriceAmount;
			creatureInst->SendStatUpdate(STAT::CREDITS);
		}

		if(csItem->mQuantityLimit > 0) {
			csItem->mQuantitySold++;
			g_CSManager.SaveItem(csItem);
		}

		int wpos = 0;
		wpos += AddItemUpdate(&SendBuf[wpos], Aux2, sendSlot);
		wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");

		g_CharacterManager.ReleaseThread();

		return wpos;
	}
}
int SimulatorThread :: handle_query_item_market_reload(void)
{
	g_CharacterManager.GetThread("SimulatorThread::MarketReload");
	g_CSManager.LoadItems();
	g_CharacterManager.ReleaseThread();
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_item_market_edit(void)
{
	if(query.args.size() < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	if(strcmp(query.GetString(0), "DELETE") == 0 && query.args.size() > 1) {
		int id = query.GetInteger(1);
		CS::CreditShopItem *item = g_CSManager.GetItem(id);
		if(item == NULL)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid item.");
		else {
			// TODO remove
			if(g_CSManager.RemoveItem(id)) {
				g_Log.AddMessageFormat("Removed credit shop item %d", item->mId);
				return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
			}
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Failed to remove.");
		}
	}
	else if(query.args.size() > 22) {
		CS::CreditShopItem * csItem;
		bool isNew = strcmp(query.GetString(0), "NEW") == 0;
		if(isNew) {
			csItem = new CS::CreditShopItem();
			csItem->mId = g_CSManager.nextMarketItemID++;
			SessionVarsChangeData.AddChange();
			Util::SafeFormat(Aux3, sizeof(Aux3), "Created market csItem %d", csItem->mId);
		}
		else {
			csItem = g_CSManager.GetItem(query.GetInteger(0));
			if(csItem == NULL)
				return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid item.");
			Util::SafeFormat(Aux3, sizeof(Aux3), "Save market csItem %d", csItem->mId);
		}
		csItem->mTitle = query.GetString(2);
		csItem->mDescription = query.GetString(4);
		csItem->mCategory = Category::GetIDByName(query.GetString(6));
		csItem->mStatus = Status::GetIDByName(query.GetString(8));
		Util::ParseDate(query.GetString(10), csItem->mStartDate);
		Util::ParseDate(query.GetString(12), csItem->mEndDate);
		csItem->mPriceAmount = query.GetInteger(14);
		csItem->mPriceCurrency = Currency::GetIDByName(query.GetString(16));
		csItem->mQuantityLimit = query.GetInteger(18);
		csItem->mQuantitySold = query.GetInteger(20);
		csItem->ParseItemProto(query.GetString(22));

		// Check the item
		ItemDef * item= g_ItemManager.GetSafePointerByID(csItem->mItemId);
		if(item == NULL) {
			if(isNew)
				delete csItem;
			return PrepExt_QueryResponseError(SendBuf, query.ID, "No such item!");
		}
		if(csItem->mTitle.compare(item->mDisplayName) == 0)
			csItem->mTitle = "";

		g_CSManager.SaveItem(csItem);
		g_CSManager.mItems[csItem->mId] = csItem;
		g_Log.AddMessageFormat("Created credit shop csItem %d", csItem->mId);
		return PrepExt_QueryResponseString(SendBuf, query.ID, Aux3);
	}
	return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid sub-query.");
}

int SimulatorThread :: handle_query_item_market_list(void)
{
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

	wpos += PutShort(&SendBuf[wpos], 0);

	uint count = 0;
	for(std::map<int, CS::CreditShopItem*>::iterator it = g_CSManager.mItems.begin(); it != g_CSManager.mItems.end(); ++it)
	{
		if(g_ServerTime < (it->second->mStartDate * 1000UL))
			// Not available yet
			continue;

		ItemDef * item= g_ItemManager.GetSafePointerByID(it->second->mItemId);
		if(item != NULL)
		{

			wpos += PutByte(&SendBuf[wpos], 12);


			sprintf(Aux1, "%d", it->second->mId);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			if(it->second->mTitle.size() == 0)
				sprintf(Aux1, "%s", item->mDisplayName.c_str());
			else
				sprintf(Aux1, "%s", it->second->mTitle.c_str());
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", it->second->mDescription.c_str());
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", Status::GetNameByID(it->second->mStatus));
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", Category::GetNameByID(it->second->mCategory));
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", it->second->mStartDate == 0 ? "" :
							 Util::FormatDate(&it->second->mEndDate).c_str());
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", it->second->mEndDate == 0 ? "" :
					( g_ServerTime >= ( it->second->mEndDate * 1000UL ) ?
							"Expired" : Util::FormatDate(&it->second->mEndDate).c_str()));
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%lu", it->second->mPriceAmount);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%s", Currency::GetNameByID(it->second->mPriceCurrency));
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%d", it->second->mQuantityLimit);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%d", it->second->mQuantitySold);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%d:%d:%d:%d", it->second->mItemId, it->second->mLookId, it->second->mIv1, it->second->mIv2);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			count++;

		}
	}
	PutShort(&SendBuf[7], count);
	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}


int SimulatorThread :: handle_query_marker_list(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Admin))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	if(query.args[0] == "zone") {
		// Do a reload so we get external updates too
		cs.Enter("SimulatorThread::WorldMarkers");
		creatureInst->actInst->worldMarkers.Reload();
		cs.Leave();

		int wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
		wpos += PutShort(&SendBuf[wpos], 0);      //Message size
		wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

		wpos += PutShort(&SendBuf[wpos], creatureInst->actInst->worldMarkers.WorldMarkerList.size());
		vector<WorldMarker>::iterator it;

		for(it = creatureInst->actInst->worldMarkers.WorldMarkerList.begin(); it != creatureInst->actInst->worldMarkers.WorldMarkerList.end(); ++it)
		{
			wpos += PutByte(&SendBuf[wpos], 4);
			sprintf(Aux1, "%s", it->Name);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			sprintf(Aux1, "%d", creatureInst->actInst->mZone);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			sprintf(Aux1, "%f %f %f", it->X,it->Y,it->Z);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			sprintf(Aux1, "%s", it->Comment);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);
		}

		PutShort(&SendBuf[1], wpos - 3);
		return wpos;
	}
	else {
		g_Log.AddMessageFormat("TODO Implement non-zone marker list query. %s", query.args[0].c_str());
	}
	return 0;
}

int SimulatorThread :: handle_query_guild_info(void)
{
	/*  Query: guild.info
		Retrieves the guild info of the Simulator player.
		Args: [none]
	*/
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index

	if(pld.charPtr->guildList.size() == 0) {
		wpos += PutShort(&SendBuf[wpos], 0);
	}
	else {
		wpos += PutShort(&SendBuf[wpos], pld.charPtr->guildList.size());
		for(uint i = 0 ; i < pld.charPtr->guildList.size(); i++)
		{

			//local defID = r[0];
			//local valour = r[1];
			//local guildType = r[2];
			//local name = r[3];
			//local motto = r[4];
			//local rankTitle = r[5];
			//local rankLevel = r[6];

			GuildDefinition *gdef = g_GuildManager.GetGuildDefinition(pld.charPtr->guildList[i].GuildDefID);
			wpos += PutByte(&SendBuf[wpos], 7);



			sprintf(Aux1, "%d", pld.charPtr->guildList[i].GuildDefID);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%d", pld.charPtr->guildList[i].Valour);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			sprintf(Aux1, "%d", gdef->guildType);
			wpos += PutStringUTF(&SendBuf[wpos], Aux1);

			wpos += PutStringUTF(&SendBuf[wpos], gdef->defName);

			wpos += PutStringUTF(&SendBuf[wpos], gdef->motto);

			GuildRankObject *rank = g_GuildManager.GetRank(pld.CreatureDefID, gdef->guildDefinitionID);
			if(rank == NULL) {
				wpos += PutStringUTF(&SendBuf[wpos], "No rank");
				wpos += PutStringUTF(&SendBuf[wpos], "0");
			}
			else {

				wpos += PutStringUTF(&SendBuf[wpos], rank->title.c_str());
				sprintf(Aux1, "%d", rank->rank);
				wpos += PutStringUTF(&SendBuf[wpos], Aux1);
			}
		}
	}

	PutShort(&SendBuf[1], wpos - 3);
	return wpos;
}

int SimulatorThread :: handle_query_guild_leave(void)
{
	int guildDefID = atoi(query.args[0].c_str());

	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Guild ID required");
	QuestDefinitionContainer::ITERATOR it;
		std::vector<int> defs;

	// Make sure any guild quests are abandoned
	for(it = QuestDef.mQuests.begin(); it != QuestDef.mQuests.end(); ++it)
	{
		QuestDefinition *qd = &it->second;
		if(qd->guildId == guildDefID)
		{
			int QID = qd->questID;
			if(pld.charPtr->questJournal.activeQuests.HasQuestID(QID))
			{
				if(qd->unabandon == true)
				{
					SendInfoMessage("You cannot abandon that quest.", INFOMSG_ERROR);
					QID = 0;  //Set to zero so it's not actually removed in the server or the client.
				}
				pld.charPtr->questJournal.QuestLeave(pld.CreatureID, QID);
			}
		}
	}

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&SendBuf[wpos], query.ID);  //Query response index
	wpos += PutShort(&SendBuf[wpos], defs.size());
	for(uint i = 0 ; i < defs.size() ; i++) {
		sprintf(Aux1, "%d", defs[i]);
		wpos += PutByte(&SendBuf[wpos], 1);
		wpos += PutStringUTF(&SendBuf[wpos], Aux1);
	}
	PutShort(&SendBuf[1], wpos - 3);
	AttemptSend(SendBuf, wpos);

	pld.charPtr->LeaveGuild(guildDefID);
	BroadcastGuildChange(guildDefID);
	pld.charPtr->pendingChanges++;
	pld.charPtr->cdef.css.SetSubName(NULL);
	creatureInst->SendStatUpdate(STAT::SUB_NAME);

	return 0;
}

//
// Helper functions
//

bool SimulatorThread :: QuestJoin(int QuestID)
{
	if(creatureInst == NULL | creatureInst->charPtr == NULL)
		return false;
	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qdef == NULL)
		return false;
	if(qdef->mScriptAcceptCondition.ExecuteAllCommands(this) < 0)
		return false;
	qdef->mScriptAcceptAction.ExecuteAllCommands(this);
	int wpos = creatureInst->charPtr->questJournal.QuestJoin(&Aux1[0], QuestID, query.ID);
	g_QuestNutManager.AddActiveScript(creatureInst, QuestID);
	AttemptSend(Aux1, wpos);
	return true;
}

bool SimulatorThread :: QuestResetObjectives(int QuestID, int objective)
{
	if(creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;
	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if(qdef == NULL)
		return false;
	creatureInst->charPtr->questJournal.QuestResetObjectives(creatureInst->CreatureID, QuestID);
	int wpos = PrepExt_QuestStatusMessage(Aux1, QuestID, objective, false, qdef->actList[creatureInst->charPtr->questJournal.GetCurrentAct(QuestID)].objective[objective].completeText);
	AttemptSend(Aux1, wpos);
	return true;
}

bool SimulatorThread :: QuestClear(int QuestID)
{
	if(creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;
	creatureInst->charPtr->questJournal.QuestClear(creatureInst->CreatureID, QuestID);
	int wpos = PutByte(&Aux1[0], 7);  //_handleQuestEventMsg
	wpos += PutShort(&Aux1[wpos], 0); //Size
	wpos += PutInteger(&Aux1[wpos], QuestID); //Quest ID
	wpos += PutByte(&Aux1[wpos], QuestObjective::EVENTMSG_ABANDONED);
	PutShort(&Aux1[1], wpos - 3);
	AttemptSend(Aux1, wpos);
	return true;
}

int SimulatorThread :: OfferLoot(int mode, ActiveLootContainer *loot, ActiveParty *party, CreatureInstance *receivingCreature, int ItemID, bool needOrGreed, int CID, int conIndex)
{
	int WriteIdx = 0;
	if(mode == ROUND_ROBIN) {
		// Offer to the robin first
		LogMessageL(MSG_SHOW, "Offer Loot Round Robin");
		PartyMember *robin = party->GetNextLooter();

		loot->robinID = robin->mCreatureID;

		// Offer to the robin first
		LootTag * tag = party->TagItem(ItemID, robin->mCreaturePtr->CreatureID, CID);
		int slot = robin->mCreaturePtr->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		tag->mSlotIndex = slot;
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag->lootTag);
		WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3, false);
		robin->mCreaturePtr->actInst->LSendToOneSimulator(SendBuf, WriteIdx, robin->mCreaturePtr->simulatorPtr);
		return 1;
	}

	if(mode == LOOT_MASTER) {
		LogMessageL(MSG_SHOW, "Offer Loot Master");
		// Offer to the leader first
		PartyMember *leader = party->GetMemberByID(party->mLeaderID);
		LootTag * tag = party->TagItem(ItemID, leader->mCreaturePtr->CreatureID, CID);
		int slot = leader->mCreaturePtr->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		tag->mSlotIndex = slot;
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag->lootTag);
		WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3, false);
		leader->mCreaturePtr->actInst->LSendToOneSimulator(SendBuf, WriteIdx, leader->mCreaturePtr->simulatorPtr);
		return 1;
	}


	// First tags for other party members
	int offers = 0;
	LogMessageL(MSG_SHOW, "Offering to %d party member", party->mMemberList.size());
	for(uint i = 0 ; i < party->mMemberList.size(); i++) {
		if(receivingCreature == NULL || party->mMemberList[i].mCreatureID != receivingCreature->CreatureID) {
			// Only send offer to players in range
			int distCheck = protected_CheckDistanceBetweenCreaturesFor(party->mMemberList[i].mCreaturePtr, CID, PARTY_LOOT_RANGE);
			if(distCheck == 0)
			{
				LootTag * tag = party->TagItem(ItemID, party->mMemberList[i].mCreaturePtr->CreatureID, CID);
				Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag->lootTag);
				g_Log.AddMessageFormat("[LOOT] Sending offer of %d to %d using tag %s", ItemID, party->mMemberList[i].mCreatureID, Aux3);
				WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3, needOrGreed);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(SendBuf, WriteIdx, party->mMemberList[i].mCreaturePtr->simulatorPtr);
				offers++;
			}
			else
			{
				LogMessageL(MSG_SHOW, "%d is too far away from %d to receive loot (%d)", party->mMemberList[i].mCreaturePtr->CreatureID, CID, distCheck);
			}
		}
	}

	// Now the tag for the looting creature. We send the slot with this one
	if(mode > -1)
	{
		LogMessageL(MSG_SHOW, "Offering loot to looter");
		LootTag * tag = party->TagItem(ItemID, receivingCreature->CreatureID, CID);
		int slot = receivingCreature->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		tag->mSlotIndex = slot;
		offers++;

		STRINGLIST qresponse;
		qresponse.push_back("OK");
		sprintf(Aux3, "%d", conIndex);
		qresponse.push_back(Aux3);
		WriteIdx= PrepExt_QueryResponseStringList(&SendBuf[WriteIdx], query.ID, qresponse);
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag->lootTag);
		g_Log.AddMessageFormat("[LOOT] Sending offer of %d to original looter (%d) using tag %s", ItemID, receivingCreature->CreatureID, Aux3);
		WriteIdx+= PartyManager::OfferLoot(SendBuf, ItemID, Aux3, needOrGreed);
		receivingCreature->actInst->LSendToOneSimulator(SendBuf, WriteIdx, receivingCreature->simulatorPtr);
	}

	return offers;
}

void SimulatorThread :: RunTranslocate(void)
{
	if(pld.charPtr->bindReturnPoint[3] == 0)
	{
		int wpos = PrepExt_SendInfoMessage(GSendBuf, "Cannot translocate.", INFOMSG_ERROR);
		AttemptSend(GSendBuf, wpos);
	}
	else
	{
		MainCallSetZone(pld.charPtr->bindReturnPoint[3], 0, false);
		SetPosition(pld.charPtr->bindReturnPoint[0], pld.charPtr->bindReturnPoint[1], pld.charPtr->bindReturnPoint[2], 1);
	}
}

void SimulatorThread :: RunPortalRequest(void)
{
	if(strlen(pld.PortalRequestDest) == 0)
	{
		SendInfoMessage("You do not have a portal destination established.", INFOMSG_ERROR);
		return;
	}

	InteractObject *iobj = g_InteractObjectContainer.GetHengeByTargetName(pld.PortalRequestDest);
	if(iobj == NULL)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "Portal request target not found: %s", pld.PortalRequestDest);
		LogMessageL(MSG_WARN, "[WARNING] %s", Aux1);
		SendInfoMessage(Aux1, INFOMSG_ERROR);
		return;
	}

	int wpos = PrepExt_RemoveCreature(SendBuf, creatureInst->CreatureID);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ, InternalID);

	MainCallSetZone(iobj->WarpID, 0, false);

	SetPosition(iobj->WarpX, iobj->WarpY, iobj->WarpZ, 1);
	CheckSpawnTileUpdate(true);
	CheckMapUpdate(true);
	pld.ClearPortalRequestDest();
}

void SimulatorThread :: JoinPrivateChannel(const char *channelname, const char *password)
{
	char strBuf[100];
	std::string returnData;
	int res = g_ChatChannelManager.JoinChannel(InternalID, creatureInst->css.display_name, channelname, password, returnData);
	bool success = false;
	switch(res)
	{
	case ChatChannelManager::RESULT_ALREADY_IN_CHANNEL:
		Util::SafeFormat(strBuf, sizeof(strBuf), "You must leave [%s] before joining another channel.", returnData.c_str());
		break;
	case ChatChannelManager::RESULT_PASSWORD_REQUIRED:
		Util::SafeFormat(strBuf, sizeof(strBuf), "Channel [%s] requires a password.", channelname);
		break;
	case ChatChannelManager::RESULT_CHANNEL_FAILED:
		Util::SafeFormat(strBuf, sizeof(strBuf), "Unable to create channel [%s]", channelname);
		break;
	case ChatChannelManager::RESULT_CHANNEL_CREATED:
		Util::SafeFormat(strBuf, sizeof(strBuf), "You have created channel [%s].", channelname);
		success = true;
		break;
	case ChatChannelManager::RESULT_JOIN_SUCCESS:
		Util::SafeFormat(strBuf, sizeof(strBuf), "You have joined channel [%s].", channelname);
		success = true;
		break;
	case ChatChannelManager::RESULT_BAD_CHANNEL:
		Util::SafeFormat(strBuf, sizeof(strBuf), "No channel name provided.");
		break;
	case ChatChannelManager::RESULT_CHANNEL_NAME_SIZE:
		Util::SafeFormat(strBuf, sizeof(strBuf), "The channel name must not exceed %d characters.", ChatChannelManager::MAX_CHANNEL_NAME_SIZE);
		break;
	case ChatChannelManager::RESULT_UNHANDLED:
	default:
		Util::SafeFormat(strBuf, sizeof(strBuf), "Unhandled error");
		break;
	}
	int msgType = INFOMSG_ERROR;
	if(success == true)
	{
		pld.charPtr->SetLastChannel(channelname, password);
		msgType = INFOMSG_INFO;
	}
	
	int wpos = PrepExt_SendInfoMessage(Aux3, strBuf, msgType);
	AttemptSend(Aux3, wpos);
}

int SimulatorThread :: handle_query_ps_join(void)
{
	const char *name = NULL;
	const char *password = NULL;
	if(query.argCount >= 1)
		name = query.GetString(0);
	if(query.argCount >= 2)
		password = query.GetString(1);

	JoinPrivateChannel(name, password);

	return PrepExt_QueryResponseNull(SendBuf, query.ID);
}

int SimulatorThread :: handle_query_ps_leave(void)
{
	for(int i = 0; i < query.argCount; i++)
		LogMessage("%d=%s", i, query.args[i].c_str());

	const char *name = NULL;
	if(query.argCount >= 1)
		name = query.GetString(0);

	int res = g_ChatChannelManager.LeaveChannel(InternalID, name);
	if(res == ChatChannelManager::RESULT_NOT_IN_CHANNEL)
	{
		SendInfoMessage("You are not in that channel.", INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(SendBuf, query.ID);
	}

	pld.charPtr->SetLastChannel(NULL, NULL);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: CreatureUseHenge(int creatureID, int creatureDefID)
{
	//Helper function to perform the actions when activating a Henge object.
	if(pld.charPtr->HengeHas(creatureDefID) == false)
	{
		InteractObject *iobj = NULL;
		iobj = g_InteractObjectContainer.GetHengeByDefID(creatureDefID);
		if(iobj == NULL)
		{
			g_Log.AddMessageFormat("[WARNING] Henge not found in interact list: %d", creatureDefID);
			return;
		}

		Util::SafeFormat(Aux1, sizeof(Aux1), "You have discovered %s", iobj->useMessage);
		SendInfoMessage(Aux1, INFOMSG_INFO);
		pld.charPtr->HengeAdd(creatureDefID);
	}

	std::vector<InteractObject*> objectSearch;
	for(size_t i = 0; i < pld.charPtr->hengeList.size(); i++)
	{
		InteractObject *iobj = NULL;
		iobj = g_InteractObjectContainer.GetHengeByDefID(pld.charPtr->hengeList[i]);
		if(iobj != NULL)
			objectSearch.push_back(iobj);
	}

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 4);   //_handleCreatureEventMsg
	wpos += PutShort(&SendBuf[wpos], 0);  //size

	wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);
	wpos += PutByte(&SendBuf[wpos], 14);  //Event for henge click

	wpos += PutInteger(&SendBuf[wpos], creatureID);
	wpos += PutByte(&SendBuf[wpos], objectSearch.size());

	for(size_t i = 0; i < objectSearch.size(); i++)
	{
		//Henge Name, Cost
		wpos += PutStringUTF(&SendBuf[wpos], objectSearch[i]->useMessage);
		wpos += PutInteger(&SendBuf[wpos], objectSearch[i]->cost);
	}
	PutShort(&SendBuf[1], wpos - 3);  //size

	AttemptSend(SendBuf, wpos);
}


int SimulatorThread :: handle_query_mod_setgrovestart(void)
{
	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	pld.zoneDef->ChangeDefaultLocation(creatureInst->CurrentX, creatureInst->CurrentY, creatureInst->CurrentZ);
	g_ZoneDefManager.NotifyConfigurationChange();
	SendInfoMessage("Set grove entrance location to your coordinates.", INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_setenvironment(void)
{
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	const char *env = query.args[0].c_str();

	pld.zoneDef->ChangeEnvironment(env);
	g_ZoneDefManager.NotifyConfigurationChange();
	SendInfoMessage("Environment type changed.", INFOMSG_INFO);

	int wpos = PrepExt_SendEnvironmentUpdateMsg(SendBuf, pld.CurrentZone, pld.zoneDef, -1, -1);
	wpos += PrepExt_SendTimeOfDayMsg(&SendBuf[wpos], GetTimeOfDay());
	creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);

	//	SendZoneInfo();
	//LogMessageL(MSG_SHOW, "Environment set.");
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_grove_togglecycle(void)
{
	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");
	pld.zoneDef->ChangeEnvironmentUsage();
	g_ZoneDefManager.NotifyConfigurationChange();
	Util::SafeFormat(Aux1, sizeof(Aux1), "Environment cycling is now %s", (pld.zoneDef->mEnvironmentCycle ? "ON":"OFF"));
	SendInfoMessage(Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_mod_setats(void)
{
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");
	//if(pld.zoneDef->mGrove == false)
	//	return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	//if(pld.zoneDef->mAccountID != pld.accPtr->ID)
	//	return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	const char *atsName = query.args[0].c_str();
	if(g_SceneryManager.ValidATSEntry(atsName) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "ATS name is not valid.");

	int success = 0;
	int failed = 0;
	int wpos = 0;
	g_SceneryManager.GetThread("SimulatorThread::handle_query_mod_setats");
	for(int i = 1; i < query.argCount; i += 2)
	{
		int propID = atoi(query.args[i].c_str());
		const char *assetStr = query.args[i + 1].c_str();
		//SceneryObject *so = g_SceneryManager.GetPropPtr(pld.CurrentZoneID, propID, NULL);
		SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(pld.CurrentZoneID, propID, NULL);
		if(so != NULL)
		{
			int check = 0;
			if(strstr(assetStr, "Bldg-") != NULL)
				check++;
			if(strstr(assetStr, "Cav-") != NULL)
				check++;
			if(strstr(assetStr, "Dng-") != NULL)
				check++;
			if(check == 0)
			{
				failed++;
				continue;
			}
			SceneryObject replaceProp;
			replaceProp.copyFrom(so);
			std::string newAsset = replaceProp.Asset;
			uint pos = newAsset.find("?ATS=");
			if(pos != string::npos)
			{
				newAsset.erase(pos + 5, newAsset.length());  //Erase everything after "?ATS="
				newAsset.append(atsName);
			}
			else
			{
				newAsset.append("?ATS=");
				newAsset.append(atsName);
			}
			replaceProp.SetAsset(newAsset.c_str());
			LogMessageL(MSG_SHOW, "Setting prop: %d to asset: %s", propID, newAsset.c_str());

			if(HasPropEditPermission(&replaceProp) == true)
			{
				g_SceneryManager.ReplaceProp(pld.CurrentZoneID, replaceProp);

				wpos += PrepExt_UpdateScenery(&SendBuf[wpos], &replaceProp);
				if(wpos >= Global::MAX_SEND_CHUNK_SIZE)
				{
					creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
					wpos = 0;
				}
				success++;
			}
			else
				failed++;
		}
	}
	g_SceneryManager.ReleaseThread();

	if(wpos > 0)
		creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);

	Util::SafeFormat(Aux1, sizeof(Aux1), "Attempted on %d props (%d success, %d failed)", success + failed, success, failed);
	SendInfoMessage(Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_getURL(void)
{
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, g_URLManager.GetURLs());
}


/* OBSOLETE
int SimulatorThread :: handle_query_mod_igforum_createcategory(void)
{
	//[0] = category ID to place the new category into
	//[1] = name of category
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int ID = atoi(query.args[0].c_str());
	const char *name = query.args[1].c_str();
	int res = g_IGFManager.CreateCategory(pld.accPtr, ID, name);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}
*/

int SimulatorThread :: handle_query_mod_igforum_editobject(void)
{
	//[0] = Object Type (category or thread)
	//[1] = Parent ID (if creating a category, this is where to place it)
	//[2] = Rename ID (nonzero indicates which ID is being renamed)
	//[3] = String name to give the object
	if(query.argCount < 4)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int type = atoi(query.args[0].c_str());
	int parentID = atoi(query.args[1].c_str());
	int renameID = atoi(query.args[2].c_str());
	const char *name = query.args[3].c_str();

	int res = g_IGFManager.EditObject(pld.accPtr, type, parentID, renameID, name);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_getcategory(void)
{
	//[0] = category ID
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");
	int ID = atoi(query.args[0].c_str());

	MULTISTRING results;
	g_IGFManager.GetCategory(ID, results);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, results);
}

int SimulatorThread :: handle_query_mod_igforum_opencategory(void)
{
	//[0] = object type (ex: category or thread)
	//[1] = object ID
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int type = atoi(query.args[0].c_str());
	int id = atoi(query.args[1].c_str());

	MULTISTRING results;
	g_IGFManager.OpenCategory(type, id, results);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, results);
}

int SimulatorThread :: handle_query_mod_igforum_openthread(void)
{
	//[0] = thread ID
	//[1] = starting post index
	//[2] = number of posts to retrieve
	if(query.argCount < 3)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int id = atoi(query.args[0].c_str());
	int start = atoi(query.args[1].c_str());
	int count = atoi(query.args[2].c_str());

	MULTISTRING results;
	g_IGFManager.OpenThread(id, start, count, results);
	uint wpos = PrepExt_QueryResponseMultiString(SendBuf, query.ID, results);

	if(wpos >= sizeof(SendBuf) - 1)
		LogMessageL(MSG_CRIT, "[CRITICAL]: IGF openthread response too large: %d", wpos);
	else if(wpos >= sizeof(SendBuf) - 1000)
		LogMessageL(MSG_CRIT, "[WARNING]: IGF openthread response dangerous size: %d", wpos);

	return wpos;//PrepExt_QueryResponseMultiString(SendBuf, query.ID, results);
}

int SimulatorThread :: handle_query_mod_igforum_sendpost(void)
{
	//[0] = Post Type
	//[1] = Placement ID (category ID if creating a thread, thread ID if creating or editing a post)
	//[2] = Post ID (if editing a post)
	//[3] = thread title, if creating a new thread
	//[4] = post body
	if(query.argCount < 5)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int type = atoi(query.args[0].c_str());
	int placementID = atoi(query.args[1].c_str());
	int postID = atoi(query.args[2].c_str());
	const char *threadTitle = query.args[3].c_str();
	const char *postBody = query.args[4].c_str();

	const char *displayName = creatureInst->css.display_name;
	int res = g_IGFManager.SendPost(pld.accPtr, type, placementID, postID, threadTitle, postBody, displayName);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_deletepost(void)
{
	//[0] = thread ID
	//[1] = post ID
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");
	int threadID = atoi(query.args[0].c_str());
	int postID = atoi(query.args[1].c_str());
	
	int res = g_IGFManager.DeletePost(pld.accPtr, threadID, postID);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

// TODO: OBSOLETE
/*
int SimulatorThread :: handle_query_mod_igforum_deletethread(void)
{
	//[0] = Category ID
	//[1] = Thread ID
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int categoryID = atoi(query.args[0].c_str());
	int threadID = atoi(query.args[1].c_str());

	int res = g_IGFManager.DeleteThread(pld.accPtr, categoryID, threadID);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}
*/

int SimulatorThread :: handle_query_mod_igforum_deleteobject(void)
{
	//[0] = Object Type (category or thread)
	//[1] = Object ID
	if(query.argCount < 2)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int objectType = atoi(query.args[0].c_str());
	int objectID = atoi(query.args[1].c_str());

	int res = g_IGFManager.DeleteObject(pld.accPtr, objectType, objectID);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_runaction(void)
{
	if(query.argCount >= 3)
	{
		int action = query.GetInteger(0);
		int param1 = query.GetInteger(1);
		int param2 = query.GetInteger(2);
		int res = g_IGFManager.RunAction(pld.accPtr, action, param1, param2);
		if(res < 0)
			return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_move(void)
{
	if(query.argCount >= 4)
	{
		int srcType = query.GetInteger(0);
		int srcID = query.GetInteger(1);
		int dstType = query.GetInteger(2);
		int dstID = query.GetInteger(3);
		int res = g_IGFManager.RunMove(pld.accPtr, srcType, srcID, dstType, dstID);
		if(res < 0)
			return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_setlockstatus(void)
{
	//[0] = Type (Category or Thread)
	//[1] = Object ID
	//[2] = Lock status (0 or 1)
	if(query.argCount < 3)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int type = atoi(query.args[0].c_str());
	int objectID = atoi(query.args[1].c_str());
	bool status = Util::IntToBool(atoi(query.args[2].c_str()));

	int res = g_IGFManager.SetLockStatus(pld.accPtr, type, objectID, status);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_igforum_setstickystatus(void)
{
	//[0] = Type (Category or Thread)
	//[1] = Object ID
	//[2] = Sticky Status (0 or 1)
	if(query.argCount < 3)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	int type = atoi(query.args[0].c_str());
	int objectID = atoi(query.args[1].c_str());
	bool status = Util::IntToBool(atoi(query.args[2].c_str()));

	int res = g_IGFManager.SetStickyStatus(pld.accPtr, type, objectID, status);
	if(res < 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_itempreview(void)
{
	//[0] = Type (0 = invisible, 1 = nude, 2 = clone)
	//[1] = Item ID
	if(query.argCount < 2 || query.argCount > 66) //32 slots * 2, + 2 params should be plenty.  Bail on greater to be safe.
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Invalid query.");

	static const char *invis = "n4:{[\"c\"]=\"Horde-Invisible_Biped\",[\"sk\"]={[\"main\"]=\"FFFFFF\"},[\"sz\"]=\"1.0\"}";

	int type = atoi(query.args[0].c_str());
	int itemID = atoi(query.args[1].c_str());

	ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
	if(itemDef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Server error.");

	std::vector<int> eqArray;
	for(int i = 2; i < query.argCount; i++)
		eqArray.push_back(atoi(query.args[i].c_str()));

	int targetEquipSlot = 0;
	switch(itemDef->mEquipType)
	{
	case ItemEquipType::NONE: targetEquipSlot = ItemEquipSlot::NONE; break;
	case ItemEquipType::WEAPON_1H: targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND; break;
	case ItemEquipType::WEAPON_1H_UNIQUE: targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND; break;
	case ItemEquipType::WEAPON_1H_MAIN: targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND; break;
	case ItemEquipType::WEAPON_1H_OFF: targetEquipSlot = ItemEquipSlot::WEAPON_OFF_HAND; break;
	case ItemEquipType::WEAPON_2H: targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND; break;
	case ItemEquipType::WEAPON_RANGED: targetEquipSlot = ItemEquipSlot::WEAPON_RANGED; break;
	case ItemEquipType::ARMOR_SHIELD: targetEquipSlot = ItemEquipSlot::WEAPON_OFF_HAND; break;
	case ItemEquipType::ARMOR_HEAD: targetEquipSlot = ItemEquipSlot::ARMOR_HEAD; break;
	case ItemEquipType::ARMOR_NECK: targetEquipSlot = ItemEquipSlot::ARMOR_NECK; break;
	case ItemEquipType::ARMOR_SHOULDER: targetEquipSlot = ItemEquipSlot::ARMOR_SHOULDER; break;
	case ItemEquipType::ARMOR_CHEST: targetEquipSlot = ItemEquipSlot::ARMOR_CHEST; break;
	case ItemEquipType::ARMOR_ARMS: targetEquipSlot = ItemEquipSlot::ARMOR_ARMS; break;
	case ItemEquipType::ARMOR_HANDS: targetEquipSlot = ItemEquipSlot::ARMOR_HANDS; break;
	case ItemEquipType::ARMOR_WAIST: targetEquipSlot = ItemEquipSlot::ARMOR_WAIST; break;
	case ItemEquipType::ARMOR_LEGS: targetEquipSlot = ItemEquipSlot::ARMOR_LEGS; break;
	case ItemEquipType::ARMOR_FEET: targetEquipSlot = ItemEquipSlot::ARMOR_FEET; break;
	case ItemEquipType::ARMOR_RING: targetEquipSlot = ItemEquipSlot::ARMOR_RING_L; break;
	case ItemEquipType::ARMOR_RING_UNIQUE: targetEquipSlot = ItemEquipSlot::ARMOR_RING_L; break;
	case ItemEquipType::ARMOR_AMULET: targetEquipSlot = ItemEquipSlot::ARMOR_AMULET; break;
	default: 
		targetEquipSlot = ItemEquipSlot::NONE;
	}
	if(targetEquipSlot == ItemEquipSlot::NONE)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Server error.");

	if(type == 0 || type == 1)
	{
		eqArray.clear();
		eqArray.push_back(targetEquipSlot);
		eqArray.push_back(itemID);
	}
	else
	{
		for(size_t i = 0; i < eqArray.size(); i += 2)
		{
			if(eqArray[i] == targetEquipSlot)
				eqArray[i + 1] = itemID;
		}
	}

	int wpos = 0;
	Aux1[wpos++] = '{';
	for(size_t i = 0; i < eqArray.size(); i+=2)
	{
		if(i > 0)
			Aux1[wpos++] = ',';
		wpos += sprintf(&Aux1[wpos], "[%d]=%d", eqArray[i], eqArray[i + 1]);
	}
	Aux1[wpos++] = '}';

	CreatureInstance *cptr = creatureInst->actInst->SpawnCreate(creatureInst, 3509);  //Generic interact object, the details will be replaced.
	if(cptr == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Server error.");

	Util::SafeCopy(cptr->css.display_name, creatureInst->css.display_name, sizeof(cptr->css.display_name));
	Util::SafeCopy(cptr->css.sub_name, "Item Preview", sizeof(cptr->css.sub_name));
	cptr->css.aggro_players = 0;
	cptr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
	cptr->_AddStatusList(StatusEffects::INVINCIBLE, -1);

	if(type == 0)
		cptr->css.SetAppearance(invis);
	else
		cptr->css.SetAppearance(creatureInst->css.appearance.c_str());
	cptr->css.SetEqAppearance(Aux1);
	
	cptr->SetServerFlag(ServerFlags::TriggerDelete, true);
	cptr->deathTime = g_ServerTime;


	CreatureDefinition cd;
	cd.CopyFrom(pld.charPtr->cdef);
	cd.DefHints = 0;
	cd.CreatureDefID = 1;
	cd.css.CopyFrom(&cptr->css);

	cptr->CreatureDefID = 1;

	int size = PrepExt_CreatureDef(SendBuf, &cd);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, size, creatureInst->CurrentX, creatureInst->CurrentZ);

	size = PrepExt_CreatureFullInstance(SendBuf, cptr);
	//int size = PrepExt_GeneralMoveUpdate(SendBuf, cptr);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, size, creatureInst->CurrentX, creatureInst->CurrentZ);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_restoreappearance(void)
{
	//One string per row.
	MULTISTRING output;
	STRINGLIST row;
	row.push_back(pld.charPtr->cdef.css.appearance);
	output.push_back(row);
	row[0] = pld.charPtr->cdef.css.eq_appearance;
	output.push_back(row);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, output);
}

void SimulatorThread :: ShardSet(const char *shardName, const char *charName)
{
	/*
	if(creatureInst->Speed != 0)
	{
		SendInfoMessage("You must be stationary.", INFOMSG_ERROR);
		return;
	}
	if(shardName == NULL)
		return;
	if(chardName != NULL)
	{
		unsigned long time = PlatformTime::getAbsoluteMilliseconds();
		time += 900000;  //15 minutes = warp timer;
		if(pld.charPtr->LastWarpTime + 900000 > time)
		{
			unsigned long difference = (time - LastWarpTime) / 1000;
			Util::FormatTime(Aux2, sizeof(Aux2), difference);
			Util::SafeFormat(Aux1, sizeof(Aux1), "You can't do that yet. Remaining time: %s", Aux2);
			SendInfoMessage(Aux1, INFOMSG_ERROR);
			return;
		}
		SimulatorThread *sim == GetSimulatorByCharacterName(charName);
		if(sim == NULL)
		{
			SendInfoMessage("That player is not logged in.", INFOMSG_ERROR);
			return;
		}
		if(sim->pld.zoneDef->mInstance == true)
		{
			SendInfoMessage("You cannot warp to a player inside an instance.", INFOMSG_ERROR);
			return;
		}
		int x = sim->creatureInst->CurrentX;
		int y = sim->creatureInst->CurrentY;
		int z = sim->creatureInst->CurrentZ;
		if(sim->pld.zoneDef->mID != pld.zoneDef->mID)
		{
			if(WarpToZone(sim->pld.zoneDef, false) == false)
				return;
		}
		SetPosition(x, y, z, 1);

		pld.charPtr->LastWarpTime = time;
	}
	*/
}

int SimulatorThread :: handle_query_shard_set(void)
{
	//Un-modified client only sets 1 row.  Modified client contains an additional field.
	//[0] Shard Same
	//[1] Character Name  [modded client]
	const char *shardName = NULL;
	const char *charName = NULL;
	if(query.argCount >= 1)
		shardName = query.args[0].c_str();
	if(query.argCount >= 2)
		charName = query.args[0].c_str();

	ShardSet(shardName, charName);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_forumlock(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	bool status = false;
	if(query.argCount >= 1)
		status = Util::IntToBool(atoi(query.args[0].c_str()));

	g_IGFManager.mForumLocked = status;
	Util::SafeFormat(Aux1, sizeof(Aux1), "Forum lock status: %d", status);
	SendInfoMessage(Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_dtrig(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int action = 0;
	if(query.argCount > 0)
		action = query.GetInteger(0);

	switch(action)
	{
	case 11:
		Fun::oFunReplace.Reset();
		break;
	case 15:
		if(query.argCount >= 2)
		{
			const char *name = query.GetString(1);
			for(size_t i = 0; i < creatureInst->actInst->NPCListPtr.size(); i++)
			{
				CreatureInstance *ptr = creatureInst->actInst->NPCListPtr[i];
				if(strstr(ptr->css.display_name, name) != NULL)
				{
					query.name = "warp";
					query.args.clear();
					sprintf(Aux1, "%d", ptr->CurrentX);
					sprintf(Aux2, "%d", ptr->CurrentZ);
					query.args.push_back(Aux1);
					query.args.push_back(Aux2);
					int dummy = 0;
					HandleCommand(dummy);
					break;
				}
			}
		}
		break;
	case 17:
		{
		int searchlen = 200;
		if(query.argCount > 1)
			searchlen = query.GetInteger(1);
		std::vector<SceneryObject*> search;
		g_SceneryManager.GetThread("SimulatorThread::handle_command_dtrig");
		g_SceneryManager.EnumPropsInRange(pld.CurrentZoneID, creatureInst->CurrentX, creatureInst->CurrentZ, searchlen, search);
		std::map<std::string, int> pkgmap;
		std::map<std::string, int>::iterator it;
		for(size_t i = 0; i < search.size(); i++)
		{
			if(strstr(search[i]->Asset, "SpawnPoint") == NULL)
				continue;
			if(search[i]->extraData == NULL)
				continue;
			pkgmap[search[i]->extraData->spawnPackage]++;
		}
		for(it = pkgmap.begin(); it != pkgmap.end(); ++it)
			g_Log.AddMessageFormat("%s----:%d", it->first.c_str(), it->second);
		g_SceneryManager.ReleaseThread();
		}
		break;
	case 19:
		{
			CreatureInstance *ptr = creatureInst->CurrentTarget.targ;
			if(ptr)
			{
				g_Log.AddMessageFormat("name=%s", ptr->css.display_name);
				g_Log.AddMessageFormat("aggro=%d", ptr->css.aggro_players);
				g_Log.AddMessageFormat("ai=%s", ptr->css.ai_package);
				g_Log.AddMessageFormat("faction=%d", ptr->Faction);
				g_Log.AddMessageFormat("sflags=%d", ptr->serverFlags);
				for(size_t i = 0; i <= 62; i++)
					if(ptr->HasStatus(i)) g_Log.AddMessageFormat("Status:%s", GetStatusNameByID(i));
			}
		}
		break;
	case 24:
		g_ActiveInstanceManager.DebugFlushInactiveInstances();
		break;
	case 50:
		if(query.argCount > 1)
			g_Config.AprilFools = query.GetInteger(1);
		break;
	case 51:
		if(query.argCount > 1)
			g_Config.AprilFoolsName = query.GetString(1);
		break;
	case 52:
		if(query.argCount > 1)
			g_Config.AprilFoolsAccount = query.GetInteger(1);
		break;
	case 100:
		if(creatureInst->CurrentTarget.targ != NULL)
			creatureInst->CurrentTarget.targ->ApplyRawDamage(99999);
		break;
	case 110:
		{
			CreatureInstance *target = NULL;
			int value = 0;
			int EffectID = 0;
			if(query.argCount > 1)
				target = g_ActiveInstanceManager.GetPlayerCreatureByName(query.GetString(1));
			if(query.argCount > 2)
				EffectID = GetStatusIDByName(query.GetString(2));
			if(query.argCount > 3)
				value = query.GetInteger(3);

			if(target == NULL)
			{
				SendInfoMessage("Could not find target.", INFOMSG_ERROR);
				break;
			}
			if(EffectID == -1)
			{
				SendInfoMessage("Invalid effect.", INFOMSG_ERROR);
				break;
			}
			if(target != NULL && EffectID >= 0)
			{
				if(value == 0)
					target->_RemoveStatusList(EffectID);
				else
					target->_AddStatusList(EffectID, -1);
			}
		}
		break;
	case 900:
		{
			int x1 = query.GetInteger(1) / pld.zoneDef->mPageSize;
			int y1 = query.GetInteger(2) / pld.zoneDef->mPageSize;
			int x2 = query.GetInteger(3) / pld.zoneDef->mPageSize;
			int y2 = query.GetInteger(4) / pld.zoneDef->mPageSize;
			Platform::MakeDirectory("SceneryCopy");
			for(int y = y1; y <= y2; y++)
			{
				for(int x = x1; x <= x2; x++)
				{
					sprintf(Aux1, "Scenery\\%d\\x%03dy%03d.txt", pld.zoneDef->mID, x, y);
					sprintf(Aux2, "SceneryCopy\\x%03dy%03d.txt", x, y);
					Platform::FixPaths(Aux1);
					Platform::FixPaths(Aux2);
					Platform::FileCopy(Aux1, Aux2);
				}
			}
		}
		break;
	case 901:  //Poke grove index entry
		{
			if(query.argCount <= 4)
				break;
			int zoneID = query.GetInteger(1);
			int accountID = query.GetInteger(2);
			const char *warpName = query.GetString(3);
			const char *groveName = query.GetString(4);
			bool allowCreate = false;
			if(query.argCount >= 5)
				allowCreate = (query.GetInteger(5) == InternalID);
			if(allowCreate == false)
				SendInfoMessage("To create an entry, provide Simulator ID to confirm.", INFOMSG_INFO);

			g_ZoneDefManager.UpdateZoneIndex(zoneID, accountID, warpName, groveName, allowCreate);
			SendInfoMessage("Updated.", INFOMSG_INFO);
		}
		break;
	case 902:  //Scenery import.  Not recommended for public server use, prefer to use offline first.
		{
			if(query.argCount < 3)
			{
				SendInfoMessage("/dtrig 902 <filename> <allowMerge>", INFOMSG_INFO);
				break;
			}
			g_SceneryManager.GetThread("dtrig");
			const char *fileName = query.GetString(1);
			bool allowMerge = query.GetBool(2);
			int add = 0;
			int merge = 0;
			SceneryPage page;
			page.LoadSceneryFromFile(fileName);
			SceneryPage::SCENERY_IT it;
			SceneryPage *found = NULL;
			int zoneID = pld.zoneDef->mID;
			for(it = page.mSceneryList.begin(); it != page.mSceneryList.end(); ++it)
			{
				SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(zoneID, it->second.ID, &found);
				if(so == NULL)
				{
					SceneryObject *nso = g_SceneryManager.AddProp(zoneID, it->second);
					if(nso)
						nso->SetName("IMPORT");
					add++;
				}
				else if(allowMerge == true)
				{
					so->copyFrom(&it->second);
					if(found != NULL)
						found->NotifyAccess(true);
					merge++;
				}
				else
				{
					g_Log.AddMessageFormat("Skipped: %d", it->second.ID);
				}
			}
			g_SceneryManager.ReleaseThread();
			g_SceneryManager.CheckAutosave(true);
			Util::SafeFormat(Aux1, sizeof(Aux1), "Scenery import to zone %d: %d in file, %d added, %d merged", zoneID, page.mSceneryList.size(), add, merge);
			SendInfoMessage(Aux1, INFOMSG_INFO);
			g_Log.AddMessageFormat(Aux1);
		}
		break;
	case 903:
		g_CraftManager.LoadData();
		break;
	case 904:
		g_InstanceScaleManager.LoadData();
		break;
	case 906:
		g_DropRateProfileManager.LoadData();
		break;
	case 909:
		if(query.argCount >= 3)
		{
			const char *pkg = query.GetString(1);
			const char *snd = query.GetString(2);
			SendPlaySound(pkg, snd);
		}
		break;
	case 999:
		if(query.argCount > 1)
		{
			const char *message = query.GetString(1);
			BroadcastMessage(message);
		}
		break;
	case 1000:
		{
			bool state = false;
			int range = 960;
			if(query.argCount > 1)
				state = query.GetInteger(1) != 0;
			if(query.argCount > 2)
				range = query.GetInteger(2);
			creatureInst->actInst->SetAllPlayerPVPStatus(creatureInst->CurrentX, creatureInst->CurrentZ, range, state);
		}
		break;
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

/*
void SimulatorThread :: VerifySendBufSize(int length)
{
	if(length >= sizeof(SendBuf))
		LogMessageL(MSG_CRIT, "[CRITICAL] Buffer overflow: %d / %d", length, sizeof(SendBuf));
	else if(length >= sizeof(SendBuf) - 1000)
		LogMessageL(MSG_WARN, "[STRING] Risky buffer size: %d / %d", length, sizeof(SendBuf));
}
*/
			
void SimulatorThread :: VerifyGenericBuffer(const char *buffer, uint buflen)
{
	//Note: SendBuf does not work if called from any of the instance batch send (LSendTo___)  functions
	//since it will likely use a different simulator's buffer at a different address.
	if(buffer == SendBuf)
	{
		if(buflen >= sizeof(SendBuf) - 2400)
		{
			LogMessageL(MSG_WARN, "[BUFFER] AttemptSend() dangerous data length in SendBuf, %d bytes (max: %d)", buflen, sizeof(SendBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	}
	else if(buffer == GSendBuf)
	{
		if(buflen >= sizeof(GSendBuf) - 3200)
		{
			LogMessageL(MSG_WARN, "[WARNING] AttemptSend() dangerous data length in GSendBuf, %d bytes (max: %d)", buflen, sizeof(GSendBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	}
	else if(buffer == GAuxBuf)
	{
		if(buflen >= sizeof(GAuxBuf) - 100)
		{
			LogMessageL(MSG_WARN, "[WARNING] AttemptSend() dangerous data length in GAuxBuf, %d bytes (max: %d)", buflen, sizeof(GAuxBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	}
	else if(buffer == Aux1)
	{
		if(buflen >= sizeof(Aux1) - 400)
		{
			LogMessageL(MSG_WARN, "[WARNING] AttemptSend() dangerous data length in Aux1, %d bytes (max: %d)", buflen, sizeof(Aux1));
			fprintf(stderr, "%s", LogBuffer);
		}
	}
}

bool SimulatorThread :: TargetRarityAboveNormal(void)
{
	CreatureInstance *target = creatureInst->CurrentTarget.targ;
	if(target == NULL)
		return false;

	if(!(target->serverFlags & ServerFlags::IsNPC))
		return false;
	
	if(target->css.rarity >= CreatureRarityType::HEROIC)
		return true;

	return false;
}

int SimulatorThread :: handle_query_mod_pet_list(void)
{
	/* Query: mod.pet.list
	   Args : [none]
	   Response: Send back a table with the pet data.   */

	MULTISTRING response;
	g_PetDefManager.FillQueryResponse(response);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, response);
}

int SimulatorThread :: handle_query_mod_pet_purchase(void)
{
	/* Query: mod.pet.purchase
	   Args : 1 [required] : integer of the CreatureDefID to purchase.  */
	int CDefID = 0;
	if(query.argCount > 0)
		CDefID = query.GetInteger(0);

	PetDef *petDef = g_PetDefManager.GetEntry(CDefID);
	if(petDef == NULL)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Pet not found.");
	
	if(creatureInst->css.copper < petDef->mCost)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Not enough coin.");

	if(creatureInst->css.level < petDef->mLevel)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not high enough level.");

	//Check item ID too because an improper line in the file may leave it zero.
	ItemDef *itemDef = g_ItemManager.GetPointerByID(petDef->mItemDefID);
	if(itemDef == NULL || petDef->mItemDefID == 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Item does not exist.");

	InventoryManager &im = pld.charPtr->inventory;
	int slot = im.GetFreeSlot(INV_CONTAINER);
	if(slot == -1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "No free backpack slots.");

	creatureInst->AdjustCopper(-petDef->mCost);

	int wpos = 0;
	InventorySlot *sendSlot = im.AddItem_Ex(INV_CONTAINER, petDef->mItemDefID, 1);
	if(sendSlot != NULL)
		wpos += AddItemUpdate(&SendBuf[wpos], Aux1, sendSlot);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_query_mod_pet_preview(void)
{
	/* Query: mod.pet.preview
	   Args : 1 [required] : integer of the CreatureDefID to preview.  */
	int CDefID = 0;
	if(query.argCount > 0)
		CDefID = query.GetInteger(0);

	PetDef *petDef = g_PetDefManager.GetEntry(CDefID);
	if(petDef == NULL)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "Pet not found.");

	CreatureDefinition *creatureDef = CreatureDef.GetPointerByCDef(CDefID);
	if(creatureDef == NULL)
		return PrepExt_QueryResponseString(SendBuf, query.ID, "Pet not found.");

	CreatureInstance preview;
	preview.CurrentX = creatureInst->CurrentX;
	preview.CurrentY = creatureInst->CurrentY;
	preview.CurrentZ = creatureInst->CurrentZ;
	preview.CreatureDefID = CDefID;
	preview.CreatureID = creatureInst->actInst->GetNewActorID();
	preview.css.CopyFrom(&creatureDef->css);
	int wpos = PrepExt_CreatureFullInstance(SendBuf, &preview);
	AttemptSend(SendBuf, wpos);
	
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_ping_statistics(void)
{
	if(query.argCount > 7)
	{
		pld.DebugPingClientID = query.GetInteger(0);
		pld.DebugPingClientSuccessCount = query.GetInteger(1);
		pld.DebugPingClientFailCount = query.GetInteger(2);
		pld.DebugPingClientTimeoutCount = query.GetInteger(3);
		pld.DebugPingClientLowestTime = query.GetInteger(4);
		pld.DebugPingClientHighestTime = query.GetInteger(5);
		pld.DebugPingClientTotalTime = query.GetInteger(6);
		pld.DebugPingClientReceivedCount = query.GetInteger(7);
	}
	
	//A convenient place to log statistics for now.
	LogPingStatistics(false, true);

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

void SimulatorThread :: LogPingStatistics(bool server, bool client)
{
	/* DISABLED.  Has no benefit anymore and didn't really help much in its original run either.

	const char *name = "<invalid>";
	if(creatureInst != NULL)
		name = creatureInst->css.display_name;
	if(server == true)
	{
		int average = 0;
		if(pld.DebugPingServerTotalReceived != 0)
			average = pld.DebugPingServerTotalTime / pld.DebugPingServerTotalReceived;
		LogMessageL(MSG_ERROR, "[PING] Name:|%s|Server|Pings:|%d|Lowest:|%d|Highest:|%d|Average:|%d|Sync:|%d/%d|", name, pld.DebugPingServerTotalReceived, pld.DebugPingServerLowest, pld.DebugPingServerHighest, average, pld.DebugPingServerSent, pld.DebugPingServerTotalReceived);
	}
	if(client == true)
	{
		int average = 0;
		if(pld.DebugPingClientReceivedCount != 0)
			average = pld.DebugPingClientTotalTime / pld.DebugPingClientReceivedCount;
		LogMessageL(MSG_ERROR, "[PING] Name:|%s|Client|Pings:|%d|Lowest:|%d|Highest:|%d|Average:|%d|Succ:|%d|Fail:|%d|TimeOut:|%d|",
			name,
			pld.DebugPingClientID,
			pld.DebugPingClientLowestTime,
			pld.DebugPingClientHighestTime,
			average,
			pld.DebugPingClientSuccessCount,
			pld.DebugPingClientFailCount,
			pld.DebugPingClientTimeoutCount);
	}
	*/
}

int SimulatorThread :: handle_command_set_earsize(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	CreatureInstance *creature = creatureInst->CurrentTarget.targ;
	if(creature == NULL)
		creature = creatureInst;

	float val = 0;
	if(query.argCount > 0)
		val = atof(query.args[0].c_str());
	char buf[10];
	Util::SafeFormat(buf, sizeof(buf), "%1.1f", val);
	CreatureAttributeModifier mod("es", buf);
	creature->charPtr->originalAppearance = mod.Modify(creature->charPtr->originalAppearance);
	creature->css.SetAppearance(mod.Modify(creature->css.appearance).c_str());
	creature->charPtr->pendingChanges++;
	int wpos = PrepExt_UpdateAppearance(SendBuf, creature);
	creature->actInst->LSendToLocalSimulator(SendBuf, wpos, creature->CurrentX, creature->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_set_tailsize(void)
{
	if(!CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	CreatureInstance *creature = creatureInst->CurrentTarget.targ;
	if(creature == NULL)
		creature = creatureInst;
	float val = 0;
	if(query.argCount > 0)
		val = atof(query.args[0].c_str());
	char buf[10];
	Util::SafeFormat(buf, sizeof(buf), "%1.1f", val);
	CreatureAttributeModifier mod("ts", buf);
	creature->css.SetAppearance(mod.Modify(creature->css.appearance).c_str());
	creature->charPtr->originalAppearance = mod.Modify(creature->charPtr->originalAppearance);
	creature->charPtr->pendingChanges++;
	int wpos = PrepExt_UpdateAppearance(SendBuf, creature);
	creature->actInst->LSendToLocalSimulator(SendBuf, wpos, creature->CurrentX, creature->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_sdiag(void)
{
	int val = 0;
	if(query.argCount > 0)
		val = atoi(query.args[0].c_str());
	pld.accPtr->SetPermission(Perm_Account, "selfdiag", (val != 0));
	Util::SafeFormat(Aux1, sizeof(Aux1), "Self diagnostic check is %s", ((val != 0) ? "ON":"OFF") );
	int wpos = PrepExt_SendInfoMessage(SendBuf, Aux1, INFOMSG_INFO);
	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

int SimulatorThread :: handle_command_sping(void)
{
	if(query.argCount == 0)
	{
		//Just send a report.
		int average = 0;
		if(pld.DebugPingServerTotalReceived != 0)
			average = pld.DebugPingServerTotalTime / pld.DebugPingServerTotalReceived;
		sprintf(Aux1, "Pings:%d  Lowest:%d  Highest:%d  Avg:%d  Sync:%d/%d", pld.DebugPingServerTotalReceived, pld.DebugPingServerLowest, pld.DebugPingServerHighest, average, pld.DebugPingServerSent, pld.DebugPingServerTotalReceived);
		SendInfoMessage(Aux1, INFOMSG_INFO);
	}
	else
	{
		//Set the notify time.
		int time = query.GetInteger(0);
		if(time >= 0)
		{
			pld.DebugPingServerNotifyTime = query.GetInteger(0);
			if(time == 0)
				sprintf(Aux1, "Server ping notification is now off.");
			else
				sprintf(Aux1, "Server ping notification set to %d ms.", time);
		}
		else
		{
			pld.DebugPingServerSent = 0;
			pld.DebugPingServerTotalReceived = 0;
			pld.DebugPingServerTotalTime = 0;
			pld.DebugPingServerLowest = 0;
			pld.DebugPingServerHighest = 0;
			sprintf(Aux1, "Server ping statistics cleared.");
		}
		SendInfoMessage(Aux1, INFOMSG_INFO);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: ResolveEmoteTarget(int target)
{
	//If target == 0, it must resolve to the player
	//If target == 1, it must resolve to a sidekick (search and return first one)
	//If target == ID, it must resolve to a specific sidekick ID (search and return exact one)
	//If a sidekick search fails, default to player.

	if(target == 0)
		return creatureInst->CreatureID;

	//The search needs to be zero for arbitrary sidekicks (returns the first sidekick for that player)
	if(target == 1)
		target = 0;  

	CreatureInstance *sk = creatureInst->actInst->GetMatchingSidekick(creatureInst, target);
	if(sk != NULL)
		return sk->CreatureID;

	return creatureInst->CreatureID;
}

int SimulatorThread :: handle_query_mod_emote(void)
{
	//Requires a modded client to perform this action.
	if(query.argCount < 3)
		PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	int target = query.GetInteger(0);
	const char *emoteName = query.GetString(1);
	float emoteSpeed = query.GetFloat(2);
	int loop = query.GetInteger(3);
	if(emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	target = ResolveEmoteTarget(target);

	int wpos = PrepExt_SendAdvancedEmote(SendBuf, target, emoteName, emoteSpeed, loop);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_emotecontrol(void)
{
	//Requires a modded client to perform this action.
	// [0] = target (0 = avatar, 1 = pet, else = explicit creature ID of pet);
	// [1] = event (1 = stop)
	if(query.argCount < 2)
		PrepExt_QueryResponseString(SendBuf, query.ID, "OK");

	int target = query.GetInteger(0);
	int emoteEvent = query.GetInteger(1);

	target = ResolveEmoteTarget(target);

	int wpos = PrepExt_SendEmoteControl(SendBuf, target, emoteEvent);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos, creatureInst->CurrentX, creatureInst->CurrentZ);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_getpet(void)
{
	//Return the ID of the player's sidekick.
	int result = ResolveEmoteTarget(1);  //See function for parameter details.
	if(result == creatureInst->CreatureID)
		result = 0;  //Let the client know if no sidekick was found
	sprintf(Aux1, "%d", result);
	return PrepExt_QueryResponseString(SendBuf, query.ID, Aux1);
}

int SimulatorThread :: handle_query_mod_craft(void)
{
	//Requires a modded client to perform this action.
	// Arguments: list of container/slot IDs in hexadecimal notation.

	InventoryManager &inv = pld.charPtr->inventory;

	//Resolve inputs.
	std::vector<CraftInputSlot> inputs;
	std::vector<CraftInputSlot> outputs;
	for(int i = 0; i < query.argCount; i++)
	{
		unsigned int CCSID = inv.GetCCSIDFromHexID(query.GetString(i));
		InventorySlot *slot = inv.GetItemPtrByCCSID(CCSID);
		if(slot == NULL)
			continue;

		int itemID = slot->IID;
		int stackCount = slot->GetStackCount();
		ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
		if(itemDef == NULL)
			continue;

		inputs.push_back(CraftInputSlot(CCSID, itemID, stackCount, itemDef));
	}
	if(inputs.size() == 0)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must provide some input items.");

	const CraftRecipe *recipe = g_CraftManager.GetRecipe(inputs);
	if(recipe == NULL)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s not match a known combination.", ((inputs.size() == 1) ? "That item does" : "Those items do"));
		return PrepExt_QueryResponseError(SendBuf, query.ID, Aux1);
	}

	int requiredSlots = recipe->GetOutputCount();
	if(inv.CountFreeSlots(INV_CONTAINER) < requiredSlots)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "You must have %d empty inventory %s.", requiredSlots, ((requiredSlots == 1) ? "slot" : "slots"));
		return PrepExt_QueryResponseError(SendBuf, query.ID, Aux1);
	}
	
	bool success = false;
	STRINGVECTOR outputMsg;
	if(g_CraftManager.RunRecipe(recipe, inputs, outputs) == true)
	{
		//Verify whether the items exist in the item database.
		bool verified = true;
		for(size_t i = 0; i < outputs.size(); i++)
		{
			if(g_ItemManager.GetPointerByID(outputs[i].mID) == NULL)
			{
				verified = false;
				break;
			}
		}

		if(verified == true)
		{
			//Give resulting items.
			for(size_t i = 0; i < outputs.size(); i++)
			{
				InventorySlot *newItem = inv.AddItem_Ex(INV_CONTAINER, outputs[i].mID, outputs[i].mStackCount);
				if(newItem)
				{
					ItemDef *itemDef = g_ItemManager.GetPointerByID(outputs[i].mID);
					std::string msg = "Obtained ";
					Util::StringAppendInt(msg, outputs[i].mStackCount);
					msg.append(" ");
					msg.append(itemDef->mDisplayName);
					outputMsg.push_back(msg);
					
					int wpos = AddItemUpdate(Aux1, Aux2, newItem);
					AttemptSend(Aux1, wpos);
				}
			}

			//Remove crafting components.
			for(size_t i = 0; i < inputs.size(); i++)
			{
				InventorySlot *remItem = inv.GetItemPtrByCCSID(inputs[i].mCCSID);
				if(remItem)
				{
					if(remItem->GetStackCount() == 1)
						g_ItemManager.NotifyDestroy(remItem->IID, "mod.craft");
					int wpos = RemoveItemUpdate(Aux1, Aux2, remItem);
					AttemptSend(Aux1, wpos);
				}
				inv.RemItem(inputs[i].mCCSID);
			}
			success = true;
		}
	}

	if(success == false)
		SendInfoMessage("Unable to craft.", INFOMSG_INFO);
	else
	{
		int wpos = 0;
		if(outputMsg.size() > 0)
			wpos = PrepExt_SendInfoMessage(SendBuf, "Succesful craft!", INFOMSG_INFO);
		for(size_t i = 0; i < outputMsg.size(); i++)
			wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], outputMsg[i].c_str(), INFOMSG_INFO);
		AttemptSend(SendBuf, wpos);
		g_Log.AddMessageFormat("[CRAFT] Crafted %s", recipe->GetName());
	}

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_mod_getdungeonprofiles(void)
{
	MULTISTRING response;
	g_InstanceScaleManager.EnumProfileList(response);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, response);
}

int SimulatorThread :: handle_query_mod_morestats(void)
{
	MULTISTRING response;
	StatInfo::GeneratePrettyStatTable(response, &creatureInst->css);
	STRINGLIST row;
	row.push_back("Spirit Resist Percentage");
	sprintf(Aux3, "%d%%", Combat::GetSpiritResistReduction(100, creatureInst->css.spirit));
	row.push_back(Aux3);
	response.push_back(row);

	row.clear();
	row.push_back("Psyche Resist Percentage");
	sprintf(Aux3, "%d%%", Combat::GetPsycheResistReduction(100, creatureInst->css.psyche));
	row.push_back(Aux3);
	response.push_back(row);
	return PrepExt_QueryResponseMultiString(SendBuf, query.ID, response);
}

void SimulatorThread :: SendAbilityErrorMessage(int abilityErrorCode)
{
	int priority = 0;
	const char *messageStr = NULL;

	/* TODO: Obsolete since we're using a new ability handling system with different error codes.
	switch(abilityErrorCode)
	{
	case AB_FACING: messageStr = "You must be facing your target."; break;
	case AB_RANGE: messageStr = "Target is out of range."; break;
	case AB_NOTSTATUS: messageStr = "You must not be in combat."; priority = 1; break;
	case AB_STUN: messageStr = "You are stunned."; priority = 3; break;
	case AB_HMAIN: messageStr = "You must have a main weapon equipped."; priority = 2; break;
	case AB_HMELEE: messageStr = "You must have a melee weapon equipped."; priority = 2; break;
	case AB_HBOW: messageStr = "You must have a bow equipped."; priority = 2; break;
	case AB_HWAND: messageStr = "You must have a wand equipped."; priority = 2; break;
	case AB_H2HPOLE: messageStr = "You must have a 2-hand or pole weapon equipped."; priority = 2; break;
	case AB_BEHIND: messageStr = "You must be behind your target."; priority = 2; break;
	case AB_HRANGED: messageStr = "You must have a ranged weapon equipped."; priority = 2; break;
	}
	*/
	switch(abilityErrorCode)
	{
	case Ability2::ABILITY_NOT_FOUND: messageStr = "Server error: ability does not exist."; priority = 3; break;
	case Ability2::ABILITY_BAD_EVENT: messageStr = "Server error: ability event does not exist."; priority = 3; break;
	case Ability2::ABILITY_FACING: messageStr = "You must be facing your target."; break;
	case Ability2::ABILITY_INRANGE: messageStr = "Target is out of range."; break;
	case Ability2::ABILITY_NOTSILENCED: messageStr = "You are silenced."; priority = 3; break;
	case Ability2::ABILITY_DISARM: messageStr = "You are disarmed and cannot use physical attacks."; priority = 3; break;
	case Ability2::ABILITY_STUN: messageStr = "You are stunned."; priority = 3; break;
	case Ability2::ABILITY_DAZE: messageStr = "You are dazed."; priority = 3; break;
	case Ability2::ABILITY_DEAD: messageStr = "You are dead."; priority = 3; break;
	/*
	case Ability2::ABILITY_GENERIC: messageStr = "DEBUG Generic failure."; break;
	case Ability2::ABILITY_NO_TARGET: messageStr = "DEBUG No resolved targets."; break;
	case Ability2::ABILITY_COOLDOWN: messageStr = "DEBUG Ability is on cooldown."; break;
	case Ability2::ABILITY_PENDING: messageStr = "DEBUG Ability already pending."; break;
	case Ability2::ABILITY_MIGHT: messageStr = "DEBUG Not enough might."; break;
	case Ability2::ABILITY_WILL: messageStr = "DEBUG Not enough will."; break;
	case Ability2::ABILITY_MIGHT_CHARGE: messageStr = "DEBUG Not enough might charges."; break;
	case Ability2::ABILITY_WILL_CHARGE: messageStr = "DEBUG Not enough will charges."; break;
	case Ability2::ABILITY_STATUS: messageStr = "DEBUG Status requirement not met."; break;
	*/

	case Ability2::ABILITY_HASMAINHAND: messageStr = "You must have a main hand weapon equipped."; priority = 2; break;
	case Ability2::ABILITY_HASOFFHAND: messageStr = "You must have an offhand weapon equipped."; priority = 2; break;
	case Ability2::ABILITY_HASMELEE: messageStr = "You must have a melee weapon equipped."; priority = 2; break;
	case Ability2::ABILITY_HASSHIELD: messageStr = "You must have a shield equipped."; priority = 2; break;
	case Ability2::ABILITY_HASBOW: messageStr = "You must have a bow equipped."; priority = 2; break;
	case Ability2::ABILITY_HAS2HORPOLE: messageStr = "You must have a 2-hand or pole weapon equipped."; priority = 2; break;
	case Ability2::ABILITY_HASWAND: messageStr = "You must have a wand equipped."; priority = 2; break;

	case Ability2::ABILITY_HEALTH_TOO_LOW: messageStr = "You do not have enough health."; break;
	case Ability2::ABILITY_BEHIND: messageStr = "You must be behind your target."; break;
	case Ability2::ABILITY_NEARBY_SANCTUARY: messageStr = "You are not near a sanctuary."; break;
	}

	if(messageStr == NULL)
		return;

	if(g_ServerTime < (pld.LastAbilityErrorMessageTime + 2000))
	{
		if(priority <= pld.LastAbilityErrorMessagePriority)
			return;
	}

	int size = PrepExt_SendInfoMessage(SendBuf, messageStr, INFOMSG_ERROR);
	AttemptSend(SendBuf, size);

	pld.LastAbilityErrorMessageTime = g_ServerTime;
	pld.LastAbilityErrorMessagePriority = priority;
}

int SimulatorThread :: handle_command_info(void)
{
	if(CheckPermissionSimple(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	if(query.argCount >= 1)
	{
		Aux1[0] = 0;
		if(query.args[0].compare("spawntile") == 0)
			sprintf(Aux1, "Spawn Tile: %d, %d", pld.oldSpawnX, pld.oldSpawnZ);
		if(query.args[0].compare("scenerytile") == 0)
			sprintf(Aux1, "Scenery Tile: %d, %d", creatureInst->CurrentX / pld.zoneDef->mPageSize, creatureInst->CurrentZ / pld.zoneDef->mPageSize);
		if(Aux1[0] != 0)
			SendInfoMessage(Aux1, INFOMSG_INFO);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_grovesetting(void)
{
	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	if(query.argCount >= 2)
	{
		//0 = bad command, 1 = success
		bool code = 1;
		if(query.args[0].compare("filtertype") == 0)
			pld.zoneDef->mPlayerFilterType = query.GetInteger(1);
		else if(query.args[0].compare("filteradd") == 0)
		{
			int cdef = g_AccountManager.GetCDefFromCharacterName(query.GetString(1));
			if(cdef == -1)
			{
				SendInfoMessage("Character not found.", INFOMSG_ERROR);
				code = 0;
			}
			else
				pld.zoneDef->AddPlayerFilterID(cdef);
		}
		else if(query.args[0].compare("filterremove") == 0)
		{
			int cdef = g_AccountManager.GetCDefFromCharacterName(query.GetString(1));
			if(cdef == -1)
			{
				SendInfoMessage("Character not found.", INFOMSG_ERROR);
				code = 0;
			}
			else
				pld.zoneDef->RemovePlayerFilter(cdef);
		}
		else if(query.args[0].compare("filterclear") == 0)
		{
			pld.zoneDef->ClearPlayerFilter();
		}
		else if(query.args[0].compare("filterlist") == 0)
		{
			const std::vector<int> &IDs = pld.zoneDef->mPlayerFilterID;
			for(size_t i = 0; i < IDs.size(); i++)
			{
				const char *name = g_AccountManager.GetCharacterNameFromCDef(IDs[i]);
				if(name == NULL)
					name = "<unknown character>";
				Util::SafeFormat(Aux1, sizeof(Aux1), "#%d:%s", i, name);
				SendInfoMessage(Aux1, INFOMSG_INFO);
			}
		}
		else
			SendInfoMessage("Unknown command.", INFOMSG_ERROR);

		if(code == 1)
		{
			SendInfoMessage("Action completed.", INFOMSG_INFO);
			g_ZoneDefManager.NotifyConfigurationChange();
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_grovepermission(void)
{
	//Sent either as a command or query from the permission editor panel in a modded client.
	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	if(query.argCount > 0)
	{
		pld.zoneDef->UpdateGrovePermission(query.args);
		/*
		STRINGLIST args;
		args.assign(query.args.begin() + 1, query.args.end());
		int r = pld.zoneDef->UpdateGrovePermission();
		*/
	}
	
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_dngscale(void)
{
	std::string outputMsg;
	if(query.argCount > 0)
	{
		const char *profName = query.GetString(0);
		const InstanceScaleProfile *prof = g_InstanceScaleManager.GetProfile(profName);
		if(prof == NULL)
		{
			outputMsg = "Profile does not exist: ";
			outputMsg.append(profName);
		}
		else
		{
			outputMsg = "Profile set to: ";
			outputMsg.append(profName);
			pld.charPtr->InstanceScaler = profName;
		}
	}
	else
	{
		outputMsg = "Profile cleared.";
		pld.charPtr->InstanceScaler.clear();
	}
	SendInfoMessage(outputMsg.c_str(), INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


// Convert linked scenery to use the correct path links.
int SimulatorThread :: handle_command_pathlinks(void)
{
	if(pld.zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You are not in a grove.");
	if(pld.zoneDef->mAccountID != pld.accPtr->ID)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "You must be in your grove.");

	int radius = 300;
	if(query.argCount > 0)
		radius = query.GetInteger(0);

	std::vector<SceneryObject*> results;
	g_SceneryManager.EnumPropsInRange(pld.CurrentZoneID, creatureInst->CurrentX, creatureInst->CurrentZ, radius, results);
	int propChanges = 0;
	for(size_t i = 0; i < results.size(); i++)
	{
		SceneryObject *so = results[i];
		if(so->IsSpawnPoint() == false)
			continue;
		if(so->HasLinks(SceneryObject::LINK_TYPE_LOYALTY) == false)
			continue;
		if(so->extraData == NULL)
			continue;
		int linkChanges = 0;
		for(int li = 0; li < so->extraData->linkCount; li++)
		{
			if(so->extraData->link[li].type == SceneryObject::LINK_TYPE_LOYALTY)
			{
				so->extraData->link[li].type = SceneryObject::LINK_TYPE_PATH;
				linkChanges++;
			}
		}
		if(linkChanges > 0)
		{
			propChanges++;
			int wpos = PrepExt_UpdateScenery(SendBuf, so);
			AttemptSend(SendBuf, wpos);

			//Flag the scenery for autosaves.
			g_SceneryManager.NotifyChangedProp(pld.CurrentZoneID, so->ID);
		}
	}
	Util::SafeFormat(Aux1, sizeof(Aux1), "Updated %d SpawnPoints", propChanges);
	SendInfoMessage(Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_command_partyinvite(void)
{
	//Hack to invite party members by command.  Replace the query
	//with the correct party query and arguments, then call the function
	//to process it.
	if(query.argCount < 1)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Usage: /partyinvite \"Character Name\"");

	std::string charName = query.GetString(0);
	query.name = "party";
	query.args.clear();
	query.args.push_back("invite");
	query.args.push_back(charName);
	query.argCount = query.args.size();
	
	return handle_query_party();
}

int SimulatorThread :: handle_command_roll(void)
{
	//Informs the players of a party of a simple "dice roll".
	ActiveParty *party = NULL;
	if(creatureInst->PartyID >0)
		party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
	
	if(creatureInst->PartyID == 0 || party == NULL)
	{
		int roll = randint(1, 100);
		Util::SafeFormat(Aux1, sizeof(Aux1), "You rolled %d", roll);
		SendInfoMessage(Aux1, INFOMSG_INFO);
	}
	else if(party != NULL)
	{
		std::vector<int> rolls;
		for(size_t mi = 0; mi < party->mMemberList.size(); mi++)
		{
			bool has = false;
			int tries = 0;
			int roll = 0;
			do
			{
				has = false;
				roll = randint(1, 100);
				for(size_t i = 0; i < rolls.size(); i++)
				{
					if(rolls[i] == roll)
					{
						has = true;
						tries++;
						break;
					}
				}
			} while(has == true && tries < 10);
			rolls.push_back(roll);
		}
		int wpos = 0;
		for(size_t mi = 0; mi < party->mMemberList.size(); mi++)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "%s rolled %d", party->mMemberList[mi].mDisplayName.c_str(), rolls[mi]);
			wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], Aux1, INFOMSG_INFO);
			if(wpos > Global::MAX_SEND_CHUNK_SIZE)
			{
				party->BroadCast(SendBuf, wpos);
				wpos = 0;
			}
		}
		if(wpos > 0)
			party->BroadCast(SendBuf, wpos);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

/* Called by the client ability import.  Placeholder function. */
int SimulatorThread :: handle_query_updateContent(void)
{
	if(pld.accPtr->HasPermission(Perm_Account, Permission_Admin) == false)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_instance(void)
{
	ActiveInstance *inst = creatureInst->actInst;
	if(inst != NULL)
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "Total mobs killed: %d", inst->mKillCount);
		SendInfoMessage(Aux1, INFOMSG_INFO);

		if(inst->mZoneDefPtr->IsDungeon() == true || CheckPermissionSimple(Perm_Account, Permission_Debug) == true)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "Drop rate bonus: %gx", inst->mDropRateBonusMultiplier);
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}

		if(inst->mZoneDefPtr->IsDungeon() == true)
		{
			if(inst->mOwnerPartyID > 0)
			{
				ActiveParty *party = g_PartyManager.GetPartyByID(inst->mOwnerPartyID);
				if(party != NULL)
				{
					Util::SafeFormat(Aux1, sizeof(Aux1), "Dungeon party leader: %s", party->mLeaderName.c_str());
					SendInfoMessage(Aux1, INFOMSG_INFO);
				}
			}
			else
			{
				Util::SafeFormat(Aux1, sizeof(Aux1), "Dungeon owner: %s", inst->mOwnerName.c_str());
				SendInfoMessage(Aux1, INFOMSG_INFO);
			}
		}

		if(inst->scaleProfile != NULL)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "Dungeon scaler: %s", inst->scaleProfile->mDifficultyName.c_str());
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}

		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == true && inst->dropRateProfile != NULL)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "Drop rate profile: %s", inst->dropRateProfile->mName.c_str());
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_script_gc(void)
{
	ActiveInstance *inst = creatureInst->actInst;
	if(inst != NULL)
	{
		if(inst->nutScriptPlayer != NULL)
		{
			Util::SafeFormat(Aux1, sizeof(Aux1), "Instance script collected %d objects", inst->nutScriptPlayer->GC());
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}

		ActiveInstance::CREATURE_IT it;
		for(it = inst->NPCList.begin(); it != inst->NPCList.end(); ++it)
		{
			AINutPlayer *player = it->second.aiNut;
			if(player != NULL)
			{
				Util::SafeFormat(Aux1, sizeof(Aux1), "CID: %d (%s) collected",
						it->first, it->second.css.display_name, player->GC());
				SendInfoMessage(Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_script_exec(void)
{
	bool ok = CheckPermissionSimple(Perm_Account, Permission_Admin);
	if(!ok) {
		if(pld.zoneDef->mGrove == true && pld.zoneDef->mAccountID != pld.accPtr->ID)
			ok = true;
	}
	if(!ok)
		return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");
	ActiveInstance *inst = creatureInst->actInst;
	if(inst != NULL && query.argCount > 0)
	{
		string funcName = query.GetString(0);
		if(inst->nutScriptPlayer != NULL) {
			std::vector<ScriptCore::ScriptParam> p;
			for(uint i = 1 ; i < query.argCount; i++) {
				p.push_back(ScriptCore::ScriptParam(query.GetString(i)));
			}
			inst->nutScriptPlayer->JumpToLabel(funcName.c_str(), p);
		}
	}
	else
	{
		Util::SafeFormat(Aux1, sizeof(Aux1), "No function name provided.");
		SendInfoMessage(Aux1, INFOMSG_ERROR);
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

int SimulatorThread :: handle_query_script_time(void)
{
	ActiveInstance *inst = creatureInst->actInst;
	if(inst != NULL)
	{
		double seconds;
		if(inst->nutScriptPlayer != NULL)
		{
			seconds = (double)inst->nutScriptPlayer->mProcessingTime / 1000.0;
			Util::SafeFormat(Aux1, sizeof(Aux1), "S Instance: %4.3f (%ul,%u,%ul). %s", seconds,
					inst->nutScriptPlayer->mInitTime, inst->nutScriptPlayer->mCalls, inst->nutScriptPlayer->mGCTime, inst->nutScriptPlayer->mActive ? "Active" : "Inactive");
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}
		if(inst->scriptPlayer != NULL)
		{
			seconds = (double)inst->scriptPlayer->mProcessingTime / 1000.0;
			Util::SafeFormat(Aux1, sizeof(Aux1), "T Instance: %4.3f. %s", seconds,
					inst->scriptPlayer->mActive ? "Active" : "Inactive");
			SendInfoMessage(Aux1, INFOMSG_INFO);
		}

		ActiveInstance::CREATURE_IT it;
		for(it = inst->NPCList.begin(); it != inst->NPCList.end(); ++it)
		{
			AINutPlayer *player = it->second.aiNut;
			if(player != NULL)
			{
				seconds = (double)player->mProcessingTime / 1000.0;
				Util::SafeFormat(Aux1, sizeof(Aux1), "S CID: %d (%s) %4.3f (%ul,%u,%ul)",
						it->first, it->second.css.display_name, seconds, player->mInitTime,player->mCalls, player->mGCTime);
				SendInfoMessage(Aux1, INFOMSG_INFO);
			}

			AIScriptPlayer *tPlayer = it->second.aiScript;
			if(tPlayer != NULL)
			{
				seconds = (double)tPlayer->mProcessingTime / 1000.0;
				Util::SafeFormat(Aux1, sizeof(Aux1), "T CID: %d (%s) %4.3f",
						it->first, it->second.css.display_name, seconds);
				SendInfoMessage(Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}


int SimulatorThread :: handle_query_go(void)
{
	//Sent by the client when pressing 'C' when the build tool is open.  Intended function is to
	//warp the player to the camera position.
	// [0], [1], [2] = x, y, z coordinates, respectively.

	if(pld.zoneDef->mGrove == false)
		if(CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(SendBuf, query.ID, "Permission denied.");

	int x = creatureInst->CurrentX;
	int y = creatureInst->CurrentY;
	int z = creatureInst->CurrentZ;

	if(query.argCount >= 3)
	{
		x = static_cast<int>(query.GetFloat(0));  //The arguments come in as floats but we use ints.
		y = static_cast<int>(query.GetFloat(1));
		z = static_cast<int>(query.GetFloat(2));
	}
	DoWarp(pld.CurrentZoneID, pld.CurrentInstanceID, x, y, z);
	return PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
}

bool SimulatorThread :: IsGMInvisible(void)
{
	return creatureInst->HasStatus(StatusEffects::GM_INVISIBLE);
}


void SimulatorThread :: Debug_GenerateCreatureReport(ReportBuffer &report)
{
	if(creatureInst == NULL)
	{	
		report.AddLine("Current player pointer is NULL.");
		return;
	}
	creatureInst->DebugGenerateReport(report);

	CreatureInstance *target = creatureInst->CurrentTarget.targ;
	if(target == NULL)
	{
		report.AddLine("No target selected.");
	}
	else
	{
		report.AddLine(NULL);
		report.AddLine(NULL);
		report.AddLine("TARGET INFORMATION");
		target->DebugGenerateReport(report);
	}
}


void SimulatorThread :: Debug_GenerateItemReport(ReportBuffer &report, bool simple)
{
	if(pld.charPtr == NULL)
	{
		report.AddLine("No character data.");
		return;
	}

	int rarityCount[7] = {0};
	int rarityTotal[7] = {0};

	for(int c = 0; c < MAXCONTAINER; c++)
	{
		report.AddLine("%s", GetContainerNameFromID(c));
		for(size_t i = 0; i < pld.charPtr->inventory.containerList[c].size(); i++)
		{
			int ID = pld.charPtr->inventory.containerList[c][i].IID;
			ItemDef *item = g_ItemManager.GetPointerByID(ID);
			if(item == NULL)
				continue;
			if(item->mQualityLevel >= 0 && item->mQualityLevel < 7)
			{
				rarityCount[(int)item->mQualityLevel]++;
				rarityTotal[(int)item->mQualityLevel]++;
			}
			if(simple == true)
				report.AddLine("%d : %s (lev:%d, qlev:%d)", item->mID, item->mDisplayName.c_str(), item->mLevel, item->mQualityLevel);
			else
				item->Debug_WriteReport(report);
		}
		for(size_t r = 0; r < 7; r++)
			if(rarityCount[r] > 0)
				report.AddLine("qlev:%d=%d", r, rarityCount[r]);
		memset(rarityCount, 0, sizeof(rarityCount));
		report.AddLine(NULL);
	}
	report.AddLine("Total rarity counts:");
	for(size_t r = 0; r < 7; r++)
		if(rarityTotal[r] > 0)
			report.AddLine("qlev:%d=%d", r, rarityTotal[r]);
	report.AddLine(NULL);
}

//Send an error message to the client and append a query success message.  This is required for certain
//official queries because the error text can be ugly.  "quest.complete failed: blah blah"
int SimulatorThread :: ErrorMessageAndQueryOK(char *buffer, const char *errorMessage)
{
	int wpos = 0;
	wpos += PrepExt_SendInfoMessage(&buffer[wpos], errorMessage, INFOMSG_ERROR);
	wpos += PrepExt_QueryResponseString(&buffer[wpos], query.ID, "OK");
	return wpos;
}



PartyMember * SimulatorThread :: RollForPartyLoot(ActiveParty *party, std::set<int> creatureIds, const char *rollType, int itemId)
{
	LogMessageL(MSG_SHOW, "Rolling for %d players", creatureIds.size());
	int maxRoll = 0;
	ItemDef *cdef = g_ItemManager.GetPointerByID(itemId);
	PartyMember *maxRoller;
	if(creatureIds.size() == 1) {
		return party->GetMemberByID(*creatureIds.begin());
	}
	for (std::set<int>::iterator it=creatureIds.begin(); it!=creatureIds.end(); ++it) {
		int rolled = randmodrng(1, 100);
		PartyMember *m = party->GetMemberByID(*it);
		if(rolled > maxRoll) {
			maxRoller = m;
			maxRoll = rolled;
		}
		int wpos = PartyManager::WriteLootRoll(SendBuf, cdef->mDisplayName.c_str(), (char)rolled, m->mCreaturePtr->css.display_name);
		party->BroadCast(SendBuf, wpos);

	}
	return maxRoller;
}

void SimulatorThread :: ClearLoot(ActiveParty *party, ActiveLootContainer *loot)
{
	party->RemoveTagsForLootCreatureId(loot->CreatureID, 0, 0);
	loot->RemoveAllRolls();
}

void SimulatorThread :: ResetLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId)
{
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, 0);
	loot->RemoveCreatureRolls(itemId, 0);
}

void SimulatorThread :: UndoLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId, int creatureId)
{
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, creatureId);
	loot->RemoveCreatureRolls(itemId, creatureId);

}

void SimulatorThread :: CheckIfLootReadyToDistribute(ActiveLootContainer *loot, LootTag *lootTag)
{
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
	bool needOrGreed =(  party->mLootFlags & NEED_B4_GREED ) > 0;

	// How many decisions do we expect to process the roll. This will depend
	// on if this is the secondary roll when round robin or loot master is in use
	// has been processed or not
	uint requiredDecisions = party->mMemberList.size();
	if((party->mLootMode == ROUND_ROBIN || party->mLootMode == LOOT_MASTER)) {
		if(loot->IsStage2(lootTag->mItemId)) {
			requiredDecisions = party->mMemberList.size() - 1;
		}
		else {
			requiredDecisions = 1;
		}
	}
	uint decisions = (uint)loot->CountDecisions(lootTag->mItemId);
	LogMessageL(MSG_SHOW, "Loot requires %d decisions, we have %d", requiredDecisions, decisions);

	if(decisions >= requiredDecisions)
	{
		LogMessageL(MSG_SHOW, "Loot %d is ready to distribute", lootTag->mItemId);

		CreatureInstance *receivingCreature = NULL;

		/*
		 * If the loot mode is loot master, and this roll was from the leader, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if(!loot->IsStage2(lootTag->mItemId) && party->mLootMode == LOOT_MASTER && party->mLeaderID == creatureInst->CreatureID) {
			LogMessageL(MSG_SHOW, "Got loot roll from party leader %d for %d", party->mLeaderID, lootTag->mItemId);
			if(loot->IsNeeded(lootTag->mItemId, creatureInst->CreatureID) || loot->IsGreeded(lootTag->mItemId, creatureInst->CreatureID)) {
				LogMessageL(MSG_SHOW, "Leader %d needed for %d", party->mLeaderID, lootTag->mItemId);
				receivingCreature = creatureInst;
			}
			else {
				// Offer again to the rest of the party
				LogMessageL(MSG_SHOW, "Offering %d to rest of party", lootTag->mItemId);
				loot->SetStage2(lootTag->mItemId, true);
				loot->RemoveCreatureRolls(lootTag->mItemId, lootTag->mCreatureId);
				party->RemoveCreatureTags(lootTag->mItemId, lootTag->mCreatureId);
				if(OfferLoot(-1, loot, party, creatureInst, lootTag->mItemId, needOrGreed, lootTag->mLootCreatureId, 0) == 0) {
					// Nobody to offer to, clean up as if the item had never been looted
					LogMessageL(MSG_SHOW, "Nobody to offer loot in %d to, cleaning up as if not yet looted.", lootTag->mLootCreatureId);
					ResetLoot(party, loot, lootTag->mItemId);
				}
				return;
			}
		}

		/*
		 * If the loot mode is round robin, and this roll was from them, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if(!loot->IsStage2(lootTag->mItemId) && party->mLootMode == ROUND_ROBIN && loot->robinID == creatureInst->CreatureID) {
			LogMessageL(MSG_SHOW, "Got loot roll from robin %d for %d", loot->robinID, lootTag->mItemId);
			if(loot->IsNeeded(lootTag->mItemId, creatureInst->CreatureID) || loot->IsGreeded(lootTag->mItemId, creatureInst->CreatureID)) {
				LogMessageL(MSG_SHOW, "Robin %d needed or greeded for %d", loot->robinID, lootTag->mItemId);
				receivingCreature = creatureInst;
			}
			else {
				// Offer again to the rest of the party
				LogMessageL(MSG_SHOW, "Offering %d to rest of party", lootTag->mItemId);
				loot->SetStage2(lootTag->mItemId, true);
				loot->RemoveCreatureRolls(lootTag->mItemId, lootTag->mCreatureId);
				party->RemoveCreatureTags(lootTag->mItemId, lootTag->mCreatureId);
				if(OfferLoot(-1, loot, party, creatureInst, lootTag->mItemId, needOrGreed, lootTag->mLootCreatureId, 0) == 0) {
					// Nobody to offer to, clean up as if the item had never been looted
					LogMessageL(MSG_SHOW, "Nobody to offer loot in %d to, cleaning up as if not yet looted.", lootTag->mLootCreatureId);
					ResetLoot(party, loot, lootTag->mItemId);
				}
				return;
			}
		}


		if(receivingCreature == NULL) {
			// No specific creature, first pick one of the needers if any
			set<int> needers = loot->needed[lootTag->mItemId];
			if(needers.size() > 0) {
				LogMessageL(MSG_SHOW, "Rolling for %d needers", needers.size());
				receivingCreature = RollForPartyLoot(party, needers, "Need", lootTag->mItemId)->mCreaturePtr;
			}
			else {
				set<int> greeders = loot->greeded[lootTag->mItemId];
				if(greeders.size() > 0) {
					LogMessageL(MSG_SHOW, "Rolling for %d greeders", greeders.size());
					receivingCreature = RollForPartyLoot(party, greeders, "Greed", lootTag->mItemId)->mCreaturePtr;
				}
			}
		}

		if(receivingCreature == NULL) {
			LogMessageL(MSG_WARN, "Everybody passed on loot %d", lootTag->mItemId);
			// Send a winner with a tag of '0'. This will close the window
			for(uint i = 0 ; i < party->mMemberList.size(); i++) {
				// Skip the loot master or robin

				LootTag *tag = party->GetTag(lootTag->mItemId, party->mMemberList[i].mCreaturePtr->CreatureID);
				if(tag != NULL)
				{
					Util::SafeFormat(Aux2, sizeof(Aux2), "%d:%d", tag->mCreatureId, tag->mSlotIndex);
					Util::SafeFormat(Aux3, sizeof(Aux3), "%d", 0);
					WritePos = PartyManager::WriteLootWin(SendBuf, Aux2, "0", "Nobody", lootTag->mCreatureId, 999);
				}
			}
			ResetLoot(party, loot, lootTag->mItemId);
			return;
		}

		InventorySlot *newItem = NULL;

		// Send the actual winner to all of the party that have a tag
		LootTag *winnerTag = party->GetTag(lootTag->mItemId, receivingCreature->CreatureID);
		for(uint i = 0 ; i < party->mMemberList.size(); i++)
		{
			LogMessageL(MSG_WARN, "Informing %d of the winner (%d)", party->mMemberList[i].mCreaturePtr->CreatureID, lootTag->mCreatureId);
			LootTag *tag = party->GetTag(lootTag->mItemId, party->mMemberList[i].mCreaturePtr->CreatureID);
			if(tag != NULL)
			{
				Util::SafeFormat(Aux2, sizeof(Aux2), "%d:%d", tag->mCreatureId, tag->mSlotIndex);
				Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag->lootTag);
				WritePos = PartyManager::WriteLootWin(SendBuf, Aux2, Aux3, receivingCreature->css.display_name, lootTag->mCreatureId, 999);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(SendBuf, WritePos, party->mMemberList[i].mCreaturePtr->simulatorPtr);
			}
			else
			{
				LogMessageL(MSG_WARN, "No tag for item %d for a player %d to be informed", lootTag->mItemId, party->mMemberList[i].mCreaturePtr->CreatureID);
			}
		}


		// Update the winners inventory
		CharacterData *charData = receivingCreature->charPtr;
		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if(slot == -1)
		{
			Util::SafeFormat(Aux3, sizeof(Aux3), "%s doesn't have enough space. Starting bidding again", receivingCreature->css.display_name);
			party->BroadcastInfoMessageToAllMembers(Aux3);
			LogMessageL(MSG_WARN, "Receive (%d) has no slots.", receivingCreature->CreatureID);
			ResetLoot(party, loot, lootTag->mItemId);
			return;
		}
		else
		{
			newItem = charData->inventory.AddItem_Ex(INV_CONTAINER, winnerTag->mItemId, 1);
			if(newItem == NULL)
			{
				LogMessageL(MSG_WARN, "Item to loot (%d) has disappeared.", winnerTag->mItemId);
				ResetLoot(party, loot, lootTag->mItemId);
				return;
			}
		}

		int conIndex = loot->HasItem(lootTag->mItemId);
		if(conIndex == -1)
		{
			LogMessageL(MSG_WARN, "Item to loot (%d) missing.", lootTag->mItemId);
		}
		else
		{
			// Remove the loot from the container
			loot->RemoveItem(conIndex);

			LogMessageL(MSG_WARN, "There is now %d items in loot container %d.", loot->itemList.size(), loot->CreatureID);

			if(loot->itemList.size() == 0)
			{
				ClearLoot(party, loot);

				// Loot container now empty, remove it
				CreatureInstance *lootCreature = creatureInst->actInst->GetNPCInstanceByCID(lootTag->mLootCreatureId);
				if(lootCreature != NULL)
				{
					lootCreature->activeLootID = 0;
					lootCreature->css.ClearLootSeeablePlayerIDs();
					lootCreature->css.ClearLootablePlayerIDs();
					lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
					lootCreature->css.appearance_override = LootSystem::DefaultTombstoneAppearanceOverride;
					static const short statList[3] = {STAT::APPEARANCE_OVERRIDE, STAT::LOOTABLE_PLAYER_IDS, STAT::LOOT_SEEABLE_PLAYER_IDS};
					WritePos = PrepExt_SendSpecificStats(SendBuf, lootCreature, &statList[0], 3);
					creatureInst->actInst->LSendToLocalSimulator(SendBuf, WritePos, creatureInst->CurrentX, creatureInst->CurrentZ);
				}
				creatureInst->actInst->lootsys.RemoveCreature(lootTag->mLootCreatureId);

				LogMessageL(MSG_WARN, "Loot %d is now empty (%d tags now in the party).",
						loot->CreatureID, party->lootTags.size());
			}

			if(newItem != NULL && receivingCreature != NULL)
			{
				// Send an update to the actual of the item
				WritePos = AddItemUpdate(SendBuf, Aux3, newItem);
				receivingCreature->actInst->LSendToOneSimulator(SendBuf, WritePos, receivingCreature->simulatorPtr);
			}

			// Reset the loot tags etc, we don't need them anymore
			ResetLoot(party, loot, lootTag->mItemId);
		}
	}
	else {
		LogMessageL(MSG_SHOW, "Loot %d not ready yet to distribute", lootTag->mItemId);
	}
}


