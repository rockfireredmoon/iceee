#include <ctime>
#include <cmath>
#include <fstream>
#include <string>
#include <time.h>

#include "Simulator.h"

#include "Debug.h"
#include "Util.h"
#include "Config.h"
#include "Globals.h"
#include "Cluster.h"
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
#include "Scheduler.h"
#include "ChatChannel.h"
#include "IGForum.h"
#include <algorithm>  //for std::replace
#include "ZoneObject.h"
#include "Fun.h"
#include "Crafting.h"
#include "URL.h"
#include "StringUtil.h"
#include "InstanceScale.h"
#include "Combat.h"
#include "ConfigString.h"
#include "Random.h"
#include "GM.h"
#include "Info.h"
#include "PVP.h"
#include "QuestScript.h"
#include "CreditShop.h"
//#include "restclient.h"
#include "auth/Auth.h"
#include "Daily.h"
#include "http/SiteClient.h"
#include "query/Query.h"
#include "message/MessageHandler.h"
#include <curl/curl.h>
#include "md5.hh"
#include "json/json.h"
#include "query/ClanHandlers.h"
#include "util/Log.h"

//This is the main function of the simulator thread.  A thread must be created for each port
//since the connecting function will halt until a connection is established.
PLATFORM_THREADRETURN SimulatorThreadProc(PLATFORM_THREADARGS lpParam);

const int UNREFASHION_LOWERBOUND = 8980; //These items remove a refashioned effect.
const int UNREFASHION_UPPERBOUND = 8989;

struct StatFormat {
	static const int TYPE_INT = 1;
	static const int TYPE_PINT = 2;
	static const int TYPE_STR = 3;

	int StatID;
	const char *formatStr;
	int Type;
};

void FormatStat(int statID, const char *valueStr, std::string &output) {
	int index = GetStatIndex(statID);
	if (index == -1) {
		output = "<error:undefined>";
		return;
	}

	float fvalue = 0.0F;

	static const StatFormat formatArray[] = { { STAT::MOD_MELEE_TO_CRIT,
			"+%g%% Physical Critical Chance", StatFormat::TYPE_PINT }, {
			STAT::MOD_MAGIC_TO_CRIT, "+%g%% Magic Critical Chance",
			StatFormat::TYPE_PINT }, { STAT::BASE_BLOCK, "+%g%% Block Chance",
			StatFormat::TYPE_PINT }, { STAT::BASE_PARRY, "+%g%% Parry Chance",
			StatFormat::TYPE_PINT }, { STAT::BASE_DODGE, "+%g%% Dodge Chance",
			StatFormat::TYPE_PINT }, { STAT::MOD_MOVEMENT,
			"+%g%% Movement Speed", StatFormat::TYPE_INT }, {
			STAT::EXPERIENCE_GAIN_RATE, "+%g%% Experience Gain",
			StatFormat::TYPE_INT }, { STAT::MELEE_ATTACK_SPEED,
			"+%g%% Increased Attack Speed", StatFormat::TYPE_PINT }, {
			STAT::MAGIC_ATTACK_SPEED, "+%g%% Increased Cast Rate",
			StatFormat::TYPE_PINT }, { STAT::DMG_MOD_FIRE,
			"+%g%% Fire Specialization", StatFormat::TYPE_PINT }, {
			STAT::DMG_MOD_FROST, "+%g%% Frost Specialization",
			StatFormat::TYPE_PINT }, { STAT::DMG_MOD_MYSTIC,
			"+%g%% Mystic Specialization", StatFormat::TYPE_PINT }, {
			STAT::DMG_MOD_DEATH, "+%g%% Death Specialization",
			StatFormat::TYPE_PINT }, { STAT::BASE_HEALING,
			"+%g%% Healing Specialization", StatFormat::TYPE_PINT }, {
			STAT::CASTING_SETBACK_CHANCE, "%g%% Casting Setback Chance",
			StatFormat::TYPE_PINT }, { STAT::CHANNELING_BREAK_CHANCE,
			"%g%% Channel Break Chance", StatFormat::TYPE_PINT }, {
			STAT::MOD_HEALTH_REGEN, "+%g Hitpoint Regeneration",
			StatFormat::TYPE_INT } };
	static const int formatArraySize = sizeof(formatArray)
			/ sizeof(formatArray[0]);
	for (int i = 0; i < formatArraySize; i++) {
		if (statID != formatArray[i].StatID)
			continue;
		fvalue = 0.0F;
		if (formatArray[i].Type != StatFormat::TYPE_STR)
			fvalue = static_cast<float>(atof(valueStr));
		if (formatArray[i].Type == StatFormat::TYPE_PINT)
			fvalue /= 10.0F;

		char buffer[64];
		if (formatArray[i].Type == StatFormat::TYPE_STR)
			Util::SafeFormat(buffer, sizeof(buffer), formatArray[i].formatStr,
					valueStr);
		else
			Util::SafeFormat(buffer, sizeof(buffer), formatArray[i].formatStr,
					fvalue);

		output = buffer;
		return;
	}
}

OnExitFunctionClearBuffers::OnExitFunctionClearBuffers(
		SimulatorThread *caller) {
	mCaller = caller;
}
OnExitFunctionClearBuffers::~OnExitFunctionClearBuffers() {
	if (mCaller) {
		mCaller->ClearAuxBuffers();
		mCaller = NULL;
	}
}

#ifndef WINDOWS_PLATFORM
#include <errno.h>
#include <stddef.h>
#endif

std::list<SimulatorThread> Simulator;

SimulatorManager g_SimulatorManager;

const int MapTickChange = 20;

char GAuxBuf[];
char GSendBuf[];

SimulatorThread * GetSimulatorByID(int ID) {
	//In the old system, it uses an index into the hardcoded Simulator array.
	//In the new system, iterate across the simulator list and search for the unique ID.
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->InternalID == ID)
			return &*it;
	return NULL;
}

SimulatorThread * GetSimulatorByCharacterName(const char *name) {
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->ProtocolState == 1
				&& it->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY)
			if (it->creatureInst != NULL)
				if (strcmp(it->creatureInst->css.display_name, name) == 0)
					return &*it;
	return NULL;
}

ThreadRequest::ThreadRequest() {
	status = STATUS_NONE;
}

ThreadRequest::~ThreadRequest() {
}

bool ThreadRequest::WaitForStatus(int statusID, int checkInterval,
		int maxError) {
	int errCount = 0;
	while (status != statusID) {
		errCount++;
		if (errCount == maxError)
			return false;

		PLATFORM_SLEEP(checkInterval);
	}
	return true;
}

SimulatorManager::SimulatorManager() {
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

	curl_global_init(CURL_GLOBAL_DEFAULT);

}

SimulatorManager::~SimulatorManager() {
	Free();
}

void SimulatorManager::Free(void) {
	regList.clear();
	pendingActions = 0;
}

//void SimulatorManager :: RegisterAction(SimulatorThread *simData)
void SimulatorManager::RegisterAction(ThreadRequest *reqData) {
	cs.Enter("SimulatorManager::RegisterAction");
	regList.push_back(reqData);
	pendingActions++;
	cs.Leave();
}

void SimulatorManager::UnregisterAction(ThreadRequest *reqData) {
	cs.Enter("SimulatorManager::UnregisterAction");
	//while(DeleteAction(reqData) == true);
	size_t pos = 0;
	while (pos < regList.size()) {
		if (regList[pos] == reqData)
			regList.erase(regList.begin() + pos);
		else
			pos++;
	}
	cs.Leave();
}

void SimulatorManager::RunPendingActions(void) {
	ProcessPendingDisconnects();
	ProcessPendingPacketData();
	ProcessPendingActions();
}

SimulatorThread* SimulatorManager::GetPtrByID(int simID) {
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->InternalID == simID)
			return &*it;
	return NULL;
}

void SimulatorManager::AddPendingDisconnect(SimulatorThread *callObject) {
	cs.Enter("SimulatorManager::AddPendingDisconnect");
	pendingDisconnects.push_back(callObject);
	cs.Leave();
}

void SimulatorManager::AddPendingPacketData(SimulatorThread *callObject) {
	cs.Enter("SimulatorManager::AddPendingPacketData");
	for (size_t i = 0; i < pendingPacketData.size(); i++) {
		if (pendingPacketData[i] == callObject) {
			cs.Leave();
			return;
		}
	}
	pendingPacketData.push_back(callObject);
	cs.Leave();
}

void SimulatorManager::ProcessPendingDisconnects(void) {
	if (pendingDisconnects.size() == 0)
		return;

	cs.Enter("SimulatorManager::AddPendingDisconnect");
	for (size_t i = 0; i < pendingDisconnects.size(); i++) {
		g_Logs.simulator->info("ProcessPendingDisconnects - Sim:%v",
				pendingDisconnects[i]->InternalID);
		pendingDisconnects[i]->ProcessDisconnect();
	}

	pendingDisconnects.clear();

	cs.Leave();
}

void SimulatorManager::ProcessPendingPacketData(void) {
	if (pendingPacketData.size() == 0)
		return;

	cs.Enter("SimulatorManager::ProcessPendingPackets");
	for (size_t i = 0; i < pendingPacketData.size(); i++)
		pendingPacketData[i]->HandleReceivedMessage2();

	pendingPacketData.clear();
	cs.Leave();
}

void SimulatorManager::ProcessPendingActions(void) {
	if (pendingActions == 0)
		return;

#ifdef DEBUG_TIME
	Debug::TimeTrack("ProcessPendingActions", 100);
#endif

	cs.Enter("SimulatorManager::RunPendingActions");
	debug_acquired = true;

	for (size_t i = 0; i < regList.size(); i++) {
		//if(regList[i]->threadsys.status != ThreadRequest::STATUS_WAITMAIN)
		if (regList[i]->status != ThreadRequest::STATUS_WAITMAIN)
			continue;

		//regList[i]->threadsys.status = ThreadRequest::STATUS_WAITWORK;
		regList[i]->status = ThreadRequest::STATUS_WAITWORK;
		//bool res = regList[i]->threadsys.WaitForStatus(ThreadRequest::STATUS_COMPLETE, 5, 1000);
		bool res = regList[i]->WaitForStatus(ThreadRequest::STATUS_COMPLETE, 1,
				ThreadRequest::DEFAULT_WAIT_TIME);
		if (res == false)
			g_Logs.server->warn(
					"RunPendingActions() timed out while waiting for worker thread to complete.");
	}

	regList.clear();
	pendingActions = 0;

	debug_acquired = false;
	cs.Leave();
}

// Scan for a simulator that is logged in to a particular account and
// remove if the simulator has been inactive for a certain duration.
void SimulatorManager::CheckIdleSimulatorBoot(AccountData *account) {
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->pld.accPtr != account)
			continue;
		if (it->LoadStage >= SimulatorThread::LOADSTAGE_LOADING
				&& (g_ServerTime > it->LastRecv + SIMULATOR_BOOT_INACTIVE)) {
			it->ForceErrorMessage(
					"Your connection is being kicked because there is another login on your account.",
					INFOMSG_INFO);
			g_Logs.simulator->warn(
					"[KICK] CheckIdleSimulatorBoot Forcing Simulator:%v to shut down",
					it->InternalID);
			it->Disconnect("SimulatorManager::CheckIdleSimulatorBoot");
			return;
		}
	}
}

void SimulatorManager::FlushHangingSimulators(void) {
	if (g_ServerTime < nextFlushTime)
		return;

	nextFlushTime = g_ServerTime + SIMULATOR_FLUSH_INTERVAL;

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == false)
			continue;

		if (it->ProtocolState == 0 && it->LoadStage == 0) {
			long testTime = SIMULATOR_FLUSH_INACTIVE;
			if (it->characterCreation == true)
				testTime = SIMULATOR_FLUSH_CHARCREATE;
			if (g_ServerTime > it->LastRecv + testTime) {
				it->ForceErrorMessage("Closing connection.", INFOMSG_INFO);
				g_Logs.simulator->info("Forcing Simulator:%v to shut down",
						it->InternalID);
				it->Disconnect("SimulatorManager::FlushHangingSimulators");
			}
		} else if (it->ProtocolState == 1 && it->LoadStage > 0) {
			//While we're here, might as well check for idle characters and boot them.
			if (g_ServerTime >= it->pld.NextIdleCheckTime) {
				if (it->pld.VerifyIdle() == true) {
					g_Logs.simulator->warn(
							"[BOT] Forcing Simulator:%v to shut down (detected idle)",
							it->InternalID);
					it->ForceErrorMessage(
							"You have been disconnected for inactivity.",
							INFOMSG_INFO);
					it->Disconnect("SimulatorManager::FlushHangingSimulators");
					continue;
				}
				it->pld.UpdateNextIdleCheckTime();
			}

			if (g_Config.HeartbeatAbortCount == -1)
				continue;
			if (it->PendingHeartbeatResponse < g_Config.HeartbeatAbortCount)
				continue;
			long testTime = g_Config.HeartbeatIntervalMS
					* g_Config.HeartbeatAbortCount;
			if (g_ServerTime > it->LastRecv + testTime) {
				it->ForceErrorMessage(
						"The server has not received a routine message from the client. Disconnecting.",
						INFOMSG_INFO);
				g_Logs.simulator->warn(
						"Forcing Simulator:%v to shut down (no heartbeat response)",
						it->InternalID);
				it->Disconnect("SimulatorManager::FlushHangingSimulators");
			}
		}
	}
}

void SimulatorManager::SendToAllSimulators(const char *buffer,
		unsigned int buflen, SimulatorThread *except) {
	cs.Enter("SimulatorManager::SendToAllSimulators");
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {

		if (except != NULL && except->InternalID == it->InternalID)
			continue;

		if (it->ProtocolState == 0)
			continue;

		if (it->isConnected == false)
			continue;

		if (it->pld.charPtr == NULL)
			continue;

		it->AttemptSend(buffer, buflen);
	}
	cs.Leave();
}

void SimulatorManager::BroadcastChat(int characterID, const char *display_name,
		const char *channel, const char *message) {
	char SendBuf[512];
	int wpos = PrepExt_Chat(SendBuf, 0, "System", "*SysChat", message);
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->ProtocolState == 0) {
			continue;
		}

		if (it->isConnected == false)
			continue;

		if (it->pld.charPtr == NULL)
			continue;

		it->AttemptSend(SendBuf, wpos);
	}
}

void SimulatorManager::BroadcastMessage(const char *message) {

	cs.Enter("SimulatorManager::BroadcastMessage");
	g_Logs.simulator->info("Broadcast message '%v'", message);
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->ProtocolState == 0) {
			continue;
		}

		if (it->isConnected == false)
			continue;

		if (it->pld.charPtr == NULL)
			continue;

		it->BroadcastMessage(message);
	}
	cs.Leave();
}

SimulatorThread * SimulatorManager::CreateSimulator(void) {
	SimulatorThread *simPtr = NULL;

	SimulatorThread newSim;
	Simulator.push_back(newSim);
	simPtr = &Simulator.back();
	return simPtr;
}

SimulatorQuery::SimulatorQuery() {
	Clear();
}

SimulatorQuery::~SimulatorQuery() {
	Clear();
}

void SimulatorQuery::Clear(void) {
	ID = 0;
	name.clear();
	args.clear();
	argCount = 0;
}

bool SimulatorQuery::ValidArgIndex(unsigned int argIndex) {
	if (argIndex < 0 || argIndex >= argCount) {
		g_Logs.simulator->warn("Invalid index: %v for query: %v", argIndex,
				name.c_str());
		return false;
	}
	return true;
}

const char* SimulatorQuery::GetString(unsigned int argIndex) {
	return ValidArgIndex(argIndex) ? args[argIndex].c_str() : NULL;
}

std::string SimulatorQuery::GetStringObject(uint argIndex) {
	return ValidArgIndex(argIndex) ? args[argIndex] : "";
}

int SimulatorQuery::GetInteger(unsigned int argIndex) {
	return ValidArgIndex(argIndex) ? atoi(args[argIndex].c_str()) : 0;
}

long SimulatorQuery::GetLong(unsigned int argIndex) {
	return ValidArgIndex(argIndex) ? atol(args[argIndex].c_str()) : 0;
}

float SimulatorQuery::GetFloat(unsigned int argIndex) {
	return ValidArgIndex(argIndex) ?
			static_cast<float>(atof(args[argIndex].c_str())) : 0.0F;
}

bool SimulatorQuery::GetBool(unsigned int argIndex) {
	return ValidArgIndex(argIndex) ? (atoi(args[argIndex].c_str()) != 0) : false;
}

