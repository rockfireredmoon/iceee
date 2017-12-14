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

#include "SpawnHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//SpawnCreateHandler
//

int SpawnCreateHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: spawn.create
	 Sent when a spawn is requested by /creaturebrowse
	 Args: [0] = Object Type (ex: "CREATURE")
	 [1] = CreatureDef ID.
	 */

	if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	//For now, just ignore the object type and spawn the desired creature def.
	int CDef = atoi(query->args[1].c_str());
	sim->AddMessage((long) creatureInstance, CDef, BCM_SpawnCreateCreature);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SpawnEmittersHandler
//

int SpawnEmittersHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//LogMessageL(MSG_SHOW, "spawn.emitters");
	//for(int i = 0; i < query->argCount; i++)
	//	LogMessageL(MSG_SHOW, "[%d]=%s", i, query->args[i].c_str());
	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}

//
//SpawnListHandler
//

int SpawnListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: spawn.list
	 Retrieves a list of spawn types that match the search query->
	 Args: [0] = Category (ex: "", "Creatures", "Packages", "NoAppearanceCreatures"
	 [1] = Search text to match.
	 */

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	const char *category = query->args[0].c_str();
	const char *search = query->args[1].c_str();

	//Response for each creature
	//  {id=row[0],name=row[1],type=mTypeMap[row[2]]};
	//  type should be "C" for Creature or "P" for Package.

	vector<int> resList;
	CreatureDef.GetSpawnList(category, search, resList);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);          //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);         //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], resList.size());         //Number of rows
	for (size_t a = 0; a < resList.size(); a++) {
		//3 elements: ID, Name (Level), Package Type
		wpos += PutByte(&sim->SendBuf[wpos], 3);

		sprintf(sim->Aux3, "%d", CreatureDef.NPC[resList[a]].CreatureDefID);
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux3);

		sprintf(sim->Aux3, "%s (%d)", CreatureDef.NPC[resList[a]].css.display_name,
				CreatureDef.NPC[resList[a]].css.level);
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux3);

		wpos += PutStringUTF(&sim->SendBuf[wpos], "C");
	}

	PutShort(&sim->SendBuf[1], wpos - 3);             //Set message size
	resList.clear();
	return wpos;
}


//
//SpawnPropertyHandler
//

int SpawnPropertyHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount < 2)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Query error.");

	//LogMessageL(MSG_SHOW, "spawn.property");
	//for(int i = 0; i < query->argCount; i++)
	//	LogMessageL(MSG_SHOW, "[%d]=[%s]", i, query->args[i].c_str());

	int propID = atoi(query->args[0].c_str());
	const char *propName = query->args[1].c_str();
	g_SceneryManager.GetThread("SimulatorThread::handle_query_spawn_property");
	//SceneryObject *so = g_SceneryManager.pageData.GlobalFindProp(propID, pld->CurrentZoneID, NULL);
	SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(pld->CurrentZoneID,
			propID, NULL);
	sim->Aux1[0] = 0;
	if (so != NULL) {
		if (so->extraData != NULL) {
			if (strcmp(propName, "spawnName") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData->spawnName, sizeof(sim->Aux1));
			else if (strcmp(propName, "leaseTime") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->leaseTime);
			else if (strcmp(propName, "spawnPackage") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData->spawnPackage, sizeof(sim->Aux1));
			else if(strcmp(propName, "dialog") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData->dialog, sizeof(sim->Aux1));
			else if (strcmp(propName, "mobTotal") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->mobTotal);
			else if (strcmp(propName, "maxActive") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->maxActive);
			else if (strcmp(propName, "aiModule") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData->aiModule, sizeof(sim->Aux1));
			else if (strcmp(propName, "maxLeash") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->maxLeash);
			else if (strcmp(propName, "loyaltyRadius") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->loyaltyRadius);
			else if (strcmp(propName, "wanderRadius") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->wanderRadius);
			else if (strcmp(propName, "despawnTime") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->despawnTime);
			else if (strcmp(propName, "sequential") == 0)
				sprintf(sim->Aux1, "%d", so->extraData->sequential);
			else if (strcmp(propName, "spawnLayer") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData->spawnLayer, sizeof(sim->Aux1));
			else
				g_Logs.simulator->warn(
						"[%v] spawn.property unknown request: [%v]", sim->InternalID,
						propName);
		} else
			g_Logs.simulator->warn(
					"[%v] spawn.property requested for standard object [%v, %v]",
					sim->InternalID, so->ID, so->Asset);
	}
	g_SceneryManager.ReleaseThread();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux1);
}
