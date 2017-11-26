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
#include "SceneryHandlers.h"
#include "../util/Log.h"
#include "../Instance.h"
#include "../Config.h"

//
// SceneryListHandler
//

int SceneryListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {/* Query: scenery.list
		 Args : 0=ZoneID, 1=TileX, 2=TileZ
		 */

	if (sim->HasQueryArgs(3) == false)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	int zone = query->GetInteger(0);
	int x = query->GetInteger(1);
	int y = query->GetInteger(2);

//	LogMessageL(MSG_SHOW, "[DEBUG] scenery.list: %d, %d, %d", zone, x, y);

	bool skipQuery = false;
	if (g_Config.ProperSceneryList == 0
			|| (sim->CheckPermissionSimple(Perm_Account, Permission_FastLoad) == true))
		skipQuery = true;

	g_SceneryManager.GetThread("SimulatorThread::handle_query_scenery_list");
	std::list<int> excludedProps;

	/* If this is a request for the players current zone and active instance, retrieve the list
	 * of props to exclude (that may be the result of script prop removal)
	 */
	if (creatureInstance != NULL && creatureInstance->actInst != NULL
			&& creatureInstance->actInst->mZone == zone) {
		excludedProps.insert(excludedProps.begin(),
				creatureInstance->actInst->RemovedProps.begin(),
				creatureInstance->actInst->RemovedProps.end());
	}

	g_SceneryManager.AddPageRequest(sim->sc.ClientSocket, query->ID, zone, x, y,
			skipQuery, excludedProps);
	g_SceneryManager.ReleaseThread();

	if (skipQuery == true)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	return 0;
}

//
// SceneryEditHandler
//
int SceneryEditHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: scenery.edit
	 Creates a new scenery object or edits an existing one.
	 Args : [variable]
	 */

	int WritePos = 0;

	WritePos = protected_helper_query_scenery_edit(sim, pld, query,
			creatureInstance);
	if (WritePos <= 0) {
		sim->SendInfoMessage(sim->GetErrorString(WritePos), INFOMSG_ERROR);
		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}
	return WritePos;
}

