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

#ifndef ZONEHANDLERS_H
#define ZONEHANDLERS_H

#include "Query.h"

class GoHandler : public QueryHandler {
public:
	~GoHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GroveEnvironmentCycleToggleHandler : public QueryHandler {
public:
	~GroveEnvironmentCycleToggleHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetEnvironmentHandler : public QueryHandler {
public:
	~SetEnvironmentHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ShardListHandler : public QueryHandler {
public:
	~ShardListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class WorldListHandler : public QueryHandler {
public:
	~WorldListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class LootPackagesListHandler : public QueryHandler {
public:
	~LootPackagesListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ZoneListHandler : public QueryHandler {
public:
	~ZoneListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ZoneGetHandler : public QueryHandler {
public:
	~ZoneGetHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ZoneEditHandler : public QueryHandler {
public:
	~ZoneEditHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ShardSetHandler : public QueryHandler {
public:
	~ShardSetHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class HengeSetDestHandler : public QueryHandler {
public:
	~HengeSetDestHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class MapMarkerHandler : public QueryHandler {
public:
	~MapMarkerHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetGroveStartHandler : public QueryHandler {
public:
	~SetGroveStartHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class PortalAcceptRequestHandler : public QueryHandler {
public:
	~PortalAcceptRequestHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class BuildTemplateListHandler : public QueryHandler {
public:
	~BuildTemplateListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class GetDungeonProfilesHandler : public QueryHandler {
public:
	~GetDungeonProfilesHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class SetATSHandler : public QueryHandler {
public:
	~SetATSHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
#endif
