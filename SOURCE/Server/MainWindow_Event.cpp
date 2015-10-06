// Contains all event code used in the Windows control system, when controls
// are interacted with.

#include "Components.h"

#ifdef USE_WINDOWS_GUI

// The various diagnostic and information controls tend to cover most of the global
// systems within the server, so there are many files which much be included.
#include "MainWindow.h"
#include "DebugTracer.h"
#include "Simulator.h"
#include "StringList.h"
#include "Chat.h"
#include "Scenery2.h"
#include "Config.h"
#include "Item.h"
#include "AIScript.h"
#include "AIScript2.h"
#include "SimulatorBase.h"
#include "Router.h"
#include "Instance.h"
#include "Globals.h"
#include "Character.h"
#include "ZoneDef.h"
#include "PartyManager.h"
#include "DebugProfiler.h"
#include "Report.h"
#include "RemoteAction.h"
#include "Ability2.h"

#include "DirectoryAccess.h"
#include <Commdlg.h>
#pragma comment(lib, "Comdlg32.lib")

extern char GSendBuf[32767];
void Debug_FullDump(void);
void CheckPendingScenerySave(bool force);
char *GetDataSizeStr(long value);

void Event_Click_Button_ServerStart(void)
{
}

void Event_Click_Button_StatusClear(void)
{
	memset(WindowTextBuffer, 0, sizeof(WindowTextBuffer));
	SetWindowText(MainWindowControlSet[MWCS_Edit_Status], "");
}

int GetIntFromControl(HWND hWnd)
{
	static char convbuf[32];
	GetWindowText(hWnd, convbuf, 32);
	return atoi(convbuf);
}

float GetFloatFromControl(HWND hWnd)
{
	static char convbuf[32];
	GetWindowText(hWnd, convbuf, 32);
	return (float)atof(convbuf);
}

void Event_Click_Button_UseTool(void)
{
	//Get the selected index of the tool dropdown list, then convert to the tool operation
	//ID based on that tool's function
	int res = SendMessage(MainWindowControlSet[MWCS_ComboBox_ToolList], CB_GETCURSEL, 0, 0);
	res = ToolControlDef[res].CallID;

	static char SendBack[2048];
	int wpos = 0;
	bool send = false;

	if(res == TCID_CueEffect)
	{
		char Effect[64] = {0};
		int sid = GetIntFromControl(MainWindowControlSet[MWCS_Edit_ToolEdit0]);
		GetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit1], Effect, sizeof(Effect));

		wpos += PutByte(&SendBack[wpos], 4);       //_handleCreatureEventMsg 
		wpos += PutShort(&SendBack[wpos], 0x00);    //Reserve for size
		wpos += PutInteger(&SendBack[wpos], sid);
		wpos += PutByte(&SendBack[wpos], 4);  //Cue effect
		wpos += PutStringUTF(&SendBack[wpos], Effect);
		PutShort(&SendBack[1], wpos - 3);       //Set message size
		send = true;
	}
	else if(res == TCID_CueEffectTarget)
	{
		char Effect[64] = {0};
		int sid = GetIntFromControl(MainWindowControlSet[MWCS_Edit_ToolEdit0]);
		GetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit1], Effect, sizeof(Effect));
		int sid_targ = GetIntFromControl(MainWindowControlSet[MWCS_Edit_ToolEdit2]);

		wpos += PutByte(&SendBack[wpos], 4);       //_handleCreatureEventMsg 
		wpos += PutShort(&SendBack[wpos], 0x00);    //Reserve for size
		wpos += PutInteger(&SendBack[wpos], sid);
		wpos += PutByte(&SendBack[wpos], 12);  //Cue effect with single target
		wpos += PutStringUTF(&SendBack[wpos], Effect);
		wpos += PutInteger(&SendBack[wpos], sid_targ);
		PutShort(&SendBack[1], wpos - 3);       //Set message size
		send = true;
	}
	else if(res == TCID_Chat)
	{
		char Name[64] = {0};
		char Channel[64] = {0};
		char Message[256] = {0};

		GetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit0], Name, sizeof(Name));
		GetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit1], Channel, sizeof(Channel));
		GetWindowText(MainWindowControlSet[MWCS_Edit_ToolEdit2], Message, sizeof(Message));

		int size = handleCommunicationMsg(Channel, Message, Name);
		/*
		int size = PrepExt_GenericChatMessage(GSendBuf, Name, Channel, Message);
		SendToAllSimulator("GenericChatMessage", GSendBuf, size, -1);
		*/
	}

	if(send == true)
	{
		SimulatorThread *simPtr = GetDebugSimulator();
		if(simPtr != NULL)
		{
			int res = simPtr->AttemptSend(SendBack, wpos);
			if(res >= 0)
				g_Log.AddMessageFormat("Sent tool message of %d bytes.", res);
			else
				g_Log.AddMessage("Failed to tool message.");
		}
	}
}

