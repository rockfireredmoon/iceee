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

#include "ZoneHandlers.h"
#include "../Creature.h"
#include "../Cluster.h"
#include "../Instance.h"
#include "../InstanceScale.h"
#include "../Interact.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Scheduler.h"
#include "../ZoneObject.h"
#include "../util/Log.h"
#include <cmath>

//
//GoHandler
//
int GoHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: go
	 Sent by various actions on the client to warp to another place. The exact
	 semantics will depend on the number of arguments
	 */

	if (pld->zoneDef->mGrove == false)
		if (!sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer))
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	int id = creatureInstance->actInst->mZone;
	int instance = pld->CurrentInstanceID;
	int x = creatureInstance->CurrentX;
	int y = creatureInstance->CurrentY;
	int z = creatureInstance->CurrentZ;

	if (query->argCount == 3) {
		x = static_cast<int>(query->GetFloat(0)); //The arguments come in as floats but we use ints.
		y = static_cast<int>(query->GetFloat(1));
		z = static_cast<int>(query->GetFloat(2));
	}
	else if (query->argCount == 4) {
		id = query->GetInteger(0);
		if(id == 0) {
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByExactWarpName(query->GetString(0));
			if(zd == NULL) {
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"No such zone.");
			}
			id = zd->mID;
		}
		x = static_cast<int>(query->GetFloat(1)); //The arguments come in as floats but we use ints.
		y = static_cast<int>(query->GetFloat(2));
		z = static_cast<int>(query->GetFloat(3));
		if(x == 0 && y == 0 && z == 0) {
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(id);
			if(zd != NULL) {
				x = zd->DefX;
				y = zd->DefY;
				z = zd->DefZ;
			}
		}
		instance = 0;
	}
	else if (query->argCount == 1) {
		string n = query->GetString(0);
		CreatureInstance *def;
		if(n == "WARP_TARGET" && creatureInstance->CurrentTarget.targ != NULL) {
			def = creatureInstance->CurrentTarget.targ;
		}
		else if(Util::HasBeginning(n, "Player/")) {
			def = g_ActiveInstanceManager.GetPlayerCreatureByDefID(Util::SafeParseInt(n.substr(7)));
		}
		if(def == NULL) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"No such player.");
		}
		else {
			id = def->actInst->mZone;
			instance = def->actInst->mInstanceID;
			x = def->CurrentX;
			y = def->CurrentY;
			z = def->CurrentZ;
		}
	}

	sim->DoWarp(id, instance, x, y, z);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GroveEnvironmentCycleToggleHandler
//

int GroveEnvironmentCycleToggleHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if(sim->CheckPermissionSimple(Perm_Account, Permission_Admin) == false && sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == false) {
		if (pld->zoneDef->mGrove == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You are not in a grove.");
		if (pld->zoneDef->mAccountID != pld->accPtr->ID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in your grove.");
	}

	pld->zoneDef->ChangeEnvironmentUsage();
	if(pld->zoneDef->IsPlayerGrove())
		g_ZoneDefManager.NotifyConfigurationChange();

	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
			"Environment cycling is now %s",
			(pld->zoneDef->mEnvironmentCycle ? "ON" : "OFF"));
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//LootPackagesList
//

struct DropPackageDefinitionCompare {
    bool operator()(pair<string, DropPackageDefinition> &left,
      pair<string, DropPackageDefinition> &right) {
        return left.second.mName < right.second.mName;
    }
};

int LootPackagesListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: lootpackages.list
		 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	bool dev = sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer | Permission_Sage);
	if(!dev)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Not allowed.");

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);           //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index

	unsigned int start = query->GetInteger(0);
	unsigned int len = 50;
	if(query->argCount > 1)
		len = query->GetInteger(1);
	string filter = "";
	if(query->argCount > 2)
		filter = query->GetString(2);
	if(len > 50 || len == 0)
		len = 50;

	vector<pair<string, DropPackageDefinition>> results(g_DropTableManager.mPackage.begin(), g_DropTableManager.mPackage.end());
	sort(results.begin(), results.end(), DropPackageDefinitionCompare());
	if(results.size() == 0)
		len = start = 0;
	else {
		if(start > results.size())
			start = results.size();
		if(start + len >= results.size())
			len = results.size() - start;
	}
	wpos += PutShort(&sim->SendBuf[wpos], len);             //Row count
	for (size_t a = start; a < start + len; a++) {
		DropPackageDefinition zdinfo = results[a].second;
		wpos += PutByte(&sim->SendBuf[wpos], 5); //String count
		wpos += PutStringUTF(&sim->SendBuf[wpos], zdinfo.mName.c_str()); // 1
		wpos += PutStringUTF(&sim->SendBuf[wpos], to_string(zdinfo.mMobFlags).c_str()); // 2
		wpos += PutStringUTF(&sim->SendBuf[wpos], to_string(zdinfo.mAuto).c_str()); // 3
		wpos += PutStringUTF(&sim->SendBuf[wpos], to_string(zdinfo.mCombinedClassFlags).c_str()); // 4
		string s;
		Util::Join(zdinfo.mSetList, ",", s);
		wpos += PutStringUTF(&sim->SendBuf[wpos], s.c_str()); // 5
	}
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
//SetEnvironmentHandler
//

int SetEnvironmentHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	if(!sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer | Permission_Sage)) {
		if (pld->zoneDef->mGrove == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You are not in a grove.");
		if (pld->zoneDef->mAccountID != pld->accPtr->ID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in your grove.");
	}

	const char *env = query->args[0].c_str();

	if(pld->zoneDef->IsPlayerGrove())
		pld->zoneDef->ChangeEnvironment(env);

	g_ZoneDefManager.NotifyConfigurationChange();
	sim->SendInfoMessage("Environment type changed.", INFOMSG_INFO);

	int wpos = PrepExt_SendEnvironmentUpdateMsg(sim->SendBuf, creatureInstance->actInst, pld->CurrentZone, pld->zoneDef, -1, -1, 0);
	wpos += PrepExt_SetTimeOfDay(&sim->SendBuf[wpos], sim->GetTimeOfDay().c_str());
	creatureInstance->actInst->LSendToAllSimulator(sim->SendBuf, wpos, -1);

	//	SendZoneInfo();
	//LogMessageL(MSG_SHOW, "Environment set.");
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ShardListHandler
//

int ShardListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: shard.list
	 Requests a list of shards for use in the minimap dropdown list.
	 */
	return PrepExt_QueryResponseStringRows(sim->SendBuf, query->ID,
			g_ClusterManager.GetAvailableShardNames());
}

//
//WorldListHandler
//

int WorldListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: world.list
	 Requests a list of shards for use in the shard selection screen.
	 */

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);           //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index

	STRINGLIST shards = g_ClusterManager.GetAvailableShardNames();
	wpos += PutShort(&sim->SendBuf[wpos], shards.size());             //Row count
	for (size_t a = 0; a < shards.size(); a++) {
		wpos += PutByte(&sim->SendBuf[wpos], 6); //String count
		Shard s = g_ClusterManager.GetActiveShard(shards[a]);
		wpos += PutStringUTF(&sim->SendBuf[wpos], s.mName.c_str());
		if(pld->charPtr->Shard.compare(s.mName))
			wpos += PutStringUTF(&sim->SendBuf[wpos], Util::Format("!%s", s.mFullName.c_str()).c_str());
		else
			wpos += PutStringUTF(&sim->SendBuf[wpos], s.mFullName.c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], Util::Format("%d", s.mPlayers).c_str());
		time_t t = s.GetLocalTime() / 1000;
		wpos += PutStringUTF(&sim->SendBuf[wpos], Util::FormatDateTime(&t).c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], Util::Format("%d", s.mPing).c_str());
		wpos += PutStringUTF(&sim->SendBuf[wpos], s.IsMaster() ? "Master" : "Slave");
	}
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
//ZoneListHandler
//
struct ZoneDefInfoCompare {
    bool operator()(pair<int, ZoneDefInfo> &left,
      pair<int, ZoneDefInfo> &right) {
        return left.second.mName < right.second.mName;
    }
};

int ZoneListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: zone.list
	 Requests a list of zones for use in the zone tweak screen.
	 */


	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	bool dev = sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer | Permission_Sage);
	if(!dev) {
		if (pld->zoneDef->mGrove == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You are not in a grove.");
		if (pld->zoneDef->mAccountID != pld->accPtr->ID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in your grove.");
	}

//	regions = row.len() > 10 ? row[10] : "",
//	displayName = row.len() > 11 ? row[11] : ""

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);           //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index

	unsigned int start = query->GetInteger(0);
	unsigned int len = 50;
	if(query->argCount > 1)
		len = query->GetInteger(1);
	string filter = "";
	if(query->argCount > 2)
		filter = query->GetString(2);
	if(len > 50 || len == 0)
		len = 50;

	if(g_Logs.server->enabled(el::Level::Debug))
		g_Logs.server->debug("Zone list query for zone, start at [%v], for length of [%v]. Filter is '%v'", start, len, filter);

	vector<pair<int, ZoneDefInfo>> results;
	for(auto it = g_ZoneDefManager.mZoneList.begin(); it != g_ZoneDefManager.mZoneList.end(); ++it) {
		if((dev && !((*it).second.mGrove)) || (!dev && (*it).second.mAccountID == pld->accPtr->ID && !((*it).second.mGrove))) {
			if(filter == "" || Util::CaseInsensitiveStringFind((*it).second.mName, filter) || Util::CaseInsensitiveStringFind((*it).second.mDesc, filter) || Util::CaseInsensitiveStringFind((*it).second.mWarpName, filter)) {
				pair<int, ZoneDefInfo> pd = pair<int, ZoneDefInfo>(
						(*it).second.mID, (*it).second);
				results.push_back(pd);
			}
		}
	}
	sort(results.begin(), results.end(), ZoneDefInfoCompare());
	if(results.size() == 0)
		len = start = 0;
	else {
		if(start > results.size())
			start = results.size();
		if(start + len >= results.size())
			len = results.size() - start;
	}
	if(g_Logs.server->enabled(el::Level::Debug))
		g_Logs.server->debug("Actual zone query results, start at [%v], for length of [%v] in results of [%v]", start, len, results.size());

	wpos += PutShort(&sim->SendBuf[wpos], len);             //Row count
	for (size_t a = start; a < start + len; a++) {
		ZoneDefInfo zdinfo = results[a].second;
		wpos += WriteZoneDefInfo(&sim->SendBuf[wpos], &zdinfo);
	}
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}


