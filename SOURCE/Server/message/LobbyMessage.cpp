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

#include "LobbyMessage.h"
#include "../query/Query.h"
#include "../auth/Auth.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Scheduler.h"
#include "../Cluster.h"
#include "../util/Log.h"

//
//LobbyAuthenticateMessage
//

int LobbyAuthenticateMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	if(g_AccountManager.AcceptingLogins() == false) {
		sim->ForceErrorMessage("Not accepting more logins.", INFOMSG_ERROR);
		sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
		return 0;
	}

	char authType = GetByte(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	AuthHandler *authHandler = g_AuthManager.GetAuthenticator(authType);
	if (authHandler == NULL) {
		g_Logs.simulator->error("[%v] Could not find authentication handler [%v]",
				sim->InternalID, authType);
		sim->ForceErrorMessage(g_Config.InvalidLoginMessage.c_str(), INFOMSG_ERROR);
		sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
		return 0;
	}

	int simID = sim->InternalID;

	std::string loginName = GetCPPStringUTF(&sim->readPtr[sim->ReadPos], 128, sim->ReadPos);  //login name
	std::string authHash = GetCPPStringUTF(&sim->readPtr[sim->ReadPos], 128, sim->ReadPos);  //authorization hash or or X-CSRF-Token:sess_id:session_name:uid

	g_Logs.simulator->info("Starting authentication for %v (%v) using %v", loginName, simID, authHandler->GetName());

	/* Authentication can take a long time as it might now be going off to an external service.
	 * So we put the time consuming bit into a pool thread, then return to the server thread
	 * once that is done.
	 */

	// TODO this pool is blocking sim switching authentication!

	g_Scheduler.Pool([this, simID, authHandler, loginName, authHash](){
		g_Logs.simulator->info("Starting first phase for %v (%v) using %v", loginName, simID, authHandler->GetName());

		SimulatorThread *sim = g_SimulatorManager.GetPtrByID(simID);
		if(sim == NULL) {
			g_Logs.server->error("Lost simulator %v whilst initializing authentication. Aborting.", simID);
			return;
		}
		std::string errorMessage;
		AccountData *accPtr = authHandler->authenticate(loginName, authHash, &errorMessage);
		g_Logs.simulator->info("Completed first phase of authentication for %v (%v)", loginName, simID);

		if (accPtr == NULL) {
			g_Logs.simulator->error("[%v] Could not find account: %v", sim->InternalID, loginName);
			sim->ForceErrorMessage(g_Config.InvalidLoginMessage.c_str(), INFOMSG_ERROR);
			sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
		}
		else {

			if(errorMessage.length() > 0) {
				g_Logs.simulator->error("[%v] Authentication failed for %v. %v", sim->InternalID, loginName, errorMessage);
				sim->ForceErrorMessage(errorMessage.c_str(), INFOMSG_ERROR);
				sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
				return;
			}

			/* Now the lengthy operation is over, complete authentication on the main server thread */

			g_Logs.simulator->info("Submitting authentication on main thread for %v (%v)", accPtr->Name, simID);
//			auto accID = accPtr->ID;

			sim->Submit([this, sim, accPtr](){

//				auto innerAccPtr = g_AccountManager.FetchIndividualAccount(accID);

				g_Logs.server->info("Completing authentication on main thread for %v (%v)", accPtr->Name, sim->InternalID);

//				SimulatorThread *sim = g_SimulatorManager.GetPtrByID(simID);
//				if(sim == NULL) {
//					g_Logs.server->error("Lost simulator %v whilst authenticating. Aborting.", simID);
//					return;
//				}

				if(g_Config.MaintenanceMessage.length() > 0 && accPtr->HasPermission(Perm_Account, Permission_Admin) == false && accPtr->HasPermission(Perm_Account, Permission_Sage) == false)	{
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "The server is currently unavailable due to mainenance. The reason given was '%s'", g_Config.MaintenanceMessage.c_str());
					sim->ForceErrorMessage(sim->Aux1, INFOMSG_ERROR);
					sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
					return;
				}

				//Check for ban.
				if (accPtr->SuspendTimeSec >= 0) {
					unsigned long timePassed = g_PlatformTime.getAbsoluteSeconds()
							- accPtr->SuspendTimeSec;
					if (timePassed < accPtr->SuspendDurationSec) {
						unsigned long remain = accPtr->SuspendDurationSec - timePassed;
						Util::FormatTime(sim->Aux3, sizeof(sim->Aux3), remain);
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
								"Your account has been suspended. Time remaining: %s",
								sim->Aux3);
						sim->ForceErrorMessage(sim->Aux1, INFOMSG_ERROR);
						sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
						return;
					} else
						accPtr->ClearBan();
				}

				//Check for multiple logins.
				if (g_ClusterManager.CountAccountSessions(accPtr->ID) > 0) {
					if (accPtr->HasPermission(Perm_Account, Permission_Admin) == false) {
						g_SimulatorManager.CheckIdleSimulatorBoot(accPtr);
						sim->ForceErrorMessage("That account is already logged in.",
								INFOMSG_ERROR);
						sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
						return;
					}
				}

				g_Logs.event->info("[ACCOUNT] Account '%v' logged in", accPtr->Name);

				//If we get here, we can successfully log into the account.
				accPtr->AdjustSessionLoginCount(1);
				g_Logs.simulator->info("[%v] Logging in as: %v [Socket:%v]", sim->InternalID,
						accPtr->Name, sim->sc.ClientSocket);
				unsigned long startTime = g_PlatformTime.getMilliseconds();
				sim->LoadAccountCharacters(accPtr);
				g_Logs.simulator->debug("[%v] TIME PASS loading account chars: %v ms",
						sim->InternalID, g_PlatformTime.getMilliseconds() - startTime);

				int wpos = 0;
				wpos += PutByte(&sim->SendBuf[wpos], 50);       //_handleLoginQueueMessage
				wpos += PutShort(&sim->SendBuf[wpos], 0);       //Placeholder for message size
				wpos += PutInteger(&sim->SendBuf[wpos], 0);     //Queue position
				PutShort(&sim->SendBuf[1], wpos - 3);
				sim->AttemptSend(sim->SendBuf, wpos);

				g_Logs.event->info("[ACCOUNT] Sent queue position to %v", accPtr->Name);

			});
		}


	});

	g_Logs.simulator->info("Pooled authentication for %v (%v) using %v", loginName, simID, authHandler->GetName());

	return 0;
}

