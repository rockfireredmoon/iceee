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
	g_Logs.simulator->trace("[%v]   Request quest.getquestoffer for %v",
			sim->InternalID, CID);

	int CDef = sim->ResolveCreatureDef(CID);

	char *response = pld->charPtr->questJournal.QuestGetQuestOffer(CDef, sim->Aux3);
	g_Logs.simulator->trace("[%v]   quest.getquestoffer for %v = %v",
			sim->InternalID, CID, response);
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
	g_Logs.simulator->trace("[%v]   Requested quest.genericdata for %v",
			sim->InternalID, QID);
	return pld->charPtr->questJournal.QuestGenericData(sim->SendBuf, sizeof(sim->SendBuf),
			sim->Aux3, QID, query->ID);
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

	g_Logs.simulator->trace("[%v]   Request quest.join (QuestID: %v, CID: %v)",
			sim->InternalID, QuestID, CID);

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

	int wpos = pld->charPtr->questJournal.QuestList(sim->SendBuf, sim->Aux3, query->ID);
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

	return pld->charPtr->questJournal.QuestData(sim->SendBuf, sim->Aux3, QID, query->ID);
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
	g_Logs.simulator->trace("[%v]   Request quest.getcompletequest for %v",
			sim->InternalID, CID);

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

		std::vector<int> selectionList;

		if (query->argCount > 0)
			QID = query->GetInteger(0);
		if (query->argCount > 1)
			CID = query->GetInteger(1);
		if (query->argCount > 2) {
			for (unsigned int i = 2; i < query->argCount; i++)
				selectionList.push_back(query->GetInteger(i));
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
		g_Log.AddMessageFormat(
				"No creature for  quest hack op for quest %s and creature %s.",
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
		int wpos = creature->charPtr->questJournal.ForceComplete(
				creature->CreatureID, qdef->questID, &sim->Aux1[0]);
		creature->simulatorPtr->AttemptSend(sim->Aux1, wpos);
	} else {
		g_Log.AddMessageFormat(
				"Unknown quest hack op for quest %s and creature %s.",
				query->GetString(1), query->GetString(2));
		WritePos = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown quest hack op.");
	}
	g_CharacterManager.ReleaseThread();
	return WritePos;

}


