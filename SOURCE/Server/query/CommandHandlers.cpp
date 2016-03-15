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
#include "Instance.h"
#include "../ChatChannel.h"
#include "../Crafting.h"
#include "../Instance.h"
#include "../Character.h"
#include "../Account.h"
#include "../InstanceScale.h"
#include "../Config.h"
#include "../Scenery2.h"
#include "../IGForum.h"
#include "../Ability2.h"
#include "../Fun.h"

//
//AbstractCommandHandler
//
AbstractCommandHandler::AbstractCommandHandler(std::string usage,
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
	mAllowedPermissions.push_back(Permission_Sage);
	mAllowedPermissions.push_back(Permission_Admin);
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
	sim->AddMessage((long) creatureInstance, 0, BCM_UpdateCreatureInstance);

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
	mAllowedPermissions.push_back(Permission_Debug);
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
	sim->AddMessage(pld->CreatureID, amount, BCM_SendHealth);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//SpeedHandler
//
SpeedHandler::SpeedHandler() :
		AbstractCommandHandler("Usage: /speed <bonusAmount>", 1) {
	mAllowedPermissions.push_back(Permission_Debug);
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
	sim->AddMessage((long) creatureInstance, 0, BCM_UpdateCreatureInstance);
	sim->SendInfoMessage("Speed set.", INFOMSG_INFO);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ForceAbilityHandler
//
ForceAbilityHandler::ForceAbilityHandler() :
		AbstractCommandHandler("Usage: /fa <ability>", 1) {
	mAllowedPermissions.push_back(Permission_Debug);
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
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Admin)
				== false) {
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
		//sim->AddMessage((long)creatureInstance, abID, BCM_AbilityRequest);
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

	bool debug = sim->CheckPermissionSimple(Perm_Account, Permission_Debug);
	SIMULATOR_IT it;
	for (it = Simulator.begin(); it != Simulator.end(); ++it) {
		if (it->isConnected == true && it->ProtocolState == 1
				&& it->LoadStage == sim->LOADSTAGE_GAMEPLAY) {
			if (it->IsGMInvisible() == true) //Hide GM Invisibility from the list.
				continue;
			CharacterData *cd = it->pld.charPtr;
			ZoneDefInfo *zd = g_ZoneDefManager.GetPointerByID(
					it->pld.CurrentZoneID);
			if (cd != NULL && zd != NULL) {
				if (debug == true)
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"%s (%d, %d) %s (%s)", cd->cdef.css.display_name,
							it->creatureInst->CurrentX,
							it->creatureInst->CurrentZ, zd->mName.c_str(),
							zd->mShardName.c_str());
				else
					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"%s (%s.%d) %s (%s)", cd->cdef.css.display_name,
							Professions::GetAbbreviation(
									cd->cdef.css.profession),
							cd->cdef.css.level, zd->mName.c_str(),
							zd->mShardName.c_str());

				WritePos += PrepExt_SendInfoMessage(&sim->SendBuf[WritePos],
						sim->Aux1, INFOMSG_INFO);
				sim->CheckWriteFlush(WritePos);
			}
		}
	}
	WritePos += PrepExt_QueryResponseString(&sim->SendBuf[WritePos], query->ID,
			"OK");
	return WritePos;
}
//
//GMWhoHandler
//
GMWhoHandler::GMWhoHandler() :
		AbstractCommandHandler("Usage: /gmwho", 0) {
	mAllowedPermissions.push_back(Permission_Sage);
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
					cd->cdef.css.display_name, it->pld.accPtr->Name,
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
//GiveHandler
//
GiveHandler::GiveHandler() :
		AbstractCommandHandler("Usage: /give \"<itemName>\"", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive);
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
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//GiveHandler
//
GiveIDHandler::GiveIDHandler() :
		AbstractCommandHandler("Usage: /giveid <itemId> [<count>]", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive);
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
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}
//
//GiveAllHandler
//
GiveAllHandler::GiveAllHandler() :
		AbstractCommandHandler("Usage: /giveall \"<searchName>\"", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive);
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
	std::vector<ItemDef*> resultList;
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
	mAllowedPermissions.push_back(Permission_ItemGive);
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
	std::vector<ItemDef*> resultList;
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
	mAllowedPermissions.push_back(Permission_ItemGive);
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

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DeleteAboveHandler
//
DeleteAboveHandler::DeleteAboveHandler() :
		AbstractCommandHandler("Usage: /deleteabove <slotNumber>", 1) {
	mAllowedPermissions.push_back(Permission_ItemGive);
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
	std::vector<std::string> groveList;

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
	std::vector<std::string> zoneList;
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
	mAllowedPermissions.push_back(Permission_Admin);
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
	int rval = sim->protected_helper_query_item_morph(true);
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
	mAllowedPermissions.push_back(Permission_Debug);
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
		sim->AddMessage((long) creatureInstance, 0, BCM_UpdateAppearance);
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//GodHandler
//
GodHandler::GodHandler() :
		AbstractCommandHandler("Usage: /god [<0|1>]", 0) {
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Admin);
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

	const char *statName = query->args[0].c_str();
	const char *statValue = query->args[1].c_str();
	WriteStatToSetByName(statName, statValue, &targ->css);
	if (targ == creatureInstance)
		WriteStatToSetByName(statName, statValue, &pld->charPtr->cdef.css);
	sim->AddMessage((long) targ, 0, BCM_UpdateCreatureInstance);
	sim->AddMessage((long) &pld->charPtr->cdef, 0, BCM_UpdateCreatureDef);

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
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Admin);
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
					it->pld.accPtr->Name);
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
	mAllowedPermissions.push_back(Permission_Admin);
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
	mAllowedPermissions.push_back(Permission_Admin);
	mAllowedPermissions.push_back(Permission_Sage);
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
		for (std::vector<BuildPermissionArea>::iterator it =
				accPtr->BuildPermissionList.begin();
				it != accPtr->BuildPermissionList.end(); ++it) {
			BuildPermissionArea pa = *it;
			if (pa.ZoneID == zoneID) {
				accPtr->BuildPermissionList.erase(it);
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
						"Removed permission for %s in zone %d (%d,%d,%d,%d)",
						accPtr->Name, zoneID, pa.x1, pa.y1, pa.x2, pa.y2);
				sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
				found = true;
				break;
			}
		}
		if (!found) {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Could not find permission for %s in zone %d", accPtr->Name,
					zoneID);
			sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
		}
	} else {
		// Add / update
		bool found = false;
		for (std::vector<BuildPermissionArea>::iterator it =
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
						accPtr->Name, pa.ZoneID, pa.x1, pa.y1, pa.x2, pa.y2);
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
					accPtr->Name, pa.ZoneID, pa.x1, pa.y1, pa.y1, pa.y2);
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
	mAllowedPermissions.push_back(Permission_Admin);
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
//SetBehaviorHandler
//
SetBehaviorHandler::SetBehaviorHandler() :
		AbstractCommandHandler(
				"Usage: /setbuildpermissions <accountName> <instanceId> [<x1>,<y1>,<x2>,<y2>]",
				2) {
	mAllowedPermissions.push_back(Permission_Admin);
}

int SetBehaviorHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//[0] = flag bit value
	//[1] = state

	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Usage: /setbehavior bitvalue state");

	int value = atoi(query->args[0].c_str());
	int state = atoi(query->args[1].c_str());
	g_Config.SetAdministrativeBehaviorFlag(value, (state != 0) ? true : false);
	sprintf(sim->Aux1, "BehaviorFlag is now: %lu",
			g_Config.debugAdministrativeBehaviorFlags);
	sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//DeriveSetHandler
//
DeriveSetHandler::DeriveSetHandler() :
		AbstractCommandHandler("Usage: /deriveset", 0) {
	mAllowedPermissions.push_back(Permission_Sage);
}

int DeriveSetHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (creatureInstance->CurrentTarget.targ == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Must select a target.");

	std::string eqApp = creatureInstance->CurrentTarget.targ->css.eq_appearance;

	unsigned int pos = 0;
	while (pos != string::npos) {
		pos = eqApp.find_first_of("{}[]");
		if (pos != string::npos)
			eqApp.erase(pos, 1);
	}

	STRINGLIST items;
	STRINGLIST itementry;
	std::vector<int> itemIDList;
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
	mAllowedPermissions.push_back(Permission_Admin);
}

int IGStatusHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int EffectID = GetStatusIDByName(query->args[0].c_str());
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
	mAllowedPermissions.push_back(Permission_Admin);
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
	std::string charName = query->GetString(0);
	query->name = "party";
	query->args.clear();
	query->args.push_back("invite");
	query->args.push_back(charName);
	query->argCount = query->args.size();

	return sim->handle_query_party();
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
		int roll = randint(1, 100);
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "You rolled %d", roll);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	} else if (party != NULL) {
		std::vector<int> rolls;
		for (size_t mi = 0; mi < party->mMemberList.size(); mi++) {
			bool has = false;
			int tries = 0;
			int roll = 0;
			do {
				has = false;
				roll = randint(1, 100);
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
		AbstractCommandHandler("Usage: /zonename <0|1> <name>", 2) {
	;
	mAllowedPermissions.push_back(Permission_Admin);
}

int ZoneNameHandler::handleCommand(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount >= 2) {
		int t = atoi(query->args[0].c_str());
		const char *name = query->args[1].c_str();
		ZoneDefInfo *zoneDef = g_ZoneDefManager.GetPointerByID(
				pld->CurrentZoneID);
		if (zoneDef != NULL) {
			if (zoneDef->mGrove == true) {
				if (t == 1)
					zoneDef->ChangeShardName(name);
				else
					zoneDef->ChangeName(name);
				g_ZoneDefManager.NotifyConfigurationChange();
				sim->SendSetMap();
				sim->SendInfoMessage(pld->zoneDef->mShardName.c_str(),
						INFOMSG_SHARD);
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
					int dummy = 0;
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
		std::vector<SceneryObject*> search;
		g_SceneryManager.GetThread("SimulatorThread::handle_command_dtrig");
		g_SceneryManager.EnumPropsInRange(pld->CurrentZoneID,
				creatureInstance->CurrentX, creatureInstance->CurrentZ,
				searchlen, search);
		std::map<std::string, int> pkgmap;
		std::map<std::string, int>::iterator it;
		for (size_t i = 0; i < search.size(); i++) {
			if (strstr(search[i]->Asset, "SpawnPoint") == NULL)
				continue;
			if (search[i]->extraData == NULL)
				continue;
			pkgmap[search[i]->extraData->spawnPackage]++;
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
			for (size_t i = 0; i <= 62; i++)
				if (ptr->HasStatus(i))
					g_Logs.simulator->info("Status:%v", GetStatusNameByID(i));
		}
	}
		break;
	case 24:
		g_ActiveInstanceManager.DebugFlushInactiveInstances();
		break;
	case 50:
		if (query->argCount > 1)
			g_Config.AprilFools = query->GetInteger(1);
		break;
	case 51:
		if (query->argCount > 1)
			g_Config.AprilFoolsName = query->GetString(1);
		break;
	case 52:
		if (query->argCount > 1)
			g_Config.AprilFoolsAccount = query->GetInteger(1);
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
	case 900: {
		int x1 = query->GetInteger(1) / pld->zoneDef->mPageSize;
		int y1 = query->GetInteger(2) / pld->zoneDef->mPageSize;
		int x2 = query->GetInteger(3) / pld->zoneDef->mPageSize;
		int y2 = query->GetInteger(4) / pld->zoneDef->mPageSize;
		Platform::MakeDirectory("SceneryCopy");
		for (int y = y1; y <= y2; y++) {
			for (int x = x1; x <= x2; x++) {
				sprintf(sim->Aux1, "Scenery\\%d\\x%03dy%03d.txt",
						pld->zoneDef->mID, x, y);
				sprintf(sim->Aux2, "SceneryCopy\\x%03dy%03d.txt", x, y);
				Platform::FixPaths(sim->Aux1);
				Platform::FixPaths(sim->Aux2);
				Platform::FileCopy(sim->Aux1, sim->Aux2);
			}
		}
	}
		break;
	case 901:  //Poke grove index entry
	{
		if (query->argCount <= 4)
			break;
		int zoneID = query->GetInteger(1);
		int accountID = query->GetInteger(2);
		const char *warpName = query->GetString(3);
		const char *groveName = query->GetString(4);
		bool allowCreate = false;
		if (query->argCount >= 5)
			allowCreate = (query->GetInteger(5) == sim->InternalID);
		if (allowCreate == false)
			sim->SendInfoMessage(
					"To create an entry, provide Simulator ID to confirm.",
					INFOMSG_INFO);

		g_ZoneDefManager.UpdateZoneIndex(zoneID, accountID, warpName, groveName,
				allowCreate);
		sim->SendInfoMessage("Updated.", INFOMSG_INFO);
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
					nso->SetName("IMPORT");
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
	mAllowedPermissions.push_back(Permission_Admin);
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
		int cdef = g_AccountManager.GetCDefFromCharacterName(
				query->GetString(1));
		if (cdef == -1) {
			sim->SendInfoMessage("Character not found.", INFOMSG_ERROR);
			code = 0;
		} else
			pld->zoneDef->AddPlayerFilterID(cdef);
	} else if (query->args[0].compare("filterremove") == 0) {
		int cdef = g_AccountManager.GetCDefFromCharacterName(
				query->GetString(1));
		if (cdef == -1) {
			sim->SendInfoMessage("Character not found.", INFOMSG_ERROR);
			code = 0;
		} else
			pld->zoneDef->RemovePlayerFilter(cdef);
	} else if (query->args[0].compare("filterclear") == 0) {
		pld->zoneDef->ClearPlayerFilter();
	} else if (query->args[0].compare("filterlist") == 0) {
		const std::vector<int> &IDs = pld->zoneDef->mPlayerFilterID;
		for (size_t i = 0; i < IDs.size(); i++) {
			const char *name = g_AccountManager.GetCharacterNameFromCDef(
					IDs[i]);
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
	std::string outputMsg;
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

	std::vector<SceneryObject*> results;
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
		if (so->extraData == NULL)
			continue;
		int linkChanges = 0;
		for (int li = 0; li < so->extraData->linkCount; li++) {
			if (so->extraData->link[li].type
					== SceneryObject::LINK_TYPE_LOYALTY) {
				so->extraData->link[li].type = SceneryObject::LINK_TYPE_PATH;
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
	mAllowedPermissions.push_back(Permission_Debug);
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

	//AddMessage((long)creatureInst, 10006, BCM_AbilityRequest);
	creatureInstance->RequestAbilityActivation(10006);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ElevHandler
//
ElevHandler::ElevHandler() :
		AbstractCommandHandler("Usage: /elev <y>", 0) {
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Debug);
}

int CycleHandler::handleCommand(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	g_EnvironmentCycleManager.EndCurrentCycle();
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
//
//SearSizeHandler
//
SearSizeHandler::SearSizeHandler() :
		AbstractCommandHandler("Usage: /searsize [<size>]", 0) {
	mAllowedPermissions.push_back(Permission_Sage);
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
	mAllowedPermissions.push_back(Permission_Debug);
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
	mAllowedPermissions.push_back(Permission_Debug);
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
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
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
			std::vector<std::string> args;
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
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Permission denied.");

	const char *warpTarg = query->args[0].c_str();
	ZoneDefInfo *targZone = g_ZoneDefManager.GetPointerByPartialWarpName(
			warpTarg);
	if (targZone == NULL) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Zone name not found: %s", warpTarg);
		g_Logs.simulator->error("[%v] %v", sim->InternalID, sim->Aux1);
		sim->SendInfoMessage(sim->Aux1, INFOMSG_ERROR);
	} else {
		int errCode = sim->CheckValidWarpZone(targZone->mID);
		if (errCode != sim->ERROR_NONE)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					sim->GetGenericErrorString(errCode));

		//If the avatar is running, it will glitch the position.

		// EM - Is this really needed?
		//		SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);

		if (sim->ProtectedSetZone(targZone->mID, 0) == false) {
			sim->ForceErrorMessage("Critical error while changing zones.",
					INFOMSG_ERROR);
			sim->Disconnect("SimulatorThread::handle_command_warpi");
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Critical error.");
		}
		sim->SetPosition(targZone->DefX, targZone->DefY, targZone->DefZ, 1);
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
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
		if (sim->CheckPermissionSimple(Perm_Account, Permission_Debug) == false)
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
	mAllowedPermissions.push_back(Permission_Sage);
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

	sim->AddMessage((long) targ, 0, BCM_UpdatePosition);
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
	mAllowedPermissions.push_back(Permission_Sage);
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
	int CDef = g_AccountManager.GetCDefFromCharacterName(
			query->args[0].c_str());
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
	mAllowedPermissions.push_back(Permission_Sage);
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

