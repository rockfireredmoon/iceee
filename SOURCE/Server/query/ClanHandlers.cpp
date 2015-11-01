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

#include "ClanHandlers.h"
#include "../Account.h"
#include "../Config.h"
#include <algorithm>

using namespace Clans;
using namespace std;

//
// ClanDisbandHandler
//

void BroadcastClanDisbandment(Clan &clan) {
	char SendBuf[7];
	char Aux2[256];
	int wpos = 0;
	wpos += PutByte(&SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutByte(&SendBuf[wpos], 8);
	PutShort(&SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->isConnected == true && it->ProtocolState == 1)
			if (clan.HasMember(it->pld.CreatureDefID))
				it->AttemptSend(SendBuf, wpos);

	Util::SafeFormat(Aux2, sizeof(Aux2), "Clan '%s' has been disbanded",
			clan.mName.c_str());
	g_SimulatorManager.BroadcastMessage(Aux2);
}

void SendClanInvite(Clan &clan, ClanMember &leader, int cdefID) {
	CharacterData *leaderData = g_CharacterManager.RequestCharacter(leader.mID,
			true);

	int wpos = 0;
	char SendBuf[256];
	wpos += PutByte(&SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutByte(&SendBuf[wpos], 10);
	wpos += PutStringUTF(&SendBuf[wpos], clan.mName.c_str());
	wpos += PutStringUTF(&SendBuf[wpos],
			leaderData == NULL ? "Unknown" : leaderData->cdef.css.display_name);
	PutShort(&SendBuf[1], wpos - 3);
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1
				&& it->pld.CreatureDefID == cdefID) {
			it->AttemptSend(SendBuf, wpos);
			break;
		}
	}
}

void BroadcastClanRankChange(string memberName, Clan &clan,
		ClanMember &member) {
	int wpos = 0;
	char SendBuf[256];
	wpos += PutByte(&SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&SendBuf[wpos], 0);
	wpos += PutByte(&SendBuf[wpos], 14);
	wpos += PutStringUTF(&SendBuf[wpos], memberName.c_str());
	wpos += PutStringUTF(&SendBuf[wpos], Rank::GetNameByID(member.mRank));
	PutShort(&SendBuf[1], wpos - 3);
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->isConnected == true && it->ProtocolState == 1)
			if (clan.HasMember(it->pld.CreatureDefID))
				it->AttemptSend(SendBuf, wpos);
}

int ClanDisbandHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.disband
	 Disband a clan.
	 Args: [none]
	 */

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	Clan c = g_ClanManager.mClans[clanID];
	ClanMember member = c.GetMember(pld->CreatureDefID);
	if (member.mRank != Rank::LEADER) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must be the leader to set the MOTD!");
	}

	for (vector<ClanMember>::iterator it = c.mMembers.begin();
			it != c.mMembers.end(); ++it) {
		CharacterData *cd = g_CharacterManager.RequestCharacter((*it).mID,
				true);
		if (cd != NULL) {
			cd->clan = 0;
			cd->pendingChanges++;
		}
	}

	BroadcastClanDisbandment(c);
	g_ClanManager.RemoveClan(c);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanCreateHandler
//

int ClanCreateHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.create
	 Creates a clan
	 Args: [clan]
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	if (creatureInstance->charPtr->clan > 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You may not create a clan, you are already part of one.");
	}

	if (creatureInstance->css.copper < g_Config.ClanCost)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You do not have enough gold to create a clan.");

	creatureInstance->AdjustCopper(-g_Config.ClanCost);

	int clanID = g_ClanManager.FindClanID(query->args[0]);
	if (clanID == -1) {
		Clans::Clan c;
		Clans::ClanMember m;
		m.mID = pld->CreatureDefID;
		m.mRank = Clans::Rank::LEADER;
		c.mMembers.push_back(m);
		c.mName = query->args[0];
		g_ClanManager.CreateClan(c);

		creatureInstance->charPtr->clan = c.mId;
		creatureInstance->charPtr->pendingChanges++;

		int wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 43);
		wpos += PutShort(&sim->SendBuf[wpos], 0);
		wpos += PutByte(&sim->SendBuf[wpos], 9);
		wpos += PutStringUTF(&sim->SendBuf[wpos], c.mName.c_str());
		PutShort(&sim->SendBuf[1], wpos - 3);

		sim->AttemptSend(sim->SendBuf, wpos);

		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"%s has been formed! '%s'",
				creatureInstance->charPtr->cdef.css.display_name,
				c.mName.c_str());
		g_SimulatorManager.BroadcastMessage(sim->Aux2);

		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	} else {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"A clan with that name already exists.");
	}
}