//
//ZoneGetHandler
//
int ZoneGetHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: zone.get
	 Requests a single zone.
	 */
	int zone = pld->zoneDef->mID;
	if(query->argCount > 0)
		zone = query->GetInteger(0);
	bool dev = sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer | Permission_Sage);
	if(!dev) {
		if (pld->zoneDef->mGrove == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You are not in a grove.");
		if (pld->zoneDef->mAccountID != pld->accPtr->ID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in your grove.");
		if(zone != pld->zoneDef->mID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Can only retrieve your grove.");
	}

	ZoneDefInfo *def = g_ZoneDefManager.GetPointerByID(zone);
	if(def == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such zone.");

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);              //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);           //Placeholder for message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 1);             //Row count
	wpos += WriteZoneDefInfo(&sim->SendBuf[wpos], def);
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
//ZoneEditHandler
//

int ZoneEditHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: zone.edit
	 Edits a zone property, create a new zone, or clone an existing zone, copying its
	 configuration and scenery from an existing zone


	 */


	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	bool dev = sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer | Permission_Sage);

	int zone = query->GetInteger(0);
	unsigned int idx = 1;
	bool isNew = false;
	if(zone == 0) {
		if(query->GetStringObject(0) == "CLONE_CONTENTS") {
			if(dev) {
				int sourceZone = query->GetInteger(1);
				ZoneDefInfo *source = g_ZoneDefManager.GetPointerByID(sourceZone);
				if(source == NULL) {
					return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
							"No such source zone.");
				}
				pld->zoneDef->PopulateFrom(*source);
				g_ZoneDefManager.NotifyConfigurationChange();
				if(pld->zoneDef->IsPlayerGrove())
					g_ZoneDefManager.CreateGroveZone(*(pld->zoneDef));
				else
					g_ZoneDefManager.UpdateWorldZone(*(pld->zoneDef));

				sim->SendInfoMessage(pld->zoneDef->mName.c_str(), INFOMSG_LOCATION);
				sim->SendInfoMessage(pld->charPtr->Shard.c_str(), INFOMSG_SHARD);
				int wpos = PrepExt_SendEnvironmentUpdateMsg(sim->SendBuf, creatureInstance->actInst, pld->CurrentZone, pld->zoneDef, -1, -1, 0);
				wpos += PrepExt_SetTimeOfDay(&sim->SendBuf[wpos], sim->GetTimeOfDay().c_str());
				creatureInstance->actInst->LSendToAllSimulator(sim->SendBuf, wpos, -1);
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID, to_string(zone).c_str());
			}
			else
				// Only devs can create zones
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Not allowed.");
		}
		else if(query->GetStringObject(0) == "NEW") {
			if(dev) {
				zone = g_ZoneDefManager.CreateWorldZone();
				if(zone == 0) {
					return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
							"Server refused to create a new zone. Please consult logs.");
				}
				isNew = true;
			}
			else
				// Only devs can create zones
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Not allowed.");
		}
		else
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid zone.");
	}

	ZoneDefInfo *def = g_ZoneDefManager.GetPointerByID(zone);
	if(def == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such zone.");
	}

	if(isNew) {
		def->mName = "New_Zone_" + to_string(def->mID);
		def->mWarpName = def->mName;
		def->DefX = 1;
		def->DefY = 1;
		def->DefZ = 1;
	}

	if(!dev) {
		if (pld->zoneDef->mGrove == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You are not in a grove.");
		if (pld->zoneDef->mAccountID != pld->accPtr->ID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in your grove.");

		if(def->mID != pld->zoneDef->mID)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You must be in this grove.");
	}

	while(idx < query->argCount) {
		string key = query->GetString(idx++);
		string value = query->GetString(idx++);
		if(key == "name")
			def->mName = value;
		else if(key == "terrain") {
			def->mTerrainConfig = value;
			if(def->mID == creatureInstance->actInst->mZone) {
				creatureInstance->actInst->SetEnvironment(value);
			}
		}
		else if(key == "displayName")
			def->mDesc = value;
		else if(key == "mapName")
			def->mMapName = value;
		else if(key == "groupPermission") {
			// TODO
		}
		else if(key == "environmentType") {
			def->mEnvironmentType = value;
			if(def->mID == creatureInstance->actInst->mZone) {
				creatureInstance->actInst->SetEnvironment(value);
			}
		}
		else if(key == "instancing")
			def->mInstance = value.compare("MULTIPLE") == 0;
		else if(key == "beefFactor") {
			// TODO
		}
		else if(key == "warpName")
			def->mWarpName = value;
		else if(key == "regions") {
			// TODO
		}
		else if(key == "defaultLayers") {
			// TODO
		}
		else if(key == "instancing")
			def->mInstance = value.compare("MULTIPLE") == 0;
		else if(key == "groveName")
			def->mGroveName = value;
		else if(key == "grove")
			def->mGrove = value == "Y" || value == "y";
		else if(key == "persist")
			def->mPersist = value == "Y" || value == "y";
		else if(key == "arena")
			def->mArena = value == "Y" || value == "y";
		else if(key == "guildHall")
			def->mGuildHall = value == "Y" || value == "y";
		else if(key == "clan")
			def->mClan = Util::SafeParseInt(value, 0);
		else if(key == "environmentCycle")
			def->mEnvironmentCycle = value == "Y" || value == "y";
		else if(key == "audit")
			def->mAudit = value == "Y" || value == "y";
		else if(key == "maxAggroRange")
			def->mMaxAggroRange = Util::SafeParseInt(value, ZoneDefInfo::DEFAULT_MAXAGGRORANGE);
		else if(key == "maxLeashRange")
			def->mMaxLeashRange = Util::SafeParseInt(value, ZoneDefInfo::DEFAULT_MAXLEASHRANGEINSTANCE);
		else if(key == "minLevel")
			def->mMinLevel = Util::SafeParseInt(value, 0);
		else if(key == "maxLevel")
			def->mMaxLevel = Util::SafeParseInt(value, 9999);
		else if(key == "pageSize")
			def->mPageSize = Util::SafeParseInt(value, ZoneDefInfo::DEFAULT_PAGESIZE);
		else if(key == "mode")
			def->mMode = Util::SafeParseInt(value);
		else if(key == "returnZone")
			def->mReturnZone = Util::SafeParseInt(value);
		else if(key == "timeOfDay")
			def->mTimeOfDay = value;
		else if(key == "def") {
			STRINGLIST l;
			Util::Split(value, " ", l);
			if(l.size() > 0)
				def->DefX = Util::SafeParseInt(l[0]);
			else if(l.size() > 1)
				def->DefY = Util::SafeParseInt(l[1]);
			else if(l.size() > 2)
				def->DefZ = Util::SafeParseInt(l[2]);
		}
		else
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid key.");
	}

	if(def->IsPlayerGrove()) {
		g_ZoneDefManager.CreateGroveZone(*def);
	}
	else {
		g_ZoneDefManager.UpdateWorldZone(*def);
	}

	if(isNew) {
		sim->WarpToZone(def, def->DefX, def->DefY, def->DefZ);
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, to_string(zone).c_str());
}
//
//ShardSetHandler
//

int ShardSetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	//Un-modified client only sets 1 row.  Modified client contains an additional field.
	//[0] Shard Same
	//[1] Character Name  [modded client]
	string shardName = "";
	string charName = "";
	if (query->argCount >= 1)
		shardName = query->args[0];
	if (query->argCount >= 2)
		charName = query->args[1];

	string res = sim->ShardSet(shardName, charName);
	if(res.length() == 0)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	else {
		sim->SendInfoMessage(res.c_str(), INFOMSG_INFO);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, res.c_str());
	}
}