SimulatorThread::SimulatorThread() {
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
	LastHeartbeatSend = 0;
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

SimulatorThread::~SimulatorThread() {
	//sim_cs.Free();
}

void SimulatorThread::ResetValues(bool hardReset) {
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

	if (hardReset == true) {
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

int SimulatorThread::InitThread(int instanceindex, int globalThreadID) {
	InternalIndex = instanceindex;
	GlobalThreadID = globalThreadID;

	int res = Platform_CreateThread(0, (void*) SimulatorThreadProc, this,
			&ThreadID);
	if (res == 0)
		g_Logs.simulator->error("[%v] Could not create thread.", InternalID);

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
		g_Logs.simulator->fatal("Simulator critical section is not initialized.");
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
	g_Logs.simulator->info("Thread for Sim:%v shut down.", controller->InternalID);
	PLATFORM_CLOSETHREAD(0);
	return 0;
}

void SimulatorThread::RunMainLoop(void) {
	cs.Init();
	while (isThreadActive == true) {
		BEGINTRY
		{
			//The server should always be in the ready state.
			if (Status == Status_Ready) {
				if (SocketStatus != 0) {
					g_Logs.simulator->error("[%v] Socket is invalidated.",
							InternalID);
					isThreadActive = false;
					//sc.DisconnectClient();
					Disconnect("RunMainLoop");
					Status = Status_Wait;
				} else {
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
					if (res > 0) {
						cs.Enter("RunMainLoop");
						recvData.Append(RecBuf, res);
						if (recvData.mData.size() > sizeof(RecBuf))
							g_Logs.simulator->warn(
									"[%d] Pending buffer has %d bytes",
									InternalID, recvData.mData.size());
						else if (recvData.mData.size() > 32000)
							g_Logs.simulator->error(
									"[%d] Pending buffer is accumulating %d bytes",
									InternalID, recvData.mData.size());
						else if (recvData.mData.size() > 64000) {
							g_Logs.simulator->error(
									"[%d] Pending buffer too full %d bytes",
									InternalID, recvData.mData.size());
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
					} else if (res == 0) {
						g_Logs.simulator->debug(
								"[%v] Connection closed message.", InternalID);
						Status = Status_Restart;
					} else if (res == -2) {
						g_Logs.simulator->error("[%v] Socket select error.",
								InternalID);
						Status = Status_Restart;
					} else {
						if (sc.disconnecting) {
							g_Logs.simulator->info("Simulator shutdown.");
							Status = Status_None;
						} else {
							g_Logs.simulator->error("[%v] Sim recv failed: %v",
									InternalID, sc.GetErrorMessage());
							Status = Status_Restart;
						}
					}
				}
			} else if (Status == Status_Wait) {
				//Wait for the simulator base to pass an incoming connection.
				PLATFORM_SLEEP(1000);
			} else if (Status == Status_Init) {
				//If this has been set to the Init phase, it means a connection
				//has been supplied by the simulator base.
				OnConnect();
				Status = Status_Ready;
			} else if (Status == Status_Restart) {
				Disconnect("RunMainLoop");
				Status = Status_Wait;
			} else {
				g_Logs.simulator->warn("[%v] Unknown status.", InternalID);
				Status = Status_Wait;
				PLATFORM_SLEEP(SleepDelayError);
			}

			//Keep it from burning up unnecessary CPU cycles.
			PLATFORM_SLEEP(g_ThreadSleep);

		} //End try
		  //__except((filter(GetExceptionCode(), GetExceptionInformation())))
		BEGINCATCH
		{
			g_Logs.simulator->fatal("Exception occurred in [Sim:%v]",
					InternalIndex);
			ForceErrorMessage("CRITICAL ERROR: EMERGENCY DISCONNECT",
					INFOMSG_ERROR);
			Disconnect("RunMainLoop");
		}
	}
}

//Since this function was previously called from both the Simulator and Main threads, this is now
//a global event.  When the main thread processes it, it calls ProcessDisconnect().
void SimulatorThread::Disconnect(const char *debugCaller) {
	PLATFORM_SLEEP(1);
	isThreadActive = false;
	sc.DisconnectClient();

	/*
	 //sc.DisconnectClient();
	 LogMessageL(MSG_SHOW, "Disconnecting: [%s]", debugCaller);
	 g_SimulatorManager.AddPendingDisconnect(this);
	 */

}
int SimulatorThread::ItemMorph(bool command) {
	g_Logs.simulator->debug("[%v] protected_helper_query_item_morph: %v",
			InternalID, command);
	if (query.argCount < 3)
		return QueryErrorMsg::GENERIC;

	unsigned long origLook = strtol(query.args[0].c_str(), NULL, 16);
	unsigned long newLook = strtol(query.args[1].c_str(), NULL, 16);
	int creatureID = strtol(query.args[2].c_str(), NULL, 10);

	InventorySlot * reagentPtr = NULL;
	ItemDef * reagentDef = NULL;

	// Run a distance check for a normal query->  The /refashion command
	// also uses this function but with a spoofed query->

	bool free = false;
	bool unstack = false;
	if (command == false) {
		/* If the refashioner is the player themselves, then make sure they have
		 * a refashioning device
		 */
		if (creatureID == creatureInst->CreatureID) {
			free = true;
			reagentPtr = pld.charPtr->inventory.GetBestSpecialItem(
					GetContainerIDFromName("inv"), PORTABLE_REFASHIONER);
			if (reagentPtr == NULL) {
				return QueryErrorMsg::NOREFASHION;
			} else {
				reagentDef = g_ItemManager.GetPointerByID(reagentPtr->IID);
				if (reagentDef == NULL) {
					return QueryErrorMsg::NOREFASHION;
				}

				/* Ok to refashion. If item is stackable, then the stack is decreased too (one-off refashion items) */
				if (reagentPtr->ResolveItemPtr()->mIvMax1 > 1
						&& reagentPtr->GetStackCount() > 0) {
					unstack = true;
				}
			}
		} else {
			// Make sure this object isn't too far away.
			int distCheck = CheckDistance(creatureID);
			if (distCheck != 0)
				return distCheck;
		}
	}

	InventorySlot * origPtr = pld.charPtr->inventory.GetItemPtrByCCSID(
			origLook);
	InventorySlot * newPtr = pld.charPtr->inventory.GetItemPtrByCCSID(newLook);
	if (origPtr == NULL || newPtr == NULL)
		return QueryErrorMsg::INVMISSING;

	ItemDef *itemPtr1 = g_ItemManager.GetPointerByID(origPtr->IID);
	ItemDef *itemPtr2 = g_ItemManager.GetPointerByID(newPtr->IID);
	if (itemPtr1 == NULL || itemPtr2 == NULL)
		return QueryErrorMsg::INVALIDITEM;

	int cost = free ? 0 : (itemPtr1->mValue + itemPtr2->mValue) / 2;
	if (creatureInst->css.copper < cost)
		return QueryErrorMsg::COIN;
	creatureInst->css.copper -= cost;

	//Log just before the actual refashion so we can print the look ID before it's modified.
	g_Logs.event->info("[REFASHION] %v : CCSID Orig:%v (%v), Look:%v (%v)",
			creatureInst->css.display_name, origLook, origPtr->IID, newLook,
			newPtr->IID);
	g_Logs.event->info(
			"[REFASHION] %v : Look %v (%v, %v) applied to %v (%v, %v)",
			creatureInst->css.display_name, itemPtr1->mDisplayName.c_str(),
			itemPtr1->mID, origPtr->GetLookID(), itemPtr2->mDisplayName.c_str(),
			itemPtr2->mID, newPtr->GetLookID());

	origPtr->customLook = newPtr->GetLookID();
	if (origPtr->customLook >= UNREFASHION_LOWERBOUND
			&& origPtr->customLook <= UNREFASHION_UPPERBOUND)
		origPtr->customLook = 0;

	int wpos = 0;

	static const short statSend = STAT::COPPER;
	wpos += PrepExt_SendSpecificStats(&SendBuf[wpos], creatureInst, &statSend,
			1);

	wpos += RemoveItemUpdate(&SendBuf[wpos], Aux3, newPtr);
	pld.charPtr->inventory.RemItem(newLook);
	pld.charPtr->pendingChanges++;
	if (reagentDef != NULL) {
		Util::SafeFormat(Aux3, sizeof(Aux3), "You have used 1 %s.",
				reagentDef->mDisplayName.c_str());
		wpos += PrepExt_SendInfoMessage(&SendBuf[wpos], Aux3, INFOMSG_INFO);
		/* This refashion is taking up a reagent of sorts, a portable refashioner */
		if (unstack)
			wpos += pld.charPtr->inventory.RemoveItemsAndUpdate(INV_CONTAINER,
					reagentDef->mID, 1, &SendBuf[wpos]);
	}

	wpos += AddItemUpdate(&SendBuf[wpos], Aux3, origPtr);

	wpos += PrepExt_QueryResponseString(&SendBuf[wpos], query.ID, "OK");
	return wpos;
}

void SimulatorThread::AddPendingDisconnect(void) {
	g_SimulatorManager.AddPendingDisconnect(this);
}

void SimulatorThread::ProcessDisconnect(void) {
	if (sim_cs.GetLockCount() > 0)
		g_Logs.simulator->debug("[%v] Disconnect() LockCount is %v", InternalID,
				sim_cs.GetLockCount());

	//sim_cs.Enter();
	g_Logs.simulator->info("[%v] Disconnecting client", InternalID);

	sc.ShutdownServer();
	ProcessDetach();
}

void SimulatorThread::ProcessDetach(void) {

	if (pld.charPtr != NULL) {
		if (LoadStage == LOADSTAGE_GAMEPLAY) {
			g_PartyManager.CheckMemberLogin(creatureInst);
			if (CheckPermissionSimple(Perm_Account, Permission_Invisible)
					== false) {

				sprintf(Aux1, "%s has disconnected.",
						pld.charPtr->cdef.css.display_name);

				ChatMessage msg(&pld);
				msg.mSimulatorID = InternalID;
				msg.mMessage = Aux1;
				g_ChatManager.LogChatMessage(msg);

				WritePos = PrepExt_SendInfoMessage(SendBuf, Aux1, INFOMSG_INFO);
				SendToAllSimulator(SendBuf, WritePos, InternalID);

				WritePos = PrepExt_FriendsLogStatus(SendBuf, pld.charPtr, 0);
				SendToFriendSimulator(SendBuf, WritePos,
						pld.charPtr->cdef.CreatureDefID);
			}

			WritePos = PrepExt_RemoveCreature(SendBuf, pld.CreatureID);
			creatureInst->actInst->LSendToAllSimulator(SendBuf, WritePos,
					InternalID);
		}
		UpdateSocialEntry(false, false); //Place here since the character may not be fully logged in if they quit.
		SetLoadingStatus(false, true);

		SaveCharacterStats();
		SetAccountCharacterCache();
		pld.charPtr->SetExpireTime();
		pld.charPtr->pendingChanges = 1;
		g_CharacterManager.SaveCharacter(pld.CreatureDefID, true);
		g_ChatChannelManager.LeaveChannel(InternalID, NULL); //Remove the simulator from any chat channels so they don't pollute the active member list.

		pld.charPtr = NULL; //Can't believe I forgot this for so long... Disconnect was often called twice, too.
	}
	g_ClusterManager.LeftShard(pld.CreatureDefID);
	g_PartyManager.RemovePlayerReferences(pld.CreatureDefID, true);
	MainCallHelperInstanceUnregister();

	if (pld.accPtr != NULL) {
		pld.accPtr->AdjustSessionLoginCount(-1);
		pld.accPtr = NULL;
		//g_AccountManager.UnloadAccount(pld.accPtr->ID);
	}

	if (creatureInst->actInst != NULL)
		g_Logs.simulator->error(
				"[%d] Disconnect() actInst still registered when resetting values",
				InternalID);

	//Soft reset, don't adjust threading state or total bandwidth counts.
	//ResetValues(false);

	g_Logs.simulator->info("[%v] Finished disconnecting.", InternalID);
	//sim_cs.Leave();

	if (procData.mData.size() > 0) {
		g_Logs.simulator->warn(
				"[%d] Disconnect Clearing pending data: %d (receive: %d)",
				InternalID, procData.mData.size(), recvData.mData.size());
		procData.Clear();
	}

	isConnected = false;

	//Allow the thread's main loop to terminate and shut down the thread.
	isThreadActive = false;
}

void SimulatorThread::ForceErrorMessage(const char *message, int msgtype) {
	int size = 0;
	if (g_Config.UseMessageBox == false) {
		size = PrepExt_SendInfoMessage(SendBuf, message, msgtype);
	} else {
		size += PutByte(&SendBuf[size], 100); //_handleModMessage   REQUIRES MODDED CLIENT (AddOns.nut)
		size += PutShort(&SendBuf[size], 0);    //Reserve for size

		size += PutByte(&SendBuf[size], MODMESSAGE_EVENT_POPUP_MSG); //event for advanced emote
		size += PutStringUTF(&SendBuf[size], message);

		PutShort(&SendBuf[1], size - 3);       //Set message size
	}
	g_PacketManager.GetThread("SimulatorThread::ForceErrorMessage");
	sc.AttemptSendNoBlock(SendBuf, size);
	g_PacketManager.ReleaseThread();
}

int SimulatorThread::AttemptSend(const char *buffer, unsigned int buflen) {
#ifdef DEBUG_TIME
	Debug::TimeTrack("SimulatorThread::AttemptSend");
#endif

	if (buflen >= sizeof(SendBuf))
		g_Logs.simulator->error("[%v] AttemptSend() bytecount: %v", InternalID,
				buflen);
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

void SimulatorThread::OnConnect(void) {
	sockaddr remotename;
	memset(&remotename, 0, sizeof(remotename));
	socklen_t remotesize = sizeof(remotename);
	getpeername(sc.ClientSocket, &remotename, &remotesize);

	Util::SafeFormat(clientPeerName, sizeof(clientPeerName), "%d.%d.%d.%d",
			(unsigned char) remotename.sa_data[2],
			(unsigned char) remotename.sa_data[3],
			(unsigned char) remotename.sa_data[4],
			(unsigned char) remotename.sa_data[5]);
	int dpos = 0;
	int cpos = 0;
	for (unsigned int i = 0; i < sizeof(remotename.sa_data); i++) {
		dpos += sprintf(&SendBuf[dpos], "%d.",
				(unsigned char) remotename.sa_data[i]);
		int c = remotename.sa_data[i];
		if (c <= ' ')
			c = 'x';
		cpos += sprintf(&Aux1[cpos], "%c.", c);
	}
	g_Logs.simulator->info("[%v] Connect num [%v]", InternalID, SendBuf);
	g_Logs.simulator->info("[%v] Char [%v]", InternalID, Aux1);
	g_Logs.simulator->info("[%v] Connecting Socket:%v", InternalID,
			sc.ClientSocket);

	//int len = HexToByte(g_SimulatorOnConnect, SendBuf, sizeof(SendBuf));
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 40);  //_handleAuthenticationRequestMsg
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutInteger(&SendBuf[wpos], g_ProtocolVersion);
	wpos += PutInteger(&SendBuf[wpos], g_AuthMode);
	wpos += PutStringUTF(&SendBuf[wpos],
			g_AuthMode == AuthMethod::SERVICE ?
					g_Config.ServiceAuthURL.c_str() : g_AuthKey);
	PutShort(&SendBuf[1], wpos - 3);

	AttemptSend(SendBuf, wpos);
	isConnected = true;

	//Update the last communication to the connection time, for good measure
	LastRecv = g_ServerTime;
}

// Return true if the client is connected and capable of receiving gameplay-related messages.
bool SimulatorThread::CheckStateGameplayProtocol(void) const {
	if (isConnected == false)
		return false;
	if (ProtocolState == 0)
		return false;
	return true;
}

// Return true if the client is connected and capable of receiving gameplay-related messages.
// Also check to see if the player is likely capable of interacting with the game
// (client is fully loaded).
bool SimulatorThread::CheckStateGameplayActive(void) const {
	if (isConnected == false)
		return false;
	if (ProtocolState == 0)
		return false;
	if (LoadStage != LOADSTAGE_GAMEPLAY)
		return false;
	return true;
}

void SimulatorThread::HandleReceivedMessage2(void) {
	Debug::LastSimulatorID = InternalID;

	//Grab the data from the simulator's pending buffer
	cs.Enter("HandleReceivedMessage2");
	procData.Append(recvData);
	recvData.Clear();
	cs.Leave();

	if (isConnected == false) {
		g_Logs.simulator->fatal("Should not be processing data (size: %v)",
				procData.mData.size());
		return;
		procData.Clear();
	}

	if (procData.mData.size() == 0) {
		g_Logs.simulator->warn(
				"[%v] HandleReceivedMessage2 procData size is zero",
				InternalID);
		return;
	}

	LastRecv = g_ServerTime;
	PendingHeartbeatResponse = 0;
	TotalMessageReceived++;

	int remain = procData.mData.size();
	if (remain < 3) {
		g_Logs.simulator->error(
				"[%d] HandleReceivedMessage2() invalid message size: %d",
				InternalID, remain);
		Disconnect("SimulatorThread::HandleReceivedMessage2");
		return;
	}

	ReadPos = 0;
	//int debugCount = 0;
	readPtr = procData.mData.data();
	while (remain > 0) {
#ifdef DEBUG_PROFILER
		unsigned long debugTimeStart = g_ServerTime;
		Debug::TimeTrack("HandleReceivedMessage2", 100);
#endif
		int startRead = ReadPos;
		unsigned char msgType = GetByte(&readPtr[ReadPos], ReadPos);
		unsigned short msgSize = GetShort(&readPtr[ReadPos], ReadPos);
		if (firstConnect == true) {
			if (msgType != 1) {
				g_Logs.simulator->error(
						"[%v] HandleReceivedMessage() Invalid first request, disconnecting (type: %v, size: %v)",
						InternalID, msgType, msgSize);
//				Disconnect("SimulatorThread::HandleReceivedMessage2");
//				return;
			}
			firstConnect = false;
		}
		if (msgSize > remain - 3) {
			g_Logs.simulator->error(
					"[%v] HandleReceivedMessage() expected more data, has: %v, needs: %v",
					InternalID, remain - 3, msgSize);
			break;
		}

		if (ProtocolState == PROTOCOLSTATE_GAME)
			HandleGameMsg(msgType);
		else
			HandleLobbyMsg(msgType);

		if (PendingSend == true) {
			if (WritePos != 0) {
				AttemptSend(SendBuf, WritePos);
				WritePos = 0;
			} else
				g_Logs.simulator->warn(
						"[%v] PendingSend marked for zero bytes.", InternalID);

			PendingSend = false;
		}
		remain -= (msgSize + 3);

		//Safeguard in case not all bytes are handled.
		ReadPos = startRead + msgSize + 3;
		//LogMessageL(MSG_SHOW, "Message: %d, type: %d, size: %d, remain: %d", debugCount, msgType, msgSize, remain);
		//debugCount++;

#ifdef DEBUG_PROFILER
		unsigned long debugTimePassed = g_ServerTime - debugTimeStart;
		if (ProtocolState == PROTOCOLSTATE_GAME)
			_DebugProfiler.IncrementGameMsg(msgType, debugTimePassed);
		else
			_DebugProfiler.IncrementLobbyMsg(msgType, debugTimePassed);
#endif
	}
	//if(debugCount > 1)
	//	LogMessageL(MSG_SHOW, "[DEBUG] DEBUG COUNT: %d", debugCount);

	if (remain == 0)
		procData.Clear();
	else {
		int offset = procData.mData.size() - remain;
		procData.TrimFront(offset);
	}
}

void SimulatorThread::HandleGameMsg(int msgType) {
#ifdef DEBUG_TIME
	Debug::TimeTrack("HandleGameMsg", 50);
#endif
	//LogMessageL(MSG_SHOW, "[DEBUG] HandleGameMsg:%d", msgType);
	MessageHandler *mh = g_MessageManager.getMessageHandler(msgType);
	if (mh == NULL) {
		g_Logs.simulator->error("[%v] Unhandled message in Game mode: %v",
				InternalID, msgType);
	} else {
		int PendingData = mh->handleMessage(this, &pld, &query, creatureInst);
		if (PendingData > 0) {
			AttemptSend(SendBuf, PendingData);
			PendingSend = false;
		}
	}
}

void SimulatorThread::HandleLobbyMsg(int msgType) {

	MessageHandler *mh = g_MessageManager.getLobbyMessageHandler(msgType);
	if (mh == NULL) {
		g_Logs.simulator->error("[%v] Unhandled message in Lobby mode: %v",
				InternalID, msgType);
	} else {
		int PendingData = mh->handleMessage(this, &pld, &query, creatureInst);
		if (PendingData > 0) {
			AttemptSend(SendBuf, PendingData);
			PendingSend = false;
		}
	}
}

void SimulatorThread::ClearAuxBuffers(void) {
	memset(Aux1, 0, sizeof(Aux1));
	memset(Aux2, 0, sizeof(Aux2));
	memset(Aux3, 0, sizeof(Aux3));
}

void SimulatorThread::BroadcastMessage(const char *message) {
	char buf[256];
	g_Logs.simulator->info("[%v] Broadcast message '%v'", InternalID, message);
	AttemptSend(buf, PrepExt_Broadcast(buf, message));
}

void SimulatorThread::SendInfoMessage(const char *message, char eventID) {
	AttemptSend(SendBuf, PrepExt_Info(SendBuf, message, eventID));
}

void SimulatorThread::JoinGuild(GuildDefinition *gDef, int startValour) {
	g_CharacterManager.GetThread("CharacterData::JoinGuild");
	pld.charPtr->JoinGuild(gDef->guildDefinitionID);
	pld.charPtr->AddValour(gDef->guildDefinitionID, startValour);
	pld.charPtr->cdef.css.SetSubName(gDef->defName.c_str());
	g_CharacterManager.ReleaseThread();
	creatureInst->SendStatUpdate(STAT::SUB_NAME);
	AddMessage((long) &pld.charPtr->cdef, 0, BCM_UpdateCreatureDef);
	pld.charPtr->pendingChanges++;
}

void SimulatorThread::SendPlaySound(const char *assetPackage,
		const char *soundFile) {
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 100);       //_handleInfoMsg
	wpos += PutShort(&SendBuf[wpos], 0);      //Placeholder for size

	wpos += PutByte(&SendBuf[wpos], 4); //Event
	wpos += PutStringUTF(&SendBuf[wpos], assetPackage);
	wpos += PutStringUTF(&SendBuf[wpos], soundFile);

	PutShort(&SendBuf[1], wpos - 3);     //Set size

	AttemptSend(SendBuf, wpos);
}

void SimulatorThread::LoadAccountCharacters(AccountData *accPtr) {
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

void SimulatorThread::ReadQueryFromMessage(void) {
	if (query.ID != 0)
		query.Clear();

	query.ID = GetInteger(&readPtr[ReadPos], ReadPos);

	GetStringUTF(&readPtr[ReadPos], Aux1, sizeof(Aux1), ReadPos);
	query.name = Aux1;
	//LogMessageL(MSG_DIAGV, "Query [%d]=[%s]", query.ID, Aux1);

	query.argCount = GetByte(&readPtr[ReadPos], ReadPos);
	for (unsigned int i = 0; i < query.argCount; i++) {
		GetStringUTF(&readPtr[ReadPos], Aux1, sizeof(Aux1), ReadPos);
		query.args.push_back(Aux1);
		//LogMessageL(MSG_SHOW, "  [%d]=%s", i, Aux1);
	}

	//Should never happen, but ensure that argCount is a safe to use
	//for traversing the args list.
	if (query.argCount != query.args.size()) {
		g_Logs.simulator->error("[%v] Query argCount mismatch.", InternalID);
		query.argCount = query.args.size();
	}
}

void SimulatorThread::SetPersona(int personaIndex) {
	if (pld.accPtr == NULL) {
		g_Logs.simulator->error("[%v] SetPersona() accPtr is NULL", InternalID);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}
	if (personaIndex < 0 || personaIndex >= AccountData::MAX_CHARACTER_SLOTS) {
		g_Logs.simulator->error("[%v] Invalid persona index: %v", InternalID,
				personaIndex);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}

	int CDefID = pld.accPtr->CharacterSet[personaIndex];
	if (CDefID == 0) {
		g_Logs.simulator->error("[%v] Character index is not valid: %v:%v",
				InternalID, pld.accPtr->Name, personaIndex);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}
	g_Logs.simulator->trace("[%v] Setting character: %v:%v", InternalID,
			pld.accPtr->Name, personaIndex);

	//Check to make sure the persona isn't already in the active list
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.CreatureDefID == CDefID) {
				ForceErrorMessage("That character is already logged in.",
						INFOMSG_INFO);
				Disconnect("SimulatorThread::SetPersona");
				return;
			}
		}
	}
	ShardPlayer sp = g_ClusterManager.GetActivePlayer(CDefID);
	if (sp.mID != 0 && sp.mShard.compare(g_ClusterManager.mShardName) != 0) {
		ForceErrorMessage(
				StringUtil::Format(
						"That character is already logged in on shard %s.",
						sp.mShard.c_str()).c_str(), INFOMSG_INFO);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}

	g_CharacterManager.GetThread("SimulatorThread::SetPersona");
	pld.charPtr = g_CharacterManager.RequestCharacter(CDefID, false);
	g_CharacterManager.ReleaseThread();

	if (pld.charPtr == NULL) {
		ForceErrorMessage("ERROR: Character could not be loaded.",
				INFOMSG_ERROR);
		g_Logs.simulator->error("[%v] Character [%v] could not be loaded.",
				InternalID, CDefID);
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

	if (g_Config.AccountCredits) {
		creatureInst->css.credits = pld.accPtr->Credits;
	}

	creatureInst->Rotation = pld.charPtr->activeData.CurRotation;
	creatureInst->Heading = pld.charPtr->activeData.CurRotation;

	ChangeProtocol(1);

	bool zoneSet = false;
	int InstanceID = 0;
	int ZoneID = pld.charPtr->activeData.CurZone;
	if(ZoneID > 0) {
		/* Try current zone */
		zoneSet= ProtectedSetZone(ZoneID, InstanceID);
	}

	if(!zoneSet && ZoneID != pld.charPtr->bindReturnPoint[3] && pld.charPtr->bindReturnPoint[3] > 0) {
		/* Try bind return point */
		ZoneID = pld.charPtr->bindReturnPoint[3];
		pld.charPtr->activeData.CurZone = ZoneID;
		pld.charPtr->activeData.CurX = pld.charPtr->bindReturnPoint[0];
		pld.charPtr->activeData.CurY = pld.charPtr->bindReturnPoint[1];
		pld.charPtr->activeData.CurZ = pld.charPtr->bindReturnPoint[2];
		pld.charPtr->activeData.CurRotation = g_InfoManager.GetStartRotation();
		zoneSet = ProtectedSetZone(ZoneID, InstanceID);
	}

	if(!zoneSet && ZoneID != g_InfoManager.GetStartZone()) {
		/* Try default start zone */
		ZoneID = g_InfoManager.GetStartZone();
		pld.charPtr->activeData.CurZone = ZoneID;
		pld.charPtr->activeData.CurX = g_InfoManager.GetStartX();
		pld.charPtr->activeData.CurY = g_InfoManager.GetStartY();
		pld.charPtr->activeData.CurZ = g_InfoManager.GetStartZ();
		pld.charPtr->activeData.CurRotation = g_InfoManager.GetStartRotation();
		zoneSet = ProtectedSetZone(ZoneID, InstanceID);
	}

	if(!zoneSet) {
		ForceErrorMessage("SetPersona: critical error setting zone.",
				INFOMSG_ERROR);
		g_Logs.server->error("SetPersona: critical error setting zone: %v\n",
				ZoneID);
		Disconnect("SimulatorThread::SetPersona");
		return;
	}

	//If an instance, reset players back to the start if nothing has been killed.
	//Prevents an exploit with boss camping and relogging into spawned instances.
	if (pld.zoneDef->IsDungeon() == true
			&& creatureInst->actInst->mKillCount == 0)
		SetPosition(pld.zoneDef->DefX, pld.zoneDef->DefY, pld.zoneDef->DefZ, 1);
	else
		SetPosition(pld.charPtr->activeData.CurX, pld.charPtr->activeData.CurY,
				pld.charPtr->activeData.CurZ, 1);

	creatureInst->actInst->FetchNearbyCreatures(this, creatureInst);

	SendSetAvatar(creatureInst->CreatureID);

	//The zone change updates the map, but need to resend again otherwise the
	//terrain won't load when you first log in.
	UpdateSocialEntry(true, false);
	UpdateSocialEntry(true, true);

	// TODO is this needed? it happens in ProtectedSetZone>MainCallSetZone too
	// BroadcastShardChanged();

	//Since the character has been loaded into an instance and has acquired
	//a character instance, run processing for abilities.
	creatureInst->ActivateSavedAbilities();
	ActivatePassiveAbilities();

	UpdateEqAppearance();
	// TODO is this needed? twice?
	//UpdateEqAppearance();

	g_Logs.simulator->debug(
			"[%v] Persona set to index:%v, (ID: %v, CDef: %v) (%v)", InternalID,
			personaIndex, pld.CreatureID, pld.CreatureDefID,
			pld.charPtr->cdef.css.display_name);

	//Hack to reset an empty quickbar preference.
	std::string value = pld.charPtr->preferenceList.GetPrefValue("quickbar.0");
	bool reset = false;
	if (value.size() < 3)
		reset = true;

	if (reset == true) {
		g_Logs.simulator->warn("[%v] Have to reset [quickbar.0]=[%v]",
				InternalID, value);
		CharacterData *defChar = g_CharacterManager.GetDefaultCharacter();
		std::string newValue = defChar->preferenceList.GetPrefValue("quickbar.0");
		if (newValue.size() > 0)
			pld.charPtr->preferenceList.SetPref("quickbar.0", newValue);
	}

	pld.UpdateNextIdleCheckTime();

	g_Logs.event->info("[LOGIN] %v has logged in [%v]",
			creatureInst->css.display_name, clientPeerName);
}

/*
 bool SimulatorThread :: RegisterMainCall3(const char *debugFunction)
 {
 return true;
 //OBSOLETE BELOW

 //Set the status before registering.
 if(threadsys.status == ThreadRequest::STATUS_WAITWORK)
 {
 g_Logs.simulator->warn("RegisterMainCall3 already waiting");
 return true;
 }

 threadsys.status = ThreadRequest::STATUS_WAITMAIN;
 g_SimulatorManager.RegisterAction(&threadsys);
 bool res = threadsys.WaitForStatus(ThreadRequest::STATUS_WAITWORK, 1, ThreadRequest::DEFAULT_WAIT_TIME);
 if(res == false)
 {
 g_Logs.simulator->warn("RegisterMainCall3 timed out, operation [%v]", debugFunction);
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

bool SimulatorThread::MainCallSetZone(int newZoneID, int newInstanceID,
		bool setDefaultLocation) {
	//int newZoneID = MainCallData.param.GetLong();
	g_Logs.simulator->info("[%v] Attempting to set zone: %v", InternalID,
			newZoneID);

	if (newZoneID == 0) {
		return false;
	}

	if (pld.zoneDef != NULL) {
		//Don't do anything if already in the required zone.
		if (newZoneID == pld.zoneDef->mID && newInstanceID == 0)
			return true;

		MainCallHelperInstanceUnregister();
	}

	//The party needs to be checked before placing into a zone.
	if (LoadStage == LOADSTAGE_UNLOADED || LoadStage == LOADSTAGE_GAMEPLAY) {
		ActiveParty *party = g_PartyManager.GetPartyWithMember(
				pld.CreatureDefID);
		if (party != NULL) {
			defcInst.PartyID = party->mPartyID;
			g_Logs.simulator->debug("[%v] Setting party: %v", InternalID,
					party->mPartyID);
		}
	}

	MainCallHelperInstanceRegister(newZoneID, newInstanceID);
	if (ValidPointers() == false) {
		SendInfoMessage("Critical error changing zones", INFOMSG_ERROR);
		Disconnect("SimulatorThread::MainCallSetZone");
		return false;
	}

	pld.MovementBlockTime = g_ServerTime + g_Config.WarpMovementBlockTime;
	if (setDefaultLocation == true)
		SetPosition(pld.zoneDef->DefX, pld.zoneDef->DefY, pld.zoneDef->DefZ, 1);

	//If a player joins another instance, it will be given a new ID.
	//The client needs to be informed of the change, otherwise it won't
	//accept proper updates for the player's creature ID.
	SendSetAvatar(creatureInst->CreatureID);

	pld.CurrentZoneID = pld.zoneDef->mID;
	pld.CurrentInstanceID = creatureInst->actInst->mInstanceID;

	Util::SafeFormat(pld.CurrentZone, sizeof(pld.CurrentZone), "[%d-%d-0]",
			pld.CurrentInstanceID, pld.CurrentZoneID);
	pld.LastMapTick = MapTickChange;

	int wpos = PrepExt_SendEnvironmentUpdateMsg(SendBuf, creatureInst->actInst,
			pld.CurrentZone, pld.zoneDef, creatureInst->CurrentX,
			creatureInst->CurrentZ, 0);
	wpos += PrepExt_SetTimeOfDay(&SendBuf[wpos], GetTimeOfDay().c_str());
	AttemptSend(SendBuf, wpos);

	CheckSpawnTileUpdate(true);

	CheckMapUpdate(true);

	g_ClusterManager.JoinedShard(InternalID, pld.zoneDef->mID, pld.charPtr);
	BroadcastShardChanged();  //Let friends know we changed shards.

	int r = pld.charPtr->questJournal.CheckTravelLocations(
			creatureInst->CreatureID, Aux1, creatureInst->CurrentX,
			creatureInst->CurrentY, creatureInst->CurrentZ, pld.CurrentZoneID);
	if (r > 0)
		AttemptSend(Aux1, r);

	if (CheckPermissionSimple(Perm_Account, Permission_Invisible) == true)
		creatureInst->_AddStatusList(StatusEffects::GM_INVISIBLE, -1);

	g_PartyManager.UpdatePlayerReferences(creatureInst);

	creatureInst->SetServerFlag(ServerFlags::Noncombatant, true);
	pld.IgnoreNextMovement = true;
	return true;
}

bool SimulatorThread::ProtectedSetZone(int newZoneID, int newInstanceID) {
	//The function originally had thread potection when using a faulty method of threading.
	//Returns false if the operation failed.
	MainCallSetZone(newZoneID, newInstanceID, false);
	return ValidPointers();
}

void SimulatorThread::LoadCharacterSession(void) {
	//Loads session information from the Character data.
	TimeOnline = g_ServerTime;
	TimeLastAutoSave = g_ServerTime;

	pld.charPtr->Shard = g_ClusterManager.mShardName;

	time_t curtime;
	time(&curtime);
	char buf[24];
	strftime(buf, sizeof(buf), "%Y-%m-%d, %I:%M %p", localtime(&curtime));
	pld.charPtr->LastLogOn = buf;
	pld.charPtr->Shard = g_ClusterManager.mShardName;

	// Determine if we should give a daily reward for the account
	if (pld.accPtr->LastLogOn.length() == 0) {
		pld.accPtr->DueDailyRewards = true;
	} else {
		// Test on the date portion only
		STRINGLIST thisDate;
		STRINGLIST lastLogonDate;
		Util::Split(pld.accPtr->LastLogOn, ",", thisDate);
		Util::Split(pld.charPtr->LastLogOn, ",", lastLogonDate);
		pld.accPtr->DueDailyRewards = strcmp(thisDate[0].c_str(),
				lastLogonDate[0].c_str()) != 0;
	}
	if (pld.accPtr->DueDailyRewards) {
		g_Logs.simulator->info("[%v] %v is logging on for the first time today",
				InternalID, pld.accPtr->Name);
		pld.charPtr->LastLogOn = pld.accPtr->LastLogOn;

		// Is this a consecutive day?
		unsigned long lastLoginDay =
				pld.accPtr->LastLogOnTimeSec == 0 ?
						0 : pld.accPtr->LastLogOnTimeSec / 86400;
		unsigned long nowTimeSec = time(NULL);
		unsigned long nowTimeDay = nowTimeSec / 86400;
		if (lastLoginDay == 0 || nowTimeDay == lastLoginDay + 1) {
			// It is!
			pld.accPtr->ConsecutiveDaysLoggedIn++;
			g_Logs.simulator->info(
					"[%v] %v has now logged in %v consecutive days.",
					InternalID, pld.accPtr->Name,
					pld.accPtr->ConsecutiveDaysLoggedIn);
		} else {
			// Not a consecutive day, so not due daily rewards
			pld.accPtr->DueDailyRewards = false;
		}

		pld.accPtr->LastLogOnTimeSec = nowTimeSec;
		pld.accPtr->PendingMinorUpdates++;
	}

	pld.charPtr->SessionsLogged++;

	defcInst.CurrentX = pld.charPtr->activeData.CurX;
	defcInst.CurrentY = pld.charPtr->activeData.CurY;
	defcInst.CurrentZ = pld.charPtr->activeData.CurZ;
	defcInst.Rotation = pld.charPtr->activeData.CurRotation;
	defcInst.Heading = pld.charPtr->activeData.CurRotation;
	defcInst.cooldownManager.CopyFrom(pld.charPtr->cooldownManager);
	defcInst.buffManager.CopyFrom(pld.charPtr->buffManager);
}

void SimulatorThread::ChangeProtocol(int newProto) {
	g_Logs.simulator->info("[%v] Changing Protocol: %v", InternalID, newProto);
	ProtocolState = newProto;
	if (ProtocolState == 1) {
		int wpos = 0;
		wpos += PutByte(&SendBuf[wpos], 255);        //_handleProtocolChangedMsg
		wpos += PutShort(&SendBuf[wpos], 1);              //Message size
		wpos += PutByte(&SendBuf[wpos], ProtocolState);   //Set to Play protocol
		AttemptSend(SendBuf, wpos);
	}
}

bool SimulatorThread::ValidPointers(void) {
	//Standard debug check.  Makes sure the pointers are valid.
	if (pld.accPtr == NULL) {
		g_Logs.simulator->error("[%v] ValidPointers failed accPtr", InternalID);
		return false;
	}
	if (pld.charPtr == NULL) {
		g_Logs.simulator->error("[%v] ValidPointers failed charPtr",
				InternalID);
		return false;
	}
	if (creatureInst == NULL) {
		g_Logs.simulator->error("[%v] ValidPointers failed creatureInst",
				InternalID);
		return false;
	}
	if (creatureInst->actInst == NULL) {
		g_Logs.simulator->error("[%v] ValidPointers failed actInst",
				InternalID);
		return false;
	}
	if (pld.zoneDef == NULL) {
		g_Logs.simulator->error("[%v] ValidPointers failed zoneDef",
				InternalID);
		return false;
	}
	return true;
}

void SimulatorThread::SendSetAvatar(int CreatureID) {
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

void SimulatorThread::MainCallHelperInstanceUnregister(void) {

	if (creatureInst->actInst != NULL)
		creatureInst->actInst->UnregisterPlayer(this);

	g_PartyManager.RemovePlayerReferences(creatureInst->CreatureDefID, false);

	if (creatureInst->actInst == NULL)
		return;

	int wpos = PrepExt_RemoveCreature(SendBuf, creatureInst->CreatureID);
	creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, InternalIndex);

	creatureInst->actInst->SidekickUnregister(creatureInst);
	g_ActiveInstanceManager.FlushSimulator(InternalID);

	creatureInst->OnInstanceExit();
	defcInst.CopyFrom(creatureInst);
	creatureInst->actInst->UnloadPlayer(this);

	creatureInst = &defcInst; //The former CreatureInstance object no longer exists.
	defcInst.actInst = NULL;
	pld.zoneDef = NULL;
	pld.CurrentZoneID = 0;
}

void SimulatorThread::FillPlayerInstancePlacementData(
		PlayerInstancePlacementData &pd, int ZoneID, int InstanceID) {
	pd.in_cInst = &defcInst; //This is the "default" holder of creature data when swapping instances.
	pd.in_simPtr = this;
	pd.in_instanceID = InstanceID;
	pd.in_creatureDefID = pld.CreatureDefID;
	pd.in_partyID = creatureInst->PartyID;   //Use current instance party.
	pd.in_serverSessionID = 0;
	pd.in_zoneID = ZoneID;
	pd.in_playerLevel = defcInst.css.level;
	pd.SetInstanceScaler(pld.charPtr->InstanceScaler);
}

void SimulatorThread::MainCallHelperInstanceRegister(int ZoneID,
		int InstanceID) {
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

	if (g_ActiveInstanceManager.AddSimulator_Ex(pd) == -1) {
		g_Logs.simulator->error("[%v] InstanceRegister() failed (Zone:%v)",
				InternalID, ZoneID);
		return;
	}

	if (pd.out_cInst != NULL) {
		creatureInst = pd.out_cInst;
		pld.zoneDef = pd.out_zoneDef;
		pld.CurrentInstanceID = pd.out_instanceID;
		pld.CreatureID = pd.out_cInst->CreatureID;
		creatureInst->actInst->SidekickRegister(creatureInst,
				&pld.charPtr->SidekickList);
	} else {
		g_Logs.simulator->error("[%v] InstanceRegister() failed (Zone:%v)",
				InternalID, ZoneID);
		creatureInst = &defcInst;
	}
}

std::string SimulatorThread::GetTimeOfDay(void) {
	if (CheckPermissionSimple(Perm_Account, Permission_Troll))
		return "Day";

	if (creatureInst != NULL) {
		return creatureInst->actInst->GetTimeOfDay().c_str();
	}

	return "Day";
}

void SimulatorThread::CheckMapUpdate(bool force) {
	//Send minimap region title information to the client.
	if (pld.zoneDef == NULL) {
		g_Logs.simulator->error("[%v] CheckMapUpdate() ZoneDef is NULL",
				InternalID);
		return;
	}

	pld.LastMapTick++;
	if (pld.LastMapTick >= MapTickChange || force == true) {
		pld.LastMapTick = 0;
		int NewMap = 0;
		NewMap = MapLocation.SearchLocation(pld.CurrentZoneID,
				creatureInst->CurrentX, creatureInst->CurrentZ);
		if (NewMap == -1)
			NewMap = MapDef.SearchMap(pld.zoneDef->mMapName.c_str(),
					creatureInst->CurrentX, creatureInst->CurrentZ);

		if (NewMap != -1) {
			if (pld.CurrentMapInt != NewMap) {
				pld.CurrentMapInt = NewMap;
				SendInfoMessage(MapDef.mMapList[pld.CurrentMapInt].Name.c_str(),
						INFOMSG_LOCATION);
				SendInfoMessage(pld.charPtr->Shard.c_str(), INFOMSG_SHARD);
				SendInfoMessage(
						MapDef.mMapList[pld.CurrentMapInt].image.c_str(),
						INFOMSG_MAPNAME);

				WeatherState *ws = g_WeatherManager.GetWeather(
						MapDef.mMapList[pld.CurrentMapInt].Name,
						pld.CurrentInstanceID);
				if (ws != NULL)
					AttemptSend(SendBuf,
							PrepExt_SetWeather(SendBuf, ws->mWeatherType,
									ws->mWeatherWeight));
				else
					AttemptSend(SendBuf, PrepExt_SetWeather(SendBuf, "", 0));
			} else if (force) {
				WeatherState *ws = g_WeatherManager.GetWeather(
						MapDef.mMapList[pld.CurrentMapInt].Name,
						pld.CurrentInstanceID);
				if (ws != NULL)
					AttemptSend(SendBuf,
							PrepExt_SetWeather(SendBuf, ws->mWeatherType,
									ws->mWeatherWeight));
				else
					AttemptSend(SendBuf, PrepExt_SetWeather(SendBuf, "", 0));
			}
		} else {
			//Most instances and smaller maps don't have specific regions, so reset the
			//internal map index so that a warp back to a map area will refresh.
			pld.CurrentMapInt = -1;

			SendInfoMessage(pld.zoneDef->mName.c_str(), INFOMSG_LOCATION);
			SendInfoMessage(pld.charPtr->Shard.c_str(), INFOMSG_SHARD);

			WeatherState *ws = g_WeatherManager.GetWeather(pld.zoneDef->mName,
					pld.CurrentInstanceID);
			if (ws != NULL) {
				AttemptSend(SendBuf,
						PrepExt_SetWeather(SendBuf, ws->mWeatherType,
								ws->mWeatherWeight));
			}
		}

	}
}

void SimulatorThread::UpdateSocialEntry(bool newOnlineStatus,
		bool onlyUpdateFriendList) {
	if (pld.charPtr == NULL) {
		g_Logs.simulator->error("[%v] UpdateSocialEntry charPtr is NULL",
				InternalID);
		return;
	}
	if (pld.zoneDef == NULL) {
		g_Logs.simulator->error("[%v] UpdateSocialEntry zoneDef is NULL",
				InternalID);
		return;
	}
	if (onlyUpdateFriendList == true) {
		std::vector<int> friendDefID;
		for (size_t i = 0; i < pld.charPtr->friendList.size(); i++)
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
	data.shard = pld.charPtr->Shard;

	g_FriendListManager.UpdateSocialEntry(data);
}

void SimulatorThread::BroadcastGuildChange(int guildDefID) {
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 99);
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutByte(&SendBuf[wpos], 1);
	wpos += PutInteger(&SendBuf[wpos], guildDefID);
	PutShort(&SendBuf[1], wpos - 3);
	AttemptSend(SendBuf, wpos);
}

void SimulatorThread::BroadcastShardChanged(void) {
	if (IsGMInvisible() == true)
		return;

	// When this Simulator has changed shards, update all other players who
	// have this player on their friend list.  Notify them of the shard change.
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);

	wpos += PutByte(&SendBuf[wpos], 15); //Event to change shards
	wpos += PutStringUTF(&SendBuf[wpos], creatureInst->css.display_name);
	wpos += PutStringUTF(&SendBuf[wpos], pld.charPtr->Shard.c_str());
	PutShort(&SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->InternalID == InternalID)
			continue;

		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.charPtr->GetFriendIndex(pld.CreatureDefID) >= 0)
				it->AttemptSend(SendBuf, wpos);
		}
	}
}

void SimulatorThread::AddMessage(long param1, long param2, int message) {
	//Normal instance check, plus hack to get the new disconnect working.
	if (creatureInst->actInst == NULL) {
		g_Logs.simulator->error(
				"[%d] AddMessage on NULL instance (msg: %d, param1: %d (%p), param2: %d (%p))",
				InternalID, message, param1, param1, param2, param2);
		return;
	}

#ifdef DEBUG_TIME
	Debug::TimeTrack("SimulatorThread::AddMessage", 30);
#endif

	/*
	 if(bcm.AddEvent2(InternalIndex, param1, param2, message, creatureInst->actInst) == -1)
	 g_Logs.simulator->error("Could not add BroadCast event.");
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

void SimulatorThread::SendSetMap(void) {
}

void SimulatorThread::SetRotation(int rot, int update) {
	creatureInst->Heading = creatureInst->Rotation = rot;

	if (update == 1) {
		pld.MovementBlockTime = g_ServerTime + g_Config.WarpMovementBlockTime;

		// Tell everyone else
		if (!IsGMInvisible()) {
			int size = PrepExt_UpdateVelocity(SendBuf, creatureInst);
			creatureInst->actInst->LSendToLocalSimulator(SendBuf, size,
					creatureInst->CurrentX, creatureInst->CurrentZ);
		}

		// Tell player
		int size = PrepExt_VelocityEvent(SendBuf, creatureInst);
		AttemptSend(SendBuf, size);
	}
}

void SimulatorThread::SetPosition(int xpos, int ypos, int zpos, int update) {
	creatureInst->CurrentX = xpos;
	creatureInst->CurrentY = ypos;
	creatureInst->CurrentZ = zpos;

	if (creatureInst->actInst != NULL)
		creatureInst->actInst->PlayerMovement(creatureInst);

	if (update == 1) {
		pld.MovementBlockTime = g_ServerTime + g_Config.WarpMovementBlockTime;

		if (IsGMInvisible() == true) {
			int size = PrepExt_CreaturePos(SendBuf, creatureInst);
			size += PrepExt_GeneralMoveUpdate(&SendBuf[size], creatureInst);
			if (g_Config.UseStopSwim == true)
				size += PrepExt_ModStopSwimFlag(&SendBuf[size], false);
			size += PrepExt_VelocityEvent(&SendBuf[size], creatureInst);
			AttemptSend(SendBuf, size);
		} else {
			int size = PrepExt_UpdateFullPosition(SendBuf, creatureInst);
			if (g_Config.UseStopSwim == true)
				size += PrepExt_ModStopSwimFlag(&SendBuf[size], false);
			creatureInst->actInst->LSendToLocalSimulator(SendBuf, size,
					creatureInst->CurrentX, creatureInst->CurrentZ);
			AddMessage((long) creatureInst, 0, BCM_UpdateVelocity);
			AttemptSend(SendBuf, PrepExt_VelocityEvent(SendBuf, creatureInst));
		}

		int r = pld.charPtr->questJournal.CheckTravelLocations(
				creatureInst->CreatureID, Aux1, creatureInst->CurrentX,
				creatureInst->CurrentY, creatureInst->CurrentZ,
				pld.CurrentZoneID);
		if (r > 0)
			AttemptSend(Aux1, r);
	}
}

void SimulatorThread::UpdateEqAppearance(void) {
	pld.charPtr->UpdateEqAppearance();
	//Util::SafeCopy(creatureInst->css.eq_appearance, pld.charPtr->cdef.css.eq_appearance, sizeof(creatureInst->css.eq_appearance));
	creatureInst->css.SetEqAppearance(
			pld.charPtr->cdef.css.eq_appearance.c_str());

	int wpos = PrepExt_SendEqAppearance(SendBuf, pld.CreatureDefID,
			creatureInst->PeekAppearanceEq().c_str());
	wpos += PrepExt_ModStopSwimFlag(&SendBuf[wpos], false);

	creatureInst->BroadcastLocal(SendBuf, wpos);

	//Stats
	float oldHealthRatio = (float) creatureInst->css.health
			/ (float) creatureInst->GetMaxHealth(true);

	creatureInst->RemoveStatModsBySource(BuffSource::ITEM);
	pld.charPtr->UpdateBaseStats(creatureInst, true);
	pld.charPtr->UpdateEquipStats(creatureInst);

	creatureInst->OnEquipmentChange(oldHealthRatio);
}

void SimulatorThread::ActivatePassiveAbilities(void) {
	Ability2::UniqueAbilityList uniqueList;
	const Ability2::AbilityEntry2 *abEntry = NULL;
	for (size_t i = 0; i < pld.charPtr->abilityList.AbilityList.size(); i++) {
		int id = pld.charPtr->abilityList.AbilityList[i];
		abEntry = g_AbilityManager.GetAbilityPtrByID(id);
		if (abEntry != NULL) {
			if (abEntry->IsPassive() == true)
				uniqueList.AddAbilityToList(abEntry);
		}
	}

	for (size_t i = 0; i < uniqueList.mAbilityList.size(); i++) {
		int abID = uniqueList.mAbilityList[i].mID;

		//Need request to fill the target data, then run activation.
		creatureInst->CallAbilityEvent(abID, EventType::onRequest);
		creatureInst->CallAbilityEvent(abID, EventType::onActivate);
		creatureInst->ab[0].Clear("ActivatePassiveAbilities");
	}
}

void SimulatorThread::SetLoadingStatus(bool status, bool shutdown) {
	//Called when a "client.loading" query is received.
	ClientLoading = status;

	// Process abnormal shutdown. This avoids sending the notification messages
	// below.  Disconnect() may call this function, this avoids pointer invalidation
	// and and yet another Disconnect() call below.
	if (shutdown == true) {
		if (LoadStage == LOADSTAGE_LOADING)
			g_Logs.simulator->info(
					"[%v] SetLoadingStatus() LOADSTAGE_LOADING when shutting down.",
					InternalID);

		LoadStage = LOADSTAGE_UNLOADED;
		return;
	}

	if (ClientLoading == true) {
		if (LoadStage == LOADSTAGE_UNLOADED) {
			LoadStage = LOADSTAGE_LOADING;
			SendSetAvatar(creatureInst->CreatureID);
		}
	} else {
		if (LoadStage == LOADSTAGE_LOADING) {
			if (ValidPointers() == false) {
				ForceErrorMessage(
						"Unexpected error when setting final load stage.",
						INFOMSG_ERROR);
				sc.DisconnectClient();
				return;
			}
			//AddMessageG(SMSG_PlayerLogIn, (long)pld->charPtr, InternalIndex);
			//AddMessageG(SMSG_PlayerFriendLogState, (long)pld->charPtr, 1);
			if (CheckPermissionSimple(Perm_Account, Permission_Invisible)
					== false) {
				AddMessage((long) pld.charPtr, 0, BCM_PlayerLogIn);
				AddMessage((long) this, 1, BCM_PlayerFriendLogState);
				AddMessage((long) creatureInst, 0, BCM_UpdatePosition);
				/* This was originally added to try and solve the initial 120% speed issue that
				 was sometimes present in the client.  Disabled here since it would override the
				 new global speed increase.
				 creatureInst->Speed = 0;
				 creatureInst->css.mod_movement = 0;
				 creatureInst->css.base_movement = 0;
				 */

				static const short stats[2] = { STAT::MOD_MOVEMENT,
						STAT::BASE_MOVEMENT };
				int wpos = PrepExt_SendSpecificStats(SendBuf, creatureInst,
						&stats[0], 2);
				AttemptSend(SendBuf, wpos);
			}

			g_PartyManager.CheckMemberLogin(creatureInst);
			g_Scheduler.Submit(
					[this]() {
						std::string message = g_InfoManager.GetMOTD();
						if (message.size() != 0) {
							int size = PrepExt_GenericChatMessage(GSendBuf, 0, g_MOTD_Name.c_str(),
									g_MOTD_Channel.c_str(), message.c_str());
							this->creatureInst->actInst->LSendToOneSimulator(GSendBuf, size, this->ThreadID);
						}
					});

			ProcessDailyRewards(pld.accPtr->ConsecutiveDaysLoggedIn,
					pld.charPtr->cdef.css.level);

			if (pld.accPtr->SiteSession.unreadMessages > 0) {
				char buf[256];
				Util::SafeFormat(buf, sizeof(buf),
						"You have %d unread private message%s. See %s",
						pld.accPtr->SiteSession.unreadMessages,
						pld.accPtr->SiteSession.unreadMessages > 1 ? "s" : "",
						g_URLManager.GetURL("NewMessages").c_str());
				AttemptSend(SendBuf,
						PrepExt_SendInfoMessage(SendBuf, buf, INFOMSG_INFO));
			}

			// TODO I cannot remember what exactly this was for.
//			char buf[256];
//			std::vector<std::string> l;
//			Util::Split(Util::CaptureCommand(buf), "\r", l);
//			for (auto it = l.begin(); it != l.end(); ++it)
//				AttemptSend(SendBuf,
//						PrepExt_SendInfoMessage(SendBuf, (*it).c_str(),
//								INFOMSG_INFO));

			LoadStage = LOADSTAGE_LOADED; //Initial loading screen is finished, players should be able to control their characters.
		}
	}
}

