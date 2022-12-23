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

#include "CommandHandlers.h"
#include "../util/Log.h"
#include "../Instance.h"
#include "../ChatChannel.h"
#include "../Crafting.h"
#include "../Instance.h"
#include "../Character.h"
#include "../Config.h"
#include "../Account.h"
#include "../Random.h"
#include "../InstanceScale.h"
#include "../ConfigString.h"
#include "../Scenery2.h"
#include "../Cluster.h"
#include "../IGForum.h"
#include "../Ability2.h"
#include "../Fun.h"
#include "../InstanceScript.h"
#include "../AIScript.h"
#include "../AIScript2.h"
#include "../Cluster.h"
#include "../Scheduler.h"
#include "../GameConfig.h"
#include <boost/format.hpp>
#include <string.h>

//
//AbstractCommandHandler
//
AbstractCommandHandler::AbstractCommandHandler(string usage,
		int requiredArgs) {
	mUsage = usage;
	mRequiredArgs = requiredArgs;
}

bool AbstractCommandHandler::isAllowed(SimulatorThread *sim) {
	if (mAllowedPermissions.size() == 0)
		return true;

	bool ok = false;
	for (auto it = mAllowedPermissions.begin(); it != mAllowedPermissions.end();
			++it) {
		if (sim->CheckPermissionSimple(Perm_Account, *it)) {
			ok = true;
			break;
		}
	}
	return ok;
}

int AbstractCommandHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	bool ok = isAllowed(sim);
	if (!ok) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	}

	if (query->argCount < mRequiredArgs
			|| (query->argCount > 0
					&& strcmp(query->GetString(0), "--help") == 0)) {
		sim->SendInfoMessage(mUsage.c_str(), INFOMSG_ERROR);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}

	if (query->argCount == 0) {
		g_Logs.event->info("[COMMAND] %v ran %v",
				creatureInstance->charPtr->cdef.css.display_name, query->name);
	} else if (query->argCount == 1) {
		g_Logs.event->info("[COMMAND] %v ran %v %v",
				creatureInstance->charPtr->cdef.css.display_name, query->name,
				query->GetString(0));
	} else if (query->argCount == 2) {
		g_Logs.event->info("[COMMAND] %v ran %v %v %v",
				creatureInstance->charPtr->cdef.css.display_name, query->name,
				query->GetString(0), query->GetString(1));
	} else if (query->argCount == 3) {
		g_Logs.event->info("[COMMAND] %v ran %v %v %v %v",
				creatureInstance->charPtr->cdef.css.display_name, query->name,
				query->GetString(0), query->GetString(1), query->GetString(2));
	} else if (query->argCount > 3) {
		g_Logs.event->info(
				"[COMMAND] %v ran %v %v %v %v ... (total of %v args)",
				creatureInstance->charPtr->cdef.css.display_name, query->name,
				query->GetString(0), query->GetString(1), query->GetString(2),
				query->argCount);
	}

	return handleCommand(sim, pld, query, creatureInstance);
}

//
//AdjustExpHandler
//
AdjustExpHandler::AdjustExpHandler() :
		AbstractCommandHandler(
				"Usage: /adjustexp <amount> - Where <amount> is a POSITIVE number",
				1) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int AdjustExpHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	int amount = atoi(query->args[0].c_str());
	creatureInstance->AddExperience(amount);
	if (amount < 0) {
		for (int i = 0; i < MAX_LEVEL; i++) {
			if (creatureInstance->css.experience >= LevelRequirements[i][1]) {
				creatureInstance->css.level = i;
				break;
			}
		}
	}
	creatureInstance->Submit(bind(&CreatureInstance::BroadcastCreatureInstanceUpdate, creatureInstance));

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PoseHandler
//
PoseHandler::PoseHandler() :
		AbstractCommandHandler("Usage: /pose <emote> [<speed>] [<loop>]", 1) {
}

int PoseHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	const char *emoteName = query->GetString(0);
	float emoteSpeed = 1.0F;
	int loop = 0;

	if (query->argCount > 1)
		emoteSpeed = query->GetFloat(1);
	if (query->argCount > 2)
		loop = query->GetInteger(2);

	if (emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	int wpos = 0;
	if (query->argCount == 1)
		wpos = PrepExt_GenericChatMessage(sim->SendBuf, pld->CreatureID,
				pld->charPtr->cdef.css.display_name, "emote",
				query->args[0].c_str());
	else {
		wpos = PrepExt_SendAdvancedEmote(sim->SendBuf,
				creatureInstance->CreatureID, emoteName, emoteSpeed, loop);
		if (query->argCount >= 3 && loop == 0)
			wpos += PrepExt_SendEmoteControl(&sim->SendBuf[wpos],
					creatureInstance->CreatureID, 1);
	}

	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//Pose2Handler
//
Pose2Handler::Pose2Handler() :
		AbstractCommandHandler("Usage: /pose2 <emote> [<speed>] [<loop>]", 1) {
}

int Pose2Handler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	const char *emoteName = query->GetString(0);
	float emoteSpeed = 1.0F;
	int loop = 0;

	if (query->argCount > 1)
		emoteSpeed = query->GetFloat(1);
	if (query->argCount > 2)
		loop = query->GetInteger(2);

	if (emoteSpeed < 0.1F)
		emoteSpeed = 0.1F;

	int targetID = sim->ResolveEmoteTarget(1);

	int wpos = 0;
	if (query->argCount == 1)
		wpos = PrepExt_GenericChatMessage(sim->SendBuf, targetID,
				pld->charPtr->cdef.css.display_name, "emote",
				query->args[0].c_str());
	else {
		wpos = PrepExt_SendAdvancedEmote(sim->SendBuf, targetID, emoteName,
				emoteSpeed, loop);
		if (query->argCount >= 3 && loop == 0)
			wpos += PrepExt_SendEmoteControl(&sim->SendBuf[wpos], targetID, 1);
	}

	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//EsayHandler
//
EsayHandler::EsayHandler() :
		AbstractCommandHandler("Usage: /esay <emote> [<text>]", 1) {
}

int EsayHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	const char *emoteName = query->GetString(0);
	const char *sayText = NULL;
	if (query->argCount > 1)
		sayText = query->GetString(1);

	int wpos = 0;
	wpos = PrepExt_GenericChatMessage(sim->SendBuf, pld->CreatureID,
			pld->charPtr->cdef.css.display_name, "emote", emoteName);
	if (sayText != NULL)
		wpos += PrepExt_GenericChatMessage(&sim->SendBuf[wpos], pld->CreatureID,
				pld->charPtr->cdef.css.display_name, "s", sayText);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//HealthHandler
//
HealthHandler::HealthHandler() :
		AbstractCommandHandler("Usage: /health <hitPoints>", 1) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int HealthHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: health
	 Cheat to change your health on demand.
	 Args : [0] = Hit Points to assign.
	 */

	int amount = atoi(query->args[0].c_str());
	int max = creatureInstance->GetMaxHealth(true);
	Util::ClipInt(amount, 1, max);
	amount = amount & 0xFFFF;
	creatureInstance->css.health = amount;
	creatureInstance->Submit([this, pld, amount, creatureInstance](){
		char buf[32];
		creatureInstance->actInst->LSendToLocalSimulator(buf, PrepExt_SendHealth(buf, pld->CreatureID,
				amount), creatureInstance->CurrentX, creatureInstance->CurrentZ);
	});
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//SpeedHandler
//
SpeedHandler::SpeedHandler() :
		AbstractCommandHandler("Usage: /speed <bonusAmount>", 1) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int SpeedHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: health
	 Cheat to change your speed on demand.
	 Args : [0] = Base Speed.
	 */

	int amount = atoi(query->args[0].c_str());
	amount = Util::ClipInt(amount, 0, 0xFFFF);

	creatureInstance->css.mod_movement = amount;
	creatureInstance->Submit(bind(&CreatureInstance::BroadcastCreatureInstanceUpdate, creatureInstance));


	sim->SendInfoMessage("Speed set.", INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ForceAbilityHandler
//
ForceAbilityHandler::ForceAbilityHandler() :
		AbstractCommandHandler("Usage: /fa <ability>", 1) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int ForceAbilityHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: fa
	 Cheat to force activation of an ability.  Bypasses client-enforced cooldowns.
	 Args : [0] = ID of the ability to use.
	 */

	int abID = 0;
	int ground = 0;
	abID = atoi(query->args[0].c_str());

	if (abID >= 26000 && abID <= 26100) {
		if (!sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer)) {
			sim->SendInfoMessage("Thou art not a deity.", INFOMSG_ERROR);
			abID = 0;
		}
	}

	if (abID > 0) {
		if (query->argCount == 2) {
			ground = atoi(query->args[1].c_str());
			if (ground > 0) {
				CreatureInstance *ptr = creatureInstance->CurrentTarget.targ;
				if (ptr == NULL)
					ptr = creatureInstance;
				creatureInstance->ab[0].SetPosition(ptr->CurrentX,
						ptr->CurrentY, ptr->CurrentZ);
			}
		} else if (query->argCount >= 3) {
			int xoffset = atoi(query->args[1].c_str());
			int zoffset = atoi(query->args[2].c_str());

			creatureInstance->ab[0].SetPosition(
					creatureInstance->CurrentX + xoffset,
					creatureInstance->CurrentY,
					creatureInstance->CurrentZ + zoffset);
		}
		creatureInstance->RequestAbilityActivation(abID);
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PartyLowestHandler
//
PartyLowestHandler::PartyLowestHandler() :
		AbstractCommandHandler("Usage: /partylowest", 0) {
}

int PartyLowestHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	float lowestHealthRatio = 1.0F;
	CreatureInstance *oldTarget = creatureInstance->CurrentTarget.targ;
	CreatureInstance *newTarget = oldTarget;
	ActiveParty *party = g_PartyManager.GetPartyByID(creatureInstance->PartyID);
	if (party != NULL) {
		for (size_t i = 0; i < party->mMemberList.size(); i++) {
			CreatureInstance *partyMember =
					creatureInstance->actInst->GetPlayerByCDefID(
							party->mMemberList[i].mCreatureDefID);
			if (partyMember == NULL)
				continue;
			float healthRatio = partyMember->GetHealthRatio();
			if (healthRatio < lowestHealthRatio) {
				lowestHealthRatio = healthRatio;
				newTarget = partyMember;
			}
		}
		if (newTarget != oldTarget) {
			creatureInstance->SelectTarget(newTarget);
			int wpos = PrepExt_ChangeTarget(sim->SendBuf, pld->CreatureID,
					newTarget->CreatureID);
			sim->AttemptSend(sim->SendBuf, wpos);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//WhoHandler
//
WhoHandler::WhoHandler() :
		AbstractCommandHandler("Usage: /who", 0) {
}

int WhoHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: who
	 Notify the client of a list of players currently logged in.
	 Args : none
	 */

	int WritePos = 0;

	bool debug = sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer);
	SYNCHRONIZED(g_ClusterManager.mMutex) {
		for (map<int, ShardPlayer>::iterator it =
				g_ClusterManager.mActivePlayers.begin();
				it != g_ClusterManager.mActivePlayers.end(); ++it) {
			ShardPlayer sp = it->second;
			CharacterData *cd = sp.mCharacterData;
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(sp.mZoneID);
			if (cd != NULL && zd != NULL) {
				if (debug == true
						&& sp.mShard.compare(g_ClusterManager.mShardName) == 0) {
					SimulatorThread *psim = g_SimulatorManager.GetPtrByID(
							sp.mSimID);
					if (psim == NULL) {
						g_Logs.server->warn(
								"Unknown simulator ID found in player list. %v for %v (on %v)",
								sp.mSimID, sp.mID, sp.mShard);
						continue;
					}
					else
						Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
								"%s (%s.%d) %s (%s) [%d, %d]",
								cd->cdef.css.display_name,
								Professions::GetAbbreviation(
										cd->cdef.css.profession),
								cd->cdef.css.level, zd->mName.c_str(),
								cd->Shard.c_str(), psim->creatureInst->CurrentX,
								psim->creatureInst->CurrentZ);
				} else
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"%s (%s.%d) %s (%s)", cd->cdef.css.display_name,
							Professions::GetAbbreviation(cd->cdef.css.profession),
							cd->cdef.css.level, zd->mName.c_str(),
							cd->Shard.c_str());

				WritePos += PrepExt_SendInfoMessage(&sim->SendBuf[WritePos],
						sim->Aux1, INFOMSG_INFO);
				sim->CheckWriteFlush(WritePos);
			}
		}
	}

//	SIMULATOR_IT it;
//	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
//		if (it->isConnected == true && it->ProtocolState == 1
//				&& it->LoadStage == sim->LOADSTAGE_GAMEPLAY) {
//			if (it->IsGMInvisible() == true) //Hide GM Invisibility from the list.
//				continue;
//			CharacterData *cd = it->pld.charPtr;
//			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(
//					it->pld.CurrentZoneID);
//			if (cd != NULL && zd != NULL) {
//				if (debug == true)
//					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
//							"%s (%d, %d) %s (%s)", cd->cdef.css.display_name,
//							it->creatureInst->CurrentX,
//							it->creatureInst->CurrentZ, zd->mName.c_str(),
//							cd->Shard.c_str());
//				else
//					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
//							"%s (%s.%d) %s (%s)", cd->cdef.css.display_name,
//							Professions::GetAbbreviation(
//									cd->cdef.css.profession),
//							cd->cdef.css.level, zd->mName.c_str(),
//							cd->Shard.c_str());
//
//				WritePos += PrepExt_SendInfoMessage(&sim->SendBuf[WritePos],
//						sim->Aux1, INFOMSG_INFO);
//				sim->CheckWriteFlush(WritePos);
//			}
//		}
//	}
	WritePos += PrepExt_QueryResponseString(&sim->SendBuf[WritePos], query->ID,
			"OK");
	return WritePos;
}
//
//GMWhoHandler
//
GMWhoHandler::GMWhoHandler() :
		AbstractCommandHandler("Usage: /gmwho", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int GMWhoHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
// Advanced /who command for administrators shows account names and ID.  Useful for identifying alts
// or account info for other admin commands.

	int wpos = 0;
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1
				&& it->LoadStage == sim->LOADSTAGE_GAMEPLAY) {
			CharacterData *cd = it->pld.charPtr;
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%s (%s, AID:%d)",
					cd->cdef.css.display_name, it->pld.accPtr->Name.c_str(),
					it->pld.accPtr->ID);
			wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], sim->Aux1,
					INFOMSG_INFO);
		}
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}
//
//CHWhoHandler
//
CHWhoHandler::CHWhoHandler() :
		AbstractCommandHandler("Usage: /chwho [<channelName>]", 0) {
}

int CHWhoHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//Inform the client with a list of player names that this player has joined a private chat channel with.
	//Return a search of all channels, or if the optional channel name argument is supplied, that specific
	//channel.
	// Arguments: [0] = channel name (optional)
	const char *channelName = NULL;
	if (query->argCount > 1)
		channelName = query->GetString(1);

	//Retrieves a list of strings as raw output, including channel name.
	//The strings will be broadcast as a series of client info messages.
	STRINGLIST results;
	g_ChatChannelManager.EnumPlayersInMemberChannels(sim->InternalID,
			channelName, results);
	int wpos = 0;
	for (size_t i = 0; i < results.size(); i++) {
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], results[i].c_str(),
				INFOMSG_INFO);
		sim->CheckWriteFlush(wpos);
	}
	if (results.size() == 0) {
		const char *errorMsg = "You are not in a channel.";
		if (channelName != NULL) {
			Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
					"You are not in channel [%s].", channelName);
			errorMsg = sim->Aux3;
		}
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], errorMsg,
				INFOMSG_ERROR);
	}
	if (wpos > 0)
		sim->AttemptSend(sim->SendBuf, wpos);

	wpos = 0; //Reset to zero for correct query response.
	return PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
}

//
//ShardsHandler
//
ListShardsHandler::ListShardsHandler() :
		AbstractCommandHandler("Usage: /listshards", 0) {
}

int ListShardsHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: who
	 Notify the client of a list of players currently logged in.
	 Args : none
	 */
	int WritePos = 0;
	STRINGLIST s = g_ClusterManager.GetAvailableShardNames();
	for (STRINGLIST::iterator it = s.begin(); it != s.end(); ++it) {
		Shard s = g_ClusterManager.GetActiveShard(*it);
		time_t t = s.GetServerTime() / 1000;
		time_t t2 = s.mStartTime / 1000;
		WritePos += PrepExt_SendInfoMessage(&sim->SendBuf[WritePos],
				Util::Format("%s%-15s %-15s %3d [%s] %5dms - Now: [%s] Start: [%s]",
						s.IsMaster() ? "*" : "", s.mName.c_str(),
						s.mSimulatorAddress.c_str(), s.mPlayers,
						s.mFullName.c_str(), s.mPing,
						Util::FormatDateTime(&t).c_str(),
						Util::FormatDateTime(&t2).c_str()).c_str(),
				INFOMSG_ERROR);
	}
	WritePos += PrepExt_QueryResponseString(&sim->SendBuf[WritePos], query->ID,
			"OK");
	return WritePos;
}

//
//GiveHandler
//
GiveHandler::GiveHandler() :
		AbstractCommandHandler("Usage: /give \"<itemName>\"", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer | Permission_Sage) ;
}

int GiveHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: give
	 Cheat to give the player an item by name.
	 Args : [0] = name of object to search for.
	 */

	int wpos = 0;
	const char *itemName = query->GetString(0);

	ItemDef *item = g_ItemManager.GetSafePointerByPartialName(itemName);
	if (item->mID == 0) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Item name not found [%s]", itemName);
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], sim->Aux1,
				INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");
		return wpos;
	}

	int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1) {
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
				"No free backpack slots", INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");
		return wpos;
	}

	InventorySlot *sendSlot = pld->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
			item->mID, 1);
	if (sendSlot != NULL) {
		sim->ActivateActionAbilities(sendSlot);
		wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux1, sendSlot);
		pld->charPtr->pendingChanges++;
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//GiveHandler
//
GiveIDHandler::GiveIDHandler() :
		AbstractCommandHandler("Usage: /giveid <itemId> [<count>]", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer) ;
}

int GiveIDHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: vgive
	 Cheat to give the player an item by ID.  Works for virtual items.
	 NOTE: virtual items should not be duplicated.  If one is destroyed by any means, deleted
	 from inventory, pushed off byback, etc, it will be removed from the database and the duplicates
	 will be invalidated.
	 Args : [0] = ID of object to give.
	 */
	int itemID = query->GetInteger(0);
	int count = 1;
	if (query->argCount > 1)
		count = query->GetInteger(1);

	ItemDef *item = g_ItemManager.GetPointerByID(itemID);
	int wpos = 0;
	if (item == NULL) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Item ID not found [%d]",
				itemID);
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], sim->Aux1,
				INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");
		return wpos;
	}

	int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1) {
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
				"No free backpack slots", INFOMSG_ERROR);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");
		return wpos;
	}

	InventorySlot *sendSlot = pld->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
			item->mID, count);
	if (sendSlot != NULL) {
		sim->ActivateActionAbilities(sendSlot);
		wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux1, sendSlot);
		pld->charPtr->pendingChanges++;
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}




//
//GamConfigHandler
//
GameConfigHandler::GameConfigHandler() :
		AbstractCommandHandler("Usage: /gameconfig", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int GameConfigHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: gameconfig
	 List, get or set cluster wide game configuration.
	 Args : [0] = optional name of configuration key to get.
	 Args : [1] = optional name of configuration key to get.
	 */
	if (query->argCount > 0) {
		auto key = query->GetStringObject(0);
		if(key.compare("reload") == 0) {
			g_ClusterManager.GameConfigChanged("","");
			g_GameConfig.Reload();
			sim->SendInfoMessage("Reloaded game configuration from database.", INFOMSG_INFO);
		}
		else {
			if(g_GameConfig.HasKey(key)) {
				if(query->argCount > 1) {
					g_GameConfig.Set(key, query->GetStringObject(1));
					sim->SendInfoMessage("Configuration changed.", INFOMSG_INFO);
				}
				else {
					sim->SendInfoMessage(str(boost::format("%s=%s")
												% key
												% g_GameConfig.Get(query->GetStringObject(0))).c_str(), INFOMSG_INFO);
				}
			}
			else {
				sim->SendInfoMessage("No such game configuration key. Type /gameconfig on it's own to list keys.", INFOMSG_ERROR);
			}
		}
	}
	else {
		auto cfg = g_GameConfig.GetAll();
		for (auto it =cfg.begin(); it != cfg.end(); ++it) {
			sim->SendInfoMessage(str(boost::format("%s=%s")
							% it->first
							% it->second).c_str(), INFOMSG_INFO);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GiveAllHandler
//
GiveAllHandler::GiveAllHandler() :
		AbstractCommandHandler("Usage: /giveall \"<searchName>\"", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer | Permission_Sage) ;
}

int GiveAllHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: giveall
	 Cheat to give the player all items (or as many as possible) that have
	 a partial match with the given name.
	 Args : [0] = name of object to search for.
	 */

	const char *itemName = query->GetString(0);

	int slotremain = pld->charPtr->inventory.MaxContainerSlot[INV_CONTAINER]
			- pld->charPtr->inventory.containerList[INV_CONTAINER].size();
	if (slotremain <= 0) {
		sim->SendInfoMessage("No free backpack slots", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}
	int wpos = 0;
	InventorySlot newItem;
	vector<ItemDef*> resultList;
	int found = g_ItemManager.EnumPointersByPartialName(itemName, resultList,
			slotremain);
	for (int i = 0; i < found; i++) {
		int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
		if (slot >= 0) {
			slotremain--;
			newItem.dataPtr = resultList[i];
			newItem.IID = resultList[i]->mID;
			newItem.ApplyFromItemDef(resultList[i]);
			newItem.CCSID = (INV_CONTAINER << 16) | slot;
			pld->charPtr->inventory.AddItem(INV_CONTAINER, newItem);
			pld->charPtr->pendingChanges++;
			sim->ActivateActionAbilities(&newItem);
			wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, &newItem);
		} else {
			g_Logs.simulator->debug("[%v] No more free slots.",
					sim->InternalID);
			break;
		}
	}

	if (wpos > 0)
		sim->AttemptSend(sim->SendBuf, wpos);

	sprintf(sim->Aux1, "Gave %d items.", found);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//GiveAppHandler
//
GiveAppHandler::GiveAppHandler() :
		AbstractCommandHandler("Usage: /giveapp <equipType> \"<searchName>\"",
				2) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer | Permission_Sage) ;
}

int GiveAppHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: giveapp
	 Cheat to give the player all items (or as many as possible) that have
	 a partial match with the given equipment type and appearance string.
	 Args : [0] = equip type ID
	 [1] = partial appearance string to match
	 */

	int equipType = query->GetInteger(0);
	const char *searchName = query->GetString(1);

	int slotremain = pld->charPtr->inventory.MaxContainerSlot[INV_CONTAINER]
			- pld->charPtr->inventory.containerList[INV_CONTAINER].size();
	if (slotremain <= 0) {
		sim->SendInfoMessage("No free backpack slots", INFOMSG_ERROR);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}

	int wpos = 0;
	InventorySlot newItem;
	vector<ItemDef*> resultList;
	int found = g_ItemManager.EnumPointersByPartialAppearance(searchName,
			resultList, slotremain);
	int given = 0;
	for (int i = 0; i < found; i++) {
		if ((resultList[i]->mEquipType == equipType) || (equipType <= 0)) {
			int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
			if (slot >= 0) {
				slotremain--;
				newItem.dataPtr = resultList[i];
				newItem.IID = resultList[i]->mID;
				newItem.ApplyFromItemDef(resultList[i]);
				newItem.CCSID = (INV_CONTAINER << 16) | slot;
				pld->charPtr->inventory.AddItem(INV_CONTAINER, newItem);
				pld->charPtr->pendingChanges++;
				wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux3, &newItem);
				given++;
			} else {
				g_Logs.simulator->debug("[%v] No more free slots.",
						sim->InternalID);
				break;
			}
		}
	}

	if (wpos > 0)
		sim->AttemptSend(sim->SendBuf, wpos);

	sprintf(sim->Aux1, "Gave %d items.", given);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//DeleteAllHandler
//
DeleteAllHandler::DeleteAllHandler() :
		AbstractCommandHandler("Usage: /deleteall", 0) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer | Permission_Sage) ;
}

int DeleteAllHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: deleteall
	 Cheat to remove all items from the player's inventory.
	 NOTE: This does not "destroy" the items so it is the only safe way to remove duplicate virtual
	 items aside from manually removing them from a character file.
	 Args : none
	 */

	int size = pld->charPtr->inventory.containerList[INV_CONTAINER].size();
	int a;
	int count = 0;
	int WritePos = 0;
	for (a = 0; a < size; a++) {
		WritePos += RemoveItemUpdate(&sim->SendBuf[WritePos], sim->Aux1,
				&pld->charPtr->inventory.containerList[INV_CONTAINER][a]);
		count++;
		if (sim->CheckWriteFlush(WritePos) == true)
			count = 0;
	}
	if (count > 0)
		sim->AttemptSend(sim->SendBuf, WritePos);

	pld->charPtr->inventory.containerList[INV_CONTAINER].clear();
	pld->charPtr->pendingChanges++;

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DeleteAboveHandler
//
DeleteAboveHandler::DeleteAboveHandler() :
		AbstractCommandHandler("Usage: /deleteabove <slotNumber>", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive | Permission_Admin | Permission_Developer) ;
}

int DeleteAboveHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: deleteabove
	 Cheat to remove all items in the player's inventory above the given
	 slot number.
	 Args : [0] = Slot number to begin deletion (slot adjusted to start at 1)
	 */

	int delSlot = atoi(query->args[0].c_str());

	int WritePos = 0;
	int rcount = 0;
	bool bFound = true;
	while (bFound == true) {
		bFound = false;
		for (size_t a = 0;
				a < pld->charPtr->inventory.containerList[INV_CONTAINER].size();
				a++) {
			unsigned long CCSID =
					pld->charPtr->inventory.containerList[INV_CONTAINER][a].CCSID;
			int slot = CCSID & CONTAINER_SLOT;
			if (slot >= delSlot) {
				WritePos +=
						RemoveItemUpdate(&sim->SendBuf[WritePos], sim->Aux3,
								&pld->charPtr->inventory.containerList[INV_CONTAINER][a]);
				pld->charPtr->inventory.RemItem(CCSID);
				pld->charPtr->pendingChanges++;
				rcount++;
				sim->CheckWriteFlush(WritePos);
				bFound = true;
				break;
			}
		}
	}
	if (WritePos > 0)
		sim->AttemptSend(sim->SendBuf, WritePos);

	sprintf(sim->Aux1, "Removed %d items.", rcount);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GroveHandler