void Event_Click_Button_SplitArray(void)
{
	GetWindowText(MainWindowControlSet[MWCS_Edit_Stats], WindowTextBuffer, sizeof(WindowTextBuffer));
	GSendBuf[0] = 0;
	char *pos = NULL;
	char *lastpos = WindowTextBuffer;
	int wpos = 0;
	int rpos = 0;
	int len = strlen(WindowTextBuffer);
	int newoffset = 0;
	int lastoffset = 0;
	do
	{
		pos = strpbrk(&WindowTextBuffer[rpos], "[]{},");
		if(pos != NULL)
		{
			int offset = pos - &WindowTextBuffer[rpos] + 1;
			rpos += offset;

			bool pass = true;
			if(pos[0] == '[')
				if(pos[1] == '"')
					pass = false;
			if(pos[0] == ']')
				if(pos[-1] == '"')
					pass = false;

			if(pass == true)
			{
				if(pos - lastpos > 1)
				{
					strncat(&GSendBuf[wpos], lastpos, pos - lastpos);
					wpos += (pos - lastpos);
				}
				lastpos = pos + 1;				GSendBuf[wpos++] = *pos;
				GSendBuf[wpos++] = '\r';
				GSendBuf[wpos++] = '\n';
			}
		}
	} while(pos != NULL && rpos < len);
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], GSendBuf);
}

void Event_Click_Button_SaveItems(void)
{
	/*
	FILE *output = fopen("ItemTable.txt", "wb");
	if(output == NULL)
		return;

	int a;
	for(a = 0; a < (int)ItemList.size(); a++)
		fprintf(output, "%d %s [%d]\r\n", a, ItemList[a].mDisplayName.c_str(), ItemList[a].mID);
	fclose(output);
	g_Log.AddMessage("Item table file saved.");
	*/
	
	FILE *output = fopen("ItemTable2.txt", "wb");
	if(output == NULL)
		return;

	fprintf(output, "Index;ID;Name;Level;Rarity;EquipType;ArmorType;Str;Dex;Con;Psy;Spi;Dmg;Armor\r\n");

	int index = 0;
	ItemManager::ITEM_CONT::iterator it;
	for(it = g_ItemManager.ItemList.begin(); it != g_ItemManager.ItemList.end(); ++it)
	{
		ItemDef *item = &it->second;
		//Index, ID, name
		fprintf(output, "%d;%d;%s;", index, item->mID, item->mDisplayName.c_str());

		fprintf(output, "%d;", item->mLevel);
		fprintf(output, "%d;", item->mQualityLevel);
		fprintf(output, "%s;", GetEquipType(item->mEquipType));
		if(item->mArmorType != 0)
			fprintf(output, "%s", GetArmorType(item->mArmorType));
		fputc(';', output);

		if(item->mWeaponType != 0)
			fprintf(output, "%s", GetWeaponType(item->mWeaponType));
		fputc(';', output);

		if(item->mBonusStrength != 0)
			fprintf(output, "%d Str", item->mBonusStrength);
		fputc(';', output);

		if(item->mBonusDexterity != 0)
			fprintf(output, "%d Dex", item->mBonusDexterity);
		fputc(';', output);

		if(item->mBonusConstitution != 0)
			fprintf(output, "%d Con", item->mBonusConstitution);
		fputc(';', output);

		if(item->mBonusPsyche != 0)
			fprintf(output, "%d Psy", item->mBonusPsyche);
		fputc(';', output);

		if(item->mBonusSpirit != 0)
			fprintf(output, "%d Spi", item->mBonusSpirit);
		fputc(';', output);

		if(item->mWeaponDamageMin != 0 && item->mWeaponDamageMax != 0)
			fprintf(output, "%d-%d damage", item->mWeaponDamageMin, item->mWeaponDamageMax);
		fputc(';', output);

		if(item->mArmorResistMelee != 0)
			fprintf(output, "%d armor", item->mArmorResistMelee);
		fputc(';', output);

		fprintf(output, "\r\n");
		index++;
	}
	fclose(output);
	g_Log.AddMessage("Item table file saved.");
}

