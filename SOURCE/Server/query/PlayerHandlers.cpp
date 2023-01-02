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

#include "PlayerHandlers.h"
#include "../Creature.h"
#include "../Account.h"
#include "../Instance.h"
#include "../PartyManager.h"
#include "../ChatChannel.h"
#include "../Debug.h"
#include "../Config.h"
#include "../GameConfig.h"
#include "../util/Log.h"

//
//RestoreAppearanceHandler
//

int RestoreAppearanceHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//One string per row.
	MULTISTRING output;
	STRINGLIST row;
	row.push_back(pld->charPtr->cdef.css.appearance);
	output.push_back(row);
	row[0] = pld->charPtr->cdef.css.eq_appearance;
	output.push_back(row);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, output);
}

//
//AccountInfoHandler
//

int AccountInfoHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount < 1)
		return 0;

	AccountQuickData quickData =
			g_AccountManager.GetAccountQuickDataByUsername(query->args[0]);
	AccountData * data = NULL;
	if (quickData.mID == 0) {
		g_CharacterManager.GetThread(
				"SimulatorThread::handle_query_account_info");
		CharacterData *friendPtr = g_CharacterManager.GetCharacterByName(
				query->args[0].c_str());
		if (friendPtr != NULL)
			data = g_AccountManager.GetActiveAccountByID(friendPtr->AccountID);
		g_CharacterManager.ReleaseThread();
	} else {
		data = g_AccountManager.GetActiveAccountByID(quickData.mID);
	}
	if (data == NULL) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Could not find account for '%s'", query->args[0].c_str());
		sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
	} else {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Username: %s (%d) with %d characters  Grove: %s", data->Name.c_str(),
				data->ID, data->GetCharacterCount(), data->GroveName.c_str());
		sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
		int b;
		for (b = 0; b < AccountData::MAX_CHARACTER_SLOTS; b++) {
			if (data->CharacterSet[b] != 0) {
				int cdefid = data->CharacterSet[b];
				g_CharacterManager.GetThread(
						"SimulatorThread::handle_query_account_info");
				CharacterData *friendPtr = g_CharacterManager.GetPointerByID(
						cdefid);
				if (friendPtr != NULL) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
							"    %s (%d) Lvl: %d Copper: %d",
							friendPtr->cdef.css.display_name,
							friendPtr->cdef.CreatureDefID,
							friendPtr->cdef.css.level,
							friendPtr->cdef.css.copper);
					sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
				}
				g_CharacterManager.ReleaseThread();
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//AccountFulfillHandler
//

int AccountFulfillHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/* Query: account.fulfill
	 Args : none
	 Notes: 0.8.9 seems to return zero
	 */
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "0");
}

//
//EmoteHandler
//

int EmoteHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//Requires a modded client to perform this action.
	if (query->argCount < 3)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

	int target = query->GetInteger(0);
	const char *emoteName = query->GetString(1);
	float emoteSpeed = query->GetFloat(2);
	int loop = query->GetInteger(3);
	if (emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	target = sim->ResolveEmoteTarget(target);

	int wpos = PrepExt_SendAdvancedEmote(sim->SendBuf, target, emoteName,
			emoteSpeed, loop);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//EmoteControlHandler
//

int EmoteControlHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//Requires a modded client to perform this action.
	// [0] = target (0 = avatar, 1 = pet, else = explicit creature ID of pet);
	// [1] = event (1 = stop)
	if (query->argCount < 2)
		PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

	int target = query->GetInteger(0);
	int emoteEvent = query->GetInteger(1);

	target = sim->ResolveEmoteTarget(target);

	int wpos = PrepExt_SendEmoteControl(sim->SendBuf, target, emoteEvent);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ResCostHandler
//

int ResCostHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: persona.resCost
	 Sent when the player dies, to determine the cost of each resurrection option.
	 Args: [none]
	 */
	//For the three resurrect options [0,1,2], the client only asks for [1,2]
	QueryResponse resp(query->ID);
	auto row1 = resp.Row();
	row1->push_back(to_string(Global::GetResurrectCost(creatureInstance->css.level, 1)));
	auto row2 = resp.Row();
	row2->push_back(to_string(Global::GetResurrectCost(creatureInstance->css.level, 2)));
	return resp.Write(sim->SendBuf);
}