void SimulatorThread::ProcessDailyRewards(unsigned int days,
		unsigned int level) {

	// Handle daily rewards
	if (pld.accPtr->DueDailyRewards) {
		pld.accPtr->DueDailyRewards = false;
		pld.accPtr->PendingMinorUpdates++;

		std::vector<DailyProfile> profiles = g_DailyProfileManager.GetProfiles(
				days, level);
		if (profiles.size() == 0) {
			// Reset to day1
			pld.accPtr->ConsecutiveDaysLoggedIn = 1;
			profiles = g_DailyProfileManager.GetProfiles(days, level);
		}

		if (profiles.size() > 0) {
			char buf[256];
			Util::SafeFormat(buf, sizeof(buf),
					"This is day %d of a maximum of %d consecutive login days. You earned :-",
					days, g_DailyProfileManager.GetMaxDayNumber());
			AttemptSend(SendBuf,
					PrepExt_SendInfoMessage(SendBuf, buf, INFOMSG_INFO));

			std::vector<DailyProfile> lootProfiles;

			for (std::vector<DailyProfile>::iterator it = profiles.begin();
					it != profiles.end(); ++it) {
				DailyProfile profile = *it;

				switch (profile.rewardType) {
				case RewardType::CREDITS:
					creatureInst->AddCredits(profile.creditReward.credits);
					Util::SafeFormat(buf, sizeof(buf),
							" * %d credits to spend in the shop",
							profile.creditReward.credits);
					AttemptSend(SendBuf,
							PrepExt_SendInfoMessage(SendBuf, buf,
									INFOMSG_INFO));
					break;
				case RewardType::VIRTUAL:
				case RewardType::ITEM:
					lootProfiles.push_back(profile);
					break;
				}
			}

			if (lootProfiles.size() > 0) {
				CreatureInstance* lootInst =
						creatureInst->actInst->SpawnGeneric(
								lootProfiles[0].spawnCreatureDefID,
								creatureInst->CurrentX, creatureInst->CurrentY,
								creatureInst->CurrentZ, 0, 0);
				lootInst->deathTime = g_ServerTime;
				lootInst->PrepareDeath();
				lootInst->PlayerLoot(creatureInst->css.level, lootProfiles);
				lootInst->AddLootableID(creatureInst->CreatureDefID);
				if (lootInst->activeLootID != 0) {
					Util::SafeFormat(buf, sizeof(buf),
							" * A daily reward chest");
					AttemptSend(SendBuf,
							PrepExt_SendInfoMessage(SendBuf, buf,
									INFOMSG_INFO));
					lootInst->SendUpdatedLoot();
				}
			}
		}

	}
}

