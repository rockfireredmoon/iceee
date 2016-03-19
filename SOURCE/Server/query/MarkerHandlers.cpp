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
#include "MarkerHandlers.h"
#include "../Config.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Components.h"

//
// MarkerListHandler
//

int MarkerListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Admin)
			&& !sim->CheckPermissionSimple(Perm_Account, Permission_Builder))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	if (query->args[0] == "zone") {
		// Do a reload so we get external updates too
		creatureInstance->actInst->worldMarkers.Reload();

		int wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
		wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index

		wpos += PutShort(&sim->SendBuf[wpos],
				creatureInstance->actInst->worldMarkers.WorldMarkerList.size());
		vector<WorldMarker>::iterator it;

		for (it = creatureInstance->actInst->worldMarkers.WorldMarkerList.begin();
				it != creatureInstance->actInst->worldMarkers.WorldMarkerList.end();
				++it) {
			wpos += PutByte(&sim->SendBuf[wpos], 4);
			sprintf(sim->Aux1, "%s", it->Name);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
			sprintf(sim->Aux1, "%d", creatureInstance->actInst->mZone);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
			sprintf(sim->Aux1, "%f %f %f", it->X, it->Y, it->Z);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
			sprintf(sim->Aux1, "%s", it->Comment);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
		}

		PutShort(&sim->SendBuf[1], wpos - 3);
		return wpos;
	} else {
		g_Logs.simulator->warn("TODO Implement non-zone marker list query-> %v",
				query->args[0].c_str());
	}
	return 0;
}
int MarkerEditHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	bool ok = sim->CheckPermissionSimple(Perm_Account, Permission_Admin)
			|| sim->CheckPermissionSimple(Perm_Account, Permission_Builder);
	if (!ok) {
		if (pld->zoneDef->mGrove == true
				&& pld->zoneDef->mAccountID != pld->accPtr->ID)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->args.size() < 5)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	vector<WorldMarker>::iterator it;
	for (it = creatureInstance->actInst->worldMarkers.WorldMarkerList.begin();
			it != creatureInstance->actInst->worldMarkers.WorldMarkerList.end();
			++it) {
		if (strcmp(it->Name, query->args[0].c_str()) == 0) {
			Util::SafeCopy(it->Name, query->args[2].c_str(), sizeof(it->Name));
			Util::SafeCopy(it->Comment, query->args[4].c_str(),
					sizeof(it->Comment));
			it->X = creatureInstance->CurrentX;
			it->Y = creatureInstance->CurrentY;
			it->Z = creatureInstance->CurrentZ;
			creatureInstance->actInst->worldMarkers.Save();
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		}
	}
	g_Logs.simulator->info("Creating new marker %v in zone %v at %v.",
			query->args[2].c_str(), creatureInstance->actInst->mZone,
			query->args[4].c_str());
	WorldMarker wm;
	Util::SafeCopy(wm.Name, query->args[2].c_str(), sizeof(wm.Name));
	Util::SafeCopy(wm.Comment, query->args[4].c_str(), sizeof(wm.Comment));
	wm.X = creatureInstance->CurrentX;
	wm.Y = creatureInstance->CurrentY;
	wm.Z = creatureInstance->CurrentZ;
	creatureInstance->actInst->worldMarkers.WorldMarkerList.push_back(wm);
	creatureInstance->actInst->worldMarkers.Save();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
int MarkerDelHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	bool ok = sim->CheckPermissionSimple(Perm_Account, Permission_Admin)
			|| sim->CheckPermissionSimple(Perm_Account, Permission_Builder);
	if (!ok) {
		if (pld->zoneDef->mGrove == true
				&& pld->zoneDef->mAccountID != pld->accPtr->ID)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	vector<WorldMarker>::iterator it;

	for (it = creatureInstance->actInst->worldMarkers.WorldMarkerList.begin();
			it != creatureInstance->actInst->worldMarkers.WorldMarkerList.end();
			++it) {
		if (strcmp(it->Name, query->args[0].c_str()) == 0) {
			creatureInstance->actInst->worldMarkers.WorldMarkerList.erase(it);
			creatureInstance->actInst->worldMarkers.Save();
			break;
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

