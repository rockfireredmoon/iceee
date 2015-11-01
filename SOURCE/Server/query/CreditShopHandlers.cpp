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

#include "CreditShopHandlers.h"
#include "../CreditShop.h"
#include "../Account.h"
#include "../Config.h"
#include <algorithm>

using namespace std;
using namespace CS;

//
// CreditShopBuyHandler
//

int CreditShopBuyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->args.size() < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");
	int id = query->GetInteger(0);
	CreditShopItem * csItem = g_CreditShopManager.GetItem(id);
	if (csItem == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such item.");
	} else {
		int errCode = g_CreditShopManager.ValidateItem(csItem, pld->accPtr, &creatureInstance->css, creatureInstance->charPtr);
		if(errCode != CreditShopError::NONE) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					CreditShopError::GetDescription(errCode).c_str());
		}
		ItemDef *itemDef = g_ItemManager.GetSafePointerByID(csItem->mItemId);
		if (itemDef == NULL)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"No such item!");

		InventorySlot *sendSlot =
				creatureInstance->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
						itemDef->mID, csItem->mIv1 + 1);
		if (sendSlot == NULL) {
			int err = creatureInstance->charPtr->inventory.LastError;
			if (err == InventoryManager::ERROR_ITEM)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Server error: item does not exist.");
			else if (err == InventoryManager::ERROR_SPACE)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"You do not have any free inventory space.");
			else if (err == InventoryManager::ERROR_LIMIT)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"You already the maximum amount of these items.");
			else
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Server error: undefined error.");
		}
		sim->ActivateActionAbilities(sendSlot);

		g_CharacterManager.GetThread("Simulator::MarketBuy");

		if (csItem->mPriceCurrency == Currency::COPPER
				|| csItem->mPriceCurrency == Currency::COPPER_CREDITS) {
			creatureInstance->css.copper -= csItem->mPriceCopper;
			creatureInstance->SendStatUpdate(STAT::COPPER);
		}

		if (csItem->mPriceCurrency == Currency::CREDITS
				|| csItem->mPriceCurrency == Currency::COPPER_CREDITS) {
			creatureInstance->css.credits -= csItem->mPriceCredits;
			if (g_Config.AccountCredits) {
				pld->accPtr->Credits = creatureInstance->css.credits;
				pld->accPtr->PendingMinorUpdates++;
			}
			creatureInstance->SendStatUpdate(STAT::CREDITS);
		}

		if (csItem->mQuantityLimit > 0) {
			csItem->mQuantitySold++;
			g_CreditShopManager.SaveItem(csItem);
		}

		int wpos = 0;
		wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux2, sendSlot);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");

		g_CharacterManager.ReleaseThread();

		return wpos;
	}
}

