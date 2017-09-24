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

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "../Simulator.h"
#include "../ByteBuffer.h"
#include "../Character.h"
#include "../Creature.h"

#include "../util/Log.h"


class MessageHandler {
public:
	virtual ~MessageHandler()=0;
	virtual int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance)=0;
};

class MessageManager {
public:
	MessageManager();
	~MessageManager();
	MessageHandler *getMessageHandler(int message);
	MessageHandler *getLobbyMessageHandler(int message);

	std::map<int, MessageHandler*> lobbyMessageHandlers;
	std::map<int, MessageHandler*> messageHandlers;
};

extern MessageManager g_MessageManager;

#endif