int SceneryEditHandler::protected_helper_query_scenery_edit(
		SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return QueryErrorMsg::GENERIC;

	if (pld->zoneDef == NULL)
		return QueryErrorMsg::GENERIC;

	SceneryObject prop;
	SceneryObject *propPtr = NULL;
	bool newProp = false;

	SceneryObject oldPropData;

	int PropID = 0;
	if (query->args[0].compare("NEW") != 0) {
		//If not a new prop, get the existing one.
		PropID = query->GetInteger(0);
		//LogMessageL(MSG_SHOW, "[DEBUG] scenery.edit: %d", PropID);

		g_SceneryManager.GetThread(
				"SimulatorThread::protected_helper_query_scenery_edit");
		//propPtr = g_SceneryManager.GetPropPtr(pld.CurrentZoneID, PropID, NULL);
		propPtr = g_SceneryManager.GlobalGetPropPtr(pld->CurrentZoneID, PropID,
		NULL);
		g_SceneryManager.ReleaseThread();

		if (propPtr == NULL)
			return QueryErrorMsg::PROPNOEXIST;
		prop.copyFrom(propPtr);

		//Save the existing prop data in case there's an error
		oldPropData.copyFrom(&prop);

		if (sim->HasPropEditPermission(&prop) == false) {
			int wpos = PrepExt_UpdateScenery(sim->SendBuf, &oldPropData);
			sim->AttemptSend(sim->SendBuf, wpos);
			return QueryErrorMsg::PROPLOCATION;
		}
	} else {
		//New prop, give it an ID.
		newProp = true;
		prop.ID = g_SceneryVars.BaseSceneryID + g_SceneryVars.SceneryAdditive++;
		SessionVarsChangeData.AddChange();
		g_Logs.simulator->debug("[%v] scenery.edit: (new) %v", sim->InternalID,
				prop.ID);
	}

	for (unsigned int i = 1; i < query->argCount; i += 2) {
		const char *field = query->args[i].c_str();
		const char *data = query->args[i + 1].c_str();

		if (strcmp(field, "asset") == 0)
			prop.SetAsset(data);
		else if (strcmp(field, "p") == 0) {
			prop.SetPosition(data);
		} else if (strcmp(field, "q") == 0) {
			prop.SetQ(data);
		} else if (strcmp(field, "s") == 0)
			prop.SetS(data);
		else if (strcmp(field, "flags") == 0)
			prop.Flags = atoi(data);
		else if (strcmp(field, "name") == 0)
			prop.SetName(data);
		else if (strcmp(field, "layer") == 0)
			prop.Layer = atoi(data);
		else if (strcmp(field, "patrolSpeed") == 0)
			prop.patrolSpeed = atoi(data);
		else if (strcmp(field, "patrolEvent") == 0)
			prop.SetPatrolEvent(data);
		else if (strcmp(field, "ID") == 0) {
			//Don't do anything for this, otherwise it might create a
			//duplicate entry.
			//prop.ID = atoi(data);
		} else if (prop.SetExtendedProperty(field, data) == true) {
			g_Logs.simulator->debug(
					"[%v] scenery.edit extended property was set [%v=%v]",
					sim->InternalID, field, data);
		} else
			g_Logs.simulator->error("Unknown property [%v] for scenery.edit",
					field);
	}

	if (strlen(prop.Asset) == 0)
		return QueryErrorMsg::PROPASSETNULL;

	//The client makes changes internally without waiting for confirmation.
	//If the prop edit fails, we need to send back the old prop data to force
	//the client to revert to its prior state.

	//if(pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID, pld.zoneDef->mPageSize, prop.LocationX, prop.LocationZ) == false)
	if (sim->HasPropEditPermission(&prop) == false) {
		if (newProp == false) {
			int wpos = PrepExt_UpdateScenery(sim->SendBuf, &oldPropData);
			sim->AttemptSend(sim->SendBuf, wpos);
		}
		return QueryErrorMsg::PROPLOCATION;
	}

	//Check valid asset
	if (g_SceneryManager.VerifyATS(prop) == false) {
		if (newProp == false) {
			int wpos = PrepExt_UpdateScenery(sim->SendBuf, &oldPropData);
			sim->AttemptSend(sim->SendBuf, wpos);
		}
		return QueryErrorMsg::PROPATS;
	}

	//Prop is good to add.
	g_SceneryManager.GetThread(
			"SimulatorThread::protected_helper_query_scenery_edit");

	bool isSpawnPoint = prop.IsSpawnPoint();

	SceneryObject *retProp = NULL;
	if (PropID == 0) {
		retProp = g_SceneryManager.AddProp(pld->zoneDef->mID, prop);
		//LogMessageL(MSG_SHOW, "Added prop: %d, %s", prop.ID, prop.Asset);
	} else {
		if (isSpawnPoint == true)
			creatureInstance->actInst->spawnsys.RemoveSpawnPoint(prop.ID);

		retProp = g_SceneryManager.ReplaceProp(pld->zoneDef->mID, prop);
		//LogMessageL(MSG_SHOW, "Replaced prop: %d, %s", prop.ID, prop.Asset);
	}

	// The spawn system needs to reference a stable prop location.  If it tries to
	// reference this prop, it will crash after this local instance is destructed.
	// So we use retProp instead, which returns with a valid pointer if AddProp()
	// or ReplaceProp() was successful.
	if (isSpawnPoint == true && retProp != NULL)
		creatureInstance->actInst->spawnsys.UpdateSpawnPoint(retProp);

	g_SceneryManager.ReleaseThread();

	int opType = (
			(newProp == true) ? SceneryAudit::OP_NEW : SceneryAudit::OP_EDIT);
	pld->zoneDef->AuditScenery(creatureInstance->css.display_name,
			pld->CurrentZoneID, &prop, opType);

	int wpos = PrepExt_UpdateScenery(sim->SendBuf, &prop);
	//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
// SceneryDeleteHandler
//
int SceneryDeleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int WritePos = 0;

	WritePos = protected_helper_query_scenery_delete(sim, pld, query,
			creatureInstance);
	if (WritePos <= 0) {
		sim->SendInfoMessage(sim->GetErrorString(WritePos), INFOMSG_ERROR);
		WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}
	return WritePos;
}

