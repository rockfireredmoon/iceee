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

#include "AuctionHouse.h"
#include "Config.h"
#include "Util.h"
#include "FileReader.h"
#include "DirectoryAccess.h"
#include "ByteBuffer.h"

#include "Simulator.h"
#include "Item.h"
#include "Instance.h"
#include "Chat.h"
#include <string.h>
#include <algorithm>

AuctionHouseManager g_AuctionHouseManager;

namespace AuctionHouseError {
std::string GetDescription(int id) {
	switch (id) {
	case NONE:
		return "No error";
	case SOLD_OUT:
		return "Sold out!";
	case NOT_ENOUGH_COPPER:
		return "You do not have enough copper!";
	case NOT_ENOUGH_CREDITS:
		return "You do not have enough credits!";
	case NOT_YET_AVAILABLE:
		return "This item is not yet available!";
	case NO_LONGER_AVAILABLE:
		return "This item is no longer available!";
	case NOT_ENOUGH_FREE_SLOTS:
		return "You do not have enough slots in your inventory!";
	case SERVER_ERROR:
		return "Internal error.";
	}
	return "<undefined>";
}
}

//
// AuctionHouseBid
//

AuctionHouseBid::AuctionHouseBid() {
	mBidTime = 0;
	mBuyer = 0;
	mCopper = 0;
	mCredits = 0;
}

AuctionHouseBid::~AuctionHouseBid() {
}

void AuctionHouseBid::WriteToJSON(Json::Value &value) {
	value["buyer"] = mBuyer;
	value["copper"] = Json::UInt64(mCopper);
	value["credits"] = Json::UInt64(mCredits);
	value["time"] = Json::UInt64(mBidTime);
}

//
// AuctionTimerTask
//
AuctionTimerTask::AuctionTimerTask(AuctionHouseItem *item) {
	mItem = item;
	mWhen = item->mEndDate;
}