//
//SelectPersonaMessage
//

int SelectPersonaMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	short personaIndex = GetShort(&sim->readPtr[sim->ReadPos], sim->ReadPos);
	if (g_ProtocolVersion >= 38) {
		//Extra stuff for 0.8.9
		int var1 = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
		int var2 = GetInteger(&sim->readPtr[sim->ReadPos], sim->ReadPos);
		if (g_Logs.simulator->Enabled(el::Level::Trace)) {
			g_Logs.simulator->trace(
					"[%d] 0.8.9 login data - Persona:%d, Unknowns:%d,%d",
					sim->InternalID, personaIndex, var1, var2);
		}
	}
	sim->SetPersona(personaIndex);
	if (g_Logs.simulator->Enabled(el::Level::Trace)) {
		g_Logs.simulator->trace("[%v] -- SetPersona --", sim->InternalID);
	}
	return 0;
}

//
//LobbyQueryMessage
//
int LobbyQueryMessage::handleMessage(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	int PendingData = 0;
	query->Clear(); //Clear it before processing so that debug polls can fetch the most recent query.
	sim->ReadQueryFromMessage();
	QueryHandler *qh = g_QueryManager.getLobbyQueryHandler(query->name);
	if (qh == NULL) {
		g_Logs.simulator->warn("[%v] Unhandled query in lobby: %v", sim->InternalID,
				query->name.c_str());
	} else {
		PendingData = qh->handleQuery(sim, pld, query, creatureInstance);
	}
	return PendingData;
}
