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
#include "../Instance.h"
#include "../Interact.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//GoHandler
//
int GoHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	//Sent by the client when pressing 'C' when the build tool is open.  Intended function is to
	//warp the player to the camera position.
	// [0], [1], [2] = x, y, z coordinates, respectively.

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	int x = creatureInstance->CurrentX;
	int y = creatureInstance->CurrentY;
	int z = creatureInstance->CurrentZ;

	if (query->argCount >= 3) {
		x = static_cast<int>(query->GetFloat(0)); //The arguments come in as floats but we use ints.
		y = static_cast<int>(query->GetFloat(1));
		z = static_cast<int>(query->GetFloat(2));
	}
	sim->DoWarp(pld->CurrentZoneID, pld->CurrentInstanceID, x, y, z);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GroveEnvironmentCycleToggleHandler
//

int GroveEnvironmentCycleToggleHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");
	pld->zoneDef->ChangeEnvironmentUsage();
	g_ZoneDefManager.NotifyConfigurationChange();
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Environment cycling is now %s",
			(pld->zoneDef->mEnvironmentCycle ? "ON" : "OFF"));
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}


//
//SetEnvironmentHandler
//

int SetEnvironmentHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query.");

	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");

	const char *env = query->args[0].c_str();

	pld->zoneDef->ChangeEnvironment(env);
	g_ZoneDefManager.NotifyConfigurationChange();
	sim->SendInfoMessage("Environment type changed.", INFOMSG_INFO);

	int wpos = PrepExt_SendEnvironmentUpdateMsg(sim->SendBuf, pld->CurrentZone,
			pld->zoneDef, -1, -1);
	wpos += PrepExt_SendTimeOfDayMsg(&sim->SendBuf[wpos], sim->GetTimeOfDay());
	creatureInstance->actInst->LSendToAllSimulator(sim->SendBuf, wpos, -1);

	//	SendZoneInfo();
	//LogMessageL(MSG_SHOW, "Environment set.");
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}



//
//ShardListHandler
//

int ShardListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: shard.list
	 Requests a list of shards for use in the minimap dropdown list.
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
			pld->zoneDef->mName.c_str());
}

//
//ShardSetHandler
//

int ShardSetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {


	//Un-modified client only sets 1 row.  Modified client contains an additional field.
	//[0] Shard Same
	//[1] Character Name  [modded client]
	const char *shardName = NULL;
	const char *charName = NULL;
	if (query->argCount >= 1)
		shardName = query->args[0].c_str();
	if (query->argCount >= 2)
		charName = query->args[0].c_str();

	sim->ShardSet(shardName, charName);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//HengeSetDestHandler
//

int HengeSetDestHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

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
					if (creatureInstance->IsSelfNearPoint((float) interact->WarpX,
							(float) interact->WarpZ, SimulatorThread::INTERACT_RANGE * 4)
							== true) {
						sim->SendInfoMessage("You are already at that location.",
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


