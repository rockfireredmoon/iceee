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
#include "../GameConfig.h"
#include "../Cluster.h"
#include "../Scheduler.h"
#include "../Debug.h"
#include "../util/Log.h"
#include <algorithm>

using namespace std;
using namespace CS;

//
// CreditShopPurchaseNameHandler
//

int CreditShopPurchaseNameHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->args.size() < 2)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");
	string firstName = query->GetString(0);
	string secondName = query->GetString(1);

	int res = g_AccountManager.ValidateNameParts(firstName.c_str(),
			secondName.c_str());
	if (res != AccountManager::CHARACTER_SUCCESS) {
		Util::SafeFormat(sim->Aux1, sizeof(sim->Aux1),
				"Renamed to %s %s returned error %d", firstName.c_str(),
				secondName.c_str(), res);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux1);
	}

	string fullName = firstName + " " + secondName;
	string currentName = creatureInstance->css.display_name;

	if (g_GameConfig.UseAccountCredits) {
		creatureInstance->css.credits = pld->accPtr->Credits;
	}
	if ((unsigned long) creatureInstance->css.credits
			< g_GameConfig.NameChangeCost) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You do not have enough credits!");
	}

	AccountData * acc = g_AccountManager.FetchIndividualAccount(
			creatureInstance->charPtr->AccountID);
	if (acc == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Name change failed, missing account.");
	}
	CharacterCacheEntry *ce = acc->characterCache.GetCacheCharacter(
			creatureInstance->CreatureDefID);
	if (ce == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Name change failed, missing cache entry.");
	}
	if (fullName.compare(ce->display_name) == 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You have chosen the same name!");
	}
	ce->display_name = fullName.c_str();
	acc->PendingMinorUpdates++;
	memcpy(creatureInstance->css.display_name, fullName.c_str(),
			fullName.size() + 1);

	g_UsedNameDatabase.Remove(pld->CreatureDefID);
	g_UsedNameDatabase.Add(pld->CreatureDefID, fullName);

	g_Logs.cs->info("Player '%v' changed their name to '%v'",
			currentName.c_str(), fullName.c_str());

	creatureInstance->css.credits -= g_GameConfig.NameChangeCost;
	if (g_GameConfig.UseAccountCredits) {
		pld->accPtr->Credits = creatureInstance->css.credits;
		pld->accPtr->PendingMinorUpdates++;
	}
	creatureInstance->SendStatUpdate(STAT::CREDITS);
	creatureInstance->SendStatUpdate(STAT::DISPLAY_NAME);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// CreditShopEditHandler
//

int CreditShopEditHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->args.size() < 1)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	if (!sim->CheckPermissionSimple(Perm_Account, Permission_Sage))
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Permission denied.");

	if (strcmp(query->GetString(0), "DELETE") == 0 && query->args.size() > 1) {
		int id = query->GetInteger(1);
		CS::CreditShopItem item = g_CreditShopManager.GetItem(id);
		if (item.mId == 0)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Invalid item.");
		else {
			if (g_CreditShopManager.RemoveItem(id)) {
				g_Logs.event->info("[CS] Removed credit shop item %v",
						item.mId);
				return PrepExt_QueryResponseString(sim->SendBuf, query->ID,
						"OK");
			}
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"Failed to remove.");
		}
	} else if (query->args.size() > 22) {
		CS::CreditShopItem csItem;
		bool isNew = strcmp(query->GetString(0), "NEW") == 0;
		if (!isNew) {
			csItem = g_CreditShopManager.GetItem(query->GetInteger(0));
			if (csItem.mId == 0)
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"Invalid item.");
		}

		int currency = query->GetInteger(16);
		unsigned long priceCopper = 0;
		unsigned long priceCredits = 0;
		STRINGLIST priceElements;
		Util::Split(query->GetString(14), "+", priceElements);
		if (currency == Currency::COPPER) {
			priceCopper = atoi(priceElements[0].c_str());
		} else if (currency == Currency::CREDITS) {
			priceCredits = atoi(priceElements[0].c_str());
		} else if (currency == Currency::COPPER_CREDITS) {
			if (priceElements.size() != 2) {
				return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
						"COPPER+CREDITS require two costs, <copperCost>+<creditCost>.");
			}
			priceCopper = atoi(priceElements[0].c_str());
			priceCredits = atoi(priceElements[1].c_str());
		}

		csItem.mTitle = query->GetString(2);
		csItem.mDescription = query->GetString(4);
		csItem.mCategory = Category::GetIDByName(query->GetString(6));
		csItem.mStatus = Status::GetIDByName(query->GetString(8));
		Util::ParseDate(query->GetString(10), csItem.mStartDate);
		Util::ParseDate(query->GetString(12), csItem.mEndDate);
		csItem.mPriceCopper = priceCopper;
		csItem.mPriceCredits = priceCredits;
		csItem.mPriceCurrency = currency;
		csItem.mQuantityLimit = query->GetInteger(18);
		csItem.mQuantitySold = query->GetInteger(20);
		csItem.ParseItemProto(query->GetString(22));

		// Check the item
		ItemDef * item = g_ItemManager.GetSafePointerByID(csItem.mItemId);
		if (item == NULL) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"No such item!");
		}
		if (csItem.mTitle.compare(item->mDisplayName) == 0)
			csItem.mTitle = "";

		if (csItem.mId == 0)
			g_CreditShopManager.CreateItem(&csItem);
		else
			g_CreditShopManager.UpdateItem(&csItem);

		g_Logs.cs->info("Updated Credit shop item %v '%v'", csItem.mId,
				csItem.mTitle.c_str());
		return PrepExt_QueryResponseString(sim->SendBuf, query->ID, sim->Aux3);
	}
	return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
			"Invalid sub-query.");
}

