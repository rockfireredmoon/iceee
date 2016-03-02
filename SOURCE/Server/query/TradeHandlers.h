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

#ifndef TRADEHANDLERS_H
#define TRADEHANDLERS_H

#include "Query.h"
#include "../Simulator.h"

class TradeShopHandler: public QueryHandler {
public:
	~TradeShopHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int helper_trade_shop_buyback(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance, unsigned int CCSID);
	int helper_trade_shop_sell(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance,
			const char *itemHex);
};

class TradeItemsHandler: public QueryHandler {
public:
	~TradeItemsHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_items(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
class TradeEssenceHandler: public QueryHandler {
public:
	~TradeEssenceHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class TradeStartHandler: public QueryHandler {
public:
	~TradeStartHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_start(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
class TradeCancelHandler: public QueryHandler {
public:
	~TradeCancelHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_cancel(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
class TradeOfferHandler: public QueryHandler {
public:
	~TradeOfferHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_offer(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
class TradeAcceptHandler: public QueryHandler {
public:
	~TradeAcceptHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_accept(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
class TradeCurrencyHandler: public QueryHandler {
public:
	~TradeCurrencyHandler() {
	}

	int handleQuery(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_trade_currency(SimulatorThread *sim,
			CharacterServerData *pld, SimulatorQuery *query,
			CreatureInstance *creatureInstance);
};
#endif
