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

#include "VaultHandlers.h"
#include "../Account.h"
#include "../Character.h"
#include "../Creature.h"
#include "../Instance.h"
#include "../Config.h"
#include <algorithm>

using namespace std;

//
// VaultSizeHandler
//

int VaultSizeHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	sprintf(sim->Aux1, "%d", pld->charPtr->VaultGetTotalCapacity());
	sprintf(sim->Aux2, "%d", pld->accPtr->DeliveryBoxSlots);
	return PrepExt_QueryResponseString2(sim->SendBuf, query->ID, sim->Aux1,
			sim->Aux2);
}

//
// VaultSendHandler
//

int VaultSendHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	/* Make sure there enough stamps in the vault to be able to send each
	 * item in the delivery box
	 */

	int items = pld->accPtr->inventory.containerList[DELIVERY_CONTAINER].size();
	if (items < 1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must place the items to send in your delivery box.");
	}

	// TODO hardcoded
	int stamps = pld->charPtr->inventory.GetItemCount(STAMPS_CONTAINER,
			POSTAGE_STAMP_ITEM_ID);
	if (items > stamps) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You do not have enough stamps.");
	}

	/**
	 * Get the recipient's creature
	 */
	int cdefID = g_UsedNameDatabase.GetIDByName(query->GetString(1));
	if (cdefID == -1) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Unknown character name '%s'", query->GetString(1));
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}

	g_CharacterManager.GetThread("SimulatorThread::VaultSend");
	g_AccountManager.cs.Enter("SimulatorThread::VaultSend");

	CharacterData *cd = g_CharacterManager.GetPointerByID(cdefID);
	if (cd == NULL) {
		// Not active
		cd = g_CharacterManager.RequestCharacter(cdefID, true);
	}

	if (cd == NULL) {
		g_CharacterManager.ReleaseThread();
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not get creature instance.");
	}

	// Make sure player is not trying to send to their own account
	if (cd->AccountID == pld->accPtr->ID) {
		g_CharacterManager.ReleaseThread();
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You cannot send items to yourself.");
	}

	// Now we have a creature, we can try and get the account
	AccountData *acc = g_AccountManager.GetActiveAccountByID(cd->AccountID);
	if (acc == NULL) {
		acc = g_AccountManager.LoadAccountID(cd->AccountID);
	}
	if (acc == NULL) {
		g_CharacterManager.ReleaseThread();
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not get account instance.");
	}

	// Now we have an account, we can make sure there are enough free delivery slots
	int freeSlots = acc->DeliveryBoxSlots
			- acc->inventory.containerList[DELIVERY_CONTAINER].size();
	if (freeSlots < items) {
		if (freeSlots < 1)
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"The recipient has no free delivery slots.");
		else
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"The recipient only has %d free delivery slots.",
					freeSlots);
		g_CharacterManager.ReleaseThread();
		g_AccountManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}

	// Notify the recipient if they are online
	CreatureInstance *inst = g_ActiveInstanceManager.GetPlayerCreatureByDefID(
			cd->cdef.CreatureDefID);

	// Have got this far, so have enough stamps, and recipient exists and has enough free delivery slots
	int wpos = 0;

	InventoryManager *origInv = &pld->accPtr->inventory;
	InventoryManager *destInv = &acc->inventory;
	std::vector<InventorySlot> l = origInv->containerList[DELIVERY_CONTAINER];
	int origSlot = 0;
	int destSlot = destInv->containerList[DELIVERY_CONTAINER].size();
	int itemsSent = 0;

	pld->accPtr->PendingMinorUpdates++;
	acc->PendingMinorUpdates++;

	for (std::vector<InventorySlot>::iterator it = l.begin(); it != l.end();
			++it) {

		if ((origInv->VerifyContainerSlotBoundary(DELIVERY_CONTAINER, origSlot)
				== false)
				|| (destInv->VerifyContainerSlotBoundary(DELIVERY_CONTAINER,
						destSlot) == false)) {
			wpos += PrepExt_QueryResponseError(&sim->SendBuf[wpos], query->ID,
					"Failed to move item.");
			goto exit;
		}

		int mpos = origInv->ItemMove(&sim->SendBuf[wpos], sim->Aux3,
				&creatureInstance->css, pld->charPtr->localCharacterVault,
				DELIVERY_CONTAINER, origSlot, destInv, DELIVERY_CONTAINER,
				destSlot, false);
		if (mpos > 0) {
			wpos += mpos;
			wpos += pld->charPtr->inventory.RemoveItemsAndUpdate(
					STAMPS_CONTAINER, POSTAGE_STAMP_ITEM_ID, 1,
					&sim->SendBuf[wpos]);
			pld->charPtr->pendingChanges++;
			itemsSent++;
		} else {
			Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
					"Failed to move item (%d).", mpos);
			wpos += PrepExt_QueryResponseError(&sim->SendBuf[wpos], query->ID,
					sim->Aux1);
			goto exit;
		}

		origSlot++;
		destSlot++;
	}

	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");

	exit:

	// Inform the active player if we have one
	if (inst != NULL && itemsSent > 0) {
		inst->simulatorPtr->SendInventoryData(
				destInv->containerList[DELIVERY_CONTAINER]);
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"%s just sent you %d item%s to your delivery box. You may collect at any vault.",
				pld->charPtr->cdef.css.display_name, itemsSent);
		inst->simulatorPtr->SendInfoMessage(sim->Aux1, INFOMSG_INFO);
	}

	g_CharacterManager.ReleaseThread();
	g_AccountManager.cs.Leave();
	return wpos;
}

//
// VaultExpandHandler
//

int VaultExpandHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, "Invalid query.");

	int CID = query->GetInteger(0);
	int r = sim->protected_CheckDistance(CID);
	if (r != 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You are too far away.");

	int CDefID = sim->ResolveCreatureDef(CID);
	CreatureDefinition *cdef = CreatureDef.GetPointerByCDef(CDefID);
	if (cdef == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid creature.");
	if (!(cdef->DefHints & CDEF_HINT_VAULT))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must speak to a vault keeper.");

	if (pld->charPtr->VaultIsMaximumCapacity() == true)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Your vault space is at maximum capacity.");

	if (g_Config.AccountCredits) {
		creatureInstance->css.credits = pld->accPtr->Credits;
	}
	if (creatureInstance->css.credits < CharacterData::VAULT_EXPAND_CREDIT_COST)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You do not have enough credits.");

	pld->charPtr->VaultDoPurchaseExpand();
	int newSize = pld->charPtr->VaultGetTotalCapacity();
	creatureInstance->css.credits -= CharacterData::VAULT_EXPAND_CREDIT_COST;

	if (g_Config.AccountCredits) {
		pld->accPtr->Credits = creatureInstance->css.credits;
		pld->accPtr->PendingMinorUpdates++;
	}

	creatureInstance->SendStatUpdate(STAT::CREDITS);

	int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	wpos += PrepExt_CreatureEventVaultSize(&sim->SendBuf[wpos],
			creatureInstance->CreatureID, newSize, pld->accPtr->DeliveryBoxSlots);
	return wpos;
}