void AuctionTimerTask::run() {
	char buf[1024];
	char buf2[256];

	// Any bids at all?
	bool success = false;
	bool reserveReached = true;

	g_AuctionHouseManager.cs.Enter("AuctionTimerTask::run");

	// Get seller objects

	CreatureInstance *sellerInstance =
			g_ActiveInstanceManager.GetPlayerCreatureByDefID(mItem->mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if (sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(
				mItem->mSeller, true);
		if (sellerCharacterData == NULL) {
			g_Logs.server->warn("Seller of auction item, no longer exists. Received copper and credits go to /dev/null!");
		} else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(
					sellerCharacterData->AccountID);
			if (sellerAccount == NULL) {
				g_Logs.server->warn(
						"Seller account of auction item, no longer exists. Received copper and credits go to /dev/null!");
			}
		}
	} else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(
				sellerCharacterData->AccountID);
	}

	// Auctioneer
	CreatureDefinition *auctioneer = CreatureDef.GetPointerByCDef(
			mItem->mAuctioneer);

	// Find the best bid and process it
	for (std::vector<AuctionHouseBid>::reverse_iterator it =
			mItem->mBids.rbegin(); it != mItem->mBids.rend(); ++it) {

		// Was reserve not reached
		if ((mItem->mReserveCopper != 0 && it->mCopper < mItem->mReserveCopper)
				|| (mItem->mReserveCredits != 0
						&& it->mCredits < mItem->mReserveCredits)) {
			success = true;
			break;
		} else
			reserveReached = false;

		// Look for an instance of the bidders character stat set or the offline players character stat set
		CreatureInstance *creatureInstance =
				g_ActiveInstanceManager.GetPlayerCreatureByDefID(it->mBuyer);
		CharacterStatSet *css;
		AccountData *account;
		CharacterData *characterData = NULL;
		if (creatureInstance == NULL) {
			characterData = g_CharacterManager.RequestCharacter(it->mBuyer,
					true);
			if (characterData == NULL) {
				// Creature now deleted?
				continue;
			}
			css = &characterData->cdef.css;
			account = g_AccountManager.FetchIndividualAccount(
					characterData->AccountID);
			if (account == NULL)
				// Account now deleted? (shouldn't happen)
				continue;
		} else {
			characterData = creatureInstance->charPtr;
			css = &creatureInstance->css;
			account = g_AccountManager.GetActiveAccountByID(
					characterData->AccountID);
		}
		InventoryManager *inventory = &characterData->inventory;

		// Check the prospective recipient has enough copper/credits/inventory space
		if (g_Config.AccountCredits) {
			css->credits = account->Credits;
		}

		// Check the prospective recipient inventory space
		int slot = inventory->GetFreeSlot(INV_CONTAINER);
		if (slot == -1) {
			Util::SafeFormat(buf, sizeof(buf),
					"Your bid on '%s' failed because you do not have enough inventory space. Item goes to next bidder or back to seller.",
					mItem->itemDef->mDisplayName.c_str());
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender = "EEBay";
			cm.mTell = true;
			cm.mRecipient = css->display_name;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						g_AuctionHouseManager.session);
			}
			continue;
		}

		// Check the prospective recipient has enough copper
		if (it->mCopper > 0 && it->mCopper > css->copper) {

			Util::SafeFormat(buf, sizeof(buf),
					"Your bid on '%s' failed because you do not have enough copper. Item goes to next bidder or back to seller.",
					mItem->itemDef->mDisplayName.c_str());
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			cm.mTell = true;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						g_AuctionHouseManager.session);
			}
			continue;
		}

		// Check the prospective recipient has enough credits
		if (it->mCredits > 0 && it->mCredits > css->credits) {
			Util::SafeFormat(buf, sizeof(buf),
					"Your bid on '%s' failed because you do not have enough credits. Item goes to next bidder or back to seller.",
					mItem->itemDef->mDisplayName.c_str());
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			cm.mTell = true;
			cm.mRecipient = css->display_name;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						g_AuctionHouseManager.session);
			}
			continue;
		}

		// Nearly there! Now try to add item to recipients inventory
		InventorySlot *sendSlot = inventory->AddItem_Ex(INV_CONTAINER,
				mItem->mItemId, mItem->mCount + 1);
		if (sendSlot == NULL) {
			int err = creatureInstance->charPtr->inventory.LastError;
			std::string errText = "Server error: undefined error.";
			if (err == InventoryManager::ERROR_ITEM)
				errText = "Server error: item does not exist.";
			else if (err == InventoryManager::ERROR_SPACE)
				errText = "You do not have any free inventory space.";
			else if (err == InventoryManager::ERROR_LIMIT)
				errText = "You already the maximum amount of these items.";
		} else {
			sendSlot->secondsRemaining = mItem->mSecondsRemaining;
			sendSlot->customLook = mItem->mLookId;

			// Update recipients inventory
			if (creatureInstance != NULL
					&& creatureInstance->simulatorPtr != NULL) {
				int wpos = AddItemUpdate(buf, buf2, sendSlot);
				creatureInstance->simulatorPtr->AttemptSend(buf, wpos);
			}

			// Take copper from recipient and give to seller
			if (it->mCopper > 0) {
				css->copper -= it->mCopper;
				characterData->pendingChanges++;
				if (creatureInstance != NULL) {
					creatureInstance->SendStatUpdate(STAT::COPPER);
				}

				if (sellerCss != NULL) {
					sellerCss->copper += it->mCopper;
					if (sellerCharacterData != NULL) {
						sellerCharacterData->pendingChanges++;
					}
					if (sellerInstance != NULL) {
						sellerInstance->SendStatUpdate(STAT::COPPER);
					}
				}
			}

			// Take credits from recipient and give to seller
			if (it->mCredits > 0) {
				css->credits -= it->mCredits;
				if (g_Config.AccountCredits)
					account->PendingMinorUpdates++;
				else
					characterData->pendingChanges++;
				if (creatureInstance != NULL) {
					creatureInstance->SendStatUpdate(STAT::CREDITS);
				}

				if (sellerCss != NULL) {
					sellerCss->credits += it->mCredits;
					if (g_Config.AccountCredits && sellerAccount != NULL)
						sellerAccount->PendingMinorUpdates++;
					else if (sellerCharacterData != NULL) {
						sellerCharacterData->pendingChanges++;
					}
					if (sellerInstance != NULL) {
						sellerInstance->SendStatUpdate(STAT::CREDITS);
					}
				}
			}

			Util::SafeFormat(buf, sizeof(buf),
					"%s is the winning bidder of '%s'!", css->display_name,
					mItem->itemDef->mDisplayName.c_str());
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
			}

			success = true;
			break;
		}

	}

	if (!success) {

		Util::SafeFormat(buf, sizeof(buf),
				"Your auction item '%s' failed to attract any bids. You have %d hours to remove it from the auction before it is automatically removed and placed in your vendor buyback inventory.",
				mItem->itemDef->mDisplayName.c_str(),
				g_Config.MaxAuctionExpiredHours);
		ChatMessage cm(buf);
		cm.mChannelName = "tc/";
		cm.mRecipient = sellerCss->display_name;
		cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
		cm.mSender =
				auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
		cm.mTell = true;
		if (!g_ChatManager.SendChatMessage(cm, NULL)) {
			g_ChatManager.SendChatMessageAsOffline(cm,
					g_AuctionHouseManager.session);
		}
		mItem->mCompleted = true;
		g_AuctionHouseManager.SaveItem(mItem);
		AuctionRemoveTimerTask *tr = new AuctionRemoveTimerTask(mItem);
		mItem->timerTask = tr;
		g_TimerManager.AddTask(tr);
		g_AuctionHouseManager.cs.Leave();
	} else {
		if (!reserveReached) {
			Util::SafeFormat(buf, sizeof(buf),
					"Your auction item '%s' failed to reach the reserve price. You have %d hours to remove it from the auction before it is automatically removed and placed in your vendor buyback inventory.",
					mItem->itemDef->mDisplayName.c_str(),
					g_Config.MaxAuctionExpiredHours);
			ChatMessage cm(buf);
			cm.mChannelName = "tc/";
			cm.mRecipient = sellerCss->display_name;
			cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
			cm.mSender =
					auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
			cm.mTell = true;
			if (!g_ChatManager.SendChatMessage(cm, NULL)) {
				g_ChatManager.SendChatMessageAsOffline(cm,
						g_AuctionHouseManager.session);
			}
			mItem->mCompleted = true;
			g_AuctionHouseManager.SaveItem(mItem);
			AuctionRemoveTimerTask *tr = new AuctionRemoveTimerTask(mItem);
			mItem->timerTask = tr;
			g_TimerManager.AddTask(tr);
			g_AuctionHouseManager.cs.Leave();
		} else {

			mItem->timerTask = NULL;

			g_AuctionHouseManager.RemoveItem(mItem->mId);
			g_AuctionHouseManager.cs.Leave();

			/* Find the active auctioneer instance. If there isn't one, then nobody can be standing at an auction house,
			 * so there is no need to broadcast the change
			 */
			CreatureInstance *instance =
					g_ActiveInstanceManager.GetNPCCreatureByDefID(
							mItem->mAuctioneer);
			if (instance != NULL) {

				// Broadcast
				int wpos2 = 0;
				wpos2 += PutByte(&buf[wpos2], 97);
				wpos2 += PutShort(&buf[wpos2], 0);
				wpos2 += PutByte(&buf[wpos2], 3);
				Util::SafeFormat(buf2, sizeof(buf2), "%d", instance->CreatureID);
				wpos2 += PutStringUTF(&buf[wpos2], buf2);
				Util::SafeFormat(buf2, sizeof(buf2), "%d", mItem->mId);
				wpos2 += PutStringUTF(&buf[wpos2], buf2);
				PutShort(&buf[1], wpos2 - 3);
				g_SimulatorManager.SendToAllSimulators(buf, wpos2, NULL);
			}
		}
	}

}