//
// ClanInfoHandler
//

int ClanInfoHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: clan.info
	 Retrieves the clan info of the Simulator player.
	 Args: [none]
	 */

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID); //Query response index

	int clanID = creatureInstance->charPtr->clan;
	if (!g_Config.Clans || clanID == 0 || !g_ClanManager.HasClan(clanID)) {
		wpos += PutShort(&sim->SendBuf[wpos], 0);
	} else {
		Clans::Clan c = g_ClanManager.mClans[clanID];
		Clans::ClanMember leader = c.GetFirstMemberOfRank(Clans::Rank::LEADER);
		wpos += PutShort(&sim->SendBuf[wpos], 3);

		wpos += PutByte(&sim->SendBuf[wpos], 1);
		wpos += PutStringUTF(&sim->SendBuf[wpos], c.mName.c_str());  //Clan name

		wpos += PutByte(&sim->SendBuf[wpos], 1);
		wpos += PutStringUTF(&sim->SendBuf[wpos], c.mMOTD.c_str()); //Message of the day

		wpos += PutByte(&sim->SendBuf[wpos], 1);
		wpos += PutStringUTF(&sim->SendBuf[wpos],
				pld->CreatureDefID == leader.mID ? "true" : "false"); //Clan leader's name.
	}

	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
// ClanMotdHandler
//

int ClanMotdHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: clan.motd
	 Set the motd.
	 Args: [none]
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	Clans::Clan c = g_ClanManager.mClans[clanID];
	Clans::ClanMember member = c.GetMember(pld->CreatureDefID);
	if (member.mRank < Clans::Rank::OFFICER) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must be at least an officer to set the MOTD!");
	}
	c.mMOTD = query->args[0];
	g_ClanManager.SaveClan(c);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 43); //_handleFriendNotificationMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutByte(&sim->SendBuf[wpos], 5);
	wpos += PutStringUTF(&sim->SendBuf[wpos], c.mMOTD.c_str());
	PutShort(&sim->SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->isConnected == true && it->ProtocolState == 1)
			if (c.HasMember(it->pld.CreatureDefID))
				it->AttemptSend(sim->SendBuf, wpos);

	Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "Clan '%s' - %s",
			c.mName.c_str(), c.mMOTD.c_str());
	g_SimulatorManager.BroadcastMessage(sim->Aux2);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanInviteHandler
//

int ClanInviteAcceptHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.invite
	 Invite a player.
	 Args: [none]
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	int myClanID = creatureInstance->charPtr->clan;
	if (myClanID != 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are already in a clan.");

	int clanID = g_ClanManager.FindClanID(query->args[0].c_str());
	if (clanID == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"The clan does not exist!");

	Clans::Clan c = g_ClanManager.mClans[clanID];
	std::vector<int>::iterator pendingIt = std::find(c.mPendingMembers.begin(),
			c.mPendingMembers.end(), pld->CreatureDefID);
	if (pendingIt == c.mPendingMembers.end()) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No invite found!");
	}

	c.mPendingMembers.erase(pendingIt);

	creatureInstance->charPtr->clan = clanID;
	creatureInstance->charPtr->pendingChanges++;

	Clans::ClanMember member;
	member.mID = pld->CreatureDefID;
	member.mRank = Clans::Rank::INITIATE;
	g_ClanManager.cs.Enter("ClanInviteAcceptHandler::handleQuery");
	c.mMembers.push_back(member);
	g_ClanManager.SaveClan(c);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 43);
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutByte(&sim->SendBuf[wpos], 11);
	wpos += PutStringUTF(&sim->SendBuf[wpos],
			creatureInstance->charPtr->cdef.css.display_name);
	wpos += PutInteger(&sim->SendBuf[wpos],
			creatureInstance->charPtr->cdef.css.level);
	wpos += PutInteger(&sim->SendBuf[wpos],
			creatureInstance->charPtr->cdef.css.profession);
	PutShort(&sim->SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.CreatureDefID == pld->CreatureDefID) {
				int wpos2 = 0;
				wpos2 += PutByte(&sim->Aux2[wpos2], 43);
				wpos2 += PutShort(&sim->Aux2[wpos2], 0);
				wpos2 += PutByte(&sim->Aux2[wpos2], 9);
				wpos2 += PutStringUTF(&sim->Aux2[wpos2], c.mName.c_str());
				PutShort(&sim->Aux2[1], wpos2 - 3);
				it->AttemptSend(sim->Aux2, wpos2);
			} else if (c.HasMember(it->pld.CreatureDefID)) {
				it->AttemptSend(sim->SendBuf, wpos);
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanInviteHandler
//

int ClanInviteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.invite
	 Invite a player.
	 Args: [none]
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	Clans::Clan c = g_ClanManager.mClans[clanID];
	Clans::ClanMember member = c.GetMember(pld->CreatureDefID);
	if (member.mRank < Clans::Rank::OFFICER) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must be at least an officer to invite a player!");
	}

	std::string invitee = query->args[0];

	CharacterData *cd = g_CharacterManager.GetCharacterByName(invitee.c_str());
	if (cd == NULL) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%s is not online.",
				invitee.c_str());
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux2);
	}

	SendClanInvite(c, member, cd->cdef.CreatureDefID);
	c.mPendingMembers.push_back(cd->cdef.CreatureDefID);
	g_ClanManager.mClans[c.mId] = c;

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanRankHandler
//

int ClanRankHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: clan.rank
	 Change a members rank.
	 Args: member name, rank name
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	std::string memberName = query->args[0];
	int newRank = Clans::Rank::GetIDByName(query->args[1].c_str());

	int memberCDefID = g_AccountManager.GetCDefFromCharacterName(
			memberName.c_str());
	if (memberCDefID == -1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such clan member.");
	}

	Clans::Clan c = g_ClanManager.mClans[clanID];

	Clans::ClanMember me = c.GetMember(pld->CreatureDefID);
	if (me.mID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in this clan.");
	Clans::ClanMember them = c.GetMember(memberCDefID);
	if (them.mID == 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Player is not in your clan.");
	}

	if (me.mID == them.mID) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot promote or demote yourself.");
	}

	if (me.mRank < Clans::Rank::OFFICER)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be at least an officer to promote or demote others.");

	if (me.mRank <= them.mRank)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be of greater rank to promote or demote someone.");

	if (newRank > me.mRank)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot promote someone above your own rank.");

	if (newRank < Clans::Rank::INITIATE)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot demote below Initiate.");

	if (newRank < Clans::Rank::LEADER && c.mMembers.size() < 2) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot demote, would leave clan without a leader. Disband instead if you want to leave.");
	}

	them.mRank = newRank;

	g_ClanManager.cs.Enter("SimulatorThread::handle_query_clan_info");
	c.UpdateMember(them);
	g_ClanManager.SaveClan(c);
	g_ClanManager.mClans[c.mId] = c;
	g_ClanManager.cs.Leave();

	BroadcastClanRankChange(memberName, c, them);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanListHandler
//

int ClanListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: clan.list
	 Retrieves the list of clan members for the player's clan.
	 Args: [none]
	 */

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);      //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);   //Query response index

	int clanID = creatureInstance->charPtr->clan;
	if (!g_Config.Clans || clanID == 0 || !g_ClanManager.HasClan(clanID)) {
		wpos += PutShort(&sim->SendBuf[wpos], 0);
	} else {
		Clans::Clan c = g_ClanManager.mClans[clanID];

		// Number of rows (number of clan members)
		wpos += PutShort(&sim->SendBuf[wpos], c.mMembers.size());

		/*	For each member, 5 elements per row:
		 [0] = Character Name
		 [1] = Level
		 [2] = Profession (ex: "1", "2", "3", "4")
		 [3] = Online Status (ex: "true" or "false")
		 [4] = Arbitrary Rank Title (ex: "Leader", "Officer")
		 */
		for (std::vector<Clans::ClanMember>::iterator it = c.mMembers.begin();
				it != c.mMembers.end(); ++it) {
			Clans::ClanMember m = *it;
			CharacterData *cd = g_CharacterManager.RequestCharacter(m.mID,
					true);
			wpos += PutByte(&sim->SendBuf[wpos], 5);
			if (cd == NULL) {
				wpos += PutStringUTF(&sim->SendBuf[wpos], "Missing!");
				wpos += PutStringUTF(&sim->SendBuf[wpos], "1");
				wpos += PutStringUTF(&sim->SendBuf[wpos], "1");
				wpos += PutStringUTF(&sim->SendBuf[wpos], "false");
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						Clans::Rank::GetNameByID(Clans::Rank::INITIATE));
			} else {
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						cd->cdef.css.display_name);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d",
						cd->cdef.css.level);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
				Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%d",
						cd->cdef.css.profession);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						cd->expireTime == 0 ? "true" : "false");
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						Clans::Rank::GetNameByID(m.mRank));
			}
		}
	}

	PutShort(&sim->SendBuf[1], wpos - 3);             //Set message size
	return wpos;
}