void Event_Click_Button_DisassembleScripts(void)
{
	FILE *output = fopen("script_disassembly.txt", "wb");
	if(output == NULL)
	{
		MessageBox(NULL, "Cannot open output file for Script Disassembly.", "Error", MB_OK);
		return;
	}

	list<AIScriptDef>::iterator it;
	for(it = aiScriptManager.aiDef.begin(); it != aiScriptManager.aiDef.end(); ++it)
		it->OutputDisassemblyToFile(output);
	fclose(output);
	MessageBox(NULL, "Script disassembly saved.", "Success", MB_OK);
}

void Event_Click_Button_TestCrash(void)
{
	char *test = NULL;
	*test = 5;
}

void Event_Click_Button_VariableDump(void)
{
	Debug_FullDump();
}

void Event_Click_Button_FlushLog(void)
{
	LOG_FLUSH();
}

void Event_Click_Button_RefreshStats(void)
{
	LastRefresh = Refresh_RefreshStats;

	long httpsend = 0;
	long httprec = 0;
	long simsend = 0;
	long simrec = 0;
	long trec = 0;
	long tsnd = 0;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);
	rep.AddLine("Simulators in use: %d", Simulator.size());
	rep.AddLine("HTTP Full Connection Errors: %d", HTTPBaseServer.Debug_MaxFullConnections);
	rep.AddLine(NULL);
	rep.AddLine("Sent: %s in %d packets", GetDataSizeStr(g_PacketManager.mTotalBytesSent), g_PacketManager.mTotalPacketsSent);
	rep.AddLine("Dropped: %s in %d packets", GetDataSizeStr(g_PacketManager.mTotalBytesDropped), g_PacketManager.mTotalPacketsDropped);
	rep.AddLine("Clustered: %s in %d packets", GetDataSizeStr(g_PacketManager.mClusterPacketBytes), g_PacketManager.mClusterPackets);
	rep.AddLine("Incomplete: %s in %d packets", GetDataSizeStr(g_PacketManager.mTotalBytesIncomplete), g_PacketManager.mTotalPacketsIncomplete);
	rep.AddLine("Send Zero: %d", g_PacketManager.mTotalSendZero);
	rep.AddLine("Forced Delays: %d", g_PacketManager.mCountForceDelay);
	rep.AddLine("Forced Delays (waiting): %d", g_PacketManager.mCountForceDelayAck);
	rep.AddLine("Delayed: %d", g_PacketManager.mTotalWait);

	rep.AddLine(NULL);

	int TimeSpan = (g_ServerTime - g_ServerLaunchTime) / 1000;
	int Hour = TimeSpan / 3600;
	int Minute = (TimeSpan / 60) % 60;
	int Second = TimeSpan % 60;
	int Day = Hour / 24;
	Hour -= (Day * 24);

	rep.AddLine("Uptime: %02dd:%02dh:%02dm:%02ds", Day, Hour, Minute, Second);
	rep.AddLine(NULL);

	rep.AddLine("[Component] (Status) Sent bytes / Received bytes");
	rep.AddLine("[HTTP] (%s) %s / %s (total of deactivated threads)",
		StatusPhaseStrings[HTTPBaseServer.Status],
		GetDataSizeStr(g_HTTPDistributeManager.mTotalSendBytes),
		GetDataSizeStr(g_HTTPDistributeManager.mTotalRecBytes));

	httpsend += g_HTTPDistributeManager.mTotalSendBytes;
	httprec += g_HTTPDistributeManager.mTotalRecBytes;

	tsnd += g_HTTPDistributeManager.mTotalSendBytes;
	trec += g_HTTPDistributeManager.mTotalRecBytes;

	HTTPDistributeManager::ITERATOR hdit;

	if(g_HTTPDistributeManager.mDebugThreadCollision == true)
		g_Log.AddMessageFormatW(MSG_CRIT, "[CRITICAL] mDebugThreadCollision is true (Event_Click_Button_RefreshStats)");
	http_cs.Enter("Event_Click_Button_RefreshStats");
	for(hdit = g_HTTPDistributeManager.mDistributeList.begin(); hdit != g_HTTPDistributeManager.mDistributeList.end(); ++hdit)
	{
		rep.AddLine("[HTTP:%d] (%s) %s / %s (Use: %d, %d, %d)",
			hdit->InternalIndex,
			StatusPhaseStrings[hdit->Status],
			GetDataSizeStr(hdit->TotalSendBytes),
			GetDataSizeStr(hdit->TotalRecBytes),
			(int)hdit->inUse,
			(int)hdit->SendFile,
			(int)hdit->Finished);

		httpsend += hdit->TotalSendBytes;
		httprec += hdit->TotalRecBytes;
		tsnd += hdit->TotalSendBytes;
		trec += hdit->TotalRecBytes;
	}
	http_cs.Leave();

	rep.AddLine("[Router] (%s) %s / %s",
		StatusPhaseStrings[Router.Status],
		GetDataSizeStr(Router.TotalSendBytes),
		GetDataSizeStr(Router.TotalRecBytes));
	tsnd += Router.TotalSendBytes;
	trec += Router.TotalRecBytes;

	rep.AddLine("[SimB] (%s) %s / %s",
		StatusPhaseStrings[SimulatorBase.Status],
		GetDataSizeStr(SimulatorBase.TotalSendBytes),
		GetDataSizeStr(SimulatorBase.TotalRecBytes));
	tsnd += SimulatorBase.TotalSendBytes;
	trec += SimulatorBase.TotalRecBytes;

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		rep.AddLine("[Sim:%d] (%s) %s / %s (End: %d)",
			it->InternalIndex,
			StatusPhaseStrings[it->Status],
			GetDataSizeStr(it->TotalSendBytes),
			GetDataSizeStr(it->TotalRecBytes),
			it->MessageEnd );

		simsend += it->TotalSendBytes;
		simrec += it->TotalRecBytes;

		tsnd += it->TotalSendBytes;
		trec += it->TotalRecBytes;
	}

	rep.AddLine(NULL);
	rep.AddLine("Total HTTP sent: %s", GetDataSizeStr(httpsend));
	rep.AddLine("Total HTTP received: %s", GetDataSizeStr(httprec));

	simsend += g_SimulatorManager.baseByteSent;
	simrec += g_SimulatorManager.baseByteRec;
	tsnd += g_SimulatorManager.baseByteSent;
	trec += g_SimulatorManager.baseByteRec;
	rep.AddLine(NULL);
	rep.AddLine("Deactivated Simulator sent: %s", GetDataSizeStr(g_SimulatorManager.baseByteSent));
	rep.AddLine("Deactivated Simulator received: %s", GetDataSizeStr(g_SimulatorManager.baseByteRec));
	rep.AddLine(NULL);
	rep.AddLine("Total Simulator sent: %s", GetDataSizeStr(simsend));
	rep.AddLine("Total Simulator received: %s", GetDataSizeStr(simrec));
	rep.AddLine(NULL);
	rep.AddLine("Grand Total sent: %s", GetDataSizeStr(tsnd));
	rep.AddLine("Grand Total received: %s", GetDataSizeStr(trec));

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void RefreshChatBox(void)
{
	int oldRead = ChatLogIndex.ReadPos;
	int oldPending = ChatLogIndex.PendingRead;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	while(ChatLogIndex.PendingRead > 0)
	{
		int r = ChatLogIndex.GetReadIndex();
		rep.AddLine("%s", ChatLogString[r].c_str());
	};

	ChatLogIndex.ReadPos = oldRead;
	ChatLogIndex.PendingRead = oldPending;

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());

	HRESULT hres;
	hres = SendMessage(MainWindowControlSet[MWCS_Edit_ChatBox], EM_GETLINECOUNT, NULL, NULL);
	SendMessage(MainWindowControlSet[MWCS_Edit_ChatBox], EM_LINESCROLL, NULL, hres);
}

