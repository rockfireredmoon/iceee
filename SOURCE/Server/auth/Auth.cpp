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

#include "Auth.h"
#include "../Util.h"
#include "../util/Log.h"
#include "../Config.h"
#include "../StringList.h"


#include "ServiceAuthentication.h"
#include "DevAuthentication.h"

//
// AuthHandler
//
AuthHandler::AuthHandler() {
}

AuthHandler::~AuthHandler() {
}

AccountData * AuthHandler::authenticate(SimulatorThread *sim) {
	char loginName[128];
	char authHash[128];

	GetStringUTF(&sim->readPtr[sim->ReadPos], loginName, sizeof(loginName), sim->ReadPos);  //login name
	GetStringUTF(&sim->readPtr[sim->ReadPos], authHash, sizeof(authHash), sim->ReadPos);  //authorization hash or or X-CSRF-Token:sess_id:session_name:uid

	if(g_AccountManager.AcceptingLogins() == false)
	{
		sim->ForceErrorMessage("Not accepting more logins.", INFOMSG_ERROR);
		sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
		return NULL;
	}

	AccountData *accPtr = this->onAuthenticate(sim, std::string(loginName), std::string(authHash));

	if(accPtr == NULL)	{
		g_Logs.simulator->error("[%v] Could not find account: %v", sim->InternalID, loginName);
		sim->ForceErrorMessage(g_Config.InvalidLoginMessage.c_str(), INFOMSG_ERROR);
		sim->Disconnect("SimulatorThread::handle_lobby_authenticate");
	}

	return accPtr;
}

//
// AuthManager
//

AuthManager g_AuthManager;

AuthManager::AuthManager()
{
	AuthHandler *dev = new DevAuthenticationHandler();
	AuthHandler *service = new ServiceAuthenticationHandler();
	authHandlers[(char)1] = dev;
	authHandlers[(char)2] = service;
}

AuthManager::~AuthManager()
{
	delete authHandlers.find((char)0)->second;
	delete authHandlers.find((char)1)->second;
}

AuthHandler *AuthManager :: GetAuthenticator(char authCode)
{
	if(authHandlers.find(authCode) == authHandlers.end())
		return NULL;
	return authHandlers[authCode];
}