//
GroveHandler::GroveHandler() :
		AbstractCommandHandler("Usage: /grove", 0) {
}

int GroveHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	vector<string> groveList;

	int groveCount = g_ZoneDefManager.EnumerateGroves(pld->accPtr->ID,
			pld->CreatureDefID, groveList);
	int wpos = 0;
	if (groveCount > 0) {
		if (groveCount >= 254)
			groveCount = 254;

		wpos += PutByte(&sim->SendBuf[wpos], 4);   //_handleCreatureEventMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);  //size

		wpos += PutInteger(&sim->SendBuf[wpos], creatureInstance->CreatureID);
		wpos += PutByte(&sim->SendBuf[wpos], 14);  //Event for henge click

		wpos += PutInteger(&sim->SendBuf[wpos],
				ZoneDefManager::HENGE_ID_CUSTOMWARP);

		if (pld->zoneDef->mGuildHall == true) {
			wpos += PutByte(&sim->SendBuf[wpos], groveCount + 1);
			wpos += PutStringUTF(&sim->SendBuf[wpos], EXIT_GUILD_HALL);
			wpos += PutInteger(&sim->SendBuf[wpos], 0);
		} else if (pld->zoneDef->mGrove == true) {
			wpos += PutByte(&sim->SendBuf[wpos], groveCount + 1);
			wpos += PutStringUTF(&sim->SendBuf[wpos], EXIT_GROVE);
			wpos += PutInteger(&sim->SendBuf[wpos], 0);
		} else
			wpos += PutByte(&sim->SendBuf[wpos], groveCount);
		for (int i = 0; i < groveCount; i++) {
			//Henge Name, Cost
			//wpos += PutStringUTF(&sim->SendBuf[wpos], groveList[i]->mWarpName.c_str());
			wpos += PutStringUTF(&sim->SendBuf[wpos], groveList[i].c_str());
			wpos += PutInteger(&sim->SendBuf[wpos], 0);
		}
		PutShort(&sim->SendBuf[1], wpos - 3);  //size
	} else {
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], "No groves found.",
				INFOMSG_INFO);
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//PVPHandler
//
PVPHandler::PVPHandler() :
		AbstractCommandHandler("Usage: /pvp", 0) {
}

int PVPHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	vector<string> zoneList;
	int zoneCount = g_ZoneDefManager.EnumerateArenas(zoneList);
	int wpos = 0;
	if (zoneCount > 0) {
		if (zoneCount >= 254)
			zoneCount = 254;

		wpos += PutByte(&sim->SendBuf[wpos], 4);   //_handleCreatureEventMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);  //size

		wpos += PutInteger(&sim->SendBuf[wpos], creatureInstance->CreatureID);
		wpos += PutByte(&sim->SendBuf[wpos], 14);  //Event for henge click

		wpos += PutInteger(&sim->SendBuf[wpos],
				ZoneDefManager::HENGE_ID_CUSTOMWARP);

		if (pld->zoneDef->mArena == true) {
			wpos += PutByte(&sim->SendBuf[wpos], zoneCount + 1);
			wpos += PutStringUTF(&sim->SendBuf[wpos], EXIT_PVP);
			wpos += PutInteger(&sim->SendBuf[wpos], 0);
		} else
			wpos += PutByte(&sim->SendBuf[wpos], zoneCount);
		for (int i = 0; i < zoneCount; i++) {
			//Henge Name, Cost
			//wpos += PutStringUTF(&sim->SendBuf[wpos], groveList[i]->mWarpName.c_str());
			wpos += PutStringUTF(&sim->SendBuf[wpos], zoneList[i].c_str());
			wpos += PutInteger(&sim->SendBuf[wpos], 0);
		}
		PutShort(&sim->SendBuf[1], wpos - 3);  //size
	} else {
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], "No arenas found.",
				INFOMSG_INFO);
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//CompleteHandler
//
CompleteHandler::CompleteHandler() :
		AbstractCommandHandler("Usage: /complete", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int CompleteHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: complete
	 Cheat to force completion of the current act for all active quests.
	 Args : [none]
	 */
	int wpos = pld->charPtr->questJournal.ForceAllComplete(
			creatureInstance->CreatureID, sim->SendBuf);
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//HelpHandler
//
HelpHandler::HelpHandler() :
		AbstractCommandHandler("Usage: /help", 0) {
}

int HelpHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount == 0) {
		for (auto it = g_QueryManager.queryHandlers.begin();
				it != g_QueryManager.queryHandlers.end(); ++it) {
			QueryHandler *qh = it->second;
			if (AbstractCommandHandler *v =
					dynamic_cast<AbstractCommandHandler*>(qh)) {
				if (v->isAllowed(sim)) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%s",
							it->first.c_str());
					sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
				}
			}
		}
	} else {
		QueryHandler *qh = g_QueryManager.queryHandlers[query->GetString(0)];
		if (qh != NULL) {
			if (AbstractCommandHandler *v =
					dynamic_cast<AbstractCommandHandler*>(qh)) {
				if (v->isAllowed(sim)) {
					Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%s: %s",
							query->GetString(0), v->mUsage.c_str());
					sim->SendInfoMessage(sim->Aux2, INFOMSG_INFO);
				}
			}
		}
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//RefashionHandler
//
RefashionHandler::RefashionHandler() :
		AbstractCommandHandler("Usage: /refashion", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int RefashionHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: refashion
	 Cheat to refashion an item directly from your inventory slots (also works
	 for weapons).  Erases the query and substitutes a new one so that the normal
	 refashion function can process the request.
	 Args : [0] Exact name of source item (for user verification).
	 */

	sprintf(sim->Aux3, "%d", sim->InternalID + 1);

	bool error = false;
	if (query->argCount < 1)
		error = true;
	else if (query->args[0].compare(sim->Aux3) != 0)
		error = true;

	InventorySlot *slot1 = NULL;
	InventorySlot *slot2 = NULL;
	ItemDef *itemPtr1 = NULL;
	ItemDef *itemPtr2 = NULL;
	unsigned long origCCSID = (INV_CONTAINER << 16) | 0;
	unsigned long lookCCSID = (INV_CONTAINER << 16) | 1;

	slot1 = pld->charPtr->inventory.GetItemPtrByCCSID(origCCSID);
	slot2 = pld->charPtr->inventory.GetItemPtrByCCSID(lookCCSID);
	if (slot1 != NULL)
		itemPtr1 = g_ItemManager.GetPointerByID(slot1->IID);
	if (slot2 != NULL)
		itemPtr2 = g_ItemManager.GetPointerByID(slot2->IID);

	if (error == true) {
		Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
				"Stat item in first slot, new look in second.  Use [/refashion %d] to confirm.",
				sim->InternalID + 1);
		sim->SendInfoMessage(sim->Aux3, INFOMSG_INFO);
		if (itemPtr1 != NULL && itemPtr2 != NULL) {
			Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
					"%s will be refashioned to look like %s.",
					itemPtr1->mDisplayName.c_str(),
					itemPtr2->mDisplayName.c_str());
			sim->SendInfoMessage(sim->Aux3, INFOMSG_INFO);
			Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
					"%s will then be destroyed!",
					itemPtr2->mDisplayName.c_str());
			sim->SendInfoMessage(sim->Aux3, INFOMSG_ERROR);
		}
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}

	if (slot1 == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must have an item in the first slot.");
	if (slot2 == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must have an item in the second slot.");

	if (itemPtr1 == NULL || itemPtr2 == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"An item does not exist in the server database.");

	bool sameType = true;
	if (itemPtr1->mType != itemPtr2->mType)
		sameType = false;
	if (itemPtr1->mEquipType != itemPtr2->mEquipType)
		sameType = false;
	if (itemPtr1->mType != itemPtr2->mType)
		sameType = false;

	if (itemPtr1->mType == 2)
		if (itemPtr1->mWeaponType != itemPtr2->mWeaponType)
			sameType = false;

	if (sameType == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Items must be equivalent type (including weapon class).");

	//Clear the arguments only.  The ID must be retained for a proper query response.
	query->args.clear();
	sprintf(sim->Aux3, "%X", (unsigned int) origCCSID);
	query->args.push_back(sim->Aux3);

	sprintf(sim->Aux3, "%X", (unsigned int) lookCCSID);
	query->args.push_back(sim->Aux3);

	query->args.push_back("0");
	query->argCount = query->args.size();

	//Run the virtual query->
	int rval = sim->ItemMorph(true);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

//
//BackupHandler
//
BackupHandler::BackupHandler() :
		AbstractCommandHandler("Usage: /backup", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int BackupHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	pld->charPtr->originalAppearance = creatureInstance->css.appearance;
	int wpos = 0;
	wpos +=
			PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
					"Appearance saved.  Use /restore1 to reload this appearance when necessary.",
					INFOMSG_INFO);
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//RestoreHandler
//
RestoreHandler::RestoreHandler() :
		AbstractCommandHandler("Usage: /restore1", 0) {
}

int RestoreHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//if(sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
	//	return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Permission denied.");

	//Restore a player's appearance based on their original string.
	int wpos = 0;
	if (pld->charPtr->originalAppearance.size() == 0)
		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
				"Your character does not have an appearance backup.",
				INFOMSG_ERROR);
	else {
		//Util::SafeCopy(pld->charPtr->cdef.css.appearance, pld->charPtr->originalAppearance.c_str(), sizeof(pld->charPtr->cdef.css.appearance));
		//Util::SafeCopy(creatureInstance->css.appearance, pld->charPtr->originalAppearance.c_str(), sizeof(creatureInstance->css.appearance));
		pld->charPtr->cdef.css.SetAppearance(
				pld->charPtr->originalAppearance.c_str());
		creatureInstance->css.SetAppearance(
				pld->charPtr->originalAppearance.c_str());

		wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
				"Appearance restored.", INFOMSG_INFO);

		creatureInstance->Submit([creatureInstance]() {
			creatureInstance->actInst->LSendToLocalSimulator(GSendBuf, PrepExt_UpdateAppearance(GSendBuf,creatureInstance), creatureInstance->CurrentX, creatureInstance->CurrentZ);
		});
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//GodHandler
//
GodHandler::GodHandler() :
		AbstractCommandHandler("Usage: /god [<0|1>]", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int GodHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: god
	 Cheat to enable or disable auto aggro hostility between players and mobs.
	 Args : [0] zero or nonzero
	 */
	int wpos = 0;
	if (query->argCount > 0) {
		int mode = atoi(query->args[0].c_str());
		if (mode != 0) {
			creatureInstance->_AddStatusList(StatusEffects::INVINCIBLE, -1);
			sim->SendInfoMessage("God mode ON", INFOMSG_INFO);
		} else {
			creatureInstance->_RemoveStatusList(StatusEffects::INVINCIBLE);
			sim->SendInfoMessage("God mode OFF", INFOMSG_INFO);
		}
	}
	wpos += ::PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;

}

//
//SetStatHandler
//
SetStatHandler::SetStatHandler() :
		AbstractCommandHandler(
				"Usage: /setstat <statName> <newValue> - NOTE, Select target first.",
				2) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int SetStatHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: setstat
	 Cheat to set any object's stats (must be selected).
	 Args : [0] stat name
	 [1] stat value
	 */

	CreatureInstance *targ = creatureInstance->CurrentTarget.targ;
	if (targ == NULL)
		targ = creatureInstance;

	string statName = query->args[0];
	string statValue = query->args[1];
	WriteStatToSetByName(statName, statValue, &targ->css);
	if (targ == creatureInstance)
		WriteStatToSetByName(statName, statValue, &pld->charPtr->cdef.css);

	creatureInstance->Submit([creatureInstance,pld]() {
		creatureInstance->BroadcastCreatureInstanceUpdate();
		creatureInstance->actInst->BroadcastUpdateCreatureDef(&pld->charPtr->cdef, creatureInstance->CurrentX, creatureInstance->CurrentZ);
	});

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ScaleHandler
//
ScaleHandler::ScaleHandler() :
		AbstractCommandHandler(
				"Usage: /scale <propId> <scale|scalex scaley scalez>", 2) {
}

int ScaleHandler::protected_helper_command_scale(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//"Usage: /scale propid scale
	//        /scale propid x y z

	if (query->argCount < 2)
		return QueryErrorMsg::GENERIC;

	int PropID = atoi(query->args[0].c_str());
	float scalex = (float) atof(query->args[1].c_str());
	float scaley = scalex;
	float scalez = scalex;
	if (query->argCount >= 4) {
		scaley = (float) atof(query->args[2].c_str());
		scalez = (float) atof(query->args[3].c_str());
	}
	SceneryObject prop;
	SceneryObject *propPtr = NULL;

	g_SceneryManager.GetThread(
			"SimulatorThread::protected_helper_command_scale");
	//propPtr = g_SceneryManager.GetPropPtr(pld->CurrentZoneID, PropID, NULL);
	propPtr = g_SceneryManager.GlobalGetPropPtr(pld->CurrentZoneID, PropID,
	NULL);
	if (propPtr != NULL)
		prop.copyFrom(propPtr);
	g_SceneryManager.ReleaseThread();

	if (propPtr == NULL)
		return QueryErrorMsg::PROPNOEXIST;

	//Check location permission
//	int tx = (int)(propPtr->LocationX / pld->zoneDef->mPageSize);
//	int ty = (int)(propPtr->LocationZ / pld->zoneDef->mPageSize);

	if (sim->HasPropEditPermission(propPtr) == false)
		return QueryErrorMsg::PROPLOCATION;

	/*  OLD
	 if(pld->accPtr->CheckBuildPermission(pld->CurrentZoneID, tx, ty) == false)
	 return QueryErrorMsg::PROPLOCATION;
	 */

	if (scalex == 0.0F)
		scalex = 1.0F;
	if (scaley == 0.0F)
		scaley = 1.0F;
	if (scalez == 0.0F)
		scalez = 1.0F;

	prop.ScaleX = scalex;
	prop.ScaleY = scaley;
	prop.ScaleZ = scalez;

	g_SceneryManager.GetThread(
			"SimulatorThread::protected_helper_command_scale");
	g_SceneryManager.ReplaceProp(pld->CurrentZoneID, prop);
	g_SceneryManager.ReleaseThread();

	int wpos = PrepExt_UpdateScenery(sim->SendBuf, &prop);
	//creatureInstance->actInst->LSendToAllSimulator(sim->SendBuf, wpos, -1);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

int ScaleHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: scale
	 Command to adjust the individual axis scales of a prop.
	 Args : [0] = propid
	 [1] = scale   OR   [1] = x, [2] = y, [3] = z
	 */

	int rval = protected_helper_command_scale(sim, pld, query,
			creatureInstance);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

//
//PartyAllHandler
//
PartyAllHandler::PartyAllHandler() :
		AbstractCommandHandler("Usage: /partyall", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int PartyAllHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: partyall
	 Command to add all players to the virtual party.
	 Args: [none]
	 */
	int wpos = creatureInstance->actInst->PartyAll(creatureInstance,
			sim->SendBuf);

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//PartyQuitHandler
//
PartyQuitHandler::PartyQuitHandler() :
		AbstractCommandHandler("Usage: /partyquit", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int PartyQuitHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: partyquit
	 Command to disband the virtual party.
	 Args: [none]
	 */
	g_PartyManager.DebugForceRemove(creatureInstance);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 6);  //_handlePartyUpdateMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);

	wpos += PutByte(&sim->SendBuf[wpos], 6);  //Left Party

	PutShort(&sim->SendBuf[1], wpos - 3);       //Set message size

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//CCCHandler
//
CCCHandler::CCCHandler() :
		AbstractCommandHandler("Usage: /ccc [<category>]", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int CCCHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: ccc
	 Cheat to clear cooldowns categories.
	 Args: [none]
	 */

	const char *category = NULL;
	if (query->argCount > 0)
		category = query->args[0].c_str();

	creatureInstance->cooldownManager.Clear();
	int wpos = 0;
	if (category != NULL)
		wpos = PrepExt_CooldownExpired(sim->SendBuf,
				creatureInstance->CreatureID, category);
	else {
		STRINGLIST categoryList;
		g_AbilityManager.GetCooldownCategoryStrings(categoryList);
		for (size_t i = 0; i < categoryList.size(); i++) {
			wpos += PrepExt_CooldownExpired(&sim->SendBuf[wpos],
					creatureInstance->CreatureID, categoryList[i].c_str());
			sim->CheckWriteFlush(wpos);
		}
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//BanHandler
//
BanHandler::BanHandler() :
		AbstractCommandHandler("Usage: /ban \"<category>\" <duration>", 2) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin) ;
}

int BanHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	char denom = 0;
	if (query->argCount >= 3)
		if (query->args[2].size() > 0)
			denom = query->args[2][0];

	const char *charName = query->args[0].c_str();
	int duration = atoi(query->args[1].c_str());

	if (denom == 'h')
		duration *= 60;      //Value should be interpreted as hours
	else if (denom == 'd')
		duration *= 1440;    //As days (60 minutes * 24 hours)
	else if (denom == 'w')
		duration *= 10080;   //As weeks (60 minutes * 24 hours * 7 days)
	else if (denom == 'y')
		duration *= 525600;  //As years (60 minutes * 24 hours * 365 days)

	CharacterData *banChar = NULL;
	g_CharacterManager.GetThread("SimulatorThread::handle_command_ban");
	banChar = g_CharacterManager.GetCharacterByName(query->args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if (banChar == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character not found.");

	bool bFound = false;
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->pld.charPtr == banChar) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"%s [%s] has been suspended.", charName,
					it->pld.accPtr->Name.c_str());
			it->pld.accPtr->SetBan(duration);
			it->Disconnect("SimulatorThread::handle_command_ban");
			bFound = true;
			break;
		}
	}

	if (bFound == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character not logged in.");

	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//UnbanHandler
//
UnbanHandler::UnbanHandler() :
		AbstractCommandHandler("Usage: /unban \"<AccountName>\"", 1) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin) ;
}

int UnbanHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	const char *userName = query->args[0].c_str();

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_unban");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(userName);
	g_AccountManager.cs.Leave();
	if (accPtr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find user.");

	accPtr->ClearBan();
	accPtr->AdjustSessionLoginCount(0);  //Force refresh.

	sim->SendInfoMessage("Ban cleared.", INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//DismountHandler
//
DismountHandler::DismountHandler() :
		AbstractCommandHandler("Usage: /dismount", 0) {
}

int DismountHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if(creatureInstance->IsRider()) {
		if(creatureInstance->Dismount()) {
			sim->SendInfoMessage("Failed to dismount.", INFOMSG_INFO);
		}
		else {
			sim->SendInfoMessage("Dismounted.", INFOMSG_INFO);
		}
	}
	else {
		sim->SendInfoMessage("Not mounted.", INFOMSG_ERROR);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//SetPermissionHandler
//
SetPermissionHandler::SetPermissionHandler() :
		AbstractCommandHandler(
				"Usage: /setpermissions \"<accountName>\" <value> <permission>",
				3) {
	mAllowedPermissions.push_back(Permission_Admin);
}

int SetPermissionHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//[0] = Account name
	//[1] = permission name
	//[2] = flag state (set or clear)

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_setpermission");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(
			query->args[0].c_str());
	g_AccountManager.cs.Leave();
	if (accPtr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find user.");

	int state = atoi(query->args[2].c_str());
	bool r = accPtr->SetPermission(Perm_Account, query->args[1].c_str(),
			(state != 0) ? true : false);
	if (r == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to set.");

	accPtr->PendingMinorUpdates++;

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SetBuildPermissionHandler
//
SetBuildPermissionHandler::SetBuildPermissionHandler() :
		AbstractCommandHandler(
				"Usage: /setbuildpermissions <accountName> <instanceId> [<x1>,<y1>,<x2>,<y2>]",
				2) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int SetBuildPermissionHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//[0] = Account name
	//[1] = permission name
	//[2] = flag state (set or clear)

	if (query->argCount != 2 && query->argCount != 6)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				mUsage.c_str());

	int zoneID = query->GetInteger(1);
	g_AccountManager.cs.Enter(
			"SimulatorThread::handle_command_setbuildpermission");
	AccountData *accPtr = g_AccountManager.FetchAccountByUsername(
			query->args[0].c_str());
	if (accPtr == NULL) {
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find user.");
	}

	if (query->argCount == 2) {
		// Remove
		bool found = false;
		for (vector<BuildPermissionArea>::iterator it =
				accPtr->BuildPermissionList.begin();
				it != accPtr->BuildPermissionList.end(); ++it) {
			BuildPermissionArea pa = *it;
			if (pa.ZoneID == zoneID) {
				accPtr->BuildPermissionList.erase(it);
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Removed permission for %s in zone %d (%d,%d,%d,%d)",
						accPtr->Name.c_str(), zoneID, pa.x1, pa.y1, pa.x2, pa.y2);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				found = true;
				break;
			}
		}
		if (!found) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Could not find permission for %s in zone %d", accPtr->Name.c_str(),
					zoneID);
			sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
		}
	} else {
		// Add / update
		bool found = false;
		for (vector<BuildPermissionArea>::iterator it =
				accPtr->BuildPermissionList.begin();
				it != accPtr->BuildPermissionList.end(); ++it) {
			BuildPermissionArea pa = *it;
			if (pa.ZoneID == zoneID) {
				pa.x1 = query->GetInteger(2);
				pa.y1 = query->GetInteger(3);
				pa.x2 = query->GetInteger(4);
				pa.y2 = query->GetInteger(5);
				found = true;
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Updated permission for %s in zone %d (%d,%d,%d,%d)",
						accPtr->Name.c_str(), pa.ZoneID, pa.x1, pa.y1, pa.x2, pa.y2);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				break;
			}
		}
		if (!found) {
			BuildPermissionArea pa;
			pa.ZoneID = zoneID;
			pa.x1 = query->GetInteger(2);
			pa.y1 = query->GetInteger(3);
			pa.x2 = query->GetInteger(4);
			pa.y2 = query->GetInteger(5);
			accPtr->BuildPermissionList.push_back(pa);
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Added permission for %s in zone %d (%d,%d,%d,%d)",
					accPtr->Name.c_str(), pa.ZoneID, pa.x1, pa.y1, pa.y1, pa.y2);
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
	}
	accPtr->PendingMinorUpdates++;
	g_AccountManager.cs.Leave();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SetPermissionCHandler