//
// AuctionRemoveTimerTask
//
AuctionRemoveTimerTask::AuctionRemoveTimerTask(AuctionHouseItem *item) {
	mItem = item;
	mWhen = item->mEndDate + (g_Config.MaxAuctionExpiredHours * 3600);
}

void AuctionRemoveTimerTask::run() {
	char buf[1024];
	char buf2[256];

	g_Logs.event->info(
			"Removing auction house item '%v' (sold by %v) from auction list",
			mItem->itemDef->mDisplayName.c_str(), mItem->mSellerName.c_str());

	g_AuctionHouseManager.cs.Enter("AuctionTimerTask::run");

	// Get seller objects

	CreatureInstance *sellerInstance =
			g_ActiveInstanceManager.GetPlayerCreatureByDefID(mItem->mSeller);
	CharacterStatSet *sellerCss = NULL;
	AccountData *sellerAccount = NULL;
	CharacterData *sellerCharacterData = NULL;
	if (sellerInstance == NULL) {
		sellerCharacterData = g_CharacterManager.RequestCharacter(
				mItem->mSeller, true);
		if (sellerCharacterData == NULL) {
			g_Logs.server->warn("Seller of auction item, no longer exists.");
		} else {
			sellerCss = &sellerCharacterData->cdef.css;
			sellerAccount = g_AccountManager.FetchIndividualAccount(
					sellerCharacterData->AccountID);
			if (sellerAccount == NULL) {
				g_Logs.server->warn("Seller account of auction item, no longer exists.");
			}
		}
	} else {
		sellerCharacterData = sellerInstance->charPtr;
		sellerCss = &sellerInstance->css;
		sellerAccount = g_AccountManager.GetActiveAccountByID(
				sellerCharacterData->AccountID);
	}

	// Auctioneer
	CreatureInstance *auctioneerInstance =
			g_ActiveInstanceManager.GetNPCCreatureByDefID(
					mItem->mAuctioneer);
	if(auctioneerInstance == NULL) {
		g_Logs.server->warn("No auctioneer instance for ID %v", mItem->mAuctioneer);
	}
	CreatureDefinition *auctioneer = auctioneerInstance == NULL ? CreatureDef.GetPointerByCDef(
			mItem->mAuctioneer) : &auctioneerInstance->charPtr->cdef;

	if(auctioneer == NULL) {
		g_Logs.server->warn("No auctioneer object for ID %v", mItem->mAuctioneer);
	}

	if (sellerCss != NULL) {
		Util::SafeFormat(buf, sizeof(buf),
				"Your expired auction item '%s' has been in the auction house too long and has been removed. You may retrieve it from your buyback.",
				mItem->itemDef->mDisplayName.c_str());
		ChatMessage cm(buf);
		cm.mChannelName = "tc/";
		cm.mRecipient = sellerCss->display_name;
		cm.mChannel = GetChatInfoByChannel(cm.mChannelName.c_str());
		cm.mSender =
				auctioneer == NULL ? "EEBay" : auctioneer->css.display_name;
		cm.mTell = true;
		if (!g_ChatManager.SendChatMessage(cm, NULL)) {
			g_ChatManager.SendChatMessageAsOffline(cm,
					g_AuctionHouseManager.session);
		}

		if (sellerCharacterData != NULL) {
			int wpos = 0;
			InventorySlot tempItem;
			tempItem.IID = mItem->mItemId;
			tempItem.dataPtr = mItem->itemDef;
			tempItem.count = mItem->mCount;
			tempItem.secondsRemaining = mItem->mSecondsRemaining;
			tempItem.customLook = mItem->mLookId;
			wpos += sellerCharacterData->inventory.AddBuyBack(&tempItem,
					&buf[wpos]);
			sellerCharacterData->pendingChanges++;
			if (sellerInstance != NULL && sellerInstance->simulatorPtr != NULL) {
				sellerInstance->simulatorPtr->AttemptSend(buf, wpos);
			}
		}

	}

	mItem->timerTask = NULL;

	if (auctioneerInstance != NULL) {

		// Broadcast
		int wpos2 = 0;
		wpos2 += PutByte(&buf[wpos2], 97);
		wpos2 += PutShort(&buf[wpos2], 0);
		wpos2 += PutByte(&buf[wpos2], 3);
		Util::SafeFormat(buf2, sizeof(buf2), "%d", auctioneerInstance->CreatureID);
		wpos2 += PutStringUTF(&buf[wpos2], buf2);
		Util::SafeFormat(buf2, sizeof(buf2), "%d", mItem->mId);
		wpos2 += PutStringUTF(&buf[wpos2], buf2);
		PutShort(&buf[1], wpos2 - 3);

		g_AuctionHouseManager.RemoveItem(mItem->mId);
		g_AuctionHouseManager.cs.Leave();
		g_SimulatorManager.SendToAllSimulators(buf, wpos2, NULL);
	} else {
		g_AuctionHouseManager.RemoveItem(mItem->mId);
		g_AuctionHouseManager.cs.Leave();
	}
}

