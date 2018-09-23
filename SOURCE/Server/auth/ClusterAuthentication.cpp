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
 
#include "ClusterAuthentication.h"
#include "../ByteBuffer.h"
#include "../Cluster.h"
#include "../Character.h"
#include "../util/Log.h"
#include <string>

ClusterAuthenticationHandler::ClusterAuthenticationHandler() {

}

ClusterAuthenticationHandler::~ClusterAuthenticationHandler() {

}

AccountData * ClusterAuthenticationHandler::authenticate(const std::string &loginName, const std::string &authorizationHash, std::string *errorMessage) {

	AccountData *accPtr = NULL;
	CharacterData *cd = NULL;

	PendingShardPlayer psp = g_ClusterManager.FindToken(authorizationHash);
	if(psp.mID == 0) {
		g_Logs.data->error("Could not find token %v associated with user %v", authorizationHash, loginName);
		return NULL;
	}

	g_Logs.data->info("Cluster authentication token password %v for user user %v", authorizationHash, loginName);
	g_CharacterManager.GetThread("ClusterAuthenticationHandler::authenticate");
	cd = g_CharacterManager.RequestCharacter(psp.mID, false);
	if(cd == NULL) {
		g_Logs.data->warn("Could not find character %v associated with token %v", psp.mID, psp.mToken);
		g_CharacterManager.ReleaseThread();
		return NULL;
	}
	g_CharacterManager.ReleaseThread();

	g_Logs.data->info("Retrieving account for %v", loginName);
	g_AccountManager.cs.Enter("ClusterAuthenticationHandler::authenticate");
	accPtr = g_AccountManager.FetchIndividualAccount(cd->AccountID);
	g_AccountManager.cs.Leave();

	return accPtr;

}