//
//HengeSetDestHandler
//

int HengeSetDestHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: henge.setDest
	 Sent when a henge selection is made.
	 Args : [0] = Destination Name
	 [1] = Creature ID (when using henge interacts, but also uses custom values for certain things like groves).
	 */

	if (query->argCount >= 2) {
		const char *destName = query->GetString(0);
		int sourceID = query->GetInteger(1);

		if (sim->CheckValidHengeDestination(destName, sourceID) == true) {
			ZoneDefInfo *zoneDef = NULL;
			InteractObject *interact = NULL;
			bool blockAttempt = false;

			if ((strcmp(destName, EXIT_GROVE) == 0)
					|| (strcmp(destName, EXIT_PVP) == 0)
					|| (strcmp(destName, EXIT_GUILD_HALL) == 0)) {
				int destID = pld->charPtr->groveReturnPoint[3]; //x, y, z, zoneID
				if (destID == 0) {
					if (creatureInstance->css.level < 3)
						destID = 59;
					else if (creatureInstance->css.level < 11)
						destID = 58;
					else
						destID = 81;
				}
				zoneDef = g_ZoneDefManager.GetPointerByID(destID);
			} else {
				interact = g_InteractObjectContainer.GetHengeByTargetName(
						destName);
				if (interact == NULL)
					zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(
							destName);
				else
					zoneDef = g_ZoneDefManager.GetPointerByID(interact->WarpID);
			}

			if (interact != NULL) {
				if (pld->CurrentZoneID == interact->WarpID) {
					if (creatureInstance->IsSelfNearPoint(
							(float) interact->WarpX, (float) interact->WarpZ,
							SimulatorThread::INTERACT_RANGE * 4) == true) {
						sim->SendInfoMessage(
								"You are already at that location.",
								INFOMSG_ERROR);
						blockAttempt = true;
					}
				}

				if (blockAttempt == false && interact->cost != 0) {
					if (creatureInstance->css.copper < interact->cost) {
						sim->SendInfoMessage("Not enough coin.", INFOMSG_ERROR);
						blockAttempt = true;
					} else {
						creatureInstance->AdjustCopper(-interact->cost);
					}
				}
			}

			if (zoneDef == NULL) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Warp target not found: %s", destName);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
			} else if (blockAttempt == false) {
				//Ready to warp!
				int xLoc = 0, yLoc = 0, zLoc = 0;
				if (interact != NULL) {
					xLoc = interact->WarpX;
					yLoc = interact->WarpY;
					zLoc = interact->WarpZ;
				}
				sim->WarpToZone(zoneDef, xLoc, yLoc, zLoc);
			}
		}
	}

	/*
	 //We need to verify the warp is acceptable.  This adds some exploit protection since it
	 //could otherwise technically be entered as a chat command and interpreted as a valid warp
	 //for civilian players.
	 if(verificationRequired == true)
	 {
	 int errCode = CheckValidWarpZone(zoneDef->mID);
	 if(errCode != ERROR_NONE)
	 {
	 SendInfoMessage(GetGenericErrorString(errCode), INFOMSG_ERROR);
	 goto endfunction;
	 }
	 }
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//MapMarkerHandler
//

int MapMarkerHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/* Query: map.marker
	 Args : at least 1
	 [0] = Zone ID
	 [...] typically a list, "Shop", "Vault", "QuestGiver", "Henge", "Sanctuary"
	 */
	//TODO: this doesn't yet handle all the client requests
	if (query->argCount < 1)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	int zoneID = strtol(query->args[0].c_str(), NULL, 10);
	if (zoneID != pld->CurrentZoneID)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	MULTISTRING qRes;
	for (unsigned int i = 1; i < query->argCount; i++) {
		if (query->args[i].compare("QuestGiver") == 0) {
			for (size_t qi = 0;
					qi
							< pld->charPtr->questJournal.availableQuests.itemList.size();
					qi++) {
				QuestDefinition *qd =
						pld->charPtr->questJournal.availableQuests.itemList[qi].GetQuestPointer();
				if (qd == NULL)
					continue;
				if (qd->giverZone != pld->CurrentZoneID)
					continue;
				if (creatureInstance->css.level < qd->levelMin)
					continue;
				if (qd->levelMax != 0)
					if (creatureInstance->css.level > qd->levelMax)
						continue;
				if (abs(creatureInstance->CurrentX - qd->giverX) > 1920)
					continue;
				if (abs(creatureInstance->CurrentZ - qd->giverZ) > 1920)
					continue;
				if (qd->Requires > 0)
					if (pld->charPtr->questJournal.completedQuests.HasQuestID(
							qd->Requires) == -1)
						continue;

				// Guild start quest
				if (qd->guildStart
						&& creatureInstance->charPtr->IsInGuildAndHasValour(
								qd->guildId, 0))
					continue;

				// Guild requirements
				if (!qd->guildStart && qd->guildId != 0
						&& !creatureInstance->charPtr->IsInGuildAndHasValour(
								qd->guildId, qd->valourRequired))
					continue;

				qRes.push_back(STRINGLIST());
				qRes.back().push_back(qd->title.c_str());
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "(%d %d %d)",
						qd->giverX, qd->giverY, qd->giverZ);
				qRes.back().push_back(sim->Aux1);
				qRes.back().push_back("QuestGiver");
			}
		}
		else if(query->args[i].compare("Henge") == 0) {
			/* All henges in zone */
			if(pld->zoneDef != NULL && !pld->zoneDef->IsOverworld()) {
				for(vector<InteractObject>::iterator it = g_InteractObjectContainer.objList.begin(); it != g_InteractObjectContainer.objList.end(); ++it) {
					if((*it).opType == InteractObject::TYPE_HENGE && (*it).WarpID == pld->CurrentZoneID) {
						qRes.push_back(STRINGLIST());
						qRes.back().push_back((*it).useMessage);
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "(%d %d %d)", (*it).WarpX, (*it).WarpY, (*it).WarpZ);
						qRes.back().push_back(sim->Aux1);
						qRes.back().push_back("Henge");
					}
				}
			}
		}
		else if(query->args[i].compare("Sanctuary") == 0) {
			/* All sanctuaries within 1000 */
			ZoneMarkerData *zmd = g_ZoneMarkerDataManager.GetPtrByZoneID(pld->CurrentZoneID);
			if(zmd != NULL && pld->zoneDef != NULL && !pld->zoneDef->IsOverworld()) {
				for(vector<WorldCoord>::iterator it = zmd->sanctuary.begin(); it != zmd->sanctuary.end(); ++it) {
					int xlen = abs((int)(*it).x - creatureInstance->CurrentX);
					int zlen = abs((int)(*it).z - creatureInstance->CurrentZ);
					double dist = sqrt((double)((xlen * xlen) + (zlen * zlen)));
					if(dist < 1000) {
						qRes.push_back(STRINGLIST());
						qRes.back().push_back((*it).descName.c_str());
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "(%d %d %d)", (int)((*it).x), (int)((*it).y), (int)((*it).z));
						qRes.back().push_back(sim->Aux1);
						qRes.back().push_back("Sanctuary");
					}
				}
			}
		}
	}
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, qRes);
}
//
//SetGroveStartHandler
//

int SetGroveStartHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");

	pld->zoneDef->ChangeDefaultLocation(creatureInstance->CurrentX,
			creatureInstance->CurrentY, creatureInstance->CurrentZ);
	g_ZoneDefManager.NotifyConfigurationChange();
	sim->SendInfoMessage("Set grove entrance location to your coordinates.",
			INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PortalAcceptRequestHandler
//

int PortalAcceptRequestHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	sim->Submit(bind(&SimulatorThread::RunPortalRequest, sim));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//BuildTemplateListHandler
//

int BuildTemplateListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: build.template.list
	 Sent when build mode is activated in the client.  Only sent for the first
	 activation.
	 Not sure what the purpose of this data is for.  It seems to form a list
	 of valid searchable templates.
	 Args: [none]
	 */

	//The client script code indicates that only one row is used, but with a variable
	//number of elements.
	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);            //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);           //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);    //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 1);           //Array count
	wpos += PutByte(&sim->SendBuf[wpos], 3);            //String count
	wpos += PutStringUTF(&sim->SendBuf[wpos], "Crate1");  //String data
	wpos += PutStringUTF(&sim->SendBuf[wpos], "Crate2");  //String data
	wpos += PutStringUTF(&sim->SendBuf[wpos], "Crate3");  //String data
	PutShort(&sim->SendBuf[1], wpos - 3);               //Set message size
	return wpos;
}

