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
#include <algorithm>

using namespace std;

static inline std::string FormatAuctionItemProto(AuctionHouseItem *item) {
	char buf[32];
	Util::SafeFormat(buf, sizeof(buf), "item%d:%d:%d:%d", item->mItemId,
			item->mLookId, item->mCount, 0);
	return buf;
}

static inline int WriteAuctionItem(char *buffer, char *scratch,
		AuctionHouseItem *item) {
	int wpos = 0;
	Util::SafeFormat(scratch, sizeof(scratch), "%d", item->mId);
	// 1
	wpos += PutStringUTF(&buffer[wpos], scratch);
	// 2
	wpos += PutStringUTF(&buffer[wpos], FormatAuctionItemProto(item).c_str());
	// 3
	wpos += PutStringUTF(&buffer[wpos], item->mSellerName.c_str());
	Util::SafeFormat(scratch, sizeof(scratch), "%lu", item->GetTimeRemaining());
	// 4
	wpos += PutStringUTF(&buffer[wpos], scratch);
	Util::SafeFormat(scratch, sizeof(scratch), "%lu", item->mBuyItNowCopper);
	// 5
	wpos += PutStringUTF(&buffer[wpos], scratch);
	Util::SafeFormat(scratch, sizeof(scratch), "%lu", item->mBuyItNowCredits);
	// 6
	wpos += PutStringUTF(&buffer[wpos], scratch);
	Util::SafeFormat(scratch, sizeof(scratch), "%d", item->mBids.size());
	// 7
	wpos += PutStringUTF(&buffer[wpos], scratch);
	if (item->mBids.size() > 0) {
		AuctionHouseBid highBid = item->mBids[item->mBids.size() - 1];
		Util::SafeFormat(scratch, sizeof(scratch), "%lu", highBid.mCopper);
		// 8
		wpos += PutStringUTF(&buffer[wpos], scratch);
		Util::SafeFormat(scratch, sizeof(scratch), "%lu", highBid.mCredits);
		// 9
		wpos += PutStringUTF(&buffer[wpos], scratch);
	} else {
		// 8
		wpos += PutStringUTF(&buffer[wpos], "0");
		// 9
		wpos += PutStringUTF(&buffer[wpos], "0");
	}
	return wpos;
}

//
// AuctionHouseContentsHandler
//

int AuctionHouseContentsHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {
	if (query->argCount < 3)
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Invalid query.");

	int auctioneerId = query->GetInteger(0);
	int itemTypeID = query->GetInteger(1);
	int qualityID = query->GetInteger(2);

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(auctioneerId);
	if (auctioneerInstance == NULL) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");
	}

	int wpos = 0;
	wpos += PutByte(&sim->SendBuf[wpos], 1);
	wpos += PutShort(&sim->SendBuf[wpos], 0);
	wpos += PutInteger(&sim->SendBuf[wpos], query->ID);

	int rowCount = g_AuctionHouseManager.mItems.size() + 1;
	int maxRows = rowCount;
	if (maxRows > 768) {
		maxRows = 768;
	}

	wpos += PutShort(&sim->SendBuf[wpos], rowCount);

	// First row contains some auction data
	wpos += PutByte(&sim->SendBuf[wpos], 2);
	Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2), "%f", g_Config.PercentageCommisionPerHour);
	wpos += PutStringUTF(&sim->SendBuf[wpos], sim->Aux2);
	wpos += PutStringUTF(&sim->SendBuf[wpos], auctioneerInstance->css.display_name);

	int row = 1;
	for (std::map<int, AuctionHouseItem*>::iterator it =
			g_AuctionHouseManager.mItems.begin();
			it != g_AuctionHouseManager.mItems.end(); ++it) {
		if (row >= maxRows)
			break;

		if (it->second->itemDef == NULL) {
			g_Log.AddMessageFormat(
					"[WARNING] Item in auction house does not link to a valid item. %d",
					it->second->mItemId);
		} else {
			bool isForAuctioneer = it->second->mAuctioneer == 0
					|| auctioneerId == it->second->mAuctioneer;
			bool isOfType = itemTypeID == -1
					|| itemTypeID == it->second->itemDef->mType;
			bool isOfQuality = qualityID == -1
					|| qualityID == it->second->itemDef->mQualityLevel;

			if (isForAuctioneer && isOfType && isOfQuality) {
				wpos += PutByte(&sim->SendBuf[wpos], 9);
				wpos += WriteAuctionItem(&sim->SendBuf[wpos], sim->Aux2,
						it->second);
				row++;
			}
		}
	}
	PutShort(&sim->SendBuf[1], wpos - 3);
	PutShort(&sim->SendBuf[7], row);
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

	int auctioneer = atoi(query->GetString(0));

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(auctioneer);
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

	if (copperCommision > creatureInstance->css.copper) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"You do not have enough copper to list this item. %lu are required, but you only have %lu.",
				copperCommision,
				creatureInstance->css.copper);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->Aux2);
	}

	if ((g_Config.AccountCredits && creditsCommision > pld->accPtr->Credits)
			|| (!g_Config.AccountCredits
					&& creditsCommision > creatureInstance->css.copper)) {
		Util::SafeFormat(sim->Aux2, sizeof(sim->Aux2),
				"You do not have enough credits to list this item. %lu are required, but you only have %lu.",
				creditsCommision,
				g_Config.AccountCredits ?
						pld->accPtr->Credits :
						creditsCommision > creatureInstance->css.copper);
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				sim->Aux2);
	}

	if (copperCommision) {
		creatureInstance->css.copper -= copperCommision;
		creatureInstance->SendStatUpdate(STAT::COPPER);
	}

	if (creditsCommision > 0) {
		creatureInstance->css.credits -= creditsCommision;
		if (g_Config.AccountCredits) {
			pld->accPtr->Credits = creatureInstance->css.credits;
			pld->accPtr->PendingMinorUpdates++;
		}
		creatureInstance->SendStatUpdate(STAT::CREDITS);
	}

	AuctionHouseItem *ahItem = new AuctionHouseItem();
	ahItem->mId = g_AuctionHouseManager.nextAuctionHouseItemID++;
	ahItem->mAuctioneer = auctioneer;
	ahItem->mReserveCopper = reserveCopper;
	ahItem->mReserveCredits = reserveCredits;
	ahItem->mBuyItNowCopper = buyItNowCopper;
	ahItem->mBuyItNowCredits = buyItNowCredits;
	ahItem->mStartDate = time(NULL);
	ahItem->mEndDate = ahItem->mStartDate + (hours * 60 * 60)
			+ (days * 60 * 60 * 24);
	ahItem->mSeller = pld->CreatureDefID;
	ahItem->mItemId = slot.IID;
	ahItem->mLookId = slot.customLook;
	ahItem->mCount = slot.count;

	SessionVarsChangeData.AddChange();

	g_AuctionHouseManager.SaveItem(ahItem);
	g_TimerManager.AddTask(new AuctionTimerTask(ahItem));

	int wpos = PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
	wpos += pld->charPtr->inventory.RemoveItemsAndUpdate(AUCTION_CONTAINER,
			ahItem->mItemId, ahItem->mCount + 1, &sim->SendBuf[wpos]);

	// Broadcast
	int wpos2 = 0;
	wpos2 += PutByte(&sim->Aux2[wpos2], 97);
	wpos2 += PutShort(&sim->Aux2[wpos2], 0);
	wpos2 += PutByte(&sim->Aux2[wpos2], 1);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", auctioneer);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	wpos2 += WriteAuctionItem(&sim->Aux2[wpos2], sim->Aux3, ahItem);
	PutShort(&sim->Aux2[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(sim->Aux2, wpos2, NULL);

	ItemDef *item = g_ItemManager.GetSafePointerByID(ahItem->mItemId);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3),
			"%s is selling %s at %s's auction house",
			pld->charPtr->cdef.css.display_name, item->mDisplayName.c_str(),
			auctioneerInstance->css.display_name);
	ChatMessage cm(sim->Aux3);
	cm.mChannelName = "tc/";
	cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
	cm.mSender = "EEBay";
	g_ChatManager.SendChatMessage(cm, NULL);

	return wpos;
}

//
// AuctionHouseBidHandler
//

int AuctionHouseBidHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int auctioneer = atoi(query->GetString(0));
	unsigned long copper = atol(query->GetString(1));
	unsigned long credits = atol(query->GetString(2));
	int auctionId = atoi(query->GetString(3));

	g_AuctionHouseManager.cs.Enter("AuctionHouseBidHandler::handleQuery");
	if (g_AuctionHouseManager.mItems.find(auctionId)
			== g_AuctionHouseManager.mItems.end()) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find item.");
	}

	AuctionHouseItem *item = g_AuctionHouseManager.mItems[auctionId];
	if (item->mAuctioneer != 0 && item->mAuctioneer != auctioneer) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect auctioneer.");
	}

	if (item->IsExpired()) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Cannot bid on expired auctions.");
	}

	bool bidOk = item->Bid(pld->CreatureDefID, copper, credits);

	g_AuctionHouseManager.SaveItem(item);

	g_AuctionHouseManager.cs.Leave();

	if (!bidOk) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Bid rejected.");
	}

	// Broadcast
	int wpos2 = 0;
	wpos2 += PutByte(&sim->Aux2[wpos2], 97);
	wpos2 += PutShort(&sim->Aux2[wpos2], 0);
	wpos2 += PutByte(&sim->Aux2[wpos2], 2);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", auctioneer);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", item->mId);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%lu",
			item->GetTimeRemaining());
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", item->mBids.size());
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	AuctionHouseBid highBid = item->mBids[item->mBids.size() - 1];
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%lu", highBid.mCopper);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%lu", highBid.mCredits);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	PutShort(&sim->Aux2[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(sim->Aux2, wpos2, NULL);

	return PrepExt_QueryResponseString(sim->SendBuf, query->ID, "OK");
}

//
// AuctionHouseBuyHandler
//

int AuctionHouseBuyHandler::handleQuery(SimulatorThread *sim,
		CharacterServerData *pld, SimulatorQuery *query,
		CreatureInstance *creatureInstance) {

	int auctioneer = atoi(query->GetString(0));
	int auctionId = atoi(query->GetString(1));

	g_AuctionHouseManager.cs.Enter("AuctionHouseBuyHandler::handleQuery");
	if (g_AuctionHouseManager.mItems.find(auctionId)
			== g_AuctionHouseManager.mItems.end()) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Could not find item.");
	}

	CreatureInstance *auctioneerInstance =
			creatureInstance->actInst->GetNPCInstanceByCID(auctioneer);
	if (auctioneerInstance == NULL) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Unknown auctioneer.");
	}

	AuctionHouseItem *item = g_AuctionHouseManager.mItems[auctionId];
	if (item->mAuctioneer != 0 && item->mAuctioneer != auctioneer) {
		g_AuctionHouseManager.cs.Leave();
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				"Incorrect auctioneer.");
	}

	int errCode = g_AuctionHouseManager.ValidateItem(item, pld->accPtr,
			&creatureInstance->css, creatureInstance->charPtr);
	if (errCode != AuctionHouseError::NONE) {
		return PrepExt_QueryResponseError(sim->SendBuf, query->ID,
				AuctionHouseError::GetDescription(errCode).c_str());
	}

	InventorySlot *sendSlot = creatureInstance->charPtr->inventory.AddItem_Ex(
			INV_CONTAINER, item->mItemId, item->mCount + 1);
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


	CreatureInstance *sellerInstance = g_ActiveInstanceManager.GetPlayerCreatureByDefID(item->mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if(sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(item->mSeller, true);
		if(sellerCharacterData == NULL) {
			g_Log.AddMessageFormat("[WARNING] Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
		}
		else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(sellerCharacterData->AccountID);
			if(sellerAccount == NULL) {
				g_Log.AddMessageFormat("[WARNING] Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
			}
		}
	}
	else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(sellerCharacterData->AccountID);
	}

	g_AuctionHouseManager.RemoveItem(item->mId);
	g_AuctionHouseManager.cs.Leave();

	// Broadcast
	int wpos2 = 0;
	wpos2 += PutByte(&sim->Aux2[wpos2], 97);
	wpos2 += PutShort(&sim->Aux2[wpos2], 0);
	wpos2 += PutByte(&sim->Aux2[wpos2], 3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", auctioneer);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	Util::SafeFormat(sim->Aux3, sizeof(sim->Aux3), "%d", item->mId);
	wpos2 += PutStringUTF(&sim->Aux2[wpos2], sim->Aux3);
	PutShort(&sim->Aux2[1], wpos2 - 3);
	g_SimulatorManager.SendToAllSimulators(sim->Aux2, wpos2, NULL);

	g_CharacterManager.GetThread("Simulator::MarketBuy");

	if (item->mBuyItNowCopper > 0) {
		creatureInstance->css.copper -= item->mBuyItNowCopper;
		creatureInstance->SendStatUpdate(STAT::COPPER);
		if(sellerCss!= NULL) {
			sellerCss->copper += item->mBuyItNowCopper;
		}
		if(sellerCharacterData != NULL)
			sellerCharacterData->pendingChanges++;
		if(sellerInstance != NULL) {
			sellerInstance->SendStatUpdate(STAT::COPPER);
		}
	}

	if (item->mBuyItNowCredits > 0) {
		creatureInstance->css.credits -= item->mBuyItNowCredits;
		if (g_Config.AccountCredits) {
			pld->accPtr->Credits = creatureInstance->css.credits;
			pld->accPtr->PendingMinorUpdates++;
			if(sellerAccount != NULL) {
				sellerAccount->Credits = sellerCss->credits;
				sellerAccount->Credits += item->mBuyItNowCredits;
				sellerAccount->PendingMinorUpdates++;
			}
		}
		else {
			if(sellerCss!= NULL) {
				sellerCss->credits += item->mBuyItNowCredits;
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

	ItemDef *itemData = g_ItemManager.GetSafePointerByID(item->mItemId);
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
