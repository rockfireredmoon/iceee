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

#include "AuctionHouseHandlers.h"
#include "../ByteBuffer.h"
#include "../Config.h"
#include "../Chat.h"
#include "../Instance.h"
#include "../AuctionHouse.h"
#include "../Cluster.h"
#include "../StringUtil.h"
#include <algorithm>

using namespace std;

//
// AuctionHouseContentsHandler
//

int AuctionHouseContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 12)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	AuctionHouseSearch search;
	search.mItemTypeId = query->GetInteger(1);
	search.mQualityId = query->GetInteger(2);
	search.mOrder = query->GetInteger(3);
	search.mSearch = query->GetString(4);
	search.mBuyPriceCopperStart = query->GetLong(5);
	search.mBuyPriceCopperEnd = query->GetLong(6);
	search.mBuyPriceCreditsStart = query->GetLong(7);
	search.mBuyPriceCreditsEnd = query->GetLong(8);
	search.mLevelStart = query->GetInteger(9);
	search.mLevelEnd = query->GetInteger(10);
	search.mReverse = query->GetBool(11);

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(query->GetInteger(0));
	if (auctioneerInstance == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");
	}
	search.mAuctioneer = auctioneerInstance->CreatureDefID;

	std::vector<AuctionHouseItem> results;
	g_AuctionHouseManager.Search(search, results);

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);

	int rows = results.size();
	if (rows > 511) {
		rows = 511;
	}

	wpos += PutShort(&sim->SendBuf[wpos], rows + 1);

	// First row contains some auction data
	wpos += PutByte(&sim->SendBuf[wpos], 2);
	Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%f", g_Config.PercentageCommisionPerHour);
	wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
	wpos += PutStringUTF(&sim->SendBuf[wpos], auctioneerInstance->css.display_name);

	int row = 0;
	for (auto it = results.begin();
			row < rows && it != results.end(); ++it, row++) {
		wpos += PutByte(&sim->SendBuf[wpos], 9);
		CharacterData *cd = g_CharacterManager.RequestCharacter((*it).mSeller, true);
		std::string sellerName = cd == NULL ? "<Missing character>" : cd->cdef.css.display_name;
		wpos += WriteAuctionItem(&sim->SendBuf[wpos], &(*it), sellerName);
	}
	PutShort(&sim->SendBuf[1], wpos - 3);
	return wpos;
}

//
// AuctionHouseAuctionHandler
//

int AuctionHouseAuctionHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 7)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	int items = pld->charPtr->inventory.containerList[AUCTION_CONTAINER].size();
	if (items < 1) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must place the items to send in your auction box.");
	}

	InventorySlot slot =
			pld->charPtr->inventory.containerList[AUCTION_CONTAINER][0];

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(atoi(query->GetString(0)));
	if (auctioneerInstance == NULL)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");

	int days = atoi(query->GetString(1));
	int hours = atoi(query->GetString(2));
	int totalHours = (days * 24) + hours;
	if (totalHours < g_Config.MinAuctionHours) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Auction too short. Must be at least %d hours.",
				g_Config.MinAuctionHours);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux2);
	}

	if (totalHours > g_Config.MaxAuctionHours) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"Auction too long. Can be at most %d hours.",
				g_Config.MaxAuctionHours);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID, sim->Aux2);
	}

	unsigned long reserveCopper = atol(query->GetString(3));
	unsigned long reserveCredits = atol(query->GetString(4));
	unsigned long buyItNowCopper = atol(query->GetString(5));
	unsigned long buyItNowCredits = atol(query->GetString(6));

	if (reserveCopper != 0 && buyItNowCopper != 0
			&& buyItNowCopper <= reserveCopper) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"If you have a reserve copper price set, it must be less than the buy-it-now price.");
	}
	if (reserveCredits != 0 && buyItNowCredits != 0
			&& buyItNowCredits <= reserveCredits) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"If you have a reserve credit price set, it must be less than the buy-it-now price.");
	}

	unsigned long copper = max(reserveCopper, buyItNowCopper);
	unsigned long credits = max(reserveCredits, buyItNowCredits);
	if (copper == 0 && credits == 0)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"You must provide either a copper and/or a credit price.");

	// Work out the commission and make sure the player has enough coin/credits
	unsigned long copperCommision = (unsigned long) ((double) copper
			* (g_Config.PercentageCommisionPerHour / 100) * totalHours);
	unsigned long creditsCommision = (unsigned long) ((double) credits
			* (g_Config.PercentageCommisionPerHour / 100) * totalHours);

	if ((int)copperCommision > creatureInstance->css.copper) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"You do not have enough copper to list this item. %lu are required, but you only have %lu.",
				copperCommision,
				creatureInstance->css.copper);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->Aux2);
	}

	if ((g_Config.AccountCredits && (int)creditsCommision > pld->accPtr->Credits)
			|| (!g_Config.AccountCredits
					&& (int)creditsCommision > creatureInstance->css.credits)) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"You do not have enough credits to list this item. %lu are required, but you only have %lu.",
				creditsCommision,
				g_Config.AccountCredits ?
						pld->accPtr->Credits :
						(int)creditsCommision > creatureInstance->css.copper);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->Aux2);
	}

	AuctionHouseItem ahItem = g_AuctionHouseManager.Auction(creatureInstance, pld, slot, copperCommision, creditsCommision, auctioneerInstance->CreatureDefID,
			reserveCopper, reserveCredits, buyItNowCopper, buyItNowCredits, days, hours);

	int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	wpos += pld->charPtr->inventory.RemoveItemsAndUpdate(AUCTION_CONTAINER,
			ahItem.mItemId, ahItem.mCount + 1, &sim->SendBuf[wpos]);
	pld->charPtr->pendingChanges++;

	return wpos;
}