//
// CreditShopListHandler
//

int CreditShopListHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	unsigned long simID = sim->InternalID;
	int queryID = query->ID;

	/* Put this operation in the pool as it may take a short while, and build the reply back in the main
	 * thread.
	 */
	g_Scheduler.Pool([this, simID, queryID]() {

		// Retrieve all of the actual items
			std::vector<CS::CreditShopItem> allItems;
			g_CreditShopManager.GetItems(allItems);

			SimulatorThread *sim = g_SimulatorManager.GetPtrByID(simID);
			if(sim == NULL) {
				// Lost simulator
				return;
			}

			// Filter and sort them back on the main thread
			std::vector<CS::CreditShopItem> items;
			std::map<int, std::string> names;

			int wpos = 0;
			wpos += PutByte(&sim->SendBuf[wpos], 1);//_handleQueryResultMsg
			wpos += PutShort(&sim->SendBuf[wpos], 0);//Message size
			wpos += PutInteger(&sim->SendBuf[wpos], queryID);//Query response index

			wpos += PutShort(&sim->SendBuf[wpos], 0);

			// First row has cost of name change
			wpos += PutByte(&sim->SendBuf[wpos], 1);
			sprintf(sim->Aux1, "%d", g_GameConfig.NameChangeCost);
			wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

			// Track maximum sold, to calculate what is 'HOT'.
			unsigned int maxSold = 0;

			// Get a list we can sort
			for (auto it = allItems.begin();
					it != allItems.end(); ++it) {

				if (g_ServerTime < (it->mStartDate * 1000UL))
				// Not available yet
				continue;

				if(it->mQuantitySold > maxSold)
					maxSold = it->mQuantitySold;

				ItemDef * item = it->mItemId != 0 ? g_ItemManager.GetSafePointerByID(it->mItemId) : NULL;
				if (item != NULL) {

					std::string name;
					if (it->mTitle.size() == 0)
					name = item->mDisplayName;
					else
					name = it->mTitle;
					names[it->mId] = name;
				}
				else {
					names[it->mId] = it->mTitle;
				}
				items.push_back(*it);

			}
			std::sort(items.begin(), items.end(),
					[&](CS::CreditShopItem a, CS::CreditShopItem b) -> bool {
						return names[a.mId] < names[b.mId];
					});

			unsigned int hotAmount = max(1, (int)((float)maxSold * 0.75));

			for (auto it = items.begin(); it != items.end(); ++it) {
				wpos += PutByte(&sim->SendBuf[wpos], 12);

				sprintf(sim->Aux1, "%d", (*it).mId);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);
				std::string name = names[(*it).mId];
				g_Logs.cs->debug("Sending name: %v", name);

				sprintf(sim->Aux1, "%s", name.c_str());
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%s", (*it).mDescription.c_str());
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				// TODO get rid of choosable status in market editor
				if((*it).mQuantitySold >= hotAmount) {
					sprintf(sim->Aux1, "%s", Status::GetNameByID(Status::HOT).c_str());
				}
				else {
					time_t expire = ( g_GameConfig.MaxNewCreditShopItemDays * 86400 ) + (*it).mCreatedDate;
					if((*it).mCreatedDate > 0 && (time_t)(g_ServerTime / 1000UL) < expire) {
						sprintf(sim->Aux1, "%s", Status::GetNameByID(Status::NEW).c_str());
					}
					else {
						sprintf(sim->Aux1, "%s", Status::GetNameByID(Status::UNDEFINED).c_str());
					}
				}
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%s", Category::GetNameByID((*it).mCategory).c_str());
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%s",
						(*it).mStartDate == 0 ?
						"" : Util::FormatDate(&(*it).mEndDate).c_str());
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%s",
						(*it).mEndDate == 0 ?
						"" :
						(g_ServerTime >= ((*it).mEndDate * 1000UL) ?
								"Expired" :
								Util::FormatDate(&(*it).mEndDate).c_str()));
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				if ((*it).mPriceCurrency == Currency::COPPER) {
					sprintf(sim->Aux1, "%lu", (*it).mPriceCopper);
				}
				else if ((*it).mPriceCurrency == Currency::CREDITS) {
					sprintf(sim->Aux1, "%lu", (*it).mPriceCredits);
				}
				else if ((*it).mPriceCurrency == Currency::COPPER_CREDITS) {
					sprintf(sim->Aux1, "%lu+%lu", (*it).mPriceCopper,
							(*it).mPriceCredits);
				}
				else {
					sprintf(sim->Aux1, "999999");
				}

				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%d", (*it).mPriceCurrency);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%d", (*it).mQuantityLimit);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%d", (*it).mQuantitySold);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

				sprintf(sim->Aux1, "%d:%d:%d:%d", (*it).mItemId, (*it).mLookId,
						(*it).mIv1, (*it).mIv2);
				wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux1);

			}
			PutShort(&sim->SendBuf[7], items.size() + 1);
			PutShort(&sim->SendBuf[1], wpos - 3);
			sim->AttemptSend(sim->SendBuf, wpos);

		});

	return 0;
}

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
	CreditShopItem csItem = g_CreditShopManager.GetItem(id);
	if (csItem.mId == 0) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"No such item.");
	} else {
		int errCode = g_CreditShopManager.ValidateItem(&csItem, pld->accPtr,
				&creatureInstance->css, creatureInstance->charPtr);
		if (errCode != CreditShopError::NONE) {
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					CreditShopError::GetDescription(errCode).c_str());
		}
		ItemDef *itemDef = g_ItemManager.GetSafePointerByID(csItem.mItemId);
		if (itemDef == NULL)
			return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
					"No such item!");

		InventorySlot *sendSlot =
				creatureInstance->charPtr->inventory.AddItem_Ex(INV_CONTAINER,
						itemDef->mID, csItem.mIv1 + 1);
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
		creatureInstance->charPtr->pendingChanges++;
		sim->ActivateActionAbilities(sendSlot);

		g_CharacterManager.GetThread("Simulator::MarketBuy");

		if (csItem.mPriceCurrency == Currency::COPPER
				|| csItem.mPriceCurrency == Currency::COPPER_CREDITS) {
			creatureInstance->css.copper -= csItem.mPriceCopper;
			creatureInstance->SendStatUpdate(STAT::COPPER);
		}

		if (csItem.mPriceCurrency == Currency::CREDITS
				|| csItem.mPriceCurrency == Currency::COPPER_CREDITS) {
			creatureInstance->css.credits -= csItem.mPriceCredits;
			if (g_GameConfig.UseAccountCredits) {
				pld->accPtr->Credits = creatureInstance->css.credits;
				pld->accPtr->PendingMinorUpdates++;
			}
			creatureInstance->SendStatUpdate(STAT::CREDITS);
		}

		csItem.mQuantitySold++;
		g_CreditShopManager.UpdateItem(&csItem);

		int wpos = 0;
		wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux2, sendSlot);
		wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID,
				"OK");

		g_CharacterManager.ReleaseThread();

		g_Logs.cs->info("%v (%v) purchased %v (%v)", pld->accPtr->Name,
				pld->charPtr->cdef.css.display_name, csItem.mId,
				csItem.mItemId);

		return wpos;
	}
}

