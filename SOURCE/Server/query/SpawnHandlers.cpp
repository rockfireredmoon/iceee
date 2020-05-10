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

int SpawnCreateHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: spawn.create
	 Sent when a spawn is requested by /creaturebrowse and the new build mode
	 Args: [0] = Object Type (ex: "CREATURE" or "PACKAGE")
	 [1] = CreatureDef ID.
	 [2] = Name
	 [3] = Flags
	 [4] = Rotation
	 [5] = X
	 [6] = Y
	 [7] = Z
	 [8] = Permanent
	 [9] = QX
	 [10] = QY
	 [11] = QZ
	 [12] = QW

	 */

	if (!sim->CheckPermissionSimple(Perm_Account,
			Permission_Debug | Permission_Admin | Permission_Developer))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	std::string id = query->args[1];
	int CDef = atoi(id.c_str());
	std::string type = query->args[0];

	if (query->argCount > 2) {
		/* New style build mode */
		std::string name = query->args[2];
		int flags = atoi(query->args[3].c_str());
		int r = atoi(query->args[4].c_str());
		float x = atoi(query->args[5].c_str());
		float y = atoi(query->args[6].c_str());
		float z = atoi(query->args[7].c_str());

		if (query->args[8] == "true") {
			/* Permanent spawn */
			SceneryObject prop;
			prop.ID = pld->zoneDef->GetNextPropID();
			prop.Asset = "Manipulator-SpawnPoint";
			prop.ScaleX = 1;
			prop.Name = name;
			prop.ScaleY = 1;
			prop.ScaleZ = 1;
			prop.LocationX = x;
			prop.LocationY = y;
			prop.LocationZ = z;
			prop.QuatX = Util::StringToFloat(query->args[9]);
			prop.QuatY = Util::StringToFloat(query->args[10]);
			prop.QuatZ = Util::StringToFloat(query->args[11]);
			prop.QuatW = Util::StringToFloat(query->args[12]);
			prop.SetExtendedProperty("MaxActive", "1");
			prop.SetExtendedProperty("MaxLeash", "500");
			prop.SetExtendedProperty("DespawnTime", "150");
			if (type == "PACKAGE") {
				prop.SetExtendedProperty("SpawnPackage", name);
			} else {
				std::string flagStr = "";
				if ((flags & SpawnPackageDef::FLAG_ENEMY) != 0) {
					flagStr += "E";
				} else if ((flags & SpawnPackageDef::FLAG_FRIENDLY) != 0) {
					flagStr += "F";
				} else if ((flags & SpawnPackageDef::FLAG_FRIENDLY_ATTACK)
						!= 0) {
					flagStr += "E";
				} else if ((flags & SpawnPackageDef::FLAG_NEUTRAL) != 0) {
					flagStr += "N";
				}
				if ((flags & SpawnPackageDef::FLAG_HIDEMAP) != 0) {
					flagStr += "H";
				}
				if ((flags & SpawnPackageDef::FLAG_HIDE_NAMEBOARD) != 0) {
					flagStr += "B";
				}
				if ((flags & SpawnPackageDef::FLAG_KILLABLE) != 0) {
					flagStr += "K";
				}
				if ((flags & SpawnPackageDef::FLAG_VISWEAPON_MELEE) != 0) {
					flagStr += "M";
				} else if ((flags & SpawnPackageDef::FLAG_VISWEAPON_RANGED)
						!= 0) {
					flagStr += "R";
				}
				prop.SetExtendedProperty("SpawnPackage", "#" + flagStr + id);
			}

			if (sim->HasPropEditPermission(&prop) == false) {
				return QueryErrorMsg::PROPLOCATION;
			}

			//Check valid asset
			if (g_SceneryManager.VerifyATS(prop) == false) {
				return QueryErrorMsg::PROPATS;
			}

			//Prop is good to add.
			g_SceneryManager.GetThread(
					"SimulatorThread::protected_helper_query_scenery_edit");

			SceneryObject *retProp = g_SceneryManager.AddProp(pld->zoneDef->mID,
					prop);

			// The spawn system needs to reference a stable prop location.  If it tries to
			// reference this prop, it will crash after this local instance is destructed.
			// So we use retProp instead, which returns with a valid pointer if AddProp()
			// or ReplaceProp() was successful.
			if (retProp != NULL)
				creatureInstance->actInst->spawnsys.UpdateSpawnPoint(retProp);

			g_SceneryManager.ReleaseThread();

			int opType = (SceneryAudit::OP_NEW);
			pld->zoneDef->AuditScenery(creatureInstance->css.display_name,
					pld->CurrentZoneID, &prop, opType);

			int wpos = PrepExt_UpdateScenery(sim->SendBuf, &prop);
			//creatureInst->actInst->LSendToAllSimulator(SendBuf, wpos, -1);
			creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
					creatureInstance->CurrentX, creatureInstance->CurrentZ);

			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		} else {
			if (type == "PACKAGE") {
				SpawnPackageDef *def = g_SpawnPackageManager.GetPointerByName(
						name.c_str());
				if (def != NULL)
					creatureInstance->actInst->SpawnGeneric(
							def->GetRandomSpawn(NULL), (int) x, (int) y,
							(int) z, r, flags);
			} else {
				creatureInstance->actInst->SpawnGeneric(CDef, (int) x, (int) y,
						(int) z, r, flags);
			}

		}
	} else {
		/* Creature Browse */

		if (type == "PACKAGE") {
			SpawnPackageDef *def = g_SpawnPackageManager.GetPointerByName(
					id.c_str());
			sim->AddMessage((long) creatureInstance, def->GetRandomSpawn(NULL),
					BCM_SpawnCreateCreature);
			;
		} else {
			sim->AddMessage((long) creatureInstance, CDef,
					BCM_SpawnCreateCreature);
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SpawnEmittersHandler
//

int SpawnEmittersHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//LogMessageL(MSG_SHOW, "spawn.emitters");
	//for(int i = 0; i < query->argCount; i++)
	//	LogMessageL(MSG_SHOW, "[%d]=%s", i, query->args[i].c_str());
	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}

//
//SpawnListHandler
//

int SpawnListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: spawn.list
	 Retrieves a list of spawn types that match the search query->
	 Args: [0] = Category (ex: "", "Creatures", "Packages", "NoAppearanceCreatures"
	 [1] = Search text to match.
	 */

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	std::string category = query->args[0];
	std::string search = query->args[1];

	//Response for each creature
	//  {id=row[0],name=row[1],type=mTypeMap[row[2]]};
	//  type should be "C" for Creature or "P" for Package.

	vector<CreatureSearchResult> resList;
	CreatureDef.GetSpawnList(category, search, resList);

	g_Logs.simulator->info("[%d] Replying with [%d] spawn search results",
			sim->InternalID, resList.size());

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);          //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);     //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], resList.size());      //Number of rows
	for (size_t a = 0; a < resList.size(); a++) {
		//3 elements: ID, Name (Level), Package Type
		wpos += PutByte(&sim->SendBuf[wpos], 3);
		sprintf(sim->Aux3, "%d", resList[a].id);
		wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux3);
		wpos += PutStringUTF(&sim->SendBuf[wpos], resList[a].name.c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], resList[a].type.c_str());
	}

	PutShort(&sim->SendBuf[1], wpos - 3);             //Set message size
	resList.clear();
	return wpos;
}

