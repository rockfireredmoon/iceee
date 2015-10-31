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


#ifndef CLANHANDLERS_H
#define CLANHANDLERS_H

#include "Query.h"

void BroadcastClanRankChange(std::string memberName, Clans::Clan &c, Clans::ClanMember &them);
void BroadcastClanDisbandment(Clans::Clan &clan);

class ClanRemoveHandler : public QueryHandler {
public:
	~ClanRemoveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanLeaveHandler : public QueryHandler {
public:
	~ClanLeaveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanInviteHandler : public QueryHandler {
public:
	~ClanInviteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanInviteAcceptHandler : public QueryHandler {
public:
	~ClanInviteAcceptHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanDisbandHandler : public QueryHandler {
public:
	~ClanDisbandHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanMotdHandler : public QueryHandler {
public:
	~ClanMotdHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanRankHandler : public QueryHandler {
public:
	~ClanRankHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanCreateHandler : public QueryHandler {
public:
	~ClanCreateHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class ClanListHandler : public QueryHandler {
public:
	~ClanListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};


class ClanInfoHandler : public QueryHandler {
public:
	~ClanInfoHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
#endif
