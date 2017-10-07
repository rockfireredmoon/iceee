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

#include "ItemHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Debug.h"
#include "../Config.h"
#include "../Crafting.h"
#include "../ConfigString.h"
#include "../Ability2.h"
#include "../util/Log.h"

void AddPet(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance, int CDefID) {
	int exist = pld->charPtr->CountSidekick(SidekickObject::PET);
	if (exist > 0) {
		creatureInstance->actInst->SidekickRemoveAll(creatureInstance,
				&pld->charPtr->SidekickList);
		return;
	}

	SidekickObject skobj(CDefID);
	skobj.summonType = SidekickObject::PET;

	pld->charPtr->AddSidekick(skobj);
	int r = creatureInstance->actInst->CreateSidekick(creatureInstance, skobj);
	if (r == -1) {
		sim->SendInfoMessage("Server error: Invalid Creature ID for sidekick.",
				INFOMSG_ERROR);
		return;
	}
}

int UseItem(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance,
		unsigned int CCSID) {

	g_Logs.simulator->debug("[%v] Uses item %v", sim->InternalID, CCSID);

	if (creatureInstance->activeLootID != 0) {
		sim->SendInfoMessage("You cannot use items while trading.",
				INFOMSG_ERROR);
		return -1;
	}

	InventorySlot *slot = pld->charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if (slot == NULL) {
		sim->SendInfoMessage("Item not found in inventory.", INFOMSG_ERROR);
		return -1;
	}

	ItemDef *itemDef = slot->ResolveItemPtr();
	if (itemDef == NULL) {
		sim->SendInfoMessage("Item not found in database.", INFOMSG_ERROR);
		return -1;
	}

	if (creatureInstance->css.level < itemDef->mMinUseLevel) {
		sim->SendInfoMessage("Your level is too low.", INFOMSG_ERROR);
		return -1;
	}

	if (itemDef->mType != ItemType::CONSUMABLE
			&& itemDef->mType != ItemType::SPECIAL)
		return -1;

	// The AddSidekick() function has its own thread guard, so we don't
	// need one here.
	bool removeOnUse = true;
	if (itemDef->mUseAbilityId != 0) {
		ConfigString cfg(itemDef->Params);
		//AddMessage((long)creatureInst, itemDef->mUseAbilityId, BCM_AbilityRequest);
		int r = creatureInstance->RequestAbilityActivation(
				itemDef->mUseAbilityId);
		if (r != Ability2::ABILITY_SUCCESS) {
			removeOnUse = false;
			return -1;
		} else {
			if (creatureInstance->ab[0].type == AbilityType::Cast) {
				pld->mItemUseInProgress = true;
				pld->mItemUseCCSID = CCSID;
				removeOnUse = false; //We'll remove it later.  Look for code uses of RunFinishedCast().
			} else {

				int keep = cfg.GetValueInt("keep");
				if (keep != 0) {
					removeOnUse = false;
				}

				pld->mItemUseInProgress = false;
				pld->mItemUseCCSID = 0;
			}
		}
	} else {
		if (itemDef->mType == ItemType::SPECIAL
				&& itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE) {
			g_Logs.simulator->debug("Opening book %v on page %v",
					itemDef->mIvMax1, itemDef->mIvMax2);
			return PrepExt_SendBookOpen(sim->SendBuf, itemDef->mIvMax1,
					itemDef->mIvMax2 - 1, 1);
		} else {
			ConfigString cfg(itemDef->Params);
			int petSpawnID = cfg.GetValueInt("pet");
			if (petSpawnID != 0) {
				AddPet(sim, pld, query, creatureInstance, petSpawnID);
				removeOnUse = false;
			} else {
				int credits = cfg.GetValueInt("credits");
				int abPoints = cfg.GetValueInt("abilitypoints");
				if (credits > 0) {
					if (g_Config.AccountCredits) {
						creatureInstance->css.credits = pld->accPtr->Credits;
					}
					creatureInstance->css.credits += credits;
					if (g_Config.AccountCredits) {
						pld->accPtr->Credits = creatureInstance->css.credits;
						pld->accPtr->PendingMinorUpdates++;
					}

					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
							"You gain %d credits.", credits);
					sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
					creatureInstance->SendStatUpdate(STAT::CREDITS);
				}
				if (abPoints > 0) {
					creatureInstance->css.current_ability_points += abPoints;
					creatureInstance->css.total_ability_points += abPoints;
					pld->charPtr->AddAbilityPoints(abPoints);
					creatureInstance->SendStatUpdate(
							STAT::CURRENT_ABILITY_POINTS);
					creatureInstance->SendStatUpdate(
							STAT::TOTAL_ABILITY_POINTS);
					//We don't need to send a text notification for ability points, the client will notify the user.
				}
				removeOnUse = true;
			}
		}
	}

	/*
	 else if(itemDef->mActionAbilityId != 0)
	 {
	 if(itemDef->mActionAbilityId == RESERVED_ABILITY_SUMMON)
	 {
	 ArbitraryKeyValueData adat;
	 adat.Assign(itemDef->Params);
	 int petSpawnID = adat.GetValueInt("pet");
	 if(petSpawnID != 0)
	 {
	 AddPet(petSpawnID);
	 removeOnUse = false;
	 }
	 else
	 {
	 if(AddSidekick(itemDef->mActionAbilityId) == -1)
	 removeOnUse = false;
	 }
	 }
	 else if(itemDef->mActionAbilityId == RESERVED_ABILITY_BONUS)
	 {
	 ArbitraryKeyValueData adat;
	 adat.Assign(itemDef->Params);
	 int credits = adat.GetValueInt("credits");
	 int abPoints = adat.GetValueInt("abilitypoints");
	 if(credits > 0)
	 {
	 creatureInst->css.credits += credits;
	 Util::SafeFormat(Aux1, sizeof(Aux1), "You gain %d credits.", credits);
	 SendInfoMessage(Aux1, INFOMSG_INFO);
	 creatureInst->SendStatUpdate(STAT::CREDITS);
	 }
	 if(abPoints > 0)
	 {
	 creatureInst->css.current_ability_points += abPoints;
	 creatureInst->css.total_ability_points += abPoints;
	 pld.charPtr->AddAbilityPoints(abPoints);
	 creatureInst->SendStatUpdate(STAT::CURRENT_ABILITY_POINTS);
	 creatureInst->SendStatUpdate(STAT::TOTAL_ABILITY_POINTS);
	 //We don't need to send a text notification for ability points, the client will notify the user.
	 }
	 removeOnUse = true;
	 }
	 }
	 else
	 {
	 LogMessageL(MSG_SHOW, "Checking pet");
	 ArbitraryKeyValueData adat;
	 adat.Assign(itemDef->Params);
	 int petSpawnID = adat.GetValueInt("pet");
	 if(petSpawnID != 0)
	 {
	 LogMessageL(MSG_SHOW, "Adding pet");
	 AddPet(petSpawnID);
	 removeOnUse = false;
	 }
	 }
	 */

	if (removeOnUse == false)  //No need for the rest, leave early.
		return 0;

	//If it's stackable, need to remove an object from the stack,
	//and delete the item if there are no more.
	sim->DecrementStack(slot);

	return 0;
}