//
//GuildInfoHandler
//

int GuildInfoHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: guild.info
	 Retrieves the guild info of the Simulator player.
	 Args: [none]
	 */
	QueryResponse resp(query->ID);
	if (pld->charPtr->guildList.size() > 0) {
		for (unsigned int i = 0; i < pld->charPtr->guildList.size(); i++) {

			auto row = resp.Row();

			GuildDefinition *gdef = g_GuildManager.GetGuildDefinition(
					pld->charPtr->guildList[i].GuildDefID);

			row->push_back(to_string(pld->charPtr->guildList[i].GuildDefID));
			row->push_back(to_string(pld->charPtr->guildList[i].Valour));
			row->push_back(to_string(gdef->guildType));
			row->push_back(gdef->defName);
			row->push_back(gdef->motto);

			GuildRankObject *rank = g_GuildManager.GetRank(pld->CreatureDefID,
					gdef->guildDefinitionID);
			if (rank == NULL) {
				row->push_back("No rank");
				row->push_back("0");
			} else {
				row->push_back(rank->title);
				row->push_back(to_string(rank->rank));
			}
		}
	}

	return resp.Write(sim->SendBuf);
}

//
//GuildLeaveHandler
//

int GuildLeaveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int guildDefID = atoi(query->args[0].c_str());

	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Guild ID required");
	QuestDefinitionContainer::ITERATOR it;
	std::vector<int> defs;

	// Make sure any guild quests are abandoned
	for (auto it : QuestDef.mQuests) {
		auto qd = it.second;
		if (qd.guildId == guildDefID) {
			int QID = qd.questID;
			if (pld->charPtr->questJournal.activeQuests.HasQuestID(QID)) {
				if (qd.unabandon == true) {
					sim->SendInfoMessage("You cannot abandon that quest.",
							INFOMSG_ERROR);
					QID = 0; //Set to zero so it's not actually removed in the server or the client.
				}
				pld->charPtr->questJournal.QuestLeave(pld->CreatureID, QID);
			}
		}
	}

	pld->charPtr->LeaveGuild(guildDefID);
	sim->BroadcastGuildChange(guildDefID);
	pld->charPtr->pendingChanges++;
	pld->charPtr->cdef.css.SetSubName(NULL);
	creatureInstance->SendStatUpdate(STAT::SUB_NAME);

	QueryResponse resp(query->ID);
	for(auto def : defs) {
		auto row = resp.Row();
		row->push_back(to_string(def));
	}
	return resp.Write(sim->SendBuf);
}

//
//ValidateNameHandler
//

int ValidateNameHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int res = g_AccountManager.ValidateNameParts(query->args[0].c_str(),
			query->args[1].c_str());
	if (res == AccountManager::CHARACTER_SUCCESS) {
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	} else {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Renamed to %s %s returned error %d", query->args[0].c_str(),
				query->args[1].c_str(), res);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}
}

//
//VisWeaponHandler
//

int VisWeaponHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: visWeapon
	 Sent to notify the server of a weapon visibility change, such as swapping
	 melee and ranged.
	 Note: despite the nonstandard query name, this is an actual query and not
	 a user command.
	 Args : [0] = New visibility status (0, 1, or 2).
	 */
	if (query->argCount > 0) {
		int visState = query->GetInteger(0);
		creatureInstance->css.vis_weapon = visState;
		int wpos = PrepExt_SendVisWeapon(sim->SendBuf,
				creatureInstance->CreatureID, visState);
		creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
				creatureInstance->CurrentX, creatureInstance->CurrentZ);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PartyHandler
//

int PartyHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: party
	 Command to create a virtual party with the selected target.
	 Args: [none]
	 */
	if (query->argCount < 1)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	const char *command = query->args[0].c_str();

	if (strcmp(command, "invite") == 0) {
		if (query->argCount >= 2) {
			if (creatureInstance->PartyID != 0) {
				if (g_PartyManager.GetPartyByLeader(
						creatureInstance->CreatureDefID) == NULL)
					return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
							"Only party leaders may send invites.");
			}

			CreatureInstance *target = NULL;
			int playerID = query->GetInteger(1);
			target = creatureInstance->actInst->GetPlayerByID(playerID);

			//Hack to look up by name since we're re-using this query as a direct command
			//to party by string.
			bool byName = false;
			if (target == NULL) {
				target = g_ActiveInstanceManager.GetPlayerCreatureByName(
						query->GetString(1));
				if (target != NULL) {
					byName = true;
					if (target->actInst->mZoneDefPtr->IsDungeon() == true)
						return PrepExt_QueryResponseError(sim->SendBuf,
								query->ID,
								"You may not invite players if they are already inside a dungeon.");
				}
			}

			if (byName == true) {
				if (creatureInstance->actInst->mZoneDefPtr->IsDungeon()
						== false)
					return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
							"You must be inside a dungeon to invite players by command.");
			}

			if (target != NULL) {
				if (target->PartyID == 0) {
					int wpos = PartyManager::WriteInvite(sim->SendBuf,
							creatureInstance->CreatureDefID,
							creatureInstance->css.display_name);
					creatureInstance->actInst->LSendToOneSimulator(sim->SendBuf,
							wpos, target->simulatorPtr);
				} else {
					return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
							"That player is already in a party.");
				}
			}
		}
	} else if (strcmp(command, "accept.invite") == 0) {
		if (query->argCount >= 2) {
			int leaderDefID = query->GetInteger(1);
			CreatureInstance *leader =
					g_ActiveInstanceManager.GetPlayerCreatureByDefID(
							leaderDefID);
			if (leader != NULL) {
				g_PartyManager.AcceptInvite(creatureInstance, leader);
				//creatureInstance->PartyID = g_PartyManager.AcceptInvite(leader->CreatureDefID, pld->CreatureDefID, pld->CreatureID, creatureInstance->css.display_name);
				int wpos = g_PartyManager.PrepMemberList(sim->SendBuf,
						creatureInstance->PartyID,
						creatureInstance->CreatureID);
				if (wpos > 0)
					sim->AttemptSend(sim->SendBuf, wpos);
				g_PartyManager.BroadcastAddMember(creatureInstance);
			}
		}
	} else if (strcmp(command, "reject.invite") == 0) {
		if (query->argCount >= 2) {
			int leaderDefID = atoi(query->args[1].c_str());
			//creatureInstanceance *leader = creatureInstance->actInst->GetPlayerByCDefID(leaderDefID);
			CreatureInstance *leader =
					g_ActiveInstanceManager.GetPlayerCreatureByDefID(
							leaderDefID);
			if (leader != NULL) {
				int wpos = PartyManager::WriteRejectInvite(sim->SendBuf,
						creatureInstance->css.display_name);
				leader->simulatorPtr->AttemptSend(sim->SendBuf, wpos);
			}
		}
	} else if (strcmp(command, "setLeader") == 0) {
		if (query->argCount >= 2) {
			int targetID = atoi(query->args[1].c_str());
			g_PartyManager.DoSetLeader(creatureInstance, targetID);
		}
	} else if (strcmp(command, "kick") == 0) {
		if (query->argCount >= 2) {
			int targetID = atoi(query->args[1].c_str());
			g_PartyManager.DoKick(creatureInstance, targetID);
		}
	} else if (strcmp(command, "quest.invite") == 0) {
		if (query->argCount >= 2) {
			int questID = atoi(query->args[1].c_str());
			QuestDefinition *qdef = QuestDef.GetQuestDefPtrByID(questID);
			ActiveParty *party = g_PartyManager.GetPartyByID(
					creatureInstance->PartyID);
			if (qdef != NULL && party != NULL) {
				int scount = 0;
				int fcount = 0;
				int wpos = PartyManager::WriteQuestInvite(sim->SendBuf,
						qdef->title.c_str(), qdef->questID);
				SIMULATOR_IT it;
				for (it = Simulator.begin(); it != Simulator.end(); ++it) {
					if (it->InternalID == sim->InternalID)
						continue;
					if (it->LoadStage != SimulatorThread::LOADSTAGE_GAMEPLAY)
						continue;
					if (it->creatureInst->PartyID != party->mPartyID)
						continue;
					if (party->HasMember(it->creatureInst->CreatureDefID)
							== false)
						continue;
					if (it->pld.charPtr->questJournal.CheckQuestShare(questID)
							== QuestJournal::SHARE_SUCCESS_QUALIFIES) {
						it->AttemptSend(sim->SendBuf, wpos);
						scount++;
					} else
						fcount++;
				}
				if (scount > 0) {
					sprintf(sim->Aux1, "Sent quest invite to %d %s.", scount,
							((scount == 1) ? "player" : "players"));
					sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				}
				if (fcount > 0) {
					sprintf(sim->Aux1, "%d %s cannot take that quest.", fcount,
							((fcount == 1) ? "player" : "players"));
					sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				}
			} else if (party == NULL)
				sim->SendInfoMessage("You must be in a party to share quests.",
						INFOMSG_INFO);
		}
	} else if (strcmp(command, "loot.mode") == 0) {
		ActiveParty *party = g_PartyManager.GetPartyByID(
				creatureInstance->PartyID);
		if (!g_GameConfig.UsePartyLoot)
			sim->SendInfoMessage("Party loot modes are currently disabled.",
					INFOMSG_ERROR);
		else if (party == NULL)
			sim->SendInfoMessage("You must be in a party to set loot mode.",
					INFOMSG_INFO);
		else {

			if (query->argCount >= 2) {
				int mode = atoi(query->args[1].c_str());
				party->mLootMode = static_cast<LootMode>(mode);
				switch (party->mLootMode) {
				case LOOT_MASTER:
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Loot master. The party leader gets all the loot.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
					break;
				case ROUND_ROBIN:
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Round robin. Loot is offered to each player in  turn.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
					break;
				case FREE_FOR_ALL:
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Free for all. Whoever picks up the loot first wins.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
					break;
				}
				int wpos = PartyManager::StrategyChange(sim->SendBuf,
						party->mLootMode);
				party->BroadCast(sim->SendBuf, wpos);
			}
		}
	} else if (strcmp(command, "loot.flags") == 0) {
		ActiveParty *party = g_PartyManager.GetPartyByID(
				creatureInstance->PartyID);
		if (!g_GameConfig.UsePartyLoot)
			sim->SendInfoMessage("Party loot modes are currently disabled.",
					INFOMSG_ERROR);
		else if (party == NULL)
			sim->SendInfoMessage("You must be in a party to set loot mode.",
					INFOMSG_INFO);
		else {
			if (query->argCount >= 2) {
				int wasFlags = party->mLootFlags;
				int flags = atoi(query->args[1].c_str());
				if (strcmp(query->args[2].c_str(), "true") == 0) {
					party->mLootFlags |= static_cast<LootFlags>(flags);
				} else {
					party->mLootFlags &= ~(static_cast<LootFlags>(flags));
				}
				if ((party->mLootFlags & MUNDANE) > 0
						&& (wasFlags & MUNDANE) == 0) {
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Mundane items included in looting rules.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
				} else if ((party->mLootFlags & MUNDANE) == 0
						&& (wasFlags & MUNDANE) > 0) {
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Mundane items no longer included in looting rules.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
				}
				if ((party->mLootFlags & NEED_B4_GREED) > 0
						&& (wasFlags & NEED_B4_GREED) == 0) {
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Need before greed now active.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
				} else if ((party->mLootFlags & NEED_B4_GREED) == 0
						&& (wasFlags & NEED_B4_GREED) > 0) {
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Need before greed no longer active.");
					party->BroadcastInfoMessageToAllMembers(sim->Aux1);
				}
				int wpos = PartyManager::StrategyFlagsChange(sim->SendBuf,
						party->mLootFlags);
				party->BroadCast(sim->SendBuf, wpos);
			}
		}
	} else if (strcmp(command, "quit") == 0) {
		g_PartyManager.DoQuit(creatureInstance);
		int wpos = PartyManager::WriteLeftParty(sim->SendBuf);
		sim->AttemptSend(sim->SendBuf, wpos);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PartyIsMemberHandler
//

int PartyIsMemberHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: party
	 Command to query if a creature is in a party.
	 Args: [none]
	 */
	if (query->argCount < 2)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	int cdefId = query->GetInteger(0);
	int cid = query->GetInteger(1);

	ActiveParty* party = g_PartyManager.GetPartyByLeader(cdefId);
	if (party != NULL) {
		if (party->GetMemberByID(cid) != NULL)
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "true");
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "false");
}