//
//SpawnPropertyHandler
//

int SpawnPropertyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Query error.");

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
		if (so->hasExtraData) {
			if (strcmp(propName, "spawnName") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData.spawnName.c_str(),
						sizeof(sim->Aux1));
			else if (strcmp(propName, "leaseTime") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.leaseTime);
			else if (strcmp(propName, "spawnPackage") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData.spawnPackage.c_str(),
						sizeof(sim->Aux1));
			else if (strcmp(propName, "dialog") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData.dialog.c_str(),
						sizeof(sim->Aux1));
			else if (strcmp(propName, "mobTotal") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.mobTotal);
			else if (strcmp(propName, "maxActive") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.maxActive);
			else if (strcmp(propName, "aiModule") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData.aiModule.c_str(),
						sizeof(sim->Aux1));
			else if (strcmp(propName, "maxLeash") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.maxLeash);
			else if (strcmp(propName, "loyaltyRadius") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.loyaltyRadius);
			else if (strcmp(propName, "wanderRadius") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.wanderRadius);
			else if (strcmp(propName, "despawnTime") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.despawnTime);
			else if (strcmp(propName, "sequential") == 0)
				sprintf(sim->Aux1, "%d", so->extraData.sequential);
			else if (strcmp(propName, "spawnLayer") == 0)
				Util::SafeCopy(sim->Aux1, so->extraData.spawnLayer.c_str(),
						sizeof(sim->Aux1));
			else
				g_Logs.simulator->warn(
						"[%v] spawn.property unknown request: [%v]",
						sim->InternalID, propName);
		} else
			g_Logs.simulator->warn(
					"[%v] spawn.property requested for standard object [%v, %v]",
					sim->InternalID, so->ID, so->Asset);
	}
	g_SceneryManager.ReleaseThread();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux1);
}
