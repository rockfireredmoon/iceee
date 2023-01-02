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

#include "FriendHandlers.h"
#include "../Creature.h"
#include "../FriendStatus.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//AddFriendHandler
//

int AddFriendHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: friends.add
	 Adds a player to the friend list.
	 Args : [0] = Character name to add.
	 */

	if (query->argCount < 1)
		return 0;

	const char *friendName = query->args[0].c_str();

	if (strcmp(friendName, creatureInstance->css.display_name) == 0) {
		sim->SendInfoMessage("You cannot add yourself as a friend.", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}

	g_CharacterManager.GetThread("SimulatorThread::handle_query_friends_add");
	CharacterData *friendPtr = g_CharacterManager.GetCharacterByName(
			friendName);
	if (friendPtr != NULL) {
		pld->charPtr->AddFriend(friendPtr->cdef.CreatureDefID,
				friendPtr->cdef.css.display_name);
		sim->UpdateSocialEntry(true, true);
		sim->AttemptSend(sim->SendBuf, PrepExt_FriendsAdd(sim->SendBuf, friendPtr));
	}
	g_CharacterManager.ReleaseThread();

	if (friendPtr == NULL) {
		Util::SafeFormat(sim->LogBuffer, sizeof(sim->LogBuffer),
				"Character is not online, or does not exist [%s]", friendName);
		sim->SendInfoMessage(sim->LogBuffer, INFOMSG_ERROR);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}


//
//RemoveFriendHandler
//

int RemoveFriendHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: friends.remove
	 Removes a player from the friend list.
	 Args : [0] = Character name to remove.
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

	const char *friendName = query->args[0].c_str();

	g_CharacterManager.GetThread(
			"SimulatorThread::handle_query_friends_remove");
	int res = pld->charPtr->RemoveFriend(friendName);
	sim->UpdateSocialEntry(true, true);
	g_CharacterManager.ReleaseThread();

	if (res == 0) {
		Util::SafeFormat(sim->LogBuffer, sizeof(sim->LogBuffer),
				"Character [%s] was not found in friend list", friendName);
		sim->SendInfoMessage(sim->LogBuffer, INFOMSG_ERROR);
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ListFriendsHandler
//

int ListFriendsHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: friends.list
	 Retrieves the friend list, with information like name, level, shard, etc.
	 Args : [none]
	 */

	if (pld->zoneDef == NULL) {
		g_Logs.simulator->error(
				"[%d] HandleQuery_friends_list() ZoneDef is NULL", sim->InternalID);
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}

	FriendListManager::SEARCH_INPUT search;
	FriendListManager::SEARCH_OUTPUT resList;
	for (size_t i = 0; i < pld->charPtr->friendList.size(); i++)
		search.push_back(
				FriendListManager::SEARCH_PAIR(
						pld->charPtr->friendList[i].CDefID,
						pld->charPtr->friendList[i].Name));
	g_FriendListManager.EnumerateFriends(search, resList);

	QueryResponse resp(query->ID);
	for(auto res : resList) {
		auto row = resp.Row();

		//Character Name
		row->push_back(res.name);

		//Level
		row->push_back(to_string(res.level));

		//Profession (integer)
		row->push_back(to_string(res.profession));

		//Online ("true", "false")
		row->push_back((res.online == true) ? "true" : "false");

		//Status Message
		row->push_back(res.status);

		//Shard
		row->push_back(res.shard);
	}

	return resp.Write(sim->SendBuf);
}


//
//FriendStatusHandler
//

int FriendStatusHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: friends.status
	 Changes the player's status text.
	 Args : [0] = New status text.
	 */

	if (query->argCount < 1)
		return 0;

	const char *newStatus = query->args[0].c_str();

	pld->charPtr->StatusText = newStatus;
	int wpos = 0;

	wpos += PutByte(&sim->SendBuf[wpos], 43);  //_handleFriendNotificationMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);  //size
	wpos += PutByte(&sim->SendBuf[wpos], 3);   //message type for status changed
	wpos += PutStringUTF(&sim->SendBuf[wpos], pld->charPtr->cdef.css.display_name);
	wpos += PutStringUTF(&sim->SendBuf[wpos], newStatus);
	PutShort(&sim->SendBuf[1], wpos - 3);

	size_t b;

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->ProtocolState == 1 && it->isConnected == true) {
			for (b = 0; b < it->pld.charPtr->friendList.size(); b++) {
				if ((it->pld.charPtr->friendList[b].CDefID == pld->CreatureDefID)
						|| (it->pld.CreatureDefID == pld->CreatureDefID)) {
					it->AttemptSend(sim->SendBuf, wpos);
					break;
				}
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}


//
//GetFriendStatusHandler
//

int GetFriendStatusHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: friends.getstatus
	 Retrieves the player's friend status text.
	 Args : [none]
	 */

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
			pld->charPtr->StatusText.c_str());
}

