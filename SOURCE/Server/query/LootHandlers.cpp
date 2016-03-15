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
#include "../util/Log.h"
#include "../Instance.h"

//
//LootListHandler
//

int LootListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.list
	 Retrieves a list of loot for a particular dead creature.
	 Args : [0] = Creature Instance ID.
	 */

	if (sim->HasQueryArgs(1) == false)
		return 0;

	int WritePos = 0;

	int CreatureID = atoi(query->args[0].c_str());

	int r = creatureInstance->actInst->lootsys.GetCreature(CreatureID);
	if (r == -1)
		WritePos = PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				"Creature does not have any loot.");
	else
		WritePos = creatureInstance->actInst->lootsys.WriteLootQueryToBuffer(r,
				sim->SendBuf, sim->Aux2, query->ID);

	return WritePos;
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
//LootItemHandler
//

int LootItemHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: loot.item
	 Loots a particular item off a creature.
	 Args : [0] = Creature Instance ID
	 [1] = Item Definition ID
	 */

	if (sim->HasQueryArgs(2) == false)
		return 0;

	int WritePos = 0;

	int result = sim->protected_helper_query_loot_item();
	if (result < 0)
		WritePos = PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "FAIL",
				sim->GetErrorString(result));
	return WritePos;
}

//
//LootNeedGreedHandler
//

int LootNeedGreedPassHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	// TODO temporary
	sim->handle_query_loot_need_greed_pass();
	return 0;
}
