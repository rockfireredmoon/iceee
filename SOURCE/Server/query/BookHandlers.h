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

#ifndef BOOKHANDLERS_H
#define BOOKHANDLERS_H

#include "Query.h"

class BookListHandler: public QueryHandler {
public:
	~BookListHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	void PopulateBookList(QueryResponse &response, InventoryManager &inv, int container, std::set<int> &booksFound);
};

class BookGetHandler: public QueryHandler {
public:
	~BookGetHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	void PopulateBookDetails(QueryResponse &response, InventoryManager &inv, int container, std::set<int> &pagesFoundSet, BookDefinition &def);

};

class BookItemHandler: public QueryHandler {
public:
	~BookItemHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};
#endif