int SimulatorThread::SendInventoryData(std::vector<InventorySlot> &cont) {
	//NOTE: This function assumes that the inventory tables are not currently being edited.
	//Should not be assumed to be thread safe if other functions are adding/removing items.

	int count = (int) cont.size();
	int proc = 0;  //Current index being processed
	int tproc = 0; //Total processed;
	int batch = 0; //Count of the number of items contained in this batch of requests.
	int wpos = 0;  //Current Write position
	while (proc < count) {
		InventorySlot *slot = &cont[proc];
		if (slot->secondsRemaining == -1 || !slot->IsExpired()) {
			wpos += AddItemUpdate(&SendBuf[wpos], Aux3, slot);
			proc++;
		}
		batch++;
		tproc++;
		if (batch > 20) {
			AttemptSend(SendBuf, wpos);
			wpos = 0;
			batch = 0;
		}
	}
	if (batch > 0)
		AttemptSend(SendBuf, wpos);
	g_Logs.simulator->debug("[%v] Sent %v item slots.", InternalID, tproc);
	return tproc;
}

bool SimulatorThread::CheckPermissionSimple(int permissionSet,
		unsigned int permissionFlag) {
	//Simple permission check.  Look to see if the given flag exists in the account
	//permissions.  Return true if it does, or false if it doesn't.
	//Intended to check for a specific permission or set of flags.
	if (pld.accPtr == NULL) {
		g_Logs.simulator->error("CheckPermissionSimple accPtr is NULL");
		return false;
	}
	if (pld.charPtr == NULL) {
		g_Logs.simulator->error("CheckPermissionSimple charPtr is NULL");
		return false;
	}

	if (pld.accPtr->HasPermission(permissionSet, permissionFlag) == true)
		return true;

	return pld.charPtr->HasPermission(permissionSet, permissionFlag);
}

