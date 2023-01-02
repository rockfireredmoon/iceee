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
#ifndef CREATUREHANDLERS_H
#define CREATUREHANDLERS_H

#include "Query.h"
using namespace std;

class CreatureIsUsableHandler : public QueryHandler {
public:
	~CreatureIsUsableHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreatureDefEditHandler : public QueryHandler {
public:
	~CreatureDefEditHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreatureUseHandler : public QueryHandler {
public:
	~CreatureUseHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreatureDeleteHandler : public QueryHandler {
public:
	~CreatureDeleteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

int protected_helper_tweak_self(SimulatorThread *sim, SimulatorQuery *query, int CDefID, int defhints, int argOffset);

#endif