//
// ClanRemoveHandler
//

int ClanRemoveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.remove
	 Remove a clan member.
	 Args: [member]
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	Clans::Clan c = g_ClanManager.mClans[clanID];
	Clans::ClanMember me = c.GetMember(pld->CreatureDefID);
	if (me.mRank < Clans::Rank::OFFICER) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must be at least an officer to remove a clan me");
	}

	// The target member
	std::string memberName = query->args[0];
	int memberCDefID = g_AccountManager.GetCDefFromCharacterName(
			memberName.c_str());
	if (memberCDefID == -1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such clan member.");
	}

	if (memberCDefID == pld->CreatureDefID) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot remove yourself from a clan.");

	}

	Clans::ClanMember them = c.GetMember(memberCDefID);
	if (them.mID == 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Player is not in your clan.");
	}

	c.RemoveMember(them);
	CharacterData *memberCd = g_CharacterManager.RequestCharacter(memberCDefID,
			true);
	if (memberCd != NULL) {
		memberCd->clan = 0;
		memberCd->pendingChanges++;
	}

	g_ClanManager.SaveClan(c);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 43);
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutByte(&sim->SendBuf[wpos], 12);
	wpos += PutStringUTF(&sim->SendBuf[wpos], memberName.c_str());
	PutShort(&sim->SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.CreatureDefID == memberCDefID) {
				int wpos2 = 0;
				wpos2 += PutByte(&sim->Aux2[wpos2], 43);
				wpos2 += PutShort(&sim->Aux2[wpos2], 0);
				wpos2 += PutByte(&sim->Aux2[wpos2], 13);
				PutShort(&sim->Aux2[1], wpos2 - 3);
				it->AttemptSend(sim->Aux2, wpos2);
			} else if (c.HasMember(it->pld.CreatureDefID)) {
				it->AttemptSend(sim->SendBuf, wpos);
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ClanLeaveHandler
//

int ClanLeaveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: clan.leave
	 Leave a clan.
	 */
	if (!g_Config.Clans)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Clans are not enabled.");

	int clanID = creatureInstance->charPtr->clan;
	if (clanID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a clan.");

	if (!g_ClanManager.HasClan(clanID))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your clan does not exist!");

	Clans::Clan c = g_ClanManager.mClans[clanID];
	Clans::ClanMember me = c.GetMember(pld->CreatureDefID);
	if (me.mID == 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Player is not in your clan.");
	}

	c.RemoveMember(me);

	g_ClanManager.SaveClan(c);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 43);
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutByte(&sim->SendBuf[wpos], 12);
	wpos += PutStringUTF(&sim->SendBuf[wpos],
			creatureInstance->charPtr->cdef.css.display_name);
	PutShort(&sim->SendBuf[1], wpos - 3);

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1) {
			if (it->pld.CreatureDefID == pld->CreatureDefID) {
				int wpos2 = 0;
				wpos2 += PutByte(&sim->Aux2[wpos2], 43);
				wpos2 += PutShort(&sim->Aux2[wpos2], 0);
				wpos2 += PutByte(&sim->Aux2[wpos2], 13);
				PutShort(&sim->Aux2[1], wpos2 - 3);
				it->AttemptSend(sim->Aux2, wpos2);
			} else if (c.HasMember(it->pld.CreatureDefID)) {
				it->AttemptSend(sim->SendBuf, wpos);
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

