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

#ifndef ITEMHANDLERS_H
#define ITEMHANDLERS_H

#include "Query.h"

void AddPet(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance, int CDefID);

int UseItem(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance, unsigned int CCSID);


class ItemUseHandler : public QueryHandler {
public:
	~ItemUseHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ItemDefUseHandler : public QueryHandler {
public:
	~ItemDefUseHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ItemContentsHandler : public QueryHandler {
public:
	~ItemContentsHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ItemMoveHandler : public QueryHandler {
public:
	~ItemMoveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ItemDeleteHandler : public QueryHandler {
public:
	~ItemDeleteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ItemSplitHandler : public QueryHandler {
public:
	~ItemSplitHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
