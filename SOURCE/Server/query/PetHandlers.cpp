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

#include "PetHandlers.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Inventory.h"
#include "../Debug.h"
#include "../Config.h"
#include "../util/Log.h"

//
//PetListHandler
//

int PetListHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/* Query: mod.pet.list
	 Args : [none]
	 Response: Send back a table with the pet data.   */

	MULTISTRING response;
	g_PetDefManager.FillQueryResponse(response);
	return PrepExt_QueryResponseMultiString(sim->SendBuf, query->ID, response);
}

//
//PetPurchaseHandler
//

int PetPurchaseHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/* Query: mod.pet.purchase
	 Args : 1 [required] : integer of the CreatureDefID to purchase.  */
	int CDefID = 0;
	if (query->argCount > 0)
		CDefID = query->GetInteger(0);

	PetDef *petDef = g_PetDefManager.GetEntry(CDefID);
	if (petDef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Pet not found.");

	if (creatureInstance->css.copper < petDef->mCost)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Not enough coin.");

	if (creatureInstance->css.level < petDef->mLevel)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are not high enough level.");

	//Check item ID too because an improper line in the file may leave it zero.
	ItemDef *itemDef = g_ItemManager.GetPointerByID(petDef->mItemDefID);
	if (itemDef == NULL || petDef->mItemDefID == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Item does not exist.");

	InventoryManager &im = pld->charPtr->inventory;
	int slot = im.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No free backpack slots.");

	creatureInstance->AdjustCopper(-petDef->mCost);

	int wpos = 0;
	InventorySlot *sendSlot = im.AddItem_Ex(INV_CONTAINER, petDef->mItemDefID,
			1);
	if (sendSlot != NULL) {
		pld->charPtr->pendingChanges++;
		sim->ActivateActionAbilities(sendSlot);
		wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux1, sendSlot);
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");
	return wpos;
}

//
//PetPreviewHandler
//

int PetPreviewHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {

	/* Query: mod.pet.preview
	 Args : 1 [required] : integer of the CreatureDefID to preview.  */
	int CDefID = 0;
	if (query->argCount > 0)
		CDefID = query->GetInteger(0);

	PetDef *petDef = g_PetDefManager.GetEntry(CDefID);
	if (petDef == NULL)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "Pet not found.");

	CreatureDefinition *creatureDef = CreatureDef.GetPointerByCDef(CDefID);
	if (creatureDef == NULL)
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "Pet not found.");

	CreatureInstance preview;
	preview.CurrentX = creatureInstance->CurrentX;
	preview.CurrentY = creatureInstance->CurrentY;
	preview.CurrentZ = creatureInstance->CurrentZ;
	preview.CreatureDefID = CDefID;
	preview.CreatureID = creatureInstance->actInst->GetNewActorID();
	preview.css.CopyFrom(&creatureDef->css);
	int wpos = PrepExt_CreatureFullInstance(sim->SendBuf, &preview);
	sim->AttemptSend(sim->SendBuf, wpos);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
//GetPetHandler
//

int GetPetHandler::handleQuery(SimulatorThread *sim, CharacterServerData *pld,
		SimulatorQuery *query, CreatureInstance *creatureInstance) {
	//Return the ID of the player's sidekick.
	int result = sim->ResolveEmoteTarget(1);  //See function for parameter details.
	if (result == creatureInstance->CreatureID)
		result = 0;  //Let the client know if no sidekick was found
	sprintf(sim->Aux1, "%d", result);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux1);
}