//
SetPermissionCHandler::SetPermissionCHandler() :
		AbstractCommandHandler(
				"Usage: /setpermissionc <name> <value> <permission>", 3) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int SetPermissionCHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//[0] = Character name
	//[1] = permission name
	//[2] = flag state (set or clear)

	g_CharacterManager.GetThread(
			"SimulatorThread::handle_command_setpermissionc");
	CharacterData *charPtr = g_CharacterManager.GetCharacterByName(
			query->args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if (charPtr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find user.");

	int state = atoi(query->args[2].c_str());
	bool r = charPtr->SetPermission(Perm_Account, query->args[1].c_str(),
			(state != 0) ? true : false);
	if (r == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to set.");

	charPtr->pendingChanges++;

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DeriveSetHandler
//
DeriveSetHandler::DeriveSetHandler() :
		AbstractCommandHandler("Usage: /deriveset", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int DeriveSetHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (creatureInstance->CurrentTarget.targ == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must select a target.");

	string eqApp = creatureInstance->CurrentTarget.targ->css.eq_appearance;

	unsigned int pos = 0;
	while (pos != string::npos) {
		pos = eqApp.find_first_of("{}[]");
		if (pos != string::npos)
			eqApp.erase(pos, 1);
	}

	STRINGLIST items;
	STRINGLIST itementry;
	vector<int> itemIDList;
	Util::Split(eqApp, ",", items);
	for (size_t i = 0; i < items.size(); i++) {
		Util::Split(items[i], "=", itementry);
		if (itementry.size() > 1)
			itemIDList.push_back(atoi(itementry[1].c_str()));
	}

	struct SetMatch {
		int ItemID;
		int equipType;
	};
	static const SetMatch baseSet[] = { { 8900, 16 }, //"Refashionable Boots",
			{ 8901, 11 }, //"Refashionable Vest",
			{ 8902, 13 }, //"Refashionable Gloves",
			{ 8903, 15 }, //"Refashionable Leggings",
			{ 8904, 12 }, //"Refashionable Sleeves",
			{ 8905, 7 }, //"Refashionable Buckler",
			{ 8906, 9 }, //"Refashionable Collar",
			{ 8907, 14 }, //"Refashionable Belt",
			{ 8908, 10 }, //"Refashionable Shoulderpads",
			{ 8909, 8 }, //"Refashionable Cap",
			};
	static const int numItemTemplate = sizeof(baseSet) / sizeof(SetMatch);

	int wpos = 0;
	bool error = false;
	InventorySlot newItem;
	for (size_t i = 0; i < itemIDList.size(); i++) {
		ItemDef *itemDef = g_ItemManager.GetPointerByID(itemIDList[i]);
		if (itemDef != NULL) {
			for (int s = 0; s < numItemTemplate; s++) {
				if (itemDef->mEquipType == baseSet[s].equipType) {
					int slot = pld->charPtr->inventory.GetFreeSlot(
							INV_CONTAINER);
					if (slot == -1) {
						if (error == false) {
							wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
									"Out of space", INFOMSG_ERROR);
							error = true;
						}
					} else {
						newItem.CCSID = pld->charPtr->inventory.GetCCSID(
								INV_CONTAINER, slot);
						newItem.count = 0;
						newItem.IID = baseSet[s].ItemID;
						newItem.customLook = itemDef->mID;
						newItem.ApplyFromItemDef(itemDef);

						newItem.dataPtr = NULL;
						pld->charPtr->inventory.AddItem(INV_CONTAINER, newItem);
						pld->charPtr->pendingChanges++;
						wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux1,
								&newItem);
					}
				}
			}
		}
	}
	wpos += ::PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//IGStatusHandler
//
IGStatusHandler::IGStatusHandler() :
		AbstractCommandHandler("Usage: /igstatus <status> <value> [target]", 2) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int IGStatusHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int EffectID = GetStatusIDByName(query->args[0]);
	if (EffectID == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid effect.");

	CreatureInstance *target = creatureInstance;
	if (query->argCount >= 3)
		target = creatureInstance->CurrentTarget.targ;
	if (target == NULL)
		target = creatureInstance;

	int value = atoi(query->args[1].c_str());
	if (value == 0)
		target->_RemoveStatusList(EffectID);
	else
		target->_AddStatusList(EffectID, -1);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");

}

//
//PartyZapHandler
//
PartyZapHandler::PartyZapHandler() :
		AbstractCommandHandler("Usage: /partyzap", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int PartyZapHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	g_PartyManager.DebugDestroyParties();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PartyInviteHandler
//
PartyInviteHandler::PartyInviteHandler() :
		AbstractCommandHandler("Usage: /partyinvite \"<characterName>\"", 1) {
}

int PartyInviteHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//Hack to invite party members by command.  Replace the query
	//with the correct party query and arguments, then call the function
	//to process it.
	string charName = query->GetString(0);
	query->name = "party";
	query->args.clear();
	query->args.push_back("invite");
	query->args.push_back(charName);
	query->argCount = query->args.size();
	g_QueryManager.getQueryHandler("party")->handleQuery(sim, pld, query,
			creatureInstance);
	return 0;
}

//
//RollHandler
//
RollHandler::RollHandler() :
		AbstractCommandHandler("Usage: /roll", 0) {
}

int RollHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//Informs the players of a party of a simple "dice roll".
	ActiveParty *party = NULL;
	if (creatureInstance->PartyID > 0)
		party = g_PartyManager.GetPartyByID(creatureInstance->PartyID);

	if (creatureInstance->PartyID == 0 || party == NULL) {
		int roll = g_RandomManager.RandInt(1, 100);
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "You rolled %d", roll);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	} else if (party != NULL) {
		vector<int> rolls;
		for (size_t mi = 0; mi < party->mMemberList.size(); mi++) {
			bool has = false;
			int tries = 0;
			int roll = 0;
			do {
				has = false;
				roll = g_RandomManager.RandInt(1, 100);
				for (size_t i = 0; i < rolls.size(); i++) {
					if (rolls[i] == roll) {
						has = true;
						tries++;
						break;
					}
				}
			} while (has == true && tries < 10);
			rolls.push_back(roll);
		}
		int wpos = 0;
		for (size_t mi = 0; mi < party->mMemberList.size(); mi++) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%s rolled %d",
					party->mMemberList[mi].mDisplayName.c_str(), rolls[mi]);
			wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos], sim->Aux1,
					INFOMSG_INFO);
			if (wpos > Global::MAX_SEND_CHUNK_SIZE) {
				party->BroadCast(sim->SendBuf, wpos);
				wpos = 0;
			}
		}
		if (wpos > 0)
			party->BroadCast(sim->SendBuf, wpos);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//ShutdownHandler
