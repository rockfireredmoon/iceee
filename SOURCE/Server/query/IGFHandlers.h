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

#ifndef IGFHANDLERS_H
#define IGFTHANDLERS_H

#include "Query.h"

//g_QueryManager.queryHandlers["mod.igforum.getcategory"] = new IGFGetCategoryHandler();
//	g_QueryManager.queryHandlers["mod.igforum.opencategory"] = new IGFOpenCategoryHandler();
//	g_QueryManager.queryHandlers["mod.igforum.openthread"] = new IGFOpenThreadHandler();
//	g_QueryManager.queryHandlers["mod.igforum.sendpost"] = new IGFSendPostHandler();
//	g_QueryManager.queryHandlers["mod.igforum.deletepost"] = new IGFDeletePostHandler();
//	g_QueryManager.queryHandlers["mod.igforum.setlockstatus"] = new IGFSetLockStatusHandler();
//	g_QueryManager.queryHandlers["mod.igforum.setstickystatus"] = new IGFSetStickStatusHandler();
//	g_QueryManager.queryHandlers["mod.igforum.editobject"] = new IGFEditObjectHandler();
//	g_QueryManager.queryHandlers["mod.igforum.deleteobject"] = new IGFDeleteObjectHandler();
//	g_QueryManager.queryHandlers["mod.igforum.runaction"] = new IGFRunActionHandler();
//	g_QueryManager.queryHandlers["mod.igforum.move"] = new IGFMoveHandler();

class IGFGetCategoryHandler : public QueryHandler {
public:
	~IGFGetCategoryHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};

class IGFOpenCategoryHandler : public QueryHandler {
public:
	~IGFOpenCategoryHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFOpenThreadHandler : public QueryHandler {
public:
	~IGFOpenThreadHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFSendPostHandler : public QueryHandler {
public:
	~IGFSendPostHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFDeletePostHandler : public QueryHandler {
public:
	~IGFDeletePostHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFSetLockStatusHandler : public QueryHandler {
public:
	~IGFSetLockStatusHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFSetStickyStatusHandler : public QueryHandler {
public:
	~IGFSetStickyStatusHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFEditObjectHandler : public QueryHandler {
public:
	~IGFEditObjectHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFDeleteObjectHandler : public QueryHandler {
public:
	~IGFDeleteObjectHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFRunActionHandler : public QueryHandler {
public:
	~IGFRunActionHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
class IGFMoveHandler : public QueryHandler {
public:
	~IGFMoveHandler() {};
	int handleQuery(SimulatorThread *sim, CharacterServerData *pld, SimulatorQuery *query, CreatureInstance *creatureInstance);
};
#endif