//
// AuctionHouseItem
//

AuctionHouseItem::AuctionHouseItem() {

	mId = 0;
	mStartDate = 0;
	mEndDate = 0;
	mItemId = 0;
	mLookId = 0;
	mCount = 0;
	mReserveCopper = 0;
	mReserveCredits = 0;
	mBuyItNowCopper = 0;
	mBuyItNowCredits = 0;
	mSeller = 0;
	mAuctioneer = 0;
	itemDef = NULL;
	mSellerName = "";
	timerTask = NULL;
	mCompleted = false;
	mSecondsRemaining = 0;
}

AuctionHouseItem::~AuctionHouseItem() {
}

bool AuctionHouseItem::IsExpired() {
	return mEndDate - time(NULL) <= 0;
}

unsigned long AuctionHouseItem::GetTimeRemaining() {
	long timeRemaining = mEndDate - time(NULL);
	if (timeRemaining < 0)
		timeRemaining = 0;
	return timeRemaining;

}
bool AuctionHouseItem::Bid(int creatureDefID, unsigned long copper,
		unsigned long credits) {

	for (std::vector<AuctionHouseBid>::iterator it = mBids.begin();
			it != mBids.end(); ++it) {
		if (it->mBuyer == creatureDefID) {
			mBids.erase(it);
			break;
		}
	}
	if (mBids.size() == 0
			|| (copper > mBids[mBids.size() - 1].mCopper
					|| credits > mBids[mBids.size() - 1].mCredits)) {
		AuctionHouseBid bid;
		bid.mBuyer = creatureDefID;
		bid.mCopper = copper;
		bid.mCredits = credits;
		bid.mBidTime = time(NULL);
		mBids.push_back(bid);
		return true;
	}
	return false;
}

