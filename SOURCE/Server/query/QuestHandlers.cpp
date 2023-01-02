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

#include "Query.h"
#include "QuestHandlers.h"
#include "../Quest.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../util/Log.h"

//
// QuestIndicatorHandler
//

int QuestIndicatorHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: quest.indicator
	 Part 1 of the quest accept procedure.
	 The client is requesting if an NPC has a quest icon over its head, and if so,
	 what kind.
	 Args : [0] = Creature Instance ID
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	//Quests cannot be interacted with in groves.
	if (creatureInstance->actInst->mZoneDefPtr->mGrove == true)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
				QuestIndicator::QueryResponse[QuestIndicator::NONE]);

	int CID = atoi(query->args[0].c_str());
	//LogMessageL(MSG_DIAGV, "  Request quest.indicator for %d", CID);

	int CDef = sim->ResolveCreatureDef(CID);

	const char *status = pld->charPtr->questJournal.QuestIndicator(CDef);
	//LogMessageL(MSG_SHOW, "  quest.indicator for %d (%d) = %s", CID, CDef, status);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, status);
}

//
// QuestGetOfferHandler
//

int QuestGetOfferHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: quest.getquestoffer
	 Part 2 of the quest accept procedure.
	 Sent when an NPC (with an available quest) is clicked.
	 Returns the Quest ID of a new quest associated with this NPC.
	 Args : [0] = Creature Instance ID
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int CID = atoi(query->args[0].c_str());

	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v]   Request quest.getquestoffer for %v",
				sim->InternalID, CID);
	}

	int CDef = sim->ResolveCreatureDef(CID);

	auto response = pld->charPtr->questJournal.QuestGetQuestOffer(CDef, sim->Aux3);

	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v]   quest.getquestoffer for %v = %v",
				sim->InternalID, CID, response);
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, response);
}


//
// QuestGenericDataHandler
//

int QuestGenericDataHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {


	/*  Query: quest.genericdata
	 Part 3 of the quest accept procedure.
	 This retrieves the quest data so that the player may review stuff like
	 description, objectives, and rewards before accepting the quest.
	 Args : [0] = Quest ID
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int QID = atoi(query->args[0].c_str());
	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v]   Requested quest.genericdata for %v",
				sim->InternalID, QID);
	}
	return QuestGenericData(sim->SendBuf, sizeof(sim->SendBuf), sim->Aux3, QID, query->ID);
}


//
// QuestJoinHandler
//

int QuestJoinHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: quest.join
	 Part 4 of the quest accept procedure.
	 This query is followed by "quest.list" to refresh the active quest list.
	 Signals the server that the player has reviewed and accepted the quest.
	 Args : [0] = Quest ID
	 [1] = Creature Instance ID of the NPC that issued the quest.
	 */
	if (query->argCount < 2)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK"); //Fail silently.

	int QuestID = query->GetInteger(0);
	int CID = query->GetInteger(1);
	CreatureInstance *giverNPC = sim->ResolveCreatureInstance(CID, 0);
	if (giverNPC == NULL)
		return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Quest giver does not exist.");

	if (ActiveInstance::GetPlaneRange(creatureInstance, giverNPC, sim->INTERACT_RANGE)
			> sim->INTERACT_RANGE)
		return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Quest giver not in range.");

	QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	if (qdef == NULL)
		return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Quest does not exist.");

	if (qdef->QuestGiverID != giverNPC->CreatureDefID)
		return sim->ErrorMessageAndQueryOK(sim->SendBuf,
				"That object does not give that quest.");

	if(qdef->accountQuest && pld->accPtr->HasAccountCompletedQuest(QuestID)) {
		return sim->ErrorMessageAndQueryOK(sim->SendBuf, "You have already completed this quest on another character, and it is a once-per-account quest type.");
	}

	if (qdef->mScriptAcceptCondition.ExecuteAllCommands(sim) < 0)
		return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Cannot accept the quest yet.");
	qdef->mScriptAcceptAction.ExecuteAllCommands(sim);

	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v]   Request quest.join (QuestID: %v, CID: %v)",
				sim->InternalID, QuestID, CID);
	}

	if (qdef->accountQuest) {
		g_AccountManager.cs.Enter("SimulatorThread::VaultSend");
		AccountData *acc = g_AccountManager.GetActiveAccountByID(
				creatureInstance->charPtr->AccountID);
		acc->AccountQuests.push_back(qdef->questID);
		acc->PendingMinorUpdates++;
		g_AccountManager.cs.Leave();
	}

	int wpos = pld->charPtr->questJournal.QuestJoin(sim->SendBuf, QuestID, query->ID);

	g_QuestNutManager.AddActiveScript(creatureInstance, QuestID);

	return wpos;
}