//
ShutdownHandler::ShutdownHandler() :
		AbstractCommandHandler("Usage: /shutdown <minutes> [<reason>]", 1) {
	mAllowedPermissions.push_back(Permission_Admin);
	mShutdownTask = 0;
}

void ShutdownHandler::ScheduleShutdown(int minutes, const string &reason) {
	string time = Util::FormatTimeOfDayMS(g_ServerTime + ( 60000 * minutes ));
	string color = "#3";
	int wait = minutes;
	if(minutes <= 1)
		color = "#0";
	else if(minutes <= 5)
		color = "#1";
	g_SimulatorManager.BroadcastChat(0, "System", "*SysChat",
			Util::Format("%sThe server will be restarted in %d minute%s (%s).# %s Please logout at your earliest convenience!", color.c_str(), minutes, minutes < 2 ? "" : "s", time.c_str(), reason.c_str()).c_str());
	if(minutes > 9) {
		wait -= 10;
	}
	else if(minutes > 5) {
		wait -= 5;
	}
	else if(minutes > 3) {
		wait -= 3;
	}
	else if(minutes > 2) {
		wait -= 2;
	}
	else if(minutes > 1) {
		wait -= 1;
	}
	else {
		mShutdownTask = g_Scheduler.Schedule([this,wait](){
			// Do shutdown!
			g_SimulatorManager.BroadcastChat(0, "System", "*SysChat", "The server is now shutting down.");
			g_Scheduler.Schedule([this](){
				g_ServerStatus = SERVER_STATUS_STOPPED;
			}, g_ServerTime + 5000);

		}, g_ServerTime + (wait * 60000));
		return;
	}

	mShutdownTask = g_Scheduler.Schedule([this,wait,minutes,reason](){
		ScheduleShutdown(minutes - wait, reason);
	}, g_ServerTime + ( wait * 60000));
	return;
}

int ShutdownHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int minutes = atoi(query->args[0].c_str());
	string reason = query->args.size() > 1 ?  Util::Format("The reason given was '%s'.", query->args[1].c_str()) : "No reason given.";
	if(mShutdownTask == 0) {
		if(query->args.size() < 3 || query->args[2] != "--nomaintenance")
			g_Config.MaintenanceMessage = reason;
		ScheduleShutdown(minutes, reason);
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	}
	else {
		if(query->args[0] == "0") {
			g_Scheduler.Cancel(mShutdownTask);
			mShutdownTask = 0;
			g_Config.MaintenanceMessage = "";
			g_SimulatorManager.BroadcastChat(0, "System", "*SysChat", Util::Format("The shutdown has been cancelled until further notice.").c_str());
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		}
		else {
			return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "Shutdown is already scheduled. Use '/shutdown 0' to cancel it.");
		}
	}
}
//
//ForumLockHandler
//
ForumLockHandler::ForumLockHandler() :
		AbstractCommandHandler("Usage: /forumlock [<status>]", 0) {
	;
	mAllowedPermissions.push_back(Permission_Admin);
}

int ForumLockHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	bool status = false;
	if (query->argCount >= 1)
		status = Util::IntToBool(atoi(query->args[0].c_str()));

	g_IGFManager.mForumLocked = status;
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Forum lock status: %d",
			status);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ZoneNameHandler
//
ZoneNameHandler::ZoneNameHandler() :
		AbstractCommandHandler("Usage: /zonename <name>", 2) {
	;
	mAllowedPermissions.push_back(Permission_Admin);
}

int ZoneNameHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount >= 2) {
		const char *name = query->args[0].c_str();
		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
				pld->CurrentZoneID);
		if (zoneDef != NULL) {
			if (zoneDef->mGrove == true) {
				zoneDef->ChangeName(name);
				g_ZoneDefManager.NotifyConfigurationChange();
				sim->SendSetMap();
//				sim->SendInfoMessage(pld->zoneDef->mShardName.c_str(),
//						INFOMSG_SHARD);
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DtrigHandler
//
DtrigHandler::DtrigHandler() :
		AbstractCommandHandler("Usage: /dtrig <op> [<arg1> ..]", 1) {
	mAllowedPermissions.push_back(Permission_Admin);
	mAllowedPermissions.push_back(Permission_Developer);
}

int DtrigHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	int action = 0;
	if (query->argCount > 0)
		action = query->GetInteger(0);

	switch (action) {
	case 11:
		Fun::oFunReplace.Reset();
		break;
	case 15:
		if (query->argCount >= 2) {
			const char *name = query->GetString(1);
			for (size_t i = 0; i < creatureInstance->actInst->NPCListPtr.size();
					i++) {
				CreatureInstance *ptr = creatureInstance->actInst->NPCListPtr[i];
				if (strstr(ptr->css.display_name, name) != NULL) {
					query->name = "warp";
					query->args.clear();
					sprintf(sim->Aux1, "%d", ptr->CurrentX);
					sprintf(sim->Aux2, "%d", ptr->CurrentZ);
					query->args.push_back(sim->Aux1);
					query->args.push_back(sim->Aux2);
					return g_QueryManager.queryHandlers[query->name]->handleQuery(
							sim, pld, query, creatureInstance);
					break;
				}
			}
		}
		break;
	case 17: {
		int searchlen = 200;
		if (query->argCount > 1)
			searchlen = query->GetInteger(1);
		vector<SceneryObject*> search;
		g_SceneryManager.GetThread("SimulatorThread::handle_command_dtrig");
		g_SceneryManager.EnumPropsInRange(pld->CurrentZoneID,
				creatureInstance->CurrentX, creatureInstance->CurrentZ,
				searchlen, search);
		map<string, int> pkgmap;
		map<string, int>::iterator it;
		for (size_t i = 0; i < search.size(); i++) {
			if (strstr(search[i]->Asset.c_str(), "SpawnPoint") == NULL)
				continue;
			if (!search[i]->hasExtraData)
				continue;
			pkgmap[search[i]->extraData.spawnPackage]++;
		}
		for (it = pkgmap.begin(); it != pkgmap.end(); ++it)
			g_Logs.simulator->info("%v----:%v", it->first.c_str(), it->second);
		g_SceneryManager.ReleaseThread();
	}
		break;
	case 19: {
		CreatureInstance *ptr = creatureInstance->CurrentTarget.targ;
		if (ptr) {
			g_Logs.simulator->info("name=%v", ptr->css.display_name);
			g_Logs.simulator->info("aggro=%v", ptr->css.aggro_players);
			g_Logs.simulator->info("ai=%v", ptr->css.ai_package);
			g_Logs.simulator->info("faction=%v", ptr->Faction);
			g_Logs.simulator->info("sflags=%v", ptr->serverFlags);
			g_Logs.simulator->info("Rotation=%d", ptr->Rotation);
			g_Logs.simulator->info("Heading=%d", ptr->Heading);
			for (size_t i = 0; i <= 62; i++)
				if (ptr->HasStatus(i))
					g_Logs.simulator->info("Status:%v", GetStatusNameByID(i));

			char ConvBuf[32];
			size_t i;
			for (i = 0; i < NumStats; i++) {
				g_Logs.simulator->info("%s=%s", StatList[i].name,
						GetStatValueAsString(i, ConvBuf, &ptr->css));
			}
		}
	}
		break;
	case 24:
		g_ActiveInstanceManager.DebugFlushInactiveInstances();
		break;
	case 100:
		if (creatureInstance->CurrentTarget.targ != NULL)
			creatureInstance->CurrentTarget.targ->ApplyRawDamage(99999);
		break;
	case 110: {
		CreatureInstance *target = NULL;
		int value = 0;
		int EffectID = 0;
		if (query->argCount > 1)
			target = g_ActiveInstanceManager.GetPlayerCreatureByName(
					query->GetString(1));
		if (query->argCount > 2)
			EffectID = GetStatusIDByName(query->GetString(2));
		if (query->argCount > 3)
			value = query->GetInteger(3);

		if (target == NULL) {
			sim->SendInfoMessage("Could not find target.", INFOMSG_ERROR);
			break;
		}
		if (EffectID == -1) {
			sim->SendInfoMessage("Invalid effect.", INFOMSG_ERROR);
			break;
		}
		if (target != NULL && EffectID >= 0) {
			if (value == 0)
				target->_RemoveStatusList(EffectID);
			else
				target->_AddStatusList(EffectID, -1);
		}
	}
		break;
	case 902: //Scenery import.  Not recommended for public server use, prefer to use offline first.
	{
		if (query->argCount < 3) {
			sim->SendInfoMessage("/dtrig 902 <filename> <allowMerge>",
					INFOMSG_INFO);
			break;
		}
		g_SceneryManager.GetThread("dtrig");
		const char *fileName = query->GetString(1);
		bool allowMerge = query->GetBool(2);
		int add = 0;
		int merge = 0;
		SceneryPage page;
		page.LoadSceneryFromFile(fileName);
		SceneryPage::SCENERY_IT it;
		SceneryPage *found = NULL;
		int zoneID = pld->zoneDef->mID;
		for (it = page.mSceneryList.begin(); it != page.mSceneryList.end();
				++it) {
			SceneryObject *so = g_SceneryManager.GlobalGetPropPtr(zoneID,
					it->second.ID, &found);
			if (so == NULL) {
				SceneryObject *nso = g_SceneryManager.AddProp(zoneID,
						it->second);
				if (nso)
					nso->Name = "IMPORT";
				add++;
			} else if (allowMerge == true) {
				so->copyFrom(&it->second);
				if (found != NULL)
					found->NotifyAccess(true);
				merge++;
			} else {
				g_Logs.simulator->info("Skipped: %v", it->second.ID);
			}
		}
		g_SceneryManager.ReleaseThread();
		g_SceneryManager.CheckAutosave(true);
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Scenery import to zone %d: %d in file, %d added, %d merged",
				zoneID, page.mSceneryList.size(), add, merge);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		g_Logs.simulator->info(sim->Aux1);
	}
		break;
	case 903:
		g_CraftManager.LoadData();
		break;
	case 904:
		g_InstanceScaleManager.LoadData();
		break;
	case 906:
		g_DropRateProfileManager.LoadData();
		break;
	case 909:
		if (query->argCount >= 3) {
			const char *pkg = query->GetString(1);
			const char *snd = query->GetString(2);
			sim->SendPlaySound(pkg, snd);
		}
		break;
	case 999:
		if (query->argCount > 1) {
			const char *message = query->GetString(1);
			g_SimulatorManager.BroadcastMessage(message);
		}
		break;
	case 1000: {
		bool state = false;
		int range = 960;
		if (query->argCount > 1)
			state = query->GetInteger(1) != 0;
		if (query->argCount > 2)
			range = query->GetInteger(2);
		creatureInstance->actInst->SetAllPlayerPVPStatus(
				creatureInstance->CurrentX, creatureInstance->CurrentZ, range,
				state);
	}
		break;
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SdiagHandler
//
SdiagHandler::SdiagHandler() :
		AbstractCommandHandler("Usage: /sdiag <val>", 0) {
}

int SdiagHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int val = 0;
	if (query->argCount > 0)
		val = atoi(query->args[0].c_str());
	pld->accPtr->SetPermission(Perm_Account, "selfdiag", (val != 0));
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
			"Self diagnostic check is %s", ((val != 0) ? "ON" : "OFF"));
	int wpos = PrepExt_SendInfoMessage(sim->SendBuf, sim->Aux1, INFOMSG_INFO);
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//SpingHandler
//
SpingHandler::SpingHandler() :
		AbstractCommandHandler("Usage: /sping [<time>]", 0) {
}

int SpingHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount == 0) {
		//Just send a report.
		int average = 0;
		if (pld->DebugPingServerTotalReceived != 0)
			average = pld->DebugPingServerTotalTime
					/ pld->DebugPingServerTotalReceived;
		sprintf(sim->Aux1,
				"Pings:%d  Lowest:%d  Highest:%d  Avg:%d  Sync:%d/%d",
				pld->DebugPingServerTotalReceived, pld->DebugPingServerLowest,
				pld->DebugPingServerHighest, average, pld->DebugPingServerSent,
				pld->DebugPingServerTotalReceived);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	} else {
		//Set the notify time.
		int time = query->GetInteger(0);
		if (time >= 0) {
			pld->DebugPingServerNotifyTime = query->GetInteger(0);
			if (time == 0)
				sprintf(sim->Aux1, "Server ping notification is now off.");
			else
				sprintf(sim->Aux1, "Server ping notification set to %d ms.",
						time);
		} else {
			pld->DebugPingServerSent = 0;
			pld->DebugPingServerTotalReceived = 0;
			pld->DebugPingServerTotalTime = 0;
			pld->DebugPingServerLowest = 0;
			pld->DebugPingServerHighest = 0;
			sprintf(sim->Aux1, "Server ping statistics cleared.");
		}
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//SpingHandler
//
InfoHandler::InfoHandler() :
		AbstractCommandHandler("Usage: /info <spawntile|scenerytile>", 1) {
}

int InfoHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	sim->Aux1[0] = 0;
	if (query->args[0].compare("spawntile") == 0)
		sprintf(sim->Aux1, "Spawn Tile: %d, %d", pld->oldSpawnX,
				pld->oldSpawnZ);
	if (query->args[0].compare("scenerytile") == 0)
		sprintf(sim->Aux1, "Scenery Tile: %d, %d",
				creatureInstance->CurrentX / pld->zoneDef->mPageSize,
				creatureInstance->CurrentZ / pld->zoneDef->mPageSize);
	if (query->args[0].compare("terraintile") == 0)
		// TODO is this always correct?
		sprintf(sim->Aux1, "Terrain Tile: %d, %d",
				creatureInstance->CurrentX / 1920,
				creatureInstance->CurrentZ / 1920);
	if (sim->Aux1[0] != 0)
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GroveSetingHandler
//
GroveSettingHandler::GroveSettingHandler() :
		AbstractCommandHandler(
				"Usage: /grovesetting <filtertype|filteradd|filterclear|filterlist> [<args> ..]",
				2) {
}

int GroveSettingHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");

	//0 = bad command, 1 = success
	bool code = 1;
	if (query->args[0].compare("filtertype") == 0)
		pld->zoneDef->mPlayerFilterType = query->GetInteger(1);
	else if (query->args[0].compare("filteradd") == 0) {
		int cdef = g_UsedNameDatabase.GetIDByName(query->GetString(1));
		if (cdef == -1) {
			sim->SendInfoMessage("Character not found.", INFOMSG_ERROR);
			code = 0;
		} else
			pld->zoneDef->AddPlayerFilterID(cdef);
	} else if (query->args[0].compare("filterremove") == 0) {
		int cdef = g_UsedNameDatabase.GetIDByName(query->GetString(1));
		if (cdef == -1) {
			sim->SendInfoMessage("Character not found.", INFOMSG_ERROR);
			code = 0;
		} else
			pld->zoneDef->RemovePlayerFilter(cdef);
	} else if (query->args[0].compare("filterclear") == 0) {
		pld->zoneDef->ClearPlayerFilter();
	} else if (query->args[0].compare("filterlist") == 0) {
		const vector<int> &IDs = pld->zoneDef->mPlayerFilterID;
		for (size_t i = 0; i < IDs.size(); i++) {
			const char *name = g_UsedNameDatabase.GetNameByID(IDs[i]);
			if (name == NULL)
				name = "<unknown character>";
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "#%d:%s", i, name);
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
	} else
		sim->SendInfoMessage("Unknown command.", INFOMSG_ERROR);

	if (code == 1) {
		sim->SendInfoMessage("Action completed.", INFOMSG_INFO);
		g_ZoneDefManager.NotifyConfigurationChange();
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GrovePermissionsHandler
//
GrovePermissionsHandler::GrovePermissionsHandler() :
		AbstractCommandHandler("Usage: /grovepermissions <permission>", 1) {
}

int GrovePermissionsHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	//Sent either as a command or query from the permission editor panel in a modded client.
	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");

	pld->zoneDef->UpdateGrovePermission(query->args);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DngScaleHandler
//
DngScaleHandler::DngScaleHandler() :
		AbstractCommandHandler("Usage: /dngscale [<profile>]", 0) {
}

int DngScaleHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (pld->zoneDef->IsDungeon()) {
		sim->SendInfoMessage(
				"You may not set your dungeon scaler inside a dungeon. Once the scale has been set, it remains until the dungeon instance completely closes (which may be some time after all of your party exit the dungeon)",
				INFOMSG_ERROR);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You may not set your dungeon scaler inside a dungeon.");
	}

	string outputMsg;
	if (query->argCount > 0) {
		const char *profName = query->GetString(0);
		const InstanceScaleProfile *prof = g_InstanceScaleManager.GetProfile(
				profName);
		if (prof == NULL) {
			outputMsg = "Profile does not exist: ";
			outputMsg.append(profName);
		} else {
			outputMsg = "Profile set to: ";
			outputMsg.append(profName);
			pld->charPtr->InstanceScaler = profName;
		}
	} else {
		outputMsg = "Profile cleared.";
		pld->charPtr->InstanceScaler.clear();
	}
	sim->SendInfoMessage(outputMsg.c_str(), INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PathLinksHandler
//
PathLinksHandler::PathLinksHandler() :
		AbstractCommandHandler("Usage: /pathlinks [<radius>]", 0) {
}

int PathLinksHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
// Convert linked scenery to use the correct path links.
	if (pld->zoneDef->mGrove == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not in a grove.");
	if (pld->zoneDef->mAccountID != pld->accPtr->ID)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must be in your grove.");

	int radius = 300;
	if (query->argCount > 0)
		radius = query->GetInteger(0);

	vector<SceneryObject*> results;
	g_SceneryManager.EnumPropsInRange(pld->CurrentZoneID,
			creatureInstance->CurrentX, creatureInstance->CurrentZ, radius,
			results);
	int propChanges = 0;
	for (size_t i = 0; i < results.size(); i++) {
		SceneryObject *so = results[i];
		if (so->IsSpawnPoint() == false)
			continue;
		if (so->HasLinks(SceneryObject::LINK_TYPE_LOYALTY) == false)
			continue;
		if (!so->hasExtraData)
			continue;
		int linkChanges = 0;
		for (auto a = so->extraData.link.begin(); a != so->extraData.link.end(); ++a) {
			if ((*a).type
					== SceneryObject::LINK_TYPE_LOYALTY) {
				(*a).type = SceneryObject::LINK_TYPE_PATH;
				linkChanges++;
			}
		}
		if (linkChanges > 0) {
			propChanges++;
			int wpos = PrepExt_UpdateScenery(sim->SendBuf, so);
			sim->AttemptSend(sim->SendBuf, wpos);

			//Flag the scenery for autosaves.
			g_SceneryManager.NotifyChangedProp(pld->CurrentZoneID, so->ID);
		}
	}
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Updated %d SpawnPoints",
			propChanges);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//TargHandler
//
TargHandler::TargHandler() :
		AbstractCommandHandler("Usage: /pathlinks [<radius>]", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int TargHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	int wpos = 0;
	if (creatureInstance->CurrentTarget.targ != NULL) {
		int dist = ActiveInstance::GetPlaneRange(creatureInstance,
				creatureInstance->CurrentTarget.targ, 99999);
		sprintf(sim->Aux1,
				"Name: %s (%d), ID: %d, CDef: %d (%d) (D:%d) PVPable: %s",
				creatureInstance->CurrentTarget.targ->css.display_name,
				creatureInstance->CurrentTarget.targ->css.level,
				creatureInstance->CurrentTarget.targ->CreatureID,
				creatureInstance->CurrentTarget.targ->CreatureDefID,
				creatureInstance->CurrentTarget.targ->css.health, dist,
				creatureInstance->CanPVPTarget(
						creatureInstance->CurrentTarget.targ) ? "yes" : "no");
		wpos += ::PrepExt_SendInfoMessage(sim->SendBuf, sim->Aux1,
				INFOMSG_INFO);
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//UnstickHandler
//
UnstickHandler::UnstickHandler() :
		AbstractCommandHandler("Usage: /unstick", 0) {
}

int UnstickHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {/* Query: unstick
		 */
	//if(creatureInst->HasStatus(StatusEffects::DEAD))
	//	SendInfoMessage("You cannot unstick while dead.", INFOMSG_ERROR);
	//else
	//SendInfoMessage("Unsticking will apply a temporary stat penalty for 2 minutes.", INFOMSG_INFO);
	if (pld->charPtr->NotifyUnstick(true) == true)
		sim->SendInfoMessage(
				"Unsticking too often will apply a temporary stat penalty for 2 minutes.",
				INFOMSG_INFO);

	creatureInstance->RequestAbilityActivation(10006);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ElevHandler
//
ElevHandler::ElevHandler() :
		AbstractCommandHandler("Usage: /elev <y>", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int ElevHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	creatureInstance->CurrentY = query->GetInteger(0);
	creatureInstance->BroadcastUpdateElevationSelf();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//CycleHandler
//
CycleHandler::CycleHandler() :
		AbstractCommandHandler("Usage: /cycle", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Developer) ;
}

int CycleHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
//	if(!g_ClusterManager.IsMaster())
//		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "You must be on the master shard to be able to cycle environments.");
////	g_EnvironmentCycleManager.EndCurrentCycle();
//	g_Logs.server->info("Cycle is now: %v (%v)",
//			g_EnvironmentCycleManager.mCurrentCycleIndex,
//			g_EnvironmentCycleManager.GetCurrentTimeOfDay());
//	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "No longer supported.");
}

//
//TimeHandler
//
TimeHandler::TimeHandler() :
		AbstractCommandHandler("Usage: /time", 0) {
}

int TimeHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	sim->SendInfoMessage(Util::Format("Gaia Time: %s (%s)", Util::FormatTimeHHMMSS(g_PlatformTime.getPseudoTimeOfDayMilliseconds()).c_str(), g_EnvironmentCycleManager.GetCurrentCycle().mName.c_str()).c_str(), INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//SearSizeHandler
//
SearSizeHandler::SearSizeHandler() :
		AbstractCommandHandler("Usage: /searsize [<size>]", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int SearSizeHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	CreatureInstance *creature = creatureInstance->CurrentTarget.targ;
	if (creature == NULL)
		creature = creatureInstance;

	float val = 0;
	if (query->argCount > 0)
		val = atof(query->args[0].c_str());
	char buf[10];
	Util::SafeFormat(buf, sizeof(buf), "%1.1f", val);
	CreatureAttributeModifier mod("es", buf);
	creature->charPtr->originalAppearance = mod.Modify(
			creature->charPtr->originalAppearance);
	creature->css.SetAppearance(mod.Modify(creature->css.appearance).c_str());
	creature->charPtr->pendingChanges++;
	int wpos = PrepExt_UpdateAppearance(sim->SendBuf, creature);
	creature->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creature->CurrentX, creature->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//StailSizeHandler
//
StailSizeHandler::StailSizeHandler() :
		AbstractCommandHandler("Usage: /stailsize [<size>]", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Developer) ;
}

int StailSizeHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	CreatureInstance *creature = creatureInstance->CurrentTarget.targ;
	if (creature == NULL)
		creature = creatureInstance;
	float val = 0;
	if (query->argCount > 0)
		val = atof(query->args[0].c_str());
	char buf[10];
	Util::SafeFormat(buf, sizeof(buf), "%1.1f", val);
	CreatureAttributeModifier mod("ts", buf);
	creature->css.SetAppearance(mod.Modify(creature->css.appearance).c_str());
	creature->charPtr->originalAppearance = mod.Modify(
			creature->charPtr->originalAppearance);
	creature->charPtr->pendingChanges++;
	int wpos = PrepExt_UpdateAppearance(sim->SendBuf, creature);
	creature->actInst->LSendToLocalSimulator(sim->SendBuf, wpos,
			creature->CurrentX, creature->CurrentZ);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DailyHandler
//
DailyHandler::DailyHandler() :
		AbstractCommandHandler("Usage: /daily [<days> <level>]", 0) {
	mAllowedPermissions.push_back(Permission_Debug | Permission_Admin | Permission_Developer) ;
}

int DailyHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*  Query: daily
	 Cheat to give yourself a new daily reward. Used for test.
	 Args : [0] = equip type ID
	 [1] = partial appearance string to match
	 */

	pld->accPtr->DueDailyRewards = true;

	unsigned int days = pld->accPtr->ConsecutiveDaysLoggedIn;
	unsigned int level = pld->charPtr->cdef.css.level;

	if (query->argCount == 2) {
		days = query->GetInteger(0);
		level = query->GetInteger(1);
	} else if (query->argCount != 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /daily [<days> <level>]");
	}
	sim->ProcessDailyRewards(days, level);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// WarpHandler
//

WarpHandler::WarpHandler() :
		AbstractCommandHandler("Usage: /warp <x> <y> | \"<playerName>\"", 1) {
}

int WarpHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/* Query: warp
	 Args : Optional types:
	 [0] = characterName
	 [0] = direction [n|e|s|w]
	 [0] = direction [n|e|s|w], [1] = distance
	 [0] = XPos, [1] = Ypos
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	int zone = pld->CurrentZoneID;
	//int instance = pld.CurrentInstanceID;
	int instance = 0; //Only set the instance if we need an explicit warp there.
	int xpos = creatureInstance->CurrentX;
	int ypos = creatureInstance->CurrentY;
	int zpos = creatureInstance->CurrentZ;

	if (query->argCount >= 2) {
		const char *param1 = query->args[0].c_str();
		int param2 = atoi(query->args[1].c_str());

		//First check for relative directional positions before
		//processing a raw coordinate
		if (strcmp(param1, "n") == 0)
			zpos -= param2;
		else if (strcmp(param1, "s") == 0)
			zpos += param2;
		else if (strcmp(param1, "w") == 0)
			xpos -= param2;
		else if (strcmp(param1, "e") == 0)
			xpos += param2;
		else {
			//Coordinate warp
			xpos = atoi(param1);
			zpos = param2;
		}
	} else if (query->argCount == 1) {
		const char *target = query->args[0].c_str();
		if (strcmp(target, "n") == 0)
			zpos -= sim->DefaultWarpDistance;
		else if (strcmp(target, "s") == 0)
			zpos += sim->DefaultWarpDistance;
		else if (strcmp(target, "w") == 0)
			xpos -= sim->DefaultWarpDistance;
		else if (strcmp(target, "e") == 0)
			xpos += sim->DefaultWarpDistance;
		else if (strchr(target, ',') != NULL) {
			vector<string> args;
			Util::Split(query->args[0], ",", args);
			if (args.size() >= 2) {
				xpos = atoi(args[0].c_str());
				zpos = atoi(args[1].c_str());
				if (args.size() >= 3)
					zpos = atoi(args[2].c_str()); //Hack for x,y,z strings since they're often copy/pasted
			}
		} else {
			//Check names
			bool bFound = false;

			SIMULATOR_IT it;
			for (it = Simulator.begin(); it != Simulator.end(); ++it) {
				if (it->isConnected == true && it->ProtocolState == 1) {
					if (it->IsGMInvisible() == true)
						continue;
					if (strstr(it->pld.charPtr->cdef.css.display_name,
							target) != NULL) {
						zone = it->pld.CurrentZoneID;
						instance = it->pld.CurrentInstanceID;
						xpos = it->creatureInst->CurrentX;
						ypos = it->creatureInst->CurrentY;
						zpos = it->creatureInst->CurrentZ;
						bFound = true;
						break;
					}
				}
			}
			if (bFound == false) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Could not find target for warp: %s", target);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
			}
		}
	}

	if (zone != pld->CurrentZoneID) {
		int errCode = sim->CheckValidWarpZone(zone);
		if (errCode != sim->ERROR_NONE)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					sim->GetGenericErrorString(errCode));
	}

	sim->DoWarp(zone, instance, xpos, ypos, zpos);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// WarpInstanceHandler
//

WarpInstanceHandler::WarpInstanceHandler() :
		AbstractCommandHandler("Usage: /warpi <zoneName>", 1) {
}

int WarpInstanceHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpi
	 Handles on-demand warping to instances.
	 Args : 1, [0] = Instance Name
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	string warpTarg = query->args[0];
	int queryID = query->ID;
	int simID = sim->InternalID;

	/* The partial name match now queries the cluster database directly as we do not
	 * have all zone names in our own processes memory. This can take a comparatively
	 * long time (>2 seconds on my machine). For this reason the whole operation is
	 * put into the new thread pool for execution
	 */
	g_Scheduler.Pool([warpTarg, queryID, simID](){
		ZoneDefInfo *targZone = g_ZoneDefManager.GetPointerByPartialWarpName(warpTarg);
		/* Put the remainder of the processing back on the main server thread */
		g_Scheduler.Submit([targZone, warpTarg, queryID, simID]() {

			SimulatorThread *sim = g_SimulatorManager.GetPtrByID(simID);
			if(sim == NULL) {
				g_Logs.simulator->error("Lost simulator [%v] before we got chance to warpi to %v", simID, warpTarg);
			}
			else  {
				int wpos = 0;
				if (targZone == NULL) {
					string msg = Util::Format("Zone name not found: %s", warpTarg.c_str());
					g_Logs.simulator->error("[%v] %v", simID, msg);
					//sim->SendInfoMessage(msg.c_str(), INFOMSG_ERROR);
					wpos += PrepExt_QueryResponseError(sim->SendBuf, queryID,
							msg.c_str());
				} else {
					int errCode = sim->CheckValidWarpZone(targZone->mID);
					if (errCode != sim->ERROR_NONE)
						wpos += PrepExt_QueryResponseError(sim->SendBuf, queryID,
								sim->GetGenericErrorString(errCode));
					else {
						//If the avatar is running, it will glitch the position.

						// EM - Is this really needed?
						//		SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);

						if (sim->ProtectedSetZone(targZone->mID, 0) == false) {
							sim->ForceErrorMessage("Critical error while changing zones.",
									INFOMSG_ERROR);
							sim->Disconnect("SimulatorThread::handle_command_warpi");
							wpos += PrepExt_QueryResponseError(sim->SendBuf, queryID,
									"Critical error.");
						}
						else  {
							sim->SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);
							wpos += PrepExt_QueryResponseString(sim->SendBuf, queryID, "OK");
						}
					}
				}
				if(wpos > 0) {
					sim->AttemptSend(sim->SendBuf, wpos);
				}
			}
		});
	});

	return 0;
}

//
// WarpTileHandler
//

WarpTileHandler::WarpTileHandler() :
		AbstractCommandHandler("Usage: /warpt <x> <y>", 2) {
}

int WarpTileHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpt
	 Handles on-demand warping to a scenery tile coordinate within the instance.
	 Args : 2, [0] = TileX, [1] = TileY
	 */

	if (pld->zoneDef->mGrove == false)
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	int x = atoi(query->args[0].c_str());
	int z = atoi(query->args[1].c_str());

	int xtarg = Util::ClipInt(x, 0, 100) * pld->zoneDef->mPageSize;
	int ztarg = Util::ClipInt(z, 0, 100) * pld->zoneDef->mPageSize;

	sim->SetPosition(xtarg, creatureInstance->CurrentY, ztarg, 1);
	sim->SendInfoMessage("Warping.", INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// WarpPullHandler
//

WarpPullHandler::WarpPullHandler() :
		AbstractCommandHandler("Usage: /warpp", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin) ;
}

int WarpPullHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpp
	 An external warp that pull a target directly to the player.
	 */

	CreatureInstance *targ = NULL;
	if (query->argCount == 0)
		targ = creatureInstance->CurrentTarget.targ;
	else
		targ = creatureInstance->actInst->GetPlayerByName(
				query->args[0].c_str());

	if (targ == NULL || targ == creatureInstance)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must select a target.");

	targ->CurrentX = creatureInstance->CurrentX;
	targ->CurrentY = creatureInstance->CurrentY;
	targ->CurrentZ = creatureInstance->CurrentZ;

	creatureInstance->Submit(bind(&CreatureInstance::BroadcastPositionUpdate, creatureInstance));

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
// WarpGroveHandler
//

WarpGroveHandler::WarpGroveHandler() :
		AbstractCommandHandler("Usage: /warpg [<grove>]", 0) {
}

int WarpGroveHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	const char *grove = NULL;
	if (query->argCount > 0)
		grove = query->GetString(0);

	if (grove == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No grove specified.");

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByExactWarpName(grove);
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Grove not found.");
	if (zoneDef->mGrove == false && !zoneDef->mGuildHall)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Destination is not a grove.");

	int errCode = sim->CheckValidWarpZone(zoneDef->mID);
	if (errCode != sim->ERROR_NONE)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetGenericErrorString(errCode));

	int xLoc = 0;
	int zLoc = 0;
	if (query->argCount == 3) {
		xLoc = query->GetInteger(1);
		zLoc = query->GetInteger(2);
	}
	sim->WarpToZone(zoneDef, xLoc, 0, zLoc);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// WarpExternalOfflineHandler
//

WarpExternalOfflineHandler::WarpExternalOfflineHandler() :
		AbstractCommandHandler(
				"Usage: /warpextoffline \"<characterName>\" <zoneID> [<x> <z> [y]]",
				2) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Debug);
}
int WarpExternalOfflineHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpext
	 Warp an external target to the player.  Used to set positions of offline characters.
	 Args: [0] = Character Name [required]
	 [1] = Zone ID to warp the character to [required]
	 [2][3] = X and Z coordinate to warp [optional]
	 [4] = Y coordinate to warp [optional]
	 */

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
			atoi(query->args[1].c_str()));
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Zone not found.");

	CharacterData *cd = NULL;
	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.GetCharacterByName(query->args[0].c_str());
	g_CharacterManager.ReleaseThread();
	if (cd != NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character must be offline.");

	g_AccountManager.cs.Enter("SimulatorThread::handle_command_warpext");
	int CDef = g_UsedNameDatabase.GetIDByName(query->args[0]);
	g_AccountManager.cs.Leave();

	if (CDef == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Character name not found.");

	g_CharacterManager.GetThread("SimulatorThread::handle_command_warpext");
	cd = g_CharacterManager.RequestCharacter(CDef, true);
	g_CharacterManager.ReleaseThread();

	if (cd == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to load character.");
	cd->ExtendExpireTime();

	int warpx = zoneDef->DefX;
	int warpy = zoneDef->DefY;
	int warpz = zoneDef->DefZ;
	if (query->argCount >= 4) {
		warpx = atoi(query->args[2].c_str());
		warpz = atoi(query->args[3].c_str());
	}
	if (query->argCount >= 5)
		warpy = atoi(query->args[4].c_str());

	cd->activeData.CurZone = zoneDef->mID;
	cd->LastWarpTime = g_ServerTime;
	cd->activeData.CurX = warpx;
	cd->activeData.CurY = warpy;
	cd->activeData.CurZ = warpz;
	cd->SetExpireTime();

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// WarpExternalHandler
//

WarpExternalHandler::WarpExternalHandler() :
		AbstractCommandHandler("Usage: /warpext \"<characterName>\" <zoneID>",
				2) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Debug);
}

int WarpExternalHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/*  Query: warpext2
	 Warp an external target to a new zone ID.
	 Args: [0] = Character Name [required]
	 [1] = Zone ID to warp the character to [required]
	 */

	ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
			atoi(query->args[1].c_str()));
	if (zoneDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Zone not found.");

	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it)
		if (it->ProtocolState == 1)
			if (strcmp(it->creatureInst->css.display_name,
					query->args[0].c_str()) == 0) {
				sim->SendInfoMessage("Warping target.", INFOMSG_INFO);
				it->MainCallSetZone(zoneDef->mID, 0, true);
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"OK");
			}
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Target not found.");
}

