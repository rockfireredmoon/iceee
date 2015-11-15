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

#ifndef CSHANDLERS_H
#define CSHANDLERS_H

#include "Query.h"

class CreditShopBuyHandler: public QueryHandler {
public:
	~CreditShopBuyHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreditShopListHandler: public QueryHandler {
public:
	~CreditShopListHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreditShopEditHandler: public QueryHandler {
public:
	~CreditShopEditHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreditShopPurchaseNameHandler: public QueryHandler {
public:
	~CreditShopPurchaseNameHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class CreditShopReloadHandler: public QueryHandler {
public:
	~CreditShopReloadHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