void AuctionHouseItem::WriteToJSON(Json::Value &value) {
	value["id"] = mId;
	value["begin"] = Json::UInt64(mStartDate);
	value["end"] = Json::UInt64(mEndDate);
	value["remain"] = Json::Int64(mEndDate - time(NULL));
	value["seller"] = mSeller;
	value["auctioneer"] = mAuctioneer;
	value["reserveCopper"] = Json::UInt64(mReserveCopper);
	value["reserveCredits"] = Json::UInt64(mReserveCredits);
	value["buyItNowCopper"] = Json::UInt64(mBuyItNowCopper);
	value["buyItNowCredits"] = Json::UInt64(mBuyItNowCredits);
	value["itemID"] = mItemId;
	value["lookID"] = mLookId;
	value["count"] = mCount;

	Json::Value bids(Json::arrayValue);

	for (std::vector<AuctionHouseBid>::iterator it = mBids.begin();
			it != mBids.end(); ++it) {
		Json::Value v;
		it->WriteToJSON(v);
		bids.append(v);
	}
	value["bids"] = bids;
}

//
// AuctionHouseManager
//

AuctionHouseManager::AuctionHouseManager() {
	nextAuctionHouseItemID = 1;
	session = NULL;
}

AuctionHouseManager::~AuctionHouseManager() {
}

int AuctionHouseManager::ValidateItem(AuctionHouseItem *ahItem,
		AccountData *accPtr, CharacterStatSet *css, CharacterData *cd) {

	if ((unsigned long) css->copper < ahItem->mBuyItNowCopper)
		return AuctionHouseError::NOT_ENOUGH_COPPER;

	if (g_Config.AccountCredits) {
		css->credits = accPtr->Credits;
	}
	if ((unsigned long) css->credits < ahItem->mBuyItNowCredits)
		return AuctionHouseError::NOT_ENOUGH_CREDITS;

	unsigned long nowTimeSec = time(NULL);
	if (ahItem->mStartDate != 0 && nowTimeSec < (ahItem->mStartDate))
		return AuctionHouseError::NOT_YET_AVAILABLE;

	if (ahItem->mEndDate != 0 && nowTimeSec >= (ahItem->mEndDate))
		return AuctionHouseError::NO_LONGER_AVAILABLE;

	int slot = cd->inventory.GetFreeSlot(INV_CONTAINER);
	if (slot == -1)
		return AuctionHouseError::NOT_ENOUGH_FREE_SLOTS;

	return AuctionHouseError::NONE;
}

bool AuctionHouseManager::SaveItem(AuctionHouseItem * item) {

	item->itemDef = g_ItemManager.GetSafePointerByID(item->mItemId);
	CharacterData *seller = g_CharacterManager.RequestCharacter(item->mSeller,
			true);
	if (seller == NULL) {
		item->mSellerName = "<Missing character>";
	} else {
		item->mSellerName = seller->cdef.css.display_name;
	}
	mItems[item->mId] = item;

	std::string path = GetPath(item->mId);
	g_Logs.data->info("Saving credit shop item to %v.", path.c_str());
	FILE *output = fopen(path.c_str(), "wb");
	if (output == NULL) {
		g_Logs.data->warn("Saving petition could not open: %v", path.c_str());
		return false;
	}

	fprintf(output, "[ENTRY]\r\n");
	fprintf(output, "BeginDate=%lld\r\n", (long long) (item->mStartDate));
	fprintf(output, "EndDate=%lld\r\n", (long long) (item->mEndDate));
	fprintf(output, "Seller=%d\r\n", item->mSeller);
	fprintf(output, "Auctioneer=%d\r\n", item->mAuctioneer);
	fprintf(output, "ReserveCopper=%lu\r\n", item->mReserveCopper);
	fprintf(output, "ReserveCredits=%lu\r\n", item->mReserveCredits);
	fprintf(output, "BuyItNowCopper=%lu\r\n", item->mBuyItNowCopper);
	fprintf(output, "BuyItNowCredits=%lu\r\n", item->mBuyItNowCredits);
	fprintf(output, "SecondsRemaining=%lu\r\n", item->mSecondsRemaining);
	fprintf(output, "Completed=%d\r\n", item->mCompleted ? 1 : 0);
	fprintf(output, "ItemID=%d\r\n", item->mItemId);
	if (item->mCount > 0)
		fprintf(output, "Count=%d\r\n", item->mCount);
	if (item->mLookId > 0)
		fprintf(output, "LookId=%d\r\n", item->mLookId);

	for (std::vector<AuctionHouseBid>::iterator it = item->mBids.begin();
			it != item->mBids.end(); ++it) {
		fprintf(output, "Bid=%d,%lu,%lu,%lld\r\n", it->mBuyer, it->mCopper,
				it->mCredits, (long long) it->mBidTime);
	}

	fprintf(output, "\r\n");

	fflush(output);
	fclose(output);

	return true;
}