const char * SimulatorThread::GetGenericErrorString(int errCode) {
	switch (errCode) {
	case ERROR_NONE:
		return "No error.";
	case ERROR_INVALIDZONE:
		return "Zone does not exist.";
	case ERROR_WARPGROVEONLY:
		return "Permission denied: you may only warp to other groves.";
	case ERROR_USERBLOCK:
		return "You are not allowed to enter that grove.";
	}
	return "Unknown error.";
}
;

int SimulatorThread::CheckValidWarpZone(int ZoneID) {
	//Determine whether the player is capable of warping to the target zone.  This involves checking
	//permissions of grove status or private lists.

	ZoneDefInfo *zonePtr = g_ZoneDefManager.GetPointerByID(ZoneID);
	if (zonePtr == NULL) {
		g_Logs.simulator->error("[%v] Invalid ZoneID for warp: %v", InternalID,
				ZoneID);
		return ERROR_INVALIDZONE;
	}

	//For administrative access
	if (CheckPermissionSimple(Perm_Account,
			Permission_Admin | Permission_Developer | Permission_Sage) == true)
		return ERROR_NONE;

	//For regular players
	if (zonePtr->mGrove == false && zonePtr->mArena == false
			&& zonePtr->mGuildHall == false)
		if (CheckPermissionSimple(Perm_Account,
				Permission_Debug | Permission_Sage | Permission_Admin
						| Permission_Developer) == false)
			return ERROR_WARPGROVEONLY;

	//For guild hall
	if (zonePtr->mGuildHall) {
		// Must be in guild
		GuildDefinition *gdef =
				g_GuildManager.GetGuildDefinitionForGuildHallZoneID(ZoneID);
		if (!pld.charPtr->IsInGuildAndHasValour(gdef->guildDefinitionID, 0)) {
			g_Logs.simulator->warn(
					"[%d] Not allowed to warp to guild hall unless in guild: %d",
					InternalID, ZoneID);
			return ERROR_INVALIDZONE;
		} else {
			return ERROR_NONE;
		}
	}

	if (zonePtr->CanPlayerWarp(creatureInst->CreatureDefID, pld.accPtr->ID)
			== false)
		return ERROR_USERBLOCK;

	return ERROR_NONE;
}

//This function performs a warp.  Assumes that the target is a verified destination for the player.
void SimulatorThread::DoWarp(int zoneID, int instanceID, int xpos, int ypos,
		int zpos) {
	if (zoneID != pld.CurrentZoneID) {
		if (ProtectedSetZone(zoneID, instanceID) == false) {
			ForceErrorMessage("Critical error while changing zones.",
					INFOMSG_ERROR);
			Disconnect("SimulatorThread::handle_command_warp");
			return;
		}
	}
	if (xpos != creatureInst->CurrentX || ypos != creatureInst->CurrentY
			|| zpos != creatureInst->CurrentZ || zoneID != pld.CurrentZoneID) {
		SendInfoMessage("Warping.", INFOMSG_INFO);
		SetPosition(xpos, ypos, zpos, 1);
		pld.LastMapTick = MapTickChange;
		CheckSpawnTileUpdate(true);
		CheckMapUpdate(true);
	}
}

bool SimulatorThread::CheckWriteFlush(int &curPos) {
	if (curPos > Global::MAX_SEND_CHUNK_SIZE) {
		AttemptSend(SendBuf, curPos);
		curPos = 0;
		return true;
	}
	return false;
}

int SimulatorThread::protected_helper_tweak_self(int CDefID, int defhints,
		int argOffset) {
	if (CheckPermissionSimple(0, Permission_TweakClient) == true) {
		const char *appearance = NULL;
		for (uint i = 1 + argOffset; i < query.argCount; i += 2) {
			const char *name = query.args[i].c_str();
			const char *value = query.args[i + 1].c_str();
			if (strcmp(name, "appearance") == 0) {
				appearance = value;
				break;
			}
		}
		int size = 0;
		if (appearance != NULL) {
			std::vector<short> statID;
			statID.push_back(STAT::APPEARANCE);
			CharacterStatSet data;
			//Util::SafeCopy(data.appearance, appearance, sizeof(data.appearance));
			data.SetAppearance(appearance);
			size = PrepExt_UpdateCreatureDef(SendBuf, CDefID, defhints, statID,
					&data);
		}
		size += PrepExt_QueryResponseString(&SendBuf[size], query.ID, "OK");
		return size;
	} else
		return -1;
}

int SimulatorThread::protected_CheckDistance(int creatureID) {
	return protected_CheckDistanceBetweenCreatures(creatureInst, creatureID);
}

int SimulatorThread::protected_CheckDistanceBetweenCreatures(
		CreatureInstance *sourceCreatureInst, int creatureID) {
	return protected_CheckDistanceBetweenCreaturesFor(sourceCreatureInst,
			creatureID, INTERACT_RANGE);
}

int SimulatorThread::protected_CheckDistanceBetweenCreaturesFor(
		CreatureInstance *sourceCreatureInst, int creatureID, int range) {
	if (sourceCreatureInst == NULL)
		return QueryErrorMsg::GENERIC;
	if (sourceCreatureInst->actInst == NULL)
		return QueryErrorMsg::GENERIC;

	CreatureInstance *object = sourceCreatureInst->actInst->GetNPCInstanceByCID(
			creatureID);
	if (object == NULL)
		return QueryErrorMsg::INVALIDOBJ;
	int dist = sourceCreatureInst->actInst->GetBoxRange(sourceCreatureInst,
			object);
	if (dist > range)
		return QueryErrorMsg::OUTOFRANGE;

	return 0;
}

