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
#include "Lobby.h"

//
// CommandHandler
//
QueryHandler::~QueryHandler() {
}

//
// CommandManager
//

QueryManager g_QueryManager;

QueryManager::QueryManager() {
	commandHandlers["util.ping"] = new LobbyPingHandler();
	commandHandlers["account.tracking"] = new AccountTrackingHandler();
}

QueryManager::~QueryManager() {
}

QueryHandler *QueryManager :: getCommandHandler(std::string command)
{
	if(commandHandlers.find(command) == commandHandlers.end())
		return NULL;
	return commandHandlers[command];
}
