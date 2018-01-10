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



#include "ServiceAuthentication.h"
#include "DevAuthentication.h"
#include "ClusterAuthentication.h"

//
// AuthHandler
//
AuthHandler::AuthHandler() {
}

AuthHandler::~AuthHandler() {
}

//
// AuthManager
//

AuthManager g_AuthManager;

AuthManager::AuthManager()
{
	AuthHandler *dev = new DevAuthenticationHandler();
	AuthHandler *service = new ServiceAuthenticationHandler();
	AuthHandler *cluster = new ClusterAuthenticationHandler();
	authHandlers[(char)1] = dev;
	authHandlers[(char)2] = service;
	authHandlers[(char)3] = cluster;
}

AuthManager::~AuthManager()
{
	delete authHandlers.find((char)1)->second;
	delete authHandlers.find((char)2)->second;
	delete authHandlers.find((char)3)->second;
}

AuthHandler *AuthManager :: GetAuthenticator(char authCode)
{
	if(authHandlers.find(authCode) == authHandlers.end())
		return NULL;
	return authHandlers[authCode];
}
