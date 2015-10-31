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

#include "GMHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"

//
//AddFundsHandler
//

int AddFundsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	if (query->args[0].compare("COPPER") == 0) {
		int amount = atoi(query->args[1].c_str());
		g_CharacterManager.GetThread("SimulatorThread::AddFunds");
		CreatureInstance *creature = query->args[3].compare(creatureInstance->charPtr->cdef.css.display_name) == 0 ? creatureInstance :
				creatureInstance->actInst->GetPlayerByName(query->args[3].c_str());
		if (creature != NULL) {
			creature->AdjustCopper(amount);
			Debug::Log("[SAGE] %s gave %s %d copper because '%s'",
					pld->charPtr->cdef.css.display_name,
					creature->charPtr->cdef.css.display_name, amount,
					query->args[2].c_str());
			g_CharacterManager.ReleaseThread();
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		} else {
			sim->SendInfoMessage("Player must be logged on to receive funds.",
					INFOMSG_ERROR);
		}
		g_CharacterManager.ReleaseThread();
	}
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Failed to add funds.");
}

