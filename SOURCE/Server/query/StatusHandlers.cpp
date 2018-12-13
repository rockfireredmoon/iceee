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

#include "StatusHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Combat.h"
#include "../util/Log.h"

//
//MoreStatsHandler
//

int MoreStatsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	MULTISTRING response;
	StatInfo::GeneratePrettyStatTable(response, &creatureInstance->css);
	STRINGLIST row;
	row.push_back("Spirit Resist Percentage");
	sprintf(sim->Aux3, "%d%%",
			Combat::GetSpiritResistReduction(100, creatureInstance->css.spirit));
	row.push_back(sim->Aux3);
	response.push_back(row);

	row.clear();
	row.push_back("Psyche Resist Percentage");
	sprintf(sim->Aux3, "%d%%",
			Combat::GetPsycheResistReduction(100, creatureInstance->css.psyche));
	row.push_back(sim->Aux3);
	response.push_back(row);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, response);
}

//
//ClientLoadingHandler
//

int ClientLoadingHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/* Query: client.loading
	 Args : [0] = status to notify, either "true" or "false"
	 Response: Standard query success "OK"
	 */

	if (query->argCount > 0) {
		if (query->args[0].compare("true") == 0)
			sim->SetLoadingStatus(true, false);
		else
			sim->SetLoadingStatus(false, false);
	}

	if (sim->LoadStage == SimulatorThread::LOADSTAGE_LOADED)
		sim->SendSetAvatar(creatureInstance->CreatureID);

	int WritePos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

	if (sim->LoadStage == SimulatorThread::LOADSTAGE_LOADED) {
		sim->LoadStage = SimulatorThread::LOADSTAGE_GAMEPLAY;

		if (pld->charPtr->PrivateChannelName.size() > 0)
			sim->JoinPrivateChannel(pld->charPtr->PrivateChannelName.c_str(),
					pld->charPtr->PrivateChannelPassword.c_str());
	}
	if (sim->LoadStage == SimulatorThread::LOADSTAGE_GAMEPLAY) //Note: Every time the client triggers a load screen this will be resent.
			{
		std::vector<short> stats;
		stats.push_back(STAT::MOD_MOVEMENT);
		stats.push_back(STAT::BASE_MOVEMENT);
		//Seems to fix the 120% speed problem when first logging in
		WritePos += PrepExt_SendSpecificStats(&sim->SendBuf[WritePos], creatureInstance,
				stats);
		//pld.ResendSpeedTime = g_ServerTime + 2000;

		if (g_Config.DebugPingClient == true) {
			int mpos = WritePos;
			WritePos += PutByte(&sim->SendBuf[WritePos], 100); //_handleModMessage   REQUIRES MODDED CLIENT
			WritePos += PutShort(&sim->SendBuf[WritePos], 0);    //Reserve for size
			WritePos += PutByte(&sim->SendBuf[WritePos],
					MODMESSAGE_EVENT_PING_START);
			WritePos += PutInteger(&sim->SendBuf[WritePos],
					g_Config.DebugPingFrequency);
			PutShort(&sim->SendBuf[mpos + 1], WritePos - mpos - 3); //Set message size
		}
	}
	return WritePos;
}

//
// AdminCheckHandler
//
//
/* TODO Remove this, its not used any more (ClientPermsHandler) */
int AdminCheckHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/* Query: admin.check
	 Args : none
	 Response: Return an error if the account does not have admin permissions.

	 This is used to unlock some debug features in the client.
	 */

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClientPermsHandler
//
int ClientPermsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/* Query: clientperms.list
	 Args : none
	 Response: Return an array of permissions the client has.
	 */

	STRINGLIST l;

	if(sim->CheckPermissionSimple(Perm_Account, Permission_Admin)) {
		l.push_back("dev");
		l.push_back("tweakScreens");
		l.push_back("scriptTest");
		l.push_back("importAbilities");
		l.push_back("worldBuild");
		l.push_back("groveBuild");
		l.push_back("debug");
		l.push_back("compositor");
		l.push_back("setappearance");
		l.push_back("playSound");
		l.push_back("reassemble");
		l.push_back("version");
		l.push_back("auditlog");
		l.push_back("sage");
	}
	else {

		if(sim->CheckPermissionSimple(Perm_Account, Permission_TweakOther) == true ||
				sim->CheckPermissionSimple(Perm_Account, Permission_TweakSelf) == true ||
				sim->CheckPermissionSimple(Perm_Account, Permission_TweakNPC) == true ||
				sim->CheckPermissionSimple(Perm_Account, Permission_TweakClient) == true) {
			l.push_back("tweakScreens");
		}

		if(sim->CheckPermissionSimple(Perm_Account, Permission_Developer) == true) {
			l.push_back("dev");
			l.push_back("scriptTest");
			l.push_back("importAbilities");
			l.push_back("playSound");
			l.push_back("compositor");
			l.push_back("reassemble");
			l.push_back("auditlog");
		}

		if(sim->CheckPermissionSimple(Perm_Account, Permission_Developer) == true ||
		   sim->CheckPermissionSimple(Perm_Account, Permission_Builder) == true) {
			l.push_back("worldBuild");
		}

		if(sim->CheckPermissionSimple(Perm_Account, Permission_Developer) == true ||
		   sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == true) {
			l.push_back("setappearance");
			l.push_back("sage");
		}

		if(sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Developer) == true) {
			l.push_back("debug");
		}

		l.push_back("groveBuild");
		l.push_back("playSound");
		l.push_back("version");
	}
	l.push_back("build");
	return PrepExt_QueryResponseStringList(sim->SendBuf, query->ID, l);
}
//
// PingSimHandler
//
int PingSimHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: util.pingsim
	 Sent by the /ping command.
	 Args : [none]
	 */

	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}

//
// PingRouterHandler
//
int PingRouterHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: util.pingrouter
	 Sent by the /ping command.
	 Args : [none]
	 */

	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}
//
// PersonaGMHandler
//
int PersonaGMHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (sim->CheckPermissionSimple(Perm_Account, Permission_Sage) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
// VersionHandler
//
int VersionHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: util.version
	 Args: [none]
	 */

	// The client seems to expect a list of rows, each with two strings for name
	// and value.
	return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, "Build",
			VersionString);
}
