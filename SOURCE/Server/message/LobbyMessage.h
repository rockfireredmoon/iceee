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

#ifndef LOBBYMESSAGE_H
#define LOBBYMESSAGE_H

#include "MessageHandler.h"

class LobbyAuthenticateMessage : public MessageHandler {
public:
	~LobbyAuthenticateMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SelectPersonaMessage : public MessageHandler {
public:
	~SelectPersonaMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class LobbyQueryMessage : public MessageHandler {
public:
	~LobbyQueryMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};


#endif
