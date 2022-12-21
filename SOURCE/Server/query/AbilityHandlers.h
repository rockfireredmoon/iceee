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
#ifndef ABILITYHANDLERS_H
#define ABILITYHANDLERS_H

#include "Query.h"
using namespace std;

class AbilityBuyHandler : public QueryHandler {
public:
	~AbilityBuyHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AbilityOwnageListHandler : public QueryHandler {
public:
	~AbilityOwnageListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AbilityRespecHandler : public QueryHandler {
public:
	~AbilityRespecHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AbilityRespecPriceHandler : public QueryHandler {
public:
	~AbilityRespecPriceHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class AbilityRemainingCooldownsHandler : public QueryHandler {
public:
	~AbilityRemainingCooldownsHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class BuffRemoveHandler : public QueryHandler {
public:
	~BuffRemoveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

#endif