//
//ItemUseHandler
//

int ItemUseHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/*  Query: item.use
	 Attempt to use an item.  Sent when an object was double-clicked
	 in the inventory.
	 Args : [0] = Hex identifier of the inventory item.
	 */
	int wpos = 0;
	if (query->argCount > 0) {
		unsigned int CCSID = pld->charPtr->inventory.GetCCSIDFromHexID(
				query->GetString(0));
		int ret = UseItem(sim, pld, query, creatureInstance, CCSID);
		if (ret > 0) {
			wpos += ret;
		}
	}
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//ItemDefUseHandler
//

int ItemDefUseHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: item.def.use
	 Attempt to use an item.  Sent when an item is used from the quickbar.
	 Args : [0] = Item Def ID
	 [1] = Creature/%d (Avatar)        Selected target.
	 */

	if (query->argCount > 0) {
		int itemID = query->GetInteger(0);
		InventorySlot *item = pld->charPtr->inventory.GetItemPtrByID(itemID);
		if (item != NULL) {
			UseItem(sim, pld, query, creatureInstance, item->CCSID);
		}
	}
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//ItemContentsHandler
//

int ItemContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: item.contents
	 Args : container name to retrieve
	 */

	if (sim->HasQueryArgs(1) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	const char *contName = query->args[0].c_str();

	int contID = GetContainerIDFromName(contName);
	if (contID == -1) {
		g_Logs.simulator->warn("[%v] Invalid [item.contents] container: [%v]",
				sim->InternalID, contName);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid item container.");
	}

	InventoryManager inv = pld->charPtr->inventory;

	if (contID == DELIVERY_CONTAINER) {
		inv = pld->accPtr->inventory;
	}

	sim->SendInventoryData(inv.containerList[contID]);

	int WritePos = 0;

	WritePos += PutByte(&sim->SendBuf[WritePos], 1);     //_handleQueryResultMsg
	WritePos += PutShort(&sim->SendBuf[WritePos], 0);      //Message size
	WritePos += PutInteger(&sim->SendBuf[WritePos], query->ID); //Query response index

	sprintf(sim->Aux2, "%d", contID);
	sprintf(sim->Aux3, "%d", (int) inv.containerList[contID].size());
	WritePos += PutShort(&sim->SendBuf[WritePos], 1);      //Array count
	WritePos += PutByte(&sim->SendBuf[WritePos], 2);       //String count
	WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux2);  //ID
	WritePos += PutStringUTF(&sim->SendBuf[WritePos], sim->Aux3);  //Count
	PutShort(&sim->SendBuf[1], WritePos - 3);             //Set message size

	return WritePos;
}

//
//ItemMoveHandler
//

int ItemMoveHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	/*
	 --- Some samples of server queries for different operations:
	 Generally it follows this format:
	 [0] = CCSID        Hex string formed by the Container ID and Container Slot,
	 which can be extracted by bitmasks.
	 [1] = inv          Target container name.
	 [2] = 23           Target container slot.
	 [3] = inventory    Current container name, varies depending on move operation.
	 [4] = -1           Current container slot, varies depending on the container name
	 and operation.

	 When moving from an equipped slot to backpack slot:
	 [0] = CCSID
	 [1] = inv          (target container)
	 [2] = 23           (target slot)
	 [3] = inventory
	 [4] = -1

	 When moving from backpack slot to an equipped slot:
	 [0] = CCSID
	 [1] = eq           (target container)
	 [2] = 3            (target slot, where slots are mapped to specific attachment points)
	 [3] = inventory    (current container)
	 [4] = 23           (current index)

	 When moving between backpack slots:
	 [0] = CCSID
	 [1] = inv          (target container)
	 [2] = 30           (target slot)
	 [3] = inventory    (current container)
	 [4] = 31           (current slot)

	 When moving a weapon from backpack to equipped slot:
	 [0] = 10002
	 [1] = eq           (target container)
	 [2] = 2            (target slot)
	 [3] = eq_ranged    (unknown)
	 [4] = -1           (unknown)

	 When moving a weapon from equipped slot to backpack:
	 [0] = 10002
	 [1] = inv          (target container)
	 [2] = 42           (target slot)
	 [3] = eq_ranged    (unknown)
	 [4] = 0            (unknown)

	 */

	/* Query: item.move
	 Args : [0] = Item ID (in hexadecimal)
	 [1] = Name of the destination container.
	 [2] = Destination slot.
	 [3] = Name of the source container (previous item location).
	 [4] = Source slot.
	 */

	//if(HasQueryArgs(5) == false)
	//	return;
	if (query->argCount < 5)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query->");

	/* Query debug output
	 for(int i = 0; i < query->argCount; i++)
	 LogMessageL(MSG_SHOW, "[%d]=%s", i, query->args[i].c_str());
	 */

	unsigned long CCSID = strtol(query->args[0].c_str(), NULL, 16);

	int origContainer = (CCSID & CONTAINER_ID) >> 16;
	int origSlot = CCSID & CONTAINER_SLOT;

	int destContainer = GetContainerIDFromName(query->args[1].c_str());
	int destSlot = strtol(query->args[2].c_str(), NULL, 10);

	if (destContainer == -1) {
		g_Logs.simulator->error(
				"[%d] item.move: unknown destination container [%s] for CCSID [%lu]",
				sim->InternalID, query->args[0].c_str(), CCSID);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown destination container.");
	}

	if (sim->CanMoveItems() == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You may not move items while busy.");

	//This is a pretty ugly hack to prevent some out of sync and potential duping issues.
	//If the player submits two items to trade, then removes one item from trade by dragging the
	//item and dropping it back into the inventory over the other item, it will swap the items and
	//allow that item to be submitted for trade twice, thus duping.
	int selfID = creatureInstance->CreatureID;
	int tradeID = creatureInstance->activeLootID;
	TradeTransaction *tradeData =
			creatureInstance->actInst->tradesys.GetExistingTransaction(tradeID);
	if (tradeData != NULL) {
		creatureInstance->actInst->tradesys.CancelTransaction(selfID, tradeID,
				sim->SendBuf);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You may not move items while trading.");
	}
	//End of hack.

	//Hack to enforce class restrictions for weapons.
	/* DEACTIVATED SINCE IT'S NOT SYNCHRONIZED WITH THE CLIENT
	 if(destContainer == EQ_CONTAINER)
	 {
	 int origItemIndex = GetItemBySlot(origContainer, origSlot);
	 if(origItemIndex >= 0)
	 {
	 InventorySlot &source = containerList[origContainer][origItemIndex];
	 ItemDef *sourceItem = source.ResolveSafeItemPtr();
	 if(sourceItem->mWeaponType != 0)
	 if(creatureInstance->ValidateEquippableWeapon(sourceItem->mWeaponType) == false)
	 return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "You cannot equip weapons of that type.");
	 }
	 }
	 */

	//Need this or else accounts with debug permissions (multiple simultaneous logins per account)
	//will overwrite each other's vault contents. (Originally intended for shared vaults, which
	//implementation was never finished, but might as well keep)
	if (((destContainer == BANK_CONTAINER) || (origContainer == BANK_CONTAINER))
			&& (pld->accPtr->GetSessionLoginCount() > 1))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot use vaults while logged into multiple account characters at once.");

	if (((destContainer == DELIVERY_CONTAINER)
			|| (origContainer == DELIVERY_CONTAINER))
			&& (pld->accPtr->GetSessionLoginCount() > 1))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot use delivery boxes while logged into multiple account characters at once.");

	if (((destContainer == STAMPS_CONTAINER)
			|| (origContainer == STAMPS_CONTAINER))
			&& (pld->accPtr->GetSessionLoginCount() > 1))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot use stamp boxes while logged into multiple account characters at once.");

	InventoryManager *origInv = &pld->charPtr->inventory;
	InventoryManager *destInv = &pld->charPtr->inventory;

	if (destContainer == DELIVERY_CONTAINER) {
		destInv = &pld->accPtr->inventory;
	}

	if (origContainer == DELIVERY_CONTAINER) {
		origInv = &pld->accPtr->inventory;
	}

	if ((origInv->VerifyContainerSlotBoundary(origContainer, origSlot) == false)
			|| (destInv->VerifyContainerSlotBoundary(destContainer, destSlot)
					== false)) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Failed to move item.");
	}

	if (destContainer == STAMPS_CONTAINER) {
		int origItemIndex = origInv->GetItemBySlot(origContainer, origSlot);
		if (origItemIndex < 0)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid item.");

		InventorySlot &source =
				origInv->containerList[origContainer][origItemIndex];
		ItemDef *sourceItem = source.ResolveItemPtr();
		if (sourceItem == NULL || sourceItem->mID != POSTAGE_STAMP_ITEM_ID) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"You may only add stamps to a stamp container.");
		}
	}

	if (destContainer == EQ_CONTAINER) {
		int origItemIndex = origInv->GetItemBySlot(origContainer, origSlot);
		if (origItemIndex < 0)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid item.");

		InventorySlot &source =
				origInv->containerList[origContainer][origItemIndex];
		ItemDef *sourceItem = source.ResolveItemPtr();
		int res = destInv->VerifyEquipItem(sourceItem, destSlot,
				creatureInstance->css.level, creatureInstance->css.profession);
		if (res != InventoryManager::EQ_ERROR_NONE)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					InventoryManager::GetEqErrorString(res));
	}

	int mpos = origInv->ItemMove(sim->Aux1, sim->Aux3, &creatureInstance->css,
			pld->charPtr->localCharacterVault, origContainer, origSlot, destInv,
			destContainer, destSlot, true);
	if (mpos > 0) {
		if (destContainer == DELIVERY_CONTAINER
				|| origContainer == DELIVERY_CONTAINER) {
			pld->accPtr->PendingMinorUpdates++;
		}

		sim->AttemptSend(sim->Aux1, mpos);
		//Send the query response after the item updates.
		int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
		sim->AttemptSend(sim->SendBuf, wpos);

		if (origContainer == EQ_CONTAINER || destContainer == EQ_CONTAINER) {
			//Just in case a container was equipped or unequipped, update the max inventory slots
			origInv->CountInventorySlots();
			if (origInv != destInv) {
				destInv->CountInventorySlots();
			}

			//Update the eq_appearance string and broadcast an update to all other players
			sim->UpdateEqAppearance();
		}

		/* DISABLED, NEVER FINISHED
		 //Check quest objectives.
		 if(origContainer == INV_CONTAINER || destContainer == INV_CONTAINER)
		 {
		 wpos = pld->charPtr->questJournal.RefreshItemQuests(pld->charPtr->inventory, sim->SendBuf);
		 if(wpos > 0)
		 AttemptSend(sim->SendBuf, wpos);
		 }
		 */
		return 0;
	}

	//Ugly error code handling.
	if (mpos == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot equip a two-handed weapon while using an off-hand item.");
	else if (mpos == -2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot equip an offhand item while using a two-handed weapon.");
	else if (mpos == -3)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK"); //Hack to allow movement when the origin and destination is the same.  The request must return successful, otherwise the client will spam retry requests.
	else if (mpos == -4) {
		if (destContainer == DELIVERY_CONTAINER) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Items bound to you cannot be placed into your delivery box.");
		} else if (destContainer == AUCTION_CONTAINER) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Items bound to you cannot be auctioned.");
		} else {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Items bound to you cannot be placed into your vault.");
		}
	} else if (mpos < -100)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				InventoryManager::GetEqErrorString(mpos + 100));

	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Failed to move item.");
}

