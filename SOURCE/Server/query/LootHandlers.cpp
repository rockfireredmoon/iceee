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

#include "LootHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//LootListHandler
//

int LootListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: loot.list
	 Retrieves a list of loot for a particular dead creature.
	 Args : [0] = Creature Instance ID.
	 */

	if (query->argCount < 1)
		return 0;

	int WritePos = 0;
	int CreatureID = query->GetInteger(0);
	int r = creatureInstance->actInst->lootsys.GetCreature(CreatureID);
	if (r == -1)
		return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				"Creature does not have any loot.");
	else
		return creatureInstance->actInst->lootsys.WriteLootQueryToBuffer(r,
				sim->SendBuf, sim->Aux2, query->ID);

}

//
//LootItemHandler
//

int LootItemHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.item
	 Loots a particular item off a creature.
	 Args : [0] = Creature Instance ID
	 [1] = Item Definition ID
	 */
	if (query->argCount >= 2) {
		int result = sim->protected_helper_query_loot_item();
		if (result < 0)
			return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
					sim->GetErrorString(result));
	}
	return 0;
}

//
//LootExitHandler
//

int LootExitHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.exit
	 Signals the server that the loot window has been closed.
	 The server currently does not process this information.
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//LootNeedGreedPassHandler
//

int LootNeedGreedPassHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: loot.need
	 Signals players interested in being needy.
	 Args : [0] = Loot tag
	 */

	if (query->argCount < 2)
		return 0;

	int WritePos = 0;
	WritePos = protected_helper_query_loot_need_greed_pass(sim, pld, query, creatureInstance);
	if (WritePos <= 0)
		WritePos = PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				sim->GetErrorString(WritePos));
	return WritePos;
}


int LootNeedGreedPassHandler::protected_helper_query_loot_need_greed_pass(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInstance->PartyID);
	if (party == NULL)
		return QueryErrorMsg::GENERIC;

	if (query->argCount < 2)
		return QueryErrorMsg::GENERIC;

	ActiveInstance *aInst = creatureInstance->actInst;
	int lootTag = atoi(query->args[1].c_str());
	LootTag *tag = party->lootTags[lootTag];
	if (tag == NULL) {
		g_Logs.simulator->warn("[%v] Loot tag missing %v", sim->InternalID, lootTag);
		return QueryErrorMsg::INVALIDITEM;
	}
	ActiveLootContainer *loot =
			&aInst->lootsys.creatureList[tag->mLootCreatureId];
	if (tag == NULL) {
		g_Logs.simulator->warn("[%v] Loot container missing %v", sim->InternalID,
				tag->mLootCreatureId);
		return QueryErrorMsg::INVALIDITEM;
	}
	g_Logs.simulator->info("[%v] %v is choosing on %v (%v / %v)", sim->InternalID,
			creatureInstance->CreatureID, lootTag, tag->mItemId, tag->mCreatureId,
			tag->mLootCreatureId);
	if (loot->HasAnyDecided(tag->mItemId, creatureInstance->CreatureID)) {
		g_Logs.simulator->warn("[%v] %v has already made loot decision on %v",
				sim->InternalID, creatureInstance->CreatureID, tag->mItemId);
		return QueryErrorMsg::LOOTDENIED;
	}
	if (tag->mCreatureId != creatureInstance->CreatureID) {
		g_Logs.simulator->warn(
				"[%d] Loot tag %d is for a different creature. The tag said %d, but this player is %d.",
				sim->InternalID, lootTag, tag->mCreatureId,
				creatureInstance->CreatureID);
		return QueryErrorMsg::LOOTDENIED;
	}

	const char *command = query->args[0].c_str();
	if (strcmp(command, "loot.need") == 0) {
		loot->Need(tag->mItemId, tag->mCreatureId);
	} else if (strcmp(command, "loot.greed") == 0) {
		loot->Greed(tag->mItemId, tag->mCreatureId);
	} else if (strcmp(command, "loot.pass") == 0) {
		loot->Pass(tag->mItemId, tag->mCreatureId);
	}
	sim->CheckIfLootReadyToDistribute(loot, tag);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