//
// QuestListHandler
//

int QuestListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {


	/*  Query: quest.list
	 Sent whenever the client needs to refresh the active quest list.
	 Args : [none]
	 */

	int wpos = QuestList(pld->charPtr->questJournal, sim->SendBuf, sim->Aux3, query->ID);
	return wpos;
}


//
// QuestDataHandler
//

int QuestDataHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {


	/*  Query: quest.data
	 Fetches the active quest data for a particular quest.  Often queried in
	 response to "quest.list" or when objectives are updated.
	 Args : [0] = Quest ID
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int QID = atoi(query->args[0].c_str());

	return QuestData(pld->charPtr->questJournal, sim->SendBuf, sim->Aux3, QID, query->ID);
}


//
// QuestGetCompleteHandler
//

int QuestGetCompleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {


	/*  Query: quest.getcompletequest
	 Sent when a quest is redeemed. The result will allow it to display
	 the quest ending screen, with completion text, reward selection, etc.
	 Args : [0] = Creature ID of the NPC
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int CID = atoi(query->args[0].c_str());

	if (g_Logs.simulator->enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v]   Request quest.getcompletequest for %v",
				sim->InternalID, CID);
	}

	// -1 means the quest is a quest without an ending creature
	int CDef = CID == creatureInstance->CreatureID ? -1 : sim->ResolveCreatureDef(CID);

	return pld->charPtr->questJournal.QuestGetCompleteQuest(sim->SendBuf, sim->Aux3, CDef,
			query->ID);
}


//
// QuestCompleteHandler
//

int QuestCompleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

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

		std::vector<size_t> selectionList;

		if (query->argCount > 0)
			QID = query->GetInteger(0);
		if (query->argCount > 1)
			CID = query->GetInteger(1);
		if (query->argCount > 2) {
			for (unsigned int i = 2; i < query->argCount; i++)
				selectionList.push_back((size_t)query->GetInteger(i));
		}

		QuestJournal *qj = pld->GetQuestJournal();
		if (qj == NULL)
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Critical server error.");

		int enderCreatureDefID = -1;
		if (CID == creatureInstance->CreatureID) {
		} else {
			CreatureInstance *questEnder = sim->ResolveCreatureInstance(CID, 1);
			if (questEnder == NULL)
				return sim->ErrorMessageAndQueryOK(sim->SendBuf,
						"Server error: creature does not exist.");

			if (ActiveInstance::GetBoxRange(creatureInstance, questEnder)
					> sim->INTERACT_RANGE)
				return sim->ErrorMessageAndQueryOK(sim->SendBuf,
						"You are too far away from the quest ender.");

			enderCreatureDefID = questEnder->CreatureDefID;
		}

		QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QID);
		if (qd == NULL)
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Quest not found.");

	//	QuestReference *qr = g_Q
		QuestReference *qr = qj->activeQuests.GetItem(qd->questID);
		if (qr == NULL) {
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "No quest reference.");
		}

		if (qj->IsQuestRedeemable(qd, QID, enderCreatureDefID) == false)
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Quest cannot be redeemed yet.");

		std::vector<QuestItemReward> rewardedItems;
		if (qd->FilterSelectedRewards(qr->Outcome, selectionList, rewardedItems)
				!= true)
			return sim->ErrorMessageAndQueryOK(sim->SendBuf,
					"Failed to select quest reward items.");

		int freeSlots = pld->charPtr->inventory.CountFreeSlots(INV_CONTAINER);
		if (rewardedItems.size() > static_cast<size_t>(freeSlots))
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "No free inventory space.");

		if (qd->mScriptCompleteCondition.ExecuteAllCommands(sim) < 0)
			return sim->ErrorMessageAndQueryOK(sim->SendBuf, "Cannot redeem the quest yet.");
		qd->mScriptCompleteAction.ExecuteAllCommands(sim);

		QuestScript::QuestNutPlayer *player = g_QuestNutManager.GetActiveScript(
				creatureInstance->CreatureID, QID);
		if (player != NULL) {
			player->RunFunction("on_complete",
					std::vector<ScriptCore::ScriptParam>(), false);
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
		 int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		 if(slot == -1)
		 return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "No free inventory space.");
		 }
		 */

		int response = pld->charPtr->questJournal.QuestComplete(QID);
		if (response == -1)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

		response = creatureInstance->ProcessQuestRewards(QID, qr->Outcome,
				rewardedItems);

		if (response < 0) {
			if (response == -2)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"No free inventory space.");

			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Unable to complete quest.");
		}

		int wpos = 0;

		wpos += PutByte(&sim->SendBuf[wpos], 7);   //_handleQuestEventMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);  //size
		wpos += PutInteger(&sim->SendBuf[wpos], QID);
		wpos += PutByte(&sim->SendBuf[wpos], QuestObjective::EVENTMSG_TURNEDIN);
		wpos += PutInteger(&sim->SendBuf[wpos], CID);
		PutShort(&sim->SendBuf[1], wpos - 3);  //size

		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");

		return wpos;


}