//
//ItemSplitHandler
//

int ItemSplitHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (sim->HasQueryArgs(2) == false)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	long CCSID = strtol(query->args[0].c_str(), NULL, 16);
	int amount = atoi(query->args[1].c_str());
	InventorySlot *item = pld->charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if (item == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item not found in inventory.");
	}

	int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No free inventory space.");
	}

	int currentCount = item->GetStackCount();
	if (amount < 1 || amount >= currentCount) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot split that amount.");
	}

	InventorySlot newItem;
	newItem.CopyFrom(*item, false);
	newItem.CCSID = pld->charPtr->inventory.GetCCSID(INV_CONTAINER, slot);
	newItem.count = amount - 1;

	int r = pld->charPtr->inventory.AddItem(INV_CONTAINER, newItem);
	if (r == -1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Error creating new stack.");
	}

	//Need to refetch since the pointer might've changed when adding the item.
	item = pld->charPtr->inventory.GetItemPtrByCCSID(CCSID);
	if (item == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item not found in inventory.");
	}
	item->count -= amount;

	int WritePos = AddItemUpdate(sim->SendBuf, sim->Aux1, item);
	WritePos += AddItemUpdate(&sim->SendBuf[WritePos], sim->Aux1, &newItem);
	WritePos += PrepExt_QueryResponseString(&sim->SendBuf[WritePos], query->ID,
			"OK");
	return WritePos;
}

//
//ItemDeleteHandler
//

int ItemDeleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	/* Query: item.delete
	 Args : [0] = Item ID (in hexadecimal)
	 */

	if (query->argCount < 1)
		return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);

	unsigned long InventoryID = strtol(query->args[0].c_str(), NULL, 16);
	int origContainer = (InventoryID & CONTAINER_ID) >> 16;
	int origSlot = InventoryID & CONTAINER_SLOT;
	int r = pld->charPtr->inventory.GetItemBySlot(origContainer, origSlot);
	if (r == -1) {
		g_Logs.simulator->error("[%v] erver error: Item ID not found [%v]",
				sim->InternalID, InventoryID);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item not found.");
	}

	int itemID = pld->charPtr->inventory.containerList[origContainer][r].IID;

	//NOTE: FOR DEBUG PURPOSES
	ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
	if (itemDef != NULL) {
		g_Logs.simulator->info("[%v] Deleting item %v [%v]", sim->InternalID,
				itemID, itemDef->mDisplayName.c_str());

		// If the item provides an always active ability, then it should be removed now
		if (itemDef->mActionAbilityId > 0) {
			creatureInstance->RemoveBuffsFromAbility(itemDef->mActionAbilityId,
					true);
		}

		// If the item is a book page, then inform the client the book is gone
		if(itemDef->mType == ItemType::SPECIAL && itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE) {
			sim->AttemptSend(sim->SendBuf, PrepExt_SendBookOpen(sim->SendBuf, itemDef->mIvMax1, itemDef->mIvMax2 - 1, 3));
		}
	}

	//Find the players best (in terms of recoup amount) grinder (if they have one)
	int invId = GetContainerIDFromName("inv");
	ItemDef *grinderDef = pld->charPtr->inventory.GetBestSpecialItem(invId,
			ITEM_GRINDER);
	if (grinderDef != NULL) {
		int amt = (int) (((double) itemDef->mValue / 100.0)
				* grinderDef->mIvMax1);
		creatureInstance->AdjustCopper(amt);
	}

	//Append a delete notice to the response string
	sim->AttemptSend(sim->SendBuf, RemoveItemUpdate(sim->SendBuf, sim->Aux3,
			&pld->charPtr->inventory.containerList[origContainer][r]));

	//Remove from server's inventory list
	pld->charPtr->inventory.RemItem(InventoryID);

	//Just in case a container was equipped or unequipped, update the max inventory slots
	pld->charPtr->inventory.CountInventorySlots();

	//Update the eq_appearance string and broadcast an update to all other players
	sim->UpdateEqAppearance();

	g_ItemManager.NotifyDestroy(itemID, "item.delete");

	return PrepExt_QueryResponseNull(sim->SendBuf, query->ID);
}