int SimulatorThread::protected_helper_query_loot_item(void) {
	// 1. Free for all + Need or greed OFF. Loot will go to the looter
	// 2. Free for all + Need or greed ON. Loot will be offered to all, player is picked accordig to greed roll
	// 3. Round Robin + Need or greed OFF. Loot will be offered to robin, then party to greed or pass.
	// 4. Round Robin + Need or greed ON. Loot will be offered to robin, then party to need, greed or pass.
	// 5. Loot Master + Need or greed OFF. Loot will be offered to leader, then party to greed or pass.
	// 6. Loot Master + Need or greed ON. Loot will be offered to leader, then party to need, greed or pass.

	if (query.argCount < 2)
		return QueryErrorMsg::GENERIC;

	int CID = atoi(query.args[0].c_str());
	int ItemID = atoi(query.args[1].c_str());
	//LogMessageL(MSG_SHOW, "loot.item: %d, %d", CID, ItemID);

	CreatureInstance *receivingCreature = creatureInst;
	CharacterData *charData = pld.charPtr;
	ItemDef *itemDef = g_ItemManager.GetPointerByID(ItemID);

	/* If the player is in a party, check the party loot rules.
	 */
	ActiveParty *party = g_PartyManager.GetPartyByID(
			receivingCreature->PartyID);
	bool partyLootable =
			party != NULL
					&& (itemDef->mQualityLevel >= 2
							|| (party->mLootFlags & MUNDANE) > 0);
	bool needOrGreed = partyLootable && (party->mLootFlags & NEED_B4_GREED) > 0;

	ActiveInstance *aInst = receivingCreature->actInst;

	//The client uses creature definitions as lootable IDs.
	int PlayerCDefID = receivingCreature->CreatureDefID;

	// Make sure this object isn't too far away.
	int distCheck = protected_CheckDistance(CID);
	if (distCheck != 0)
		return distCheck;

	int r = aInst->lootsys.GetCreature(CID);
	if (r == -1)
		return QueryErrorMsg::LOOT;

	ActiveLootContainer *loot = &aInst->lootsys.creatureList[r];

	int canLoot = loot->HasLootableID(PlayerCDefID);
	if (canLoot == -1)
		return QueryErrorMsg::LOOTDENIED;

	int conIndex = loot->HasItem(ItemID);
	if (conIndex == -1)
		return QueryErrorMsg::LOOTMISSING;

	// Check if already waiting on rolls
	if (partyLootable && party->HasTags(PlayerCDefID, ItemID)) {
		g_Logs.simulator->warn("[%v] Denying loot. Already looting %v",
				InternalID, ItemID);
		return QueryErrorMsg::LOOTDENIED;
	}

	// Offer the loot instead if appropriate
	if (partyLootable && !(party->mLootMode == FREE_FOR_ALL && !needOrGreed)) {
		g_Logs.simulator->info("[%v] Trying party loot for %v", InternalID,
				ItemID);
		WritePos = OfferLoot(party->mLootMode, loot, party, receivingCreature,
				ItemID, needOrGreed, CID, conIndex);
		if (WritePos < 0) {
			return WritePos;
		}
	} else {
		g_Logs.simulator->info("[%v] An ordinary single player loot %v",
				InternalID, ItemID);

		// Either there is no party, or the loot rules decided that the looter just gets the item

		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if (slot == -1)
			return QueryErrorMsg::INVSPACE;

		InventorySlot *newItem = charData->inventory.AddItem_Ex(INV_CONTAINER,
				ItemID, 1);
		if (newItem == NULL)
			return QueryErrorMsg::INVCREATE;

		charData->pendingChanges++;
		ActivateActionAbilities(newItem);

		loot->RemoveItem(conIndex);
		if (loot->itemList.size() == 0) {
			CreatureInstance *lootCreature = aInst->GetNPCInstanceByCID(CID);
			if (lootCreature != NULL) {
				lootCreature->activeLootID = 0;
				lootCreature->css.ClearLootSeeablePlayerIDs();
				lootCreature->css.ClearLootablePlayerIDs();
				lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
				lootCreature->css.appearance_override =
						LootSystem::DefaultTombstoneAppearanceOverride;
				static const short statList[3] =
						{ STAT::APPEARANCE_OVERRIDE, STAT::LOOTABLE_PLAYER_IDS,
								STAT::LOOT_SEEABLE_PLAYER_IDS };
				WritePos = PrepExt_SendSpecificStats(SendBuf, lootCreature,
						&statList[0], 3);
				aInst->LSendToLocalSimulator(SendBuf, WritePos,
						creatureInst->CurrentX, creatureInst->CurrentZ);
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
	WritePos += PrepExt_QueryResponseStringList(&SendBuf[WritePos], query.ID,
			qresponse);

	return QueryErrorMsg::NONE;
}

const char * SimulatorThread::GetErrorString(int error) {
	switch (error) {
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

bool SimulatorThread::HasQueryArgs(unsigned int minCount) {
	if (query.argCount < minCount) {
		g_Logs.simulator->warn("[%v] Expected more arguments for %v",
				InternalID, query.name.c_str());
		WritePos = PrepExt_QueryResponseString(SendBuf, query.ID, "OK");
		PendingSend = true;
		return false;
	}
	return true;
}

const char * SimulatorThread::GetScriptUsable(CreatureInstance *target) {

	std::vector<ScriptCore::ScriptParam> parms;
	parms.push_back(ScriptCore::ScriptParam(target->CreatureID));
	parms.push_back(ScriptCore::ScriptParam(target->CreatureDefID));
	parms.push_back(ScriptCore::ScriptParam(creatureInst->CreatureID));
	parms.push_back(ScriptCore::ScriptParam(creatureInst->CreatureDefID));
	return creatureInst->actInst->nutScriptPlayer->RunFunctionWithStringReturn(
			"is_usable", parms, true).c_str();
}

/*
 Check for build permission at the specified coordinate (in the player's current standing
 zone.  If a prop is supplied, use the coordinates of the prop instead.
 */
bool SimulatorThread::HasPropEditPermission(SceneryObject *prop, float x,
		float z) {
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
	if (prop != NULL) {
		checkX = prop->LocationX;
		checkZ = prop->LocationZ;
	}

	if (CheckPermissionSimple(Perm_Account,
			Permission_Admin | Permission_Developer)
			|| pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID,
					pld.zoneDef->mPageSize, checkX, checkZ) == true)
		return true;

	if (pld.zoneDef->HasEditPermission(pld.accPtr->ID, pld.CreatureDefID,
			creatureInst->css.display_name, checkX, checkZ) == true)
		return true;

	return false;
}

void SimulatorThread::SaveCharacterStats(void) {
	//This function pushes the Active Character data back into the main Character Data
	//in preparation for an autosave.

	if (pld.charPtr == NULL) {
		g_Logs.simulator->error("[%v] SaveCharacterStats() charPtr is NULL",
				InternalID);
		return;
	}

	pld.charPtr->activeData.CurInstance = pld.CurrentInstanceID;
	pld.charPtr->activeData.CurZone = pld.CurrentZoneID;
	pld.charPtr->activeData.CurX = creatureInst->CurrentX;  //pld->CurrentX;
	pld.charPtr->activeData.CurY = creatureInst->CurrentY;  //pld->CurrentY;
	pld.charPtr->activeData.CurZ = creatureInst->CurrentZ;  //pld->CurrentZ;
	pld.charPtr->activeData.CurRotation = creatureInst->Rotation; //pld->CurrentZ;

	int TimeSpan = (g_ServerTime - TimeOnline) / 1000;
	int Hour = TimeSpan / 3600;
	int Minute = (TimeSpan / 60) % 60;
	int Second = TimeSpan % 60;

	pld.charPtr->LastSession = StringUtil::Format("%02d:%02d:%02d", Hour,
			Minute, Second);

	int secSinceAutoSave = (g_ServerTime - TimeLastAutoSave) / 1000;
	TimeLastAutoSave = g_ServerTime;

	pld.charPtr->SecondsLogged += secSinceAutoSave;
	int TotalSec = pld.charPtr->SecondsLogged;

	Hour = TotalSec / 3600;
	Minute = (TotalSec / 60) % 60;
	Second = TotalSec % 60;

	pld.charPtr->TimeLogged = StringUtil::Format("%02d:%02d:%02d", Hour, Minute,
			Second);

	time_t curtime;
	time(&curtime);
	char buf[24];
	strftime(buf, sizeof(buf), "%Y-%m-%d, %I:%M %p", localtime(&curtime));
	pld.charPtr->LastLogOff = buf;
	pld.charPtr->Shard = g_ClusterManager.mShardName;

	//Need to copy the stats to the character definition, otherwise the updated
	//information won't save out.
	//First restore any transformed appearances.
//	creatureInst->AppearanceUnTransform();
	pld.charPtr->cdef.css.CopyFrom(&creatureInst->css);

	//Restore originals from any active stat mods.
	for (size_t i = 0; i < creatureInst->baseStats.size(); i++) {
		float value = creatureInst->baseStats[i].fBaseVal;
		WriteValueToStat(creatureInst->baseStats[i].StatID, value,
				&pld.charPtr->cdef.css);
	}
	pld.charPtr->cooldownManager.CopyFrom(creatureInst->cooldownManager);
	pld.charPtr->buffManager.CopyFrom(creatureInst->buffManager);
}

void SimulatorThread::SetAccountCharacterCache(void) {
	if (pld.accPtr != NULL && pld.charPtr != NULL)
		pld.accPtr->characterCache.UpdateCharacter(pld.charPtr);
	pld.accPtr->PendingMinorUpdates++;
}

void SimulatorThread::DecrementStack(InventorySlot *slot) {
	if (slot == NULL)
		return;

	int wpos = 0;
	if (slot->GetMaxStack() > 0) {
		slot->count--;
		if (slot->count < 0) {
			wpos += RemoveItemUpdate(SendBuf, Aux3, slot);
			pld.charPtr->inventory.RemItem(slot->CCSID);
			pld.charPtr->pendingChanges++;
		} else
			wpos += AddItemUpdate(SendBuf, Aux3, slot);
	}
	if (wpos > 0)
		AttemptSend(SendBuf, wpos);
}

void SimulatorThread::RunFinishedCast(bool success) {
	if (pld.mItemUseInProgress == true) {
		pld.mItemUseInProgress = false;
		if (success == true) {
			InventorySlot *slot = pld.charPtr->inventory.GetItemPtrByCCSID(
					pld.mItemUseCCSID);
			if (slot != NULL)
				DecrementStack(slot);
		}
	}
}

bool SimulatorThread::CanMoveItems(void) {
	if (pld.mItemUseInProgress == true)
		return false;

	return true;
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

bool SimulatorThread::WaitUntilNonzero(int *data) {
	//Waits for an integer value to be changed by another thread.
	unsigned long quitTime = g_ServerTime + 5000;
	while (*data == 0) {
		PLATFORM_SLEEP(1);
		if (g_ServerTime >= quitTime)
			break;
	};
	if (data == 0)
		return false;

	return true;
}

int SimulatorThread::ResolveCreatureDef(int CreatureID) {
	int response = 0;
	creatureInst->actInst->ResolveCreatureDef(CreatureID, &response);
	return response;
}

CreatureInstance* SimulatorThread::ResolveCreatureInstance(int CreatureID,
		int searchHint) {
	if (creatureInst == NULL) {
		g_Logs.simulator->error(
				"[%d] ResolveCreatureInstance creatureInst is NULL",
				InternalID);
		return NULL;
	}
	if (creatureInst->actInst == NULL) {
		g_Logs.simulator->error("[%v] ResolveCreatureInstance actInst is NULL",
				InternalID);
		return NULL;
	}
	switch (searchHint) {
	case 1:
		return creatureInst->actInst->GetNPCInstanceByCID(CreatureID);

	case 0:  //No hints is same as default
	default:
		return creatureInst->actInst->inspectCreature(CreatureID);
	}

	return NULL;
}

int SimulatorThread::CheckValidHengeDestination(const char *destName,
		int creatureID) {
	if (pld.zoneDef == NULL || creatureInst == NULL)
		return false;

	if (creatureInst->actInst == NULL)
		return false;

	if (creatureInst->Speed != 0) {
		SendInfoMessage("You must be stationary.", INFOMSG_ERROR);
		return false;
	}
	if (creatureInst->HasStatus(StatusEffects::DEAD)) {
		SendInfoMessage("You must be alive.", INFOMSG_ERROR);
		return false;
	}

	if (creatureID != ZoneDefManager::HENGE_ID_CUSTOMWARP) {
		//We've clicked a henge object.  The target zone must exist in the henge interact list.
		CreatureInstance *target = creatureInst->actInst->inspectCreature(
				creatureID);
		if (target == NULL) {
			SendInfoMessage("Could not find object.", INFOMSG_ERROR);
			return false;
		}
		if (target->HasStatus(StatusEffects::HENGE) == false) {
			SendInfoMessage("Object is not a henge.", INFOMSG_ERROR);
			return false;
		}
		if (creatureInst->IsObjectInRange(target, INTERACT_RANGE) == false) {
			SendInfoMessage("Out of range.", INFOMSG_ERROR);
			return false;
		}
		if (g_InteractObjectContainer.GetHengeByTargetName(destName) == NULL) {
			SendInfoMessage("There is no henge with that name.", INFOMSG_ERROR);
			return false;
		}
		if (pld.zoneDef->IsFreeTravel() == true) {
			SendInfoMessage("You cannot use henges in groves or arenas.",
					INFOMSG_ERROR);
			return false;
		}
	} else {
		//We're using the henge screen as a makeshift destination selector like from /grove or /pvp.
		//The target zone must be a grove or arena, or an exit command for those respective areas.

		//Quick check to make sure the exit commands are not used outside their respective zones.
		if (strcmp(destName, EXIT_GROVE) == 0) {
			if (pld.zoneDef->mGrove == false) {
				SendInfoMessage("You are not in a grove.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}
		if (strcmp(destName, EXIT_GUILD_HALL) == 0) {
			if (pld.zoneDef->mGuildHall == false) {
				SendInfoMessage("You are not in a guild hall.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}
		if (strcmp(destName, EXIT_PVP) == 0) {
			if (pld.zoneDef->mArena == false) {
				SendInfoMessage("You are not in a PvP arena.", INFOMSG_ERROR);
				return false;
			}
			return true;
		}

		//If we're not in a grove or arena, make sure we're near a sanctuary.
		if (pld.zoneDef->IsFreeTravel() == false) {
			WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(
					pld.zoneDef->mID, creatureInst->CurrentX,
					creatureInst->CurrentZ, SANCTUARY_PROXIMITY_USE);
			if (sanct == NULL) {
				SendInfoMessage("You must be within range of a sanctuary.",
						INFOMSG_ERROR);
				return false;
			}
		}

		ZoneDefInfo *destZone = g_ZoneDefManager.GetPointerByExactWarpName(
				destName);
		if (destZone == NULL)
			return false;

		//Make sure the target zone is a grove or arena.
		if (destZone->IsFreeTravel() == false)
			return false;

		//Certain groves may disallow player warps, so check that here too.
		if (CheckValidWarpZone(destZone->mID) != ERROR_NONE)
			return false;
	}

	//If we get here, no error conditions have bailed out.
	return true;
}

/* DISABLED, NEVER FINISHED
 void SimulatorThread :: CheckQuestItems(void)
 {
 pld.charPtr->questJournal.CheckInventoryItems(pld.charPtr->inventory);
 }
 */

int SimulatorThread::CheckDistance(int creatureID) {
	//Could only be called from within a guarded thread operation.
	if (creatureInst == NULL)
		return QueryErrorMsg::GENERIC;
	if (creatureInst->actInst == NULL)
		return QueryErrorMsg::GENERIC;

	CreatureInstance *object = creatureInst->actInst->GetNPCInstanceByCID(
			creatureID);
	if (object == NULL)
		return QueryErrorMsg::INVALIDOBJ;
	int dist = creatureInst->actInst->GetBoxRange(creatureInst, object);
	if (dist > INTERACT_RANGE)
		return QueryErrorMsg::OUTOFRANGE;

	return 0;
}

void SimulatorThread::Debug_GenerateReport(ReportBuffer *report) {
	report->AddLine("Sim:%d", InternalIndex);
	if (creatureInst != NULL)
		report->AddLine("display_name=%s", creatureInst->css.display_name);
	if (pld.accPtr != NULL) {
		report->AddLine("AccountID=%d", pld.accPtr->ID);
		report->AddLine("uid=%d", pld.accPtr->SiteSession.uid);
		report->AddLine("unreadMessages=%d",
				pld.accPtr->SiteSession.unreadMessages);
		report->AddLine("XCSRF=%s", pld.accPtr->SiteSession.xCSRF.c_str());
		report->AddLine("SessionName=%s",
				pld.accPtr->SiteSession.sessionName.c_str());
		report->AddLine("SessionID=%s",
				pld.accPtr->SiteSession.sessionID.c_str());
	}
	if (pld.charPtr != NULL) {
		int sec = ((g_ServerTime - TimeLastAutoSave) / 1000)
				+ pld.charPtr->SecondsLogged;
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
	report->AddLine("LogBuffer [%d,%d]=%s",
			Util::IsStringTerminated(LogBuffer, sizeof(LogBuffer)),
			strlen(LogBuffer), LogBuffer);
	report->AddLine("RecBuf [%d,%d]=%s",
			Util::IsStringTerminated(RecBuf, sizeof(RecBuf)), strlen(RecBuf),
			RecBuf);
	report->AddLine("SendBuf [%d,%d]=%s",
			Util::IsStringTerminated(SendBuf, sizeof(SendBuf)), strlen(SendBuf),
			SendBuf);
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
	report->AddLine("Aux1 [%d,%d]=%s",
			Util::IsStringTerminated(Aux1, sizeof(Aux1)), strlen(Aux1), Aux1);
	report->AddLine("Aux2 [%d,%d]=%s",
			Util::IsStringTerminated(Aux2, sizeof(Aux2)), strlen(Aux2), Aux2);
	report->AddLine("Aux3 [%d,%d]=%s",
			Util::IsStringTerminated(Aux3, sizeof(Aux3)), strlen(Aux3), Aux3);
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
	report->AddLine("pld.LastCheckDistanceMoved=%ld",
			pld.LastCheckDistanceMoved);
	report->AddLine("pld.NextIdleCheckTime=%lu", pld.NextIdleCheckTime);
	report->AddLine("pld.LastCastX=%d", pld.LastCastX);
	report->AddLine("pld.LastCastZ=%d", pld.LastCastZ);
	report->AddLine("pld.IdleCastCount=%d", pld.IdleCastCount);
	report->AddLine("pld.CurrentMapInt=%d", pld.CurrentMapInt);
	report->AddLine("pld.LastMapTick=%d", pld.LastMapTick);

	report->AddLine("pld.DebugPingServerLastMsgReceived=%d",
			pld.DebugPingServerLastMsgReceived);
	report->AddLine("pld.DebugPingServerTotalMsgReceived=%d",
			pld.DebugPingServerTotalMsgReceived);
	report->AddLine("pld.DebugPingServerNotifyTime=%d",
			pld.DebugPingServerNotifyTime);
	report->AddLine("pld.DebugPingServerLowest=%d", pld.DebugPingServerLowest);
	report->AddLine("pld.DebugPingServerHighest=%d",
			pld.DebugPingServerHighest);
	report->AddLine("pld.DebugPingServerTotalTime=%d",
			pld.DebugPingServerTotalTime);
	report->AddLine("pld.DebugPingServerTotalReceived=%d",
			pld.DebugPingServerTotalReceived);
	report->AddLine("pld.DebugPingServerSent=%d", pld.DebugPingServerSent);

	report->AddLine("creatureInst=%p", creatureInst);
	if (creatureInst != NULL)
		report->AddLine("creatureInst->css.display_name=%s",
				creatureInst->css.display_name);
	report->AddLine("defcInst=%p", &defcInst);
	report->AddLine("TimeOnline=%ld", TimeOnline);
	report->AddLine("query.ID=%d", query.ID);
	report->AddLine("query.name=%s", query.name.c_str());
	report->AddLine("query.argCount=%d", query.argCount);
	for (size_t i = 0; i < query.args.size(); i++)
		report->AddLine("query.args[%d]=%s", i, query.args[i].c_str());

	report->AddLine(NULL);
}

void Debug_GenerateSimulatorReports(ReportBuffer *report) {
	report->AddLine("Simulator pointers:");
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		report->AddLine("Sim:%d [%p]", it->InternalID, &*it);
	report->AddLine(NULL);

	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		it->sim_cs.Enter("Debug_GenerateSimulatorReports");
		it->Debug_GenerateReport(report);
		it->sim_cs.Leave();
	}
}

void SimulatorThread::CheckSpawnTileUpdate(bool force) {
	int tx = creatureInst->CurrentX / SpawnTile::SPAWN_TILE_SIZE;
	int tz = creatureInst->CurrentZ / SpawnTile::SPAWN_TILE_SIZE;
	if ((tx != pld.oldSpawnX || tz != pld.oldSpawnZ) || (force == true)) {
		unsigned long startTime = g_PlatformTime.getMilliseconds();

		pld.oldSpawnX = tx;
		pld.oldSpawnZ = tz;
		//int scount = 0;

		creatureInst->actInst->spawnsys.GenerateAreaTile(tx, tz);
		creatureInst->actInst->spawnsys.RunProcessing(true);

		int wpos = 0;
#ifdef CREATUREQAV
		for (size_t i = 0; i < creatureInst->actInst->NPCListPtr.size(); i++) {
			CreatureInstance *cInst = creatureInst->actInst->NPCListPtr[i];
#else
			ActiveInstance::CREATURE_IT it;
			for(it = creatureInst->actInst->NPCList.begin(); it != creatureInst->actInst->NPCList.end(); ++it)
			{
				CreatureInstance *cInst = &it->second;
#endif
			if (cInst->CurrentX
					< (creatureInst->CurrentX - g_CreatureListenRange))
				continue;
			if (cInst->CurrentX
					> (creatureInst->CurrentX + g_CreatureListenRange))
				continue;
			if (cInst->CurrentZ
					< (creatureInst->CurrentZ - g_CreatureListenRange))
				continue;
			if (cInst->CurrentZ
					> (creatureInst->CurrentZ + g_CreatureListenRange))
				continue;
			wpos += PrepExt_GeneralMoveUpdate(&SendBuf[wpos], cInst);
			CheckWriteFlush(wpos);
			//scount++;
		}
		if (wpos > 0)
			AttemptSend(SendBuf, wpos);

		unsigned long timeDif = g_PlatformTime.getMilliseconds() - startTime;
		if (timeDif > 50)
			g_Logs.simulator->debug(
					"[%v] CheckSpawnTileUpdate complete in %v ms, zone:%v",
					InternalID, timeDif, pld.CurrentZoneID);
		//LogMessageL(MSG_SHOW, "[DEBUG] Updating %d creatures", scount);
	}
}

void SimulatorThread::WarpToZone(ZoneDefInfo *zoneDef, int xOverride,
		int yOverride, int zOverride) {
	if (zoneDef == NULL)
		return;

	int targX = zoneDef->DefX;
	int targY = zoneDef->DefY;
	int targZ = zoneDef->DefZ;

	//Hack for Arena too, semantics are the same.
	if (zoneDef->mGrove == true || zoneDef->mArena == true) {
		if (pld.zoneDef->mGrove == false && pld.zoneDef->mArena == false) {
			//We're warping from a non-grove to grove.

			//Make sure we're in range of a sanctuary.
			WorldCoord* sanct = g_ZoneMarkerDataManager.GetSanctuaryInRange(
					pld.zoneDef->mID, creatureInst->CurrentX,
					creatureInst->CurrentZ, SANCTUARY_PROXIMITY_USE);
			if (sanct == NULL) {
				SendInfoMessage("You must be within range of a sanctuary.",
						INFOMSG_ERROR);
				return;
			}
			//Need to save a special set of return coordinates so the player can
			//return to this location.
			pld.charPtr->groveReturnPoint[0] = (int) sanct->x; //creatureInst->CurrentX;
			pld.charPtr->groveReturnPoint[1] = (int) sanct->y
					+ SANCTUARY_ELEVATION_ADDITIVE; //creatureInst->CurrentY;
			pld.charPtr->groveReturnPoint[2] = (int) sanct->z; //creatureInst->CurrentZ;
			pld.charPtr->groveReturnPoint[3] = pld.zoneDef->mID;
		}
	} else {
		if (pld.zoneDef->mGrove == true || pld.zoneDef->mArena == true) {
			//Warping from a grove back to normal territory.
			if (pld.charPtr->groveReturnPoint[0] != 0
					&& pld.charPtr->groveReturnPoint[2] != 0) {
				targX = pld.charPtr->groveReturnPoint[0];
				targY = pld.charPtr->groveReturnPoint[1];
				targZ = pld.charPtr->groveReturnPoint[2];
			}
		}
	}

	SetPosition(creatureInst->CurrentX, creatureInst->CurrentY,
			creatureInst->CurrentZ, 1);

	MainCallSetZone(zoneDef->mID, 0, false);
	if (ValidPointers() == false) {
		ForceErrorMessage("Critical error while changing zones.",
				INFOMSG_ERROR);
		Disconnect("SimulatorThread::WarpToZone");
		return;
	}

	if (xOverride != 0) {
		targX = xOverride;
	}
	if (yOverride != 0) {
		targY = yOverride;
	}
	if (zOverride != 0) {
		targZ = zOverride;
	}

	SetPosition(targX, targY, targZ, 1);
	//SendInfoMessage("Changing zone.", INFOMSG_INFO);
	CheckSpawnTileUpdate(true);
	CheckMapUpdate(true);
}

void SimulatorThread::SetZoneMode(int mode) {
	creatureInst->actInst->mZoneDefPtr->mMode = mode;
	creatureInst->actInst->arenaRuleset.mEnabled =
			creatureInst->actInst->mZoneDefPtr->mMode
					!= PVP::GameMode::PVE_ONLY;
	creatureInst->actInst->arenaRuleset.mPVPStatus = mode;

	cs.Enter("Simulator::SetZoneMode");

	switch (mode) {
	case PVP::GameMode::PVP:
		for (std::vector<CreatureInstance*>::iterator it =
				creatureInst->actInst->PlayerListPtr.begin();
				it != creatureInst->actInst->PlayerListPtr.end(); ++it) {
			CreatureInstance *c = *it;
			if (c->charPtr->Mode == PVP::GameMode::PVP &&
				c->_HasStatusList(StatusEffects::PVPABLE) == -1)
				c->_AddStatusList(StatusEffects::PVPABLE, -1);
		}
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s is now a PVP zone",
				creatureInst->actInst->mZoneDefPtr->mName.c_str());
		break;
	case PVP::GameMode::PVP_ONLY:
		for (std::vector<CreatureInstance*>::iterator it =
				creatureInst->actInst->PlayerListPtr.begin();
				it != creatureInst->actInst->PlayerListPtr.end(); ++it) {
			CreatureInstance *c = *it;
			if (c->_HasStatusList(StatusEffects::PVPABLE) == -1)
				c->_AddStatusList(StatusEffects::PVPABLE, -1);
		}
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s is now a PVP only zone",
				creatureInst->actInst->mZoneDefPtr->mName.c_str());
		break;
	case PVP::GameMode::PVE_ONLY:
		for (std::vector<CreatureInstance*>::iterator it =
				creatureInst->actInst->PlayerListPtr.begin();
				it != creatureInst->actInst->PlayerListPtr.end(); ++it) {
			CreatureInstance *c = *it;
			if (c->_HasStatusList(StatusEffects::PVPABLE) != -1)
				c->_RemoveStatusList(StatusEffects::PVPABLE);
		}
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s is now a PVE only zone",
				creatureInst->actInst->mZoneDefPtr->mName.c_str());
		break;
	case PVP::GameMode::SPECIAL_EVENT:
		for (std::vector<CreatureInstance*>::iterator it =
				creatureInst->actInst->PlayerListPtr.begin();
				it != creatureInst->actInst->PlayerListPtr.end(); ++it) {
			CreatureInstance *c = *it;
			if (c->_HasStatusList(StatusEffects::PVPABLE) == -1)
				c->_AddStatusList(StatusEffects::PVPABLE, -1);
		}
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s is now a special event zone!",
				creatureInst->actInst->mZoneDefPtr->mName.c_str());
		break;
	default:
		for (std::vector<CreatureInstance*>::iterator it =
				creatureInst->actInst->PlayerListPtr.begin();
				it != creatureInst->actInst->PlayerListPtr.end(); ++it) {
			CreatureInstance *c = *it;
			if (c->charPtr->Mode == PVP::GameMode::PVE &&
				c->_HasStatusList(StatusEffects::PVPABLE) != -1)
				c->_RemoveStatusList(StatusEffects::PVPABLE);
		}
		Util::SafeFormat(Aux1, sizeof(Aux1), "%s is now a PVE zone",
				creatureInst->actInst->mZoneDefPtr->mName.c_str());
		break;
	}

	cs.Leave();

	g_SimulatorManager.BroadcastMessage(Aux1);
}

//
// Helper functions
//
bool SimulatorThread::QuestInvite(int QuestID) {

	if (creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;

	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
//	ActiveParty *party =
//			creatureInst->PartyID > 0 ?
//					g_PartyManager.GetPartyByID(creatureInst->PartyID) : NULL;
	if (qdef != NULL) {
		creatureInst->charPtr->BuildAvailableQuests(QuestDef);

		char SendBuf[256];
		int wpos = PartyManager::WriteQuestInvite(SendBuf, qdef->title.c_str(),
				qdef->questID);
		SIMULATOR_IT it;
		int res = creatureInst->charPtr->questJournal.CheckQuestShare(QuestID);
		if (res == QuestJournal::SHARE_SUCCESS_QUALIFIES) {
			creatureInst->simulatorPtr->AttemptSend(SendBuf, wpos);
		}
//		if(party != NULL) {
//			for(it = Simulator.begin(); it != Simulator.end(); ++it) {
//				if(it->InternalID == source->simulatorPtr->InternalID)
//					continue;
//				if(it->creatureInst->PartyID != party->mPartyID)
//					continue;
//				if(party->HasMember(it->creatureInst->CreatureDefID) == false)
//					continue;
//				if(it->pld.charPtr->questJournal.CheckQuestShare(questID) == QuestJournal::SHARE_SUCCESS_QUALIFIES) {
//					it->AttemptSend(SendBuf, wpos);
//				}
//			}
//		}
		return true;
	}
	return false;
}

bool SimulatorThread::QuestJoin(int QuestID) {
	if (creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;
	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if (qdef == NULL)
		return false;
	if (qdef->mScriptAcceptCondition.ExecuteAllCommands(this) < 0)
		return false;
	qdef->mScriptAcceptAction.ExecuteAllCommands(this);
	int wpos = creatureInst->charPtr->questJournal.QuestJoin(&Aux1[0], QuestID,
			query.ID);
	g_QuestNutManager.AddActiveScript(creatureInst, QuestID);
	AttemptSend(Aux1, wpos);
	return true;
}

bool SimulatorThread::QuestResetObjectives(int QuestID, int objective) {
	if (creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;
	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if (qdef == NULL)
		return false;
	creatureInst->charPtr->questJournal.QuestResetObjectives(
			creatureInst->CreatureID, QuestID);
	int wpos = PrepExt_QuestStatusMessage(Aux1, QuestID, objective, false,
			qdef->actList[creatureInst->charPtr->questJournal.GetCurrentAct(
					QuestID)].objective[objective].completeText);
	AttemptSend(Aux1, wpos);
	return true;
}

bool SimulatorThread::QuestClear(int QuestID) {
	if (creatureInst == NULL || creatureInst->charPtr == NULL)
		return false;
	creatureInst->charPtr->questJournal.QuestClear(creatureInst->CreatureID,
			QuestID);
	int wpos = PutByte(&Aux1[0], 7);  //_handleQuestEventMsg
	wpos += PutShort(&Aux1[wpos], 0); //Size
	wpos += PutInteger(&Aux1[wpos], QuestID); //Quest ID
	wpos += PutByte(&Aux1[wpos], QuestObjective::EVENTMSG_ABANDONED);
	PutShort(&Aux1[1], wpos - 3);
	AttemptSend(Aux1, wpos);
	return true;
}

int SimulatorThread::OfferLoot(int mode, ActiveLootContainer *loot,
		ActiveParty *party, CreatureInstance *receivingCreature, int ItemID,
		bool needOrGreed, int CID, int conIndex) {
	int WriteIdx = 0;
	if (mode == ROUND_ROBIN) {
		// Offer to the robin first
		g_Logs.simulator->info("[%v] Offer Loot Round Robin", InternalID);
		PartyMember *robin = party->GetNextLooter();

		loot->robinID = robin->mCreatureID;

		// Offer to the robin first
		LootTag tag = party->TagItem(ItemID, robin->mCreaturePtr->CreatureID,
				CID, 0);
//		int slot = robin->mCreaturePtr->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
//		tag->mSlotIndex = slot;
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag.lootTag);
		WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3, false);
		if (receivingCreature->CreatureID == robin->mCreaturePtr->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Round Robin to looter, so returning with this response",
					InternalID);
			return WriteIdx;
		} else {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Round Robin to someone other than looter, so returning on its simulator",
					InternalID);

//			ItemDef *item = g_ItemManager.GetSafePointerByID(tag->mItemId);
//			WritePos = PrepExt_ItemDef(SendBuf, item, ProtocolState);

			robin->mCreaturePtr->actInst->LSendToOneSimulator(
					&SendBuf[WritePos], WriteIdx,
					robin->mCreaturePtr->simulatorPtr);
			return QueryErrorMsg::NONE;
		}
	}

	if (mode == LOOT_MASTER) {
		g_Logs.simulator->info("[%v] Offer Loot Master", InternalID);
		// Offer to the leader first
		PartyMember *member = party->GetMemberByID(party->mLeaderID);
		unsigned int idx = 0;
		while ((member == NULL || !member->IsOnlineAndValid())
				&& idx < party->mMemberList.size()) {
			member = &party->mMemberList[idx++];
		}
		if (member == NULL || !member->IsOnlineAndValid()) {
			/* Nobody left! */
			return QueryErrorMsg::GENERIC;
		}
		int slot = member->mCreaturePtr->charPtr->inventory.GetFreeSlot(
				INV_CONTAINER);
		LootTag tag = party->TagItem(ItemID, member->mCreaturePtr->CreatureID,
				CID, slot);
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag.lootTag);
		WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3, false);

		if (receivingCreature->CreatureID == member->mCreaturePtr->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Master to looter, so returning with this response",
					InternalID);
			return WriteIdx;
		} else {
			g_Logs.simulator->info(
					"[%v] Sending Offer Loot Master to someone other than looter, so returning on its simulator",
					InternalID);
			member->mCreaturePtr->actInst->LSendToOneSimulator(SendBuf,
					WriteIdx, member->mCreaturePtr->simulatorPtr);
			return QueryErrorMsg::NONE;
		}
	}

	// First tags for other party members
	g_Logs.simulator->info("[%v] Offering to %v party member", InternalID,
			party->mMemberList.size());
	int offers = 0;
	for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
		if (!party->mMemberList[i].IsOnlineAndValid()) {
			g_Logs.simulator->info(
					"[%v] Skipping party member %v because they have no simulator",
					InternalID, party->mMemberList[i].mCreatureID);
			continue;
		}

		if (receivingCreature == NULL
				|| party->mMemberList[i].mCreatureID
						!= receivingCreature->CreatureID) {
			// Only send offer to players in range
			int distCheck = protected_CheckDistanceBetweenCreaturesFor(
					party->mMemberList[i].mCreaturePtr, CID, PARTY_LOOT_RANGE);
			if (distCheck == 0) {
				LootTag tag = party->TagItem(ItemID,
						party->mMemberList[i].mCreaturePtr->CreatureID, CID, 0);
				Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag.lootTag);
				g_Logs.simulator->info("Sending offer of %v to %v using tag %v",
						ItemID, party->mMemberList[i].mCreatureID, Aux3);
				WriteIdx = PartyManager::OfferLoot(SendBuf, ItemID, Aux3,
						needOrGreed);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(
						SendBuf, WriteIdx,
						party->mMemberList[i].mCreaturePtr->simulatorPtr);
				offers++;
			} else {
				g_Logs.event->info(
						"%v is too far away from %d to receive loot (%v)",
						party->mMemberList[i].mCreaturePtr->CreatureID, CID,
						distCheck);
			}
		}
	}

	// Now the tag for the looting creature. We send the slot with this one
	if (mode > -1 && receivingCreature != NULL) {
		g_Logs.simulator->info("[%v] Offering loot to looter", InternalID);
		int slot = receivingCreature->charPtr->inventory.GetFreeSlot(
				INV_CONTAINER);
		LootTag tag = party->TagItem(ItemID, receivingCreature->CreatureID, CID,
				slot);
		offers++;

//		STRINGLIST qresponse;
//		qresponse.push_back("OK");
//		sprintf(Aux3, "%d", conIndex);
//		qresponse.push_back(Aux3);
//		WriteIdx= PrepExt_QueryResponseStringList(&SendBuf[WriteIdx], query.ID, qresponse);
		Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag.lootTag);
		g_Logs.simulator->info(
				"Sending offer of %v to original looter (%v) using tag %v",
				ItemID, receivingCreature->CreatureID, Aux3);
		return PartyManager::OfferLoot(SendBuf, ItemID, Aux3, needOrGreed);
	}

	if (offers == 0) {
		return QueryErrorMsg::GENERIC;
	} else {
		return QueryErrorMsg::NONE;
	}
}

void SimulatorThread::RunTranslocate(void) {
	if (pld.charPtr->bindReturnPoint[3] == 0) {
		int wpos = PrepExt_SendInfoMessage(GSendBuf, "Cannot translocate.",
				INFOMSG_ERROR);
		AttemptSend(GSendBuf, wpos);
	} else {
		MainCallSetZone(pld.charPtr->bindReturnPoint[3], 0, false);
		SetPosition(pld.charPtr->bindReturnPoint[0],
				pld.charPtr->bindReturnPoint[1],
				pld.charPtr->bindReturnPoint[2], 1);
	}
}

void SimulatorThread::RunPortalRequest(void) {
	if (strlen(pld.PortalRequestDest) == 0) {
		SendInfoMessage("You do not have a portal destination established.",
				INFOMSG_ERROR);
		return;
	}

	int zone = 0;
	int x;
	int y;
	int z;

	if (pld.PortalRequestType == 0) {

		InteractObject *iobj = g_InteractObjectContainer.GetHengeByTargetName(
				pld.PortalRequestDest);
		if (iobj == NULL) {
			g_Logs.server->warn("Portal request target not found: %v",
					pld.PortalRequestDest);
			SendInfoMessage(Aux1, INFOMSG_ERROR);
			return;
		}

		zone = iobj->WarpID;
		x = iobj->WarpX;
		y = iobj->WarpY;
		z = iobj->WarpZ;
	} else if (pld.PortalRequestType == 1) {
		SimulatorThread *sim = GetSimulatorByCharacterName(
				pld.PortalRequestDest);
		if (sim == NULL || sim->LoadStage != LOADSTAGE_GAMEPLAY) {
			g_Logs.server->warn("Portal request target not found: %v",
					pld.PortalRequestDest);
			SendInfoMessage(Aux1, INFOMSG_ERROR);
			return;
		}
		x = sim->creatureInst->CurrentX + g_RandomManager.RandModRng(5, 20);
		y = sim->creatureInst->CurrentY;
		z = sim->creatureInst->CurrentZ + g_RandomManager.RandModRng(5, 20);
		zone = sim->creatureInst->actInst->mZone;
	} else {
		g_Logs.server->warn("Portal request type not found: %v (%v)",
				pld.PortalRequestDest, pld.PortalRequestType);
		SendInfoMessage(Aux1, INFOMSG_ERROR);
		return;
	}

	int wpos = PrepExt_RemoveCreature(SendBuf, creatureInst->CreatureID);
	creatureInst->actInst->LSendToLocalSimulator(SendBuf, wpos,
			creatureInst->CurrentX, creatureInst->CurrentZ, InternalID);

	MainCallSetZone(zone, 0, false);
	SetPosition(x, y, z, 1);
	CheckSpawnTileUpdate(true);
	CheckMapUpdate(true);
	pld.ClearPortalRequestDest();
}

void SimulatorThread::JoinPrivateChannel(const char *channelname,
		const char *password) {
	char strBuf[100];
	std::string returnData;
	int res = g_ChatChannelManager.JoinChannel(InternalID,
			creatureInst->css.display_name, channelname, password, returnData);
	bool success = false;
	switch (res) {
	case ChatChannelManager::RESULT_ALREADY_IN_CHANNEL:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"You must leave [%s] before joining another channel.",
				returnData.c_str());
		break;
	case ChatChannelManager::RESULT_PASSWORD_REQUIRED:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"Channel [%s] requires a password.", channelname);
		break;
	case ChatChannelManager::RESULT_CHANNEL_FAILED:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"Unable to create channel [%s]", channelname);
		break;
	case ChatChannelManager::RESULT_CHANNEL_CREATED:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"You have created channel [%s].", channelname);
		success = true;
		break;
	case ChatChannelManager::RESULT_JOIN_SUCCESS:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"You have joined channel [%s].", channelname);
		success = true;
		break;
	case ChatChannelManager::RESULT_BAD_CHANNEL:
		Util::SafeFormat(strBuf, sizeof(strBuf), "No channel name provided.");
		break;
	case ChatChannelManager::RESULT_CHANNEL_NAME_SIZE:
		Util::SafeFormat(strBuf, sizeof(strBuf),
				"The channel name must not exceed %d characters.",
				ChatChannelManager::MAX_CHANNEL_NAME_SIZE);
		break;
	case ChatChannelManager::RESULT_UNHANDLED:
	default:
		Util::SafeFormat(strBuf, sizeof(strBuf), "Unhandled error");
		break;
	}
	int msgType = INFOMSG_ERROR;
	if (success == true) {
		pld.charPtr->SetLastChannel(channelname, password);
		msgType = INFOMSG_INFO;
	}

	int wpos = PrepExt_SendInfoMessage(Aux3, strBuf, msgType);
	AttemptSend(Aux3, wpos);
}