//
//PrivateChannelJoinHandler
//

int PrivateChannelJoinHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	const char *name = NULL;
	const char *password = NULL;
	if (query->argCount >= 1)
		name = query->GetString(0);
	if (query->argCount >= 2)
		password = query->GetString(1);

	sim->JoinPrivateChannel(name, password);

	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}

//
//PrivateChannelLeaveHandler
//

int PrivateChannelLeaveHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	const char *name = NULL;
	if (query->argCount >= 1)
		name = query->GetString(0);

	int res = g_ChatChannelManager.LeaveChannel(sim->InternalID, name);
	if (res == ChatChannelManager::RESULT_NOT_IN_CHANNEL) {
		sim->SendInfoMessage("You are not in that channel.", INFOMSG_ERROR);
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
	}

	pld->charPtr->SetLastChannel(NULL, NULL);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PlayerAchievementsHandler
//

int PlayerAchievementsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount > 0) {

		CreatureInstance *cdata = creatureInstance->actInst->GetPlayerByID(
				query->GetInteger(0));
		if (cdata == NULL) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"No such character %s", query->GetString(0));
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					sim->Aux1);
		}

		AccountData *acc = g_AccountManager.GetActiveAccountByID(
				cdata->charPtr->AccountID);
		if (acc == NULL) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Account is not active.");
		}

		STRINGLIST l;

		// First row is the player creature ID
		l.push_back(query->GetString(0));

		// Second row is the player score
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%d:%d:%d:%d",
				g_AchievementsManager.GetTotalAchievements(),
				acc->GetTotalCompletedAchievements(),
				g_AchievementsManager.GetTotalObjectives(),
				acc->GetTotalAchievementObjectives());
		l.push_back(sim->Aux1);

		for (std::map<std::string, Achievements::Achievement>::iterator it =
				pld->accPtr->Achievements.begin();
				it != pld->accPtr->Achievements.end(); ++it) {
			Achievements::Achievement a = it->second;
			for (std::vector<Achievements::AchievementObjectiveDef*>::iterator ait =
					a.mCompletedObjectives.begin();
					ait != a.mCompletedObjectives.end(); ++ait) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%s/%s",
						a.mDef->mName.c_str(), (*ait)->mName.c_str());
				l.push_back(sim->Aux1);
			}
		}

		return PrepExt_QueryResponseStringList(sim->SendBuf, query->ID, l);
	} else {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Missing argument.");
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//AchievementDefHandler
//

int AchievementDefHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	QueryResponse resp(query->ID, sim->SendBuf);
	if (query->argCount > 0) {
		auto adata = g_AchievementsManager.GetItem(query->GetString(0));
		if (adata == NULL)
			return resp.Error("No such achievement");

		auto firstRow = resp.Row();
		firstRow->push_back(adata->mName);  //name
		firstRow->push_back(adata->mTitle); //title
		firstRow->push_back(to_string(adata->mCategory));  //category
		firstRow->push_back(adata->mDescription); //description
		firstRow->push_back(adata->mIcon1); //icon1
		firstRow->push_back(adata->mIcon2); //icon2
		firstRow->push_back(adata->mTag);   //tag

		for (auto odef : adata->mObjectives) {
			auto row = resp.Row();
			row->push_back(odef->mName); //name
			row->push_back(odef->mTitle); //title
			row->push_back(odef->mDescription); //description
			row->push_back(odef->mIcon1); //icon1
			row->push_back(odef->mIcon2); //icon2
			row->push_back(odef->mTag); //tag
		}

		return resp.Data();

	} else {
		return resp.Error("Missing argument.");
	}
	return resp.String("OK");
}
