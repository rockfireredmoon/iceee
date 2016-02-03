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
#include "SidekickHandlers.h"
#include "../Account.h"
#include "../Character.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Config.h"

//
// SidekickAddHandler
//

static int AddSidekick(SimulatorThread *sim, int CDefID, int type) {
	int exist = sim->pld.charPtr->CountSidekick(type);
	if (exist > 0) {
		sim->SendInfoMessage("You already have a pet.", INFOMSG_INFO);
		return 0;
	}
	SidekickObject skobj(CDefID);
	skobj.summonType = type;

	sim->pld.charPtr->AddSidekick(skobj);
	int r = sim->creatureInst->actInst->CreateSidekick(sim->creatureInst,
			skobj);
	if (r == -1)
		sim->SendInfoMessage("Server error: Invalid Creature ID for sidekick.",
				INFOMSG_ERROR);
	return r;
}
int SidekickAddHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: skadd
	 Cheat to add a sidekick.
	 Args : 0 or 1, [0] = CreatureDefID
	 */

	if (sim->CheckPermissionSimple(Perm_Account, Permission_Admin) == false
			&& sim->CheckPermissionSimple(Perm_Account, Permission_Sage)
					== false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	int CDefID = 0;
	int type = SidekickObject::GENERIC;
	if (query->argCount > 0) {
		if(Util::CaseInsensitiveStringCompare("pet", query->args[0])) {
			type = SidekickObject::PET;
		}
		else if(Util::CaseInsensitiveStringCompare("quest", query->args[0])) {
			type = SidekickObject::QUEST;
		}
		else if(Util::CaseInsensitiveStringCompare("ability", query->args[0])) {
			type = SidekickObject::ABILITY;
		}
		else if(Util::CaseInsensitiveStringCompare("generic", query->args[0])) {
			type = SidekickObject::GENERIC;
		}
		else {
			type = atoi(query->args[0].c_str());
		}
	}
	if (query->argCount < 2) {
		if (creatureInstance->CurrentTarget.targ != NULL)
			CDefID = creatureInstance->CurrentTarget.targ->CreatureDefID;
	} else
		CDefID = atoi(query->args[1].c_str());

	int wpos = 0;
	if (CDefID == 0)
		wpos = PrepExt_SendInfoMessage(sim->SendBuf,
				"Usage: /skadd [<type>] ID", INFOMSG_ERROR);
	else
		AddSidekick(sim, CDefID, type);

	//UpdateSidekick(SidekickObject::PET, CDefID);

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
// SidekickRemoveHandler
//

int SidekickRemoveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: skremove
	 Cheat to remove a sidekick (must be selected).
	 Args : 0 or 1, [0] = CreatureDefID
	 */

	int wpos = 0;
	if (creatureInstance->CurrentTarget.targ == NULL) {
		wpos = PrepExt_SendInfoMessage(sim->SendBuf,
				"You must select a sidekick to remove.", INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");
		sim->PendingSend = true;
		return wpos;
	}

	int r = creatureInstance->actInst->SidekickRemoveOne(creatureInstance,
			&pld->charPtr->SidekickList);

	wpos = 0;
	if (r == -1)
		wpos = PrepExt_SendInfoMessage(sim->SendBuf,
				"Server error: Target is not a sidekick.", INFOMSG_ERROR);

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
// SidekickRemoveAllHandler
//

int SidekickRemoveAllHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: skremoveall
	 Cheat to remove all sidekicks.
	 Args : none
	 */
	creatureInstance->actInst->SidekickRemoveAll(creatureInstance,
			&pld->charPtr->SidekickList);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SidekickAttackHandler
//

int SidekickAttackHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	creatureInstance->RemoveNoncombatantStatus("skattack");
	sim->AddMessage((long) creatureInstance, 0, BCM_SidekickAttack);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SidekickCallHandler
//

int SidekickCallHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	sim->AddMessage((long) creatureInstance, 0, BCM_SidekickCall);
	sim->PendingSend = true;
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SidekickWarpHandler
//

int SidekickWarpHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	sim->AddMessage((long) creatureInstance, 0, BCM_SidekickWarp);
	sim->PendingSend = true;
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
// SidekickScatterHandler
//

int SidekickScatterHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	sim->AddMessage((long) creatureInstance, 0, BCM_SidekickScatter);
	sim->PendingSend = true;
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SidekickLowHandler
//

int SidekickLowHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: sklow
	 Select the sidekick with the lowest health.
	 Args : [0] = optional, health percentage threshold to search below.
	 */
	int percent = 99;
	if (query->argCount > 0)
		percent = Util::ClipInt(atoi(query->args[0].c_str()), 1, 99);

	int r = creatureInstance->actInst->SidekickLow(creatureInstance, percent);

	int wpos = 0;
	if (r == 1) {
		//Success, the current target will have been changed to the sidekick
		//with the lowest percent health.
		//Send a target change notification back to the Simulator.
		wpos = PrepExt_ChangeTarget(&sim->SendBuf[wpos],
				creatureInstance->CreatureID,
				creatureInstance->CurrentTarget.targ->CreatureID);
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	sim->PendingSend = true;
	return wpos;
}

//
// SidekickPartyHandler
//

int SidekickPartyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: skparty
	 Create a virtual party with all sidekicks as members.  Virtual in this case means the health
	 bars appear in the client, but there is no server side activity whatsoever.
	 Args : none
	 */
	int WritePos = creatureInstance->actInst->SidekickParty(creatureInstance,
			sim->SendBuf);
	WritePos += PrepExt_QueryResponseString(&sim->SendBuf[WritePos], query->ID,
			"OK");
	sim->PendingSend = true;
	return WritePos;
}
