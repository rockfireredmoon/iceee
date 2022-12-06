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
 
#include "DevAuthentication.h"
#include "../ByteBuffer.h"

DevAuthenticationHandler::DevAuthenticationHandler() {

}

DevAuthenticationHandler::~DevAuthenticationHandler() {

}

std::string DevAuthenticationHandler::GetName() {
	return "User/Password";
}

AccountData * DevAuthenticationHandler::authenticate(const std::string &loginName, const std::string &authorizationHash, std::string *errorMessage) {

	AccountData *accPtr = NULL;

	std::string password;
	AccountData::GenerateSaltedHash(authorizationHash.c_str(), password);

	g_AccountManager.cs.Enter("SimulatorThread::handle_lobby_authenticate");
	accPtr = g_AccountManager.GetValidLogin(loginName.c_str(), password.c_str());
	g_AccountManager.cs.Leave();

	return accPtr;

}
