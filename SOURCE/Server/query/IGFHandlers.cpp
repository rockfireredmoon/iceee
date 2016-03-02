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

#include "Query.h"
#include "IGFHandlers.h"
#include "../IGForum.h"
#include "../util/Log.h"

//
// IGFGetCategoryHandler
//

int IGFGetCategoryHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = category ID
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");
	int ID = atoi(query->args[0].c_str());

	MULTISTRING results;
	g_IGFManager.GetCategory(ID, results);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, results);
}

//
// IGFOpenCategoryHandler
//

int IGFOpenCategoryHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = object type (ex: category or thread)
	//[1] = object ID
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int type = atoi(query->args[0].c_str());
	int id = atoi(query->args[1].c_str());

	MULTISTRING results;
	g_IGFManager.OpenCategory(type, id, results);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, results);
}

//
// IGFOpenThreadHandler
//

int IGFOpenThreadHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = thread ID
	//[1] = starting post index
	//[2] = number of posts to retrieve
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int id = atoi(query->args[0].c_str());
	int start = atoi(query->args[1].c_str());
	int count = atoi(query->args[2].c_str());

	MULTISTRING results;
	g_IGFManager.OpenThread(id, start, count, results);
	unsigned int wpos = PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID,
			results);

	if (wpos >= sizeof(sim->SendBuf) - 1)
		g_Logs.simulator->error("[%v] IGF openthread response too large: %v",
				sim->InternalID, wpos);
	else if (wpos >= sizeof(sim->SendBuf) - 1000)
		g_Logs.simulator->warn(
				"[%d] IGF openthread response dangerous size: %d", sim->InternalID,
				wpos);

	return wpos;//PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, results);
}

//
// IGFSendPostHandler
//

int IGFSendPostHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = Post Type
	//[1] = Placement ID (category ID if creating a thread, thread ID if creating or editing a post)
	//[2] = Post ID (if editing a post)
	//[3] = thread title, if creating a new thread
	//[4] = post body
	if (query->argCount < 5)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int type = atoi(query->args[0].c_str());
	int placementID = atoi(query->args[1].c_str());
	int postID = atoi(query->args[2].c_str());
	const char *threadTitle = query->args[3].c_str();
	const char *postBody = query->args[4].c_str();

	const char *displayName = creatureInstance->css.display_name;
	int res = g_IGFManager.SendPost(pld->accPtr, type, placementID, postID,
			threadTitle, postBody, displayName);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFDeletePostHandler
//

int IGFDeletePostHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = thread ID
	//[1] = post ID
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");
	int threadID = atoi(query->args[0].c_str());
	int postID = atoi(query->args[1].c_str());

	int res = g_IGFManager.DeletePost(pld->accPtr, threadID, postID);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFSetLockStatusHandler
//

int IGFSetLockStatusHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {//[0] = Type (Category or Thread)
	//[1] = Object ID
	//[2] = Lock status (0 or 1)
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int type = atoi(query->args[0].c_str());
	int objectID = atoi(query->args[1].c_str());
	bool status = Util::IntToBool(atoi(query->args[2].c_str()));

	int res = g_IGFManager.SetLockStatus(pld->accPtr, type, objectID, status);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFSetStickyStatusHandler
//

int IGFSetStickyStatusHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = Type (Category or Thread)
	//[1] = Object ID
	//[2] = Sticky Status (0 or 1)
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int type = atoi(query->args[0].c_str());
	int objectID = atoi(query->args[1].c_str());
	bool status = Util::IntToBool(atoi(query->args[2].c_str()));

	int res = g_IGFManager.SetStickyStatus(pld->accPtr, type, objectID, status);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFEditObjectHandler
//

int IGFEditObjectHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = Object Type (category or thread)
	//[1] = Parent ID (if creating a category, this is where to place it)
	//[2] = Rename ID (nonzero indicates which ID is being renamed)
	//[3] = String name to give the object
	if (query->argCount < 4)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int type = atoi(query->args[0].c_str());
	int parentID = atoi(query->args[1].c_str());
	int renameID = atoi(query->args[2].c_str());
	const char *name = query->args[3].c_str();

	int res = g_IGFManager.EditObject(pld->accPtr, type, parentID, renameID,
			name);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFDeleteObjectHandler
//

int IGFDeleteObjectHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//[0] = Object Type (category or thread)
	//[1] = Object ID
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	int objectType = atoi(query->args[0].c_str());
	int objectID = atoi(query->args[1].c_str());

	int res = g_IGFManager.DeleteObject(pld->accPtr, objectType, objectID);
	if (res < 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				g_IGFManager.GetErrorString(res));

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFRunActionHandler
//

int IGFRunActionHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount >= 3) {
		int action = query->GetInteger(0);
		int param1 = query->GetInteger(1);
		int param2 = query->GetInteger(2);
		int res = g_IGFManager.RunAction(pld->accPtr, action, param1, param2);
		if (res < 0)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					g_IGFManager.GetErrorString(res));
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// IGFMoveHandler
//

int IGFMoveHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount >= 4) {
		int srcType = query->GetInteger(0);
		int srcID = query->GetInteger(1);
		int dstType = query->GetInteger(2);
		int dstID = query->GetInteger(3);
		int res = g_IGFManager.RunMove(pld->accPtr, srcType, srcID, dstType,
				dstID);
		if (res < 0)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					g_IGFManager.GetErrorString(res));
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
