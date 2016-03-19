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

#ifndef QUERY_H
#define QUERY_H

#include "../Simulator.h"
#include "../ByteBuffer.h"
#include "../Character.h"
#include "../Creature.h"

#include "../util/Log.h"


class QueryHandler {
public:
	virtual ~QueryHandler()=0;
	virtual int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance)=0;
};

class QueryManager {
public:
	QueryManager();
	~QueryManager();
	QueryHandler *getQueryHandler(std::string query);
	QueryHandler *getLobbyQueryHandler(std::string query);

	std::map<std::string, QueryHandler*> lobbyQueryHandlers;
	std::map<std::string, QueryHandler*> queryHandlers;
};

extern QueryManager g_QueryManager;

#endif

