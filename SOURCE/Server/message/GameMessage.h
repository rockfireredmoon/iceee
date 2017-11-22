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

#ifndef GAMEMESSAGE_H
#define GAMEMESSAGE_H

#include "MessageHandler.h"

class GameQueryMessage : public MessageHandler {
public:
	~GameQueryMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};


class InspectCreatureDefMessage : public MessageHandler {
public:
	~InspectCreatureDefMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class UpdateVelocityMessage : public MessageHandler {
public:
	~UpdateVelocityMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SelectTargetMessage : public MessageHandler {
public:
	~SelectTargetMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CommunicateMessage : public MessageHandler {
public:
	~CommunicateMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	char LogBuffer[4096];
};

class InspectCreatureMessage : public MessageHandler {
public:
	~InspectCreatureMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AbilityActiveMessage : public MessageHandler {
public:
	~AbilityActiveMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SwimStateChangeMessage : public MessageHandler {
public:
	~SwimStateChangeMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DisconnectMessage : public MessageHandler {
public:
	~DisconnectMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class DebugServerPingMessage : public MessageHandler {
public:
	~DebugServerPingMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class MouseClickMessage : public MessageHandler {
public:
	~MouseClickMessage() {};
	int handleMessage(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
