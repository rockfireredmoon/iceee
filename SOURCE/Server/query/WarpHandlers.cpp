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
#include "WarpHandlers.h"
#include "../Account.h"
#include "../Character.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Config.h"
#include "../util/Log.h"

//
// WarpHandler
//

int WarpHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: warp
	 Args : Optional types:
	 [0] = characterName
	 [0] = direction [n|e|s|w]
	 [0] = direction [n|e|s|w], [1] = distance
	 [0] = XPos, [1] = Ypos
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /warp x y [or] /warp \"Character Name\"");

	int zone = pld->CurrentZoneID;
	//int instance = pld.CurrentInstanceID;
	int instance = 0; //Only set the instance if we need an explicit warp there.
	int xpos = creatureInstance->CurrentX;
	int ypos = creatureInstance->CurrentY;
	int zpos = creatureInstance->CurrentZ;

	if (query->argCount >= 2) {
		const char *param1 = query->args[0].c_str();
		int param2 = atoi(query->args[1].c_str());

		//First check for relative directional positions before
		//processing a raw coordinate
		if (strcmp(param1, "n") == 0)
			zpos -= param2;
		else if (strcmp(param1, "s") == 0)
			zpos += param2;
		else if (strcmp(param1, "w") == 0)
			xpos -= param2;
		else if (strcmp(param1, "e") == 0)
			xpos += param2;
		else {
			//Coordinate warp
			xpos = atoi(param1);
			zpos = param2;
		}
	} else if (query->argCount == 1) {
		const char *target = query->args[0].c_str();
		if (strcmp(target, "n") == 0)
			zpos -= sim->DefaultWarpDistance;
		else if (strcmp(target, "s") == 0)
			zpos += sim->DefaultWarpDistance;
		else if (strcmp(target, "w") == 0)
			xpos -= sim->DefaultWarpDistance;
		else if (strcmp(target, "e") == 0)
			xpos += sim->DefaultWarpDistance;
		else if (strchr(target, ',') != NULL) {
			std::vector<std::string> args;
			Util::Split(query->args[0], ",", args);
			if (args.size() >= 2) {
				xpos = atoi(args[0].c_str());
				zpos = atoi(args[1].c_str());
				if (args.size() >= 3)
					zpos = atoi(args[2].c_str()); //Hack for x,y,z strings since they're often copy/pasted
			}
		} else {
			//Check names
			bool bFound = false;

			SIMULATOR_IT it;
			for (it = Simulator.begin(); it != Simulator.end(); ++it) {
				if (it->isConnected == true && it->ProtocolState == 1) {
					if (it->IsGMInvisible() == true)
						continue;
					if (strstr(it->pld.charPtr->cdef.css.display_name,
							target) != NULL) {
						zone = it->pld.CurrentZoneID;
						instance = it->pld.CurrentInstanceID;
						xpos = it->creatureInst->CurrentX;
						ypos = it->creatureInst->CurrentY;
						zpos = it->creatureInst->CurrentZ;
						bFound = true;
						break;
					}
				}
			}
			if (bFound == false) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Could not find target for warp: %s", target);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
			}
		}
	}

	if (zone != pld->CurrentZoneID) {
		int errCode = sim->CheckValidWarpZone(zone);
		if (errCode != sim->ERROR_NONE)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					sim->GetGenericErrorString(errCode));
	}

	sim->DoWarp(zone, instance, xpos, ypos, zpos);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpInstanceHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpi
	 Handles on-demand warping to instances.
	 Args : 1, [0] = Instance Name
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /warpi groveName");

	const char *warpTarg = query->args[0].c_str();
	ZoneDefInfo *targZone = g_ZoneDefManager.GetPointerByPartialWarpName(
			warpTarg);
	if (targZone == NULL) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Zone name not found: %s", warpTarg);
		g_Logs.simulator->error("[%v] %v", sim->InternalID, sim->Aux1);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
	} else {
		int errCode = sim->CheckValidWarpZone(targZone->mID);
		if (errCode != sim->ERROR_NONE)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					sim->GetGenericErrorString(errCode));

		//If the avatar is running, it will glitch the position.

		// EM - Is this really needed?
		//		SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);

		if (sim->ProtectedSetZone(targZone->mID, 0) == false) {
			sim->ForceErrorMessage("Critical error while changing zones.",
					INFOMSG_ERROR);
			sim->Disconnect("SimulatorThread::handle_command_warpi");
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Critical error.");
		}
		sim->SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpTileHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: warpt
	 Handles on-demand warping to a scenery tile coordinate within the instance.
	 Args : 2, [0] = TileX, [1] = TileY
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /warpt x y");

	int x = atoi(query->args[0].c_str());
	int z = atoi(query->args[1].c_str());

	int xtarg = Util::ClipInt(x, 0, 100) * pld->zoneDef->mPageSize;
	int ztarg = Util::ClipInt(z, 0, 100) * pld->zoneDef->mPageSize;

	sim->SetPosition(xtarg, creatureInstance->CurrentY, ztarg, 1);
	sim->SendInfoMessage("Warping.", INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpPullHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: warpp
	 An external warp that pull a target directly to the player.
	 */
	if (sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	CreatureInstance *targ = NULL;
	if (query->argCount == 0)
		targ = creatureInstance->CurrentTarget.targ;
	else
		targ = creatureInstance->actInst->GetPlayerByName(
				query->args[0].c_str());

	if (targ == NULL || targ == creatureInstance)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must select a target.");

	targ->CurrentX = creatureInstance->CurrentX;
	targ->CurrentY = creatureInstance->CurrentY;
	targ->CurrentZ = creatureInstance->CurrentZ;

	sim->AddMessage((long) targ, 0, BCM_UpdatePosition);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpGroveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	const char *grove = NULL;
	if (query->argCount > 0)
		grove = query->GetString(0);

	if (grove == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No grove specified.");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(grove);
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Grove not found.");
	if (zoneDef->mGrove == false && !zoneDef->mGuildHall)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Destination is not a grove.");

	int errCode = sim->CheckValidWarpZone(zoneDef->mID);
	if (errCode != sim->ERROR_NONE)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetGenericErrorString(errCode));

	int xLoc = 0;
	int zLoc = 0;
	if (query->argCount == 3) {
		xLoc = query->GetInteger(1);
		zLoc = query->GetInteger(2);
	}
	sim->WarpToZone(zoneDef, xLoc, 0, zLoc);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpExternalOfflineHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpext
	 Warp an external target to the player.  Used to set positions of offline characters.
	 Args: [0] = Character Name [required]
	 [1] = Zone ID to warp the character to [required]
	 [2][3] = X and Z coordinate to warp [optional]
	 [4] = Y coordinate to warp [optional]
	 */

	if (sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /warpext \"Char Name\" zoneID [x z] [y]");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
			atoi(query->args[1].c_str()));
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Zone not found.");

	CharacterData *cd = NULL;
	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.GetCharacterByName(query->args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if (cd != NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character must be offline.");

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_warpext");
	int CDef = g_AccountManager.GetCDefFromCharacterName(
			query->args[0].c_str());
	g_AccountManager.cs.Leave();

	if (CDef == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character name not found.");

	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.RequestCharacter(CDef, true);
	g_CharacterManager.ReleaseThread();

	if (cd == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to load character.");
	cd->ExtendExpireTime();

	int warpx = zoneDef->DefX;
	int warpy = zoneDef->DefY;
	int warpz = zoneDef->DefZ;
	if (query->argCount >= 4) {
		warpx = atoi(query->args[2].c_str());
		warpz = atoi(query->args[3].c_str());
	}
	if (query->argCount >= 5)
		warpy = atoi(query->args[4].c_str());

	cd->activeData.CurZone = zoneDef->mID;
	cd->activeData.CurX = warpx;
	cd->activeData.CurY = warpy;
	cd->activeData.CurZ = warpz;
	cd->SetExpireTime();

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int WarpExternalHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpext2
	 Warp an external target to a new zone ID.
	 Args: [0] = Character Name [required]
	 [1] = Zone ID to warp the character to [required]
	 */

	if (sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /warpext \"Char Name\" zoneID");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
			atoi(query->args[1].c_str()));
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Zone not found.");

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->ProtocolState == 1)
			if (strcmp(it->creatureInst->css.display_name,
					query->args[0].c_str()) == 0) {
				sim->SendInfoMessage("Warping target.", INFOMSG_INFO);
				it->MainCallSetZone(zoneDef->mID, 0, true);
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"OK");
			}
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Target not found.");
}