//
// QuestLeaveHandler
//

int QuestLeaveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {


	/* Query: quest.leave
	 Args : 1, [0] = Quest ID
	 */
	int QID = 0;
	if (query->argCount > 0)
		QID = query->GetInteger(0);

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QID);
	if (qd != NULL) {
		if (qd->unabandon == true) {
			sim->SendInfoMessage("You cannot abandon that quest.", INFOMSG_ERROR);
			QID = 0; //Set to zero so it's not actually removed in the server or the client.
		}
	}
	pld->charPtr->questJournal.QuestLeave(pld->CreatureID, QID);
	sprintf(sim->Aux1, "%d", QID);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux1);

}

//
// QuestHackHandler
//

int QuestHackHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	QuestDefinition *qdef;
	int QuestID = query->GetInteger(1);
	if (QuestID == 0) {
		qdef = QuestDef.GetQuestDefPtrByName(query->GetString(1));
	} else {
		qdef = QuestDef.GetQuestDefPtrByID(QuestID);
	}
	if (qdef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Quest does not exist.");

	g_CharacterManager.GetThread("SimulatorThread::QuestHack");
	CreatureInstance *creature;
	if (strcmp(query->GetString(2), "SELECT_TARGET") == 0) {
		creature = creatureInstance->CurrentTarget.targ;
		if (creature == NULL)
			creature = creatureInstance;
	} else {
		creature = creatureInstance->actInst->GetPlayerByID(query->GetInteger(2));
	}
	int WritePos = 0;
	if (creature == NULL) {
		g_Logs.simulator->warn(
				"No creature for  quest hack op for quest %v and creature %v.",
				query->GetString(1), query->GetString(2));
		WritePos = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Selected creature does not exist.");
	} else if (query->args[0].compare("add") == 0) {
		g_Logs.event->info("[SAGE] Quest join (QuestID: %v, CID: %v)",
				qdef->questID, creature->CreatureID);
		creature->simulatorPtr->QuestJoin(qdef->questID);
		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	} else if (query->args[0].compare("remove") == 0) {
		g_Logs.event->info("[SAGE] Quest remove (QuestID: %v, CID: %v)",
				qdef->questID, creature->CreatureID);
		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		creature->simulatorPtr->QuestClear(qdef->questID);
	} else if (query->args[0].compare("complete") == 0) {
		g_Logs.event->info("[SAGE] Quest complete (QuestID: %v, CID: %v)",
				qdef->questID, creature->CreatureID);
		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		char buf[1024];
		creature->simulatorPtr->AttemptSend(buf, creature->charPtr->questJournal.ForceComplete(creature->CreatureID, qdef->questID, buf));
	} else {
		g_Logs.simulator->warn(
				"Unknown quest hack op for quest %v and creature %v.",
				query->GetString(1), query->GetString(2));
		WritePos = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown quest hack op.");
	}
	g_CharacterManager.ReleaseThread();
	return WritePos;

}

//
// QuestShareHandler
//

int QuestShareHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
			return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	int questID = atoi(query->args[0].c_str());
	int res = pld->charPtr->questJournal.CheckQuestShare(questID);
	if (res != QuestJournal::SHARE_SUCCESS_QUALIFIES)
		sim->SendInfoMessage(QuestJournal::GetQuestShareErrorString(res),
				INFOMSG_ERROR);
	else {
		if (pld->charPtr->questJournal.QuestJoin_Helper(questID) == 0) {
			int wpos = QuestJournal::WriteQuestJoin(sim->SendBuf, questID);
			wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
					"You have joined the quest.", INFOMSG_INFO);
			sim->AttemptSend(sim->SendBuf, wpos);
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int QuestGenericData(char *buffer, int bufsize, char *convBuf, int QuestID, int QueryIndex) {
	//The client issues this query for the quest data after receiving the
	//quest ID from the "quest.getquestoffer" query.
	//Prepares a response buffer for the "quest.genericdata" query.
	//Returns the size of the buffer.

	//TODO: Very long quest data may cause a buffer overflow.  May want to handle
	//this more gracefully.
	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QuestID);
	if (qd == NULL) {
		g_Logs.server->error("Quest ID [%v] not found", QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex,
				"Server error: quest not found.");
	}

	QuestOutcome *outcome = qd->GetOutcome(0);

	int debug_check = qd->title.size() + qd->bodyText.size()
			+ outcome->compText.size();
	if (debug_check > bufsize - 100) {
		g_Logs.server->error(
				"QUEST DATA TOO LARGE FOR BUFFER (Quest ID:%d)",
				QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex,
				"Server error: too much data");
	}

	//Generic data has 23 rows.
	QueryResponse resp(QueryIndex);
	auto row = resp.Row();
	row->push_back(to_string(qd->questID)); //[0] = Quest ID
	row->push_back(qd->title);   //[1] = Title
	row->push_back(qd->bodyText);   //[2] = body
	row->push_back(outcome->compText); //[3] = completion text
	row->push_back(to_string(qd->levelSuggested));   //[4] = level
	row->push_back(to_string(outcome->experience));   //[5] = experience
	row->push_back(to_string(qd->partySize)); //[6] = party size
	row->push_back(to_string(outcome->numRewards));   //[7] = rewards
	row->push_back(to_string(qd->coin)); //[8] = coin
	row->push_back(StringFromBool(qd->unabandon)); //[9] = unabandon
	row->push_back(to_string(outcome->valourGiven)); //[10] = valour

	//3 sets of data, 3 elements each
	//  [0] = Objective text
	//  [1] = Complete: either "true" or "false"
	//  [2] = myItemID
	//Spans Rows: {11, 12, 13}, {14, 15, 16}, {17, 18, 19}
	int a;
	for (a = 0; a < MAXOBJECTIVES; a++) {
		row->push_back(qd->actList[0].objective[a].description);
		row->push_back(StringFromBool(qd->actList[0].objective[a].complete));
		row->push_back(to_string(qd->actList[0].objective[a].myItemID));
	}

	for (a = 0; a < 4; a++)
		row->push_back(outcome->rewardItem[a].Print(convBuf));

	return resp.Write(buffer);
}

int QuestList(QuestJournal &journal, char *buffer, char *convBuf, int QueryID) {
	//Fill a "quest.list" query with the appropriate response data.

	QueryResponse resp(QueryID);

	for (auto quest : journal.activeQuests.itemList) {
		int qid = quest.QuestID;
		QuestDefinition *qdef = quest.GetQuestPointer();
		if (qdef == NULL) {
			g_Logs.server->warn(
					"QuestList() Unknown active quest ID [%v]",
					qid);
		} else {
			auto row = resp.Row();
			row->push_back(to_string(qid));
			row->push_back(qdef->title);
			row->push_back(to_string(qdef->partySize));
		}
	}
	return resp.Write(buffer);
}

