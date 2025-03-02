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

#ifndef SCENERYHANDLERS_H
#define SCENERYTHANDLERS_H

#include "Query.h"

class SceneryDeleteHandler : public QueryHandler {
public:
	~SceneryDeleteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_scenery_delete(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};

class SceneryEditHandler : public QueryHandler {
public:
	~SceneryEditHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_scenery_edit(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};

class CreatureEditHandler : public QueryHandler {
public:
	~CreatureEditHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_creature_edit(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};

class SceneryLinkAddHandler : public QueryHandler {
public:
	~SceneryLinkAddHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SceneryLinkDelHandler : public QueryHandler {
public:
	~SceneryLinkDelHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SceneryListHandler : public QueryHandler {
public:
	~SceneryListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