//
// AuctionHouseBidHandler
//

int AuctionHouseBidHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	unsigned long copper = atol(query->GetString(1));
	unsigned long credits = atol(query->GetString(2));
	int auctionId = atoi(query->GetString(3));

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(atoi(query->GetString(0)));
	if (auctioneerInstance == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");
	}

	g_AuctionHouseManager.cs.Enter("AuctionHouseBidHandler::handleQuery");
	AuctionHouseItem item = g_AuctionHouseManager.LoadItem(auctionId);
	if (item.mId == 0) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find item.");
	}

	if (item.mAuctioneer != 0 && item.mAuctioneer != auctioneerInstance->CreatureDefID) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect auctioneer.");
	}

	if (item.IsExpired()) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot bid on expired auctions.");
	}

	bool bidOk = item.Bid(pld->CreatureDefID, copper, credits);
	g_AuctionHouseManager.cs.Leave();

	if (!bidOk) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Bid rejected.");
	}
	g_AuctionHouseManager.SaveItem(item);
	g_AuctionHouseManager.BroadcastUpdate(auctioneerInstance->CreatureID, item);
	g_ClusterManager.AuctionItemUpdated(item.mId);
	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// AuctionHouseBuyHandler
//

int AuctionHouseBuyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int auctionId = atoi(query->GetString(1));
	AuctionHouseItem item = g_AuctionHouseManager.LoadItem(auctionId);
	g_AuctionHouseManager.cs.Enter("AuctionHouseBuyHandler::handleQuery");
	if (item.mId == 0) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find item.");
	}

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(atoi(query->GetString(0)));
	if (auctioneerInstance == NULL) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");
	}

	if (item.mAuctioneer != 0 && item.mAuctioneer != auctioneerInstance->CreatureDefID) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect auctioneer.");
	}

	int errCode = g_AuctionHouseManager.ValidateItem(&item, pld->accPtr,
			&creatureInstance->css, creatureInstance->charPtr);
	if (errCode != AuctionHouseError::NONE) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				AuctionHouseError::GetDescription(errCode).c_str());
	}

	InventorySlot *sendSlot = creatureInstance->charPtr->inventory.AddItem_Ex(
			INV_CONTAINER, item.mItemId, item.mCount + 1);
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
	sendSlot->secondsRemaining = item.mSecondsRemaining;
	sendSlot->customLook = item.mLookId;


	CreatureInstance *sellerInstance = g_ActiveInstanceManager.GetPlayerCreatureByDefID(item.mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if(sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(item.mSeller, true);
		if(sellerCharacterData == NULL) {
			g_Logs.simulator->warn("Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
		}
		else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(sellerCharacterData->AccountID);
			if(sellerAccount == NULL) {
				g_Logs.simulator->warn("Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
			}
		}
	}
	else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(sellerCharacterData->AccountID);
	}

	g_AuctionHouseManager.RemoveItem(item.mId);
	g_AuctionHouseManager.cs.Leave();
	g_AuctionHouseManager.BroadcastRemovedItem(auctioneerInstance->CreatureID, item.mId);

	g_CharacterManager.GetThread("Simulator::MarketBuy");

	if (item.mBuyItNowCopper > 0) {
		creatureInstance->css.copper -= item.mBuyItNowCopper;
		creatureInstance->SendStatUpdate(STAT::COPPER);
		if(sellerCss!= NULL) {
			sellerCss->copper += item.mBuyItNowCopper;
		}
		if(sellerCharacterData != NULL)
			sellerCharacterData->pendingChanges++;
		if(sellerInstance != NULL) {
			sellerInstance->SendStatUpdate(STAT::COPPER);
		}
	}

	if (item.mBuyItNowCredits > 0) {
		creatureInstance->css.credits -= item.mBuyItNowCredits;
		if (g_Config.AccountCredits) {
			pld->accPtr->Credits = creatureInstance->css.credits;
			pld->accPtr->PendingMinorUpdates++;
			if(sellerAccount != NULL) {
				sellerAccount->Credits = sellerCss->credits;
				sellerAccount->Credits += item.mBuyItNowCredits;
				sellerAccount->PendingMinorUpdates++;
			}
		}
		else {
			if(sellerCss!= NULL) {
				sellerCss->credits += item.mBuyItNowCredits;
			}
			if(sellerCharacterData != NULL) {
				sellerCharacterData->pendingChanges++;
			}
		}
		creatureInstance->SendStatUpdate(STAT::CREDITS);
		if(sellerInstance != NULL) {
			sellerInstance->SendStatUpdate(STAT::CREDITS);
		}
	}

	int wpos = 0;
	wpos += AddItemUpdate(&sim->SendBuf[wpos], sim->Aux2, sendSlot);
	wpos += PrepExt_QueryResponseString(&sim->SendBuf[wpos], query->ID, "OK");

	g_CharacterManager.ReleaseThread();

	ItemDef *itemData = g_ItemManager.GetSafePointerByID(item.mItemId);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
			"%s bought %s at %s's auction house",
			pld->charPtr->cdef.css.display_name, itemData->mDisplayName.c_str(),
			auctioneerInstance->css.display_name);
	ChatMessage cm(sim->Aux3);
	cm.mChannelName = "tc/";
	cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
	cm.mSender = "EEBay";
	g_ChatManager.SendChatMessage(cm, NULL);

	return wpos;
}
