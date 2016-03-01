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
#include "../Config.h"
#include "../util/Log.h"

//
//AddFundsHandler
//

int AddFundsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	std::string err;

	if (query->args[0].compare("CREDITS") == 0
			|| query->args[0].compare("COPPER") == 0) {
		int amount = atoi(query->args[1].c_str());
		g_CharacterManager.GetThread("SimulatorThread::AddFunds");
		CreatureInstance *creature =
				query->args[3].compare(
						creatureInstance->charPtr->cdef.css.display_name) == 0 ?
						creatureInstance :
						creatureInstance->actInst->GetPlayerByName(
								query->args[3].c_str());
		CharacterStatSet *css;
		CharacterData *cd;
		if (creature == NULL) {
			cd = g_CharacterManager.GetCharacterByName(query->args[3].c_str());
			if (cd != NULL)
				css = &(cd->cdef.css);
		} else {
			cd = creature->charPtr;
			css = &creature->css;
		}

		g_AccountManager.cs.Enter("AddFundsHandler::handleQuery");
		AccountData *ad = g_AccountManager.FetchIndividualAccount(
				cd->AccountID);

		if (cd == NULL) {
			err = "Cannot find character.";
		} else if (ad == NULL) {
			err = "Cannot find account.";
		} else {
			if (css != NULL) {
				if (query->args[0].compare("COPPER") == 0) {
					css->copper += amount;
					if (css->copper < 0)
						css->copper = 0;
					if (creatureInstance != NULL)
						creatureInstance->SendStatUpdate(STAT::COPPER);
					cd->pendingChanges++;
				} else {
					if (g_Config.AccountCredits)
						css->credits = ad->Credits;
					css->credits += amount;
					if (css->credits < 0)
						css->credits = 0;
					if (g_Config.AccountCredits) {
						ad->Credits = css->credits;
						ad->PendingMinorUpdates++;
					} else
						cd->pendingChanges++;
					if (creatureInstance != NULL)
						creatureInstance->SendStatUpdate(STAT::CREDITS);
				}

				g_Logs.event->info("[SAGE] %v gave %v %v copper because '%v'",
						pld->charPtr->cdef.css.display_name, css->display_name,
						amount, query->args[2].c_str());
			} else {
				err = "Cannot find player.";
			}
		}
		g_AccountManager.cs.Leave();
		g_CharacterManager.ReleaseThread();
	}

	if (err.size() == 0)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	else {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Failed to add funds. %s", err.c_str());
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux2);
	}
}