void Event_Click_Button_ClearChat(void)
{
	ChatLogIndex.ReadPos = 0;
	ChatLogIndex.WritePos = 0;
	ChatLogIndex.PendingRead = 0;

	RefreshChatBox();
}

void Event_Click_Button_RefreshPlayers(void)
{
	LastRefresh = Refresh_RefreshPlayers;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true && it->ProtocolState == 1)
		{
			rep.AddLine("%s (%d, %d, %d) [%s] [Sim:%d] [ID:%d, CDef: %d]", 
				it->creatureInst->css.display_name,
				it->creatureInst->CurrentX,
				it->creatureInst->CurrentY,
				it->creatureInst->CurrentZ,
				it->pld.zoneDef->mWarpName.c_str(),
				it->InternalID,
				it->creatureInst->CreatureID,
				it->creatureInst->CreatureDefID);
		}
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshPrefs(void)
{
	size_t i;
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	SimulatorThread *simPtr = GetDebugSimulator();
	if(simPtr == NULL)
	{
		rep.AddLine("Must select a simulator index (use Main tab)");
	}
	else
	{
		CharacterData *charData = simPtr->pld.charPtr;
		if(charData != NULL)
		{
			rep.AddLine("Preferences for: %s [CID:%d]", charData->cdef.css.display_name, charData->cdef.CreatureDefID);
			rep.AddLine(NULL);
			for(i = 0; i < charData->preferenceList.PrefList.size(); i++)
				rep.AddLine("%s=%s", charData->preferenceList.PrefList[i].name.c_str(), charData->preferenceList.PrefList[i].value.c_str());
		}
		else
		{
			rep.AddLine("Sim:%d contains an invalid character pointer", simPtr->InternalID);
		}
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	if(rep.getLength() > sizeof(WindowTextBuffer))
		MessageBox(NULL, "Checkpoint", "Checkpoint", 0);

	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshInventory(void)
{
	size_t a, b;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	SimulatorThread *simPtr = GetDebugSimulator();
	if(simPtr == NULL)
		return;

	CharacterData *charData = simPtr->pld.charPtr;
	if(charData != NULL)
	{
		for(a = 0; a < MAXCONTAINER; a++)
		{
			for(b = 0; b < charData->inventory.containerList[a].size(); b++)
			{
				ItemDef *itemPtr = g_ItemManager.GetPointerByID(charData->inventory.containerList[a][b].IID);
				rep.AddLine("%s=%d,%d (CCSID:%d, [%s] C:%d)",
					GetContainerNameFromID((charData->inventory.containerList[a][b].CCSID & CONTAINER_ID) >> 16),
					charData->inventory.containerList[a][b].CCSID & CONTAINER_SLOT,
					charData->inventory.containerList[a][b].IID,
					charData->inventory.containerList[a][b].CCSID,
					(itemPtr != NULL) ? itemPtr->mDisplayName.c_str() : "Invalid item",
					charData->inventory.containerList[a][b].count);
			}
		}
	}
	else
	{
		rep.AddLine("Sim:%d contains an invalid character reference", simPtr->InternalID);
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshInstance(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	Report::RefreshInstance(rep);

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshPointer(void)
{
	size_t a, b;
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true)
		{
			rep.AddLine("Sim:%d  Def:%p  cInst:%p (aInst: %p)", it->InternalID, &it->defcInst, it->creatureInst, it->creatureInst->actInst);
		}
	}

	rep.AddLine(NULL);
	rep.AddLine("Active Instances:");
	for(a = 0; a < g_ActiveInstanceManager.instListPtr.size(); a++)
	{
		ActiveInstance *aInst = g_ActiveInstanceManager.instListPtr[a];
		rep.AddLine("[%d] = %d (%p)", a, aInst->mInstanceID, aInst);
		for(b = 0; b < aInst->PlayerListPtr.size(); b++)
		{
			rep.AddLine("  [%d] = CDef:%d  CID:%d (%s)  Ptr:%p  Party:%d",
				b,
				aInst->PlayerListPtr[b]->CreatureDefID,
				aInst->PlayerListPtr[b]->CreatureID,
				aInst->PlayerListPtr[b]->css.display_name,
				aInst->PlayerListPtr[b],
				aInst->PlayerListPtr[b]->PartyID);
		}
	}
	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshSpawnPackages(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	int wpos = 0;
	for(size_t a = 0; a < g_SpawnPackageManager.packageList.size(); a++)
	{
		rep.AddLine("Zone: %d", g_SpawnPackageManager.packageList[a].ZoneID);
		for(size_t b = 0; b < g_SpawnPackageManager.packageList[a].defList.size(); b++)
		{
			SpawnPackageDef *ptr = &g_SpawnPackageManager.packageList[a].defList[b];
			rep.AddLine("%s: ", ptr->packageName);
			for(int c = 0; c < ptr->spawnCount; c++)
				rep.AppendLine("  %d:%d", ptr->spawnID[c], ptr->spawnShare[c]);

			rep.AddLine(NULL);
			if(rep.WasTruncated() == true)
				break;
		}
		rep.AddLine(NULL);
		if(rep.WasTruncated() == true)
			break;
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshSpawn(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	for(size_t i = 0; i < g_ActiveInstanceManager.instListPtr.size(); i++)
	{
		ActiveInstance *aInst = g_ActiveInstanceManager.instListPtr[i];
		rep.AddLine("Instance: %d (Zone:%d]:", aInst->mInstanceID, aInst->mZone);
		std::list<SpawnTile>::iterator it;
		SpawnTile::SPAWN_MAP::iterator spit;
		for(it = aInst->spawnsys.spawnTiles.begin(); it != aInst->spawnsys.spawnTiles.end(); ++it)
		{
			rep.AddLine("%d, %d : %d", it->TileX, it->TileY, it->activeSpawn.size());
			for(spit = it->activeSpawn.begin(); spit != it->activeSpawn.end(); ++spit)
				rep.AddLine("%d - %s (%g, %g, %g)", spit->second.refCount, spit->second.spawnPackage->packageName, spit->second.spawnPoint->LocationX, spit->second.spawnPoint->LocationY, spit->second.spawnPoint->LocationZ);
		}
	}
	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshTime(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);
	rep.AddLine("This session:");

	SIMULATOR_IT it;
	for(it = Simulator.begin(); it != Simulator.end(); ++it)
	{
		if(it->isConnected == true && it->ProtocolState == 1)
		{
			int TimeSpan = (g_ServerTime - it->TimeOnline) / 1000;
			int SHour = TimeSpan / 3600;
			int SMinute = (TimeSpan / 60) % 60;
			int SSecond = TimeSpan % 60;
			int TotalSec = it->pld.charPtr->SecondsLogged + TimeSpan;
			int THour = TotalSec / 3600;
			int TMinute = (TotalSec / 60) % 60;
			int TSecond = TotalSec % 60;
			rep.AddLine("%s [Session: %02d:%02d:%02d] [Total: %02d:%02d:%02d]",
				it->pld.charPtr->cdef.css.display_name,
				SHour, SMinute, SSecond,
				THour, TMinute, TSecond);
		}
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshParty(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	for(size_t p = 0; p < g_PartyManager.mPartyList.size(); p++)
	{
		ActiveParty *party = &g_PartyManager.mPartyList[p];
		rep.AddLine("[%d] ID:%d leader:%d", p, party->mPartyID, party->mLeaderID);
		for(size_t m = 0; m < party->mMemberList.size(); m++)
		{
			rep.AddLine("  [%d] id:%d (%s)", m, party->mMemberList[m].mCreatureID, party->mMemberList[m].mDisplayName.c_str());
		}
		rep.AddLine(NULL);
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshAIScripts(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	rep.AddLine("AI Script Definitions (%d):", aiScriptManager.aiDef.size());
	list<AIScriptDef>::iterator itDef;
	for(itDef = aiScriptManager.aiDef.begin(); itDef != aiScriptManager.aiDef.end(); ++itDef)
		rep.AddLine("  %s : %d ops (Ptr: %p)", itDef->scriptName.c_str(), itDef->instr.size(), &*itDef);

	rep.AddLine(NULL);

	rep.AddLine("Active AI Scripts (%d):", aiScriptManager.aiAct.size());
	list<AIScriptPlayer>::iterator itAct;
	for(itAct = aiScriptManager.aiAct.begin(); itAct != aiScriptManager.aiAct.end(); ++itAct)
		rep.AddLine("  Ptr: %p, Def Ptr: %p (%s), Instr: %d, Active: %d", &*itAct, itAct->def, itAct->def->scriptName.c_str(), itAct->curInst, itAct->active);

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshMods(void)
{
	SimulatorThread *simPtr = GetDebugSimulator();
	if(simPtr == NULL)
		return;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);
	simPtr->Debug_GenerateCreatureReport(rep);

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshHate(void)
{
	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);
	Report::RefreshHateProfile(rep);
	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshProfiler(void)
{
	ReportBuffer rep(ReportBuffer::NEWLINE_RN);
#ifndef DEBUG_PROFILER
	rpd.AddLine("Profiling not enabled.")
#else
	_DebugProfiler.GenerateReport(rep);
#endif
	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}


void Event_Click_Button_RefreshProps(void)
{
	LastRefresh = Refresh_RefreshProps;

	ReportBuffer rep(sizeof(WindowTextBuffer) - 1000, ReportBuffer::NEWLINE_RN);

	rep.AddLine("Zones: %d", g_SceneryManager.mZones.size());
	long total = 0;

	SceneryManager::ITERATOR zit;
	SceneryZone::PAGEMAP::iterator pit;
	for(zit = g_SceneryManager.mZones.begin(); zit != g_SceneryManager.mZones.end(); ++zit)
	{
		rep.AddLine("Zone: %d (pages:%d, size:%d)", zit->first, zit->second.mPages.size(), zit->second.mPageSize);
		for(pit = zit->second.mPages.begin(); pit != zit->second.mPages.end(); ++pit)
		{
			rep.AddLine("[%02d]: X:%02d, Z:%02d (Count: %d) [Pend: %d]", pit->second.mZone, pit->second.mTileX, pit->second.mTileY, (int)pit->second.mSceneryList.size(), pit->second.mPendingChanges);

			SceneryPage::SCENERY_IT::iterator propit;
			for(propit = pit->second.mSceneryList.begin(); propit != pit->second.mSceneryList.end(); ++propit)
			{
				SceneryObject *so = &propit->second;
				//if(strstr(so->Asset, "SpawnPoint") != NULL)
					rep.AddLine("  ID: %d, Asset: %s (%g, %g, %g)", so->ID, so->Asset, so->LocationX, so->LocationY, so->LocationZ);
			}
			total += pit->second.mSceneryList.size();
			rep.AddLine(NULL);
			if(rep.WasTruncated() == true)
				break;
		}
	}

	rep.SetMaxSize(sizeof(WindowTextBuffer));
	rep.AddLine( "Total props: %d", total);
	rep.AddLine("Additive: %d", g_SceneryVars.SceneryAdditive);
	rep.AddLine("Pending Changes: %d", g_SceneryVars.PendingItems);

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_RefreshQuests(void)
{
	SimulatorThread *simPtr = GetDebugSimulator();
	if(simPtr == NULL)
		return;

	CharacterData *charData = simPtr->pld.charPtr;
	if(charData == NULL)
		return;
	QuestJournal &qj = charData->questJournal;

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	size_t i;
	rep.AddLine("Active Quests: %d", qj.activeQuests.itemList.size());
	for(i = 0; i < qj.activeQuests.itemList.size(); i++)
	{
		int QID = qj.activeQuests.itemList[i].QuestID;
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QID);
		if(qdef != NULL)
			rep.AddLine("%d = %s", QID, qdef->title.c_str());
		else
			rep.AddLine("%d = QUEST NOT FOUND", QID);
	}

	rep.AddLine(NULL);
	rep.AddLine("Completed Quests: %d", qj.completedQuests.itemList.size());
	for(i = 0; i < qj.completedQuests.itemList.size(); i++)
	{
		int QID = qj.completedQuests.itemList[i].QuestID;
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QID);
		if(qdef != NULL)
			rep.AddLine("%d = %s", QID, qdef->title.c_str());
		else
			rep.AddLine("%d = QUEST NOT FOUND", QID);
	}

	rep.AddLine(NULL);
	rep.AddLine("Available Quests: %d", qj.availableQuests.itemList.size());
	for(i = 0; i < qj.availableQuests.itemList.size(); i++)
	{
		int QID = qj.availableQuests.itemList[i].QuestID;
		QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QID);
		if(qdef != NULL)
			rep.AddLine("%d = %s", QID, qdef->title.c_str());
		else
			rep.AddLine("%d = QUEST NOT FOUND", QID);
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}

void Event_Click_Button_ChatSend(void)
{
	char Name[64] = {0};
	char Channel[64] = {0};
	char Message[256] = {0};

	GetWindowText(MainWindowControlSet[MWCS_Edit_ChatName], Name, sizeof(Name));
	GetWindowText(MainWindowControlSet[MWCS_Edit_ChatChannel], Channel, sizeof(Channel));
	GetWindowText(MainWindowControlSet[MWCS_Edit_ChatMessage], Message, sizeof(Message));

	handleCommunicationMsg(Channel, Message, Name);
	SetWindowText(MainWindowControlSet[MWCS_Edit_ChatMessage], "");
}

void Event_Click_Button_ReloadQuests(void)
{
	QuestDef.Clear();
	QuestDef.LoadQuestPackages(Platform::GenerateFilePath(WindowTextBuffer, "Packages", "QuestPack.txt"));
	g_Log.AddMessageFormat("Loaded %d Quests.", QuestDef.mQuests.size());

	QuestScript::ClearQuestScripts();
	QuestScript::LoadQuestScripts(Platform::GenerateFilePath(WindowTextBuffer, "Data", "QuestScript.txt"));
}

void Event_Click_Button_ReloadChecksum(void)
{
	g_FileChecksum.LoadFromFile(Platform::GenerateFilePath(WindowTextBuffer, "Data", "HTTPChecksum.txt"));
}

void Event_Click_Button_ReloadAbilities(void)
{
	g_AbilityManager.LoadData();
}

void Event_Click_Button_DamageTest(void)
{
	SimulatorThread *simPtr = GetDebugSimulator();
	if(simPtr == NULL)
		return;

	g_AbilityManager.DamageTest(simPtr->creatureInst);
}

void Event_Click_Button_RefreshChar(void)
{
	g_CharacterManager.GetThread("Event_Click_Button_RefreshChar");

	ReportBuffer rep(sizeof(WindowTextBuffer), ReportBuffer::NEWLINE_RN);

	CharacterManager::CHARACTER_MAP::iterator it;
	rep.AddLine("Characters: %d, Current Time:%d", g_CharacterManager.charList.size(), g_ServerTime);
	for(it = g_CharacterManager.charList.begin(); it != g_CharacterManager.charList.end(); ++it)
	{
		rep.AddLine("ID: %d, Name: %s, Expires: %d (%d)",
			it->first,
			it->second.cdef.css.display_name,
			it->second.expireTime,
			(it->second.expireTime != 0) ? (it->second.expireTime - g_ServerTime) / 1000 : 0);
	}
	g_CharacterManager.ReleaseThread();

	rep.AddLine(NULL);
	rep.AddLine("Accounts: %d", g_AccountManager.accountQuickData.size());
	rep.AddLine("Active Accounts:%d",  g_AccountManager.AccList.size());
	AccountManager::ACCOUNT_ITERATOR ait;
	for(ait = g_AccountManager.AccList.begin(); ait != g_AccountManager.AccList.end(); ++ait)
	{
		rep.AddLine("[%d]=%s", ait->ID, ait->Name);
	}

	rep.Truncate(sizeof(WindowTextBuffer), "[truncated]");
	SetWindowText(MainWindowControlSet[MWCS_Edit_Stats], rep.getData());
}



SimulatorThread* GetDebugSimulator(void)
{
	int SimID = GetIntFromControl(MainWindowControlSet[MWCS_Edit_SimulatorID]);
	if(SimID <= 0)
		if(Simulator.size() > 0)
			return &Simulator.front();

	return GetSimulatorByID(SimID);
}

#endif  //#ifdef USE_WINDOWS_GUI