int QuestData(QuestJournal &journal, char *buffer, char *convBuf, int QuestID,
		int QueryIndex) {
	//The client issues the "quest.data" query when a quest is accepted, or when
	//objectives are refreshed.
	//Returns the size of the buffer.

	QuestDefinition *qd = QuestDef.GetQuestDefPtrByID(QuestID);
	if (qd == NULL) {
		g_Logs.server->error("Quest ID [%v] not found", QuestID);
		return PrepExt_QueryResponseError(buffer, QueryIndex,
				"Server error: quest not found.");
	}

	int act = journal.GetCurrentAct(QuestID);
	if (act >= (int) qd->actList.size())
		return PrepExt_QueryResponseError(buffer, QueryIndex,
				"Server error: quest act does not exist");
	else if (act < 0) {
		return PrepExt_QueryResponseError(buffer, QueryIndex,
				"Server error: quest act not set");
	}

	int QuestData = journal.activeQuests.HasQuestID(QuestID);
	QuestReference *qref = NULL;
	if (QuestData >= 0)
		qref = &journal.activeQuests.itemList[QuestData];

	QuestOutcome *outcome = qd->GetOutcome(qref == NULL ? 0 : qref->Outcome);
	QueryResponse response(QueryIndex);

	/*
	 if(r == -1)
	 {
	 g_Log.AddMessageFormat("[WARNING] Quest data not found for [%d]", questID);

	 for(int a = 0; a < 34; a++)
	 wpos += PutStringUTF(&buffer[wpos], "");

	 PutShort(&buffer[1], wpos - 3);
	 return wpos;
	 }*/
	auto row = response.Row();

	row->push_back(to_string(qd->questID)); //[0] = Quest ID
	row->push_back(qd->title);   //[1] = Title

	//Updated: The body text changes as you complete acts.
	//FIXED: it used to be a reference (string&) and was overwriting the body text.
	auto bodyText = qd->bodyText;
	if (qd->actList[act].BodyText.size() > 0)
		bodyText = qd->actList[act].BodyText;
	row->push_back(bodyText);   //[2] = body

	row->push_back(outcome->compText); //[3] = completion text
	row->push_back(to_string(qd->levelSuggested));   //[4] = level
	row->push_back(to_string(outcome->experience));   //[5] = experience
	row->push_back(to_string(qd->partySize)); //[6] = party size
	row->push_back(to_string(outcome->numRewards));   //[7] = rewards
	row->push_back(to_string(qd->coin)); //[8] = coin
	row->push_back(StringFromBool(qd->unabandon)); //[9] = unabandon
	row->push_back(to_string(outcome->valourGiven));   //[10] = valour

	/*
	 sprintf(ConvBuf, "%g,%g,%g,%d", qd->sGiver.x, qd->sGiver.y, qd->sGiver.z, qd->sGiver.zone);
	 wpos += PutStringUTF(&buffer[wpos], ConvBuf);   //[10] = giver

	 sprintf(ConvBuf, "%g,%g,%g,%d", qd->sEnder.x, qd->sEnder.y, qd->sEnder.z, qd->sEnder.zone);
	 wpos += PutStringUTF(&buffer[wpos], ConvBuf);   //[11] = ender
	 */
	row->push_back(qd->sGiver);  //[11]
	row->push_back(qd->sEnder);  //[12]

	//3 sets of data, 6 elements each
	// [13]   i+0  description.  If not empty, get the rest.
	// [14]   i+1  complete ("true", "false" ?)
	// [15]   i+2  myCreatureDefID
	// [16]   i+3  myItemID
	// [17]   i+4  completeText
	// [18]   i+5  markerLocations  "x,y,z,zone;x,y,z,zone;..."

	//Spans Rows: {13, 14, 15, 16, 17, 18}
	//            {19, 20, 21, 22, 23, 24}
	//            {25, 26, 27, 28, 29, 30}

	int a;
	for (a = 0; a < MAXOBJECTIVES; a++) {
		row->push_back(qd->actList[act].objective[a].description);

		//TODO: probably need to enforce updated objectives at all time
		int complete = qd->actList[act].objective[a].complete;
		if (qref != NULL) {
			complete = qref->ObjComplete[a];
		}

		row->push_back(StringFromBool(complete));
		row->push_back(to_string(qd->actList[act].objective[a].myCreatureDefID));
		row->push_back(to_string(qd->actList[act].objective[a].myItemID));

		//Check for updated objectives.
		convBuf[0] = 0;
		if (complete == 0) {
			if (qref != NULL) {
				if (qd->actList[act].objective[a].completeText.find(" of ")
						!= string::npos) {
					if (qd->actList[act].objective[a].type
							== QuestObjective::OBJECTIVE_TYPE_ACTIVATE
							|| qd->actList[act].objective[a].type
									== QuestObjective::OBJECTIVE_TYPE_KILL) {
						int need = qd->actList[act].objective[a].data2;
						int have = qref->ObjCounter[a];
						sprintf(convBuf, "%d of %d", have, need);
					}
				}
			}
		} else {
			strcpy(convBuf, "Complete");
		}
		if (convBuf[0] == 0)
			row->push_back(qd->actList[act].objective[a].completeText);
		else
			row->push_back(convBuf);

		row->push_back(qd->actList[act].objective[a].markerLocations);
	}

	// {31, 32, 33, 34}
	for (a = 0; a < 4; a++)
		row->push_back(outcome->rewardItem[a].Print(convBuf));

	return response.Write(buffer);
}