AuctionHouseItem * AuctionHouseManager::LoadItem(int id) {
	std::string path = GetPath(id);
	if (!Platform::FileExists(path)) {
		g_Logs.data->error("No file for CS item [%v]", path.c_str());
		return NULL;
	}

	AuctionHouseItem *item = new AuctionHouseItem();

	FileReader lfr;
	if (lfr.OpenText(path.c_str()) != Err_OK) {
		g_Logs.data->error("Could not open file [%v]", path.c_str());
		return NULL;
	}

//		unsigned long mStartDate;
//		unsigned long mEndDate;

	lfr.CommentStyle = Comment_Semi;
	int r = 0;
	long amt = -1;
	while (lfr.FileOpen() == true) {
		r = lfr.ReadLine();
		lfr.SingleBreak("=");
		lfr.BlockToStringC(0, Case_Upper);
		if (r > 0) {
			if (strcmp(lfr.SecBuffer, "[ENTRY]") == 0) {
				if (item->mId != 0) {
					g_Logs.data->warn("%v contains multiple entries. Auction house items have one entry per file", path.c_str());
					break;
				}
				item->mId = id;
			} else if (strcmp(lfr.SecBuffer, "SELLER") == 0)
				item->mSeller = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "AUCTIONEER") == 0)
				item->mAuctioneer = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "BEGINDATE") == 0)
				item->mStartDate = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "ENDDATE") == 0)
				item->mEndDate = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "RESERVECOPPER") == 0)
				item->mReserveCopper = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "RESERVECREDITS") == 0)
				item->mReserveCredits = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "BUYITNOWCOPPER") == 0)
				item->mBuyItNowCopper = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "BUYITNOWCREDITS") == 0)
				item->mBuyItNowCredits = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "SECONDSREMAINING") == 0)
				item->mSecondsRemaining = lfr.BlockToULongC(1);
			else if (strcmp(lfr.SecBuffer, "COMPLETED") == 0)
				item->mCompleted = lfr.BlockToBoolC(1);
			else if (strcmp(lfr.SecBuffer, "ITEMID") == 0)
				item->mItemId = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "COUNT") == 0)
				item->mCount = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "LOOKID") == 0)
				item->mLookId = lfr.BlockToIntC(1);
			else if (strcmp(lfr.SecBuffer, "BID") == 0) {
				std::string bidText = lfr.BlockToString(1);
				std::vector<std::string> l;
				Util::Split(bidText, ",", l);
				AuctionHouseBid bid;
				if (l.size() == 4) {
					bid.mBuyer = atoi(l[0].c_str());
					bid.mCopper = atol(l[1].c_str());
					bid.mCredits = atol(l[2].c_str());
					bid.mBidTime = atol(l[3].c_str());
				} else {
					g_Logs.data->error("Invalid auction house bid for %v", item->mId);
				}
				item->mBids.push_back(bid);
			} else
				g_Logs.data->warn("Unknown identifier [%v] in file [%v]", lfr.SecBuffer, path.c_str());
		}
	}
	lfr.CloseCurrent();

	item->itemDef = g_ItemManager.GetSafePointerByID(item->mItemId);

	CharacterData *seller = g_CharacterManager.RequestCharacter(item->mSeller,
			true);
	if (seller == NULL) {
		item->mSellerName = "<Missing character>";
	} else {
		item->mSellerName = seller->cdef.css.display_name;
	}

	cs.Enter("AuctionHouseManager::LoadItem");
	mItems[id] = item;
	cs.Leave();

	if (!item->mCompleted) {
		// Finish auction
		AuctionTimerTask *tt = new AuctionTimerTask(item);
		item->timerTask = tt;
		g_TimerManager.AddTask(tt);
	} else {
		// Remove the item completely
		AuctionRemoveTimerTask *tr = new AuctionRemoveTimerTask(item);
		item->timerTask = tr;
		g_TimerManager.AddTask(tr);
	}

	return item;
}

