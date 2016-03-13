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
#include "../util/Log.h"
#include "Ability2.h"
#include "Account.h"
#include "Config.h"
#include "Creature.h"
#include "Instance.h"
#include "ConfigString.h"

void AddPet(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance, int CDefID) {
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

int UseItem(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance, unsigned int CCSID) {

	g_Logs.simulator->debug("[%v] Uses item %v", sim->InternalID, CCSID);

	if (creatureInstance->activeLootID != 0) {
		sim->SendInfoMessage("You cannot use items while trading.", INFOMSG_ERROR);
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

	if (itemDef->mType != ItemType::CONSUMABLE && itemDef->mType != ItemType::SPECIAL)
		return -1;

	// The AddSidekick() function has its own thread guard, so we don't
	// need one here.
	bool removeOnUse = true;
	if (itemDef->mUseAbilityId != 0) {
		ConfigString cfg(itemDef->Params);
		//AddMessage((long)creatureInst, itemDef->mUseAbilityId, BCM_AbilityRequest);
		int r = creatureInstance->RequestAbilityActivation(itemDef->mUseAbilityId);
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
		if(itemDef->mType == ItemType::SPECIAL && itemDef->mIvType1 == ItemIntegerType::BOOK_PAGE) {
			g_Logs.simulator->debug("Opening book %v on page %v", itemDef->mIvMax1, itemDef->mIvMax2);
			return PrepExt_SendBookOpen(sim->SendBuf, itemDef->mIvMax1, itemDef->mIvMax2 - 1);
		}
		else {
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

					Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1), "You gain %d credits.",
							credits);
					sim->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
					creatureInstance->SendStatUpdate(STAT::CREDITS);
				}
				if (abPoints > 0) {
					creatureInstance->css.current_ability_points += abPoints;
					creatureInstance->css.total_ability_points += abPoints;
					pld->charPtr->AddAbilityPoints(abPoints);
					creatureInstance->SendStatUpdate(STAT::CURRENT_ABILITY_POINTS);
					creatureInstance->SendStatUpdate(STAT::TOTAL_ABILITY_POINTS);
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
		if(ret > 0) {
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