void SimulatorThread::CreatureUseHenge(int creatureID, int creatureDefID) {
	//Helper function to perform the actions when activating a Henge object.
	if (pld.charPtr->HengeHas(creatureDefID) == false) {
		InteractObject *iobj = NULL;
		iobj = g_InteractObjectContainer.GetHengeByDefID(creatureDefID);
		if (iobj == NULL) {
			g_Logs.simulator->warn("Henge not found in interact list: %v",
					creatureDefID);
			return;
		}

		Util::SafeFormat(Aux1, sizeof(Aux1), "You have discovered %s",
				iobj->useMessage);
		SendInfoMessage(Aux1, INFOMSG_INFO);
		pld.charPtr->HengeAdd(creatureDefID);
	}

	std::vector<InteractObject*> objectSearch;
	for (size_t i = 0; i < pld.charPtr->hengeList.size(); i++) {
		InteractObject *iobj = NULL;
		iobj = g_InteractObjectContainer.GetHengeByDefID(
				pld.charPtr->hengeList[i]);
		if (iobj != NULL)
			objectSearch.push_back(iobj);
	}

	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 4);   //_handleCreatureEventMsg
	wpos += PutShort(&SendBuf[wpos], 0);  //size

	wpos += PutInteger(&SendBuf[wpos], creatureInst->CreatureID);
	wpos += PutByte(&SendBuf[wpos], 14);  //Event for henge click

	wpos += PutInteger(&SendBuf[wpos], creatureID);
	wpos += PutByte(&SendBuf[wpos], objectSearch.size());

	for (size_t i = 0; i < objectSearch.size(); i++) {
		//Henge Name, Cost
		wpos += PutStringUTF(&SendBuf[wpos], objectSearch[i]->useMessage);
		wpos += PutInteger(&SendBuf[wpos], objectSearch[i]->cost);
	}
	PutShort(&SendBuf[1], wpos - 3);  //size

	AttemptSend(SendBuf, wpos);
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

void SimulatorThread::FinaliseTransfer(Shard &shard, const std::string &token) {
	g_Logs.simulator->info("Finalising transfer of %v to %v (%v)",
			pld.charPtr->cdef.css.display_name, shard.mName, shard.mHTTPAddress);
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 56);  //_handleSimSwitch
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutStringUTF(&SendBuf[wpos], shard.mSimulatorAddress);
	wpos += PutInteger(&SendBuf[wpos], shard.mSimulatorPort);
	wpos += PutStringUTF(&SendBuf[wpos], token);
	wpos += PutStringUTF(&SendBuf[wpos], pld.charPtr->cdef.css.display_name);
	wpos += PutStringUTF(&SendBuf[wpos], shard.mHTTPAddress);
	PutShort(&SendBuf[1], wpos - 3);
	AttemptSend(SendBuf, wpos);

	/* Make sure this and any other packets are delivered immediately. Upon receiving this
	 * the client will do the actual disconnection, then reconnection to the new simulator
	 */
	g_PacketManager.SendPacketsFor(this->sc.ClientSocket);
	ProcessDetach();
}