int SceneryDeleteHandler::protected_helper_query_scenery_delete(
		SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return QueryErrorMsg::GENERIC;

	int PropID = query->GetInteger(0);
	SceneryObject *propPtr = NULL;

	g_Logs.simulator->debug("[%v] scenery.delete: %v", sim->InternalID, PropID);

	g_SceneryManager.GetThread(
			"SimulatorThread::protected_helper_query_scenery_delete");
	//propPtr = g_SceneryManager.pageData.GlobalFindProp(PropID, pld.CurrentZoneID, NULL);
	propPtr = g_SceneryManager.GlobalGetPropPtr(pld->CurrentZoneID, PropID,
	NULL);
	g_SceneryManager.ReleaseThread();

	if (propPtr == NULL)
		return QueryErrorMsg::PROPNOEXIST;

	if (sim->HasPropEditPermission(propPtr) == false)
		return QueryErrorMsg::PROPLOCATION;
	/* OLD
	 if(pld.accPtr->CheckBuildPermissionAdv(pld.zoneDef->mID, pld.zoneDef->mPageSize, propPtr->LocationX, propPtr->LocationZ) == false)
	 return QueryErrorMsg::PROPLOCATION;
	 */

	pld->zoneDef->AuditScenery(creatureInstance->css.display_name,
			pld->CurrentZoneID, propPtr, SceneryAudit::OP_DELETE);

	//Spawn point must be deactivated before it is deleted, otherwise pointers
	//will be invalidated and it may crash.
	creatureInstance->actInst->spawnsys.RemoveSpawnPoint(PropID);

	g_SceneryManager.GetThread(
			"SimulatorThread::protected_helper_query_scenery_delete");
	//g_SceneryManager.pageData.DeleteProp(pld.CurrentZoneID, PropID);
	g_SceneryManager.DeleteProp(pld->CurrentZoneID, PropID);
	g_SceneryManager.ReleaseThread();

	//Generate a bare prop that only has the necessary data for a delete
	//operation.
	SceneryObject prop;
	prop.ID = PropID;
	prop.Asset[0] = 0;
	int wpos = PrepExt_UpdateScenery(sim->SendBuf, &prop);
	//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SceneryLinkAddHandler
//
int SceneryLinkAddHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {/* Query: scenery.link.add
		 Args : 3, [0] = PropID, [1] = PropID, [2] = type
		 */
	if (query->argCount < 3)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	int p1 = query->GetInteger(0);
	int p2 = query->GetInteger(1);
	int type = query->GetInteger(2);

	g_SceneryManager.GetThread(
			"SimulatorThread::handle_query_scenery_link_add");
	bool result = g_SceneryManager.UpdateLink(pld->CurrentZoneID, p1, p2, type);
	g_SceneryManager.ReleaseThread();

	if (result == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Link failed.");

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// SceneryLinkDelHandler
//
int SceneryLinkDelHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: scenery.link.del
	 Args : 2, [0] = PropID, [1] = PropID
	 */
	if (query->argCount < 2)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	int p1 = atoi(query->args[0].c_str());
	int p2 = atoi(query->args[1].c_str());

	g_Logs.simulator->debug("[%v] scenery.link.del: %v, %v", sim->InternalID,
			p1, p2);

	g_SceneryManager.GetThread(
			"SimulatorThread::handle_query_scenery_link_del");
	int r = g_SceneryManager.UpdateLink(pld->CurrentZoneID, p1, p2, -1);
	g_SceneryManager.ReleaseThread();

	if (r == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unlink failed.");

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