//
//ItemCreateHandler
//

int ItemCreateHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	int itemID = atoi(query->args[0].c_str());
	int targetID = atoi(query->args[1].c_str());
	g_CharacterManager.GetThread("SimulatorThread::ItemCreate");
	CreatureInstance *creature = creatureInstance->actInst->GetPlayerByID(targetID);
	if (creature != NULL) {
		ItemDef * item;
		if (itemID == 0) {
			item = g_ItemManager.GetSafePointerByPartialName(
					query->args[0].c_str());
		} else {
			item = g_ItemManager.GetSafePointerByID(itemID);
		}
		if (item != NULL) {
			int slot = pld->charPtr->inventory.GetFreeSlot(INV_CONTAINER);
			if (slot != -1) {
				InventorySlot *sendSlot =
						creature->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
								item->mID, 1);
				if (sendSlot != NULL) {
					g_Logs.event->info(
							"[SAGE] %v gave %v to %v because '%v'",
							pld->charPtr->cdef.css.display_name,
							item->mDisplayName.c_str(),
							creature->charPtr->cdef.css.display_name,
							query->args[2].c_str());
					int wpos = AddItemUpdate(&sim->Aux1[0], sim->Aux2, sendSlot);
					sim->ActivateActionAbilities(sendSlot);
					g_CharacterManager.ReleaseThread();
					creature->simulatorPtr->AttemptSend(sim->Aux1, wpos);
					return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
				}
			}

		}
	}
	g_CharacterManager.ReleaseThread();
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Failed to give item.");
}



//
//ItemDefDeleteHandler
//

int ItemDefDeleteHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");
	int itemID = atoi(query->args[0].c_str());
	int targetID = atoi(query->args[1].c_str());
	g_CharacterManager.GetThread("SimulatorThread::ItemCreate");
	CreatureInstance *creature = creatureInstance->actInst->GetPlayerByID(targetID);
	if (creature != NULL) {
		ItemDef * item;
		if (itemID == 0) {
			item = g_ItemManager.GetSafePointerByPartialName(
					query->args[0].c_str());
		} else {
			item = g_ItemManager.GetSafePointerByID(itemID);
		}
		if (item != NULL) {

			CharacterData *cdata = creature->charPtr;
			int size = cdata->inventory.containerList[INV_CONTAINER].size();
			int a;
			for (a = 0; a < size; a++) {
				InventorySlot *slot =
						&cdata->inventory.containerList[INV_CONTAINER][a];
				if (slot != NULL) {
					if (slot->IID == item->mID) {
						g_Logs.event->info(
								"[SAGE] %v removed %v from %v because '%v'",
								pld->charPtr->cdef.css.display_name,
								item->mDisplayName.c_str(),
								creature->charPtr->cdef.css.display_name,
								query->args[2].c_str());
						int wpos = RemoveItemUpdate(&sim->Aux1[0], sim->Aux2, slot);
						g_CharacterManager.ReleaseThread();
						creature->simulatorPtr->AttemptSend(sim->Aux1, wpos);
						cdata->inventory.RemItem(slot->CCSID);
						return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
								"OK");
					}
				}
			}
		}
	}
	g_CharacterManager.ReleaseThread();
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Failed to delete item.");
}

//
//ItemDefContentsHandler
//

int ItemDefContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Permission denied.");

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);       //_handleQueryResultMsg
	wpos += PutShort(&sim->SendBuf[wpos], 0);      //Message size
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);  //Query response index
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	int id = atoi(query->args[0].c_str());
	g_CharacterManager.GetThread("SimulatorThread::ItemDefContents");
	CreatureInstance *creature = creatureInstance->actInst->GetPlayerByID(id);
	int count = 0;
	if (creature != NULL) {
		CharacterData *cdata = creature->charPtr;
		int size = cdata->inventory.containerList[INV_CONTAINER].size();
		int a;
		for (a = 0; a < size; a++) {
			InventorySlot *slot =
					&cdata->inventory.containerList[INV_CONTAINER][a];
			if (slot != NULL) {
				wpos += PutByte(&sim->SendBuf[wpos], 2);
				Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "%d", slot->IID);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
				wpos += PutStringUTF(&sim->SendBuf[wpos],
						slot->dataPtr->mDisplayName.c_str());
				count++;
			}
		}
	}
	g_CharacterManager.ReleaseThread();
	PutShort(&sim->SendBuf[7], count);
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
//ItemMorphHandler
//

int ItemMorphHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: item.morph
	 Sent when an item is refashioned via the NPC.
	 Args : [0] Item Inventory Hex ID (Original Item)
	 [1] Item Inventory Hex ID (New Equipment Look)
	 [2] = Creature Instance ID of the vendor who processed this action.
	 */
	int rval = sim->ItemMorph(false);
	if (rval <= 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

//
//ShopContentsHandler
//

int ShopContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: shop.contents
	 Sent when an NPC shop is clicked on.
	 Args : [0] = Creature ID of NPC.
	 */

	if (query->argCount < 1)
		return 0;

	int CID = atoi(query->args[0].c_str());
	g_Logs.simulator->debug("[%v] shop.contents: %v", sim->InternalID, CID);

	if (CID == creatureInstance->CreatureID) {

		//Fill the query with the contents of the buyback information.
		g_Logs.simulator->debug("[%v] Shop contents self.", sim->InternalID);
		int wpos = 0;
		wpos += PutByte(&sim->SendBuf[wpos], 1);              //_handleQueryResultMsg
		wpos += PutShort(&sim->SendBuf[wpos], 0);          //Placeholder for message size
		wpos += PutInteger(&sim->SendBuf[wpos], query->ID);     //Query response ID

		int rowCount =
				(int) pld->charPtr->inventory.containerList[BUYBACK_CONTAINER].size();
		wpos += PutShort(&sim->SendBuf[wpos], rowCount);

		//Note: low index = newest, also highest slot number
		for (int index = 0; index < rowCount; index++) {
			wpos += PutByte(&sim->SendBuf[wpos], 1); //Always one string for regular items.
			GetItemProto(sim->Aux3,
					pld->charPtr->inventory.containerList[BUYBACK_CONTAINER][index].IID,
					0);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux3);
		}
		PutShort(&sim->SendBuf[1], wpos - 3);
		return wpos;
	}

	int CDef = sim->ResolveCreatureDef(CID);

	int wpos = creatureInstance->actInst->itemShopList.ProcessQueryShop(sim->SendBuf,
			sim->Aux1, CDef, query->ID);
	if (wpos == 0) {
		g_Logs.simulator->warn("[%v] Shop Contents not found for [%v]",
				sim->InternalID, CDef);
		sprintf(sim->Aux3, "Shop Contents not found for [%d]", CDef);
		wpos = PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux3);
	}
	return wpos;
}

//
//EssenceShopContentsHandler
//

int EssenceShopContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: essenceShop.contents
	 Sent when an essence shop (chest) is clicked on.
	 Args : [0] = Creature ID of the chest spawn object.
	 */

	if (query->argCount < 1)
		return 0;

	int CID = atoi(query->args[0].c_str());
	g_Logs.simulator->trace("[%v] essenceShop.contents: %v", sim->InternalID, CID);

	int CDef = sim->ResolveCreatureDef(CID);

	int WritePos = creatureInstance->actInst->essenceShopList.ProcessQueryEssenceShop(
			sim->SendBuf, sim->Aux1, CDef, query->ID);
	if (WritePos == 0) {
		g_Logs.simulator->warn("[%v] EssenceShop not found for [%v]",
				sim->InternalID, CDef);
		sprintf(sim->Aux3, "EssenceShop not found for [%d]", CDef);
		WritePos = PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux3);
	}
	return WritePos;
}


//
//ItemPreviewHandler
//

int ItemPreviewHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//[0] = Type (0 = invisible, 1 = nude, 2 = clone)
	//[1] = Item ID
	if (query->argCount < 2 || query->argCount > 66) //32 slots * 2, + 2 params should be plenty.  Bail on greater to be safe.
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query->");

	static const char *invis =
			"n4:{[\"c\"]=\"Horde-Invisible_Biped\",[\"sk\"]={[\"main\"]=\"FFFFFF\"},[\"sz\"]=\"1.0\"}";

	int type = atoi(query->args[0].c_str());
	int itemID = atoi(query->args[1].c_str());

	ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
	if (itemDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Server error.");

	std::vector<int> eqArray;
	for (unsigned int i = 2; i < query->argCount; i++)
		eqArray.push_back(atoi(query->args[i].c_str()));

	int targetEquipSlot = 0;
	switch (itemDef->mEquipType) {
	case ItemEquipType::NONE:
		targetEquipSlot = ItemEquipSlot::NONE;
		break;
	case ItemEquipType::WEAPON_1H:
		targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND;
		break;
	case ItemEquipType::WEAPON_1H_UNIQUE:
		targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND;
		break;
	case ItemEquipType::WEAPON_1H_MAIN:
		targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND;
		break;
	case ItemEquipType::WEAPON_1H_OFF:
		targetEquipSlot = ItemEquipSlot::WEAPON_OFF_HAND;
		break;
	case ItemEquipType::WEAPON_2H:
		targetEquipSlot = ItemEquipSlot::WEAPON_MAIN_HAND;
		break;
	case ItemEquipType::WEAPON_RANGED:
		targetEquipSlot = ItemEquipSlot::WEAPON_RANGED;
		break;
	case ItemEquipType::ARMOR_SHIELD:
		targetEquipSlot = ItemEquipSlot::WEAPON_OFF_HAND;
		break;
	case ItemEquipType::ARMOR_HEAD:
		targetEquipSlot = ItemEquipSlot::ARMOR_HEAD;
		break;
	case ItemEquipType::ARMOR_NECK:
		targetEquipSlot = ItemEquipSlot::ARMOR_NECK;
		break;
	case ItemEquipType::ARMOR_SHOULDER:
		targetEquipSlot = ItemEquipSlot::ARMOR_SHOULDER;
		break;
	case ItemEquipType::ARMOR_CHEST:
		targetEquipSlot = ItemEquipSlot::ARMOR_CHEST;
		break;
	case ItemEquipType::ARMOR_ARMS:
		targetEquipSlot = ItemEquipSlot::ARMOR_ARMS;
		break;
	case ItemEquipType::ARMOR_HANDS:
		targetEquipSlot = ItemEquipSlot::ARMOR_HANDS;
		break;
	case ItemEquipType::ARMOR_WAIST:
		targetEquipSlot = ItemEquipSlot::ARMOR_WAIST;
		break;
	case ItemEquipType::ARMOR_LEGS:
		targetEquipSlot = ItemEquipSlot::ARMOR_LEGS;
		break;
	case ItemEquipType::ARMOR_FEET:
		targetEquipSlot = ItemEquipSlot::ARMOR_FEET;
		break;
	case ItemEquipType::ARMOR_RING:
		targetEquipSlot = ItemEquipSlot::ARMOR_RING_L;
		break;
	case ItemEquipType::ARMOR_RING_UNIQUE:
		targetEquipSlot = ItemEquipSlot::ARMOR_RING_L;
		break;
	case ItemEquipType::ARMOR_AMULET:
		targetEquipSlot = ItemEquipSlot::ARMOR_AMULET;
		break;
	default:
		targetEquipSlot = ItemEquipSlot::NONE;
	}
	if (targetEquipSlot == ItemEquipSlot::NONE)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Server error.");

	if (type == 0 || type == 1) {
		eqArray.clear();
		eqArray.push_back(targetEquipSlot);
		eqArray.push_back(itemID);
	} else {
		for (size_t i = 0; i < eqArray.size(); i += 2) {
			if (eqArray[i] == targetEquipSlot)
				eqArray[i + 1] = itemID;
		}
	}

	int wpos = 0;
	sim->Aux1[wpos++] = '{';
	for (size_t i = 0; i < eqArray.size(); i += 2) {
		if (i > 0)
			sim->Aux1[wpos++] = ',';
		wpos += sprintf(&sim->Aux1[wpos], "[%d]=%d", eqArray[i], eqArray[i + 1]);
	}
	sim->Aux1[wpos++] = '}';

	CreatureInstance *cptr = creatureInstance->actInst->SpawnCreate(creatureInstance,
			3509);  //Generic interact object, the details will be replaced.
	if (cptr == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Server error.");

	Util::SafeCopy(cptr->css.display_name, creatureInstance->css.display_name,
			sizeof(cptr->css.display_name));
	Util::SafeCopy(cptr->css.sub_name, "Item Preview",
			sizeof(cptr->css.sub_name));
	cptr->css.aggro_players = 0;
	cptr->_AddStatusList(StatusEffects::UNATTACKABLE, -1);
	cptr->_AddStatusList(StatusEffects::INVINCIBLE, -1);

	if (type == 0)
		cptr->css.SetAppearance(invis);
	else
		cptr->css.SetAppearance(creatureInstance->css.appearance.c_str());
	cptr->css.SetEqAppearance(sim->Aux1);

	cptr->SetServerFlag(ServerFlags::TriggerDelete, true);
	cptr->deathTime = g_ServerTime;

	CreatureDefinition cd;
	cd.CopyFrom(pld->charPtr->cdef);
	cd.DefHints = 0;
	cd.CreatureDefID = 1;
	cd.css.CopyFrom(&cptr->css);

	cptr->CreatureDefID = 1;

	int size = PrepExt_CreatureDef(sim->SendBuf, &cd);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, size,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);

	size = PrepExt_CreatureFullInstance(sim->SendBuf, cptr);
	//int size = PrepExt_GeneralMoveUpdate(sim->SendBuf, cptr);
	creatureInstance->actInst->LSendToLocalSimulator(sim->SendBuf, size,
			creatureInstance->CurrentX, creatureInstance->CurrentZ);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//CraftCreateHandler
//

int CraftCreateHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	/*  Query: craft.create
	 Sent when an item is crafted via the crafting NPC.
	 Args : [0] Item ID of the plan to create from.
	 [1] Creature Instance ID of the vendor who processed this action.
	 */
	int rval = protected_helper_query_craft_create(sim, pld, query, creatureInstance);
	if (rval <= 0)
		rval = PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->GetErrorString(rval));

	return rval;
}

int CraftCreateHandler::protected_helper_query_craft_create(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 2)
		return QueryErrorMsg::GENERIC;

	int ItemID = atoi(query->args[0].c_str());
	int CreatureID = atoi(query->args[1].c_str());

	// Make sure this object isn't too far away.
	int distCheck = sim->CheckDistance(CreatureID);
	if (distCheck != 0)
		return distCheck;

	//Check that the item plan exists in the database.
	ItemDef *itemPlan = g_ItemManager.GetPointerByID(ItemID);
	if (itemPlan == NULL)
		return QueryErrorMsg::INVALIDITEM;

	//Check that the resulting item exists in the database.
	ItemDef *resultItem = g_ItemManager.GetPointerByID(itemPlan->resultItemId);
	if (resultItem == NULL)
		return QueryErrorMsg::INVALIDITEM;

	//Create an alias to make the code a bit cleaner.
	InventoryManager &inv = pld->charPtr->inventory;

	// Check that the crafting component exists in the player's inventory.
	int count = inv.GetItemCount(INV_CONTAINER, itemPlan->keyComponentId);
	if (count < 1)
		return QueryErrorMsg::INVMISSING;

	//Build a list of component counts.
	std::vector<int> CraftID;
	std::vector<int> CraftReq;
	for (size_t i = 0; i < itemPlan->craftItemDefId.size(); i++) {
		int materialID = itemPlan->craftItemDefId[i];

		bool bFound = false;
		for (size_t s = 0; s < CraftID.size(); s++) {
			if (CraftID[s] == materialID) {
				bFound = true;
				CraftReq[s]++;
				break;
			}
		}
		if (bFound == false) {
			CraftID.push_back(materialID);
			CraftReq.push_back(1);
		}
	}

	//Check that the player has the required components.
	for (size_t i = 0; i < CraftID.size(); i++) {
		ItemDef *craftItem = g_ItemManager.GetPointerByID(CraftID[i]);
		if (craftItem == NULL)
			return QueryErrorMsg::INVALIDITEM;
		count = inv.GetItemCount(INV_CONTAINER, CraftID[i]);
		if (count < CraftReq[i])
			return QueryErrorMsg::INVMISSING;
	}

	//Check that the player has a free slot.
	int slotIndex = inv.GetFreeSlot(INV_CONTAINER);
	if (slotIndex == -1)
		return QueryErrorMsg::INVSPACE;

	//Good to go, create the item.
	InventorySlot *itemPtr = NULL;
	itemPtr = inv.AddItem_Ex(INV_CONTAINER, itemPlan->resultItemId, 1);
	if (itemPtr == NULL)
		return QueryErrorMsg::INVCREATE;

	sim->ActivateActionAbilities(itemPtr);

	int wpos = AddItemUpdate(sim->SendBuf, sim->Aux3, itemPtr);

	//Remove the crafting materials.
	for (size_t i = 0; i < CraftID.size(); i++)
		wpos += inv.RemoveItemsAndUpdate(INV_CONTAINER, CraftID[i], CraftReq[i],
				&sim->SendBuf[wpos]);

	vector<InventoryQuery> iq;

	//Remove the crafting component.
	inv.ScanRemoveItems(INV_CONTAINER, itemPlan->keyComponentId, 1, iq);
	if (iq.size() > 0)
		wpos += RemoveItemUpdate(&sim->SendBuf[wpos], sim->Aux3, iq[0].ptr);
	inv.RemoveItems(INV_CONTAINER, iq);

	//Remove the plan.
	inv.ScanRemoveItems(INV_CONTAINER, itemPlan->mID, 1, iq);
	if (iq.size() > 0)
		wpos += RemoveItemUpdate(&sim->SendBuf[wpos], sim->Aux3, iq[0].ptr);
	inv.RemoveItems(INV_CONTAINER, iq);

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//ModCraftHandler
//

int ModCraftHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	//Requires a modded client to perform this action.
	// Arguments: list of container/slot IDs in hexadecimal notation.

	InventoryManager &inv = pld->charPtr->inventory;

	//Resolve inputs.
	std::vector<CraftInputSlot> inputs;
	std::vector<CraftInputSlot> outputs;
	for (unsigned int i = 0; i < query->argCount; i++) {
		unsigned int CCSID = inv.GetCCSIDFromHexID(query->GetString(i));
		InventorySlot *slot = inv.GetItemPtrByCCSID(CCSID);
		if (slot == NULL)
			continue;

		int itemID = slot->IID;
		int stackCount = slot->GetStackCount();
		ItemDef *itemDef = g_ItemManager.GetPointerByID(itemID);
		if (itemDef == NULL)
			continue;

		inputs.push_back(CraftInputSlot(CCSID, itemID, stackCount, itemDef));
	}
	if (inputs.size() == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must provide some input items.");

	const CraftRecipe *recipe = g_CraftManager.GetRecipe(inputs);
	if (recipe == NULL) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"%s not match a known combination.",
				((inputs.size() == 1) ? "That item does" : "Those items do"));
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}

	int requiredSlots = recipe->GetOutputCount();
	if (inv.CountFreeSlots(INV_CONTAINER) < requiredSlots) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"You must have %d empty inventory %s.", requiredSlots,
				((requiredSlots == 1) ? "slot" : "slots"));
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}

	bool success = false;
	STRINGVECTOR outputMsg;
	if (g_CraftManager.RunRecipe(recipe, inputs, outputs) == true) {
		//Verify whether the items exist in the item database.
		bool verified = true;
		for (size_t i = 0; i < outputs.size(); i++) {
			if (g_ItemManager.GetPointerByID(outputs[i].mID) == NULL) {
				verified = false;
				break;
			}
		}

		if (verified == true) {
			//Give resulting items.
			for (size_t i = 0; i < outputs.size(); i++) {
				InventorySlot *newItem = inv.AddItem_Ex(INV_CONTAINER,
						outputs[i].mID, outputs[i].mStackCount);
				if (newItem) {
					ItemDef *itemDef = g_ItemManager.GetPointerByID(
							outputs[i].mID);
					std::string msg = "Obtained ";
					Util::StringAppendInt(msg, outputs[i].mStackCount);
					msg.append(" ");
					msg.append(itemDef->mDisplayName);
					outputMsg.push_back(msg);

					sim->ActivateActionAbilities(newItem);
					int wpos = AddItemUpdate(sim->Aux1, sim->Aux2, newItem);
					sim->AttemptSend(sim->Aux1, wpos);
				}
			}

			//Remove crafting components.
			for (size_t i = 0; i < inputs.size(); i++) {
				InventorySlot *remItem = inv.GetItemPtrByCCSID(
						inputs[i].mCCSID);
				if (remItem) {
					if (remItem->GetStackCount() == 1)
						g_ItemManager.NotifyDestroy(remItem->IID, "mod.craft");
					int wpos = RemoveItemUpdate(sim->Aux1, sim->Aux2, remItem);
					sim->AttemptSend(sim->Aux1, wpos);
				}
				inv.RemItem(inputs[i].mCCSID);
			}
			success = true;
		}
	}

	if (success == false)
		sim->SendInfoMessage("Unable to craft.", INFOMSG_INFO);
	else {
		int wpos = 0;
		if (outputMsg.size() > 0)
			wpos = PrepExt_SendInfoMessage(sim->SendBuf, "Succesful craft!",
					INFOMSG_INFO);
		for (size_t i = 0; i < outputMsg.size(); i++)
			wpos += PrepExt_SendInfoMessage(&sim->SendBuf[wpos],
					outputMsg[i].c_str(), INFOMSG_INFO);
		sim->AttemptSend(sim->SendBuf, wpos);
		g_Logs.event->info("[CRAFT] Crafted %v", recipe->GetName());
	}

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}