std::string SimulatorThread::ShardSet(std::string shardName,
		std::string charName) {
	if (creatureInst->Speed != 0) {
		return "You must be stationary.";
	}

	Shard s = g_ClusterManager.GetActiveShard(shardName);
	if (s.mName.length() == 0) {
		return "No such shard.";
	}

	g_Logs.data->info("%v is switching to shard %v",
			pld.charPtr->cdef.css.display_name, shardName);

	unsigned warpAllowed =
			pld.charPtr->LastWarpTime == 0 ?
					0 : pld.charPtr->LastWarpTime + 90000;
	if (g_ServerTime < warpAllowed) {
		return StringUtil::Format("You can't do that yet. Remaining time: %s",
				StringUtil::FormatTimeHHMMSS(warpAllowed - g_ServerTime).c_str());
	}

	/* Flush to characters and the account now so that the new Sim has most up-to-date data.
	 * This must be a synchronous save */
	SaveCharacterStats();
	g_CharacterManager.SaveCharacter(pld.CreatureDefID, true);
	g_AccountManager.SaveIndividualAccount(pld.accPtr, true);


	if (charName.length() > 0) {
//		SimulatorThread * sim == GetSimulatorByCharacterName(charName);
//		if (sim == NULL) {
//			SendInfoMessage("That player is not logged in.", INFOMSG_ERROR);
//			return;
//		}
//		if (pld.zoneDef->mInstance == true) {
//			SendInfoMessage("You cannot warp to a player inside an instance.",
//					INFOMSG_ERROR);
//			return;
//		}
//		int x = creatureInst->CurrentX;
//		int y = creatureInst->CurrentY;
//		int z = creatureInst->CurrentZ;
//		if (pld.zoneDef->mID != pld.zoneDef->mID) {
//			if (WarpToZone(pld.zoneDef, false) == false)
//				return;
//		}
//		SetPosition(x, y, z, 1);
//
//		pld.charPtr->LastWarpTime = time;
	} else {

	}
	g_ClusterManager.SimTransfer(pld.CreatureDefID, shardName, InternalID);
	return "";
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

void SimulatorThread::VerifyGenericBuffer(const char *buffer,
		unsigned int buflen) {
	//Note: SendBuf does not work if called from any of the instance batch send (LSendTo___)  functions
	//since it will likely use a different simulator's buffer at a different address.
	if (buffer == SendBuf) {
		if (buflen >= sizeof(SendBuf) - 2400) {
			g_Logs.simulator->warn(
					"[%d] AttemptSend() dangerous data length in SendBuf, %d bytes (max: %d)",
					InternalID, buflen, sizeof(SendBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	} else if (buffer == GSendBuf) {
		if (buflen >= sizeof(GSendBuf) - 3200) {
			g_Logs.simulator->warn(
					"[%d] AttemptSend() dangerous data length in GSendBuf, %d bytes (max: %d)",
					InternalID, buflen, sizeof(GSendBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	} else if (buffer == GAuxBuf) {
		if (buflen >= sizeof(GAuxBuf) - 100) {
			g_Logs.simulator->warn(
					"[%d] AttemptSend() dangerous data length in GAuxBuf, %d bytes (max: %d)",
					InternalID, buflen, sizeof(GAuxBuf));
			fprintf(stderr, "%s", LogBuffer);
		}
	} else if (buffer == Aux1) {
		if (buflen >= sizeof(Aux1) - 400) {
			g_Logs.simulator->warn(
					"[%d] AttemptSend() dangerous data length in Aux1, %d bytes (max: %d)",
					InternalID, buflen, sizeof(Aux1));
			fprintf(stderr, "%s", LogBuffer);
		}
	}
}

bool SimulatorThread::TargetRarityAboveNormal(void) {
	CreatureInstance *target = creatureInst->CurrentTarget.targ;
	if (target == NULL)
		return false;

	if (!(target->serverFlags & ServerFlags::IsNPC))
		return false;

	if (target->css.rarity >= CreatureRarityType::HEROIC)
		return true;

	return false;
}

int SimulatorThread::ResolveEmoteTarget(int target) {
	//If target == 0, it must resolve to the player
	//If target == 1, it must resolve to a sidekick (search and return first one)
	//If target == ID, it must resolve to a specific sidekick ID (search and return exact one)
	//If a sidekick search fails, default to player.

	if (target == 0)
		return creatureInst->CreatureID;

	//The search needs to be zero for arbitrary sidekicks (returns the first sidekick for that player)
	if (target == 1)
		target = 0;

	CreatureInstance *sk = creatureInst->actInst->GetMatchingSidekick(
			creatureInst, target);
	if (sk != NULL)
		return sk->CreatureID;

	return creatureInst->CreatureID;
}

void SimulatorThread::SendAbilityErrorMessage(int abilityErrorCode) {
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
	switch (abilityErrorCode) {
	case Ability2::ABILITY_NOT_FOUND:
		messageStr = "Server error: ability does not exist.";
		priority = 3;
		break;
	case Ability2::ABILITY_BAD_EVENT:
		messageStr = "Server error: ability event does not exist.";
		priority = 3;
		break;
	case Ability2::ABILITY_FACING:
		messageStr = "You must be facing your target.";
		break;
	case Ability2::ABILITY_INRANGE:
		messageStr = "Target is out of range.";
		break;
	case Ability2::ABILITY_NOTSILENCED:
		messageStr = "You are silenced.";
		priority = 3;
		break;
	case Ability2::ABILITY_DISARM:
		messageStr = "You are disarmed and cannot use physical attacks.";
		priority = 3;
		break;
	case Ability2::ABILITY_STUN:
		messageStr = "You are stunned.";
		priority = 3;
		break;
	case Ability2::ABILITY_DAZE:
		messageStr = "You are dazed.";
		priority = 3;
		break;
	case Ability2::ABILITY_DEAD:
		messageStr = "You are dead.";
		priority = 3;
		break;
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

	case Ability2::ABILITY_HASMAINHAND:
		messageStr = "You must have a main hand weapon equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HASOFFHAND:
		messageStr = "You must have an offhand weapon equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HASMELEE:
		messageStr = "You must have a melee weapon equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HASSHIELD:
		messageStr = "You must have a shield equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HASBOW:
		messageStr = "You must have a bow equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HAS2HORPOLE:
		messageStr = "You must have a 2-hand or pole weapon equipped.";
		priority = 2;
		break;
	case Ability2::ABILITY_HASWAND:
		messageStr = "You must have a wand equipped.";
		priority = 2;
		break;

	case Ability2::ABILITY_HEALTH_TOO_LOW:
		messageStr = "You do not have enough health.";
		break;
	case Ability2::ABILITY_BEHIND:
		messageStr = "You must be behind your target.";
		break;
	case Ability2::ABILITY_NEARBY_SANCTUARY:
		messageStr = "You are not near a sanctuary.";
		break;
	case Ability2::ABILITY_REAGENTS:
		messageStr =
				"You do not have enough reagents of the right type to use this ability.";
		break;
	}

	if (messageStr == NULL)
		return;

	if (g_ServerTime < (pld.LastAbilityErrorMessageTime + 2000)) {
		if (priority <= pld.LastAbilityErrorMessagePriority)
			return;
	}

	int size = PrepExt_SendInfoMessage(SendBuf, messageStr, INFOMSG_ERROR);
	AttemptSend(SendBuf, size);

	pld.LastAbilityErrorMessageTime = g_ServerTime;
	pld.LastAbilityErrorMessagePriority = priority;
}

bool SimulatorThread::IsGMInvisible(void) {
	return creatureInst->HasStatus(StatusEffects::GM_INVISIBLE);
}

void SimulatorThread::Debug_GenerateCreatureReport(ReportBuffer &report) {
	if (creatureInst == NULL) {
		report.AddLine("Current player pointer is NULL.");
		return;
	}
	creatureInst->DebugGenerateReport(report);

	CreatureInstance *target = creatureInst->CurrentTarget.targ;
	if (target == NULL) {
		report.AddLine("No target selected.");
	} else {
		report.AddLine(NULL);
		report.AddLine(NULL);
		report.AddLine("TARGET INFORMATION");
		target->DebugGenerateReport(report);
	}
}

void SimulatorThread::Debug_GenerateItemReport(ReportBuffer &report,
		bool simple) {
	if (pld.charPtr == NULL) {
		report.AddLine("No character data.");
		return;
	}

	int rarityCount[7] = { 0 };
	int rarityTotal[7] = { 0 };

	for (int c = 0; c < MAXCONTAINER; c++) {
		if (IsContainerIDValid(c)) {
			report.AddLine("%s", GetContainerNameFromID(c).c_str());
			for (size_t i = 0;
					i < pld.charPtr->inventory.containerList[c].size(); i++) {
				int ID = pld.charPtr->inventory.containerList[c][i].IID;
				ItemDef *item = g_ItemManager.GetPointerByID(ID);
				if (item == NULL)
					continue;
				if (item->mQualityLevel >= 0 && item->mQualityLevel < 7) {
					rarityCount[(int) item->mQualityLevel]++;
					rarityTotal[(int) item->mQualityLevel]++;
				}
				if (simple == true)
					report.AddLine("%d : %s (lev:%d, qlev:%d)", item->mID,
							item->mDisplayName.c_str(), item->mLevel,
							item->mQualityLevel);
				else
					item->Debug_WriteReport(report);
			}
			for (size_t r = 0; r < 7; r++)
				if (rarityCount[r] > 0)
					report.AddLine("qlev:%d=%d", r, rarityCount[r]);
			memset(rarityCount, 0, sizeof(rarityCount));
			report.AddLine(NULL);
		}
	}
	report.AddLine("Total rarity counts:");
	for (size_t r = 0; r < 7; r++)
		if (rarityTotal[r] > 0)
			report.AddLine("qlev:%d=%d", r, rarityTotal[r]);
	report.AddLine(NULL);
}

//Send an error message to the client and append a query success message.  This is required for certain
//official queries because the error text can be ugly.  "quest.complete failed: blah blah"
int SimulatorThread::ErrorMessageAndQueryOK(char *buffer,
		const char *errorMessage) {
	int wpos = 0;
	wpos += PrepExt_SendInfoMessage(&buffer[wpos], errorMessage, INFOMSG_ERROR);
	wpos += PrepExt_QueryResponseString(&buffer[wpos], query.ID, "OK");
	return wpos;
}

PartyMember * SimulatorThread::RollForPartyLoot(ActiveParty *party,
		std::set<int> creatureIds, const char *rollType, int itemId) {
	g_Logs.simulator->info("[%v] Rolling for %v players", InternalID,
			creatureIds.size());
	int maxRoll = 0;
	ItemDef *cdef = g_ItemManager.GetPointerByID(itemId);
	PartyMember *maxRoller;
	if (creatureIds.size() == 1) {
		return party->GetMemberByID(*creatureIds.begin());
	}
	for (std::set<int>::iterator it = creatureIds.begin();
			it != creatureIds.end(); ++it) {
		int rolled = g_RandomManager.RandModRng(1, 100);
		PartyMember *m = party->GetMemberByID(*it);
		if (rolled > maxRoll) {
			maxRoller = m;
			maxRoll = rolled;
		}
		int wpos = PartyManager::WriteLootRoll(SendBuf,
				cdef->mDisplayName.c_str(), (char) rolled,
				m->mCreaturePtr->css.display_name);
		party->BroadCast(SendBuf, wpos);

	}
	return maxRoller;
}

void SimulatorThread::ClearLoot(ActiveParty *party, ActiveLootContainer *loot) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, 0, 0);
	loot->RemoveAllRolls();
}

void SimulatorThread::ResetLoot(ActiveParty *party, ActiveLootContainer *loot,
		int itemId) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, 0);
	loot->RemoveCreatureRolls(itemId, 0);
}

void SimulatorThread::UndoLoot(ActiveParty *party, ActiveLootContainer *loot,
		int itemId, int creatureId) {
	party->RemoveTagsForLootCreatureId(loot->CreatureID, itemId, creatureId);
	loot->RemoveCreatureRolls(itemId, creatureId);

}

bool SimulatorThread::ActivateActionAbilities(InventorySlot *slot) {
	ItemDef *itemDef = g_ItemManager.GetPointerByID(slot->IID);
	if (itemDef == NULL) {
		g_Logs.server->warn(
				"Item [%v] does not exist. Cannot activate action abilities.",
				slot->IID);
		return false;
	} else {
		if (itemDef->mActionAbilityId != 0)
			return creatureInst->RequestAbilityActivation(
					itemDef->mActionAbilityId) == Ability2::ABILITY_SUCCESS;
		else {
			if (itemDef->mType == ItemType::SPECIAL
					&& itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE) {
				AttemptSend(Aux1,
						PrepExt_SendBookOpen(Aux1, itemDef->mIvMax1,
								itemDef->mIvMax2 - 1, true));
			}
		}
		return true;
	}
}

void SimulatorThread::CheckIfLootReadyToDistribute(ActiveLootContainer *loot,
		LootTag lootTag) {
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInst->PartyID);
	bool needOrGreed = (party->mLootFlags & NEED_B4_GREED) > 0;

	// How many decisions do we expect to process the roll. This will depend
	// on if this is the secondary roll when round robin or loot master is in use
	// has been processed or not
	unsigned int requiredDecisions = party->GetOnlineMemberCount();
	if ((party->mLootMode == ROUND_ROBIN || party->mLootMode == LOOT_MASTER)) {
		if (loot->IsStage2(lootTag.mItemId)) {
			requiredDecisions = party->GetOnlineMemberCount() - 1;
		} else {
			requiredDecisions = 1;
		}
	}
	unsigned int decisions = (unsigned int) loot->CountDecisions(
			lootTag.mItemId);
	g_Logs.simulator->info("[%v] Loot requires %v decisions, we have %v",
			InternalID, requiredDecisions, decisions);

	if (decisions >= requiredDecisions) {
		g_Logs.simulator->info("[%v] Loot %v is ready to distribute",
				InternalID, lootTag.mItemId);

		CreatureInstance *receivingCreature = NULL;

		/*
		 * If the loot mode is loot master, and this roll was from the leader, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if (!loot->IsStage2(lootTag.mItemId) && party->mLootMode == LOOT_MASTER
				&& party->mLeaderID == creatureInst->CreatureID) {
			g_Logs.simulator->info(
					"[%v] Got loot roll from party leader %v for %v",
					InternalID, party->mLeaderID, lootTag.mItemId);
			if (loot->IsNeeded(lootTag.mItemId, creatureInst->CreatureID)
					|| loot->IsGreeded(lootTag.mItemId,
							creatureInst->CreatureID)) {
				g_Logs.simulator->info("[%v] Leader %v needed for %v",
						InternalID, party->mLeaderID, lootTag.mItemId);
				receivingCreature = creatureInst;
			} else {
				// Offer again to the rest of the party
				int iid = lootTag.mItemId;
				int cid = lootTag.mCreatureId;
				int lcid = lootTag.mLootCreatureId;
				g_Logs.simulator->info("[%v] Offering %v to rest of party",
						InternalID, iid);
				loot->SetStage2(iid, true);
				loot->RemoveCreatureRolls(iid, cid);
				party->RemoveCreatureTags(iid, cid);
				if (OfferLoot(-1, loot, party, creatureInst, iid, needOrGreed,
						lcid, 0) == QueryErrorMsg::GENERIC) {
					// Nobody to offer to, clean up as if the item had never been looted
					g_Logs.simulator->info(
							"[%v] Nobody to offer loot in %v to, cleaning up as if not yet looted.",
							InternalID, lcid);
					ResetLoot(party, loot, iid);
				}
				return;
			}
		}

		/*
		 * If the loot mode is round robin, and this roll was from them, then
		 * either give them the item, or offer again to the rest of the party depending
		 * on whether they needed or not
		 */
		if (!loot->IsStage2(lootTag.mItemId) && party->mLootMode == ROUND_ROBIN
				&& loot->robinID == creatureInst->CreatureID) {
			g_Logs.simulator->info("[%v] Got loot roll from robin %v for %v",
					InternalID, loot->robinID, lootTag.mItemId);
			if (loot->IsNeeded(lootTag.mItemId, creatureInst->CreatureID)
					|| loot->IsGreeded(lootTag.mItemId,
							creatureInst->CreatureID)) {
				g_Logs.simulator->info("[%v] Robin %v needed or greeded for %v",
						InternalID, loot->robinID, lootTag.mItemId);
				receivingCreature = creatureInst;
			} else {
				// Offer again to the rest of the party
				int iid = lootTag.mItemId;
				int cid = lootTag.mCreatureId;
				int lcid = lootTag.mLootCreatureId;
				g_Logs.simulator->info(
						"[%v] Robin passed, offering %d to rest of party",
						InternalID, iid);
				loot->SetStage2(iid, true);
				loot->RemoveCreatureRolls(iid, cid);
				party->RemoveCreatureTags(iid, cid);
				if (OfferLoot(-1, loot, party, creatureInst, iid, needOrGreed,
						lcid, 0) == QueryErrorMsg::GENERIC) {
					// Nobody to offer to, clean up as if the item had never been looted
					g_Logs.simulator->info(
							"[%v] Nobody to offer loot in %d to, cleaning up as if not yet looted.",
							InternalID, lcid);
					ResetLoot(party, loot, iid);
				}
				return;
			}
		}

		if (receivingCreature == NULL) {
			// No specific creature, first pick one of the needers if any
			set<int> needers = loot->needed[lootTag.mItemId];
			if (needers.size() > 0) {
				g_Logs.simulator->info("[%v] Rolling for %v needers",
						InternalID, needers.size());
				receivingCreature = RollForPartyLoot(party, needers, "Need",
						lootTag.mItemId)->mCreaturePtr;
			} else {
				set<int> greeders = loot->greeded[lootTag.mItemId];
				if (greeders.size() > 0) {
					g_Logs.simulator->info("[%v] Rolling for %v greeders",
							InternalID, greeders.size());
					receivingCreature = RollForPartyLoot(party, greeders,
							"Greed", lootTag.mItemId)->mCreaturePtr;
				}
			}
		}

		if (receivingCreature == NULL) {
			g_Logs.simulator->info("[%v] Everybody passed on loot %v",
					InternalID, lootTag.mItemId);
			// Send a winner with a tag of '0'. This will close the window
			for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
				// Skip the loot master or robin

				LootTag tag =
						party->mMemberList[i].IsOnlineAndValid() ?
								party->GetTag(lootTag.mItemId,
										party->mMemberList[i].mCreaturePtr->CreatureID) :
								NULL;
				if (tag.Valid()) {
					Util::SafeFormat(Aux2, sizeof(Aux2), "%d:%d",
							tag.mCreatureId, tag.mSlotIndex);
					Util::SafeFormat(Aux3, sizeof(Aux3), "%d", 0);
					WritePos = PartyManager::WriteLootWin(SendBuf, Aux2, "0",
							"Nobody", lootTag.mCreatureId, 999);
				}
			}
			ResetLoot(party, loot, lootTag.mItemId);
			return;
		}

		InventorySlot *newItem = NULL;

		// Send the actual winner to all of the party that have a tag
		LootTag winnerTag = party->GetTag(lootTag.mItemId,
				receivingCreature->CreatureID);
		for (unsigned int i = 0; i < party->mMemberList.size(); i++) {
			if (!party->mMemberList[i].IsOnlineAndValid()) {
				g_Logs.simulator->info(
						"[%v] Skipping informing %v of the winner (%v) as they have no simulator",
						InternalID, party->mMemberList[i].mCreatureID,
						lootTag.mCreatureId);
				continue;
			}

			g_Logs.simulator->info("[%v] Informing %v of the winner (%v)",
					InternalID, party->mMemberList[i].mCreaturePtr->CreatureID,
					lootTag.mCreatureId);
			LootTag tag = party->GetTag(lootTag.mItemId,
					party->mMemberList[i].mCreaturePtr->CreatureID);
			if (tag.Valid()) {
				Util::SafeFormat(Aux2, sizeof(Aux2), "%d:%d", tag.mCreatureId,
						tag.mSlotIndex);
				Util::SafeFormat(Aux3, sizeof(Aux3), "%d", tag.lootTag);
				WritePos = PartyManager::WriteLootWin(SendBuf, Aux2, Aux3,
						receivingCreature->css.display_name,
						lootTag.mCreatureId, 999);
				party->mMemberList[i].mCreaturePtr->actInst->LSendToOneSimulator(
						SendBuf, WritePos,
						party->mMemberList[i].mCreaturePtr->simulatorPtr);
			} else {
				g_Logs.simulator->warn(
						"[%v] No tag for item %v for a player %d to be informed",
						InternalID, lootTag.mItemId,
						party->mMemberList[i].mCreaturePtr->CreatureID);
			}
		}

		// Update the winners inventory
		CharacterData *charData = receivingCreature->charPtr;
		int slot = charData->inventory.GetFreeSlot(INV_CONTAINER);
		if (slot == -1) {
			Util::SafeFormat(Aux3, sizeof(Aux3),
					"%s doesn't have enough space. Starting bidding again",
					receivingCreature->css.display_name);
			party->BroadcastInfoMessageToAllMembers(Aux3);
			g_Logs.simulator->warn("[%v] Receive (%v) has no slots.",
					InternalID, receivingCreature->CreatureID);
			ResetLoot(party, loot, lootTag.mItemId);
			return;
		} else {
			newItem = charData->inventory.AddItem_Ex(INV_CONTAINER,
					winnerTag.mItemId, 1);
			if (newItem == NULL) {
				g_Logs.simulator->warn(
						"[%v] Item to loot (%v) has disappeared.", InternalID,
						winnerTag.mItemId);
				ResetLoot(party, loot, lootTag.mItemId);
				return;
			}
			charData->pendingChanges++;
			ActivateActionAbilities(newItem);
		}

		int conIndex = loot->HasItem(lootTag.mItemId);
		if (conIndex == -1) {
			g_Logs.simulator->warn("[%v] Item to loot (%v) missing.",
					InternalID, lootTag.mItemId);
		} else {
			// Remove the loot from the container
			loot->RemoveItem(conIndex);

			g_Logs.simulator->warn(
					"[%v] There is now %v items in loot container %v.",
					InternalID, loot->itemList.size(), loot->CreatureID);

			/* Reset the loot tags etc, we don't need them anymore.
			 * NOTE: Be careful not to use the lootTag object after this point as it may have been
			 * deleted.
			 */
			int lootCreatureID = lootTag.mLootCreatureId;
			ResetLoot(party, loot, lootTag.mItemId);

			if (loot->itemList.size() == 0) {
				ClearLoot(party, loot);

				// Loot container now empty, remove it
				CreatureInstance *lootCreature =
						creatureInst->actInst->GetNPCInstanceByCID(
								lootCreatureID);
				if (lootCreature != NULL) {
					lootCreature->activeLootID = 0;
					lootCreature->css.ClearLootSeeablePlayerIDs();
					lootCreature->css.ClearLootablePlayerIDs();
					lootCreature->_RemoveStatusList(StatusEffects::IS_USABLE);
					lootCreature->css.appearance_override =
							LootSystem::DefaultTombstoneAppearanceOverride;
					static const short statList[3] = {
							STAT::APPEARANCE_OVERRIDE,
							STAT::LOOTABLE_PLAYER_IDS,
							STAT::LOOT_SEEABLE_PLAYER_IDS };
					WritePos = PrepExt_SendSpecificStats(SendBuf, lootCreature,
							&statList[0], 3);
					creatureInst->actInst->LSendToLocalSimulator(SendBuf,
							WritePos, creatureInst->CurrentX,
							creatureInst->CurrentZ);
				}
				creatureInst->actInst->lootsys.RemoveCreature(lootCreatureID);

				g_Logs.simulator->warn(
						"[%v] Loot %v is now empty (%v tags now in the party).",
						InternalID, loot->CreatureID, party->lootTags.size());
			}

			if (newItem != NULL && receivingCreature != NULL)
				// Send an update to the actual of the item
				receivingCreature->actInst->LSendToOneSimulator(SendBuf,
						AddItemUpdate(SendBuf, Aux3, newItem),
						receivingCreature->simulatorPtr);
		}
	} else {
		g_Logs.simulator->info("[%v] Loot %v not ready yet to distribute",
				InternalID, lootTag.mItemId);
	}
}

int SendToOneSimulator(char *buffer, int length, SimulatorThread *simPtr) {
	if (simPtr == NULL)
		return 0;
	simPtr->AttemptSend(buffer, length);
	return 1;
}

int SendToAllSimulator(char *buffer, int length, int ignoreIndex) {
	SIMULATOR_IT it;
	int success = 0;

	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->InternalID != ignoreIndex) {
			if (it->ProtocolState == 1) {
				int res = it->AttemptSend(buffer, length);
				if (res >= 0)
					success++;
			}
		}
	}
	return success;
}

int SendToOneSimulator(char *buffer, int length, int simIndex) {
	SIMULATOR_IT it;
	int success = 0;

	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->InternalID == simIndex) {
			if (it->ProtocolState == 1) {
				int res = it->AttemptSend(buffer, length);
				if (res >= 0)
					success++;
			}
		}
	}
	return success;
}

int SendToFriendSimulator(char *buffer, int length, int CDefID) {
	SIMULATOR_IT it;
	int success = 0;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.charPtr->GetFriendIndex(CDefID) >= 0) {
				int res = it->AttemptSend(buffer, length);
				if (res >= 0)
					success++;
			}
		}
	}
	return success;
}
