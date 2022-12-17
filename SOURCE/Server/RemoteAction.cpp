#include "RemoteAction.h"

#include "Config.h"
#include "Globals.h"

#include "Router.h"
#include "Simulator.h"
#include "SimulatorBase.h"

#include "VirtualItem.h"
#include "Ability2.h"     //Need for remote reloading of ability data.
#include "EliteMob.h"

#include "Creature.h"
#include "NPC.h"
#include "Instance.h"
#include "InstanceScale.h"
#include "AIScript.h"
#include "AIScript2.h"
#include "QuestScript.h"
#include "ZoneDef.h"
#include "Account.h"
#include "Util.h"
#include "DebugProfiler.h"
#include <stdlib.h>
#include "Report.h"
#include "http/HTTPService.h"
#include "util/Log.h"


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

const char *GetValueOfKey(MULTISTRING &extract, const char *key) {
	//Search the given list of rows for an entry with a certain key value
	//Then return a string pointer to its value.
	for (size_t i = 0; i < extract.size(); i++) {
		size_t vals = extract[i].size();
		if (vals >= 2) {
			if (extract[i][0].compare(key) == 0)
				return extract[i][1].c_str();
		}
	}
	//g_Log.AddMessageFormat("Not found: %s", key);
	return NULL;
}

int RunRemoteAction(ReportBuffer &report, MULTISTRING &header,
		MULTISTRING &params) {
	//dataStart = unused
	//header = extracted list of HTTP header information
	//params = extracted list of HTTP POST parameters.

	const char *action = GetValueOfKey(params, "action");
	const char *auth = GetValueOfKey(params, "authtoken");
	if (action == NULL || auth == NULL) {
		g_Logs.http->error("Invalid POST message.");
		return REMOTE_INVALIDPOST;
	}

	if (g_Config.RemotePasswordMatch(auth) == false) {
		g_Logs.http->error("Invalid remote authentication string.");
		//g_Log.AddMessageFormat("Has:[%s], Need:[%s]", auth, g_Config.RemoteAuthenticationPassword.c_str());
		return REMOTE_AUTHFAILED;
	}

	if (strcmp(action, "shutdown") == 0) {
		g_Logs.event->info("[NOTICE] The server was remotely shut down.");
		g_ServerStatus = SERVER_STATUS_STOPPED;
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "refreshthreads") == 0) {
		Report::RefreshThreads(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshtime") == 0) {
		Report::RefreshTime(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshmods") == 0) {
		Report::RefreshMods(report, GetValueOfKey(params, "sim_id"));
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshplayers") == 0) {
		Report::RefreshPlayers(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshinstance") == 0) {
		Report::RefreshInstance(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshscripts") == 0) {
		Report::RefreshScripts(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshhateprofile") == 0) {
		Report::RefreshHateProfile(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshcharacter") == 0) {
		Report::RefreshCharacter(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshsim") == 0) {
		Report::RefreshSim(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshprofiler") == 0) {
		Report::RefreshProfiler(report);
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshitem") == 0) {
		Report::RefreshItem(report, GetValueOfKey(params, "sim_id"));
		return REMOTE_REPORT;
	} else if (strcmp(action, "refreshitemdetailed") == 0) {
		Report::RefreshItemDetailed(report, GetValueOfKey(params, "sim_id"));
		return REMOTE_REPORT;
	} else if (strcmp(action, "reloadvi") == 0) {
		g_VirtualItemModSystem.LoadSettings();
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "reloadchecksum") == 0) {
		g_FileChecksum.LoadFromFile();
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "reloadconfig") == 0) {
		std::vector<std::string> paths = g_Config.ResolveLocalConfigurationPath();
		for (std::vector<std::string>::iterator it = paths.begin();
				it != paths.end(); ++it) {
			std::string dir = *it;
			std::string filename = Platform::JoinPath(dir, "ServerConfig.txt");
			if(!LoadConfig(filename) && it == paths.begin())
				g_Logs.data->error("Could not open server configuration file: %v", filename);
		}
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "reloadability") == 0) {
		g_AbilityManager.LoadData();
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "reloadelite") == 0) {
		g_EliteManager.LoadData();
		return REMOTE_COMPLETE;
	} else if (strcmp(action, "syschat") == 0) {
		const char *data = GetValueOfKey(params, "data");
		if (data == NULL)
			return REMOTE_FAILED;

		g_SimulatorManager.BroadcastChat(0, "System", "*SysChat", data);

		return REMOTE_COMPLETE;
	} else if (strcmp(action, "refreshpacket") == 0) {
		Report::RefreshPacket(report);
		return REMOTE_REPORT;
	}
	return REMOTE_HANDLER;
}

namespace Report {

void RefreshThreads(ReportBuffer &report) {
	long httpsend = 0;
	long httprec = 0;
	long simsend = 0;
	long simrec = 0;
	long trec = 0;
	long tsnd = 0;

	report.AddLine("Simulators in use: %d", Simulator.size());
//	report.AddLine("HTTP Full Connection Errors: %d", HTTPBaseServer.Debug_MaxFullConnections);
//	report.AddLine("HTTP Full Kicked connections: %d", HTTPBaseServer.Debug_KickedFullConnections);
//	report.AddLine("HTTP Dropped hanging connections: %d", g_HTTPDistributeManager.mDroppedConnections);
	report.AddLine(NULL);
	report.AddLine("Sent: %s in %d packets",
			GetDataSizeStr(g_PacketManager.mTotalBytesSent),
			g_PacketManager.mTotalPacketsSent);
	report.AddLine("Dropped: %s in %d packets",
			GetDataSizeStr(g_PacketManager.mTotalBytesDropped),
			g_PacketManager.mTotalPacketsDropped);
	report.AddLine("Clustered: %s in %d packets",
			GetDataSizeStr(g_PacketManager.mClusterPacketBytes),
			g_PacketManager.mClusterPackets);
	report.AddLine("Incomplete: %s in %d packets",
			GetDataSizeStr(g_PacketManager.mTotalBytesIncomplete),
			g_PacketManager.mTotalPacketsIncomplete);
	report.AddLine("Send Zero: %d", g_PacketManager.mTotalSendZero);
	report.AddLine("Forced Delays: %d", g_PacketManager.mCountForceDelay);
	report.AddLine("Forced Delays (waiting): %d",
			g_PacketManager.mCountForceDelayAck);
	report.AddLine("Delayed: %d", g_PacketManager.mTotalWait);
	report.AddLine(NULL);

	int TimeSpan = (g_ServerTime - g_ServerLaunchTime) / 1000;
	int Hour = TimeSpan / 3600;
	int Minute = (TimeSpan / 60) % 60;
	int Second = TimeSpan % 60;
	int Day = Hour / 24;
	Hour -= (Day * 24);

	report.AddLine("Uptime: %02dd:%02dh:%02dm:%02ds", Day, Hour, Minute,
			Second);
	report.AddLine(NULL);

	report.AddLine("[Component] (Status) Sent bytes / Received bytes");
//	report.AddLine("[HTTP] (%s) %s / %s (total of deactivated threads) ",
//		StatusPhaseStrings[HTTPBaseServer.Status],
//		GetDataSizeStr(g_HTTPDistributeManager.mTotalSendBytes),
//		GetDataSizeStr(g_HTTPDistributeManager.mTotalRecBytes));

//	httpsend += g_HTTPDistributeManager.mTotalSendBytes;
//	httprec += g_HTTPDistributeManager.mTotalRecBytes;
//
//	tsnd += g_HTTPDistributeManager.mTotalSendBytes;
//	trec += g_HTTPDistributeManager.mTotalRecBytes;

//	//Thread guard to fix a potential multithreaded issue of accessing while adding or removing list items.
//	http_cs.Enter("RefreshThreads");
//	HTTPDistributeManager::ITERATOR hdit;
//	for(hdit = g_HTTPDistributeManager.mDistributeList.begin(); hdit != g_HTTPDistributeManager.mDistributeList.end(); ++hdit)
//	{
//		report.AddLine("[HTTP:%d] (%s) %s / %s (%d, %d, %d)",
//			hdit->InternalIndex,
//			StatusPhaseStrings[hdit->Status],
//			GetDataSizeStr(hdit->TotalSendBytes),
//			GetDataSizeStr(hdit->TotalRecBytes),
//			(int)hdit->inUse,
//			(int)hdit->SendFile,
//			(int)hdit->Finished);
//
//		httpsend += hdit->TotalSendBytes;
//		httprec += hdit->TotalRecBytes;
//		tsnd += hdit->TotalSendBytes;
//		trec += hdit->TotalRecBytes;
//	}
//	http_cs.Leave();

	report.AddLine("[Router] (%s) %s / %s", StatusPhaseStrings[Router.Status],
			GetDataSizeStr(Router.TotalSendBytes),
			GetDataSizeStr(Router.TotalRecBytes));
	tsnd += Router.TotalSendBytes;
	trec += Router.TotalRecBytes;

	report.AddLine("[SimB] (%s) %s / %s",
			StatusPhaseStrings[SimulatorBase.Status],
			GetDataSizeStr(SimulatorBase.TotalSendBytes),
			GetDataSizeStr(SimulatorBase.TotalRecBytes));
	tsnd += SimulatorBase.TotalSendBytes;
	trec += SimulatorBase.TotalRecBytes;

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		report.AddLine("[Sim:%d] (%s) %s / %s (End: %d)", it->InternalID,
				StatusPhaseStrings[it->Status],
				GetDataSizeStr(it->TotalSendBytes),
				GetDataSizeStr(it->TotalRecBytes), it->MessageEnd);

		simsend += it->TotalSendBytes;
		simrec += it->TotalRecBytes;

		tsnd += it->TotalSendBytes;
		trec += it->TotalRecBytes;
	}

	report.AddLine(NULL);
	report.AddLine("Total HTTP sent: %s", GetDataSizeStr(httpsend));
	report.AddLine("Total HTTP received: %s", GetDataSizeStr(httprec));

	simsend += g_SimulatorManager.baseByteSent;
	simrec += g_SimulatorManager.baseByteRec;
	tsnd += g_SimulatorManager.baseByteSent;
	trec += g_SimulatorManager.baseByteRec;
	report.AddLine(NULL);
	report.AddLine("Deactivated Simulator sent: %s",
			GetDataSizeStr(g_SimulatorManager.baseByteSent));
	report.AddLine("Deactivated Simulator received: %s",
			GetDataSizeStr(g_SimulatorManager.baseByteRec));
	report.AddLine("Total Simulator sent: %s", GetDataSizeStr(simsend));
	report.AddLine("Total Simulator received: %s", GetDataSizeStr(simrec));

	report.AddLine(NULL);
	report.AddLine("Grand Total sent: %s", GetDataSizeStr(tsnd));
	report.AddLine("Grand Total received: %s", GetDataSizeStr(trec));
}

void RefreshTime(ReportBuffer &report) {
	char timeBuf[64];
	report.AddLine("Current Time: %lu", g_ServerTime);
	report.AddLine("Launch Time: %lu", g_ServerLaunchTime);
	report.AddLine("Difference: %lu", g_ServerTime - g_ServerLaunchTime);
	report.AddLine(NULL);
	size_t i;

	report.AddLine("INSTANCES:%d", g_ActiveInstanceManager.instListPtr.size());
	for (i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++) {
		ActiveInstance *aInst = g_ActiveInstanceManager.instListPtr[i];
		if (aInst->mZoneDefPtr == NULL)
			continue;

		report.AddLine("ZoneID:%d  InstanceID:%d  Name:%s", aInst->mZone,
				aInst->mInstanceID, aInst->mZoneDefPtr->mName.c_str());
		report.AddLine("Owner CDefID:%d  Name:%s  Party:%d",
				aInst->mOwnerCreatureDefID, aInst->mOwnerName.c_str(),
				aInst->mOwnerPartyID);
		report.AddLine("Players: %d", aInst->mPlayers);
		int timeSec = (aInst->mExpireTime - g_ServerTime) / 1000;
		report.AddLine("Expire Time: %s",
				Util::FormatTime(timeBuf, sizeof(timeBuf), timeSec));
		report.AddLine(NULL);
	}

	report.AddLine(NULL);
	g_ZoneDefManager.GenerateReportActive(report);
}

int GetSimulatorID(const char *strID) {
	if (strID == NULL)
		return 0;
	return atoi(strID);
}

void RefreshMods(ReportBuffer &report, const char *simID) {
	int ID = GetSimulatorID(simID);
	SimulatorThread *simPtr = GetSimulatorByID(ID);
	if (simPtr == NULL)
		report.AddLine("Invalid simulator: %d", ID);
	else
		simPtr->Debug_GenerateCreatureReport(report);
}

void RefreshPlayers(ReportBuffer &report) {
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			report.AddLine("%s (%d, %d, %d) [%s] [Sim:%d] [ID:%d, CDef: %d]",
					it->pld.charPtr->cdef.css.display_name,
					it->creatureInst->CurrentX, it->creatureInst->CurrentY,
					it->creatureInst->CurrentZ,
					it->pld.zoneDef->mWarpName.c_str(), it->InternalID,
					it->pld.CreatureID, it->pld.CreatureDefID);
		}
	}
}

void Helper_OutputCreature(ReportBuffer &report, int index,
		CreatureInstance *obj) {
	int c, d;

	report.AddLine("  [%d] ID:%d, Def:%d, Ptr:%p (%s), TargPtr:%p (%s)", index,
			obj->CreatureID, obj->CreatureDefID, obj, obj->css.display_name,
			obj->CurrentTarget.targ,
			(obj->CurrentTarget.targ != NULL) ?
					obj->CurrentTarget.targ->css.display_name : "null");

	if (obj->aiScript != NULL && obj->aiScript->mActive)
		report.AddLine("TSL %s", obj->aiScript->def->scriptName.c_str());
	if (obj->aiNut != NULL)
		report.AddLine(
				"Squirrel %s calls: %lu ptime: %lu itime: %lu exec: %s, halting: %s status: %s",
				obj->aiNut->def->scriptName.c_str(), obj->aiNut->mCalls,
				obj->aiNut->mProcessingTime, obj->aiNut->mInitTime,
				obj->aiNut->GetStatus().c_str(),
				obj->aiNut->mHalting ? "yes" : "no",
				obj->aiNut->mActive ? "yes" : "no");

	report.AddLine("%d,%d,%d", obj->CurrentX, obj->CurrentY, obj->CurrentZ);
	for (d = 0; d < 2; d++) {
		report.AppendLine("    ab[%d]: %d (%d, %c)", d, obj->ab[d].TargetCount,
				obj->ab[d].abilityID, obj->ab[d].bPending == true ? 'T' : 'F');
		for (c = 0; c < obj->ab[d].TargetCount; c++) {
			report.AppendLine(", %p (%s)", obj->ab[d].TargetList[c],
					(obj->ab[d].TargetList[c] != NULL) ?
							obj->ab[d].TargetList[c]->css.display_name :
							"null");
		}
		report.AddLine(NULL);
	}
	if (obj->AnchorObject != NULL) {
		report.AddLine("  Officer: %p (%s)\r\n", obj->AnchorObject,
				obj->AnchorObject->css.display_name);
		report.AddLine("  Position: %d, %d, %d (MovT: %d)\r\n", obj->CurrentX,
				obj->CurrentY, obj->CurrentZ, obj->movementTime - g_ServerTime);
	}
	if (obj->MountedBy != NULL) {
		report.AddLine("  Mounter: %p (%s)\r\n", obj->MountedBy,
				obj->MountedBy->css.display_name);
		report.AddLine("  Position: %d, %d, %d (MovT: %d)\r\n", obj->CurrentX,
				obj->CurrentY, obj->CurrentZ, obj->movementTime - g_ServerTime);
	}
	if (obj->MountedOn != NULL) {
		report.AddLine("  Mount: %p (%s)\r\n", obj->MountedOn,
				obj->MountedOn->css.display_name);
		report.AddLine("  Position: %d, %d, %d (MovT: %d)\r\n", obj->CurrentX,
				obj->CurrentY, obj->CurrentZ, obj->movementTime - g_ServerTime);
	}
}

void RefreshScripts(ReportBuffer &report) {
	report.AddLine("Quest Scripts");
	g_QuestNutManager.cs.Enter("RemoteAction::RefreshScripts");
	std::map<int, std::list<QuestScript::QuestNutPlayer *> >::iterator it =
			g_QuestNutManager.questAct.begin();
	double seconds = 0;
	for (; it != g_QuestNutManager.questAct.end(); ++it) {
		std::list<QuestScript::QuestNutPlayer *> l = it->second;
		report.AddLine("+-Quest %d", it->first);
		for (std::list<QuestScript::QuestNutPlayer *>::iterator lit = l.begin();
				lit != l.end(); ++lit) {
			QuestScript::QuestNutPlayer *player = *lit;
			seconds = (double) player->mProcessingTime / 1000.0;
			report.AddLine("    %-50s %-20s %-20s %4.4f %5d %5d %5d %-10s",
					player->def->mSourceFile.c_str(),
					player->def->scriptName.c_str(),
					player->def->mAuthor.c_str(), seconds, player->mInitTime,
					player->mCalls, player->mGCTime,
					player->GetStatus().c_str());

			if (report.WasTruncated())
				break;
		}

		if (report.WasTruncated())
			break;
	}
	g_QuestNutManager.cs.Leave();
	report.AddLine("Instance Scripts");
	g_ActiveInstanceManager.cs.Enter("RemoteAction::RefreshScripts");
	size_t a;
	for (a = 0; a < g_ActiveInstanceManager.instListPtr.size(); a++) {
		ActiveInstance *ainst = g_ActiveInstanceManager.instListPtr[a];
		InstanceScript::InstanceNutPlayer * player = ainst->nutScriptPlayer;
		if (player != NULL) {
			report.AddLine("    %-50s %-20s %-20s %4.4f %5d %5d %5d %-10s",
					player->def->mSourceFile.c_str(),
					player->def->scriptName.c_str(),
					player->def->mAuthor.c_str(), seconds, player->mInitTime,
					player->mCalls, player->mGCTime,
					player->GetStatus().c_str());
		}
		report.AddLine("  AI (NPC)");
		for (std::vector<CreatureInstance*>::iterator it =
				ainst->NPCListPtr.begin(); it != ainst->NPCListPtr.end();
				++it) {
			CreatureInstance *cinst = *it;
			if (cinst->aiNut != NULL) {
				AINutPlayer *ai = cinst->aiNut;
				seconds = (double) ai->mProcessingTime / 1000.0;
				report.AddLine("    %-50s %-20s %-20s %4.4f %5d %5d %5d %-10s",
						ai->def->mSourceFile.c_str(),
						ai->def->scriptName.c_str(), ai->def->mAuthor.c_str(),
						seconds, ai->mInitTime, ai->mCalls, ai->mGCTime,
						ai->GetStatus().c_str());
			}
		}
		report.AddLine("  AI (Sidekicks)");
		for (std::vector<CreatureInstance*>::iterator it =
				ainst->SidekickListPtr.begin();
				it != ainst->SidekickListPtr.end(); ++it) {
			CreatureInstance *cinst = *it;
			if (cinst->aiNut != NULL) {
				AINutPlayer *ai = cinst->aiNut;
				seconds = (double) ai->mProcessingTime / 1000.0;
				report.AddLine("    %-50s %-20s %-20s %4.4f %5d %5d %5d %-10s",
						ai->def->mSourceFile.c_str(),
						ai->def->scriptName.c_str(), ai->def->mAuthor.c_str(),
						seconds, ai->mInitTime, ai->mCalls, ai->mGCTime,
						ai->GetStatus().c_str());
			}
		}

		if (report.WasTruncated())
			break;
	}
	g_ActiveInstanceManager.cs.Leave();

}

void RefreshInstance(ReportBuffer &report) {
	size_t a, b;
	g_ActiveInstanceManager.cs.Enter("RemoteAction::RefreshInstance");
	for (a = 0; a < g_ActiveInstanceManager.instListPtr.size(); a++) {
		ActiveInstance *ainst = g_ActiveInstanceManager.instListPtr[a];
		report.AddLine("[%d : (%s) - ID: %d, Zone: %d, Players: %d]", a,
				ainst->mZoneDefPtr->mName.c_str(), ainst->mInstanceID,
				ainst->mZone, ainst->mPlayers);
		if (ainst->scaleProfile != NULL)
			report.AddLine("Scaling profile:%s",
					ainst->scaleProfile->mDifficultyName.c_str());
		report.AddLine("Mobs killed: %d, Drop multiplier: %g",
				ainst->mKillCount, ainst->mDropRateBonusMultiplier);

		report.AddLine("Registered Simulators:");
		for (b = 0; b < ainst->RegSim.size(); b++)
			report.AddLine("Sim:%d (name: %s, ptr: %p)",
					ainst->RegSim[b]->InternalID,
					ainst->RegSim[b]->creatureInst->css.display_name,
					ainst->RegSim[b]);

		report.AddLine("Player Instances: %d", ainst->PlayerListPtr.size());
		report.AddLine("Sidekick Instances: %d", ainst->SidekickListPtr.size());
		report.AddLine("Creature Instances: %d", ainst->NPCList.size());
		report.AddLine(NULL);
	}

	report.AddLine(NULL);
	report.AddLine("DETAILED MOB REPORT");
	for (a = 0; a < g_ActiveInstanceManager.instListPtr.size(); a++) {
		ActiveInstance *ainst = g_ActiveInstanceManager.instListPtr[a];
		report.AddLine("[%d : (%s) - ID: %d, Zone: %d, Players: %d]", a,
				ainst->mZoneDefPtr->mName.c_str(), ainst->mInstanceID,
				ainst->mZone, ainst->mPlayers);

		for (b = 0; b < ainst->PlayerListPtr.size(); b++)
			Helper_OutputCreature(report, b, ainst->PlayerListPtr[b]);

		report.AddLine(NULL);

		for (b = 0; b < ainst->SidekickListPtr.size(); b++)
			Helper_OutputCreature(report, b, ainst->SidekickListPtr[b]);

		report.AddLine(NULL);

		ActiveInstance::CREATURE_IT it;
		for (it = ainst->NPCList.begin(); it != ainst->NPCList.end(); ++it)
			Helper_OutputCreature(report, b, &it->second);

		report.AddLine(NULL);
		report.AddLine(NULL);

		if (report.WasTruncated())
			break;
	}
	g_ActiveInstanceManager.cs.Leave();
}

void Helper_HateProfile(ReportBuffer &report, CreatureInstance *obj,
		const char *label) {
	if (obj == NULL)
		return;
	if (obj->hateProfilePtr == NULL)
		return;

	report.AddLine("%s (%d)=0x%p", label, obj->CreatureID, obj->hateProfilePtr);
}

void RefreshHateProfile(ReportBuffer &report) {
	for (size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++) {
		ActiveInstance *aInst = g_ActiveInstanceManager.instListPtr[i];
		report.AddLine("[%s (%d)]", aInst->mZoneDefPtr->mName.c_str(),
				aInst->mZone);
		HateProfileContainer *hpc = &aInst->hateProfiles;
		int count = hpc->profileList.size();
		report.AddLine("Hate profiles: ", count);
		std::list<HateProfile>::iterator hpi;
		for (hpi = hpc->profileList.begin(); hpi != hpc->profileList.end();
				++hpi) {
			for (size_t hpp = 0; hpp < hpi->hateList.size(); hpp++) {
				HateCreatureData *hcd = &hpi->hateList[hpp];
				report.AddLine("[0x%p] CDef:%d  Lev:%d  Dmg:%d  Hate:%d", &*hpi,
						hcd->CDefID, hcd->level, hcd->damage, hcd->hate);
			}
		}
		report.AddLine(NULL);
		report.AddLine(NULL);

		for (size_t i = 0; i < aInst->PlayerListPtr.size(); i++)
			Helper_HateProfile(report, aInst->PlayerListPtr[i], "Player");

		ActiveInstance::CREATURE_IT it;
		for (it = aInst->NPCList.begin(); it != aInst->NPCList.end(); ++it)
			Helper_HateProfile(report, &it->second, "Creature");

		for (size_t i = 0; i < aInst->SidekickListPtr.size(); i++)
			Helper_HateProfile(report, aInst->SidekickListPtr[i], "Sidekick");
	}
}

void RefreshCharacter(ReportBuffer &report) {
	g_CharacterManager.GetThread("RefreshCharacter");
	CharacterManager::CHARACTER_MAP::iterator it;
	report.AddLine("Characters: %d", g_CharacterManager.charList.size());
	for (it = g_CharacterManager.charList.begin();
			it != g_CharacterManager.charList.end(); ++it) {
		report.AddLine("ID: %d, Name: %s, Expires: %d (%d)", it->first,
				it->second.cdef.css.display_name, it->second.expireTime,
				(it->second.expireTime != 0) ?
						(it->second.expireTime - g_ServerTime) / 1000 : 0);
	}
	g_CharacterManager.ReleaseThread();
}

void RefreshSim(ReportBuffer &report) {
	Debug_GenerateSimulatorReports(&report);
}

void RefreshProfiler(ReportBuffer &report) {
#ifndef DEBUG_PROFILER
	report.addLine("Profiling not enabled.");
#else
	_DebugProfiler.GenerateReport(report);
#endif
}

void RefreshItem(ReportBuffer &report, const char *simID) {
	int ID = GetSimulatorID(simID);
	SimulatorThread *simPtr = GetSimulatorByID(ID);
	if (simPtr == NULL)
		report.AddLine("Invalid simulator: %d", ID);
	else
		simPtr->Debug_GenerateItemReport(report, true);
}

void RefreshItemDetailed(ReportBuffer &report, const char *simID) {
	int ID = GetSimulatorID(simID);
	SimulatorThread *simPtr = GetSimulatorByID(ID);
	if (simPtr == NULL)
		report.AddLine("Invalid simulator: %d", ID);
	else
		simPtr->Debug_GenerateItemReport(report, false);
}

void RefreshPacket(ReportBuffer &report) {
	g_PacketManager.GenerateDebugReport(report);
}

} //namespace Report