//
// ScriptExecHandler
//

ScriptExecHandler::ScriptExecHandler() :
		AbstractCommandHandler(
				"Usage: /script.exec [-q] <function> [<arg1> [<arg2> ..]]", 1) {
}

int ScriptExecHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	bool ok = sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer);
	if (!ok) {
		if (pld->zoneDef->mGrove == true
				&& pld->zoneDef->mAccountID != pld->accPtr->ID)
			ok = true;
	}
	if (!ok)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	ActiveInstance *inst = creatureInstance->actInst;
	size_t start = 0;
	bool queue = false;
	if (string(query->GetString(0)).compare("-q") == 0) {
		queue = true;
		start = 1;
	}
	if (start > query->argCount)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Missing arguments.");
	string funcName = query->GetString(start);
	if (inst->nutScriptPlayer != NULL) {
		vector<ScriptCore::ScriptParam> p;
		for (unsigned int i = 1; i < query->argCount; i++) {
			p.push_back(ScriptCore::ScriptParam(query->GetString(i)));
		}
		inst->nutScriptPlayer->mCaller = creatureInstance->CreatureID;
		inst->nutScriptPlayer->JumpToLabel(funcName.c_str(), p, queue);
		inst->nutScriptPlayer->mCaller = 0;
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

ScriptTimeHandler::ScriptTimeHandler() :
		AbstractCommandHandler("Usage: /script.time", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Debug);
}

int ScriptTimeHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL) {
		double seconds;
		if (inst->nutScriptPlayer != NULL) {
			seconds = (double) inst->nutScriptPlayer->mProcessingTime / 1000.0;
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"S Instance: %4.3f (%ul,%u,%ul). %s", seconds,
					inst->nutScriptPlayer->mInitTime,
					inst->nutScriptPlayer->mCalls,
					inst->nutScriptPlayer->mGCTime,
					inst->nutScriptPlayer->mActive ? "Active" : "Inactive");
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
		if (inst->scriptPlayer != NULL) {
			seconds = (double) inst->scriptPlayer->mProcessingTime / 1000.0;
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"T Instance: %4.3f. %s", seconds,
					inst->scriptPlayer->mActive ? "Active" : "Inactive");
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

		ActiveInstance::CREATURE_IT it;
		for (it = inst->NPCList.begin(); it != inst->NPCList.end(); ++it) {
			AINutPlayer *player = it->second.aiNut;
			if (player != NULL) {
				seconds = (double) player->mProcessingTime / 1000.0;
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"S CID: %d (%s) %4.3f (%ul,%u,%ul)", it->first,
						it->second.css.display_name, seconds, player->mInitTime,
						player->mCalls, player->mGCTime);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}

			AIScriptPlayer *tPlayer = it->second.aiScript;
			if (tPlayer != NULL) {
				seconds = (double) tPlayer->mProcessingTime / 1000.0;
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"T CID: %d (%s) %4.3f", it->first,
						it->second.css.display_name, seconds);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

ScriptWakeVMHandler::ScriptWakeVMHandler() :
		AbstractCommandHandler("Usage: /script.wakevm", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Debug);
}

int ScriptWakeVMHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL) {
		if (inst->nutScriptPlayer != NULL) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Woken VM for %s: %s",
					inst->nutScriptDef.mDescription.c_str(),
					inst->nutScriptPlayer->WakeVM("By User") ?
							"Woke OK" : "Did not wake");
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

		ActiveInstance::CREATURE_IT it;
		for (it = inst->NPCList.begin(); it != inst->NPCList.end(); ++it) {
			AINutPlayer *player = it->second.aiNut;
			if (player != NULL) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"CID: %d (%s) collected", it->first,
						it->second.css.display_name, player->GC());
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

ScriptGCHandler::ScriptGCHandler() :
		AbstractCommandHandler("Usage: /script.gc", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Debug);
}

int ScriptGCHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL) {
		if (inst->nutScriptPlayer != NULL) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Instance script collected %d objects",
					inst->nutScriptPlayer->GC());
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

		ActiveInstance::CREATURE_IT it;
		for (it = inst->NPCList.begin(); it != inst->NPCList.end(); ++it) {
			AINutPlayer *player = it->second.aiNut;
			if (player != NULL) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"CID: %d (%s) collected", it->first,
						it->second.css.display_name, player->GC());
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

ScriptClearQueueHandler::ScriptClearQueueHandler() :
		AbstractCommandHandler("Usage: /script.clearqueue", 0) {
	mAllowedPermissions.push_back(Permission_Admin | Permission_Debug);
}

int ScriptClearQueueHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL) {
		if (inst->nutScriptPlayer != NULL) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Instance script cleared of %d queue events",
					inst->nutScriptPlayer->ClearQueue());
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// ScriptExecHandler
//

RotHandler::RotHandler() :
		AbstractCommandHandler("Usage: /rot [<amount>]", 1) {
}

int RotHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	if (query->argCount > 0) {
		sim->SetRotation(query->GetInteger(0), 1);
	}
	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Rotation: %d",
			creatureInstance->Rotation);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PVPTeamHandler
//
PVPTeamHandler::PVPTeamHandler() :
		AbstractCommandHandler("Usage: /team [<team>]", 0) {
}

int PVPTeamHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount > 0) {

		if (pld->accPtr->HasPermission(Perm_Account, Permission_Sage) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

		CreatureInstance *targ = creatureInstance->CurrentTarget.targ;
		if (targ == NULL) {
			targ = creatureInstance;
		} else {
			if (targ->charPtr == NULL)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Must select a player.");
		}

		targ->css.pvp_team = PVP::PVPTeams::GetIDByName(query->GetString(0));
		if (targ->css.pvp_team == PVP::PVPTeams::NONE)
			if (targ == creatureInstance) {
				sim->SendInfoMessage("Your are now not in a PVP team.",
						INFOMSG_INFO);
			} else {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"%s is now not in PVP team.", targ->css.display_name,
						PVP::PVPTeams::GetNameByID(targ->css.pvp_team));
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		else {
			if (targ == creatureInstance) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"You are now in the %s team.",
						PVP::PVPTeams::GetNameByID(targ->css.pvp_team));
			} else {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"%s is now in the %s team.", targ->css.display_name,
						PVP::PVPTeams::GetNameByID(targ->css.pvp_team));
			}
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
	} else {
		if (creatureInstance->css.pvp_team == PVP::PVPTeams::NONE)
			sim->SendInfoMessage("Your are not in a PVP team.", INFOMSG_INFO);
		else {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"You are in the %s team.",
					PVP::PVPTeams::GetNameByID(creatureInstance->css.pvp_team));
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//PVPModeHandler
//
PVPModeHandler::PVPModeHandler() :
		AbstractCommandHandler("Usage: /mode [pvp|pve]", 1) {
}

int PVPModeHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount > 0) {
		if (creatureInstance->actInst->mZoneDefPtr->mArena == true)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Cannot change PVP mode when in arena.");

		if (creatureInstance->_HasStatusList(StatusEffects::IN_COMBAT) != -1)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Cannot change PVP mode when in combat.");

		string mode = query->GetString(0);
		if (mode.compare("pvp") == 0) {
			if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					!= PVP::GameMode::PVE_ONLY) {
				if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
						== -1)
					creatureInstance->_AddStatusList(StatusEffects::PVPABLE,
							-1);
			}
			if (creatureInstance->charPtr->Mode == PVP::GameMode::PVP) {
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Default for character is already PVP mode.");
			}
			creatureInstance->charPtr->Mode = PVP::GameMode::PVP;
			creatureInstance->charPtr->pendingChanges++;
			if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					== PVP::GameMode::PVE_ONLY)
				sim->SendInfoMessage(
						"Default is now PVP mode, but you are in a PVE only zone",
						INFOMSG_INFO);
			else
				sim->SendInfoMessage("Now in PVP mode", INFOMSG_INFO);
		} else if (mode.compare("pve") == 0) {
			if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					!= PVP::GameMode::PVP_ONLY
					&& creatureInstance->actInst->arenaRuleset.mPVPStatus
							!= PVP::GameMode::SPECIAL_EVENT) {
				if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
						!= -1)
					creatureInstance->_RemoveStatusList(StatusEffects::PVPABLE);
			}
			if (creatureInstance->charPtr->Mode == PVP::GameMode::PVE) {
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Default for character is already PVE mode.");
			}
			creatureInstance->charPtr->Mode = PVP::GameMode::PVE;
			creatureInstance->charPtr->pendingChanges++;
			if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					== PVP::GameMode::PVP_ONLY)
				sim->SendInfoMessage(
						"Default is now PVE mode, but you are in a PVP only zone so your status will not yet change.",
						INFOMSG_INFO);
			else if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					== PVP::GameMode::SPECIAL_EVENT)
				sim->SendInfoMessage(
						"Default is now PVE mode, but a special event is active so your status will not yet change.",
						INFOMSG_INFO);
			else
				sim->SendInfoMessage("Now in PVE mode", INFOMSG_INFO);
		} else {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid parameters.");
		}
	} else {
		if (creatureInstance->actInst->mZoneDefPtr->mArena == true)
			sim->SendInfoMessage("You are in an arena.", INFOMSG_INFO);
		else {
			if (creatureInstance->actInst->arenaRuleset.mPVPStatus
					== PVP::GameMode::SPECIAL_EVENT) {
				if (creatureInstance->charPtr->Mode == PVP::GameMode::PVE) {
					if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
							== -1) {
						sim->SendInfoMessage(
								"Your are currently in PVE mode and a special event is active.",
								INFOMSG_INFO);
					} else {
						sim->SendInfoMessage(
								"Your are currently in PVE mode during a special event, but have a PVP status.",
								INFOMSG_INFO);
					}
				} else {
					if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
							!= -1) {
						sim->SendInfoMessage(
								"Your are currently in PVP mode during a special event.",
								INFOMSG_INFO);
					} else {
						sim->SendInfoMessage(
								"Your are currently in PVP mode during a special event, but have no PVP status.",
								INFOMSG_INFO);
					}
				}
			} else {
				if (creatureInstance->charPtr->Mode == PVP::GameMode::PVE) {
					if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
							== -1) {
						sim->SendInfoMessage("Your are currently in PVE mode.",
								INFOMSG_INFO);
					} else {
						sim->SendInfoMessage(
								"Your are currently in PVE mode, but have a PVP status.",
								INFOMSG_INFO);
					}
				} else {
					if (creatureInstance->_HasStatusList(StatusEffects::PVPABLE)
							!= -1) {
						sim->SendInfoMessage("Your are currently in PVP mode.",
								INFOMSG_INFO);
					} else {
						sim->SendInfoMessage(
								"Your are currently in PVP mode, but have no PVP status.",
								INFOMSG_INFO);
					}
				}
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//InstanceHandler
//
InstanceHandler::InstanceHandler() :
		AbstractCommandHandler("Usage: /instance", 0) {
}

int InstanceHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	ActiveInstance *inst = creatureInstance->actInst;
	if (inst != NULL) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Total mobs killed: %d",
				inst->mKillCount);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

		if (inst->mZoneDefPtr->IsDungeon() == true
				|| sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Developer | Permission_Admin)
						== true) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Drop rate bonus: %gx", inst->mDropRateBonusMultiplier);
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

		if (inst->mZoneDefPtr->IsDungeon() == true) {
			if (inst->mOwnerPartyID > 0) {
				ActiveParty *party = g_PartyManager.GetPartyByID(
						inst->mOwnerPartyID);
				if (party != NULL) {
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"Dungeon party leader: %s",
							party->mLeaderName.c_str());
					sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				}
			} else {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Dungeon owner: %s", inst->mOwnerName.c_str());
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		}

		if (inst->scaleProfile != NULL) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "Dungeon scaler: %s",
					inst->scaleProfile->mDifficultyName.c_str());
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}

		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug | Permission_Admin | Permission_Developer)
				== true) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Drop rate profile: %s",
					inst->mZoneDefPtr->GetDropRateProfile().c_str());
			sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//UserAuthResetHandler
//
UserAuthResetHandler::UserAuthResetHandler() :
		AbstractCommandHandler("Usage: /user.auth.reset", 0) {
	mAllowedPermissions.push_back(Permission_Sage | Permission_Admin | Permission_Debug);
}

int UserAuthResetHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	string accName = query->GetString(0);
	AccountData * acc = g_AccountManager.FetchAccountByUsername(
			accName.c_str());
	if (acc == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such account.");
	}

	// Generate a new recovery key
	string newKey = Util::RandomStr(32, false);

	// Remove the existing registration key
	acc->RegKey = newKey;

//	// Generate a salt hash for the key
//	string convertedKey;
//	acc->GenerateSaltedHash(newKey.c_str(), convertedKey);

	// Build a new recovery key
	ConfigString str(acc->RecoveryKeys);
	str.SetKeyValue("regkey", "");
	str.GenerateString(acc->RecoveryKeys);

	Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
			"Give the player this key :-\n   %s\n.. and tell them to visit the password reset URL",
			newKey.c_str());
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);

	// Give player password reset permission
	acc->SetPermission(Perm_Account, "passwordreset", true);

	acc->PendingMinorUpdates++;

	g_Logs.event->info("[SAGE] User key and password reset for %v by %v",
			creatureInstance->css.display_name, accName.c_str());

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//MaintainHandler
//
MaintainHandler::MaintainHandler() :
		AbstractCommandHandler("Usage: /maintain", 0) {
	mAllowedPermissions.push_back(Permission_Admin);
}

int MaintainHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount > 0) {
		g_SimulatorManager.BroadcastChat(0, "System", "*SysChat", "The server is now in maintenance mode. Further logins by anyone other than Administrators or Sages will be denied.");
		g_Config.MaintenanceMessage = query->GetString(0);
	} else {
		g_SimulatorManager.BroadcastChat(0, "System", "*SysChat", "The server has now left maintenance mode. Anyone may login again");
		g_Config.MaintenanceMessage = "";
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//AchievementsHandler
//
AchievementsHandler::AchievementsHandler() :
		AbstractCommandHandler("Usage: /achievements", 0) {
}

int AchievementsHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (query->argCount >= 1) {
		if (!sim->CheckPermissionSimple(Perm_Account, Permission_Admin | Permission_Developer))
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

		if (query->args[0].compare("add") == 0) {
			if (query->argCount > 1) {
				pld->accPtr->AddAchievement(query->args[1]);
				pld->accPtr->PendingMinorUpdates++;
				int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"OK");
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%d:%d:%d:%d",
						g_AchievementsManager.GetTotalAchievements(),
						pld->accPtr->GetTotalCompletedAchievements(),
						g_AchievementsManager.GetTotalObjectives(),
						pld->accPtr->GetTotalAchievementObjectives());
				wpos += PrepExt_Achievement(&sim->SendBuf[wpos],
						creatureInstance->CreatureID, query->args[1].c_str(),
						sim->Aux1);
				return wpos;
			} else
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Add command requires fully qualified name of achievement.");
		}
	} else {
		for (map<string, Achievements::Achievement>::iterator it =
				pld->accPtr->Achievements.begin();
				it != pld->accPtr->Achievements.end(); ++it) {
			Achievements::Achievement a = it->second;
			for (vector<Achievements::AchievementObjectiveDef*>::iterator ait =
					a.mCompletedObjectives.begin();
					ait != a.mCompletedObjectives.end(); ++ait) {
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%s/%s",
						a.mDef->mName.c_str(), (*ait)->mName.c_str());
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
			}
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