bool AuctionHouseManager::RemoveItem(int id) {
	std::string path = GetPath(id);
	if (!Platform::FileExists(path)) {
		g_Logs.data->error("No file for auction house item [%v] to remove", path.c_str());
		return false;
	}
	cs.Enter("AuctionHouseManager::RemoveItem");

	std::map<int, AuctionHouseItem*>::iterator it = mItems.find(id);
	if (it != mItems.end()) {
		if (it->second->timerTask != NULL) {
			it->second->timerTask->cancel();
			delete it->second->timerTask;
		}

		delete it->second;
		mItems.erase(it);
	}
	cs.Leave();
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "%d.del", id);
	std::string filename = Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "AuctionHouse"), buf);
	if (!Platform::FileExists(filename) || remove(filename.c_str()) == 0) {
		if (!rename(path.c_str(), buf) == 0) {
			g_Logs.data->error("Failed to remove auction house item %v",
					id);
			return false;
		}
	}
	g_Logs.event->info("Auction house item %v removed", id);
	return true;
}

std::string AuctionHouseManager::GetPath(int id) {
	char buf[128];
	Util::SafeFormat(buf, sizeof(buf), "%d.txt", id);
	return Platform::JoinPath(Platform::JoinPath(g_Config.ResolveUserDataPath(), "AuctionHouse"), buf);
}

bool remainingSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	time_t now = time(NULL);
	return l1->mEndDate < now && l2->mEndDate >= now ?
			true :
			(l1->mEndDate >= now && l2->mEndDate < now ?
					false : l1->mEndDate > l2->mEndDate);
}

bool endTimeSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	return l1->mEndDate > l2->mEndDate;
}

bool levelSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	int lev1 = l1->itemDef == NULL ? 0 : l1->itemDef->mLevel;
	int lev2 = l2->itemDef == NULL ? 0 : l2->itemDef->mLevel;
	return lev1 > lev2;
}

bool qualitySort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	int q1 = l1->itemDef == NULL ? 0 : l1->itemDef->mQualityLevel;
	int q2 = l2->itemDef == NULL ? 0 : l2->itemDef->mQualityLevel;
	return q1 > q2;
}

bool typeSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	int t1 = l1->itemDef == NULL ? 0 : l1->itemDef->mType;
	int t2 = l2->itemDef == NULL ? 0 : l2->itemDef->mType;
	return t1 > t2;
}

bool weaponTypeSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	int t1 = l1->itemDef == NULL ? 0 : l1->itemDef->mWeaponType;
	int t2 = l2->itemDef == NULL ? 0 : l2->itemDef->mWeaponType;
	return t1 > t2;
}

bool buyCopperSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	return l1->mBuyItNowCopper > l2->mBuyItNowCopper;
}

bool buyCreditsSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	return l1->mBuyItNowCredits > l2->mBuyItNowCredits;
}

bool bidCopperSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	unsigned long c1 =
			l1->mBids.size() == 0 ? 0 : l1->mBids[l1->mBids.size() - 1].mCopper;
	unsigned long c2 =
			l2->mBids.size() == 0 ? 0 : l2->mBids[l2->mBids.size() - 1].mCopper;
	return c1 > c2;
}

bool bidCreditsSort(const AuctionHouseItem *l1, const AuctionHouseItem *l2) {
	unsigned long c1 =
			l1->mBids.size() == 0 ?
					0 : l1->mBids[l1->mBids.size() - 1].mCredits;
	unsigned long c2 =
			l2->mBids.size() == 0 ?
					0 : l2->mBids[l2->mBids.size() - 1].mCredits;
	return c1 > c2;
}

