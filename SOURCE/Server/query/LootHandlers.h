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

#ifndef LOOTHANDLERS_H
#define LOOTHANDLERS_H

#include "Query.h"
#include "../PartyManager.h"

class LootListHandler : public QueryHandler {
public:
	~LootListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class LootItemHandler : public QueryHandler {
public:
	~LootItemHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);

};

class LootExitHandler : public QueryHandler {
public:
	~LootExitHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class LootNeedGreedPassHandler : public QueryHandler {
public:
	~LootNeedGreedPassHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
private:
	int protected_helper_query_loot_need_greed_pass(SimulatorThread *sim, CharacterServerData *pld,
			SimulatorQuery *query, CreatureInstance *creatureInstance);
};
#endif

int OfferLoot(SimulatorThread *sim, int mode, ActiveLootContainer *loot, ActiveParty *party, CreatureInstance *receivingCreature, int ItemID, bool needOrGreed, int CID, int conIndex);
void CheckIfLootReadyToDistribute(SimulatorThread *sim, CreatureInstance *creatureInstance, ActiveLootContainer *loot, LootTag lootTag);
int protected_helper_query_loot_item(SimulatorThread *sim, int CID, int itemID, CreatureInstance *creatureInstance, CharacterServerData *pld, int &conIndex);
void ClearLoot(ActiveParty *party, ActiveLootContainer *loot);
void ResetLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId);
void UndoLoot(ActiveParty *party, ActiveLootContainer *loot, int itemId, int creatureId);
PartyMember * RollForPartyLoot(SimulatorThread *sim, ActiveParty *party, std::set<int> creatureIds, const char *rollType, int itemId);
