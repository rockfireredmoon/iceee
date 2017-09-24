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

#include "MessageHandler.h"

//
// MessageHandler
//
MessageHandler::~MessageHandler() {
}

//
// CommandManager
//

MessageManager g_MessageManager;

MessageManager::MessageManager() {
}

MessageManager::~MessageManager() {
}

MessageHandler *MessageManager::getMessageHandler(int message) {
	if (messageHandlers.find(message) == messageHandlers.end())
		return NULL;
	return messageHandlers[message];
}

MessageHandler *MessageManager::getLobbyMessageHandler(int message) {
	if (lobbyMessageHandlers.find(message) == lobbyMessageHandlers.end())
		return NULL;
	return lobbyMessageHandlers[message];
}