int RunAccountCreation(MULTISTRING &params) {
	const char *regkey = GetValueOfKey(params, "regkey");
	const char *username = GetValueOfKey(params, "username");
	const char *password = GetValueOfKey(params, "password");
	const char *grove = GetValueOfKey(params, "grove");
	const char *auth = GetValueOfKey(params, "authtoken");

	if (regkey != NULL && auth != NULL) {
		if (g_Config.RemotePasswordMatch(auth) == false) {
			g_Logs.http->error("Invalid remote authentication string.");
			return REMOTE_AUTHFAILED;
		}
		if (!g_AccountManager.PopRegistrationKey(regkey)) {
			/* Key is not found, authtoken provided, so import this key as it is so
			 * it can be immediately used to create an account */
			g_AccountManager.ImportKey(regkey);
		}
	}
	else {
		if (!g_AccountManager.PopRegistrationKey(regkey)) {
			return AccountManager::ErrorCode::ACCOUNT_KEY;
		}
	}

	int retval = 0;
	g_AccountManager.cs.Enter("RunAccountCreation");
	retval = g_AccountManager.CreateAccount(username, password, regkey, grove);
	g_AccountManager.cs.Leave();
	return retval;
}

int RunPasswordReset(MULTISTRING &params) {
	const char *regkey = GetValueOfKey(params, "regkey");
	const char *username = GetValueOfKey(params, "username");
	const char *newpassword = GetValueOfKey(params, "password");
	const char *auth = GetValueOfKey(params, "authtoken");
	bool checkPermission = true;
	if (auth != NULL && g_Config.RemotePasswordMatch(auth) == true)
		checkPermission = false;

	int retval = 0;
	g_AccountManager.cs.Enter("RunPasswordReset");
	retval = g_AccountManager.ResetPassword(username, newpassword, regkey,
			checkPermission);
	g_AccountManager.cs.Leave();
	return retval;
}

int RunAccountRecover(MULTISTRING &params) {
	const char *username = GetValueOfKey(params, "username");
	const char *keypass = GetValueOfKey(params, "keypass");
	const char *type = GetValueOfKey(params, "type");
	int retval = 0;
	g_AccountManager.cs.Enter("RunPasswordReset");
	retval = g_AccountManager.AccountRecover(username, keypass, type);
	g_AccountManager.cs.Leave();
	return retval;
}

