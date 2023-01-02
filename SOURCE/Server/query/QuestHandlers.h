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

#ifndef QUESTHANDLERS_H
#define QUESTHANDLERS_H

#include "Query.h"

class QuestIndicatorHandler : public QueryHandler {
public:
	~QuestIndicatorHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestGetOfferHandler : public QueryHandler {
public:
	~QuestGetOfferHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestGenericDataHandler : public QueryHandler {
public:
	~QuestGenericDataHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestJoinHandler : public QueryHandler {
public:
	~QuestJoinHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestListHandler : public QueryHandler {
public:
	~QuestListHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestDataHandler : public QueryHandler {
public:
	~QuestDataHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestGetCompleteHandler : public QueryHandler {
public:
	~QuestGetCompleteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestCompleteHandler : public QueryHandler {
public:
	~QuestCompleteHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestLeaveHandler : public QueryHandler {
public:
	~QuestLeaveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestHackHandler : public QueryHandler {
public:
	~QuestHackHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class QuestShareHandler : public QueryHandler {
public:
	~QuestShareHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

int QuestGenericData(char *buffer, int bufsize, char *convBuf, int QuestID, int QueryIndex);
int QuestList(QuestJournal &journal, char *buffer, char *convBuf, int QueryID);
int QuestData(QuestJournal &journal, char *buffer, char *convBuf, int QuestID, int QueryIndex);

#endif