//
//GetDungeonProfilesHandler
//

int GetDungeonProfilesHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	MULTISTRING response;
	g_InstanceScaleManager.EnumProfileList(response);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, response);
}

//
//SetATSHandler
//

int SetATSHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");
	//if(pld->zoneDef->mGrove == false)
	//	return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "You are not in a grove.");
	//if(pld->zoneDef->mAccountID != pld->accPtr->ID)
	//	return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "You must be in your grove.");

	const char *atsName = query->args[0].c_str();
	if (g_SceneryManager.ValidATSEntry(atsName) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"ATS name is not valid.");

	int success = 0;
	int failed = 0;
	int wpos = 0;
	g_SceneryManager.GetThread("SimulatorThread::handle_query_mod_setats");
	for (unsigned int i = 1; i < query->argCount; i += 2) {
		int propID = atoi(query->args[i].c_str());
		const char *assetStr = query->args[i + 1].c_str();

		//SceneryObject *so = g_SceneryManager.GetPropPtr(pld->CurrentZoneID, propID, NULL);
		SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(pld->CurrentZoneID,
				propID, NULL);
		if (so != NULL) {
			int check = 0;
			if (strstr(assetStr, "Bldg-") != NULL)
				check++;
			if (strstr(assetStr, "Cav-") != NULL)
				check++;
			if (strstr(assetStr, "Dng-") != NULL)
				check++;
			if (check == 0) {
				failed++;
				continue;
			}
			SceneryObject replaceProp;
			replaceProp.copyFrom(so);
			string newAsset = replaceProp.Asset;
			size_t pos = newAsset.find("?ATS=");
			if (pos != string::npos) {
				newAsset.erase(pos + 5, newAsset.length()); //Erase everything after "?ATS="
				newAsset.append(atsName);
			} else {
				newAsset.append("?ATS=");
				newAsset.append(atsName);
			}
			replaceProp.Asset = newAsset;
			g_Logs.simulator->info("[%v] Setting prop: %v to asset: %v",
					sim->InternalID, propID, newAsset.c_str());

			if (sim->HasPropEditPermission(&replaceProp) == true) {
				g_SceneryManager.ReplaceProp(pld->CurrentZoneID, replaceProp);

				wpos += PrepExt_UpdateScenery(&sim->SendBuf[wpos], &replaceProp);
				if (wpos >= Global::MAX_SEND_CHUNK_SIZE) {
					creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
							creatureInstance->CurrentX, creatureInstance->CurrentZ);
					wpos = 0;
				}
				success++;
			} else
				failed++;
		}
	}
	g_SceneryManager.ReleaseThread();

	if (wpos > 0)
		creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
				creatureInstance->CurrentX, creatureInstance->CurrentZ);

	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
			"Attempted on %d props (%d success, %d failed)", success + failed,
			success, failed);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
