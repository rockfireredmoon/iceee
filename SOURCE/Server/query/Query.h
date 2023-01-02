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
#include <functional>
#include <initializer_list>


using namespace std;


class QueryHandler {
public:
	virtual ~QueryHandler()=0;
	virtual int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance)=0;
};

class QueryManager {
public:
	QueryManager();
	~QueryManager();
	QueryHandler *getQueryHandler(string query);
	QueryHandler *getLobbyQueryHandler(string query);

	map<string, QueryHandler*> lobbyQueryHandlers;
	map<string, QueryHandler*> queryHandlers;
};


class QueryResponse {
public:
	QueryResponse(int queryID);
	QueryResponse(int queryID, char *buffer);
	~QueryResponse();
	vector<string>* Row();
	void AddRow(initializer_list<reference_wrapper<string>> list);
	unsigned int Data();
	unsigned int Error(const string &error);
	unsigned int String(const string &string);
	unsigned int Null();
	unsigned int Write(char *buffer);
private:
	bool mCommitted;
	bool mBufferSet;
	char *mBuffer;
	int mQueryID;
	vector<vector<string>> mResponse;
};

extern QueryManager g_QueryManager;

#endif