void AuctionHouseManager::Search(AuctionHouseSearch &search,
		std::vector<AuctionHouseItem*> &results) {

	for (std::map<int, AuctionHouseItem*>::iterator it = mItems.begin();
			it != mItems.end(); ++it) {
		if (search.mMaxRows > -1 && results.size() >= search.mMaxRows)
			break;

		if (it->second->itemDef == NULL) {
			g_Logs.data->warn("Item in auction house does not link to a valid item. %v",
					it->second->mItemId);
		} else {
			bool matches = it->second->mAuctioneer == 0
					|| search.mAuctioneer == it->second->mAuctioneer;
			if (matches) {
				matches = search.mItemTypeId == -1
						|| search.mItemTypeId == it->second->itemDef->mType;
			}
			if (matches) {
				matches = search.mQualityId == -1
						|| search.mQualityId
								== it->second->itemDef->mQualityLevel;
			}
			if (matches) {
				matches = it->second->itemDef->mLevel >= search.mLevelStart
						&& it->second->itemDef->mLevel <= search.mLevelEnd;
			}
			if (matches) {
				matches = it->second->mBuyItNowCopper
						>= search.mBuyPriceCopperStart
						&& it->second->mBuyItNowCopper
								<= search.mBuyPriceCopperEnd;
			}
			if (matches) {
				matches = it->second->mBuyItNowCredits == 0
						|| (it->second->mBuyItNowCredits
								>= search.mBuyPriceCreditsStart
								&& it->second->mBuyItNowCredits
										<= search.mBuyPriceCreditsEnd);
			}
//			if (matches) {
//				unsigned long bidCopper = 0;
//				unsigned long bidCredits = 0;
//				if(it->second->mBids.size() > 0) {
//					bidCopper = it->second->mBids[it->second->mBids.size() - 1].mCopper;
//					bidCredits = it->second->mBids[it->second->mBids.size() - 1].mCredits;
//				}
//				matches = bidCopper >= search.mit->second->mBuyItNowCredits == 0
//						|| (it->second->mBuyItNowCredits
//								>= search.mBuyPriceCreditsStart
//								&& it->second->mBuyItNowCredits
//										<= search.mBuyPriceCreditsEnd);
//			}
			if (matches) {
				matches = search.mSearch.length() == 0
						|| Util::CaseInsensitiveStringFind(
								it->second->itemDef->mDisplayName,
								search.mSearch);
			}

			if (matches) {
				results.push_back(it->second);
			}
		}
	}

	switch (search.mOrder) {
	case AuctionHouseSearchOrder::REMAINING:
		sort(results.begin(), results.end(), remainingSort);
		break;
	case AuctionHouseSearchOrder::END_TIME:
		sort(results.begin(), results.end(), endTimeSort);
		break;
	case AuctionHouseSearchOrder::LEVEL:
		sort(results.begin(), results.end(), levelSort);
		break;
	case AuctionHouseSearchOrder::QUALITY:
		sort(results.begin(), results.end(), qualitySort);
		break;
	case AuctionHouseSearchOrder::TYPE:
		sort(results.begin(), results.end(), typeSort);
		break;
	case AuctionHouseSearchOrder::WEAPON_TYPE:
		sort(results.begin(), results.end(), weaponTypeSort);
		break;
	case AuctionHouseSearchOrder::BUY_COPPER:
		sort(results.begin(), results.end(), buyCopperSort);
		break;
	case AuctionHouseSearchOrder::BUY_CREDITS:
		sort(results.begin(), results.end(), buyCreditsSort);
		break;
	case AuctionHouseSearchOrder::BID_COPPER:
		sort(results.begin(), results.end(), bidCopperSort);
		break;
	case AuctionHouseSearchOrder::BID_CREDITS:
		sort(results.begin(), results.end(), bidCreditsSort);
		break;
	}

	if (search.mReverse)
		std::reverse(results.begin(), results.end());
}

int AuctionHouseManager::LoadItems(void) {
	for (std::map<int, AuctionHouseItem*>::iterator it = mItems.begin();
			it != mItems.end(); ++it)
		delete it->second;
	mItems.clear();

	Platform_DirectoryReader r;
	std::string dir = r.GetDirectory();
	r.SetDirectory(Platform::JoinPath(g_Config.ResolveUserDataPath(), "AuctionHouse"));
	r.ReadFiles();
	r.SetDirectory(dir);

	std::vector<std::string>::iterator it;
	for (it = r.fileList.begin(); it != r.fileList.end(); ++it) {
		std::string p = *it;
		if (Util::HasEnding(p, ".txt")) {
			LoadItem(atoi(Platform::Basename(p).c_str()));
		}
	}

	return 0;
}

void AuctionHouseManager::ConnectToSite() {
	/* We need a session with which to send private message from the auction system. This
	 * may need to be refreshed periodically
	 */
	session = new HTTPD::SiteSession();
	SiteClient sc(g_Config.ServiceAuthURL);
	sc.refreshXCSRF(session);
	sc.login(session, "eebay", "changeme!");
	sc.refreshXCSRF(session);
}

AuctionHouseItem * AuctionHouseManager::GetItem(int id) {
	cs.Enter("AuctionHouseManager::GetItem");
	std::map<int, AuctionHouseItem*>::iterator it = mItems.find(id);
	AuctionHouseItem *item = it == mItems.end() ? NULL : it->second;
	cs.Leave();
	return item;
}

//
// AuctionHouseSearch
//
AuctionHouseSearch::AuctionHouseSearch() {
	mAuctioneer = 0;
	mItemTypeId = -1;
	mQualityId = -1;
	mMaxRows = -1;
	mReverse = false;
	mOrder = AuctionHouseSearchOrder::REMAINING;
	mBuyPriceCopperStart = 0;
	mBuyPriceCopperEnd = 99999999;
	mBuyPriceCreditsStart = 0;
	mBuyPriceCreditsEnd = 9999999;
	mLevelStart = 0;
	mLevelEnd = 99;
}

AuctionHouseSearch::~AuctionHouseSearch() {
}
